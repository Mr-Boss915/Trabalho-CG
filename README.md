## Formato da cena
 
Tokens separados por espaço ou quebra de linha; linhas que começam com `#` são comentários.
Favor usar quebra de linha para objetos e espaco para transformacoes
```
camera   eye(3) lookAt(3) up(3) fov
janela   largura altura z          # distância do olho à janela = abs(eye.z - z)
ambient  r g b
background r g b
light    posição(3) intensidade(3)
 
sphere   centro(3) R   ka(3) kd(3) ks(3) n
plane    Q(3) u(3) v(3) ka(3) kd(3) ks(3) n  [bounded] [texture arquivo escala]
cylinder base(3) eixo(3) H R ka(3) kd(3) ks(3) n  [open] 
cone     base(3) eixo(3) H R ka(3) kd(3) ks(3) n
mesh     tetra | arquivo.obj  ka(3) kd(3) ks(3) n
```
Exemplo

```
camera
0.25 0.25 0.5
0.25 0.14 0.22
0 1 0
38
 
janela
0.6 0.6 0.2
 
ambient
0.3 0.3 0.3
 
background
0.3 0.3 0.3
 
light
0.15 0.39 0.48
0.6 0.6 0.6
 
light
0.44 0.35 0.7
0.15 0.15 0.15

plane
0.05 0.4 0.1
0 0 0.5
0.4 0 0
0.686 0.933 0.933
0.686 0.933 0.933
0.686 0.933 0.933
30
bounded

mesh
cubo_q.obj
0.1 0.8 0.1
0.1 0.8 0.1
0.3 0.3 0.3
30
pivot
0.25 0.12 0.3
rotatey
45
translate
-0.04 0 0

Cone e cilindro podem incluir ou nao suas tampas com a adicao do termo open na criacao segue exemplo

cylinder
0.25 0.1 0.17
0 1 0
0.08
0.03
1 1 0.1
1 1 0.1
0.3 0.3 0.3
30
open
pivot
0.25 0.1 0.17
shear
0.8 0 0 0 0 0


planos podem ou nao ser infitos
com o uso do termo bounded apos seus parametros


plane
0.05 0.1 0.1
0 0.3 0
0.4 0 0
0.686 0.933 0.933
0.686 0.933 0.933
0.686 0.933 0.933
30
bounded

```
### Transformacoes
```
translate tx ty tz         scale sx sy sz
rotatex g | rotatey g | rotatez g
rotate ax ay az g          rotateq ax ay az g     # eixo arbitrário: mudança de base / quatérnio
shear xy xz yx yz zx zy    mirror px py pz nx ny nz
pivot cx cy cz             matrix m00 m01 ... m33   # 16 valores, linha a linha
```
```Inserir apos os objetos```

### Terminal
 
`list`, `select N`, `pick X Y`, `info`, `pivot X Y Z`, `translate`, `scale`, `rotatex|y|z`,
`rotate`, `rotateq`, `shear`, `mirror`, `reset`, `export`, `cam`, `recarregar`, `ajuda`, `sair`.
 
`export` imprime uma linha `matrix` com a matriz acumulada do objeto; cole-a no `.txt` depois
do objeto para guardar a transformação.


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
| `Backspace` | Desfaz as transformações do objeto selecionado ||
| `Esc` | Sai |

### Requisitos

| Item | Requisito | Peso | Situação | Comentario |
| --- | --- | --- | --- | --- |
| 1.1 | Coerência temática | Obrig. | - | - |
| 1.2 | Todos os objetos no primeiro octante | Obrig. | - | - |
| 1.3.1 | Esfera, cilindro, cone e malha | Obrig. | Atendido | - |
| 1.3.2 | Pelo menos quatro materiais distintos | Obrig. | - | - |
| 1.3.3 | Pelo menos uma textura | Obrig. | - | apenas plano |
| 1.4.1 | Translação | Obrig. | Atendido | - |
| 1.4.2 | Rotação em torno de x, y ou z | Obrig. | Atendido | - |
| 1.4.2 | Rotação em eixo arbitrário  | Obrig. | Atendido | - |
| 1.4.3 | Escala | Obrig. | Atendido | - |
| 1.4.4 | Cisalhamento | +0,5 | Atendido | - |
| 1.4.5 | Espelho em relação a plano arbitrário | +0,5 | Atendido | - |
| 1.5.1 | Luz pontual | Obrig. | Atendido | - |
| 1.5.2 | Luz spot | +1,0 | - | - |
| 1.5.3 | Luz direcional | +0,5 | - | - |
| 1.5.4 | Luz ambiente | Obrig. | Atendido | - |
| 2.1 | Eye, At point e Up | Obrig. | Atendido | - |
| 2.2 | Distância focal e janela (xmin, xmax, ymin, ymax) | Obrig. | - | - |
| 3.1 | Projeção em perspectiva | Obrig. | Atendido | - |
| 3.1.1.1 | Zoom out | Obrig. | - | - |
| 3.1.1.2 | Zoom in | Obrig. | - | - |
| 3.1.2 | Perspectiva com um, dois e três pontos de fuga | +0,5 cada | - | - |
| 3.2 | Projeção ortográfica | +0,5 | - | - |
| 3.3 | Projeção oblíqua | +0,5 | - | - |
| 5.1 | Função de pick | Obrig. | Atendido | - |
| 5.2 | Interface gráfica | +0,5 a 1,0 | Parcial | Terminal com comandos fazer interface |


| Biblioteca | Uso |
| --- | --- |
| SDL3 | Janela interativa do visualizador |



