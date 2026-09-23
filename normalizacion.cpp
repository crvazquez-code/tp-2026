//
// Created by santi on 9/22/2026.
//

#include <stdio.h>
#include <string.h>
#include "funciones/funciones.cpp"

#define RUTA_DATOS "../datos/"
#define MAX_MOZOS 100
#define MAX_COMANDAS 500

// -- por defecto --
struct ComandaHistorica {
    char fecha[11];
    char nombreMozo[50];
    int codigoProducto;
    int cantidad;
    float comision;
};

struct Producto {
    int codigo;
    char descripcion[50];
    float precio;
    int stockActual;
};

// -- propios --
struct Mozo {
    int codigo;
    char nombreMozo[50];
    char contrasenia[20];
    float totalComision;
};

struct Comanda {
    char fecha[11];
    int codMozo;
    int codProd;
    int cant;
    float comision;
};

// funciones/utils.cpp
void intAChar(int numero, char texto[]);
void encriptado(char contrasenia[]);

int archivoMozos(Mozo mozos[], int lenMozos);
void nombresAgrupados(ComandaHistorica array[], int len);
int llenarArrayComandas(ComandaHistorica comandas[]);
void arrayMozos(ComandaHistorica array[], int len, Mozo arrayMozo[], int &lenMozos);
void arrayComandas(ComandaHistorica array[], int len, Mozo mozos[], int lenMozos, Comanda arrayComanda[]);
void agruparPorFecha(ComandaHistorica array[], int len);
int archivosComandas(Comanda comandas[], int lenComandas);
int main() {
    ComandaHistorica comandas[MAX_COMANDAS];
    Mozo mozos[MAX_MOZOS];
    int lenMozos;
    Comanda comandasNuevas[MAX_COMANDAS];

    int lenComandas = llenarArrayComandas(comandas);
    if (lenComandas < 0) {
        printf("\n Fallo al leer comandas_historicas.dat \n");

        return 1;
    }

    nombresAgrupados(comandas, lenComandas);
    arrayMozos(comandas, lenComandas, mozos, lenMozos);

    if (archivoMozos(mozos, lenMozos) < 0) {
        printf("\n Fallo al escribir mozos.dat \n");

        return 1;
    }

    agruparPorFecha(comandas, lenComandas);
    arrayComandas(comandas, lenComandas, mozos, lenMozos, comandasNuevas);

    if (archivosComandas(comandasNuevas, lenComandas) < 0) {
        printf("\n Fallo al escribir los archivos de comandas por dia \n");

        return 1;
    }

    return 0;
}

// agrupar por fecha.
void agruparPorFecha(ComandaHistorica array[], int len) {
    for (int i = 0; i < len; i++) {
        for (int j = i + 1; j < len; j++) {
            if (strcmp(array[i].fecha, array[j].fecha) == 0) {
                i++;

                ComandaHistorica temp = array[i];
                array[i] = array[j];
                array[j] = temp;
            }
        }
    }
}

// Agrupar por nombre.
void nombresAgrupados(ComandaHistorica array[], int len) {
    for (int i = 0; i < len; i++) {
        for (int j = i + 1; j < len; j++) {
            if (strcmp(array[i].nombreMozo, array[j].nombreMozo) == 0) {
                i++;

                ComandaHistorica temp = array[i];
                array[i] = array[j];
                array[j] = temp;
            }
        }
    }
}

// Cargar el archivo comandas historicas en un array.
int llenarArrayComandas(ComandaHistorica comandas[]) {
    FILE *f = fopen(RUTA_DATOS "comandas_historicas.dat", "rb");
    int len = 0;

    if (f == nullptr) {
        return -1;
    }

    while (len < MAX_COMANDAS && fread(&comandas[len], sizeof(ComandaHistorica), 1, f) == 1) {
        len++;
    }

    fclose(f); return len;
}

// Corte de control por nombre para crear el array de mozos.
void arrayMozos(ComandaHistorica array[], int len, Mozo arrayMozo[], int &lenMozos) {
    lenMozos = 0;
    int i = 0;

    while (i < len) {
        char *control = array[i].nombreMozo;
        float totalComision = 0;

        while (i < len && strcmp(array[i].nombreMozo, control) == 0) {
            totalComision += array[i].comision;
            i++;
        }

        strcpy(arrayMozo[lenMozos].nombreMozo, control);
        arrayMozo[lenMozos].totalComision = totalComision;
        arrayMozo[lenMozos].codigo = lenMozos + 1;

        intAChar(arrayMozo[lenMozos].codigo, arrayMozo[lenMozos].contrasenia);
        encriptado(arrayMozo[lenMozos].contrasenia);

        lenMozos++;
    }
}

// Pasa cada ComandaHistorica a Comanda.
void arrayComandas(ComandaHistorica array[], int len, Mozo mozos[], int lenMozos, Comanda arrayComanda[]) {
    for (int i = 0; i < len; i++) {

        int j = 0;
        while (j < lenMozos && strcmp(mozos[j].nombreMozo, array[i].nombreMozo) != 0) {
            j++;
        }

        strcpy(arrayComanda[i].fecha, array[i].fecha);
        arrayComanda[i].codMozo = mozos[j].codigo;
        arrayComanda[i].codProd = array[i].codigoProducto;
        arrayComanda[i].cant = array[i].cantidad;
        arrayComanda[i].comision = array[i].comision;
    }
}

// crea el archivo mozos.dat final.
int archivoMozos(Mozo mozos[], int lenMozos) {

    FILE *f = fopen(RUTA_DATOS "mozos.dat", "wb");

    if (f == nullptr) {
        return -1;
    }

    int escritos = fwrite(mozos, sizeof(Mozo), lenMozos, f);
    int errorCierre = fclose(f);

    if (escritos != lenMozos || errorCierre != 0) {
        return -1;
    }

    return lenMozos;
}

// Crea archivos usando corte de control por fecha, los ordena antes de escribir.
int archivosComandas(Comanda comandas[], int lenComandas) {
    int i = 0;

    while (i < lenComandas) {
        char *control = comandas[i].fecha;
        Comanda dia[MAX_COMANDAS];
        int lenDia = 0;

        char nombre[40];
        sprintf(nombre, RUTA_DATOS "comandas_%s.dat", control);

        FILE *f = fopen(nombre, "wb");

        if (f == nullptr) {
            return -1;
        }

        while (i < lenComandas && strcmp(comandas[i].fecha, control) == 0) {
            dia[lenDia] = comandas[i];
            lenDia++;
            i++;
        }

        ordBurbujaGenerico(dia, lenDia, [](const Comanda &c) { return c.codMozo; });
        fwrite(dia, sizeof(Comanda), lenDia, f);

        if (fclose(f) != 0) {
            return -1;
        }
    }

    return 0;
}



