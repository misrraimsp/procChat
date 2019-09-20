#!/bin/bash

# Compilo los tres códigos fuente
gcc -o Ej1 fuente1.c
gcc -o Ej2 fuente2.c
gcc -o Ej3 fuente3.c

# Aseguro permisos de ejecución a todos los usuarios sobre los ejecutables
chmod +x Ej1
chmod +x Ej2
chmod +x Ej3

# Ejecuto Ej1
./Ej1

# Borro los tres ejecutables
rm Ej1
rm Ej2
rm Ej3
