#pragma once

Bounds GetBoundingBox(const BasicVertex *pVertice, UINT numVertice);
Bounds GetBoundingBox(const SkinnedVertex *pVertice, UINT numVertice);

inline Vector3 Reflect(const Vector3 vi, const Vector3 vn) { return vi - 2 * vi.Dot(vn) * vn; }

inline Vector3 Slide(const Vector3 vi, const Vector3 vn) { return vi - vi.Dot(vn) * vn; }

void FootOfAltitude(const Vector3& a, const Vector3& b, const Vector3& p, Vector3 *pOutPoint);