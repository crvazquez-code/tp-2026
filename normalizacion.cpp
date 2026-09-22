//
// Created by santi on 9/21/2026.
//

#include <stdio.h>
#include <string.h>

struct ComandaHistorica {
    char fecha[11]; // "DD-MM-AAAA"
    char nombreMozo[50]; // el nombre completo, repetido en cada venta
    int codigoProducto;
    int cantidad;
    float comision;
};

struct Mozo {
    int idMozo;
    char nombre[50];
    char password[20];
    float totalComision;
};


int main() {

    return 0;
}


int arrayMozos(Mozo mozos[]) {
    FILE *f = fopen("comandas_historicas.dat", "rb");
    ComandaHistorica ch;

    int lenMozos = 0;

    if (f == nullptr) {
        return -1;
    }

    while (fread(&ch, sizeof(ComandaHistorica), 1, f) == 1) {
        int pos = -1;
        int i = 0;

        while (i < lenMozos && pos == -1) {
            // strcmp, si no compara las direcciones de memoria.
            if (strcmp(mozos[i].nombre, ch.nombreMozo) == 0) {
                pos = i;
            }

            i++;
        }

        if (pos == -1) {
            pos = lenMozos;
            strcpy(mozos[pos].nombre, ch.nombreMozo);
            mozos[pos].idMozo = lenMozos;
            mozos[pos].totalComision = 0;

            lenMozos++;
        }

        mozos[pos].totalComision += ch.comision;
    }

    fclose(f); return lenMozos;
}





