#pragma once
#include "Vector3.h"

// Quaternion unitario q = (w, x, y, z) = cos(a/2) + sin(a/2) * (eixo unitario).
// Representa uma rotacao de 'a' graus em torno do eixo.
// 
// Rotacionar um ponto p:   p' = q * (0,p) * q^-1
// Compor rotacoes:         q = q2 * q1                 (q1 atua primeiro)
class Quaternion {
public:
    float w, x, y, z;

    Quaternion(float w = 1.f, float x = 0.f, float y = 0.f, float z = 0.f);

    static Quaternion fromAxisAngle(const Vector3& axis, float deg);

    Quaternion operator*(const Quaternion& o) const;
    Quaternion conjugate() const;                      // inversa se |q| = 1
    Quaternion normalized() const;

    // p' = q p q^-1, usando a forma expandida
    //   t = 2 (qv x p);  p' = p + w t + qv x t
    Vector3 rotate(const Vector3& p) const;
};

