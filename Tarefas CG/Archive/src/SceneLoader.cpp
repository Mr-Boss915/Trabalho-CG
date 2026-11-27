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

static void tryAttachTexture(std::istream& in, Material& m) {
    std::string tok;
    if (!readToken(in, tok)) return;

    if (tok == "texture") {
        std::string path;
        float scale;
        in >> path >> scale;
        m.texture = std::make_shared<Texture>(Texture::load(path));
        m.textureScale = scale;
    }
    else {
        pushBackToken(tok); // devolve pro loop principal
    }
}


static std::shared_ptr<Object> applyTransforms(std::istream& in,
    std::shared_ptr<Object> obj) {
    Mat4 M;                      
    Vector3 pivot(0.f, 0.f, 0.f);
    bool any = false;

    auto aroundPivot = [&](const Mat4& op) {
        return Mat4::translate(pivot.x, pivot.y, pivot.z) * op *
            Mat4::translate(-pivot.x, -pivot.y, -pivot.z);
        };

    std::string tok;
    while (readToken(in, tok)) {
        if (!tok.empty() && tok[0] == '#') {      
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
        else if (tok == "rotateq") {            
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
        else if (tok == "matrix") {              
            for (int i = 0; i < 4; ++i)
                for (int j = 0; j < 4; ++j) in >> op.m[i][j];
        }
        else if (tok == "pivot") {
            pivot = v3(in);
            continue;
        }
        else {
            pushBackToken(tok);                    
            break;
        }
        M = op * M;                                
        any = true;
    }

    if (!any) return obj;
    return std::make_shared<TransformedObject>(obj, M);
}

Scene SceneLoader::loadFromFile(const std::string& f) {
    std::ifstream in(f);
    if (!in) throw std::runtime_error("Nao abriu: " + f);
    Scene s;
    hasPending = false; // seguranca: garante estado limpo entre chamadas **nao remover
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
