#include "SceneEditor.h"
#include "TransformedObject.h"
#include <algorithm>
#include <cmath>

SceneEditor::SceneEditor(Scene& scene) : scene_(scene) { reset(); }

void SceneEditor::reset() {
    sel_ = -1;
    pivot_ = Vector3(0, 0, 0);
    base_ = scene_.objects;                       // o que foi carregado (ja com os comandos do .txt)
    M_.assign(base_.size(), Mat4());
    eye_ = scene_.camera.getEye();
    target_ = scene_.camera.getLookAt();
    up_ = scene_.camera.getUp();
    fov_ = scene_.camera.getFov();
}

// ------------------------------------------------------------ selecao
void SceneEditor::select(int index, const Vector3& pivot) {
    if (index < 0 || index >= int(base_.size())) { sel_ = -1; return; }
    sel_ = index;
    pivot_ = pivot;
}

void SceneEditor::selectNext() {
    if (base_.empty()) return;
    sel_ = (sel_ + 1) % int(base_.size());
}

// ------------------------------------------------------ transformacoes
Vector3 SceneEditor::axisVector() const {
    switch (axis) {
    case Axis::X: return Vector3(1, 0, 0);
    case Axis::Y: return Vector3(0, 1, 0);
    case Axis::Z: return Vector3(0, 0, 1);
    default:      return (-scene_.camera.getW()).normalized();   // "todos" = eixo do olhar
    }
}

void SceneEditor::rebuild(int i) {
    bool identity = true;
    for (int r = 0; r < 4 && identity; ++r)
        for (int c = 0; c < 4; ++c)
            if (std::fabs(M_[i].m[r][c] - (r == c ? 1.f : 0.f)) > 1e-7f) { identity = false; break; }
    scene_.objects[i] = identity ? base_[i]
        : std::make_shared<TransformedObject>(base_[i], M_[i]);
}

void SceneEditor::commit(const Mat4& op) {
    if (sel_ < 0) return;
    Mat4 next = op * M_[sel_];
    try {
        (void)next.inverse();                    // recusa matrizes singulares (escala ~0)
    }
    catch (...) { return; }
    M_[sel_] = next;
    rebuild(sel_);
}

void SceneEditor::apply(float a) {
    if (sel_ < 0) return;
    const Vector3 p = pivot_;
    auto around = [&](const Mat4& op) {          // T(p) * op * T(-p)
        return Mat4::translate(p.x, p.y, p.z) * op * Mat4::translate(-p.x, -p.y, -p.z);
        };

    switch (mode) {
    case Mode::Translate: {
        Vector3 d = axisVector() * a;
        commit(Mat4::translate(d.x, d.y, d.z));
        break;
    }
    case Mode::Rotate:                            // eixos x, y, z: matrizes de rotacao basicas
        if (axis == Axis::X) commit(around(Mat4::rotateX(a)));
        else if (axis == Axis::Y) commit(around(Mat4::rotateY(a)));
        else if (axis == Axis::Z) commit(around(Mat4::rotateZ(a)));
        else                      commit(around(Mat4::rotateAxisQuat(axisVector(), a)));
        break;
    case Mode::RotateView:                        // eixo arbitrario (olhar da camera) via quaternio
        commit(around(Mat4::rotateAxisQuat((-scene_.camera.getW()), a)));
        break;
    case Mode::Scale: {
        float f = std::max(0.05f, 1.f + a);
        float sx = (axis == Axis::X || axis == Axis::All) ? f : 1.f;
        float sy = (axis == Axis::Y || axis == Axis::All) ? f : 1.f;
        float sz = (axis == Axis::Z || axis == Axis::All) ? f : 1.f;
        commit(around(Mat4::scale(sx, sy, sz)));
        break;
    }
    case Mode::Shear:                             // X: x+=a*y   Y: y+=a*z   Z: z+=a*x
        if (axis == Axis::X) commit(around(Mat4::shear(a, 0, 0, 0, 0, 0)));
        else if (axis == Axis::Y) commit(around(Mat4::shear(0, 0, 0, a, 0, 0)));
        else                      commit(around(Mat4::shear(0, 0, 0, 0, a, 0)));
        break;
    }
}

void SceneEditor::applyMatrix(const Mat4& op, bool aroundPivot) {
    if (!aroundPivot) { commit(op); return; }
    commit(Mat4::translate(pivot_.x, pivot_.y, pivot_.z) * op *
        Mat4::translate(-pivot_.x, -pivot_.y, -pivot_.z));
}

void SceneEditor::translateScreen(float dx, float dy) {
    if (sel_ < 0) return;
    Vector3 d = scene_.camera.getU() * dx + scene_.camera.getV() * dy;
    commit(Mat4::translate(d.x, d.y, d.z));
}

void SceneEditor::mirror() {
    if (sel_ < 0) return;
    commit(Mat4::mirror(pivot_, axisVector()));
}

void SceneEditor::resetSelected() {
    if (sel_ < 0) return;
    M_[sel_] = Mat4();
    rebuild(sel_);
}

// -------------------------------------------------------------- camera
void SceneEditor::applyCamera() {
    scene_.camera = Camera(eye_, target_, up_, fov_);
}

void SceneEditor::orbit(float yawDeg, float pitchDeg) {
    Vector3 off = eye_ - target_;
    Vector3 up = up_.normalized();

    // yaw: em torno do "cima" do mundo
    off = Quaternion::fromAxisAngle(up, yawDeg).rotate(off);

    // pitch: em torno do eixo "direita" da camera; evita virar de cabeca para baixo
    Vector3 right = up.cross(off).normalized();
    Vector3 tilted = Quaternion::fromAxisAngle(right, pitchDeg).rotate(off);
    if (std::fabs(tilted.normalized().dot(up)) < 0.98f) off = tilted;

    eye_ = target_ + off;
    applyCamera();
}

void SceneEditor::dolly(float factor) {
    Vector3 off = (eye_ - target_) * factor;
    if (off.length() < minDistance) return;               // nao atravessa o alvo
    eye_ = target_ + off;
    applyCamera();
}

void SceneEditor::pan(float right, float up) {
    Vector3 d = scene_.camera.getU() * right + scene_.camera.getV() * up;
    eye_ = eye_ + d;
    target_ = target_ + d;
    applyCamera();
}

void SceneEditor::walk(float forward) {
    Vector3 d = (-scene_.camera.getW()) * forward;
    eye_ = eye_ + d;
    target_ = target_ + d;
    applyCamera();
}

std::string SceneEditor::status() const {
    static const char* modes[] = { "TRANSLACAO", "ROTACAO", "ROTACAO-olhar(quaternio)", "ESCALA", "CISALHAMENTO" };
    static const char* axes[] = { "X", "Y", "Z", "todos" };
    std::string s = "modo: ";
    s += modes[int(mode)];
    s += " | eixo: ";
    s += axes[int(axis)];
    s += " | selecionado: ";
    s += (sel_ < 0) ? std::string("nenhum") : "#" + std::to_string(sel_);
    return s;
}
