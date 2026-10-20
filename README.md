# TP Integrador — Buffet Albert Einstein

Trabajo Práctico grupal de **Algoritmos y Estructuras de Datos** (UTN FRBA) —
unidad de **archivos binarios en C/C++**.

> **Grupo:** _(completar: número de grupo)_
> **Integrantes:** 

cristian vazquez crvazquez@frba.utn.edu.ar (ventas.cpp)
mateo pidal mpidal@frba.utn.edu.ar (cierre.cpp)
luis cova lucova@frba.utn.edu.ar (resumen.cpp)
santiago manchin samachin@frba.utn.edu.ar (normalizacion.cpp)


La consigna completa está en **`enunciado.pdf`**. Leela antes de arrancar: el
cliente (Alberto) cuenta su problema a su manera y ustedes tienen que descubrir
cómo resolverlo con lo que vimos de archivos.

## Estructura del repo

```
.
├── enunciado.pdf              # la consigna
├── datos/                     # archivos PROVISTOS por la cátedra (no se tocan)
│   ├── comandas_historicas.dat
│   ├── inventario.dat
│   └── dump_datos_de_prueba.txt   # los dos .dat en texto, para verificar la lectura
├── .gitignore
│
│   # --- estos los crean USTEDES (no vienen en el repo base) ---
├── normalizacion.cpp          # cada uno con su propio main; NO hay main.cpp
├── ventas.cpp
├── cierre.cpp
└── resumen.cpp
```

Los 4 programas los escriben ustedes desde cero: el repo base no trae código.
Créenlos en la raíz del repo (como en el diagrama del enunciado).

## Cómo trabajar

Los programas leen y escriben archivos `.dat` en la carpeta donde se ejecutan.
Los archivos de `datos/` son los **originales** y no se modifican: copiálos a la
carpeta donde vas a compilar y correr, y trabajá sobre esas copias.

```bash
cp datos/comandas_historicas.dat datos/inventario.dat .
```

Compilar y correr cada programa (cada uno tiene su propio `main`, no hay `main.cpp`):

```bash
g++ -O2 -o normalizacion normalizacion.cpp
g++ -O2 -o ventas        ventas.cpp
g++ -O2 -o cierre        cierre.cpp
g++ -O2 -o resumen       resumen.cpp
```

Orden de ejecución (cada programa deja los archivos que usa el siguiente):

```
./normalizacion     # historicas + inventario  ->  mozos.dat, comandas_dd-mm-aaaa.dat (varios), inventario actualizado
./ventas            # carga interactiva de la semana  ->  agrega a las planillas del día
./cierre            # junta los días de la semana      ->  comandas_semana_sX-mm.dat
./resumen           # imprime el resumen por mozo + total del buffet
```

## Verificar que leen bien el binario

`datos/dump_datos_de_prueba.txt` tiene el contenido de los dos archivos provistos
en texto legible. Si tu programa imprime otra cosa al leerlos, revisá los `struct`
(orden de campos / tamaños): tienen que dar `sizeof(ComandaHistorica)=76` y
`sizeof(Producto)=64`.

## Qué se entrega

- Este mismo repo (uno por grupo), con los **4 programas** `.cpp` en la raíz
  (cada uno con su propio `main`, no hay `main.cpp`).
- El **README** con el grupo, los integrantes (nombre + usuario de GitHub) y, si
  hace falta, cualquier aclaración de cómo correrlo.
- Los `.dat` de `datos/` son los **provistos por la cátedra** y ya vienen en el
  repo: no los borren ni agreguen otros. Cualquier `.dat` que ustedes copien a la
  raíz o generen al correr **no** se versiona (el `.gitignore` ya los deja afuera).
  La cátedra corre sus programas sobre su propio dataset.
- **Commits repartidos:** cada integrante tiene que tener commits propios a lo
  largo del trabajo. El historial es parte de lo que se mira para la defensa
  individual (no vale un único commit final ni que suba todo una sola persona).
