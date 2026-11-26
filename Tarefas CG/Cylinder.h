#pragma once
#include "Object.h"
#include "Vector3.h"

class Cylinder : public Object {
    Vector3 B_;   //ponto incial
    Vector3 u_;   //Direcao
    float   H_;
	float   R_;   // raio
    Material material_;

public:
    Cylinder(const Vector3&, const Vector3&, const float&, const float&, const Material&);
    bool intersect(const Ray&, float, float, HitRecord&)const override;
};