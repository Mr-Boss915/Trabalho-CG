#pragma once
#include <cstdint>
#include <vector>
#include "Scene.h"
#include "Framebuffer.h"

class Renderer {
    float eps_ = 1e-2f;  //diminuir o valor antes de f em caso os pontoos voltem de novo
    
    bool  closestHit(const Scene&, const Ray&, HitRecord&, int* index = nullptr)const;
    bool  isInShadow(const Scene&, const Vector3&, const Vector3&, const PointLight&)const;
    
    Color shade(const Scene&, const Ray&, const HitRecord&, bool verbose = false)const;
public:
    void  render(const Scene&, Framebuffer&)const;
    Color centerColor(const Scene&, bool verbose = true)const;

    static float windowDistance(const Scene&);

    Ray pixelRay(const Scene&, int x, int y, int W, int H, float dist)const;
    void renderTo(const Scene&, int W, int H, float dist, int step,
        std::vector<uint32_t>& argb, std::vector<int>& ids)const;

    // dispara o raio do pixel (x,y) e devolve o indice em s.objects
    int pickAt(const Scene&, int x, int y, int W, int H, float dist, Vector3* point = nullptr)const;

    // devolve o HitRecord
    int pickHit(const Scene&, int x, int y, int W, int H, float dist, HitRecord& hit, Ray* ray = nullptr)const;

    bool project(const Scene&, const Vector3& p, int W, int H, float dist, float& sx, float& sy)const;
};
