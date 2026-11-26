#pragma once
#include <string>
#include "Scene.h"
#include "Renderer.h"
#include "SceneEditor.h"

// Console de terminal do visualizador
class SceneConsole {
public:
    SceneConsole(Scene&, SceneEditor&, const Renderer&, int W, int H, float dist);

    // Chamado pelo viewer quando o clique acerta (idx>=0) ou erra (idx<0) um objeto.
    void onPick(int idx, const HitRecord& hit, const Ray& ray, int px, int py);

    //interpreta o console
    bool execute(const std::string& line, bool& quit);

    void setDist(float d) { dist_ = d; hasHit_ = false; }   // apos abrir outra cena
    void printSelected() const;     // matriz acumulada, pivot, comandos uteis
    void printCamera() const;
    static void printHelp();

private:
    Scene& scene_;
    SceneEditor& ed_;
    const Renderer& rend_;
    int   W_, H_;
    float dist_;

    bool        hasHit_ = false;     // ultimo picking valido
    HitRecord   lastHit_;
    int         lastIdx_ = -1, lastPx_ = 0, lastPy_ = 0;

    bool need(bool cond, const char* msg) const;
};
