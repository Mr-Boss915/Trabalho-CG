#pragma once
#include "Object.h"
#include "Vector3.h"

class Cone : public Object {
    Vector3 V_;       // vértice do cone
    Vector3 d_;       // direção/eixo do cone
    float   H_;       // altura
    float   R_;       // raio da base
    Material material_;

public:
    Cone(const Vector3&, const Vector3&, const float&, const float&, const Material&);
    bool intersect(const Ray&, float, float, HitRecord&) const override;
};