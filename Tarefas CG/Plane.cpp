#include "Plane.h"

#include <cmath>

Plane::Plane(const Vector3& Q, const Vector3& u, const Vector3& v, const Material& m, bool bounded)
    : Q_(Q), u_(u), v_(v), material_(m), bounded_(bounded) {



}

bool Plane::intersect(const Ray& ray, float tMin, float tMax, HitRecord& hit)const {
    // STUDENT TODO 1: intersecao raio-plano.

    /*
    Vector3 oc = ray.origin - center_;
    float   a = ray.direction.dot(ray.direction);
    float   hb = oc.dot(ray.direction);
    float   c = oc.dot(oc) - radius_ * radius_;
    float   disc = hb * hb - a * c;
    if (disc < 0)return false;
    float s = std::sqrt(disc);
    float t = (-hb - s) / a;
    if (t<tMin || t>tMax) {
        t = (-hb + s) / a;
        if (t<tMin || t>tMax) return false;
    }
    */
    Vector3 n = v_.cross(u_).normalized();
    double  D = n.dot(Q_);

    auto denom = n.dot(ray.direction);
    if (std::fabs(denom) < 1e-8) return false;

    float t = (D - n.dot(ray.origin)) / denom;
    if (t < tMin || t > tMax) return false;

    auto intersectionPoint = ray.at(t);
    Vector3 plan_hit = intersectionPoint - Q_;
    auto u_dot = plan_hit.dot(u_);
    auto v_dot = plan_hit.dot(v_);
    // Plano limitado
    if (bounded_) {
        float a = u_dot / u_.dot(u_);
        float b = v_dot / v_.dot(v_);
        if (a < 0.f || a > 1.f || b < 0.f || b > 1.f) return false;
        u_dot = a * u_.length();
        v_dot = b * v_.length();
    }

    hit.t = t;
    hit.point = ray.at(t);
    hit.normal = n;
    hit.u = static_cast<float>(u_dot) * material_.textureScale;
    hit.v = static_cast<float>(v_dot) * material_.textureScale;
    hit.material = &material_;
    hit.object = this;
    return true;
}
