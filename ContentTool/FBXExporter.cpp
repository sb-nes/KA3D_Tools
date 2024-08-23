#include <iostream>
#include <fstream>
#include <filesystem>
#include <Windows.h>
#include "FBXExporter.h"
#include "HGR/HGR.h"

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
        class Exporter {

            FbxManager* gSdkManager = nullptr;
#ifdef IOS_REF
#undef  IOS_REF
#define IOS_REF (*(gSdkManager->GetIOSettings()))
#endif

            hgr::hgr_info* header;

            // Exports a scene to a file
            bool SaveScene( FbxManager* pSdkManager, FbxScene* pScene, const char* pFilename, 
                            int pFileFormat, bool pEmbedMedia ) {
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
}