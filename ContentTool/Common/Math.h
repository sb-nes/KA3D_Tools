#pragma once
#include "PrimitiveTypes.h"

namespace tools::math {
	struct float3 {
		f32 x[3]{};
	};

	struct float3x4 { // is it 4xfloat3 or 3xfloat4
		f32 x[4]{};
		f32 y[4]{};
		f32 z[4]{};
	};
}
