## “Resolución eficiente del Problema del Viajero mediante Algoritmos Genéticos y Paralelización MPI”

El siguiente proyecto corresponde a la materia de cómputo de alto rendimiento de la UNAM campus Morelia.

## Contacto:

* Hannia Ashley Alvarado Galván

## Introducción 
La programación evolutiva es considerada uno de los principales paradigmas de la computación evolutiva. Su principal componente es la adaptación

Metodología de la PE:

"PE crea una población inicial, para esto genera aleatoriamente μ individuos. Cada individuo es representado por un arreglo de valores flotantes de tamaño 2n. Las primeras n componentes corresponden a las n variables
del problema y las siguientes n componentes corresponden a los tamaños de paso para la mutación de cada una de las variables del problema. Posteriormente, se aplica el operador de mutación a cada invidividuo para generar μ individuos nuevos (hijos). Esto es porque PE considera que todos los individuos de la población son padres. Finalmente, PE calcula la aptitud de cada individuo, basándose en la función que se está optimizando, y realiza la selección de sobrevivientes."


## Pseudocódigo

<img width="733" alt="image" src="https://github.com/user-attachments/assets/b18ed373-2cb7-4ac7-8a72-4f748b6f6f80" />

## Objetivo general:

Implementar un algoritmo de PE paralelizado para optimizar la función de Beale.

<img width="657" alt="image" src="https://github.com/user-attachments/assets/de32faf8-514b-4351-be3e-b84f94ea4dee" />
