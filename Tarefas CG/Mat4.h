#pragma once
#include "Vector3.h"
#include "Quaternion.h"

// anguloss em GRAUS
class Mat4 {
public:
    float m[4][4];

    Mat4();

    static Mat4 translate(float tx, float ty, float tz);
    static Mat4 scale(float sx, float sy, float sz);
    static Mat4 rotateX(float deg);
    static Mat4 rotateY(float deg);
    static Mat4 rotateZ(float deg);

    // Rotacao em torno de um eixo arbitrario
    // monta uma base ortonormal (u, v, w) com w = eixo,
    // gira em torno de z' nessa base e volta:  R = B * Rz(theta) * B^T.
    static Mat4 rotateAxis(const Vector3& axis, float deg);

    // QUATERNIO: q = (cos(a/2), sin(a/2)*eixo) e a matriz
    static Mat4 rotateAxisQuat(const Vector3& axis, float deg);
    static Mat4 fromQuaternion(const Quaternion& q);

    // Cisalhamento:  x' = x + xy*y + xz*z
    //                y' = y + yx*x + yz*z
    //                z' = z + zx*x + zy*y
    static Mat4 shear(float xy, float xz, float yx, float yz, float zx, float zy);

    // Espelho
    static Mat4 mirror(const Vector3& p, const Vector3& n);

    Mat4 operator*(const Mat4& o) const;
    Mat4 inverse() const;                     // Gauss-Jordan

    Vector3 point(const Vector3& v) const;    // w = 1
    Vector3 dir(const Vector3& v) const;      // w = 0
    Vector3 dirTransposed(const Vector3& v) const;
};

