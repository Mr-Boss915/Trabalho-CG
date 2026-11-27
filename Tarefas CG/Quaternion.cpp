#include "Quaternion.h"
#include <cmath>

Quaternion::Quaternion(float w_, float x_, float y_, float z_)
    : w(w_), x(x_), y(y_), z(z_) {
}

Quaternion Quaternion::fromAxisAngle(const Vector3& axis, float deg) {
    const float PI = 3.14159265358979323846f;
    Vector3 a = axis.normalized();
    float h = deg * PI / 180.f * 0.5f;      
    float s = std::sin(h);
    return Quaternion(std::cos(h), a.x * s, a.y * s, a.z * s);
}

Quaternion Quaternion::operator*(const Quaternion& o) const {
    return Quaternion(
        w * o.w - x * o.x - y * o.y - z * o.z,
        w * o.x + x * o.w + y * o.z - z * o.y,
        w * o.y - x * o.z + y * o.w + z * o.x,
        w * o.z + x * o.y - y * o.x + z * o.w);
}

Quaternion Quaternion::conjugate() const { return Quaternion(w, -x, -y, -z); }

Quaternion Quaternion::normalized() const {
    float n = std::sqrt(w * w + x * x + y * y + z * z);
    return Quaternion(w / n, x / n, y / n, z / n);
}

Vector3 Quaternion::rotate(const Vector3& p) const {
    Vector3 qv(x, y, z);
    Vector3 t = qv.cross(p);  t = Vector3(2 * t.x, 2 * t.y, 2 * t.z);
    Vector3 c = qv.cross(t);
    return Vector3(p.x + w * t.x + c.x, p.y + w * t.y + c.y, p.z + w * t.z + c.z);
}
