# Constructor y analizador de redes productivas

B�sicamente este directorio contiene c�digos fuente y misc�laneos para
la construcci�n y an�lisis de cadenas productivas.

## Programas contenidos en esta distribuci�n

1. `test`: int�rprete de comandos de un lentguaje sencillo para
   construcci�n y an�lisis de cadenas productivas. Pare ejecutarlo
   simplemente tipea:

       ./test

   y el int�rprete se ejecutar�.

2. `transform-data`: 1ra fase  de conversi�n de los datos. Esta fase lee
   los archivos en formato csv de la base de datos de sigesic y genera
   los archivos `insumos.txt`, `productos.txt`, `productores.txt`,
   `socios.txt` y `grafo.txt`. Por omisi�n se genera data para el a�o
   2012 y se buscan los nombres de las tablas de sigecic. Tipea
   `./transform-data --help` para ver opciones que te permitan modificar
   los nombres de los archivos de entrada y salida as� como el a�o.

3. `transform-dada-2`: 2da fase de conversi�n de datos. Esta fase lee
   los archivos generados por `transform-data` y genera un �nico archivo
   con toda la metadata el cual por omisi�n es llamado `mapa.txt`.


## Requerimientos para compilar

1. Biblioteca `TCLAP`
2. 

   

