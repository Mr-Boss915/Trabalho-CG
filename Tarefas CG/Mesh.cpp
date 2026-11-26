#include "Mesh.h"
#include <cmath>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>

Mesh::Mesh(std::vector<Triangle> tris, const Material& material)
    : triangles_(std::move(tris)), material_(material) {
}

bool Mesh::intersect(const Ray& ray, float tMin, float tMax, HitRecord& hit) const {

    bool  hitAny = false;
    float closestT = tMax;

    for (const auto& tri : triangles_) {
        Vector3 e1 = tri.v1 - tri.v0;
        Vector3 e2 = tri.v2 - tri.v0;

        Vector3 pvec = ray.direction.cross(e2);
        float   det = e1.dot(pvec);
        if (std::abs(det) < 1e-8f) continue;          // raio paralelo ao triangulo

        float invDet = 1.f / det;
        Vector3 tvec = ray.origin - tri.v0;
        float u = tvec.dot(pvec) * invDet;
        if (u < 0.f || u > 1.f) continue;             // fora da aresta v0-v1

        Vector3 qvec = tvec.cross(e1);
        float v = ray.direction.dot(qvec) * invDet;
        if (v < 0.f || u + v > 1.f) continue;          // fora das outras arestas

        float t = e2.dot(qvec) * invDet;
        if (t < tMin || t > closestT) continue;

        closestT = t;
        hitAny = true;

        hit.t = t;
        hit.point = ray.at(t);
        hit.normal = e1.cross(e2).normalized();
        hit.material = &material_;
        hit.object = this;
    }
    return hitAny;
}

Mesh Mesh::loadOBJ(const std::string& path, const Material& material) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("Nao abriu: " + path);

    std::vector<Vector3>  verts;
    std::vector<Triangle> tris;
    std::string line;

    while (std::getline(in, line)) {
        std::istringstream ss(line);
        std::string tag;
        ss >> tag;

        if (tag == "v") {
            float x, y, z;
            ss >> x >> y >> z;
            verts.emplace_back(x, y, z);
        }
        else if (tag == "f") {
            int i, j, k;
            ss >> i >> j >> k;
            tris.push_back({ verts[i - 1], verts[j - 1], verts[k - 1] });
        }
    }
    return Mesh(std::move(tris), material);
}

Mesh Mesh::fromListMesh(const ListMesh& mesh, const Material& material) {
    std::vector<Triangle> tris;

    auto toVector3 = [](const Vector3& v) {
        return Vector3(v.rx(), v.ry(), v.rz());
        };

    for (const auto& face : mesh.faces()) {
        const auto& idx = face.verts();
        if (idx.size() < 3) continue;

        Vector3 v0 = toVector3(mesh.vertices()[idx[0]].pos());

        for (size_t i = 1; i + 1 < idx.size(); ++i) {
            Vector3 v1 = toVector3(mesh.vertices()[idx[i]].pos());
            Vector3 v2 = toVector3(mesh.vertices()[idx[i + 1]].pos());
            tris.push_back({ v0, v1, v2 });
        }
    }
    return Mesh(std::move(tris), material);
}
