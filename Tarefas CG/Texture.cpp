#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Texture.h"
#include <stdexcept>
#include <cmath>

Texture Texture::load(const std::string& path) {
    int w, h, channelsInFile;
    
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &channelsInFile, 3); ///-----------
    if (!data) {
        throw std::runtime_error("Textura: nao conseguiu carregar '" + path +
            "' (" + stbi_failure_reason() + ")");
    }

    Texture tex;
    tex.width_ = w;
    tex.height_ = h;
    tex.pixels_.assign(data, data + (size_t)w * h * 3);

    stbi_image_free(data); 
    return tex;
}

bool Texture::valid() const {
    return width_ > 0 && height_ > 0;
}

Color Texture::sample(float u, float v) const {
    if (!valid()) return Color(1.f, 0.f, 1.f);

    u = u - std::floor(u);
    v = v - std::floor(v);

    int px = (int)(u * width_);
    int py = (int)((1.f - v) * height_);

    px = std::min(std::max(px, 0), width_ - 1);
    py = std::min(std::max(py, 0), height_ - 1);

    size_t idx = ((size_t)py * width_ + px) * 3;
    float r = pixels_[idx + 0] / 255.f;
    float g = pixels_[idx + 1] / 255.f;
    float b = pixels_[idx + 2] / 255.f;
    return Color(r, g, b);
}
