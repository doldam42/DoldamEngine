#pragma once

Bounds GetBoundingBox(const BasicVertex *pVertice, UINT numVertice);
Bounds GetBoundingBox(const SkinnedVertex *pVertice, UINT numVertice);

inline Vector3 Reflect(const Vector3 vi, const Vector3 vn) { return vi - 2 * vi.Dot(vn) * vn; }

inline Vector3 Slide(const Vector3 vi, const Vector3 vn) { return vi - vi.Dot(vn) * vn; }

void FootOfAltitude(const Vector3 &a, const Vector3 &b, const Vector3 &p, Vector3 *pOutPoint);

/*
 * 입력값: a, b, N (0 <= a < b < N)
 * 출력값: 0과 (N-1)(N+2)/2 사이의 연속되고, 중복이 없는 정수값
 * a가 0일 땐: b = 0 ~ N - 1 → offset = b
 * a가 1일 땐: b = 1 ~ N - 1 → offset = N + b - 1 
 * a가 2일 땐: b = 2 ~ N - 1 → offset = N + (N - 1) + (b - 2)
 */
UINT GetAscendingOrderedPairIndex(UINT a, UINT b, UINT N) { return a * (2 * N - a + 1) / 2 + b - a; }
