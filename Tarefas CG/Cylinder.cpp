#include "Cylinder.h"
#include <cmath>

Cylinder::Cylinder(const Vector3& B, const Vector3& u, const float& H, const float& R,
    const Material& material, bool capped)
    : B_(B), u_(u.normalized()), H_(H), R_(R), material_(material), capped_(capped) {
}

bool Cylinder::intersect(const Ray& ray, float tMin, float tMax, HitRecord& hit) const {

    bool  hitOB = false;
    float closestT = tMax;

    // correcao da area interna do cilindro. previamente gerava glitchs quando tirava a tampa
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

    // |oc_perp + t d_perp|^2 = R^2
    Vector3 oc = ray.origin - B_;
    Vector3 dPerp = ray.direction - u_ * ray.direction.dot(u_);
    Vector3 ocPerp = oc - u_ * oc.dot(u_);

    float a = dPerp.dot(dPerp);
    float hb = ocPerp.dot(dPerp);
    float c = ocPerp.dot(ocPerp) - R_ * R_;
    float disc = hb * hb - a * c;

    if (disc >= 0.f && a > 1e-8f) {
        float s = std::sqrt(disc);
        float ts[2] = { (-hb - s) / a, (-hb + s) / a };     // entrada, saida

        for (float t : ts) {
            if (t < tMin || t > closestT) continue;
            Vector3 p = ray.at(t);
            float   h = (p - B_).dot(u_);
            if (h < 0.f || h > H_) continue;
            Vector3 radial = (p - B_) - u_ * h;
            accept(t, p, radial.normalized());
            break;
        }
    }

    // tampas
    float denom = ray.direction.dot(u_);
    if (capped_ && std::abs(denom) > 1e-8f) {
        Vector3 centers[2] = { B_, B_ + u_ * H_ };
        Vector3 normals[2] = { -u_, u_ };

        for (int k = 0; k < 2; ++k) {
            float t = (centers[k] - ray.origin).dot(u_) / denom;
            if (t < tMin || t > closestT) continue;

            Vector3 p = ray.at(t);
            Vector3 v = p - centers[k];
            Vector3 vPerp = v - u_ * v.dot(u_);
            if (vPerp.dot(vPerp) <= R_ * R_)
                accept(t, p, normals[k]);
        }
    }

    return hitOB;
}
