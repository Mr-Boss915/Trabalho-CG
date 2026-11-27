# Ray Tracer CPU — Computação Gráfica I (UFC)

Ray tracer em C++ que roda na CPU, com iluminação de Phong, sombras, texturas,
malhas `.obj`, transformações geométricas e um visualizador interativo com *picking*.

## Recursos

- **Objetos:** esfera, plano (ilimitado ou retangular), cilindro (com ou sem tampas), cone e malha de triângulos (`.obj`).
- **Iluminação:** luz ambiente, luzes pontuais, modelo de Phong e sombras por raio de sombra.
- **Texturas:** JPEG, aplicadas no plano (`texture <arquivo> <escala>`).
- **Transformações:** translação, escala, rotações em x, y e z, rotação em eixo arbitrário (mudança de base ou quatérnio), cisalhamento e espelho em plano arbitrário, com `pivot` e matriz 4×4 (`matrix`).
- **Câmera:** olho, ponto de visada (`lookAt`) e `up`; janela de projeção definida por largura, altura e `z`.
- **Visualizador (SDL3):** clique para selecionar o objeto sob o cursor (picking), arrastar para transformá-lo, câmera orbital, terminal com comandos e exportação da matriz.

## Estrutura

| Pasta / arquivo | Conteúdo |
| --- | --- |
| `header/`, `src/` | Um par `.h` / `.cpp` por classe |
| `Archive/scenes/` | Cenas `.txt`, malhas `.obj` e texturas |
| `src/viewer_sdl3.cpp` | `main` do visualizador interativo (SDL3) |

Classes principais: `Vector3`, `Color`, `Ray`, `Object` / `HitRecord`, `Material`, `Texture`,
`Sphere`, `Plane`, `Cylinder`, `Cone`, `Mesh`, `Camera`, `Scene`, `SceneLoader`, `Renderer`,
`Mat4`, `Quaternion`, `TransformedObject`, `SceneEditor`, `SceneConsole`.

## Compilar

Requisitos: Visual Studio com C++17, configuração Release x64.

| Biblioteca | Uso |
| --- | --- |
| SDL3 | Janela interativa do visualizador |
| `stb_image.h` | Leitura de texturas JPEG (copiar para `header/`) |
| `stb_image_write.h` | Gravação do `imagem.jpg` (copiar para `header/`) |
| GLEW e GLFW | Somente o `main` original, que usa a janela OpenGL |

Configuração do SDL3 em *Properties*:

1. *C/C++ → General → Additional Include Directories*: pasta `include` do SDL3.
2. *Linker → General → Additional Library Directories*: pasta `lib\x64`.
3. *Linker → Input → Additional Dependencies*: `SDL3.lib`.
4. *Linker → System → SubSystem*: Console.
5. Copiar `SDL3.dll` para a pasta do `.exe`.

O projeto deve ter um único `main`. Com o `.exe` aberto o linker não sobrescreve o arquivo (erro LNK1104).

## Executar

```
Tarefas CG.exe [cena.txt] [largura]
```

Sem argumento, o visualizador abre a cena definida em `cenaPadrao()` (`viewer_sdl3.cpp`),
montada a partir da pasta do arquivo-fonte. A largura padrão é 800 pixels e a altura segue a
proporção da `janela` da cena. Os caminhos de `.obj` e de textura dentro da cena são lidos a
partir da *Working Directory*.

### Controles

| Entrada | Ação |
| --- | --- |
| Clique esquerdo | Seleciona o objeto (picking); o ponto clicado vira o pivot |
| Arrastar esquerdo | Aplica a transformação do modo atual |
| Arrastar direito | Orbita a câmera |
| Arrastar do meio / roda | Pan / aproximar |
| `T` `R` `V` `C` `H` `M` | Modo: translação, rotação, rotação em torno do olhar, escala, cisalhamento, espelho |
| `X` `Y` `Z` / Espaço | Eixo da transformação / todos |
| `W A S D Q E` | Move a câmera |
| `Home` | Volta a câmera ao estado do `.txt` |
| `Backspace` | Desfaz as transformações do objeto selecionado |
| `F2` | Salva `captura.ppm` |
| `Esc` | Sai |

### Terminal

`list`, `select N`, `pick X Y`, `info`, `pivot X Y Z`, `translate`, `scale`, `rotatex|y|z`,
`rotate`, `rotateq`, `shear`, `mirror`, `reset`, `export`, `cam ...` (`reset`, `eye`, `target`,
`orbit`, `dolly`, `pan`, `walk`), `abrir CAMINHO.txt`, `recarregar`, `ajuda`, `sair`.

`export` imprime uma linha `matrix` com a matriz acumulada do objeto; cole-a no `.txt` depois
do objeto para guardar a transformação.

## Formato da cena

Tokens separados por espaço ou quebra de linha; linhas que começam com `#` são comentários.

```
camera   eye(3) lookAt(3) up(3) fov
janela   largura altura z          # distância do olho à janela = abs(eye.z - z)
ambient  r g b
background r g b
light    posição(3) intensidade(3)

sphere   centro(3) R   ka(3) kd(3) ks(3) n
plane    Q(3) u(3) v(3) ka(3) kd(3) ks(3) n  [bounded] [texture arquivo escala]
cylinder base(3) eixo(3) H R ka(3) kd(3) ks(3) n  [open] [texture arquivo escala]
cone     base(3) eixo(3) H R ka(3) kd(3) ks(3) n  [texture arquivo escala]
mesh     tetra | arquivo.obj  ka(3) kd(3) ks(3) n  [texture arquivo escala]
```

Depois de qualquer objeto podem vir transformações, aplicadas na ordem escrita:

```
translate tx ty tz         scale sx sy sz
rotatex g | rotatey g | rotatez g
rotate ax ay az g          rotateq ax ay az g     # eixo arbitrário: mudança de base / quatérnio
shear xy xz yx yz zx zy    mirror px py pz nx ny nz
pivot cx cy cz             matrix m00 m01 ... m33   # 16 valores, linha a linha
```

Ângulos em graus. `pivot` vale para as transformações seguintes do mesmo objeto.
Com `bounded`, `u` e `v` do plano são os lados de um retângulo que parte de `Q`.

## Unidades das cenas

As cenas podem estar em centímetros (janela a ~200) ou em metros (janela a ~0,3). Use o
`cubo.obj` da unidade certa: `cubo.obj` (cm), `cubo_m.obj` (m) ou `cubo_q.obj` (m, no
primeiro octante). A velocidade da câmera no visualizador se ajusta sozinha à escala da cena.

## Documentação completa

Descrição das classes, derivação das interseções, comparação com a lista de requisitos e
plano de trabalho estão no documento "Ray Tracer CG I: documentação e requisitos".
