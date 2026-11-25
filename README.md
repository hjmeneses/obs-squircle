# Squircle

*English below.*

Fuente para OBS Studio que dibuja un **squircle**: un rectángulo con esquinas de superelipse (|x|ⁿ + |y|ⁿ = 1), la forma de los íconos de iOS. Se dibuja en la GPU con una función de distancia, así que las orillas salen suavizadas a cualquier tamaño y escala.

- **Ancho** y **Alto** en pixeles.
- **Redondez de las esquinas** (0–100 %): radio como porcentaje de la mitad del lado corto. 100 % en un cuadrado da el squircle clásico.
- **Curvatura de las esquinas** (2–12): 2 = esquinas circulares (rectángulo redondeado normal); 4 a 5 = squircle; valores más altos se acercan a una esquina recta.
- **Color de relleno** y **color del borde**, ambos con transparencia. Con el relleno transparente queda solo el contorno.
- **Grosor del borde** (px), hacia adentro de la forma.

Sirve como máscara o forma para otros filtros, por ejemplo como fuente de forma de un efecto de vidrio.

**Estado:** versión 0.1.0, compilada para OBS 33. Probada en Windows; los paquetes de macOS y Ubuntu salen del mismo código pero no se han probado.

## Descarga e instalación

Descarga el último [release](https://github.com/hjmeneses/obs-squircle/releases/latest) (compilado para OBS 33):

- **Windows:** con OBS cerrado, descomprime el `.zip` en `C:\ProgramData\obs-studio\plugins`.
- **Ubuntu:** `sudo apt install ./squircle-<versión>-x86_64-linux-gnu.deb`.
- **macOS:** instala el `.pkg`.

En una escena: **+ → Squircle**.

## Compilar

Requiere CMake 3.28+ y Visual Studio 2022 con C++.

```powershell
cmake --preset windows-x64
cmake --build --preset windows-x64
cmake --install build_x64 --config RelWithDebInfo --prefix release
```

---

## English

An OBS Studio source that draws a **squircle**: a rectangle with superellipse corners (|x|ⁿ + |y|ⁿ = 1), the iOS icon shape. It is drawn on the GPU from a signed distance function, so edges stay anti-aliased at any size and scale.

- **Width** and **Height** in pixels.
- **Corner roundness** (0–100 %): radius as a percentage of half the shorter side; 100 % on a square gives the classic squircle.
- **Corner curvature** (2–12): 2 = circular corners (a regular rounded rectangle); 4–5 = squircle; higher values approach a sharp corner.
- **Fill color** and **border color**, both with alpha. A transparent fill leaves just the outline.
- **Border width** (px), drawn inward.

**Download:** [latest release](https://github.com/hjmeneses/obs-squircle/releases/latest) (built for OBS 33; tested on Windows, macOS and Ubuntu packages untested). Windows: with OBS closed, extract the zip into `C:\ProgramData\obs-studio\plugins`. Ubuntu: `sudo apt install ./squircle-<version>-x86_64-linux-gnu.deb`. macOS: install the `.pkg`. Then **+ → Squircle** in a scene. Build with the commands above.

## License

GPL-2.0, like the OBS plugin template it is based on.
