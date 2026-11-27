#pragma once
#include <memory>
#include <string>
#include <vector>
#include "Scene.h"
#include "Mat4.h"
#include "Quaternion.h"

// Logica do editor interativo
// Cada objeto da cena e guardado como  base  +  matriz acumulada M. Quando M muda, scene.objects[i] vira TransformedObject(base, M)
class SceneEditor {
public:
    enum class Mode { Translate, Rotate, RotateView, Scale, Shear };
    enum class Axis { X, Y, Z, All };

    explicit SceneEditor(Scene& scene);
    void reset();

    // Selecao
    float minDistance = 5.f;   // dolly nao chega mais perto do alvo que isso (escala com a cena)
    int  selected() const { return sel_; }
    void select(int index, const Vector3& pivot);   // pivot = ponto clicado
    void deselect() { sel_ = -1; }
    void selectNext();
    const Vector3& pivot() const { return pivot_; }

    // Consulta
    int  count() const { return int(base_.size()); }
    const Mat4& matrix(int i) const { return M_[i]; }          // matriz acumulada do objeto i
    const std::shared_ptr<Object>& baseObject(int i) const { return base_[i]; }
    void setPivot(const Vector3& p) { pivot_ = p; }
    // M = op * M no objeto selecionado; 'aroundPivot' faz T(p) * op * T(-p).
    void applyMatrix(const Mat4& op, bool aroundPivot);
    Vector3 eye() const { return eye_; }
    Vector3 target() const { return target_; }
    void setEye(const Vector3& e) { eye_ = e;    applyCamera(); }
    void setTarget(const Vector3& t) { target_ = t; applyCamera(); }

	// Transformacoes
    Mode mode = Mode::Translate;
    Axis axis = Axis::All;

    // amount = unidade do modo
    void apply(float amount);
    void translateScreen(float dxRight, float dyUp);
    void mirror();
    void resetSelected();    // volta o objeto ao estado carregado

    // Camera
    void orbit(float yawDeg, float pitchDeg);   // gira o olho em torno do alvo
    void dolly(float factor);                   // factor < 1 aproxima, > 1 afasta
    void pan(float right, float up);            // desloca olho E alvo no plano da tela
    void walk(float forward);                   // desloca olho E alvo ao longo do olhar

    std::string status() const;                 // texto para a barra de titulo

private:
    Scene& scene_;
    std::vector<std::shared_ptr<Object>> base_;
    std::vector<Mat4>                    M_;
    int                                  sel_ = -1;
    Vector3                              pivot_{ 0,0,0 };

    Vector3 eye_, target_, up_;
    float   fov_;

    void rebuild(int i);
    void commit(const Mat4& op);                // M_[sel] = op * M_[sel]
    void applyCamera();
    Vector3 axisVector() const;
};
