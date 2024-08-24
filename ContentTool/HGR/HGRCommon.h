#pragma once
#include "../Common/PrimitiveTypes.h"
#include <string.h>
#include <map>

namespace tools::hgr {

	struct texParam {
		std::string param_type{};
		u16 texIndex{}; // which texture the material uses
	};

	struct vec4Param {
		std::string param_type{};
		f32 value[4]{};
	};

	struct floatParam {
		std::string param_type{};
		f32 value{};
	};

	struct vertFormat {
		std::string type{};
		std::string format{};
	};

	struct vertArray { // maybe i'll convert it to a class
		f32 scale{};
		f32 bias[4]{0};
		s16* value{};
		f32* actualValue{};
		u32 size{};
	};

	enum dataFormats
	{
		/** Null vertex data format. */
		DF_NONE,
		/** Single 32-bit float. */
		DF_S_32,
		/** Single 16-bit integer. */
		DF_S_16,
		/** Single 8-bit integer. */
		DF_S_8,
		/** 32-bit float 2-vector. */
		DF_V2_32,
		/** 16-bit integer 2-vector. */
		DF_V2_16,
		/** 8-bit integer 2-vector. */
		DF_V2_8,
		/** 32-bit float 3-vector. */
		DF_V3_32,
		/** 16-bit integer 3-vector. */
		DF_V3_16,
		/** 8-bit integer 3-vector. */
		DF_V3_8,
		/** 32-bit float 4-vector. */
		DF_V4_32,
		/** 16-bit integer 4-vector. */
		DF_V4_16,
		/** 8-bit integer 4-vector. */
		DF_V4_8,
		/** 5-bit integer 4-vector. */
		DF_V4_5,
		/** Number of different vertex data formats. */
		DF_SIZE
	};

	static std::map<std::string, dataFormats> dfMap{ // std::map::operator[] is a non-const member function; Therefore, i can't make it const.
		{"DF_S_16", dataFormats::DF_S_16}, // int16
		{"DF_V2_16", dataFormats::DF_V2_16}, // int16[2]
		{"DF_V3_16", dataFormats::DF_V3_16}, // int16[3]
		{"DF_V4_16", dataFormats::DF_V4_16}, // int16[4]
		{"DF_V3_32", dataFormats::DF_V3_32}, // float32[3]
		{"DF_V4_32", dataFormats::DF_V4_32}, // float32[4]
	};

	enum DataType
	{
		/** Vertex has model space position data. */
		DT_POSITION,
		/** Vertex has screen space position data. */
		DT_POSITIONT,
		/** Vertex has bone weights used in skinning. */
		DT_BONEWEIGHTS,
		/** Vertex has bone indices used in skinning. */
		DT_BONEINDICES,
		/** Vertex has normal pointing away from surface. */
		DT_NORMAL,
		/** Vertex has diffuse color data. */
		DT_DIFFUSE,
		/** Vertex has specular color data. */
		DT_SPECULAR,
		/** Vertex has texture layer 0. */
		DT_TEX0,
		/** Vertex has texture layer 1. */
		DT_TEX1,
		/** Vertex has texture layer 2. */
		DT_TEX2,
		/** Vertex has texture layer 3. */
		DT_TEX3,
		/** Vertex has tangent data. */
		DT_TANGENT,
		/** Number of different vertex component types. */
		DT_SIZE
	};
}
