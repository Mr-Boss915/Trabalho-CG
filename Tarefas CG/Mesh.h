#pragma once
#include <vector>
#include <string>
#include "Object.h"
#include "Vector3.h"
#include "ListMesh.h"

struct Triangle {
    Vector3 v0, v1, v2;
};

class Mesh : public Object {
    std::vector<Triangle> triangles_;
    Material material_;

public:
    Mesh(std::vector<Triangle>, const Material&);

    static Mesh loadOBJ(const std::string&, const Material&);
    static Mesh fromListMesh(const ListMesh&, const Material&);

    bool intersect(const Ray&, float, float, HitRecord&) const override;
};
