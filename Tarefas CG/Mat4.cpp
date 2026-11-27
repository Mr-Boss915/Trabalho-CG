#include "Mat4.h"
#include <cmath>
#include <stdexcept>
#include <utility>

namespace {
    constexpr float PI = 3.14159265358979323846f;
    float rad(float deg) { return deg * PI / 180.f; }
}

Mat4::Mat4() {
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            m[i][j] = (i == j) ? 1.f : 0.f;
}

Mat4 Mat4::translate(float tx, float ty, float tz) {
    Mat4 r;
    r.m[0][3] = tx;
    r.m[1][3] = ty;
    r.m[2][3] = tz;
    return r;
}

Mat4 Mat4::scale(float sx, float sy, float sz) {
    Mat4 r;
    r.m[0][0] = sx;
    r.m[1][1] = sy;
    r.m[2][2] = sz;
    return r;
}

Mat4 Mat4::rotateX(float deg) {
    float c = std::cos(rad(deg)), s = std::sin(rad(deg));
    Mat4 r;
    r.m[1][1] = c;  r.m[1][2] = -s;
    r.m[2][1] = s;  r.m[2][2] = c;
    return r;
}

Mat4 Mat4::rotateY(float deg) {
    float c = std::cos(rad(deg)), s = std::sin(rad(deg));
    Mat4 r;
    r.m[0][0] = c;  r.m[0][2] = s;
    r.m[2][0] = -s; r.m[2][2] = c;
    return r;
}

Mat4 Mat4::rotateZ(float deg) {
    float c = std::cos(rad(deg)), s = std::sin(rad(deg));
    Mat4 r;
    r.m[0][0] = c;  r.m[0][1] = -s;
    r.m[1][0] = s;  r.m[1][1] = c;
    return r;
}

Mat4 Mat4::rotateAxis(const Vector3& axis, float deg) {
    // Base ortonormal (u, v, w) --- w = eixo de rotacao.
    Vector3 w = axis.normalized();
    Vector3 helper = (std::fabs(w.x) < 0.9f) ? Vector3(1.f, 0.f, 0.f)
        : Vector3(0.f, 1.f, 0.f);
    Vector3 u = helper.cross(w).normalized();
    Vector3 v = w.cross(u);                     // u x v = w

    // B tem u, v, w nas COLUNAS
    Mat4 B, Bt;
    B.m[0][0] = u.x; B.m[0][1] = v.x; B.m[0][2] = w.x;
    B.m[1][0] = u.y; B.m[1][1] = v.y; B.m[1][2] = w.y;
    B.m[2][0] = u.z; B.m[2][1] = v.z; B.m[2][2] = w.z;
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            Bt.m[i][j] = B.m[j][i];            // inversa = transposta

    // mundo -> base, gira em torno de z', base -> mundo
    return B * rotateZ(deg) * Bt;
}

Mat4 Mat4::fromQuaternion(const Quaternion& qin) {
    Quaternion q = qin.normalized();
    float w = q.w, x = q.x, y = q.y, z = q.z;
    Mat4 r;
    r.m[0][0] = 1 - 2 * (y * y + z * z); r.m[0][1] = 2 * (x * y - w * z);     r.m[0][2] = 2 * (x * z + w * y);
    r.m[1][0] = 2 * (x * y + w * z);     r.m[1][1] = 1 - 2 * (x * x + z * z); r.m[1][2] = 2 * (y * z - w * x);
    r.m[2][0] = 2 * (x * z - w * y);     r.m[2][1] = 2 * (y * z + w * x);     r.m[2][2] = 1 - 2 * (x * x + y * y);
    return r;
}

Mat4 Mat4::rotateAxisQuat(const Vector3& axis, float deg) {
    return fromQuaternion(Quaternion::fromAxisAngle(axis, deg));
}

Mat4 Mat4::shear(float xy, float xz, float yx, float yz, float zx, float zy) {
    Mat4 r;
    r.m[0][1] = xy; r.m[0][2] = xz;
    r.m[1][0] = yx; r.m[1][2] = yz;
    r.m[2][0] = zx; r.m[2][1] = zy;
    return r;
}

Mat4 Mat4::mirror(const Vector3& p, const Vector3& n) {
    Vector3 nn = n.normalized();
    float a[3] = { nn.x, nn.y, nn.z };
    Mat4 H;                                     // H = I - 2 n n^T
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            H.m[i][j] -= 2.f * a[i] * a[j];
    // leva o plano para a origem, reflete, e traz de volta
    return translate(p.x, p.y, p.z) * H * translate(-p.x, -p.y, -p.z);
}

Mat4 Mat4::operator*(const Mat4& o) const {
    Mat4 r;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j) {
            float s = 0.f;
            for (int k = 0; k < 4; ++k) s += m[i][k] * o.m[k][j];
            r.m[i][j] = s;
        }
    return r;
}

Mat4 Mat4::inverse() const {
    // Gauss-Jordan com pivoteamento parcial sobre [A | I]
    double a[4][8];
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j) {
            a[i][j] = m[i][j];
            a[i][j + 4] = (i == j) ? 1.0 : 0.0;
        }

    for (int col = 0; col < 4; ++col) {
        int piv = col;
        for (int r = col + 1; r < 4; ++r)
            if (std::fabs(a[r][col]) > std::fabs(a[piv][col])) piv = r;
        if (std::fabs(a[piv][col]) < 1e-12)
            throw std::runtime_error("Mat4::inverse: matriz singular (escala 0?)");
        if (piv != col)
            for (int j = 0; j < 8; ++j) std::swap(a[piv][j], a[col][j]);

        double d = a[col][col];
        for (int j = 0; j < 8; ++j) a[col][j] /= d;

        for (int r = 0; r < 4; ++r) {
            if (r == col) continue;
            double f = a[r][col];
            if (f == 0.0) continue;
            for (int j = 0; j < 8; ++j) a[r][j] -= f * a[col][j];
        }
    }

    Mat4 inv;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            inv.m[i][j] = static_cast<float>(a[i][j + 4]);
    return inv;
}

Vector3 Mat4::point(const Vector3& v) const {
    return Vector3(
        m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3],
        m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3],
        m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3]);
}

Vector3 Mat4::dir(const Vector3& v) const {
    return Vector3(
        m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z,
        m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z,
        m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z);
}

Vector3 Mat4::dirTransposed(const Vector3& v) const {
    return Vector3(
        m[0][0] * v.x + m[1][0] * v.y + m[2][0] * v.z,
        m[0][1] * v.x + m[1][1] * v.y + m[2][1] * v.z,
        m[0][2] * v.x + m[1][2] * v.y + m[2][2] * v.z);
}
