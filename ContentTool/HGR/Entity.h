#pragma once
#include <string.h>
#include "../Common/PrimitiveTypes.h"
#include "../Common/Math.h"

namespace tools::hgr {
	struct node {
		std::string				name{};
		math::float3x4			modeltm{};
		u32						nodeFlags{}; // Refer to Node::NodeFlags
		u32						parentIndex{}; // -1 if no parent
	};

	struct meshbone {
		u32						boneNodeIndex{};
		math::float3x4			invresttm;
	};

	struct mesh : node {
		//node					Node{};
		u32						primCount{};
		u32*					primIndex{};
		u32						meshboneCount{};
		u32*					meshbone{};
	};

	struct camera : node {
		node			Node{};
		f32				front{};
		f32				back{};
		f32				FOV{}; // Horizon FOV in Radians
	};

	struct light : node {
		node			Node{};
		math::float3	colour{};
		f32				reserved1{};
		f32				reserved2{};
		f32				farAttenStart{};
		f32				farAttenEnd{};
		f32				inner{};
		f32				outer{};
		u8				type{}; // refer to Light::Type

	};

	struct dummy : node {
		node			Node{};
		math::float3	boxMin{};
		math::float3	boxMax{};
	};

	struct line {
		math::float3	start{};
		math::float3	end{};
	};

	struct path {
		u32				beginLine{};
		u32				endLine{};
	};

	struct shape : node {
		s32				lineCount{};
		s32				pathCount{};
		line*			lines{};
		path*			paths{};
	};
}