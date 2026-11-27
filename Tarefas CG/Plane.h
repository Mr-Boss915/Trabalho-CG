#pragma once
#include "Object.h"
#include "Vector3.h"

class Plane : public Object {
    Vector3 Q_;   //ponto incial
    Vector3 v_;  // lado a
    Vector3 u_;  // lado b
    Material material_;
    bool bounded_ = false; //infinito por default

public:

    Plane(const Vector3&, const Vector3&, const Vector3&, const Material&, bool bounded = false);
    bool intersect(const Ray&, float, float, HitRecord&)const override;
};
