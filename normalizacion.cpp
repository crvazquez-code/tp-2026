//
// Created by santi on 9/22/2026.
//

#include <stdio.h>
#include <string.h>

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

void nombresAgrupados(ComandaHistorica array[], int len);
int llenarArrayComandas(ComandaHistorica comandas[]);
void arrayMozos(ComandaHistorica array[], int len, Mozo arrayMozo[], int &lenMozos);
int main() {
    ComandaHistorica comandas[MAX_COMANDAS];
    Mozo mozos[MAX_MOZOS];
    int lenMozos;

    int lenComandas = llenarArrayComandas(comandas);
    if (lenComandas < 0) {
        printf("\n Fallo al leer comandas_historicas.dat \n");

        return 1;
    }

    nombresAgrupados(comandas, lenComandas);
    arrayMozos(comandas, lenComandas, mozos, lenMozos);

    return 0;
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

// Corte de control para crear el array de mozos.
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