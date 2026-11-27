#pragma once
#include "Vector3.h"

// q = (w, x, y, z) = cos(a/2) + sin(a/2) * (eixo unitario).
// rotacao de 'a' graus em torno do eixo.

class Quaternion {
public:
    float w, x, y, z;

    Quaternion(float w = 1.f, float x = 0.f, float y = 0.f, float z = 0.f);

    static Quaternion fromAxisAngle(const Vector3& axis, float deg);

    Quaternion operator*(const Quaternion& o) const;
    Quaternion conjugate() const;                      // inversa se |q| = 1
    Quaternion normalized() const;

    //   p' = q p q^-1
    //   t = 2 (qv x p);  p' = p + w t + qv x t
    Vector3 rotate(const Vector3& p) const;
};

