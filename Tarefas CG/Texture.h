#pragma once
#include <vector>
#include <string>
#include "Color.h"

class Texture {
    int width_{ 0 }, height_{ 0 };
    std::vector<unsigned char> pixels_; 

public:
    Texture() = default;

    static Texture load(const std::string& path);

    Color sample(float u, float v) const;

    bool valid() const;
};
