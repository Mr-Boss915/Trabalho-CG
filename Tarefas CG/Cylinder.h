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
    bool capped_;
    Cylinder(const Vector3& B, const Vector3& u, const float& H, const float& R, const Material& material, bool capped = true);
    bool intersect(const Ray&, float, float, HitRecord&)const override;
};