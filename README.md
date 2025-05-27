## Resolución del Problema del Viajero mediante Algoritmos Genéticos y Paralelización MPI

El siguiente proyecto corresponde a la materia de cómputo de alto rendimiento de la UNAM campus Morelia.

## Contacto:

* Hannia Ashley Alvarado Galván

## Introducción 

Los algoritmos genéticos (AG) son técnicas de búsqueda y optimización inspiradas en los procesos de evolución natural. Se apoyan en la idea de que, al igual que en la biología, poblaciones de “individuos” (posibles soluciones) pueden mejorar generación tras generación mediante mecanismos análogos a la selección natural, el cruce (reproducción) y la mutación.

## Objetivo general:

Desarrollar e implementar un algoritmo genético paralelo, capaz de resolver el problema del viajero y maximizando la calidad de las soluciones.

El algoritmo emplea cruce PMX (Partially-Mapped Crossover) para combinar pares de rutas y, durante la fase de mutación, selecciona al azar dos posiciones dentro de la solución y las intercambia, preservando la validez del recorrido mientras introduce diversidad en la población.

### Ejemplo de cruza: 
<img width="635" alt="image" src="https://github.com/user-attachments/assets/bd051333-3ad0-45a5-99e4-0db02e5998c1" />
<img width="637" alt="image" src="https://github.com/user-attachments/assets/d1c7f102-5176-427b-bf39-79d14e2ff564" />

En este proyecto se emplea el patrón maestro-esclavo para paralelizar el algoritmo genético: en cada generación se divide la población global en bloques contiguos que asigna a los demás procesos. Cada esclavo recibe su subpoblación, crea la descendencia correspondiente, los evalúa , hace la cruza o mutación si es que corresponde, y, al terminar, envía de vuelta ese bloque de individuos al maestro. Con los resultados de todos los esclavos, el maestro recompone la población completa y pasa a la siguiente generación. 

## Tiempo de ejecución de acuerdo al número de hilos.
<img width="690" alt="image" src="https://github.com/user-attachments/assets/9192d15d-b69d-4da0-9ae1-1f305fbb677e" />

## Ejemplo de solución 
<img width="1295" alt="image" src="https://github.com/user-attachments/assets/3066179b-d2d6-41a0-90a3-fd7c34080e0c" />

## Evolución
<img width="303" alt="image" src="https://github.com/user-attachments/assets/09ff923c-a3e1-4a42-9542-5ebfa3fbca5c" />
<img width="966" alt="image" src="https://github.com/user-attachments/assets/ce5ff64c-0722-4fe8-bc63-bd94beb719b4" />
