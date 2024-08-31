#pragma once
#include "PrimitiveTypes.h"

#define PI 3.141592653589793

namespace tools::math {
	struct float3 {
		f32 x[3]{};
	};

	struct float4 {
		f32 x,y,z,w;
	};

	// Should i make a custom version of float3x4
	struct float3x4 { // is it 4xfloat3 or 3xfloat4 -> its 4xfloat3 | Also, its column-major
		f32 w[3]{};
		f32 x[3]{};
		f32 y[3]{};
		f32 z[3]{};
	};
}
