#pragma once
#include <string.h>
#include "../Common/PrimitiveTypes.h"
#include "HGRCommon.h"
#include "Entity.h"

#define MINVERSION 170
#define MAXVERSION 193

namespace tools::hgr {
	struct hgr_info {
		u16			m_ver{}; // major*100+minor
		u32			m_exportedVer = 0x020903;
		u16			m_dataFlags{}; // <data-descriptor>
		u32			m_platformID{}; // for m_ver > 180 only

		u32			check_id {}; // initial value [hex] = 0x12345600 | Upon check_id(), increments by 0x1
	};

	struct scene_param_info {
		u8			fogType{};
		f32			fogStart{};
		f32			fogEnd{};
		f32			fogColour[3]{};
	};

	struct entity_info {
		u32						Texture_Count {};
		u32						Material_Count {};
		u32						Primitive_Count {};
		u32						Mesh_Count {};
		u32						Camera_Count {};
		u32						Light_Count {};
		u32						Dummy_Count {};
		u32						Shape_Count {};
		u32						OtherNodes_Count {};
		u32						TransformAnimation_Count {};
		u32						UserProperties_Count {};
	};

	struct texture_info {
		std::string				name{};
		int						type;
	};

	struct material_info {
		std::string				name{};
		std::string				shaderName{};
		s32						lightmap_info{};
		u8						texParamCount{};
		texParam*				TexParams{};
		u8						vec4ParamCount{};
		vec4Param*				Vec4Params{};
		u8						floatParamCount{};
		floatParam*				FloatParams{};
	};

	struct primitive_info {
		u32					verts{};
		u32					indices{};
		vertFormat*			formats{};
		u16					matIndex{};
		u16					primitiveIndex{};
		vertArray*			VArray{};
		u16*				indexData{};
		u8					usedBoneCount{};
		u8*					usedBones{};
	};

	struct keyframeSequence {
		s32					keyCount{};
		std::string			dataFormat{};
		f32					scale{};
		f32					bias[4]{};
		f32*				value{};
	};

	struct transformAnimation {
		std::string			nodeName{};
		u8					posKeyRate{}; // position
		u8					rotKeyRate{}; // rotation
		u8					sclKeyRate{}; // scale
		u8					endBehaviour{};

		keyframeSequence	posKeyData{};
		keyframeSequence	rotKeyData{};
		keyframeSequence	sclKeyData{};
	};

	struct userProperty {
		std::string			nodeName{};
		std::string			propertyText{};
		//  <propertytext> can contain following optional properties separated by newline:
		//  Time = <time for following particle systems to start>
		//	Particle = <particle system name without.prt suffix>
		//
		//	Example(starts first fire after 1 second, then snow after 3 seconds) :
		//	Time = 1
		//	Particle = "fire"
		//	Time = 3
		//	Particle = "snow"
	};
}