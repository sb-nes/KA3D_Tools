#pragma once
#include "../Common/PrimitiveTypes.h"
#include "VertexFormat.h"
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
		u32 size{};
	};
}
