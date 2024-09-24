#pragma once
#include "PrimitiveTypes.h"

#define PI 3.141592653589793

namespace tools::math {
	struct float3 {
		f32 x[3]{};
	};


	struct float4 {
		f32 x,y,z,w;

		float4() = default;

		// copy-conversion construction
		float4( float3& f3 ) : x(f3.x[0]), y(f3.x[1]), z(f3.x[2]), w(0) {}

		// scalar to float4 constructor
		explicit float4( f32 x0, f32 y0, f32 z0, f32 w0 ) : x(x0), y(y0), z(z0), w(w0) {}

		float4 operator=(const float3 other) {
			return float4(other.x[0], other.x[0], other.x[0], 0);
		}
	};

	// Should i make a custom version of float3x4
	struct float3x4 { // is it 4xfloat3 or 3xfloat4 -> its 4xfloat3 | Also, its column-major
		f32 w[3]{};
		f32 x[3]{};
		f32 y[3]{};
		f32 z[3]{};
	};


}
