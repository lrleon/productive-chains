# Constructor y analizador de redes productivas

Básicamente este directorio contiene códigos fuente y miscélaneos para
la construcción y análisis de cadenas productivas.

## Programas contenidos en esta distribución

1. `test`: intérprete de comandos de un lentguaje sencillo para
   construcción y análisis de cadenas productivas. Pare ejecutarlo
   simplemente tipea:

       ./test

   y el intérprete se ejecutará.

2. `transform-data`: 1ra fase  de conversión de los datos. Esta fase lee
   los archivos en formato csv de la base de datos de sigesic y genera
   los archivos `insumos.txt`, `productos.txt`, `productores.txt`,
   `socios.txt` y `grafo.txt`. Por omisión se genera data para el año
   2012 y se buscan los nombres de las tablas de sigecic. Tipea
   `./transform-data --help` para ver opciones que te permitan modificar
   los nombres de los archivos de entrada y salida así como el año.

3. `transform-dada-2`: 2da fase de conversión de datos. Esta fase lee
   los archivos generados por `transform-data` y genera un único archivo
   con toda la metadata el cual por omisión es llamado `mapa.txt`.


## Requerimientos para compilar

1. Biblioteca `TCLAP`
2. 

   

