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
		f32 x[3]{};
		f32 y[3]{};
		f32 z[3]{};
		f32 w[3]{};
	};

	struct float3x3 { // is it 4xfloat3 or 3xfloat4 -> its 4xfloat3 | Also, its column-major
		f32 x[3]{};
		f32 y[3]{};
		f32 z[3]{};

		float3x3() = default;

		float3x3(const float3x4 other) {
			x[0] = other.x[0];
			x[1] = other.x[1];
			x[2] = other.x[2];
			y[0] = other.y[0];
			y[1] = other.y[1];
			y[2] = other.y[2];
			z[0] = other.z[0];
			z[1] = other.z[1];
			z[2] = other.z[2];
		}

		float3x3 operator=(const float3x4 other) {
			float3x3 retn;
			retn.x[0] = other.x[0];
			retn.x[1] = other.x[1];
			retn.x[2] = other.x[2];
			retn.y[0] = other.y[0];
			retn.y[1] = other.y[1];
			retn.y[2] = other.y[2];
			retn.z[0] = other.z[0];
			retn.z[1] = other.z[1];
			retn.z[2] = other.z[2];
			return retn;
		}

		float get(int c, int d) {
			float r = 0.0;
			switch (c) {
			case 0:
				r = x[d];
				break;
			case 1:
				r = y[d];
				break;
			case 2:
				r = z[d];
				break;
			default:
				break;
			}
			return r;
		}
	};

}
