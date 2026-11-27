#include "Renderer.h"
#include <limits>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <typeinfo>
#include <thread>
#include <algorithm>


bool Renderer::closestHit(const Scene& s, const Ray& r, HitRecord& hit, int* index)const {
    // STUDENT TODO 3: escolher a intersecao mais proxima.
    bool  found = false;
    float closest = std::numeric_limits<float>::infinity();

    for (size_t i = 0;i < s.objects.size();++i) {
        const auto& o = s.objects[i];
        HitRecord h;
        if (o->intersect(r, eps_, closest, h)) {
            found = true;
            closest = h.t;
            hit = h;
            if (index) *index = static_cast<int>(i);
        }
    }
    return found;
}

bool Renderer::isInShadow(const Scene& s, const Vector3& p, const Vector3& n, const PointLight& l)const {
    // STUDENT TODO 4: raio de sombra.
    Vector3 d = l.position() - p;
    float   dist = d.length();
    Ray     sr(p + n * eps_, d / dist);

    for (const auto& o : s.objects) {
        HitRecord h;
        if (o->intersect(sr, eps_, dist - eps_, h)) return true;
    }
    return false;
}

Color Renderer::shade(const Scene& s, const Ray& r, const HitRecord& h, bool verbose)const {
    // STUDENT TODO 5: modelo de Phong (ambiente + difusa + especular).
    const Material& m = *h.material;

	// Se o material tem uma textura valida substitui a cor difusa e ambiente pela amostra da textura - apenas para plaano
    bool    hasTex = (m.texture != nullptr) && m.texture->valid();
    Color   surface = hasTex ? m.texture->sample(h.u, h.v) : m.kd;
    Color   ambient = hasTex ? surface : m.ka;

	// usar "ambient"/"surface" devida cor vir da textura
    Color   ambPart = ambient * s.ambientLight;
    Color   out = ambPart;
    Vector3 N = h.normal.normalized();
    Vector3 V = (-r.direction).normalized();

    if (verbose) {
        std::cout << "  Ambiente  = (" << ambPart.r << ", " << ambPart.g << ", " << ambPart.b << ")\n";
    }

    for (const auto& l : s.lights) {
        bool shadow = isInShadow(s, h.point, N, l);
        if (verbose) {
            std::cout << "  Sombra    = " << (shadow ? "SIM (luz bloqueada)" : "NAO (luz chega ao ponto)") << "\n";
        }
        if (shadow) continue;
        Vector3 L = (l.position() - h.point).normalized();
        float   ndotl = std::max(0.f, N.dot(L));
        Color   diff = (surface * l.intensity()) * ndotl;
        Vector3 R = (2.f * N.dot(L) * N - L).normalized();
        float   rv = std::max(0.f, R.dot(V));
        Color   spec = (m.ks * l.intensity()) * std::pow(rv, m.shininess);
        out += diff + spec;
        if (verbose) {
            std::cout << "  L         = (" << L.x << ", " << L.y << ", " << L.z << ")\n";
            std::cout << "  N*L       = " << ndotl << "\n";
            std::cout << "  R*V       = " << rv << "   (n = " << m.shininess << ")\n";
            std::cout << "  Difusa    = (" << diff.r << ", " << diff.g << ", " << diff.b << ")\n";
            std::cout << "  Especular = (" << spec.r << ", " << spec.g << ", " << spec.b << ")\n";
        }
    }
    if (verbose) {
        std::cout << "  E = (" << out.r << ", " << out.g << ", " << out.b << ")\n";
    }
    return out.clamped();
}

Color Renderer::centerColor(const Scene& s, bool verbose)const {
    // Raio central: do olho para o lookAt. Como a camera olha para -W,
    // a direcao e exatamente -W (o mesmo que (lookAt - eye) normalizado).
    const Vector3 eye = s.camera.getEye();
    const Vector3 dir = (-s.camera.getW()).normalized();
    Ray r(eye, dir);

    HitRecord h;
    int idx = -1;
    bool hit = closestHit(s, r, h, &idx);

    if (verbose) {
        std::cout << std::fixed << std::setprecision(5);
        std::cout << "  PE        = (" << eye.x << ", " << eye.y << ", " << eye.z << ")\n";
        std::cout << "  d         = (" << dir.x << ", " << dir.y << ", " << dir.z << ")\n";
    }

    if (!hit) {
        if (verbose) std::cout << "  O raio nao acertou nenhum objeto: cor = fundo\n";
        return s.background;
    }

    if (verbose) {
        std::cout << "  Objeto    = #" << idx << " (" << typeid(*s.objects[idx]).name() << ")\n";
        std::cout << "  t         = " << h.t << "\n";
        std::cout << "  P         = (" << h.point.x << ", " << h.point.y << ", " << h.point.z << ")\n";
        Vector3 N = h.normal.normalized();
        std::cout << "  N         = (" << N.x << ", " << N.y << ", " << N.z << ")\n";
    }

    Color c = shade(s, r, h, verbose);

    if (verbose) {
        std::cout << "  COR       = (" << c.r << ", " << c.g << ", " << c.b << ")\n";
        std::cout << "  COR 8 bit = (" << int(c.r * 255.f + 0.5f) << ", " << int(c.g * 255.f + 0.5f) << ", " << int(c.b * 255.f + 0.5f) << ")\n";
        std::cout << "======================================\n\n";
    }
    return c;
}

void Renderer::render(const Scene& s, Framebuffer& fb)const {
    // STUDENT TODO 6: loop sobre pixels.
       // A janela agora acompanha a camera: ela fica a uma distancia 'dist' do
       // olho, na direcao do lookAt (-w), e seus eixos sao u (direita) e v (cima).
       //   ponto do pixel = eye + (-w)*dist + u*cx + v*cy
       // Para cameras antigas (olho em (0,0,0) olhando para -z) isso e igual ao
       // calculo anterior: cm = (cx, cy, janela.z).
    float wj = s.janela.getJanela().x;
    float hj = s.janela.getJanela().y;
    float delx = wj / fb.width();
    float dely = hj / fb.height();

    const Vector3 eye = s.camera.getEye();
    const Vector3 u = s.camera.getU();
    const Vector3 v = s.camera.getV();
    const Vector3 w = s.camera.getW();

    // distancia olho -> janela, medida em z (janela.z e a coordenada z do plano)
    float dist = std::abs(eye.z - s.janela.getJanela().z);
    if (dist < 1e-6f) dist = std::abs(s.janela.getJanela().z);
    const Vector3 center = eye + (-w) * dist;   // centro da janela no mundo

    std::cout << " H " << fb.height() << std::endl;
    std::cout << " W " << fb.width() << std::endl;

    for (int y = 0; y < fb.height();++y)
    {
        float cy = hj / 2 - dely / 2.f - dely * y;

        for (int x = 0;x < fb.width();++x) {
            float cx = -wj / 2.f + delx / 2.f + delx * x;
            Vector3 cm = center + u * cx + v * cy;
            Vector3 dr = (cm - eye).normalized();
            Ray r = Ray(eye, dr);

            HitRecord h;
            fb.setPixel(x, y, closestHit(s, r, h) ? shade(s, r, h) : s.background);
        }
    }
    centerColor(s, true);
}


// adaptacoes para o novo modelo de display
float Renderer::windowDistance(const Scene& s) {
    const Vector3 eye = s.camera.getEye();
    float dist = std::abs(eye.z - s.janela.getJanela().z);
    if (dist < 1e-6f) dist = std::abs(s.janela.getJanela().z);
    return dist;
}

Ray Renderer::pixelRay(const Scene& s, int x, int y, int W, int H, float dist)const {
    float wj = s.janela.getJanela().x;
    float hj = s.janela.getJanela().y;
    float delx = wj / W, dely = hj / H;
    float cx = -wj / 2.f + delx / 2.f + delx * x;
    float cy = hj / 2.f - dely / 2.f - dely * y;
    const Vector3 eye = s.camera.getEye();
    Vector3 cm = eye + (-s.camera.getW()) * dist + s.camera.getU() * cx + s.camera.getV() * cy;
    return Ray(eye, (cm - eye).normalized());
}

void Renderer::renderTo(const Scene& s, int W, int H, float dist, int step,
    std::vector<uint32_t>& argb, std::vector<int>& ids)const {
    if (step < 1) step = 1;
    argb.assign(size_t(W) * H, 0);
    ids.assign(size_t(W) * H, -1);

    auto to8 = [](float v) { return uint32_t(std::min(255.f, std::max(0.f, v * 255.f + 0.5f))); };

    const int blocksY = (H + step - 1) / step;
    unsigned nT = std::max(1u, std::thread::hardware_concurrency());
    auto work = [&](unsigned tid) {
        for (int by = int(tid); by < blocksY; by += int(nT)) {
            int y0 = by * step;
            for (int x0 = 0;x0 < W;x0 += step) {
                // raio no centro do bloco
                int cx = std::min(W - 1, x0 + step / 2), cy = std::min(H - 1, y0 + step / 2);
                Ray r = pixelRay(s, cx, cy, W, H, dist);
                HitRecord h; int idx = -1;
                Color c = closestHit(s, r, h, &idx) ? shade(s, r, h) : s.background;
                uint32_t px = 0xFF000000u | (to8(c.r) << 16) | (to8(c.g) << 8) | to8(c.b);
                for (int yy = y0; yy < std::min(H, y0 + step); ++yy)
                    for (int xx = x0; xx < std::min(W, x0 + step); ++xx) {
                        argb[size_t(yy) * W + xx] = px;
                        ids[size_t(yy) * W + xx] = idx;
                    }
            }
        }
        };
    std::vector<std::thread> th;
    for (unsigned t = 1;t < nT;++t) th.emplace_back(work, t);
    work(0);
    for (auto& t : th) t.join();
}

int Renderer::pickHit(const Scene& s, int x, int y, int W, int H, float dist, HitRecord& hit, Ray* ray)const {
    Ray r = pixelRay(s, x, y, W, H, dist);
    if (ray) *ray = r;
    int idx = -1;
    if (!closestHit(s, r, hit, &idx)) return -1;
    return idx;
}

int Renderer::pickAt(const Scene& s, int x, int y, int W, int H, float dist, Vector3* point)const {
    HitRecord h;
    int idx = pickHit(s, x, y, W, H, dist, h);
    if (idx >= 0 && point) *point = h.point;
    return idx;
}

bool Renderer::project(const Scene& s, const Vector3& p, int W, int H, float dist, float& sx, float& sy)const {
    const Vector3 eye = s.camera.getEye();
    Vector3 q = p - eye;
    float z = q.dot(-s.camera.getW());          // profundidade ao longo do olhar
    if (z < 1e-4f) return false;
    float cx = q.dot(s.camera.getU()) / z * dist;   // posicao na janela
    float cy = q.dot(s.camera.getV()) / z * dist;
    float wj = s.janela.getJanela().x, hj = s.janela.getJanela().y;
    sx = (cx / wj + .5f) * W;
    sy = (.5f - cy / hj) * H;
    return true;
}
