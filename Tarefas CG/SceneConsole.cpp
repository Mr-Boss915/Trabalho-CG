#include "SceneConsole.h"
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <typeinfo>
#ifdef __GNUG__
#include <cxxabi.h>
#include <cstdlib>
#endif

namespace {

    std::string typeName(const Object& o) {
        std::string n = typeid(o).name();
#ifdef __GNUG__
        int st = 0;
        char* d = abi::__cxa_demangle(n.c_str(), nullptr, nullptr, &st);
        if (st == 0 && d) n = d;
        std::free(d);
#endif
        for (const char* pre : { "class ", "struct " })
            if (n.rfind(pre, 0) == 0) n = n.substr(std::string(pre).size());
        return n;
    }

    std::string vec(const Vector3& v) {
        std::ostringstream o;
        o << std::fixed << std::setprecision(3) << "(" << v.x << ", " << v.y << ", " << v.z << ")";
        return o.str();
    }

    std::string col(const Color& c) {
        std::ostringstream o;
        o << std::fixed << std::setprecision(3) << "(" << c.r << ", " << c.g << ", " << c.b << ")";
        return o.str();
    }

    // Nome do objeto interno
    std::string kindOf(const SceneEditor& ed, int i) { return typeName(*ed.baseObject(i)); }

    bool isIdentity(const Mat4& M) {
        for (int r = 0; r < 4; ++r)
            for (int c = 0; c < 4; ++c)
                if (std::fabs(M.m[r][c] - (r == c ? 1.f : 0.f)) > 1e-6f) return false;
        return true;
    }

} // namespace

SceneConsole::SceneConsole(Scene& s, SceneEditor& e, const Renderer& r, int W, int H, float dist)
    : scene_(s), ed_(e), rend_(r), W_(W), H_(H), dist_(dist) {
}

bool SceneConsole::need(bool cond, const char* msg) const {
    if (!cond) std::cout << "  ! " << msg << "\n";
    return cond;
}

// --------------------------------------------------------------- picking
void SceneConsole::onPick(int idx, const HitRecord& h, const Ray& ray, int px, int py) {
    std::cout << "\nPICKING pixel (" << px << ", " << py << ")\n";
    if (idx < 0) {
        hasHit_ = false;
        std::cout << "  nenhum objeto - selecao removida.\n";
        return;
    }
    hasHit_ = true; lastHit_ = h; lastIdx_ = idx; lastPx_ = px; lastPy_ = py;

    std::cout << std::fixed << std::setprecision(3);
    std::cout << "  Objeto     : #" << idx << "  (" << kindOf(ed_, idx) << ")\n";
    std::cout << "  Raio       : origem " << vec(ray.origin) << "  direcao " << vec(ray.direction) << "\n";
    std::cout << "  t          : " << h.t << "\n";
    std::cout << "  Ponto P    : " << vec(h.point) << "\n";
    std::cout << "  Normal N   : " << vec(h.normal) << "\n";
    if (h.material) {
        const Material& m = *h.material;
        std::cout << "  Material   : ka " << col(m.ka) << "  kd " << col(m.kd)
            << "  ks " << col(m.ks) << "  n " << m.shininess << "\n";
        if (m.texture) std::cout << "  Textura    : sim (escala " << m.textureScale
            << ")  u,v = (" << h.u << ", " << h.v << ")\n";
    }
    printSelected();
}

void SceneConsole::printSelected() const {
    int i = ed_.selected();
    if (i < 0) { std::cout << "  (nenhum objeto selecionado)\n"; return; }
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "  Selecionado: #" << i << " (" << kindOf(ed_, i) << ")  pivot " << vec(ed_.pivot()) << "\n";
    const Mat4& M = ed_.matrix(i);
    if (isIdentity(M)) {
        std::cout << "  Matriz acumulada: identidade\n";
    }
    else {
        std::cout << "  Matriz acumulada (objeto -> mundo):\n";
        for (int r = 0; r < 4; ++r)
            std::cout << "    [" << std::setw(9) << M.m[r][0] << " " << std::setw(9) << M.m[r][1] << " "
            << std::setw(9) << M.m[r][2] << " " << std::setw(9) << M.m[r][3] << " ]\n";
    }
    std::cout << "  Comandos: translate | scale | rotatex/y/z | rotate | rotateq | shear | mirror | reset | export | ajuda\n";
}

void SceneConsole::printCamera() const {
    const Camera& c = scene_.camera;
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "\nCamera:\nolho " << vec(c.getEye()) << "  \nalvo " << vec(c.getLookAt())
        << "  \nup " << vec(c.getUp()) << "  \nfov " << c.getFov() << "\n";
    std::cout << "          \nU(direita) " << vec(c.getU()) << "  \nV(cima) " << vec(c.getV())
        << "  \nW(tras) " << vec(c.getW()) << "\n";
}

void SceneConsole::printHelp() {
    std::cout <<
        R"(
COMANDOS (angulos em graus; transformacoes atuam no objeto SELECIONADO)
  Objetos
    list                       lista os objetos da cena
    select N                   seleciona o objeto #N (pivot = ultimo clique nele, se houver)
    pick X Y                   picking no pixel (X,Y) da janela
    info                       mostra o objeto selecionado e a matriz acumulada
    pivot X Y Z                define o pivot (centro de scale/rotate/shear)
    export                     imprime a linha 'matrix ...' para colar no .txt, depois do objeto
    reset                      desfaz as transformacoes do viewer no objeto
  Transformacoes (ordem: a nova atua depois das anteriores)
    translate TX TY TZ
    scale SX SY SZ             em torno do pivot
    rotatex|rotatey|rotatez A  em torno do pivot
    rotate AX AY AZ A          eixo arbitrario (mudanca de base)
    rotateq AX AY AZ A         eixo arbitrario (quaternio)
    shear XY XZ YX YZ ZX ZY
    mirror NX NY NZ            espelho no plano que passa pelo pivot
  Camera
    cam                        mostra a camera
    cam eye X Y Z | cam target X Y Z
    cam orbit YAW PITCH        gira em torno do alvo
    cam dolly F                F<1 aproxima, F>1 afasta
    cam pan DIREITA CIMA       cam walk D (frente/tras)
  Cena
    abrir CAMINHO.txt          abre outra cena (ou arraste o .txt para a janela)
    recarregar                 recarrega a cena atual (descarta as edicoes)
  Outros: ajuda, sair
)";
}

// ------------------------------------------------------------- comandos
bool SceneConsole::execute(const std::string& line, bool& quit) {
    std::istringstream in(line);
    std::string cmd;
    if (!(in >> cmd)) return false;

    auto num = [&](float& v) { return static_cast<bool>(in >> v); };
    auto vec3 = [&](Vector3& v) { return static_cast<bool>(in >> v.x >> v.y >> v.z); };
    auto hasSel = [&] { return need(ed_.selected() >= 0, "nenhum objeto selecionado (clique ou 'select N')"); };
    auto bad = [&] { std::cout << "  ! argumentos invalidos (digite 'ajuda')\n"; return false; };

    if (cmd == "ajuda" || cmd == "help" || cmd == "?") { printHelp(); return false; }
    if (cmd == "sair" || cmd == "quit" || cmd == "exit") { quit = true; return false; }

    if (cmd == "list") {
        for (int i = 0; i < ed_.count(); ++i)
            std::cout << (i == ed_.selected() ? "  * #" : "    #") << i << "  " << kindOf(ed_, i)
            << (isIdentity(ed_.matrix(i)) ? "" : "  [transformado]") << "\n";
        return false;
    }
    if (cmd == "info") { if (hasSel()) printSelected(); return false; }

    if (cmd == "select") {
        int i;
        if (!(in >> i) || i < 0 || i >= ed_.count()) { std::cout << "  ! indice invalido\n"; return false; }
        Vector3 p = ed_.pivot();
        if (hasHit_ && lastIdx_ == i) p = lastHit_.point;      // reaproveita o ponto do ultimo clique
        ed_.select(i, p);
        printSelected();
        return true;
    }
    if (cmd == "pick") {
        int x, y;
        if (!(in >> x >> y)) return bad();
        HitRecord h; Ray r(Vector3(0, 0, 0), Vector3(0, 0, -1));
        int idx = rend_.pickHit(scene_, x, y, W_, H_, dist_, h, &r);
        if (idx >= 0) ed_.select(idx, h.point); else ed_.deselect();
        onPick(idx, h, r, x, y);
        return true;
    }
    if (cmd == "pivot") {
        Vector3 p;
        if (!vec3(p)) return bad();
        ed_.setPivot(p);
        std::cout << "  pivot = " << vec(p) << "\n";
        return true;
    }
    if (cmd == "reset") { if (!hasSel()) return false; ed_.resetSelected(); printSelected(); return true; }
    if (cmd == "export") {
        if (!hasSel()) return false;
        const Mat4& M = ed_.matrix(ed_.selected());
        std::cout << std::setprecision(7) << "  # cole abaixo do objeto #" << ed_.selected() << " no .txt\n  matrix";
        for (int r = 0; r < 4; ++r)
            for (int c = 0; c < 4; ++c) std::cout << " " << M.m[r][c];
        std::cout << "\n";
        return false;
    }

    // ---- transformacoes ----
    if (cmd == "translate") {
        Vector3 t; if (!vec3(t)) return bad(); if (!hasSel()) return false;
        ed_.applyMatrix(Mat4::translate(t.x, t.y, t.z), false);
    }
    else if (cmd == "scale") {
        Vector3 s; if (!vec3(s)) return bad(); if (!hasSel()) return false;
        ed_.applyMatrix(Mat4::scale(s.x, s.y, s.z), true);
    }
    else if (cmd == "rotatex" || cmd == "rotatey" || cmd == "rotatez") {
        float a; if (!num(a)) return bad(); if (!hasSel()) return false;
        Mat4 R = cmd == "rotatex" ? Mat4::rotateX(a) : cmd == "rotatey" ? Mat4::rotateY(a) : Mat4::rotateZ(a);
        ed_.applyMatrix(R, true);
    }
    else if (cmd == "rotate" || cmd == "rotateq") {
        Vector3 ax; float a;
        if (!vec3(ax) || !num(a)) return bad();
        if (ax.length() < 1e-6f) { std::cout << "  ! eixo nulo\n"; return false; }
        if (!hasSel()) return false;
        ed_.applyMatrix(cmd == "rotate" ? Mat4::rotateAxis(ax, a) : Mat4::rotateAxisQuat(ax, a), true);
    }
    else if (cmd == "shear") {
        float v[6];
        for (float& x : v) if (!num(x)) return bad();
        if (!hasSel()) return false;
        ed_.applyMatrix(Mat4::shear(v[0], v[1], v[2], v[3], v[4], v[5]), true);
    }
    else if (cmd == "mirror") {
        Vector3 n; if (!vec3(n)) return bad();
        if (n.length() < 1e-6f) { std::cout << "  ! normal nula\n"; return false; }
        if (!hasSel()) return false;
        ed_.applyMatrix(Mat4::mirror(ed_.pivot(), n), false);
    }
    // ---- camera ----
    else if (cmd == "cam") {
        std::string sub;
        if (!(in >> sub)) { printCamera(); return false; }
        Vector3 v; float a, b;
        if (sub == "eye") { if (!vec3(v)) return bad(); ed_.setEye(v); }
        else if (sub == "target") { if (!vec3(v)) return bad(); ed_.setTarget(v); }
        else if (sub == "orbit") { if (!num(a) || !num(b)) return bad(); ed_.orbit(a, b); }
        else if (sub == "dolly") { if (!num(a) || a <= 0) return bad(); ed_.dolly(a); }
        else if (sub == "pan") { if (!num(a) || !num(b)) return bad(); ed_.pan(a, b); }
        else if (sub == "walk") { if (!num(a)) return bad(); ed_.walk(a); }
        else return bad();
        printCamera();
        return true;
    }
    else {
        std::cout << "  ! comando desconhecido: " << cmd << " (digite 'ajuda')\n";
        return false;
    }

    printSelected();
    return true;
}
