#include "TransformedObject.h"
#include <cmath>
#include <utility>

TransformedObject::TransformedObject(std::shared_ptr<Object> inner, const Mat4& M)
    : inner_(std::move(inner)), M_(M), Minv_(M.inverse()) {
}

bool TransformedObject::intersect(const Ray& ray, float tMin, float tMax, HitRecord& hit) const {
    // 1) raio no espaco local do objeto
    Vector3 o = Minv_.point(ray.origin);
    Vector3 dl = Minv_.dir(ray.direction);

    // Com escala, |dl| != |direcao|. Normalizamos a direcao local (assim o
    // objeto de dentro recebe sempre um raio unitario, quer a classe Ray
    // normalize ou nao) e convertemos o parametro:
    //   P_local = o + t_local * (dl/len)  e  P_local = o + t * dl  =>  t_local = t * len
    float len = std::sqrt(dl.x * dl.x + dl.y * dl.y + dl.z * dl.z);
    if (len < 1e-12f) return false;
    Vector3 d(dl.x / len, dl.y / len, dl.z / len);
    Ray local(o, d);

    // 2) interseção local (o intervalo [tMin, tMax] tambem passa a ser local)
    HitRecord h;
    if (!inner_->intersect(local, tMin * len, tMax * len, h)) return false;

    // 3) resultado de volta no mundo
    hit = h;                                   // material, u, v, ...
    hit.t = h.t / len;
    hit.point = ray.at(hit.t);
    hit.normal = Minv_.dirTransposed(h.normal).normalized();
    hit.object = this;
    return true;
}
