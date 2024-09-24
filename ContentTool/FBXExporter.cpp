#include <iostream>
#include <fstream>
#include <filesystem>
#include <Windows.h>
#include "FBXExporter.h"
#include "HGR/Mesh.h"
#include <cmath>
#include <set>

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
// 2) add '/ignore:4099' to Properties/Configuration Properties/ Linker /Command Line

namespace tools {
	namespace {

        static const char* gDiffuseElementName = "DiffuseUV";
        // static const char* gAmbientElementName = "AmbientUV";
        // static const char* gEmissiveElementName = "EmissiveUV";

        class Exporter {
        public:

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

            // Exports a scene to an FBX file
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

                _outPath = _outPath + "\\" + pFilename;
                // Initialize the exporter by providing a filename.
                if (lExporter->Initialize(_outPath.c_str(), pFileFormat, pSdkManager->GetIOSettings()) == false)
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

            bool CreateScene(FbxScene*& pScene) {
                lRootNode = pScene->GetRootNode();

                // Can you build a node tree?
                for (auto hgrNode : _assets.Nodes) {
                    CreateHGRNode(pScene, hgrNode);
                }

                return true;
            }

            FbxNode* CreateHGRNode(FbxScene*& pScene, hgr::node& hgrNode) { // Recursive Function to create the tree structure
                if (_uid.contains(hgrNode.name)) {
                    if (hgrNode.parentIndex == 4294967295) {
                        return lRootNode->FindChild(hgrNode.name.c_str()); // returns parent node if exists
                    } else {
                        FbxNode* lParent = CreateHGRNode(pScene, _assets.Nodes[hgrNode.parentIndex]); // Create or Find Parent Node
                        return lParent->FindChild(hgrNode.name.c_str());
                    }
                }

                // if Node doesn't exist :
                if (hgrNode.parentIndex == 4294967295) {
                    FbxNode* lNode = CreateNode(pScene, hgrNode);
                    lRootNode->AddChild(lNode);
                    _uid.insert(hgrNode.name);
                    return lNode;
                } else {
                    FbxNode* lParent = CreateHGRNode(pScene, _assets.Nodes[hgrNode.parentIndex]); // Create or Find Parent Node
                    assert(lParent);
                    FbxNode* lNode = CreateNode(pScene, hgrNode);
                    lParent->AddChild(lNode);
                    _uid.insert(hgrNode.name);
                    return lNode;
                }
            }

            [[nodiscard]]
            FbxNode* CreateNode(FbxScene*& pScene, hgr::node hgrNode) {
                FbxNode* lNode = FbxNode::Create(pScene, hgrNode.name.c_str());

                // TODO: Complete implementation of other types
                // Type of NODE to add Attribute
                switch (hgrNode.classID) {
                    case hgr::NODE_MESH:
                        CreateHGRMesh(pScene, _assets.meshInfo[hgrNode.index], lNode);
                    break;

                    case hgr::NODE_CAMERA:
                        CreateCamera(pScene, _assets.cameraInfo[hgrNode.index], lNode);
                    break;

                    case hgr::NODE_LIGHT:
                        CreateLight(pScene, _assets.lightInfo[hgrNode.index], lNode);
                    break;

                    case hgr::NODE_LINES:
                    break;

                    case hgr::NODE_DUMMY:
                    break;

                    case hgr::NODE_OTHER:
                    break;

                    default:
                        char const* configfile = "Unimplemented Type!";
                        throw std::runtime_error(std::string("Failed: ") + configfile);
                    break;
                }

                SetTransform(lNode, hgrNode.modeltm);

                return lNode;
            }

            void SetTransform(FbxNode*& lNode, math::float3x4 modeltm)
            {
                // TODO: Set Node Transform; Also fix it
                FbxVector4 Position(modeltm.x[0], modeltm.x[1], modeltm.x[2]);
                FbxVector4 Rotation(0, 0, 0);
                FbxVector4 Scale(1.0, 1.0, 1.0);

                lNode->LclTranslation.Set(Position);
                lNode->LclRotation.Set(Rotation);
                lNode->LclScaling.Set(Scale);
            }

            void CreateHGRMesh(FbxScene*& pScene, hgr::mesh& hgrMesh, FbxNode*& lNode) {
                hgr::material_info mat_info;
                hgr::texture_info tex_info;

                for (u32 i{ 0 };i < hgrMesh.primCount;++i) {
                    u32 primitiveID = hgrMesh.primIndex[i];

                    mat_info = _assets.matInfo[_assets.primInfo[primitiveID].matIndex];
                    tex_info = _assets.texInfo[mat_info.TexParams[0].texIndex];

                    FbxMesh* lMesh = CreateMesh(pScene, hgrMesh.name.c_str(), _assets.primInfo[primitiveID],
                                                   mat_info, tex_info);

                    lNode->SetNodeAttribute(lMesh);
                    lNode->SetShadingMode(FbxNode::eTextureShading);

                    CreateHGRTexture(pScene, lMesh, mat_info, (_texPath + "\\" + (tex_info.name).substr(0, tex_info.name.length() - 4) + ".jpg").c_str());
                }
            }

            [[nodiscard]]
            FbxMesh* CreateMesh(FbxScene* pScene, const char* pName, hgr::primitive_info prim_info,
                                hgr::material_info mat_info, hgr::texture_info tex_info) {

                // Mesh -> node
                // Vertices -> control points
                // Normal, Diffuse, Ambient -> FbxGeometryElements


                u32 i{ 0 }; int pos{ -1 }; int tex0{ -1 }; int j{ 0 };
                FbxMesh* lMesh = FbxMesh::Create(pScene, pName); // Object Container -> pScene

                std::vector<FbxVector4> lVertices;
                std::vector<FbxVector2> lVectors;

                for (i = 0; i < prim_info.formatCount; ++i) {
                    if (VertexFormat::toDataType(prim_info.formats[i].type.c_str()) == VertexFormat::DT_POSITION) {
                        pos = i;
                    }
                    else if (VertexFormat::toDataType(prim_info.formats[i].type.c_str()) == VertexFormat::DT_TEX0) {
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

                    lVectors.push_back({ -y + 1.0, x });
                    //lVectors.push_back({ x, y });
                }

                for (i = 0; i < prim_info.indices; ++i) {
                    lUVDiffuseElement->GetDirectArray().Add(lVectors[prim_info.indexData[i]]);
                }

                //lUVDiffuseElement->GetIndexArray().SetCount(prim_info.indices); // Resize

                // Create Polygon and Map UVs
                assert(prim_info.primitiveType == tools::hgr::Mesh::PRIM_TRI);
                for (i = 0; i < (prim_info.indices / 3); i++) // It's a trigon
                {
                    // we won't use the default way of assigning textures, as we have
                    // textures on more than just the default (diffuse) channel.
                    lMesh->BeginPolygon(-1, -1, false);

                    // Invert Faces / Normals Direction
                    // Writing in inverse order
                    for (j = 2; j >= 0; --j) {
                        // this function points 
                        lMesh->AddPolygon(lPolygonVertices[i * 3 + j]); // For each index

                        // Update the index array of the UVs for diffuse, ambient and emissive
                        lUVDiffuseElement->GetIndexArray().SetAt(i * 3 + j, j);
                    }

                    lMesh->EndPolygon();
                }

                delete[] lPolygonVertices;
                lVectors.clear();
                lVertices.clear();
                return lMesh;
            }

            // Fix Wrong Blend Mode being used -> Occurs due to using PNG instead of JPG
            void CreateHGRTexture(FbxScene* pScene, FbxMesh* pMesh, hgr::material_info hgrMaterial, const char* texture) {
                FbxSurfacePhong* lMaterial = NULL;

                //get the node of mesh, add material for it.
                FbxNode* lNode = pMesh->GetNode();
                lNode->mCullingType = FbxNode::eCullingOnCCW;
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
                        lLayerElementMaterial->SetMappingMode(FbxLayerElement::eAllSame);
                        lLayerElementMaterial->SetReferenceMode(FbxLayerElement::eIndexToDirect);

                        lLayer->SetMaterials(lLayerElementMaterial);
                        lLayerElementMaterial->GetIndexArray().Add(0);

                        lMaterial = FbxSurfacePhong::Create(pScene, lMaterialName.Buffer());

                        lMaterial->AmbientFactor.Set(1.0);
                        lMaterial->DiffuseFactor.Set(1.0);
                        lMaterial->TransparencyFactor.Set(0.4);

                        lMaterial->ShadingModel.Set(lShadingName);
                        lMaterial->Shininess.Set(0.5);
                        lMaterial->SpecularFactor.Set(0.3);

                        for (int i = 0; i < hgrMaterial.vec4ParamCount;++i) {
                            if (hgrMaterial.Vec4Params[i].param_type == "AMBIENTC") {
                                FbxDouble3 lAmbient(hgrMaterial.Vec4Params[i].value[0], hgrMaterial.Vec4Params[i].value[1], hgrMaterial.Vec4Params[i].value[2]);
                                lMaterial->Ambient.Set(lAmbient);
                                lMaterial->AmbientFactor.Set(hgrMaterial.Vec4Params[i].value[3]);
                            } else if (hgrMaterial.Vec4Params[i].param_type == "DIFFUSEC") {
                                FbxDouble3 lDiffuse(hgrMaterial.Vec4Params[i].value[0], hgrMaterial.Vec4Params[i].value[1], hgrMaterial.Vec4Params[i].value[2]);
                                lMaterial->Diffuse.Set(lDiffuse);
                                //lMaterial->TransparencyFactor.Set(hgrMaterial.Vec4Params[i].value[3]);
                            } else if (hgrMaterial.Vec4Params[i].param_type == "SPECULARC") {
                                FbxDouble3 lSpecular(hgrMaterial.Vec4Params[i].value[0], hgrMaterial.Vec4Params[i].value[1], hgrMaterial.Vec4Params[i].value[2]);
                                lMaterial->Specular.Set(lSpecular);
                                lMaterial->SpecularFactor.Set(hgrMaterial.Vec4Params[i].value[3]);
                            } else assert(false && "Vector 4 Parameter not implemented!");
                        }

                        for (int i = 0; i < hgrMaterial.floatParamCount;++i) {
                            if(hgrMaterial.FloatParams[i].param_type == "SHININESS") lMaterial->Shininess.Set(hgrMaterial.FloatParams[i].value);
                        }

                        lNode->AddMaterial(lMaterial);
                    }
                }

                FbxFileTexture* lTexture = FbxFileTexture::Create(pScene, "Diffuse Texture");

                // Set texture properties.
                lTexture->SetFileName(texture);
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

            void CreateCamera(FbxScene*& pScene, hgr::camera& hgrCamera, FbxNode*& lNode)
            {
                if (!pScene) return;

                FbxCamera* lCamera = FbxCamera::Create(pScene, hgrCamera.name.c_str());
                lNode->SetNodeAttribute(lCamera);

                lCamera->SetFormat(FbxCamera::eHD);
                lCamera->SetApertureFormat(FbxCamera::e16mmTheatrical);
                lCamera->SetApertureMode(FbxCamera::eVertical);
                //set camera FOV
                double lFOV = hgrCamera.FOV; // is it degrees or radians
                lCamera->FieldOfViewY.Set(lFOV); // Degrees
                double lFocalLength = lCamera->ComputeFocalLength(lFOV);
                lCamera->FocalLength.Set(lFocalLength);

                // View Frustum Data
                // hgrCamera.front & hgrCamera.back
            }

            void CreateLight(FbxScene*& pScene, hgr::light& hgrLight, FbxNode*& lNode)
            {
                FbxLight* lLight = FbxLight::Create(pScene, hgrLight.name.c_str());

                lLight->LightType.Set(FbxLight::eSpot);
                lLight->CastLight.Set(true);

                // TODO: Add hgr data
                lLight->Color.Set(FbxDouble3(1.0, 0.0, 0.0));
                lLight->Intensity.Set(33.0);
                lLight->OuterAngle.Set(90.0);
                lLight->Fog.Set(100.0);

                // extras
                lLight->DrawGroundProjection.Set(true);
                lLight->DrawVolumetricLight.Set(true);
                lLight->DrawFrontFacingVolumetricLight.Set(false);

                lNode->SetNodeAttribute(lLight);
            }

            // _asset
            void SetAssets(hgr::assetData assets) { _assets = assets; }

            // _texPath
            void SetTexPath(std::string texPath) { _texPath = texPath; }

            // _outPath
            void SetOutPath(std::string outPath) { _outPath = outPath; }

            // gSdkManager
            [[nodiscard]]
            FbxManager* GetFbxManager() { return gSdkManager; }

        private:
            FbxNode*                        lRootNode = nullptr;
            std::set<std::string>           _uid;
            FbxManager*                     gSdkManager = nullptr;
            hgr::assetData                  _assets;
            std::string                     _texPath;
            std::string                     _outPath;
        };

	} // Anonymous Namespace

    void CreateFBX(hgr::assetData& asset, const char* path, const char* texpath, const char* outpath) {
        // Filter the filename from path
        std::string file = path;
        file = file.substr(file.find_last_of("\\") + 1, file.length() - file.find_last_of("\\") - 5);

        // Initialize
        Exporter* ex = new Exporter();
        FbxScene* gScene = FbxScene::Create(ex->GetFbxManager(), file.c_str());

        // Create and save fbx
        //ex->_assets = asset;
        ex->SetAssets(asset);
        ex->SetTexPath(texpath);
        ex->SetOutPath(outpath);
        ex->CreateScene(gScene);
        ex->SaveScene(ex->GetFbxManager(), gScene, file.c_str(), 0);

        // De-allocate memory
        gScene->Destroy();
        delete ex; // automatically calls the destructor
    }
}