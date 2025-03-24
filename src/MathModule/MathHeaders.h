#pragma once

#include <cassert>

#include <cmath>
#include <algorithm>
#include <array>
#include <iterator>
#include <string>
#include <vector>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>
#include <DirectXMath.h>

#include "SimpleMath.h"

#include "framework.h"

using DirectX::XM_PI;
using DirectX::XM_2PI;
using DirectX::XM_PIDIV2;
using DirectX::XMMin;
using DirectX::XMMax;
using DirectX::XMMatrixPerspectiveFovLH;
using DirectX::XMMatrixOrthographicOffCenterLH;
using DirectX::XMConvertToRadians;
using DirectX::XMMatrixLookAtLH;
using DirectX::XMVectorSet;
using DirectX::XMVectorSetW;

using DirectX::SimpleMath::Vector2;
using DirectX::SimpleMath::Vector3;
using DirectX::SimpleMath::Vector4;
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Quaternion;

using DirectX::BoundingFrustum;
using DirectX::BoundingBox;

constexpr float MachineEpsilon = std::numeric_limits<float>::epsilon();
inline constexpr float gamma(int n) { return (n * MachineEpsilon) / (1 - n * MachineEpsilon); }

#include "Vector12.h"

#include "Particle.h"

#include "Ray.h"
#include "Bounds.h"
#include "Collisions.h"

#include "Plane.h"
#include "Frustum.h"

#include "Camera.h"
#include "Joint.h"
#include "Material.h"

#include "Transform.h"
#include "Vertex.h"

#include "ColorUtils.h"

#include "MathUtils.h"
