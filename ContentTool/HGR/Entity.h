#pragma once
#include <string.h>
#include "../Common/PrimitiveTypes.h"
#include "../Common/Math.h"
#include "Nodes/Node.h"

namespace tools::hgr {
	struct node {
		std::string				name{};
		math::float3x4			modeltm{}; // Transform
		u32						nodeFlags{}; // Refer to Node::NodeFlags
		u32						id{u32_invalid_id}; // Node Class ID ?
		u32						parentIndex{}; // -1 if no parent

		std::vector<u32>		childIndex;
		bool					isEnabled{ false };
		u32						classID{};
		u32						index{u32_invalid_id};
	};

	struct meshbone {
		u32						boneNodeIndex{};
		math::float3x4			invresttm; // BoneInverseRestTransform
	};

	struct mesh : node {
		u32						primCount{};
		u32*					primIndex{};
		u32						meshboneCount{};
		meshbone*				meshbone{};
	};

	struct camera : node {
		f32				front{};
		f32				back{};
		f32				FOV{}; // Horizon FOV in Radians
	};

	struct light : node {
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

	enum NodeClassId
	{
		/** Node is either plain node or unknown derived class. Default type. */
		NODE_OTHER = (0 << 4),
		/** Node is Camera. */
		NODE_CAMERA = (1 << 4),
		/** Node is Console. */
		NODE_CONSOLE = (2 << 4),
		/** Node is Dummy. */
		NODE_DUMMY = (3 << 4),
		/** Node is Light. */
		NODE_LIGHT = (4 << 4),
		/** Node is Scene. */
		NODE_SCENE = (5 << 4),
		/** First class id of Visuals. */
		NODE_FIRST_VISUAL = (6 << 4),
		/** Node is Lines. */
		NODE_LINES = (6 << 4),
		/** Node is Mesh. */
		NODE_MESH = (7 << 4),
		/** Node is Visual, but not Mesh or ParticleSystem. */
		NODE_OTHER_VISUAL = (8 << 4),
		/** Node is ParticleSystem. */
		NODE_PARTICLESYSTEM = (9 << 4),
		/** Last class id of Visuals. */
		NODE_LAST_VISUAL = (9 << 4),
		/** The first of the class IDs reserved to the user application. */
		NODE_USERCLASSID_1 = (12 << 4),
		/** The second of the class IDs reserved to the user application. */
		NODE_USERCLASSID_2 = (13 << 4),
		/** The third of the class IDs reserved to the user application. */
		NODE_USERCLASSID_3 = (14 << 4),
		/** The fourth of the class IDs reserved to the user application. */
		NODE_USERCLASSID_4 = (15 << 4),
	};

	enum NodeFlags
	{
		/** Node enabled flag. Exact usage semantics is dependent on derived class. */
		NODE_ENABLED = 1, // bit 0
		/** Light type mask for Light class instances. */
		NODE_LIGHTTYPE = (3 << 2), // bits 2:3
		/** Shift needed to access light type. */
		NODE_LIGHTTYPE_SHIFT = 2,
		/** NodeClassId of this Node. Default is NODE_OTHER. */
		NODE_CLASS = (31 << 4), // bits 4:8
		/** Set if Node's bounding volumes are stored in world space coordinates. */
		NODE_BOUNDWORLDSPACE = (1 << 9),
		/** Default type flags for Node. */
		NODE_DEFAULTS = NODE_ENABLED || NODE_OTHER,
	};

	enum BehaviourType
	{
		/** Animation loops to start after reaching end. */
		BEHAVIOUR_REPEAT,
		/** Animation changes direction at the ends. */
		BEHAVIOUR_OSCILLATE,
		/** Animation resets to start frame at the end. */
		BEHAVIOUR_RESET,
		/** Animation stops at the last frame. */
		BEHAVIOUR_STOP,
		/** Number of supported behaviours. */
		BEHAVIOUR_COUNT
	};
}