#define _CRT_SECURE_NO_WARNINGS

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include "SceneLoader.h"
#include "Renderer.h"
#include "SceneEditor.h"
#include "SceneConsole.h"


namespace fs = std::filesystem;

static fs::path cenaPadrao() {
    fs::path sourceFile(__FILE__);
    fs::path projectDirectory = sourceFile.parent_path();
    return projectDirectory / "Archive" / "scenes" / "Testes.txt";
}

static void savePPM(const char* path, const std::vector<uint32_t>& px, int W, int H) {
    FILE* f = std::fopen(path, "wb");
    if (!f) return;
    std::fprintf(f, "P6\n%d %d\n255\n", W, H);
    for (uint32_t p : px) {
        unsigned char rgb[3] = { (unsigned char)(p >> 16), (unsigned char)(p >> 8), (unsigned char)p };
        std::fwrite(rgb, 1, 3, f);
    }
    std::fclose(f);
}

int main(int argc, char** argv) {
    std::string scenePath = (argc > 1) ? std::string(argv[1]) : cenaPadrao().string();

    Scene scene;
    try { scene = SceneLoader::loadFromFile(scenePath); }
    catch (const std::exception& e) { std::cerr << "Erro: " << e.what() << "\n"; return 1; }

    Renderer renderer;
    float dist = Renderer::windowDistance(scene);
    float unit = dist / 200.f;
    const int   W = (argc > 2) ? std::max(200, std::atoi(argv[2])) : 800;
    const float wj = scene.janela.getJanela().x, hj = scene.janela.getJanela().y;
    const int   H = std::max(100, int(W * hj / wj + 0.5f));

    SceneEditor editor(scene);
    editor.minDistance = 5.f * unit;
    SceneConsole console(scene, editor, renderer, W, H, dist);

    auto openScene = [&](const std::string& path) {
        try {
            Scene ns = SceneLoader::loadFromFile(path);
            scene = ns;
            scenePath = path;
            dist = Renderer::windowDistance(scene);
            unit = dist / 200.f;
            editor.minDistance = 5.f * unit;
            editor.reset();
            console.setDist(dist);
            std::cout << "Cena aberta: " << path << " (" << editor.count() << " objetos)\n";
            console.printCamera();
            return true;
        }
        catch (const std::exception& ex) {
            std::cerr << "Erro ao abrir '" << path << "': " << ex.what() << "\n";
            return false;
        }
        };

    // Leitura do terminal em outra thread (getline bloqueia); as linhas entram numa fila e sao executadas no laco principal, onde mexer na cena e seguro.
    std::mutex qMutex;
    std::queue<std::string> cmdQueue;
    std::thread([&] {
        std::string line;
        while (std::getline(std::cin, line)) {
            std::lock_guard<std::mutex> lk(qMutex);
            cmdQueue.push(line);
        }
        }).detach();
    std::cout << "Clique num objeto para ver as informacoes aqui. Digite 'ajuda' para os comandos.\n";
    console.printCamera();

    if (!SDL_Init(SDL_INIT_VIDEO))   /* SDL3: retorna true em sucesso */ { std::cerr << SDL_GetError() << "\n"; return 1; }
    SDL_Window* win = SDL_CreateWindow("Ray Tracer", W, H, 0);
    SDL_Renderer* ren = SDL_CreateRenderer(win, nullptr);
    SDL_Texture* tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, W, H);

    std::vector<uint32_t> img;
    std::vector<int>      ids;
    bool     dirty = true, lowRes = false, running = true;
    Uint64   lastInput = 0;
    bool     lDown = false, rDown = false, mDown = false;
    bool     objMoved = false, camMoved = false;

    auto setTitle = [&] { SDL_SetWindowTitle(win, ("Ray Tracer - " + editor.status()).c_str()); };

    auto draw = [&](int step) {
        renderer.renderTo(scene, W, H, dist, step, img, ids);

        // contorno amarelo do objeto selecionado
        int s = editor.selected();
        if (s >= 0) {
            auto id = [&](int x, int y) { return (x < 0 || y < 0 || x >= W || y >= H) ? -2 : ids[size_t(y) * W + x]; };
            for (int y = 0; y < H; ++y)
                for (int x = 0; x < W; ++x)
                    if (id(x, y) == s && (id(x - 1, y) != s || id(x + 1, y) != s || id(x, y - 1) != s || id(x, y + 1) != s))
                        img[size_t(y) * W + x] = 0xFFFFE000u;

            // cruz vermelha no pivot
            float px, py;
            if (renderer.project(scene, editor.pivot(), W, H, dist, px, py))
                for (int k = -6; k <= 6; ++k) {
                    int xs[2] = { int(px) + k, int(px) }, ys[2] = { int(py), int(py) + k };
                    for (int j = 0; j < 2; ++j)
                        if (xs[j] >= 0 && xs[j] < W && ys[j] >= 0 && ys[j] < H)
                            img[size_t(ys[j]) * W + xs[j]] = 0xFFFF2020u;
                }
        }
        SDL_UpdateTexture(tex, nullptr, img.data(), W * 4);
        SDL_RenderTexture(ren, tex, nullptr, nullptr);
        SDL_RenderPresent(ren);
        };

    // sensibilidade
    const float kCamStep = 10.f;
    const float kMove = 1.0f, kRot = 0.5f, kScale = 0.005f, kShear = 0.005f;
    const float sMove = 10.f, sRot = 5.f, sScale = 0.05f, sShear = 0.05f;

    auto dragAmount = [&](float dx) {
        switch (editor.mode) {
        case SceneEditor::Mode::Translate: return dx * kMove * unit;
        case SceneEditor::Mode::Scale:     return dx * kScale;
        case SceneEditor::Mode::Shear:     return dx * kShear;
        default:                           return dx * kRot;
        }
        };
    auto keyAmount = [&](int sign) {
        switch (editor.mode) {
        case SceneEditor::Mode::Translate: return sign * sMove * unit;
        case SceneEditor::Mode::Scale:     return sign * sScale;
        case SceneEditor::Mode::Shear:     return sign * sShear;
        default:                           return sign * sRot;
        }
        };

    setTitle();
    while (running) {
        SDL_Event e;
        bool input = false;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_EVENT_QUIT: running = false; break;

            case SDL_EVENT_DROP_FILE:
                if (e.drop.data && openScene(e.drop.data)) { input = true; setTitle(); }
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (e.button.button == SDL_BUTTON_LEFT) {
                    lDown = true;
                    HitRecord hit; Ray ray(Vector3(0, 0, 0), Vector3(0, 0, -1));
                    int idx = renderer.pickHit(scene, int(e.button.x), int(e.button.y), W, H, dist, hit, &ray);
                    if (idx >= 0) editor.select(idx, hit.point); else editor.deselect();
                    console.onPick(idx, hit, ray, int(e.button.x), int(e.button.y));
                    setTitle(); input = true;
                }
                if (e.button.button == SDL_BUTTON_RIGHT)  rDown = true;
                if (e.button.button == SDL_BUTTON_MIDDLE) mDown = true;
                break;

            case SDL_EVENT_MOUSE_BUTTON_UP:
                if (e.button.button == SDL_BUTTON_LEFT) { lDown = false; if (objMoved) { console.printSelected(); objMoved = false; } }
                if (e.button.button == SDL_BUTTON_RIGHT) { rDown = false; if (camMoved) { console.printCamera();   camMoved = false; } }
                if (e.button.button == SDL_BUTTON_MIDDLE) { mDown = false; if (camMoved) { console.printCamera();   camMoved = false; } }
                input = true;      
                break;

            case SDL_EVENT_MOUSE_MOTION:
                if (lDown && editor.selected() >= 0) {
                    if (editor.mode == SceneEditor::Mode::Translate && editor.axis == SceneEditor::Axis::All)
                        editor.translateScreen(e.motion.xrel * kMove * unit, -e.motion.yrel * kMove * unit);
                    else
                        editor.apply(dragAmount(e.motion.xrel));
                    input = true; objMoved = true;
                }
                if (rDown) { editor.orbit(-e.motion.xrel * 0.4f, -e.motion.yrel * 0.4f); input = true; camMoved = true; }
                if (mDown) { editor.pan(-e.motion.xrel * 1.0f * unit, e.motion.yrel * 1.0f * unit);    input = true; camMoved = true; }
                break;

            case SDL_EVENT_MOUSE_WHEEL:
                editor.dolly(e.wheel.y > 0 ? 0.9f : 1.1f);
                input = true;
                break;

            case SDL_EVENT_KEY_DOWN: {
                using M = SceneEditor::Mode; using A = SceneEditor::Axis;
                switch (e.key.key) {
                case SDLK_ESCAPE: running = false; break;
                case SDLK_T: editor.mode = M::Translate;  break;
                case SDLK_R: editor.mode = M::Rotate;     break;
                case SDLK_V: editor.mode = M::RotateView; break;
                case SDLK_C: editor.mode = M::Scale;      break;
                case SDLK_H: editor.mode = M::Shear;      break;
                case SDLK_X: editor.axis = A::X;   break;
                case SDLK_Y: editor.axis = A::Y;   break;
                case SDLK_Z: editor.axis = A::Z;   break;
                case SDLK_SPACE: editor.axis = A::All; break;
                case SDLK_M: editor.mirror(); break;
                case SDLK_TAB: editor.selectNext(); break;
                case SDLK_BACKSPACE: editor.resetSelected(); break;
                case SDLK_RIGHT: case SDLK_UP:    editor.apply(keyAmount(+1)); break;
                case SDLK_LEFT:  case SDLK_DOWN:  editor.apply(keyAmount(-1)); break;
                case SDLK_PAGEUP:   editor.apply(keyAmount(+1)); break;
                case SDLK_PAGEDOWN: editor.apply(keyAmount(-1)); break;
                case SDLK_W: editor.walk(+kCamStep * unit);  break;
                case SDLK_S: editor.walk(-kCamStep * unit);  break;
                case SDLK_A: editor.pan(-kCamStep * unit, 0); break;
                case SDLK_D: editor.pan(+kCamStep * unit, 0); break;
                case SDLK_Q: editor.pan(0, +kCamStep * unit); break;
                case SDLK_E: editor.pan(0, -kCamStep * unit); break;
                case SDLK_F2:
                    savePPM("captura.ppm", img, W, H);
                    std::cout << "captura.ppm salva\n";
                    break;
                default: break;
                }
                setTitle(); input = true;
                break;
            }
            default: break;
            }
        }

        // comandos digitados no terminal
        for (;;) {
            std::string line;
            {
                std::lock_guard<std::mutex> lk(qMutex);
                if (cmdQueue.empty()) break;
                line = cmdQueue.front(); cmdQueue.pop();
            }
            bool quit = false;
            {   // abrir <caminho> | recarregar  (tratados aqui: trocam a cena inteira)
                std::istringstream ls(line); std::string c0; ls >> c0;
                if (c0 == "abrir" || c0 == "open" || c0 == "recarregar" || c0 == "reload") {
                    std::string p;
                    if (c0 == "recarregar" || c0 == "reload") p = scenePath;
                    else { std::getline(ls, p); }
                    size_t a = p.find_first_not_of(" \t\"'"), b = p.find_last_not_of(" \t\r\"'");
                    p = (a == std::string::npos) ? "" : p.substr(a, b - a + 1);
                    if (p.empty()) std::cout << "  uso: abrir caminho\\da\\cena.txt\n";
                    else if (openScene(p)) input = true;
                    setTitle();
                    continue;
                }
            }
            if (console.execute(line, quit)) input = true;
            if (quit) running = false;
            setTitle();
        }

        Uint64 now = SDL_GetTicks();
        if (input) { dirty = true; lastInput = now; }

        if (dirty) {
            bool interacting = lDown || rDown || mDown || (now - lastInput < 150);
            lowRes = interacting;
            draw(interacting ? 4 : 1);
            dirty = false;
        }
        else if (lowRes && now - lastInput >= 150) {
            draw(1);                              // refina quando para de mexer
            lowRes = false;
        }
        else {
            SDL_Delay(10);
        }
    }

    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
