#include "VertexFormat.h"
#include "../ToolCommon.h"

namespace tools::VertexFormat {

	static const char* const DATATYPE_NAMES[] = {
		/** Vertex has model space position data. */
		"DT_POSITION",
		/** Vertex has screen space position data. */
		"DT_POSITIONT",
		/** Vertex has bone weights used in skinning. */
		"DT_BONEWEIGHTS",
		/** Vertex has bone indices used in skinning. */
		"DT_BONEINDICES",
		/** Vertex has normal pointing away from surface. */
		"DT_NORMAL",
		/** Vertex has diffuse color data. */
		"DT_DIFFUSE",
		/** Vertex has specular color data. */
		"DT_SPECULAR",
		/** Vertex has texture layer 0. */
		"DT_TEX0",
		/** Vertex has texture layer 1. */
		"DT_TEX1",
		/** Vertex has texture layer 2. */
		"DT_TEX2",
		/** Vertex has texture layer 3. */
		"DT_TEX3",
		/** Vertex has tangent data. */
		"DT_TANGENT",
	};

	const char* const DATAFORMAT_NAMES[] = {
		/** Null vertex data format. */
		"NONE",
		/** Single 32-bit float. */
		"S_32",
		/** Single 16-bit unsigned integer. */
		"S_16",
		/** Single 8-bit unsigned integer. */
		"S_8",
		/** 32-bit float 2-vector. */
		"V2_32",
		/** 16-bit unsigned integer 2-vector. */
		"V2_16",
		/** 8-bit unsigned integer 2-vector. */
		"V2_8",
		/** 32-bit float 3-vector. */
		"V3_32",
		/** 16-bit unsigned integer 3-vector. */
		"V3_16",
		/** 8-bit unsigned integer 3-vector. */
		"V3_8",
		/** 32-bit float 4-vector. */
		"V4_32",
		/** 16-bit unsigned integer 4-vector. */
		"V4_16",
		/** 8-bit unsigned integer 4-vector. */
		"V4_8",
		/** 5-bit unsigned integer 4-vector. */
		"V4_5",
	};

	static_assert(sizeof(DATAFORMAT_NAMES) / sizeof(DATAFORMAT_NAMES[0]) == DF_SIZE);
	static_assert(sizeof(DATATYPE_NAMES) / sizeof(DATATYPE_NAMES[0]) == DT_SIZE);

	// Returns data dimensions (in number of components) of specified data format.
	[[nodiscard]]
	int		
	getDataDim(DataFormat df) {
		switch (df)
		{
		case DF_S_32:	return 1;
		case DF_S_16:	return 1;
		case DF_S_8:	return 1;
		case DF_V2_32:	return 2;
		case DF_V2_16:	return 2;
		case DF_V2_8:	return 2;
		case DF_V3_32:	return 3;
		case DF_V3_16:	return 3;
		case DF_V3_8:	return 3;
		case DF_V4_32:	return 4;
		case DF_V4_16:	return 4;
		case DF_V4_8:	return 4;
		case DF_V4_5:	return 4;
		case DF_NONE:	return 0;
		case DF_SIZE:	return 0;
		}
		return 0;
	}

	// Returns data size (in bytes) of specified data format.
	[[nodiscard]]
	int		
	getDataSize(DataFormat df) {
		switch (df)
		{
		case DF_S_32:			return 4;
		case DF_S_16:			return 2;
		case DF_S_8:			return 1;
		case DF_V2_32:			return 8;
		case DF_V2_16:			return 4;
		case DF_V2_8:			return 2;
		case DF_V3_32:			return 12;
		case DF_V3_16:			return 6;
		case DF_V3_8:			return 3;
		case DF_V4_32:			return 16;
		case DF_V4_16:			return 8;
		case DF_V4_8:			return 4;
		case DF_V4_5:			return 2;
		case DF_NONE:			return 0;
		case DF_SIZE:			return 0;
		}
		return 0;
	}

	const char* toString(DataFormat df) {
		assert(df < DF_SIZE);
		return DATAFORMAT_NAMES[df];
	}

	const char* toString(DataType dt) {
		assert(dt < DT_SIZE);
		return DATATYPE_NAMES[dt];
	}

	DataFormat toDataFormat(const char* str)
	{
		for (int i = 0; i < DF_SIZE; ++i)
			if (!strcmp(toString(DataFormat(i)), str)) return DataFormat(i);
		return DF_SIZE;
	}

	DataType toDataType(const char* str)
	{
		for (int i = 0; i < DT_SIZE; ++i)
			if (!strcmp(toString(DataType(i)), str))
				return DataType(i);
		return DT_SIZE;
	}
}