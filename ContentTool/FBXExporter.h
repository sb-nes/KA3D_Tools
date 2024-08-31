#pragma once
#include "ToolCommon.h"
#include <fbxsdk.h>
#include "HGR/HGR.h"

namespace tools {

	void CreateFBX(hgr::assetData& asset, const char* path);
}