#pragma once

#include <cassert>

#include <cmath>
#include <algorithm>
#include <array>
#include <iterator>
#include <string>
#include <vector>

#include <DirectXMath.h>

#include "SimpleMath.h"
#include "SimpleMath.inl"

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
using DirectX::SimpleMath::Plane;
using DirectX::SimpleMath::Matrix;
using DirectX::SimpleMath::Quaternion;

using DirectX::BoundingBox;
using DirectX::BoundingSphere;
using DirectX::BoundingFrustum;
using DirectX::BoundingOrientedBox;
using DirectX::ContainmentType;

#ifdef _MSC_VER
#define MachineEpsilon (std::numeric_limits<float>::epsilon() * 0.5)
#else
static float MachineEpsilon = std::numeric_limits<float>::epsilon() * 0.5;
#endif
inline float gamma(int n) { return (n * MachineEpsilon) / (1 - n * MachineEpsilon); }

#include "Ray.h"
#include "Bounds.h"
#include "ShapeBase.h"
#include "Sphere.h"

#include "Camera.h"
#include "Frustum.h"
#include "Joint.h"
#include "Material.h"

#include "Transform.h"
#include "Vertex.h"

#include "ColorUtils.h"
#include "MathUtils.h"
