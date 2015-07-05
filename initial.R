require("data.table")
#require("lubridate")

ciu.classes <- c("character", "integer", "integer", "character", "character")
ciu <- fread("comun_codigo_aran.csv", colClasses=ciu.classes)
uni <- fread("unidad_economica.csv")

insumos <- read.csv("produccion_insumo.csv")
insumos$nombre <- as.character(insumos$nombre)
insumos$esp_tec<- as.character(insumos$esp_tec)
insumos$codigo_aran_id <- as.character(insumos$codigo_aran_id)
insumos$marca<- as.character(insumos$marca)

productos <- read.csv("produccion_producto.csv")
productos$nombre <- as.character(productos$nombre)
productos$esp_tec<- as.character(productos$esp_tec)
productos$codigo_aran_id <- as.character(productos$codigo_aran_id)
productos$marca<- as.character(productos$marca)

plantas <- fread("unidadecon_subunidad_economica.csv")

proveedores <- fread("cmproveedores_proveedor.csv")
proveedores.ins <- fread("cmproveedores_proveedorinsumo.csv")


arcs <- fread("produccion_producto_t_insumo.csv")

# retorna a data frame ==> pueden haber varios insumos con el mismo id
locate <- function (df, id) {

    idx <- df$id == id
    df[idx, ]
}

locate.insumo <- function (id) {

    locate(insumos, id)
}

locate.producto <- function (id) {

    locate(productos, id)
}

locate.planta <- function(plantaid) {

    plantas[plantas$id == plantaid, ]
}

locate.plantas <- function(provs) {

    ret <- data.frame()
    for (p in provs) {

        pls <- proveedores[proveedores$id == p$proveedor_id_id, ]
        ret <- rbind(ret, pls)
    }

    ret
}

