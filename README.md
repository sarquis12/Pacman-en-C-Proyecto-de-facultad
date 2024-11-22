# Pac-Man en C usando SDL2
## 📖 Descripción del Proyecto
Este proyecto es una implementación del clásico juego Pac-Man, desarrollado en el lenguaje de programación C utilizando la biblioteca SDL2. El objetivo principal es recrear la funcionalidad del juego original, centrándonos en la lógica, las interacciones y el diseño de niveles, mientras se minimiza la dependencia de gráficos avanzados.

En el juego, controlas a Pac-Man para recoger todos los puntos en un laberinto mientras evitas a los fantasmas que te persiguen. Los niveles aumentan en dificultad a medida que progresas, con fantasmas más rápidos y mapas más complejos

---

## 🎯 Objetivo del Proyecto

Este proyecto fue desarrollado con fines educativos y como un desafío técnico para:

- Aprender a usar estructuras de datos en C.
- Implementar algoritmos como A (A-Star)* para la inteligencia artificial de los fantasmas.
- Explorar el uso de bibliotecas externas como SDL2 para la gestión de gráficos y sonidos.
- Desarrollar una lógica de juego basada en niveles y dificultad progresiva.

---

## 🕹️ Tutorial: Cómo Ejecutar el Juego

### Descarga e Instalación de MSYS2
#### Descargar MSYS2

- Visita la página oficial de MSYS2: https://www.msys2.org/.
- Haz clic en Download para descargar el instalador.
- Ejecuta el instalador y sigue los pasos para instalar MSYS2. Es recomendable instalarlo en una ruta sencilla, por ejemplo: C:/MSYS2/.

#### Actualizar MSYS2

- Abre la terminal MSYS2 MINGW64 desde el menú de inicio (puedes buscarla usando el buscador de Windows). El icono de la aplicación debería verse de la siguiente manera:


- Actualiza los paquetes con los siguientes comandos en la terminal de MSYS2:

``` makefile
pacman -Syu
pacman -Su
```
Si el proceso te pide cerrar y reabrir la terminal, hazlo.

#### Instalar SDL2

Ejecuta el siguiente comando en la terminal de MSYS2 para instalar la biblioteca SDL2, que es esencial para ejecutar el juego:


```makefile
pacman -S mingw-w64-x86_64-SDL2
```

### Ejecutar el Juego

- Asegúrate de que todos los archivos necesarios del proyecto, incluido el ejecutable PACMAN.exe y los mapas (Mapa_1.txt, Mapa_2.txt, etc.), estén en la misma carpeta.

- Abre la terminal MSYS2 MINGW64 y navega hasta la carpeta donde se encuentra el archivo PACMAN.exe. Por ejemplo:

```makefile
cd /c/Users/tu_usuario/Desktop/Proyecto-PacMan
```

- Ejecuta el juego con el siguiente comando:

```makefile
./PACMAN.exe
```
---

## Nota

Si prefieres no usar MSYS2 cada vez que ejecutes el programa, puedes configurar tu variable de entorno Path para incluir el directorio de SDL2. Esto te permitirá ejecutar el programa directamente desde cualquier terminal. Para hacerlo:

- Abre el Panel de control y navega a Sistema > Configuración avanzada del sistema.
- Haz clic en Variables de entorno.
- En Variables del sistema, selecciona Path y haz clic en Editar.
- Añade la ruta al directorio bin de MSYS2, por ejemplo:
```makefile
C:\MSYS2\mingw64\bin
```
Después de configurar esto, podrás ejecutar PACMAN.exe directamente desde el Explorador de Windows.