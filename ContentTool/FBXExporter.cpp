#include <iostream>
#include <fstream>
#include <filesystem>
#include <Windows.h>
#include "FBXExporter.h"

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

            // Forward Declaration
            FbxNode* CreateCube(FbxScene* pScene, char* pName);

            // void CreateTexture(FbxScene* pScene, FbxMesh* pMesh);
            // void CreateMaterials(FbxScene* pScene, FbxMesh* pMesh);
            // void SetCubeDefaultPosition(FbxNode* pCube);

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

            bool CreateScene(FbxScene*& pScene, const char* pSampleFileName) {
                FbxNode* lCube = CreateCube(pScene, "Cube");

                // Build the node tree.
                FbxNode* lRootNode = pScene->GetRootNode();
                lRootNode->AddChild(lCube);

                return true;
            }

            bool CreateScene(FbxScene*& pScene, hgr::primitive_info* prim_info, hgr::entity_info info) {
                FbxNode* lRootNode = pScene->GetRootNode();

                for (u32 i{ 0 };i < info.Primitive_Count;++i) {
                    FbxNode* lMesh = CreateHGRMesh(pScene, "Mesh "+i, prim_info[i]);

                    // Build the node tree.
                    lRootNode->AddChild(lMesh);
                }

                return true;
            }

            FbxNode* CreateHGRMesh(FbxScene* pScene, const char* pName, hgr::primitive_info prim_info) {
                u32 i{ 0 }; int selected{ -1 }; int j{ 0 };
                FbxMesh* lMesh = FbxMesh::Create(pScene, pName); // Object Container -> pScene

                // FbxVector4* lVertices = new FbxVector4[prim_info.verts]; // I'll replace this with std::vector
                std::vector<FbxVector4> lVertices;

                for (i = 0; i < prim_info.formatCount; ++i) {
                    if (VertexFormat::toDataType(prim_info.formats[i].type.c_str()) == VertexFormat::DT_POSITION) {
                        selected = i;
                        break;
                    }
                }
                assert(selected != -1);

                u32 size = VertexFormat::getDataDim(VertexFormat::toDataFormat(prim_info.formats[i].format.c_str()));
                size *= prim_info.verts;

                s16* buf = prim_info.vArray[selected].value;

                // Create Control Points from Vertices
                for (i = 0; i < size; ++i) {
                    lVertices.push_back({ (*buf++) * prim_info.vArray[selected].scale + prim_info.vArray[selected].bias[0], 
                                          (*buf++) * prim_info.vArray[selected].scale + prim_info.vArray[selected].bias[1], 
                                          (*buf++) * prim_info.vArray[selected].scale + prim_info.vArray[selected].bias[2] });
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

                // Set Normals
                FbxGeometryElementNormal* lGeometryElementNormal = lMesh->CreateElementNormal();
                lGeometryElementNormal->SetMappingMode(FbxGeometryElement::eNone);

                lMesh->GenerateNormals(false, false, true);

                // Create UV for Diffuse Channel, Ambient Channel and Emissive Channel [idk if there are any more]

                // Create Polygon and Map UVs
                for (i = 0; i < prim_info.indices/3; i++)
                {
                    // we won't use the default way of assigning textures, as we have
                    // textures on more than just the default (diffuse) channel.
                    lMesh->BeginPolygon(-1, -1, false);

                    for (j = 0; j < 3; j++)
                    {
                        // this function points 
                        lMesh->AddPolygon(lPolygonVertices[i * 3 + j]); // For each index

                        // Update the index array of the UVs for diffuse, ambient and emissive
                        // lUVDiffuseElement->GetIndexArray().SetAt(i * 4 + j, j);
                        // lUVAmbientElement->GetIndexArray().SetAt(i * 4 + j, j);
                        // lUVEmissiveElement->GetIndexArray().SetAt(i * 4 + j, j);

                    }

                    lMesh->EndPolygon();
                }

                // Finalize
                FbxNode* lNode = FbxNode::Create(pScene, pName);
                lNode->SetNodeAttribute(lMesh);
                lNode->SetShadingMode(FbxNode::eTextureShading);

                // CreateTexture(pScene, lMesh); // if none created, Default material will be applied without textures.

                delete[] lPolygonVertices;
                lVertices.clear();
                return lNode;
            }

            // Mesh -> node
            // Vertices -> control points
            // Normal, Diffuse, Ambient -> FbxGeometryElements
            FbxNode* CreateCube(FbxScene* pScene, const char* pName) {
                int i, j;
                FbxMesh* lMesh = FbxMesh::Create(pScene, pName); // Object Container -> pScene

                // Control Point -> Synonym for Vertex; Co-ordinates in XYZ
                FbxVector4 lControlPoint0(-50, 0, 50);
                FbxVector4 lControlPoint1(50, 0, 50);
                FbxVector4 lControlPoint2(50, 100, 50);
                FbxVector4 lControlPoint3(-50, 100, 50);
                FbxVector4 lControlPoint4(-50, 0, -50);
                FbxVector4 lControlPoint5(50, 0, -50);
                FbxVector4 lControlPoint6(50, 100, -50);
                FbxVector4 lControlPoint7(-50, 100, -50);
                // Normal Vectors for Each Face
                FbxVector4 lNormalXPos(1, 0, 0);
                FbxVector4 lNormalXNeg(-1, 0, 0);
                FbxVector4 lNormalYPos(0, 1, 0);
                FbxVector4 lNormalYNeg(0, -1, 0);
                FbxVector4 lNormalZPos(0, 0, 1);
                FbxVector4 lNormalZNeg(0, 0, -1);

                lMesh->InitControlPoints(24); // Initialize the indices buffer size for the mesh
                FbxVector4* lControlPoints = lMesh->GetControlPoints();

                // N-gon formation
                lControlPoints[0] = lControlPoint0; lControlPoints[1] = lControlPoint1; lControlPoints[2] = lControlPoint2; lControlPoints[3] = lControlPoint3; 
                lControlPoints[4] = lControlPoint1; lControlPoints[5] = lControlPoint5; lControlPoints[6] = lControlPoint6; lControlPoints[7] = lControlPoint2; 
                lControlPoints[8] = lControlPoint5; lControlPoints[9] = lControlPoint4; lControlPoints[10] = lControlPoint7; lControlPoints[11] = lControlPoint6;
                lControlPoints[12] = lControlPoint4; lControlPoints[13] = lControlPoint0; lControlPoints[14] = lControlPoint3; lControlPoints[15] = lControlPoint7; 
                lControlPoints[16] = lControlPoint3; lControlPoints[17] = lControlPoint2; lControlPoints[18] = lControlPoint6; lControlPoints[19] = lControlPoint7; 
                lControlPoints[20] = lControlPoint1; lControlPoints[21] = lControlPoint0; lControlPoints[22] = lControlPoint4; lControlPoints[23] = lControlPoint5;

                // We want to have one normal for each vertex (or control point),
                // so we set the mapping mode to eByControlPoint.
                FbxGeometryElementNormal* lGeometryElementNormal = lMesh->CreateElementNormal();
                lGeometryElementNormal->SetMappingMode(FbxGeometryElement::eByControlPoint);

                
                // Here are two different ways to set the normal values.
                bool directCalculation = true;
                if (directCalculation) {
                    // The first method is to manually set the actual normal value
                    // for every control point.
                    lGeometryElementNormal->SetReferenceMode(FbxGeometryElement::eDirect);

                    // For each index
                    lGeometryElementNormal->GetDirectArray().Add(lNormalZPos);
                    lGeometryElementNormal->GetDirectArray().Add(lNormalZPos);
                    lGeometryElementNormal->GetDirectArray().Add(lNormalZPos);
                    lGeometryElementNormal->GetDirectArray().Add(lNormalZPos);
                    lGeometryElementNormal->GetDirectArray().Add(lNormalXPos);
                    lGeometryElementNormal->GetDirectArray().Add(lNormalXPos);
                    lGeometryElementNormal->GetDirectArray().Add(lNormalXPos);
                    lGeometryElementNormal->GetDirectArray().Add(lNormalXPos);
                    lGeometryElementNormal->GetDirectArray().Add(lNormalZNeg);
                    lGeometryElementNormal->GetDirectArray().Add(lNormalZNeg);
                    lGeometryElementNormal->GetDirectArray().Add(lNormalZNeg);
                    lGeometryElementNormal->GetDirectArray().Add(lNormalZNeg);
                    lGeometryElementNormal->GetDirectArray().Add(lNormalXNeg);
                    lGeometryElementNormal->GetDirectArray().Add(lNormalXNeg);
                    lGeometryElementNormal->GetDirectArray().Add(lNormalXNeg);
                    lGeometryElementNormal->GetDirectArray().Add(lNormalXNeg);
                    lGeometryElementNormal->GetDirectArray().Add(lNormalYPos);
                    lGeometryElementNormal->GetDirectArray().Add(lNormalYPos);
                    lGeometryElementNormal->GetDirectArray().Add(lNormalYPos);
                    lGeometryElementNormal->GetDirectArray().Add(lNormalYPos);
                    lGeometryElementNormal->GetDirectArray().Add(lNormalYNeg);
                    lGeometryElementNormal->GetDirectArray().Add(lNormalYNeg);
                    lGeometryElementNormal->GetDirectArray().Add(lNormalYNeg);
                    lGeometryElementNormal->GetDirectArray().Add(lNormalYNeg);
                } else {
                    // The second method is to the possible values of the normals
                    // in the direct array, and set the index of that value
                    // in the index array for every control point.
                    lGeometryElementNormal->SetReferenceMode(FbxGeometryElement::eIndexToDirect);

                    // Add the 6 different normals to the direct array
                    lGeometryElementNormal->GetDirectArray().Add(lNormalZPos);
                    lGeometryElementNormal->GetDirectArray().Add(lNormalXPos);
                    lGeometryElementNormal->GetDirectArray().Add(lNormalZNeg);
                    lGeometryElementNormal->GetDirectArray().Add(lNormalXNeg);
                    lGeometryElementNormal->GetDirectArray().Add(lNormalYPos);
                    lGeometryElementNormal->GetDirectArray().Add(lNormalYNeg);

                    // Now for each control point, we need to specify which normal to use
                    lGeometryElementNormal->GetIndexArray().Add(0); // index of lNormalZPos in the direct array.
                    lGeometryElementNormal->GetIndexArray().Add(0); // index of lNormalZPos in the direct array.
                    lGeometryElementNormal->GetIndexArray().Add(0); // index of lNormalZPos in the direct array.
                    lGeometryElementNormal->GetIndexArray().Add(0); // index of lNormalZPos in the direct array.
                    lGeometryElementNormal->GetIndexArray().Add(1); // index of lNormalXPos in the direct array.
                    lGeometryElementNormal->GetIndexArray().Add(1); // index of lNormalXPos in the direct array.
                    lGeometryElementNormal->GetIndexArray().Add(1); // index of lNormalXPos in the direct array.
                    lGeometryElementNormal->GetIndexArray().Add(1); // index of lNormalXPos in the direct array.
                    lGeometryElementNormal->GetIndexArray().Add(2); // index of lNormalZNeg in the direct array.
                    lGeometryElementNormal->GetIndexArray().Add(2); // index of lNormalZNeg in the direct array.
                    lGeometryElementNormal->GetIndexArray().Add(2); // index of lNormalZNeg in the direct array.
                    lGeometryElementNormal->GetIndexArray().Add(2); // index of lNormalZNeg in the direct array.
                    lGeometryElementNormal->GetIndexArray().Add(3); // index of lNormalXNeg in the direct array.
                    lGeometryElementNormal->GetIndexArray().Add(3); // index of lNormalXNeg in the direct array.
                    lGeometryElementNormal->GetIndexArray().Add(3); // index of lNormalXNeg in the direct array.
                    lGeometryElementNormal->GetIndexArray().Add(3); // index of lNormalXNeg in the direct array.
                    lGeometryElementNormal->GetIndexArray().Add(4); // index of lNormalYPos in the direct array.
                    lGeometryElementNormal->GetIndexArray().Add(4); // index of lNormalYPos in the direct array.
                    lGeometryElementNormal->GetIndexArray().Add(4); // index of lNormalYPos in the direct array.
                    lGeometryElementNormal->GetIndexArray().Add(4); // index of lNormalYPos in the direct array.
                    lGeometryElementNormal->GetIndexArray().Add(5); // index of lNormalYNeg in the direct array.
                    lGeometryElementNormal->GetIndexArray().Add(5); // index of lNormalYNeg in the direct array.
                    lGeometryElementNormal->GetIndexArray().Add(5); // index of lNormalYNeg in the direct array.
                    lGeometryElementNormal->GetIndexArray().Add(5); // index of lNormalYNeg in the direct array.
                }
                
                
                // Array of polygon vertices -> Vertex Indices
                int lPolygonVertices[] = { 0, 1, 2, 3,
                                           4, 5, 6, 7,
                                           8, 9, 10, 11,
                                           12, 13, 14, 15,
                                           16, 17, 18, 19,
                                           20, 21, 22, 23 };

                // Create UV for Diffuse channel - automatically calculate UV co-ords
                FbxGeometryElementUV* lUVDiffuseElement = lMesh->CreateElementUV(gDiffuseElementName);
                FBX_ASSERT(lUVDiffuseElement != NULL);
                lUVDiffuseElement->SetMappingMode(FbxGeometryElement::eByPolygonVertex);
                lUVDiffuseElement->SetReferenceMode(FbxGeometryElement::eIndexToDirect);

                FbxVector2 lVectors0(0, 0);
                FbxVector2 lVectors1(1, 0);
                FbxVector2 lVectors2(1, 1);
                FbxVector2 lVectors3(0, 1);

                lUVDiffuseElement->GetDirectArray().Add(lVectors0);
                lUVDiffuseElement->GetDirectArray().Add(lVectors1);
                lUVDiffuseElement->GetDirectArray().Add(lVectors2);
                lUVDiffuseElement->GetDirectArray().Add(lVectors3);

                // Create UV for Ambient channel
                FbxGeometryElementUV* lUVAmbientElement = lMesh->CreateElementUV(gAmbientElementName);

                lUVAmbientElement->SetMappingMode(FbxGeometryElement::eByPolygonVertex);
                lUVAmbientElement->SetReferenceMode(FbxGeometryElement::eIndexToDirect);

                lVectors0.Set(0, 0);
                lVectors1.Set(1, 0);
                lVectors2.Set(0, 0.418586879968643);
                lVectors3.Set(1, 0.418586879968643);

                lUVAmbientElement->GetDirectArray().Add(lVectors0);
                lUVAmbientElement->GetDirectArray().Add(lVectors1);
                lUVAmbientElement->GetDirectArray().Add(lVectors2);
                lUVAmbientElement->GetDirectArray().Add(lVectors3);

                // Create UV for Emissive channel
                FbxGeometryElementUV* lUVEmissiveElement = lMesh->CreateElementUV(gEmissiveElementName);

                lUVEmissiveElement->SetMappingMode(FbxGeometryElement::eByPolygonVertex);
                lUVEmissiveElement->SetReferenceMode(FbxGeometryElement::eIndexToDirect);

                lVectors0.Set(0.2343, 0);
                lVectors1.Set(1, 0.555);
                lVectors2.Set(0.333, 0.999);
                lVectors3.Set(0.555, 0.666);

                lUVEmissiveElement->GetDirectArray().Add(lVectors0);
                lUVEmissiveElement->GetDirectArray().Add(lVectors1);
                lUVEmissiveElement->GetDirectArray().Add(lVectors2);
                lUVEmissiveElement->GetDirectArray().Add(lVectors3);

                // Now we have set the UVs as eIndexToDirect reference and in eByPolygonVertex  mapping mode
                // we must update the size of the index array.
                lUVDiffuseElement->GetIndexArray().SetCount(24);
                lUVAmbientElement->GetIndexArray().SetCount(24);
                lUVEmissiveElement->GetIndexArray().SetCount(24);

                // Create polygons. Assign texture and texture UV indices.
                for (i = 0; i < 6; i++)
                {
                    // we won't use the default way of assigning textures, as we have
                    // textures on more than just the default (diffuse) channel.
                    lMesh->BeginPolygon(-1, -1, false);

                    for (j = 0; j < 4; j++)
                    {
                        // this function points 
                        lMesh->AddPolygon(lPolygonVertices[i * 4 + j] ); // For each index

                        // Update the index array of the UVs for diffuse, ambient and emissive
                        lUVDiffuseElement->GetIndexArray().SetAt(i * 4 + j, j);
                        lUVAmbientElement->GetIndexArray().SetAt(i * 4 + j, j);
                        lUVEmissiveElement->GetIndexArray().SetAt(i * 4 + j, j);

                    }

                    lMesh->EndPolygon();
                }
                

                FbxNode* lNode = FbxNode::Create(pScene, pName);

                lNode->SetNodeAttribute(lMesh);
                lNode->SetShadingMode(FbxNode::eTextureShading);

                //CreateTexture(pScene, lMesh); // if none created, Default material will be applied 

                return lNode;
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

            // Create materials for pyramid.
            void CreateMaterials(FbxScene* pScene, FbxMesh* pMesh) { 
                for (int i = 0; i < 5; i++) {
                    FbxString lMaterialName = "material";
                    FbxString lShadingName = "Phong";
                    lMaterialName += i;
                    FbxDouble3 lBlack(0.0, 0.0, 0.0);
                    FbxDouble3 lRed(1.0, 0.0, 0.0);
                    FbxDouble3 lColor;
                    FbxSurfacePhong* lMaterial = FbxSurfacePhong::Create(pScene, lMaterialName.Buffer());


                    // Generate primary and secondary colors.
                    lMaterial->Emissive.Set(lBlack);
                    lMaterial->Ambient.Set(lRed);
                    lColor = FbxDouble3(i > 2 ? 1.0 : 0.0,
                        i > 0 && i < 4 ? 1.0 : 0.0,
                        i % 2 ? 0.0 : 1.0);
                    lMaterial->Diffuse.Set(lColor);
                    lMaterial->TransparencyFactor.Set(0.0);
                    lMaterial->ShadingModel.Set(lShadingName);
                    lMaterial->Shininess.Set(0.5);

                    //get the node of mesh, add material for it.
                    FbxNode* lNode = pMesh->GetNode();
                    if (lNode)
                        lNode->AddMaterial(lMaterial);
                }
            }

            // Cube is translated to the left.
            void SetCubeDefaultPosition(FbxNode* pCube)
            {
                pCube->LclTranslation.Set(FbxVector4(-75.0, -50.0, 0.0));
                pCube->LclRotation.Set(FbxVector4(0.0, 0.0, 0.0));
                pCube->LclScaling.Set(FbxVector4(1.0, 1.0, 1.0));
            }


        private:

        };

	} // Anonymous Namespace

    void Importer() {
        // Change the following filename to a suitable filename value.
        const char* lFilename = "C:\\Users\\Soumya\\Desktop\\Cardboard box\\Models and Textures\\Cardboard box.fbx";

        // Initialize the SDK manager. This object handles memory management.
        FbxManager* lSdkManager = FbxManager::Create();

        // Create the IO settings object.
        FbxIOSettings* ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
        lSdkManager->SetIOSettings(ios);

        // Create an importer using the SDK manager.
        FbxImporter* lImporter = FbxImporter::Create(lSdkManager, "");

        // Use the first argument as the filename for the importer.
        if (!lImporter->Initialize(lFilename, -1, lSdkManager->GetIOSettings())) {
            printf("Call to FbxImporter::Initialize() failed.\n");
            printf("Error returned: %s\n\n", lImporter->GetStatus().GetErrorString());
            //return false;
        }

        // Create a new scene so that it can be populated by the imported file.
        FbxScene* lScene = FbxScene::Create(lSdkManager, "myScene");

        // Import the contents of the file into the scene.
        lImporter->Import(lScene);

        // The file is imported, so get rid of the importer.
        lImporter->Destroy();
    }

    void test(hgr::primitive_info* prim_info, hgr::entity_info info) {
        Exporter* ex = new Exporter();
        FbxScene* gScene = FbxScene::Create(ex->gSdkManager, "myScene");

        ex->CreateScene(gScene, prim_info, info);
        ex->SaveScene(ex->gSdkManager, gScene, "TestHGRFile", 0);

        // De-allocate memory
        gScene->Destroy();
        delete ex; // automatically calls the destructor
    }

    void test() {
        Exporter* ex = new Exporter();
        FbxScene* gScene = FbxScene::Create(ex->gSdkManager, "myScene");

        ex->CreateScene(gScene, "mySceneName");
        ex->SaveScene(ex->gSdkManager, gScene, "TestFile", 0);

        // De-allocate memory
        gScene->Destroy();
        delete ex; // automatically calls the destructor
    }
}