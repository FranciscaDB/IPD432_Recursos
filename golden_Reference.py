import numpy as np

def escribir_archivo_datos(N, num_iteraciones):
    # Abrir el archivo .dat para escribir
    with open('golden_Reference.dat', 'w') as archivo:
        for _ in range(num_iteraciones):
            # Generar dos vectores aleatorios de tamaño N para cada iteración
            vector1 = np.random.rand(N)
            vector2 = np.random.rand(N)
            
            # Escribir los elementos del primer vector
            for elemento in vector1:
                archivo.write(f"{elemento}\n")
            
            # Escribir los elementos del segundo vector
            for elemento in vector2:
                archivo.write(f"{elemento}\n")
            
            # Calcular y escribir el producto punto entre los dos vectores
            producto_punto = np.dot(vector1, vector2)
            archivo.write(f"{producto_punto}\n")

# Definir el tamaño del vector y el número de iteraciones
N = 5  # Puedes cambiar este valor según lo necesites
num_iteraciones = 5

# Generar y escribir los datos en el archivo
escribir_archivo_datos(N, num_iteraciones)
