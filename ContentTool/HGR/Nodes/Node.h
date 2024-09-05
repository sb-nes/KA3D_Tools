#pragma once

namespace tools::hgr::nodes {
	enum NodeClassId {
		NODE_OTHER = (0 << 4),
		NODE_CAMERA = (1 << 4),
		NODE_CONSOLE = (2 << 4),
		NODE_DUMMY = (3 << 4),
		NODE_LIGHT = (4 << 4),
		NODE_SCENE = (5 << 4),
		NODE_FIRST_VISUAL = (6 << 4),
		NODE_LINES = (6 << 4),
		NODE_MESH = (7 << 4),
		NODE_OTHER_VISUAL = (8 << 4),
		NODE_PARTICLESYSTEM = (9 << 4),
		NODE_LAST_VISUAL = (9 << 4),
		NODE_USERCLASSID_1 = (12 << 4),
		NODE_USERCLASSID_2 = (13 << 4),
		NODE_USERCLASSID_3 = (14 << 4),
		NODE_USERCLASSID_4 = (15 << 4),
	}; 

	enum NodeFlags {
		NODE_ENABLED = 1, // bit 0
		NODE_LIGHTTYPE = (3 << 2), // bits 2:3
		NODE_LIGHTTYPE_SHIFT = 2,
		NODE_CLASS = (31 << 4), // bits 4:8
		NODE_BOUNDWORLDSPACE = (1 << 9),
		NODE_DEFAULTS = NODE_ENABLED + NODE_OTHER,
	};
}