#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <limits.h>
#include <string.h>
#include <mpi.h>

// Estructura para soluciones
typedef struct {
    int* ruta;
    int  costo;
} Solucion;

// Prototipos
void generar_costos_aleatorios(int N, int low, int high, int*** costos);
void construir_matriz(int N, int** costos, int** matriz);
int  evaluar_costo(int* solucion, int N, int** matriz_costos);
Solucion* poblacion_inicial(int N, int tam_pob, int** matriz_costos);
void seleccion_torneo(Solucion* poblacion, int tam_pob, int P,
                      Solucion* padre1, Solucion* padre2);
void cruzaPMX(int* ind1, int* ind2, int size, int* hijo1, int* hijo2);
void mutacion(int* camino, int size);

int main(int argc, char** argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Seed distinto por proceso
    srand(time(NULL) + rank * 31);

    // Parámetros del AG
    const int N       = 200;   // número de ciudades
    const int G       = 500;   // generaciones
    const int TAM_POB = 1000;// tamaño población
    const float pc    = 0.9f; // prob. cruce
    const float pm    = 0.1f; // prob. mutación
    const int P       = 10;    // tamaño torneo

    // 1) Maestro genera matriz de costes, esclavos reservan espacio
    int** costos = NULL;
    int** matriz_costos = malloc(N * sizeof(int*));
    for (int i = 0; i < N; i++) {
        matriz_costos[i] = malloc(N * sizeof(int));
    }
    if (rank == 0) {
        generar_costos_aleatorios(N, 5, 500, &costos);
        construir_matriz(N, costos, matriz_costos);
    }

    // 2) Broadcast fila a fila de la matriz de costes
    for (int i = 0; i < N; i++) {
        MPI_Bcast(matriz_costos[i], N, MPI_INT, 0, MPI_COMM_WORLD);
    }

    // 3) Crear población inicial
    Solucion* poblacion = malloc(TAM_POB * sizeof(Solucion));
    for (int i = 0; i < TAM_POB; i++) {
        poblacion[i].ruta = malloc(N * sizeof(int));
    }
    if (rank == 0) {
        Solucion* tmp = poblacion_inicial(N, TAM_POB, matriz_costos);
        for (int i = 0; i < TAM_POB; i++) {
            memcpy(poblacion[i].ruta, tmp[i].ruta, N * sizeof(int));
            poblacion[i].costo = tmp[i].costo;
            free(tmp[i].ruta);
        }
        free(tmp);
    }

    // Broadcast población inicial completada
    for (int i = 0; i < TAM_POB; i++) {
        MPI_Bcast(poblacion[i].ruta, N, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(&poblacion[i].costo, 1, MPI_INT, 0, MPI_COMM_WORLD);
    }

    // 5) Preparar nueva población
    Solucion* nueva_pob = malloc(TAM_POB * sizeof(Solucion));
    for (int i = 0; i < TAM_POB; i++) {
        nueva_pob[i].ruta = malloc(N * sizeof(int));
    }

    // Medir tiempo del bucle de generaciones
    MPI_Barrier(MPI_COMM_WORLD);
    double t_start = MPI_Wtime();

    // 6) Bucle de generaciones
    for (int gen = 0; gen < G; gen++) {
        int chunk = TAM_POB / size;
        int start = rank * chunk;
        int end   = (rank == size-1 ? TAM_POB : start + chunk);

        // Distribución de rangos
        if (rank == 0) {
            for (int dest = 1; dest < size; dest++) {
                int r[2] = { dest*chunk,
                             (dest==size-1 ? TAM_POB : dest*chunk+chunk) };
                MPI_Send(r, 2, MPI_INT, dest, 0, MPI_COMM_WORLD);
            }
        } else {
            int r[2];
            MPI_Recv(r, 2, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            start = r[0]; end = r[1];
        }

        // Generar nueva población local
        for (int i = start; i < end; i += 2) {
            Solucion padre1, padre2, hijo1, hijo2;
            hijo1.ruta = malloc(N * sizeof(int));
            hijo2.ruta = malloc(N * sizeof(int));

            seleccion_torneo(poblacion, TAM_POB, P, &padre1, &padre2);

            if ((float)rand()/RAND_MAX < pc) {
                cruzaPMX(padre1.ruta, padre2.ruta, N, hijo1.ruta, hijo2.ruta);
            } else {
                memcpy(hijo1.ruta, padre1.ruta, N*sizeof(int));
                memcpy(hijo2.ruta, padre2.ruta, N*sizeof(int));
            }

            if ((float)rand()/RAND_MAX < pm) mutacion(hijo1.ruta, N);
            if ((float)rand()/RAND_MAX < pm) mutacion(hijo2.ruta, N);

            hijo1.costo = evaluar_costo(hijo1.ruta, N, matriz_costos);
            hijo2.costo = evaluar_costo(hijo2.ruta, N, matriz_costos);

            memcpy(nueva_pob[i].ruta,     hijo1.ruta, N*sizeof(int));
            nueva_pob[i].costo = hijo1.costo;
            if (i+1 < end) {
                memcpy(nueva_pob[i+1].ruta, hijo2.ruta, N*sizeof(int));
                nueva_pob[i+1].costo = hijo2.costo;
            }

            free(hijo1.ruta);
            free(hijo2.ruta);
        }

        // Recoger bloques
        if (rank == 0) {
            for (int src = 1; src < size; src++) {
                int s = src*chunk;
                int e = (src==size-1? TAM_POB:src*chunk+chunk);
                for (int i = s; i < e; i++) {
                    MPI_Recv(nueva_pob[i].ruta, N, MPI_INT, src, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    MPI_Recv(&nueva_pob[i].costo, 1, MPI_INT, src, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                }
            }
            for (int i = 0; i < TAM_POB; i++) {
                memcpy(poblacion[i].ruta, nueva_pob[i].ruta, N*sizeof(int));
                poblacion[i].costo = nueva_pob[i].costo;
            }
        } else {
            for (int i = start; i < end; i++) {
                MPI_Send(nueva_pob[i].ruta, N, MPI_INT, 0, 1, MPI_COMM_WORLD);
                MPI_Send(&nueva_pob[i].costo, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
            }
        }

        MPI_Barrier(MPI_COMM_WORLD);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double t_end = MPI_Wtime();
    if (rank == 0) {
        printf("Tiempo total GA paralelo: %f segundos\n", t_end - t_start);
    }

    // Imprimir mejor solución
    if (rank == 0) {
        int mejor = 0;
        for (int i = 1; i < TAM_POB; i++) {
            if (poblacion[i].costo < poblacion[mejor].costo)
                mejor = i;
        }
        printf("Mejor costo final: %d\nRuta: ", poblacion[mejor].costo);
        for (int j = 0; j < N; j++) printf("%d ", poblacion[mejor].ruta[j]);
        printf("\n");
    }

    MPI_Finalize();
    return 0;
}

// ——— Implementaciones ———

void generar_costos_aleatorios(int N, int low, int high, int*** costos) {
    *costos = malloc((N-1) * sizeof(int*));
    for (int i = 0; i < N-1; i++) {
        int c = N - i - 1;
        (*costos)[i] = malloc(c * sizeof(int));
        for (int j = 0; j < c; j++)
            (*costos)[i][j] = low + rand() % (high - low + 1);
    }
}

void construir_matriz(int N, int** costos, int** matriz) {
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            matriz[i][j] = 0;
    for (int i = 0; i < N-1; i++)
        for (int j = 0; j < N-i-1; j++) {
            matriz[i][j+i+1] = costos[i][j];
            matriz[j+i+1][i] = costos[i][j];
        }
}

int evaluar_costo(int* solucion, int N, int** m) {
    int costo = 0;
    for (int i = 0; i < N-1; i++)
        costo += m[ solucion[i] ][ solucion[i+1] ];
    costo += m[ solucion[N-1] ][ solucion[0] ];
    return costo;
}

Solucion* poblacion_inicial(int N, int tam_pob, int** m) {
    Solucion* pop = malloc(tam_pob * sizeof(Solucion));
    int* ciudades = malloc(N * sizeof(int));
    for (int i = 0; i < N; i++) ciudades[i] = i;

    for (int i = 0; i < tam_pob; i++) {
        // Fisher–Yates
        for (int j = 0; j < N; j++) {
            int k = j + rand() % (N - j);
            int tmp = ciudades[j]; ciudades[j] = ciudades[k]; ciudades[k] = tmp;
        }
        pop[i].ruta = malloc(N * sizeof(int));
        for (int j = 0; j < N; j++) pop[i].ruta[j] = ciudades[j];
        pop[i].costo = evaluar_costo(pop[i].ruta, N, m);
    }
    free(ciudades);
    return pop;
}

void seleccion_torneo(Solucion* poblacion, int tam_pob, int P,
                      Solucion* padre1, Solucion* padre2) {
    int idx = rand() % tam_pob;
    for (int i = 1; i < P; i++) {
        int c = rand() % tam_pob;
        if (poblacion[c].costo < poblacion[idx].costo)
            idx = c;
    }
    *padre1 = poblacion[idx];

    idx = rand() % tam_pob;
    for (int i = 1; i < P; i++) {
        int c = rand() % tam_pob;
        if (poblacion[c].costo < poblacion[idx].costo)
            idx = c;
    }
    *padre2 = poblacion[idx];
}

void cruzaPMX(int* ind1, int* ind2, int size, int* hijo1, int* hijo2) {
    int p1, p2, ok;
    do { p1 = rand()%size; p2 = rand()%size; }
    while (abs(p1-p2) < 2);
    if (p1 > p2) { int t=p1; p1=p2; p2=t; }

    for (int i = 0; i < size; i++) hijo1[i] = hijo2[i] = -1;
    for (int i = p1; i < p2; i++) {
        hijo1[i] = ind2[i];
        hijo2[i] = ind1[i];
    }
    for (int i = 0; i < size; i++) {
        if (i >= p1 && i < p2) continue;
        int v = ind1[i];
        do {
            ok = 1;
            for (int j = p1; j < p2; j++)
                if (hijo1[j] == v) { ok = 0; break; }
            if (!ok) for (int k = 0; k < size; k++)
                if (ind2[k] == v) { v = ind1[k]; break; }
        } while (!ok);
        hijo1[i] = v;

        v = ind2[i];
        do {
            ok = 1;
            for (int j = p1; j < p2; j++)
                if (hijo2[j] == v) { ok = 0; break; }
            if (!ok) for (int k = 0; k < size; k++)
                if (ind1[k] == v) { v = ind2[k]; break; }
        } while (!ok);
        hijo2[i] = v;
    }
}

void mutacion(int* camino, int size) {
    int i = rand()%size, j;
    do { j = rand()%size; } while (j == i);
    int tmp = camino[i]; camino[i] = camino[j]; camino[j] = tmp;
}