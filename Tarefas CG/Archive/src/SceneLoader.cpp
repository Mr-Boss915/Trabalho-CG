#include "SceneLoader.h"
#include "Sphere.h"
#include "Cylinder.h"
#include "Cone.h"
#include "Plane.h"
#include "Mesh.h"
#include "Texture.h"
#include "Mat4.h"
#include "TransformedObject.h"
#include <memory>
#include <fstream>
#include <stdexcept>

static Vector3 v3(std::istream& i) {
    float x, y, z;
    i >> x >> y >> z;
    return{ x,y,z };
}

static Color col(std::istream& i) {
    float r, g, b;
    i >> r >> g >> b;
    return{ r,g,b };
}

// ---------------------------------------------------------------------
// Leitura de tokens com 1 palavra de "lookahead" (sem seekg/tellg).
//
// tryAttachTexture precisa olhar o PROXIMO token pra decidir se e um
// bloco "texture" opcional ou o comando do proximo objeto. Tentar usar
// stream.tellg()/seekg() pra "devolver" um token nao lido e fragil (da
// problema em streams de texto, principalmente no Windows com \r\n) e
// foi a causa do bug anterior. A forma robusta e mais simples: guardar
// o token lido numa variavel-buffer e, na proxima vez que o loop
// principal pedir um token, devolver esse valor guardado em vez de ler
// de novo do arquivo.
// ---------------------------------------------------------------------
static std::string pendingToken;
static bool        hasPending = false;

static bool readToken(std::istream& in, std::string& out) {
    if (hasPending) {
        out = pendingToken;
        hasPending = false;
        return true;
    }
    return static_cast<bool>(in >> out);
}

static void pushBackToken(const std::string& tok) {
    pendingToken = tok;
    hasPending = true;
}

// Depois de montar o Material de um objeto, olha se o proximo token no
// arquivo e "texture" (caminho + escala). Se for, anexa a textura ao
// material. Se nao for, "devolve" o token (via pushBackToken, nao via
// seekg) para o loop principal processar normalmente - assim isso
// funciona como um bloco OPCIONAL, sem quebrar cenas que nao usam textura.
static void tryAttachTexture(std::istream& in, Material& m) {
    std::string tok;
    if (!readToken(in, tok)) return; // fim do arquivo, nada a fazer

    if (tok == "texture") {
        std::string path;
        float scale;
        in >> path >> scale;
        m.texture = std::make_shared<Texture>(Texture::load(path));
        m.textureScale = scale;
    }
    else {
        pushBackToken(tok); // nao era pra gente: devolve pro loop principal
    }
}

// ---------------------------------------------------------------------
// Transformacoes opcionais, escritas DEPOIS de um objeto (e da textura dele):
//
//   translate  tx ty tz
//   scale      sx sy sz
//   rotatex    graus
//   rotatey    graus
//   rotatez    graus
//   rotate     ax ay az graus        (eixo arbitrario, por mudanca de base)
//   rotateq    ax ay az graus        (eixo arbitrario, por quaternio)
//   shear      xy xz yx yz zx zy
//   mirror     px py pz  nx ny nz    (espelho no plano por p com normal n)
//   matrix     m00 m01 ... m33       (matriz 4x4 pronta, 16 valores por linha)
//   pivot      cx cy cz              (centro p/ scale/rotate*/shear seguintes)
//
// Sao aplicadas NA ORDEM em que aparecem (a primeira linha atua primeiro).
// Angulos em graus. 'pivot' vale ate o proximo 'pivot'; o padrao e a origem.
// Para girar/escalar um objeto em torno de um ponto c: M = T(c) * Op * T(-c).
// ---------------------------------------------------------------------
static std::shared_ptr<Object> applyTransforms(std::istream& in,
    std::shared_ptr<Object> obj) {
    Mat4 M;                       // acumulada: objeto -> mundo
    Vector3 pivot(0.f, 0.f, 0.f);
    bool any = false;

    auto aroundPivot = [&](const Mat4& op) {
        return Mat4::translate(pivot.x, pivot.y, pivot.z) * op *
            Mat4::translate(-pivot.x, -pivot.y, -pivot.z);
        };

    std::string tok;
    while (readToken(in, tok)) {
        if (!tok.empty() && tok[0] == '#') {      // comentario entre transformacoes
            std::string line;
            std::getline(in, line);
            continue;
        }

        Mat4 op;
        if (tok == "translate") {
            auto t = v3(in);
            op = Mat4::translate(t.x, t.y, t.z);
        }
        else if (tok == "scale") {
            auto s = v3(in);
            op = aroundPivot(Mat4::scale(s.x, s.y, s.z));
        }
        else if (tok == "rotatex") {
            float a; in >> a;
            op = aroundPivot(Mat4::rotateX(a));
        }
        else if (tok == "rotatey") {
            float a; in >> a;
            op = aroundPivot(Mat4::rotateY(a));
        }
        else if (tok == "rotatez") {
            float a; in >> a;
            op = aroundPivot(Mat4::rotateZ(a));
        }
        else if (tok == "rotate") {
            auto ax = v3(in);
            float a; in >> a;
            op = aroundPivot(Mat4::rotateAxis(ax, a));
        }
        else if (tok == "rotateq") {            // eixo arbitrario via quaternio
            auto ax = v3(in);
            float a; in >> a;
            op = aroundPivot(Mat4::rotateAxisQuat(ax, a));
        }
        else if (tok == "shear") {
            float v[6];
            for (float& x : v) in >> x;
            op = aroundPivot(Mat4::shear(v[0], v[1], v[2], v[3], v[4], v[5]));
        }
        else if (tok == "mirror") {
            auto p = v3(in);
            auto n = v3(in);
            op = Mat4::mirror(p, n);
        }
        else if (tok == "matrix") {              // 16 valores, linha a linha (gerado por 'export')
            for (int i = 0; i < 4; ++i)
                for (int j = 0; j < 4; ++j) in >> op.m[i][j];
        }
        else if (tok == "pivot") {
            pivot = v3(in);
            continue;
        }
        else {
            pushBackToken(tok);                    // proximo objeto/comando
            break;
        }
        M = op * M;                                // a nova atua DEPOIS das anteriores
        any = true;
    }

    if (!any) return obj;
    return std::make_shared<TransformedObject>(obj, M);
}

Scene SceneLoader::loadFromFile(const std::string& f) {
    std::ifstream in(f);
    if (!in) throw std::runtime_error("Nao abriu: " + f);
    Scene s;
    hasPending = false; // seguranca: garante estado limpo entre chamadas
    std::string t;
    while (readToken(in, t)) {
        if (t[0] == '#') {
            std::string line;
            std::getline(in, line);
            continue;
        }

        if (t == "camera") {
            auto  e = v3(in), l = v3(in), u = v3(in);
            float fov;
            in >> fov;
            s.camera = Camera(e, l, u, fov);
        }
        else if (t == "janela") {
            auto  j = v3(in);
            s.janela = Janela(j);
        }
        else if (t == "ambient")    s.ambientLight = col(in);
        else if (t == "background") s.background = col(in);
        else if (t == "light") {
            auto p = v3(in);
            auto c = col(in);
            s.lights.emplace_back(p, c);
        }
        else if (t == "sphere") {
            auto c = v3(in);
            float r;
            in >> r;
            auto ka = col(in), kd = col(in), ks = col(in);
            float n;
            in >> n;
            s.objects.push_back(applyTransforms(in,
                std::make_shared<Sphere>(c, r, Material(ka, kd, ks, n))));
        }
        else if (t == "plane") {
            auto Q = v3(in);
            auto u = v3(in);
            auto v = v3(in);
            auto ka = col(in), kd = col(in), ks = col(in);
            float n;
            in >> n;
            Material material(ka, kd, ks, n);
            // Opcoes do plano, em qualquer ordem: "bounded" (retangulo de lados u e v)
            // e "texture <arquivo> <escala>".
            bool bounded = false;
            std::string tok;
            while (readToken(in, tok)) {
                if (tok == "bounded") {
                    bounded = true;
                }
                else if (tok == "texture") {
                    std::string path; float scale;
                    in >> path >> scale;
                    material.texture = std::make_shared<Texture>(Texture::load(path));
                    material.textureScale = scale;
                }
                else {
                    pushBackToken(tok);
                    break;
                }
            }
            s.objects.push_back(applyTransforms(in,
                std::make_shared<Plane>(Q, u, v, material, bounded)));

        }
        else if (t == "cylinder") {
            auto B = v3(in);
            auto u = v3(in);
            float H, R;
            in >> H >> R;
            auto ka = col(in), kd = col(in), ks = col(in);
            float n;
            in >> n;
            Material material(ka, kd, ks, n);
            // Opcoes do cilindro, em qualquer ordem: "open" (tubo, sem tampas)
            // e "texture <arquivo> <escala>".
            bool capped = true;
            std::string tok;
            while (readToken(in, tok)) {
                if (tok == "open") {
                    capped = false;
                }
                else if (tok == "texture") {
                    std::string path; float scale;
                    in >> path >> scale;
                    material.texture = std::make_shared<Texture>(Texture::load(path));
                    material.textureScale = scale;
                }
                else {
                    pushBackToken(tok);
                    break;
                }
            }
            s.objects.push_back(applyTransforms(in,
                std::make_shared<Cylinder>(B, u, H, R, material, capped)));
        }
        else if (t == "cone") {
            auto V = v3(in);
            auto d = v3(in);
            float H, R;
            in >> H >> R;
            auto ka = col(in), kd = col(in), ks = col(in);
            float n;
            in >> n;
            Material material(ka, kd, ks, n);
            // Opcoes do cone, em qualquer ordem: "open" (sem o disco da base)
            // e "texture <arquivo> <escala>".
            bool capped = true;
            std::string tok;
            while (readToken(in, tok)) {
                if (tok == "open") {
                    capped = false;
                }
                else if (tok == "texture") {
                    std::string path; float scale;
                    in >> path >> scale;
                    material.texture = std::make_shared<Texture>(Texture::load(path));
                    material.textureScale = scale;
                }
                else {
                    pushBackToken(tok);
                    break;
                }
            }
            s.objects.push_back(applyTransforms(in,
                std::make_shared<Cone>(V, d, H, R, material, capped)));
        }
        else if (t == "mesh") {
            // Formato:
            //   mesh
            //   tetra | caminho/para/arquivo.obj
            //   ka  kd  ks  n
            std::string source;
            in >> source;
            auto ka = col(in), kd = col(in), ks = col(in);
            float n;
            in >> n;
            Material material(ka, kd, ks, n);
            tryAttachTexture(in, material);

            std::shared_ptr<Object> mesh;
            if (source == "tetra")
                mesh = std::make_shared<Mesh>(
                    Mesh::fromListMesh(ListMesh::MakeTetra(), material));
            else
                mesh = std::make_shared<Mesh>(Mesh::loadOBJ(source, material));
            s.objects.push_back(applyTransforms(in, mesh));
        }
        else throw std::runtime_error("Token desconhecido: " + t);
    }return s;
}
