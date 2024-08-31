#include <iostream>
#include <fstream>
#include <filesystem>
#include <Windows.h>
#include "FBXExporter.h"
#include "HGR/Mesh.h"
#include <cmath>

// If any compilation or linking errors occur, make sure:
// 1) FBX SDK 2020.2 or later is installed on your system
// 2) The Include Path to fbxsdk.h is added to the "Additional Include Directories" (Compiler Settings)
// 3) The Library Paths in the following section point to the correct location

#if _DEBUG
#pragma comment (lib, "C:\\Program Files\\Autodesk\\FBX\\FBX SDK\\2020.3.7\\lib\\x64\\debug\\libfbxsdk-md.lib")
#pragma comment (lib, "C:\\Program Files\\Autodesk\\FBX\\FBX SDK\\2020.3.7\\lib\\x64\\debug\\libxml2-md.lib")
#pragma comment (lib, "C:\\Program Files\\Autodesk\\FBX\\FBX SDK\\2020.3.7\\lib\\x64\\debug\\zlib-md.lib")
#else
#pragma comment (lib, "C:\\Program Files\\Autodesk\\FBX\\FBX SDK\\2020.3.7\\lib\\x64\\release\\libfbxsdk-md.lib")
#pragma comment (lib, "C:\\Program Files\\Autodesk\\FBX\\FBX SDK\\2020.3.7\\lib\\x64\\release\\libxml2-md.lib")
#pragma comment (lib, "C:\\Program Files\\Autodesk\\FBX\\FBX SDK\\2020.3.7\\lib\\x64\\release\\zlib-md.lib")
#endif // _DEBUG

// After linking, it may throw warnings regarding pdb symbols being not found. to fix this, either
// 1) download the symbols from Autodesk's website
// 2) add '/ignore:4099' to Properties/Configuration Properties/ C/C++ /Command Line

namespace tools {
	namespace {

        static const char* gDiffuseElementName = "DiffuseUV";
        static const char* gAmbientElementName = "AmbientUV";
        static const char* gEmissiveElementName = "EmissiveUV";

        class Exporter {
        public:
            FbxManager* gSdkManager = nullptr;
            int         lemmy{ 0 };

#ifdef IOS_REF
#undef  IOS_REF
#define IOS_REF (*(gSdkManager->GetIOSettings()))
#endif

            Exporter() {
                // Create the FBX SDK memory manager object.
                // The SDK Manager allocates and frees memory
                // for almost all the classes in the SDK.
                gSdkManager = FbxManager::Create();
                FbxIOSettings* ios = FbxIOSettings::Create(gSdkManager, IOSROOT);
                gSdkManager->SetIOSettings(ios);
            }
            ~Exporter()
            {
                // Delete the FBX SDK manager.
                // All the objects that
                // (1) have been allocated by the memory manager, AND that
                // (2) have not been explicitly destroyed
                // will be automatically destroyed.
                if (gSdkManager) gSdkManager->Destroy();
            }

            // Exports a scene to a file
            bool SaveScene( FbxManager* pSdkManager, FbxScene* pScene, const char* pFilename, 
                            int pFileFormat, bool pEmbedMedia = false ) {
                int lMajor, lMinor, lRevision;
                bool lStatus = true;

                FbxExporter* lExporter = FbxExporter::Create(pSdkManager, "");

                if (pFileFormat < 0 || pFileFormat >= pSdkManager->GetIOPluginRegistry()->GetWriterFormatCount()) {
                    // Write in fall back format if pEmbedMedia is true
                    pFileFormat = pSdkManager->GetIOPluginRegistry()->GetNativeWriterFormat();

                    if (!pEmbedMedia) {
                        //Try to export in ASCII if possible
                        int lFormatIndex, lFormatCount = pSdkManager->GetIOPluginRegistry()-> GetWriterFormatCount();

                        for (lFormatIndex = 0; lFormatIndex < lFormatCount; lFormatIndex++)
                        {
                            if (pSdkManager->GetIOPluginRegistry()-> WriterIsFBX(lFormatIndex))
                            {
                                FbxString lDesc = pSdkManager->GetIOPluginRegistry()-> GetWriterFormatDescription(lFormatIndex);
                                if (lDesc.Find("ascii") >= 0) {
                                    pFileFormat = lFormatIndex;
                                    break;
                                }
                            }
                        }
                    }
                }

                // Initialize the exporter by providing a filename.
                if (lExporter->Initialize(pFilename, pFileFormat, pSdkManager->GetIOSettings()) == false)
                {
                    //UI_Printf("Call to FbxExporter::Initialize() failed.");
                    //UI_Printf("Error returned: %s", lExporter->GetStatus().GetErrorString());
                    return false;
                }

                FbxManager::GetFileFormatVersion(lMajor, lMinor, lRevision);
                //UI_Printf("FBX version number for this FBX SDK is %d.%d.%d", lMajor, lMinor, lRevision);

                if (pSdkManager->GetIOPluginRegistry()->WriterIsFBX(pFileFormat)) {
                    // Export options determine what kind of data is to be imported.
                    // The default (except for the option eEXPORT_TEXTURE_AS_EMBEDDED)
                    // is true, but here we set the options explicitly.
                    IOS_REF.SetBoolProp(EXP_FBX_MATERIAL, true);
                    IOS_REF.SetBoolProp(EXP_FBX_TEXTURE, true);
                    IOS_REF.SetBoolProp(EXP_FBX_EMBEDDED, pEmbedMedia);
                    IOS_REF.SetBoolProp(EXP_FBX_SHAPE, true);
                    IOS_REF.SetBoolProp(EXP_FBX_GOBO, true);
                    IOS_REF.SetBoolProp(EXP_FBX_ANIMATION, true);
                    IOS_REF.SetBoolProp(EXP_FBX_GLOBAL_SETTINGS, true);
                }

                // Export the scene.
                lStatus = lExporter->Export(pScene);

                // Destroy the exporter.
                lExporter->Destroy();

                return lStatus;
            }

            bool CreateScene(FbxScene*& pScene, hgr::assetData& asset) {
                FbxNode* lRootNode = pScene->GetRootNode();
                hgr::material_info mat_info;
                hgr::texture_info tex_info;
                for (u32 i{ 0 };i < (asset.entityInfo)->Primitive_Count;++i) {
                    mat_info = asset.matInfo[asset.primInfo[i].matIndex];
                    tex_info = asset.texInfo[mat_info.TexParams[0].texIndex];

                    FbxNode* lMesh = CreateHGRMesh(pScene, "Mesh", asset.primInfo[i], 
                                                   mat_info, tex_info);

                    // Build the node tree.
                    lRootNode->AddChild(lMesh);
                }

                return true;
            }

            // Mesh -> node
            // Vertices -> control points
            // Normal, Diffuse, Ambient -> FbxGeometryElements
            FbxNode* CreateHGRMesh(FbxScene* pScene, const char* pName, hgr::primitive_info prim_info, 
                                   hgr::material_info mat_info, hgr::texture_info tex_info) {
                u32 i{ 0 }; int pos{ -1 }; int tex0{ -1 }; int j{ 0 }; lemmy++;
                FbxMesh* lMesh = FbxMesh::Create(pScene, pName); // Object Container -> pScene

                std::vector<FbxVector4> lVertices;
                std::vector<FbxVector2> lVectors;

                for (i = 0; i < prim_info.formatCount; ++i) {
                    if (VertexFormat::toDataType(prim_info.formats[i].type.c_str()) == VertexFormat::DT_POSITION) {
                        pos = i;
                    } else if (VertexFormat::toDataType(prim_info.formats[i].type.c_str()) == VertexFormat::DT_TEX0) {
                        tex0 = i;
                    }
                }
                assert(pos != -1 && tex0 != -1);

                s16* buf = prim_info.vArray[pos].value;

                // Create Control Points from Vertices
                for (i = 0; i < prim_info.verts; ++i) {
                    lVertices.push_back({ (*buf++) * prim_info.vArray[pos].scale + prim_info.vArray[pos].bias[0],
                                          (*buf++) * prim_info.vArray[pos].scale + prim_info.vArray[pos].bias[1],
                                          (*buf++) * prim_info.vArray[pos].scale + prim_info.vArray[pos].bias[2] });
                }

                // Map Vertices to Faces/Indices
                lMesh->InitControlPoints(prim_info.indices);
                FbxVector4* lControlPoints = lMesh->GetControlPoints();

                // Create Array of Polygon Vertices / Vertex Indices
                int* lPolygonVertices = new int[prim_info.indices];

                for (i = 0;i < prim_info.indices;++i) {
                    lControlPoints[i] = lVertices[prim_info.indexData[i]]; // Set using IndexData
                    lPolygonVertices[i] = i;
                }

                // Set Normals -> is it necessary to assign if i'm generating at a later stage
                //FbxGeometryElementNormal* lGeometryElementNormal = lMesh->CreateElementNormal();
                //lGeometryElementNormal->SetMappingMode(FbxGeometryElement::eNone);

                // Create UV for Diffuse Channel, Ambient Channel and Emissive Channel [idk if there are any more]
                
                // Diffuse channel - automatically calculate UV co-ords
                FbxGeometryElementUV* lUVDiffuseElement = lMesh->CreateElementUV(gDiffuseElementName);
                FBX_ASSERT(lUVDiffuseElement != NULL);
                lUVDiffuseElement->SetMappingMode(FbxGeometryElement::eByPolygonVertex);
                lUVDiffuseElement->SetReferenceMode(FbxGeometryElement::eIndexToDirect);

                buf = prim_info.vArray[tex0].value;
                double x, y;

                for (i = 0; i < prim_info.verts; ++i) {
                    x = (*buf++) * prim_info.vArray[tex0].scale + prim_info.vArray[tex0].bias[0];
                    y = (*buf++) * prim_info.vArray[tex0].scale + prim_info.vArray[tex0].bias[1];
                    
                    //lVectors.push_back({ -y + 1.0, x });
                    lVectors.push_back({ x, y });
                }

                for (i = 0; i < prim_info.indices; ++i) {
                    lUVDiffuseElement->GetDirectArray().Add(lVectors[prim_info.indexData[i]]);
                }
                
                //lUVDiffuseElement->GetIndexArray().SetCount(prim_info.indices); // Resize

                // Create Polygon and Map UVs
                assert(prim_info.primitiveType == tools::hgr::Mesh::PRIM_TRI);
                for (i = 0; i < (prim_info.indices/3); i++) // It's a trigon
                {
                    // we won't use the default way of assigning textures, as we have
                    // textures on more than just the default (diffuse) channel.
                    lMesh->BeginPolygon(-1, -1, false);

                    for (j = 0; j < 3; j++) {
                        // this function points 
                        lMesh->AddPolygon(lPolygonVertices[i * 3 + j]); // For each index

                        // Update the index array of the UVs for diffuse, ambient and emissive
                        lUVDiffuseElement->GetIndexArray().SetAt(i * 3 + j, j);
                        // lUVAmbientElement->GetIndexArray().SetAt(i * 4 + j, j);
                        // lUVEmissiveElement->GetIndexArray().SetAt(i * 4 + j, j);

                    }

                    lMesh->EndPolygon();
                }

                // Finalize
                FbxNode* lNode = FbxNode::Create(pScene, pName);
                lNode->SetNodeAttribute(lMesh);
                lNode->SetShadingMode(FbxNode::eTextureShading);
                lMesh->GenerateNormals(true, false, false);

                CreateHGRTexture(pScene, lMesh, mat_info, ("textures\\" + (tex_info.name).substr(0, tex_info.name.length() - 4) + ".png").c_str());
                // if none created, Default material will be applied without textures.

                delete[] lPolygonVertices;
                lVectors.clear();
                lVertices.clear();
                return lNode;
            }

            void CreateHGRTexture(FbxScene* pScene, FbxMesh* pMesh, hgr::material_info hgrMaterial, const char* texture) {
                // A texture need to be connected to a property on the material,
                // so let's use the material (if it exists) or create a new one
                FbxSurfacePhong* lMaterial = NULL;

                //get the node of mesh, add material for it.
                FbxNode* lNode = pMesh->GetNode();
                if (lNode) {
                    lMaterial = lNode->GetSrcObject<FbxSurfacePhong>(0);
                    if (lMaterial == NULL) {
                        FbxString lMaterialName = hgrMaterial.name.c_str();
                        FbxString lShadingName = hgrMaterial.shaderName.c_str();
                        FbxDouble3 lBlack(0.0, 0.0, 0.0);
                        FbxDouble3 lRed(1.0, 0.0, 0.0);
                        FbxDouble3 lDiffuseColor(0.75, 0.75, 0.0);

                        FbxLayer* lLayer = pMesh->GetLayer(0);

                        // Create a layer element material to handle proper mapping.
                        FbxLayerElementMaterial* lLayerElementMaterial = FbxLayerElementMaterial::Create(pMesh, lMaterialName.Buffer());

                        // This allows us to control where the materials are mapped.  Using eAllSame
                        // means that all faces/polygons of the mesh will be assigned the same material.
                        lLayerElementMaterial->SetMappingMode(FbxLayerElement::eAllSame);
                        lLayerElementMaterial->SetReferenceMode(FbxLayerElement::eIndexToDirect);

                        // Save the material on the layer
                        lLayer->SetMaterials(lLayerElementMaterial);

                        // Add an index to the lLayerElementMaterial.  Since we have only one, and are using eAllSame mapping mode,
                        // we only need to add one.
                        lLayerElementMaterial->GetIndexArray().Add(0);

                        lMaterial = FbxSurfacePhong::Create(pScene, lMaterialName.Buffer());

                        // Generate primary and secondary colors.
                        lMaterial->Emissive.Set(lBlack);
                        lMaterial->Ambient.Set(lRed);
                        lMaterial->AmbientFactor.Set(1.);
                        // Add texture for diffuse channel
                        lMaterial->Diffuse.Set(lDiffuseColor);
                        lMaterial->DiffuseFactor.Set(1.);
                        lMaterial->TransparencyFactor.Set(0.4);

                        lMaterial->ShadingModel.Set(lShadingName);
                        lMaterial->Shininess.Set(0.5);
                        lMaterial->Specular.Set(lBlack);
                        lMaterial->SpecularFactor.Set(0.3);

                        for (int i = 0; i < hgrMaterial.vec4ParamCount;++i) {
                            if (hgrMaterial.Vec4Params[i].param_type == "AMBIENTC") {
                                FbxDouble3 lAmbient(hgrMaterial.Vec4Params[i].value[0], hgrMaterial.Vec4Params[i].value[1], hgrMaterial.Vec4Params[i].value[2]);
                                lMaterial->Ambient.Set(lAmbient);
                                lMaterial->AmbientFactor.Set(hgrMaterial.Vec4Params[i].value[3]);
                            } else if (hgrMaterial.Vec4Params[i].param_type == "DIFFUSEC") {
                                FbxDouble3 lDiffuse(hgrMaterial.Vec4Params[i].value[0], hgrMaterial.Vec4Params[i].value[1], hgrMaterial.Vec4Params[i].value[2]);
                                lMaterial->Diffuse.Set(lDiffuse);
                                lMaterial->TransparencyFactor.Set(hgrMaterial.Vec4Params[i].value[3]);
                            } else if (hgrMaterial.Vec4Params[i].param_type == "SPECULARC") {
                                FbxDouble3 lSpecular(hgrMaterial.Vec4Params[i].value[0], hgrMaterial.Vec4Params[i].value[1], hgrMaterial.Vec4Params[i].value[2]);
                                lMaterial->Specular.Set(lSpecular);
                                // lMaterial->SpecularFactor.Set(hgrMaterial.Vec4Params[i].value[3]);
                            } else assert(false && "Vector 4 Parameter not implemented!");
                        }

                        for (int i = 0; i < hgrMaterial.floatParamCount;++i) {
                            if(hgrMaterial.FloatParams[i].param_type == "SHININESS") lMaterial->Shininess.Set(hgrMaterial.FloatParams[i].value);
                        }

                        lNode->AddMaterial(lMaterial);
                    }
                }

                FbxFileTexture* lTexture = FbxFileTexture::Create(pScene, hgrMaterial.name.c_str());

                // Set texture properties.
                lTexture->SetFileName(texture); // Resource file is in current directory.
                lTexture->SetTextureUse(FbxTexture::eStandard);
                lTexture->SetMappingType(FbxTexture::eUV);
                lTexture->SetMaterialUse(FbxFileTexture::eModelMaterial);
                lTexture->SetSwapUV(false);

                lTexture->SetTranslation(0.0, 0.0);
                lTexture->SetScale(1.0, 1.0);
                lTexture->SetRotation(0.0, 0.0);

                lTexture->UVSet.Set(FbxString(gDiffuseElementName)); // Connect texture to the proper UV

                // don't forget to connect the texture to the corresponding property of the material
                if (lMaterial) lMaterial->Diffuse.ConnectSrcObject(lTexture);
            }

            // Create texture and material for cube.
            void CreateTexture(FbxScene* pScene, FbxMesh* pMesh)
            {
                // A texture need to be connected to a property on the material,
                // so let's use the material (if it exists) or create a new one
                FbxSurfacePhong* lMaterial = NULL;

                //get the node of mesh, add material for it.
                FbxNode* lNode = pMesh->GetNode();
                if (lNode) {
                    lMaterial = lNode->GetSrcObject<FbxSurfacePhong>(0);
                    if (lMaterial == NULL) {
                        FbxString lMaterialName = "toto";
                        FbxString lShadingName = "Phong";
                        FbxDouble3 lBlack(0.0, 0.0, 0.0);
                        FbxDouble3 lRed(1.0, 0.0, 0.0);
                        FbxDouble3 lDiffuseColor(0.75, 0.75, 0.0);

                        FbxLayer* lLayer = pMesh->GetLayer(0);

                        // Create a layer element material to handle proper mapping.
                        FbxLayerElementMaterial* lLayerElementMaterial = FbxLayerElementMaterial::Create(pMesh, lMaterialName.Buffer());

                        // This allows us to control where the materials are mapped.  Using eAllSame
                        // means that all faces/polygons of the mesh will be assigned the same material.
                        lLayerElementMaterial->SetMappingMode(FbxLayerElement::eAllSame);
                        lLayerElementMaterial->SetReferenceMode(FbxLayerElement::eIndexToDirect);

                        // Save the material on the layer
                        lLayer->SetMaterials(lLayerElementMaterial);

                        // Add an index to the lLayerElementMaterial.  Since we have only one, and are using eAllSame mapping mode,
                        // we only need to add one.
                        lLayerElementMaterial->GetIndexArray().Add(0);

                        lMaterial = FbxSurfacePhong::Create(pScene, lMaterialName.Buffer());

                        // Generate primary and secondary colors.
                        lMaterial->Emissive.Set(lBlack);
                        lMaterial->Ambient.Set(lRed);
                        lMaterial->AmbientFactor.Set(1.);
                        // Add texture for diffuse channel
                        lMaterial->Diffuse.Set(lDiffuseColor);
                        lMaterial->DiffuseFactor.Set(1.);
                        lMaterial->TransparencyFactor.Set(0.4);
                        lMaterial->ShadingModel.Set(lShadingName);
                        lMaterial->Shininess.Set(0.5);
                        lMaterial->Specular.Set(lBlack);
                        lMaterial->SpecularFactor.Set(0.3);
                        lNode->AddMaterial(lMaterial);
                    }
                }

                FbxFileTexture* lTexture = FbxFileTexture::Create(pScene, "Diffuse Texture");

                // Set texture properties.
                lTexture->SetFileName("scene03.jpg"); // Resource file is in current directory.
                lTexture->SetTextureUse(FbxTexture::eStandard);
                lTexture->SetMappingType(FbxTexture::eUV);
                lTexture->SetMaterialUse(FbxFileTexture::eModelMaterial);
                lTexture->SetSwapUV(false);
                lTexture->SetTranslation(0.0, 0.0);
                lTexture->SetScale(1.0, 1.0);
                lTexture->SetRotation(0.0, 0.0);
                lTexture->UVSet.Set(FbxString(gDiffuseElementName)); // Connect texture to the proper UV


                // don't forget to connect the texture to the corresponding property of the material
                if (lMaterial)
                    lMaterial->Diffuse.ConnectSrcObject(lTexture);

                lTexture = FbxFileTexture::Create(pScene, "Ambient Texture");
               
                // Set texture properties.
                lTexture->SetFileName("gradient.jpg"); // Resource file is in current directory.
                lTexture->SetTextureUse(FbxTexture::eStandard);
                lTexture->SetMappingType(FbxTexture::eUV);
                lTexture->SetMaterialUse(FbxFileTexture::eModelMaterial);
                lTexture->SetSwapUV(false);
                lTexture->SetTranslation(0.0, 0.0);
                lTexture->SetScale(1.0, 1.0);
                lTexture->SetRotation(0.0, 0.0);
                lTexture->UVSet.Set(FbxString(gAmbientElementName)); // Connect texture to the proper UV
               
                // don't forget to connect the texture to the corresponding property of the material
                if (lMaterial)
                    lMaterial->Ambient.ConnectSrcObject(lTexture);
               
                lTexture = FbxFileTexture::Create(pScene, "Emissive Texture");
               
                // Set texture properties.
                lTexture->SetFileName("spotty.jpg"); // Resource file is in current directory.
                lTexture->SetTextureUse(FbxTexture::eStandard);
                lTexture->SetMappingType(FbxTexture::eUV);
                lTexture->SetMaterialUse(FbxFileTexture::eModelMaterial);
                lTexture->SetSwapUV(false);
                lTexture->SetTranslation(0.0, 0.0);
                lTexture->SetScale(1.0, 1.0);
                lTexture->SetRotation(0.0, 0.0);
                lTexture->UVSet.Set(FbxString(gEmissiveElementName)); // Connect texture to the proper UV
               
                // don't forget to connect the texture to the corresponding property of the material
                if (lMaterial)
                    lMaterial->Emissive.ConnectSrcObject(lTexture);
            }

        private:

        };

	} // Anonymous Namespace

    void CreateFBX(hgr::assetData& asset, const char* path) {
        // Initialize
        Exporter* ex = new Exporter();
        FbxScene* gScene = FbxScene::Create(ex->gSdkManager, "hgrScene");

        // Filter the filename from path
        std::string file = path;
        file = file.substr(file.find_last_of("\\") + 1, file.length() - file.find_last_of("\\") - 5);

        // Create and save fbx
        ex->CreateScene(gScene, asset);
        ex->SaveScene(ex->gSdkManager, gScene, file.c_str(), 0);

        // De-allocate memory
        gScene->Destroy();
        delete ex; // automatically calls the destructor
    }
}