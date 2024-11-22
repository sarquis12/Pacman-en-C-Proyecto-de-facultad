// ========================== Librerías ==========================
#include <SDL.h>        // Librería para manejo de gráficos y eventos
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <SDL_ttf.h>    // Librería para renderizado de fuentes
#include <SDL_mixer.h>  // Librería para manejo de audio

// ========================== Definiciones ==========================

// Dimensiones máximas del mapa
#define COLUMNAS 50
#define FILAS 50
char maze[FILAS][COLUMNAS]; // Mapa del laberinto

// Tamaño de cada celda en píxeles (cada tile mide 30x30 píxeles)
#define TILE_SIZE 38    

// Caracteres que representan componentes del mapa
#define OBSTACLE '#'   // Obstáculo o pared
#define POINT '.'      // Punto comestible
#define PATH ' '       // Camino libre

// Caracteres que representan los personajes
#define PACMAN 'C'
#define GHOST 'G'
#define GHOST2 'H'
#define GHOST3 'I'

// Longitud máxima del camino calculado (A*), usado para IA de fantasmas
#define LONGITUD_MAXIMA_CAMINO 100

// Velocidades de los personajes (menor valor, más rápido)
#define VELOCIDAD_PACMAN 100
#define VELOCIDAD_FANTASMA 400
#define VELOCIDAD_FANTASMA2 200
#define VELOCIDAD_FANTASMA3 300

// Cantidad total de niveles en el juego
#define cant_tot_niv 4

// Tiempo de retardo para la salida de fantasmas adicionales (en milisegundos)
#define SALIDA_FANTASMA2 4000  // Fantasma 2 sale después de 4 segundos
#define SALIDA_FANTASMA3 7500  // Fantasma 3 sale después de 7.5 segundos

// ========================== Estructuras de Datos ==========================

// Estructura para representar un mapa
typedef struct {
    char maze[FILAS][COLUMNAS]; // Matriz que representa el laberinto
    char* nombre;               // Nombre del archivo del mapa
    int columnas, filas;        // Número de columnas y filas del mapa
} Mapa;

// Estructura para representar una posición en el mapa
typedef struct {
    int x, y; // Coordenadas X e Y
} Posicion;

// ========================== Variables Globales ==========================

// Arreglo de mapas para los niveles
Mapa mapas[] = {
    {.nombre = "Mapa_1.txt", .filas = 13, .columnas = 22}, // Nivel 1
    {.nombre = "Mapa_2.txt", .filas = 13, .columnas = 25}, // Nivel 2
    {.nombre = "Mapa_3.txt", .filas = 13, .columnas = 25}, // Nivel 3
    {.nombre = "Mapa_4.txt", .filas = 13, .columnas = 25}, // Nivel 4
};

// Nivel actual del juego (comienza en 0)
int nivel_actual = 0;

// ========================== Variables SDL ==========================

// Ventana y renderizador de SDL
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

// ========================== Variables de Juego ==========================

// Variables para puntaje
int puntaje = 0;         // Puntaje acumulado del jugador
int puntos_totales = 0;  // Puntos necesarios para completar el nivel

// ========================== Movimientos ==========================

// Enum para direcciones de movimiento
enum Direccion { ARRIBA, ABAJO, IZQUIERDA, DERECHA, NINGUNA };

// Arreglo que define los desplazamientos para cada dirección
Posicion direcciones[] = {
    {0, -1},  // ARRIBA
    {0, 1},   // ABAJO
    {-1, 0},  // IZQUIERDA
    {1, 0}    // DERECHA
};

// ========================== Colores ==========================

// Definición de colores para los diferentes elementos del juego
SDL_Color COLOR_OBSTACLE = {0, 0, 255, 255};    // Azul para obstáculos
SDL_Color COLOR_POINT = {255, 255, 255, 255};   // Blanco para puntos
SDL_Color COLOR_GHOST = {255, 0, 0, 255};       // Rojo para fantasmas
SDL_Color COLOR_PACMAN = {255, 255, 0, 255};    // Amarillo para Pac-Man
SDL_Color COLOR_FONDO = {0, 0, 0, 255};         // Negro para el fondo

// ========================== Variables de Audio ==========================

// Variables para controlar la reproducción del sonido
int ultimaX = -1, ultimaY = -1; // Posiciones anteriores de Pac-Man
Uint32 ultimoTiempoSonido = 0;  // Tiempo del último sonido reproducido
Uint32 delaySonido = 100;       // Delay de 100 ms entre sonidos

// Punteros a los efectos de sonido
Mix_Chunk *pacmanSound = NULL;  // Sonido de comer puntos
Mix_Chunk *inicioSound = NULL;  // Sonido de inicio del juego
Mix_Chunk *muerteSound = NULL;  // Sonido de muerte de Pac-Man



// =========================== Función cargar_mapa ===========================
// Función para cargar el mapa desde un archivo y contar los puntos
// Recibe un puntero a Mapa (estructura que contiene el nombre del archivo, filas y columnas)
int cargar_mapa(Mapa* mapa) {
    // Abre el archivo de texto que contiene el diseño del mapa
    FILE* file = fopen(mapa->nombre, "r");
    if (!file) {
        // Si no se pudo abrir el archivo, muestra un error y retorna -1
        perror("No se pudo abrir el archivo de mapa");
        return -1;
    }

    puntos_totales = 0; // Inicializa el contador de puntos

    // Lee el archivo línea por línea y guarda el contenido en la matriz del mapa
    for (int i = 0; i < mapa->filas; i++) {
        // fgets lee una línea del archivo y la almacena en el arreglo correspondiente
        if (fgets(mapa->maze[i], mapa->columnas + 2, file) == NULL) {
            // Si hay un error al leer una línea, muestra un error, cierra el archivo y retorna -1
            perror("Error al leer el mapa");
            fclose(file);
            return -1;
        }

        // Cuenta el número de puntos ('.') en el mapa para calcular el puntaje total
        for (int j = 0; j < mapa->columnas; j++) {
            if (mapa->maze[i][j] == POINT) {
                puntos_totales++;
            }
        }
    }

    fclose(file); // Cierra el archivo después de cargar el mapa
    return 0; // Retorna 0 si se cargó correctamente
}



// =========================== Función cargar_sonido ===========================
// Función para cargar los efectos de sonido utilizando SDL_mixer
void cargar_sonido() {
    // Inicializa SDL_mixer con frecuencia de audio de 44100 Hz, formato de audio predeterminado
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("Error al inicializar SDL_mixer: %s\n", Mix_GetError());
    }

    // Carga el sonido de Pac-Man al comer
    pacmanSound = Mix_LoadWAV("pacman_chomp.wav");
    if (pacmanSound == NULL) {
        printf("Error al cargar el sonido: %s\n", Mix_GetError());
    }

    // Carga el sonido de inicio del juego
    inicioSound = Mix_LoadWAV("Inicio.wav");
    if (inicioSound == NULL) {
        printf("Error al cargar el sonido: %s\n", Mix_GetError());
    }

    // Carga el sonido de la muerte de Pac-Man
    muerteSound = Mix_LoadWAV("Muerte.wav");
    if (muerteSound == NULL) {
        printf("Error al cargar el sonido: %s\n", Mix_GetError());
    }
}



// ====================== Función reproducir_y_liberar_sonido ======================
// Función para reproducir un efecto de sonido y liberar el recurso si se especifica
// Recibe el sonido a reproducir, un booleano para indicar si se debe esperar (delay) y la duración del delay
void reproducir_y_liberar_sonido(Mix_Chunk *Sonido, bool delay, int Duracion_delay) {
    if (Sonido != NULL) {
        // Reproduce el sonido en el canal -1 (primer canal disponible)
        Mix_PlayChannel(-1, Sonido, 0);

        // Si se especifica, espera la duración indicada y libera el recurso de sonido
        if (delay) {
            SDL_Delay(Duracion_delay);   // Espera la duración del sonido
            Mix_FreeChunk(Sonido);       // Libera el recurso de sonido
        }
    }
}



// ========================= Función mostrar_mensaje =========================
// Función para mostrar un mensaje de texto en pantalla, centrado
// Recibe el mapa, el mensaje de texto, y el color del texto
void mostrar_mensaje(Mapa* mapa, const char* mensaje, SDL_Color color) {
    // Inicializa SDL_ttf para manejo de fuentes y verifica errores
    if (TTF_Init() == -1) {
        printf("Error al inicializar SDL_ttf: %s\n", TTF_GetError());
        return;
    }

    // Carga la fuente desde el archivo .ttf con un tamaño de 64 puntos
    TTF_Font* font = TTF_OpenFont("Letras.ttf", 64);
    if (!font) {
        printf("Error al cargar la fuente: %s\n", TTF_GetError());
        return;
    }

    // Crea una superficie con el texto renderizado de forma sólida
    SDL_Surface* surfaceMessage = TTF_RenderText_Solid(font, mensaje, color);
    if (!surfaceMessage) {
        printf("Error al crear la superficie de texto: %s\n", TTF_GetError());
        TTF_CloseFont(font);
        TTF_Quit();
        return;
    }

    // Convierte la superficie de texto a una textura para renderizarla
    SDL_Texture* message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
    SDL_FreeSurface(surfaceMessage); // Libera la superficie ya que no se necesita más

    // Calcula el tamaño y posición del texto para centrarlo en pantalla
    int text_width, text_height;
    TTF_SizeText(font, mensaje, &text_width, &text_height);
    SDL_Rect textRect = { 
        ((mapa->columnas * TILE_SIZE) - text_width) / 2,  // Posición X centrada
        (mapa->filas * TILE_SIZE) / 2,                    // Posición Y centrada
        text_width, 
        text_height 
    };

    // Limpia la pantalla y establece el color de fondo a negro
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Renderiza el texto en pantalla
    SDL_RenderCopy(renderer, message, NULL, &textRect);
    SDL_RenderPresent(renderer); // Muestra el renderizado en pantalla

    SDL_Delay(2000); // Pausa la pantalla por 2 segundos para que el mensaje sea visible

    // Libera los recursos utilizados
    SDL_DestroyTexture(message);
    TTF_CloseFont(font);
    TTF_Quit();
}




// =========================== Función iniciar_SDL ===========================
// Función para inicializar SDL y crear la ventana y el renderizador
// Recibe un parámetro "mapa" de tipo Mapa para ajustar el tamaño de la ventana
int iniciar_SDL(Mapa mapa) {
    
    // Inicializar SDL con el subsistema de video
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        // Si falla la inicialización, muestra el error y retorna -1
        printf("Error al iniciar SDL: %s\n", SDL_GetError());
        return -1;
    }

    // Crear una ventana para el juego Pac-Man con las dimensiones basadas en el mapa
    window = SDL_CreateWindow(
        "Pacman con SDL2",               // Título de la ventana
        SDL_WINDOWPOS_UNDEFINED,         // Posición X de la ventana (centrada)
        SDL_WINDOWPOS_UNDEFINED,         // Posición Y de la ventana (centrada)
        mapa.columnas * TILE_SIZE,       // Ancho de la ventana (columnas * tamaño de celda)
        mapa.filas * TILE_SIZE,          // Alto de la ventana (filas * tamaño de celda)
        SDL_WINDOW_SHOWN                 // Mostrar ventana al crearla
    );

    // Verificar si la ventana se creó correctamente
    if (!window) {
        // Si falla la creación de la ventana, muestra el error, cierra SDL y retorna -1
        printf("Error al crear la ventana: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    // Crear el renderizador para dibujar en la ventana
    renderer = SDL_CreateRenderer(
        window,                          // Ventana asociada
        -1,                              // Índice del driver (-1 para usar el primero disponible)
        SDL_RENDERER_ACCELERATED         // Utilizar renderizado acelerado por hardware
    );

    // Verificar si el renderizador se creó correctamente
    if (!renderer) {
        // Si falla la creación del renderizador, muestra el error, destruye la ventana, cierra SDL y retorna -1
        printf("Error al crear el renderizador: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);       // Destruir la ventana creada previamente
        SDL_Quit();                      // Cerrar SDL
        return -1;
    }

    // Si todo se inicializó correctamente, retorna 0
    return 0;
}



// ======================= Función dibujar_rectangulo =======================
// Función para dibujar un rectángulo en la posición especificada con un color dado
// Parámetros:
//   x, y: Coordenadas del rectángulo en el mapa
//   porcentaje: Tamaño del rectángulo como un porcentaje del tamaño de una celda (TILE_SIZE)
//   color: Color del rectángulo (estructura SDL_Color)
void dibujar_rectangulo(int x, int y, int porcentaje, SDL_Color color) {
    // Establece el color para el renderizador
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    // Calcula el tamaño del rectángulo basado en el porcentaje
    int size = (TILE_SIZE * porcentaje) / 100; 
    int offset = (TILE_SIZE - size) / 2; // Calcula el desplazamiento para centrar el rectángulo

    // Define el rectángulo a dibujar
    SDL_Rect rect = {x * TILE_SIZE + offset, y * TILE_SIZE + offset, size, size};

    // Dibuja el rectángulo relleno
    SDL_RenderFillRect(renderer, &rect);
}



// ========================= Función dibujar_juego =========================
// Función para dibujar el mapa, Pac-Man, los fantasmas y contar puntos
void dibujar_juego(Mapa* mapa, Posicion* pacman, Posicion ghost, Posicion ghost2, Posicion ghost3) {
    // Limpia la pantalla y establece el color de fondo a negro
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Obtiene el tiempo actual para futuras animaciones (aunque aquí no se utiliza)
    Uint32 tiempoActual = SDL_GetTicks();

    // Itera sobre las filas y columnas del mapa
    for (int i = 0; i < mapa->filas; i++) {
        for (int j = 0; j < mapa->columnas; j++) {
            switch (mapa->maze[i][j]) {
                case OBSTACLE:
                    // Dibuja un obstáculo (bloque) en el mapa
                    dibujar_rectangulo(j, i, 100, COLOR_OBSTACLE);
                    break;
                case POINT:
                    // Si Pac-Man está en la posición del punto, elimina el punto
                    if (pacman->x == j && pacman->y == i) {
                        mapa->maze[i][j] = PATH; // Marca la celda como vacía
                    } else {
                        // Dibuja el punto con un tamaño más pequeño (10% de la celda)
                        dibujar_rectangulo(j, i, 10, COLOR_POINT);
                    }
                    break;
                default:
                    break;
            }
        }
    }

    // Dibuja a Pac-Man con un tamaño del 85% de la celda
    dibujar_rectangulo(pacman->x, pacman->y, 85, COLOR_PACMAN);

    // Dibuja los tres fantasmas con un tamaño del 75% de la celda
    dibujar_rectangulo(ghost.x, ghost.y, 75, COLOR_GHOST);
    dibujar_rectangulo(ghost2.x, ghost2.y, 75, COLOR_GHOST);
    dibujar_rectangulo(ghost3.x, ghost3.y, 75, COLOR_GHOST);

    // Muestra el renderizado en pantalla
    SDL_RenderPresent(renderer);
}



// ======================= Función Animacion_Muerte =======================
// Función para mostrar la animación de la muerte de Pac-Man
void Animacion_Muerte(Mapa* mapa, Posicion pacman, Posicion ghost, Posicion ghost2, Posicion ghost3) {

    // Dibuja el estado actual del juego
    dibujar_juego(mapa, &pacman, ghost, ghost2, ghost3);

    // Limpia la posición de Pac-Man
    dibujar_rectangulo(pacman.x, pacman.y, 100, COLOR_FONDO);

    // Reproduce el sonido de muerte de Pac-Man
    reproducir_y_liberar_sonido(muerteSound, false, 0);

    // Animación de reducción del tamaño de Pac-Man
    for (int i = 90; i > 5; i -= 5) {
        // Establece el color de Pac-Man (amarillo)
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);

        // Limpia la posición de Pac-Man
        dibujar_rectangulo(pacman.x, pacman.y, 100, COLOR_FONDO);

        // Dibuja Pac-Man con un tamaño decreciente
        dibujar_rectangulo(pacman.x, pacman.y, i, COLOR_PACMAN);

        // Muestra el cambio en pantalla
        SDL_RenderPresent(renderer);

        // Espera un breve periodo para crear el efecto de animación
        SDL_Delay(50);
    }

    // Limpia la pantalla después de la animación
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    dibujar_rectangulo(pacman.x, pacman.y, 0, COLOR_FONDO);
    SDL_RenderPresent(renderer);

    // Muestra el mensaje "GAME OVER"
    mostrar_mensaje(mapa, "PERDISTE", COLOR_GHOST);
}



// ====================== Función es_posicion_valida ======================
// Verifica si una posición es válida para moverse (no hay obstáculos ni fantasmas)
// Parámetros:
//   mapa: Estructura del mapa
//   pos: Posición a verificar
//   ghost1, ghost2, ghost3: Posiciones de los fantasmas
int es_posicion_valida(Mapa mapa, Posicion pos, Posicion ghost1, Posicion ghost2, Posicion ghost3) {
    // Verifica que la posición esté dentro de los límites del mapa y no haya obstáculos ni fantasmas
    return (pos.x >= 0 && pos.x < mapa.columnas &&
            pos.y >= 0 && pos.y < mapa.filas &&
            mapa.maze[pos.y][pos.x] != OBSTACLE &&
            !(pos.x == ghost1.x && pos.y == ghost1.y) &&
            !(pos.x == ghost2.x && pos.y == ghost2.y) &&
            !(pos.x == ghost3.x && pos.y == ghost3.y));
}

// =================== Función distancia_manhattan ===================
// Calcula la distancia de Manhattan (heurística) entre dos posiciones
// Parámetros:
//   a, b: Dos posiciones del mapa
// Retorna: La distancia de Manhattan entre las posiciones
int distancia_manhattan(Posicion a, Posicion b) {
    // Distancia de Manhattan: |x1 - x2| + |y1 - y2|
    return abs(a.x - b.x) + abs(a.y - b.y);
}


// ================= Algoritmo A* =================
// Función para encontrar el camino más corto entre un fantasma y Pac-Man
// Parámetros:
//   mapa: Estructura del mapa
//   inicio: Posición inicial del fantasma
//   objetivo: Posición de Pac-Man
//   camino: Arreglo para almacenar el camino encontrado
// Retorna: La longitud del camino encontrado o -1 si no hay camino
int a_estrella(Mapa mapa, Posicion inicio, Posicion objetivo, Posicion camino[LONGITUD_MAXIMA_CAMINO]) {

    // Inicializa la lista cerrada para marcar las posiciones visitadas
    int lista_cerrada[FILAS][COLUMNAS] = {0};

    // Variable para almacenar la longitud del camino
    int longitud_camino = 0;

    // Arreglo para registrar el recorrido del camino
    Posicion recorrido[FILAS][COLUMNAS] = {{-1, -1}};
    
    // Variable que indica si se ha encontrado el camino
    int encontrado = 0;

    // Estructura de datos para los nodos del algoritmo A*
    typedef struct {
        Posicion pos; // Posición del nodo
        int g; // Costo desde el nodo inicial hasta este nodo
        int h; // Heurística: Distancia estimada desde este nodo hasta el objetivo
        int f; // Costo total: g + h
    } Nodo;

    // Lista abierta para almacenar los nodos por explorar
    Nodo lista_abierta[mapa.filas * mapa.columnas];
    int contador_abierta = 0;

    // Inicializa el nodo de inicio
    Nodo nodo_inicio = {inicio, 0, distancia_manhattan(inicio, objetivo), distancia_manhattan(inicio, objetivo)};
    
    // Agrega el nodo inicial a la lista abierta
    lista_abierta[contador_abierta++] = nodo_inicio;

    // Bucle principal del algoritmo A*
    while (contador_abierta > 0 && !encontrado) {
        // Encuentra el nodo con el menor costo total (f) en la lista abierta
        int indice_min = 0;
        for (int i = 1; i < contador_abierta; i++) {
            if (lista_abierta[i].f < lista_abierta[indice_min].f) {
                indice_min = i;
            }
        }

        // Extrae el nodo con el menor costo de la lista abierta
        Nodo nodo_actual = lista_abierta[indice_min];
        lista_abierta[indice_min] = lista_abierta[--contador_abierta]; // Reduce el tamaño de la lista abierta

        // Marca el nodo actual como visitado en la lista cerrada
        lista_cerrada[nodo_actual.pos.y][nodo_actual.pos.x] = 1;

        // Si se ha alcanzado el objetivo, se reconstruye el camino
        if (nodo_actual.pos.x == objetivo.x && nodo_actual.pos.y == objetivo.y) {
            Posicion pos = objetivo;
            while (pos.x != inicio.x || pos.y != inicio.y) {
                camino[longitud_camino++] = pos; // Agrega la posición al camino
                pos = recorrido[pos.y][pos.x]; // Sigue el rastro del recorrido
            }
            camino[longitud_camino++] = inicio; // Agrega la posición inicial al final del camino

            // Invierte el camino para que vaya del inicio al objetivo
            for (int i = 0; i < longitud_camino / 2; i++) {
                Posicion temp = camino[i];
                camino[i] = camino[longitud_camino - i - 1];
                camino[longitud_camino - i - 1] = temp;
            }

            encontrado = 1; // Indica que se ha encontrado el camino
            break; // Sale del bucle
        }

        // Explora los vecinos del nodo actual (arriba, abajo, izquierda, derecha)
        for (int i = 0; i < 4; i++) {
            // Calcula la posición del vecino
            Posicion vecino = {nodo_actual.pos.x + direcciones[i].x, nodo_actual.pos.y + direcciones[i].y};

            // Verifica si el vecino es válido (dentro del mapa, no es un obstáculo y no ha sido visitado)
            if (vecino.x >= 0 && vecino.x < mapa.columnas && 
                vecino.y >= 0 && vecino.y < mapa.filas &&
                mapa.maze[vecino.y][vecino.x] != OBSTACLE &&
                !lista_cerrada[vecino.y][vecino.x]) {

                // Calcula los costos del vecino
                int g = nodo_actual.g + 1; // Costo desde el inicio hasta el vecino
                int h = distancia_manhattan(vecino, objetivo); // Heurística (distancia estimada al objetivo)
                int f = g + h; // Costo total

                // Verifica si el vecino ya está en la lista abierta
                int en_lista = 0;
                for (int j = 0; j < contador_abierta; j++) {
                    if (lista_abierta[j].pos.x == vecino.x && lista_abierta[j].pos.y == vecino.y) {
                        en_lista = 1;
                        // Si el nuevo costo total es menor, actualiza el nodo en la lista abierta
                        if (f < lista_abierta[j].f) {
                            lista_abierta[j].g = g;
                            lista_abierta[j].h = h;
                            lista_abierta[j].f = f;
                            recorrido[vecino.y][vecino.x] = nodo_actual.pos; // Registra el recorrido
                        }
                        break;
                    }
                }

                // Si el vecino no estaba en la lista abierta, lo agrega
                if (!en_lista) {
                    Nodo nodo_vecino = {vecino, g, h, f};
                    lista_abierta[contador_abierta++] = nodo_vecino;
                    recorrido[vecino.y][vecino.x] = nodo_actual.pos;
                }
            }
        }
    }

    // Retorna la longitud del camino encontrado o -1 si no se encontró un camino
    return encontrado ? longitud_camino : -1;
}

int main(int argc, char* argv[]) {
    // ========================== Configuración Inicial ==========================
    
    // Nivel de dificultad inicial
    int nivel_actual = 0;

    // Inicializar las rutas de los fantasmas usando el algoritmo A*
    Posicion camino[LONGITUD_MAXIMA_CAMINO];
    Posicion camino2[LONGITUD_MAXIMA_CAMINO];
    Posicion camino3[LONGITUD_MAXIMA_CAMINO];

    // Variables para almacenar la longitud del camino calculado por cada fantasma
    int longitud_camino, longitud_camino2, longitud_camino3;

    // Cargar sonido del juego
    cargar_sonido();

    // Indicador para salir del juego
    bool quit_game = false;

    // ========================== Bucle Principal del Juego ==========================
    while (!quit_game) {

        // Iniciar SDL y cargar el mapa del nivel actual
        if (iniciar_SDL(mapas[nivel_actual]) == -1 || cargar_mapa(&mapas[nivel_actual]) == -1) {
            perror("Error al inicializar SDL o cargar el mapa.");
            return -1;
        }

        // Mensaje de inicio en el nivel 0
        if (nivel_actual == 0) {
            mostrar_mensaje(&mapas[nivel_actual], "Pac-Man", COLOR_PACMAN);
            reproducir_y_liberar_sonido(inicioSound, true, 4200);
        }

        // Configuración de velocidades basadas en la dificultad
        int velocidad_pacman = VELOCIDAD_PACMAN - (20 * nivel_actual);
        int velocidad_fantasma = VELOCIDAD_FANTASMA - (25 * nivel_actual);
        int velocidad_fantasma2 = VELOCIDAD_FANTASMA2 - (25 * nivel_actual);
        int velocidad_fantasma3 = VELOCIDAD_FANTASMA3 - (30 * nivel_actual);

        // Posiciones iniciales de Pac-Man y los fantasmas
        Posicion pacman = {1, 6};
        Posicion ghost = {9, 5};
        Posicion ghost2 = {8, 5};
        Posicion ghost3 = {12, 5};
        enum Direccion direccion_actual = DERECHA;

        // Variables para controlar el ciclo del nivel
        bool quit_level = false;
        int contador_pacman = 0, contador_fantasma = 0, contador_fantasma2 = 0, contador_fantasma3 = 0;

        // Contadores para el retardo de salida de los fantasmas
        int Contador_Salida_fantasma2 = 0;
        int Contador_Salida_fantasma3 = 0;

        // Reiniciar puntaje del nivel
        puntaje = 0;
        mostrar_mensaje(&mapas[nivel_actual], "Comenzando", COLOR_POINT);

        // ========================== Bucle del Nivel ==========================
        while (!quit_level) {
            // Renderizar el estado actual del juego
            dibujar_juego(&mapas[nivel_actual], &pacman, ghost, ghost2, ghost3);

            // Manejo de eventos SDL
            SDL_Event e;
            while (SDL_PollEvent(&e) != 0) {
                if (e.type == SDL_QUIT) {
                    quit_game = true;
                    quit_level = true;
                } else if (e.type == SDL_KEYDOWN) {
                    switch (e.key.keysym.sym) {
                        case SDLK_UP: direccion_actual = ARRIBA; break;
                        case SDLK_DOWN: direccion_actual = ABAJO; break;
                        case SDLK_LEFT: direccion_actual = IZQUIERDA; break;
                        case SDLK_RIGHT: direccion_actual = DERECHA; break;
                        case SDLK_e: // Saltar nivel
                            mostrar_mensaje(&mapas[nivel_actual], "SALTAR NIVEL", COLOR_POINT);
                            nivel_actual++;
                            quit_level = true;
                            break;
                        case SDLK_q: // Salir del juego
                            mostrar_mensaje(&mapas[nivel_actual], "SALIENDO DEL JUEGO", COLOR_PACMAN);
                            quit_game = true;
                            quit_level = true;
                            break;
                    }
                }
            }

            // ========================== Movimiento de Pac-Man ==========================
            if (contador_pacman >= velocidad_pacman) {
                Posicion nueva_posicion = {
                    pacman.x + direcciones[direccion_actual].x,
                    pacman.y + direcciones[direccion_actual].y
                };
                if (es_posicion_valida(mapas[nivel_actual], nueva_posicion, ghost, ghost2, ghost3)) {
                    if (mapas[nivel_actual].maze[nueva_posicion.y][nueva_posicion.x] == POINT) {
                        puntaje++;
                        if (pacmanSound != NULL && !Mix_Playing(-1)) {
                            Mix_PlayChannel(-1, pacmanSound, 0);
                        }
                        mapas[nivel_actual].maze[nueva_posicion.y][nueva_posicion.x] = PATH;
                    }
                    pacman = nueva_posicion;
                }
                contador_pacman = 0;
            } else {
                contador_pacman++;
            }

            // ========================== Movimiento de Fantasmas ==========================
            // Fantasma 1
            if (contador_fantasma >= velocidad_fantasma) {
                longitud_camino = a_estrella(mapas[nivel_actual], ghost, pacman, camino);
                if (longitud_camino > 1 && es_posicion_valida(mapas[nivel_actual], camino[1], ghost, ghost2, ghost3)) {
                    ghost = camino[1];
                }
                contador_fantasma = 0;
            } else {
                contador_fantasma++;
            }

            // Fantasma 2
            if (contador_fantasma2 >= velocidad_fantasma2 && SALIDA_FANTASMA2 <= Contador_Salida_fantasma2) {
                longitud_camino2 = a_estrella(mapas[nivel_actual], ghost2, pacman, camino2);
                if (longitud_camino2 > 1 && es_posicion_valida(mapas[nivel_actual], camino2[1], ghost, ghost2, ghost3)) {
                    ghost2 = camino2[1];
                }
                contador_fantasma2 = 0;
            } else {
                if (SALIDA_FANTASMA2 > Contador_Salida_fantasma2) {
                    Contador_Salida_fantasma2++;
                } else {
                    contador_fantasma2++;
                }
            }

            // Fantasma 3
            if (contador_fantasma3 >= velocidad_fantasma3 && SALIDA_FANTASMA3 <= Contador_Salida_fantasma3) {
                longitud_camino3 = a_estrella(mapas[nivel_actual], ghost3, pacman, camino3);
                if (longitud_camino3 > 1 && es_posicion_valida(mapas[nivel_actual], camino3[1], ghost, ghost2, ghost3)) {
                    ghost3 = camino3[1];
                }
                contador_fantasma3 = 0;
            } else {
                if (SALIDA_FANTASMA3 > Contador_Salida_fantasma3) {
                    Contador_Salida_fantasma3++;
                } else {
                    contador_fantasma3++;
                }
            }

            // ========================== Verificar Condiciones de Fin de Nivel ==========================
            if (puntaje == puntos_totales || (nivel_actual == cant_tot_niv-1  && puntaje == puntos_totales - 1)) {
                mostrar_mensaje(&mapas[nivel_actual], "COMPLETADO", COLOR_PACMAN);
                

                if (nivel_actual == cant_tot_niv-1) {
                    mostrar_mensaje(&mapas[nivel_actual], "GANASTE!!", COLOR_PACMAN);
                    quit_game = true;
                }
                nivel_actual++;
                quit_level = true;
            }

            // Verificar colisión de Pac-Man con los fantasmas
            if ((pacman.x == ghost.x && pacman.y == ghost.y) ||
                (pacman.x == ghost2.x && pacman.y == ghost2.y) ||
                (pacman.x == ghost3.x && pacman.y == ghost3.y)) {
                Animacion_Muerte(&mapas[nivel_actual], pacman, ghost, ghost2, ghost3);
                quit_game = true;
                quit_level = true;
            }

            SDL_Delay(1);
        
        }

        // Destruir recursos SDL del nivel actual
        if (quit_game){
            mostrar_mensaje(&mapas[nivel_actual - 1], "JUEGO TERMINADO", COLOR_PACMAN);
        }
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
    }

    // ========================== Finalizar Juego ==========================
    SDL_Delay(2000);

    SDL_Quit();
    Mix_CloseAudio();
    return 0;
}
