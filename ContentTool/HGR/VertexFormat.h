#pragma once

#include <map>
#include <string>

namespace tools::VertexFormat {
	enum DataFormat
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
	
	// Returns data dimensions (in number of components) of specified data format.
	int		
	getDataDim(DataFormat df);
	
	// Returns data size (in bytes) of specified data format.
	int		
	getDataSize(DataFormat df);

	const char* toString(DataType dt);
	const char* toString(DataFormat dt);
	DataFormat toDataFormat(const char* str);
	DataType toDataType(const char* str);
}