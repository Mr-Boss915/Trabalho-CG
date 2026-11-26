#pragma once
#include "Ray.h"

class Camera {
    Vector3 eye_, lookAt_, up_, u_, v_, w_;
    float   fov_;
public:
    Camera();
    Camera(const Vector3&, const Vector3&, const Vector3&, float);
    void updateBasis();
    Vector3 getEye()const;
    Vector3 getLookAt()const { return lookAt_; }
    Vector3 getUp()const { return up_; }
    float   getFov()const { return fov_; }
    Vector3 getU()const;   // eixo 'direita' da camera
    Vector3 getV()const;   // eixo 'cima' da camera
    Vector3 getW()const;   // eixo 'para tras' (olho - lookAt); a camera olha para -W
    Ray generateRay(int, int, int, int)const;
};
