#pragma once
#include <memory>
#include "Object.h"
#include "Mat4.h"

// Em vez de mover o objeto, leva o RAIO para o espaco local com M^-1:

//   origem_local  = M^-1 * origem            (ponto,   w = 1)
//   direcao_local = M^-1 * direcao           (direcao, w = 0)

// O objeto de dentro resolve a interseção como se nada tivesse mudado, e o resultado volta para o mundo

class TransformedObject : public Object {
    std::shared_ptr<Object> inner_;
    Mat4 M_;        // objeto -> mundo
    Mat4 Minv_;     // mundo  -> objeto

public:
    TransformedObject(std::shared_ptr<Object> inner, const Mat4& M);
    bool intersect(const Ray&, float, float, HitRecord&) const override;
};
#pragma once
