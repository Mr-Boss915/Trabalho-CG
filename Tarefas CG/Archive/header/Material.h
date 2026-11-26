#pragma once
#include <memory>
#include "Color.h"
#include "Texture.h"

class Material{
public:
    Color ka,kd,ks;
    float shininess;

    std::shared_ptr<Texture> texture;
    float textureScale = 1.f;

    Material();
    Material(const Color&, const Color&, const Color&,float);
};
