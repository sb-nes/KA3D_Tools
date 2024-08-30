#pragma once

namespace tools::hgr::Mesh {

	enum PrimitiveType {
		PRIM_POINT = 0,
		PRIM_LINE = 1,
		PRIM_LINESTRIP = 2,
		PRIM_TRI = 3,
		PRIM_TRISTRIP = 4,
		PRIM_TRIFAN = 5,
		PRIM_SPRITE = 6,
		PRIM_INVALID = 7
	};
}