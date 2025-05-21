import random
import numpy as np

def construir_matriz(N, costos):
    matriz = np.zeros([N,N])

    for i in range(len(costos)):
        for j in range(len(costos[i])):
            #nos colocamos en la matriza en la fila i
            #tenemos que sumarle i +1 a j para no caer en la diagonal
            #matriz[i][j + i + 1] = costos[i][j] 
            matriz[i][j + i+ 1] = costos[i][j] 
            #como es una matriz simétrica solo se invierten [j + i + 1][i]
            matriz[j + i+ 1][i] = costos[i][j] 

    return matriz

#hacer la entrada con en la práctica
def generar_costos_aleatorios(N, low=5, high=500):
    costos = []
    
    for i in range(N - 1):
        c = N - i - 1
        # generar costos para la ciudad i a las demás ciudades
        lista_costos= np.random.randint(low, high, size=c).tolist()
        costos.append(lista_costos)
    return costos


def evaluar_costo(solucion, N, matriz_costos):

    costo = 0
    for i in range(N-1):
        ciudad = solucion[i] #Me fijo en que ciudad estoy
        #print(f'ciudad en la que estoy {ciudad}')
        siguiente_ciudad= solucion[i+1] #Me fijo a qué ciudad me voy 
        costo += matriz_costos[ciudad][siguiente_ciudad] #reviso el costo en la matriz original 
        #print(matriz[ciudad][siguiente_ciudad], costo)
    #regresar al inicio 
    costo += matriz_costos[solucion[-1]][solucion[0]] 

    return costo



def poblacion_inicial(N, tamano_poblacion, matriz_costos):
    ciudades = np.arange(N)
    poblacion = []
    for _ in range(tamano_poblacion):

        ruta = np.random.permutation(ciudades).tolist()
        costo = evaluar_costo(ruta, N, matriz_costos)
        poblacion.append((ruta, costo))

    return poblacion



def seleccion_torneo(poblacion, P):
    
    competidores1 = random.sample(poblacion, P)
    padre1 = min(competidores1, key=lambda x: x[1])
    
    competidores2 = random.sample(poblacion, P)
    padre2 = min(competidores2, key=lambda x: x[1])
    
    return padre1, padre2


def cruzaPMX(ind1, ind2):
    size = len(ind1)
    
    # 1. Elegir puntos de corte (2 puntos aleatorios)
    p1, p2 = sorted(random.sample(range(size), 2))
    
    #print(f'{p1,p2}')
    
    # 2. Inicializar hijos con None
    hijo1 = [None] * size
    hijo2 = [None] * size
    
    # 3. Copiar la región entre p1 y p2 de los padres al hijo opuesto
    hijo1[p1:p2] = ind2[p1:p2]
    hijo2[p1:p2] = ind1[p1:p2]
    
    # mapeo desde 0 hasta el primer punto
    for i in range(0, p1):
        # Para hijo1
        valor = ind1[i]
        while valor in hijo1[p1:p2]:  # si se encuentra en el segmento cambiado 
            idx = ind2.index(valor)   # obtenemos el índice donde se encuentra ese valor por el que fue cambiado
            valor = ind1[idx]
        hijo1[i] = valor    # actualizamos el valor en el hijo
        
        # Para hijo2
        valor = ind2[i]
        while valor in hijo2[p1:p2]:  #  si se encuentra en el segmento cambiado
            idx = ind1.index(valor)   # obtenemos el índice donde se encuentra ese valor por el que fue cambiado
            valor = ind2[idx]
        hijo2[i] = valor

    # mapeo desde p2 hasta la longitud del individuo 

    for i in range(p2, size):
        # Para hijo1
        valor = ind1[i]
        while valor in hijo1[p1:p2]:  # si se encuentra en el segmento cambiado 
            idx = ind2.index(valor)   # obtenemos el índice donde se encuentra ese valor por el que fue cambiado
            valor = ind1[idx]
        hijo1[i] = valor    # actualizamos el valor en el hijo
        
        # Para hijo2
        valor = ind2[i]
        while valor in hijo2[p1:p2]:  #  si se encuentra en el segmento cambiado
            idx = ind1.index(valor)   # obtenemos el índice donde se encuentra ese valor por el que fue cambiado
            valor = ind2[idx]
        hijo2[i] = valor

    
    return hijo1, hijo2



def mutacion(camino):
    '''
    Mutación para permutaciones: Por intercambio recíproco
    En este caso, se seleccionan dos posiciones al azar y se
    intercambian sus valores:
    '''
    # Elegir posiciones aleatorias 
    i, j = random.sample(range(len(camino)), 2)
    new_ind = camino.copy()
    new_ind[i], new_ind[j] = camino[j], camino[i]
    return new_ind



def algoritmo_genetico(G, N, matriz_costos, tam_pob, prob_cruce, prob_mutacion, P):
    # 1) Inicializar y evaluar población
    poblacion = poblacion_inicial(N, tam_pob, matriz_costos)
  
    for gen in range(G):
        nueva_pob = []

        #conservar el mejor de la generacion
        mejor_actual = min(poblacion, key=lambda x: x[1])
        nueva_pob.append(mejor_actual)
    

        while len(nueva_pob) < tam_pob:
           
            padre1, padre2 = seleccion_torneo(poblacion, P)

            # Cruza PMX
            if random.random() < prob_cruce:
                hijo1, hijo2 = cruzaPMX(padre1[0], padre2[0])
            else:
                hijo1, hijo2 = padre1[0].copy(), padre2[0].copy()

            # Mutación
            if random.random() < prob_mutacion:
                hijo1 = mutacion(hijo1)
            if random.random() < prob_mutacion:
               hijo2 = mutacion(hijo2)

            # Evaluar
            costo1 = evaluar_costo(hijo1, N, matriz_costos)
            costo2 = evaluar_costo(hijo2, N, matriz_costos)

            # Añadir
            nueva_pob.append((hijo1, costo1))
            if len(nueva_pob) < tam_pob:
                nueva_pob.append((hijo2, costo2))

        # Actualizar
        poblacion = nueva_pob

        #mejor = min(poblacion, key=lambda x: x[1])
        #print(f"Generación {gen} — Mejor costo: {mejor[1]}")

    


    mejor_final = min(poblacion, key=lambda x: x[1])
    
    return mejor_final



