#include "Cone.h"
#include <cmath>

Cone::Cone(const Vector3& V, const Vector3& d, const float& H, const float& R, const Material& material)
    : V_(V + d.normalized() * H), d_(-d.normalized()), H_(H), R_(R), material_(material) {}

bool Cone::intersect(const Ray& ray, float tMin, float tMax, HitRecord& hit) const {

    bool hitOB = false;
    float closestT = tMax;

    Vector3 oc = ray.origin - V_;
    float ocPar = oc.dot(d_);
    float dPar = ray.direction.dot(d_);
    Vector3 ocPerp = oc - d_ * ocPar;                     
    Vector3 rayPerp = ray.direction - d_ * dPar;           

    float k = R_ / H_;
    float k2 = k * k;

    float a = rayPerp.dot(rayPerp) - k2 * dPar * dPar;
    float hb = ocPerp.dot(rayPerp) - k2 * ocPar * dPar;
    float c = ocPerp.dot(ocPerp) - k2 * ocPar * ocPar;
    float disc = hb * hb - a * c;

    if (disc >= 0 && std::abs(a) > 1e-8) {

        float s = std::sqrt(disc);
        float t1 = (-hb - s) / a;
        float t2 = (-hb + s) / a;
        
        float t = t1;

        if (t < tMin || t > closestT) t = t2;
        if (t >= tMin && t <= closestT) {

            Vector3 p = ray.at(t);
            float y = (p - V_).dot(d_);

            if (y >= 0 && y <= H_) {

                closestT = t;
                hitOB = true;

                hit.t = t;
                hit.point = p;
                Vector3 v = p - V_;
                float proj = v.dot(d_);
                Vector3 radial = v - d_ * proj;
                Vector3 normal = radial - d_ * (k2 * proj);
                hit.normal = normal.normalized();
                hit.material = &material_;
                hit.object = this;
            }
        }
    }

 
    // fundo

    Vector3 baseCenter = V_ + d_ * H_;
    float denom = ray.direction.dot(d_);

    if (std::abs(denom) > 1e-8) {

        float t = (baseCenter - ray.origin).dot(d_) / denom;

        if (t >= tMin && t <= closestT) {

            Vector3 p = ray.at(t);
            Vector3 v = p - baseCenter;
            Vector3 vPerp = v - d_ * v.dot(d_);
            float dist2 = vPerp.dot(vPerp);

            if (dist2 <= R_ * R_) {

                closestT = t;
                hitOB = true;
                hit.t = t;
                hit.point = p;
                hit.normal = d_;
                hit.material = &material_;
                hit.object = this;

            }
        }
    }

    /*tampa

    Vector3 topCenter = B_ + u_ * H_;

    if (std::abs(denom) > 1e-8) {

        float t = (topCenter - ray.origin).dot(u_) / denom;

        if (t >= tMin && t <= closestT) {

            Vector3 p = ray.at(t);
            Vector3 v = p - topCenter;
            Vector3 vPerp = v - u_ * v.dot(u_);
            float dist2 = vPerp.dot(vPerp);

            if (dist2 <= R_ * R_) {

                closestT = t;
                hitOB = true;
                hit.t = t;
                hit.point = p;
                hit.normal = u_;
                hit.material = &material_;
                hit.object = this;

            }
        }
    }
    */

    return hitOB;
}