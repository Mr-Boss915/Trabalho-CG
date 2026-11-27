#include "Cone.h"
#include <cmath>
#include <utility>

Cone::Cone(const Vector3& V, const Vector3& d, const float& H, const float& R,
    const Material& material, bool capped)
    : V_(V + d.normalized() * H), d_(-d.normalized()), H_(H), R_(R),
    material_(material), capped_(capped) {
}

bool Cone::intersect(const Ray& ray, float tMin, float tMax, HitRecord& hit) const {

    bool  hitOB = false;
    float closestT = tMax;

    // Normal sempre virada para o lado de onde vem o raio (N . d < 0).
    auto accept = [&](float t, const Vector3& p, Vector3 n) {
        if (n.dot(ray.direction) > 0.f) n = -n;
        closestT = t;
        hitOB = true;
        hit.t = t;
        hit.point = p;
        hit.normal = n;
        hit.material = &material_;
        hit.object = this;
        };

   
    Vector3 oc = ray.origin - V_;
    float   ocPar = oc.dot(d_);
    float   dPar = ray.direction.dot(d_);
    Vector3 ocPerp = oc - d_ * ocPar;
    Vector3 rayPerp = ray.direction - d_ * dPar;

    float k = R_ / H_;
    float k2 = k * k;

    float a = rayPerp.dot(rayPerp) - k2 * dPar * dPar;
    float hb = ocPerp.dot(rayPerp) - k2 * ocPar * dPar;
    float c = ocPerp.dot(ocPerp) - k2 * ocPar * ocPar;
    float disc = hb * hb - a * c;

    if (disc >= 0.f && std::abs(a) > 1e-8f) {
        float s = std::sqrt(disc);
        float t1 = (-hb - s) / a;
        float t2 = (-hb + s) / a;
        if (t1 > t2) std::swap(t1, t2);    

        float ts[2] = { t1, t2 };
        for (float t : ts) { 
            if (t < tMin || t > closestT) continue;
            Vector3 p = ray.at(t);
            Vector3 v = p - V_;
            float   h = v.dot(d_);
            if (h < 0.f || h > H_) continue;    

            Vector3 radial = v - d_ * h;
            Vector3 normal = radial - d_ * (k2 * h);
            accept(t, p, normal.normalized());
            break;
        }
    }


    Vector3 baseCenter = V_ + d_ * H_;
    float   denom = ray.direction.dot(d_);

    if (capped_ && std::abs(denom) > 1e-8f) {
        float t = (baseCenter - ray.origin).dot(d_) / denom;
        if (t >= tMin && t <= closestT) {
            Vector3 p = ray.at(t);
            Vector3 v = p - baseCenter;
            Vector3 vPerp = v - d_ * v.dot(d_);
            if (vPerp.dot(vPerp) <= R_ * R_)
                accept(t, p, d_);
        }
    }

    return hitOB;
}