#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <limits.h>

// Estructura para representar una solución (ruta y su costo)
typedef struct {
    int* ruta;
    int costo;
} Solucion;

// Función para construir la matriz de costos
void construir_matriz(int N, int** costos, int** matriz) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            matriz[i][j] = 0;
        }
    }

    for (int i = 0; i < N - 1; i++) {
        for (int j = 0; j < N - i - 1; j++) {
            matriz[i][j + i + 1] = costos[i][j];
            matriz[j + i + 1][i] = costos[i][j];
        }
    }
}

// Función para generar costos aleatorios
void generar_costos_aleatorios(int N, int low, int high, int*** costos) {
    *costos = (int**)malloc((N - 1) * sizeof(int*));
    for (int i = 0; i < N - 1; i++) {
        int c = N - i - 1;
        (*costos)[i] = (int*)malloc(c * sizeof(int));
        for (int j = 0; j < c; j++) {
            (*costos)[i][j] = low + rand() % (high - low + 1);
        }
    }
}

// Función para evaluar el costo de una solución
int evaluar_costo(int* solucion, int N, int** matriz_costos) {
    int costo = 0;
    for (int i = 0; i < N - 1; i++) {
        int ciudad = solucion[i];
        int siguiente_ciudad = solucion[i + 1];
        costo += matriz_costos[ciudad][siguiente_ciudad];
    }
    costo += matriz_costos[solucion[N - 1]][solucion[0]];
    return costo;
}

// Función para generar población inicial
Solucion* poblacion_inicial(int N, int tamano_poblacion, int** matriz_costos) {
    Solucion* poblacion = (Solucion*)malloc(tamano_poblacion * sizeof(Solucion));
    int* ciudades = (int*)malloc(N * sizeof(int));
    
    for (int i = 0; i < N; i++) {
        ciudades[i] = i;
    }

    for (int i = 0; i < tamano_poblacion; i++) {
        // Crear una permutación aleatoria
        for (int j = 0; j < N; j++) {
            int k = j + rand() % (N - j);
            int temp = ciudades[j];
            ciudades[j] = ciudades[k];
            ciudades[k] = temp;
        }

        poblacion[i].ruta = (int*)malloc(N * sizeof(int));
        for (int j = 0; j < N; j++) {
            poblacion[i].ruta[j] = ciudades[j];
        }
        poblacion[i].costo = evaluar_costo(poblacion[i].ruta, N, matriz_costos);
    }

    free(ciudades);
    return poblacion;
}

// Función de selección por torneo
void seleccion_torneo(Solucion* poblacion, int tam_pob, int P, Solucion* padre1, Solucion* padre2) {
    // Selección del primer padre
    int mejor_idx = rand() % tam_pob;
    for (int i = 1; i < P; i++) {
        int candidato = rand() % tam_pob;
        if (poblacion[candidato].costo < poblacion[mejor_idx].costo) {
            mejor_idx = candidato;
        }
    }
    *padre1 = poblacion[mejor_idx];

    // Selección del segundo padre (asegurando que sea diferente)
    mejor_idx = rand() % tam_pob;
    for (int i = 1; i < P; i++) {
        int candidato = rand() % tam_pob;
        if (poblacion[candidato].costo < poblacion[mejor_idx].costo) {
            mejor_idx = candidato;
        }
    }
    *padre2 = poblacion[mejor_idx];
}

// Función de cruce PMX
void cruzaPMX(int* ind1, int* ind2, int size, int* hijo1, int* hijo2) {
    // Elegir dos puntos de corte aleatorios diferentes
    int p1, p2;
    do {
        p1 = rand() % size;
        p2 = rand() % size;
    } while (abs(p1 - p2) < 2); // Asegurar una región de al menos 2 elementos
    
    // Ordenar los puntos para que p1 < p2
    if (p1 > p2) {
        int temp = p1;
        p1 = p2;
        p2 = temp;
    }

    // Inicializar hijos con -1
    for (int i = 0; i < size; i++) {
        hijo1[i] = -1;
        hijo2[i] = -1;
    }

    // Copiar la región entre p1 y p2
    for (int i = p1; i < p2; i++) {
        hijo1[i] = ind2[i];
        hijo2[i] = ind1[i];
    }

    // Mapeo para hijo1
    for (int i = 0; i < size; i++) {
        if (i >= p1 && i < p2) continue; // Saltar la región ya copiada

        int valor = ind1[i];
        while (1) {
            int encontrado = 0;
            for (int j = p1; j < p2; j++) {
                if (hijo1[j] == valor) {
                    encontrado = 1;
                    break;
                }
            }
            if (!encontrado) break;
            
            // Buscar el valor en ind2
            int idx = -1;
            for (int j = 0; j < size; j++) {
                if (ind2[j] == valor) {
                    idx = j;
                    break;
                }
            }
            valor = ind1[idx];
        }
        hijo1[i] = valor;
    }

    // Mapeo para hijo2
    for (int i = 0; i < size; i++) {
        if (i >= p1 && i < p2) continue; // Saltar la región ya copiada

        int valor = ind2[i];
        while (1) {
            int encontrado = 0;
            for (int j = p1; j < p2; j++) {
                if (hijo2[j] == valor) {
                    encontrado = 1;
                    break;
                }
            }
            if (!encontrado) break;
            
            // Buscar el valor en ind1
            int idx = -1;
            for (int j = 0; j < size; j++) {
                if (ind1[j] == valor) {
                    idx = j;
                    break;
                }
            }
            valor = ind2[idx];
        }
        hijo2[i] = valor;
    }
}

// Función de mutación
void mutacion(int* camino, int size) {
    int i = rand() % size;
    int j;
    do {
        j = rand() % size;
    } while (j == i);

    int temp = camino[i];
    camino[i] = camino[j];
    camino[j] = temp;
}

// Algoritmo genético principal con impresión por generación
Solucion algoritmo_genetico(int G, int N, int** matriz_costos, int tam_pob, float prob_cruce, float prob_mutacion, int P) {
    // Inicializar población
    Solucion* poblacion = poblacion_inicial(N, tam_pob, matriz_costos);
    Solucion mejor_final;
    mejor_final.costo = INT_MAX;

    printf("Generación | Mejor Costo\n");
    printf("-----------+------------\n");

    for (int gen = 0; gen < G; gen++) {
        Solucion* nueva_pob = (Solucion*)malloc(tam_pob * sizeof(Solucion));
        int nueva_pob_size = 0;

        // Encontrar el mejor de la generación actual
        int mejor_idx = 0;
        for (int i = 1; i < tam_pob; i++) {
            if (poblacion[i].costo < poblacion[mejor_idx].costo) {
                mejor_idx = i;
            }
        }
        Solucion mejor_actual = poblacion[mejor_idx];
        
        // Mostrar información de la generación
        printf("%9d | %10d\n", gen, mejor_actual.costo);

        // Conservar el mejor en la nueva población
        nueva_pob[0] = mejor_actual;
        nueva_pob_size++;

        while (nueva_pob_size < tam_pob) {
            Solucion padre1, padre2;
            seleccion_torneo(poblacion, tam_pob, P, &padre1, &padre2);

            Solucion hijo1, hijo2;
            hijo1.ruta = (int*)malloc(N * sizeof(int));
            hijo2.ruta = (int*)malloc(N * sizeof(int));

            // Cruza
            if ((float)rand() / RAND_MAX < prob_cruce) {
                cruzaPMX(padre1.ruta, padre2.ruta, N, hijo1.ruta, hijo2.ruta);
            } else {
                for (int i = 0; i < N; i++) {
                    hijo1.ruta[i] = padre1.ruta[i];
                    hijo2.ruta[i] = padre2.ruta[i];
                }
            }

            // Mutación
            if ((float)rand() / RAND_MAX < prob_mutacion) {
                mutacion(hijo1.ruta, N);
            }
            if ((float)rand() / RAND_MAX < prob_mutacion) {
                mutacion(hijo2.ruta, N);
            }

            // Evaluar
            hijo1.costo = evaluar_costo(hijo1.ruta, N, matriz_costos);
            hijo2.costo = evaluar_costo(hijo2.ruta, N, matriz_costos);

            // Añadir a la nueva población
            nueva_pob[nueva_pob_size++] = hijo1;
            if (nueva_pob_size < tam_pob) {
                nueva_pob[nueva_pob_size++] = hijo2;
            }
        }

        // Liberar la población anterior
        for (int i = 0; i < tam_pob; i++) {
            if (i >= nueva_pob_size) {
                free(poblacion[i].ruta);
            }
        }
        free(poblacion);
        poblacion = nueva_pob;
    }

    // Encontrar la mejor solución final
    mejor_final = poblacion[0];
    for (int i = 1; i < tam_pob; i++) {
        if (poblacion[i].costo < mejor_final.costo) {
            mejor_final = poblacion[i];
        }
    }

    // Mostrar información final
    printf("\nMejor solución final:\n");
    printf("Costo: %d\n", mejor_final.costo);
    printf("Ruta: ");
    for (int i = 0; i < N; i++) {
        printf("%d ", mejor_final.ruta[i]);
    }
    printf("\n");

    // Liberar memoria
    for (int i = 0; i < tam_pob; i++) {
        if (poblacion[i].ruta != mejor_final.ruta) {
            free(poblacion[i].ruta);
        }
    }
    free(poblacion);

    return mejor_final;
}

// Función principal para probar el algoritmo
int main() {
    srand(time(NULL));

    int N = 20; // Número de ciudades
    int G = 30; // Número de generaciones
    int tam_pob = 100; // Tamaño de la población
    float prob_cruce = 0.9; // Probabilidad de cruce
    float prob_mutacion = 0.1; // Probabilidad de mutación
    int P = 2; // Tamaño del torneo

    // Generar costos aleatorios
    int** costos;
    generar_costos_aleatorios(N, 5, 500, &costos);

    // Construir matriz de costos
    int** matriz = (int**)malloc(N * sizeof(int*));
    for (int i = 0; i < N; i++) {
        matriz[i] = (int*)malloc(N * sizeof(int));
    }
    construir_matriz(N, costos, matriz);

    // Ejecutar algoritmo genético
    Solucion mejor = algoritmo_genetico(G, N, matriz, tam_pob, prob_cruce, prob_mutacion, P);

    // Mostrar resultados
    printf("Mejor ruta encontrada:\n");
    for (int i = 0; i < N; i++) {
        printf("%d ", mejor.ruta[i]);
    }
    printf("\nCosto: %d\n", mejor.costo);

    // Liberar memoria
    for (int i = 0; i < N - 1; i++) {
        free(costos[i]);
    }
    free(costos);
    
    for (int i = 0; i < N; i++) {
        free(matriz[i]);
    }
    free(matriz);
    
    free(mejor.ruta);

    return 0;
}