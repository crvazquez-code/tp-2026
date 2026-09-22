//
// Created by santi on 9/21/2026.
//

#include <stdio.h>
#include <string.h>

// -- por defecto --
struct ComandaHistorica {
    char fecha[11]; // "DD-MM-AAAA"
    char nombreMozo[50]; // el nombre completo, repetido en cada venta
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



int arrayMozos(Mozo mozos[]);
int arrayComandas(Comanda comandas[], Mozo mozos[], int lenMozos);
int archivoMozos();
int archivosComandas(Comanda comandas[], int lenComandas);
void ordenamientoBurbuja(Comanda comandas[], int lenComandas);
bool prunarInventario(Comanda comandas[], int lenComandas);

int main() {
    Mozo mozos[100];
    Comanda comandas[500];

    int lenMozos = arrayMozos(mozos);

    if (lenMozos < 0) {
        printf("\n Fallo al generar mozos.dat, no se puede leer comandas_historicas.dat \n");

        return 1;
    }

    int lenComandas = arrayComandas(comandas, mozos, lenMozos);

    archivoMozos();
    ordenamientoBurbuja(comandas, lenComandas);
    archivosComandas(comandas, lenComandas);

    bool resultado = prunarInventario(comandas, lenComandas);

    if (!resultado) {
        printf("\n No se puede actualizar inventario.dat debido a que una comanda es mayor al inventario disponible. \n");

        return 1;
    }

    return 0;
}


void ordenamientoBurbuja(Comanda comandas[], int lenComandas) {
    for (int i = 0; i < lenComandas - 1; i++) {
        for (int j = 0; j < lenComandas - 1 - i; j++) {
            if (comandas[j].codMozo > comandas[j + 1].codMozo) {
                Comanda aux = comandas[j];
                comandas[j] = comandas[j + 1];
                comandas[j + 1] = aux;
            }
        }
    }
}

void encriptado(char contrasenia[]) {
    // Hace un shift de +5 (osea K = 5), '\0' es el final de una cadena de chars.
    // Recordar que la suma entre un char y un int hace casting a char pero la suma
    // es igual a una suma de ints.
    for (int i = 0; contrasenia[i] != '\0'; i++) {
        contrasenia[i] += 5;
    }
}
void intAChar(int numero, char texto[]) {
    int i = 0;

    if (numero == 0) {
        texto[0] = '0';
        texto[1] = '\0'; // El \0 indica el final del array de caracteres
        return;
    }

    // Nota:
    // Numero: 32
    // Ultimo digito: 2
    // texto[0] = '0' + 2 = '2'
    // Numero: 3
    // --- Segunda iteracion ---
    // Ultimo digito: 3
    // texto[1] = '0' + 3 = '3'
    // numero = 0
    // --- numero > 0 = F, corta bucle, texto quedo: '23' ---
    // Lo invertimos, queda '32'

    while (numero > 0) {
        // del int sacamos el ultimo digito con % 10.
        int digito = numero % 10;

        // char + int lo va a convertir a char.
        texto[i] = '0' + digito;

        // del int eliminamos el ultimo digito con la division por 10.
        numero = numero / 10;

        i++;
    }

    texto[i] = '\0';

    // invertir el array
    for (int j = 0; j < i / 2; j++) {
        char aux = texto[j];
        texto[j] = texto[i - 1 - j];
        texto[i - 1 - j] = aux;
    }
}

int arrayMozos(Mozo mozos[]) {
    FILE *f = fopen("../datos/comandas_historicas.dat", "rb");
    ComandaHistorica ch;

    int lenMozos = 0;

    if (f == nullptr) {
        return -1;
    }

    while (fread(&ch, sizeof(ComandaHistorica), 1, f) == 1) {
        int pos = -1;
        int i = 0;

        while (i < lenMozos && pos == -1) {
            // strcmp, si no compara las direcciones de memoria (libreria standard).
            if (strcmp(mozos[i].nombreMozo, ch.nombreMozo) == 0) {
                pos = i;
            }

            i++;
        }

        if (pos == -1) {
            pos = lenMozos;
            strcpy(mozos[pos].nombreMozo, ch.nombreMozo);
            mozos[pos].codigo = lenMozos;
            mozos[pos].totalComision = 0;

            intAChar(mozos[pos].codigo, mozos[pos].contrasenia);
            encriptado(mozos[pos].contrasenia);

            lenMozos++;
        }

        mozos[pos].totalComision += ch.comision;
    }

    fclose(f); return lenMozos;
}
int arrayComandas(Comanda comandas[], Mozo mozos[], int lenMozos) {
    FILE *f = fopen("../datos/comandas_historicas.dat", "rb");
    ComandaHistorica ch;

    int lenComandas = 0;

    if (f == nullptr) {
        return -1;
    }

    while (fread(&ch, sizeof(ComandaHistorica), 1, f) == 1) {
        int pos = -1;
        int i = 0;

        while (i < lenMozos && pos == -1) {
            if (strcmp(mozos[i].nombreMozo, ch.nombreMozo) == 0) {
                pos = i;
            }

            i++;
        }

        if (pos != -1) {
            strcpy(comandas[lenComandas].fecha, ch.fecha);
            comandas[lenComandas].codMozo = mozos[pos].codigo;
            comandas[lenComandas].comision = ch.comision;
            comandas[lenComandas].cant = ch.cantidad;
            comandas[lenComandas].codProd = ch.codigoProducto;

            lenComandas++;
        }
    }

    fclose(f); return lenComandas;
}

int archivoMozos() {
    Mozo mozos[100];
    int lenMozos = arrayMozos(mozos);

    if (lenMozos < 0) {
        return -1;
    }

    //printf("\n===== MOZOS =====\n");

    //for (int i = 0; i < lenMozos; i++) {
    //    printf("Mozo %d\n", i);
    //    printf("Codigo: %d\n", mozos[i].codigo);
    //    printf("Nombre: %s\n", mozos[i].nombreMozo);
    //    printf("Contrasenia encriptada: %s\n", mozos[i].contrasenia);
    //    printf("Comision total: %.2f\n", mozos[i].totalComision);
    //    printf("-------------------------\n");
    //}


    FILE *f = fopen("../datos/mozos.dat", "wb");
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
int archivosComandas(Comanda comandas[], int lenComandas) {
    int archivos = 0;

    for (int i = 0; i < lenComandas; i++) {
        int pos = -1;
        int j = 0;

        while (j < i && pos == -1) {
            if (strcmp(comandas[j].fecha, comandas[i].fecha) == 0) {
                pos = j;
            }

            j++;
        }

        if (pos == -1) {
            char nombre[40];
            sprintf(nombre, "../datos/comandas_%s.dat", comandas[i].fecha);

            FILE *f = fopen(nombre, "wb");

            if (f == nullptr) {
                return -1;
            }

            for (int k = i; k < lenComandas; k++) {
                if (strcmp(comandas[k].fecha, comandas[i].fecha) == 0) {
                    fwrite(&comandas[k], sizeof(Comanda), 1, f);
                }
            }

            if (fclose(f) != 0) {
                return -1;
            }

            archivos++;
        }
    }

    return archivos;
}

bool prunarInventario(Comanda comandas[], int lenComandas) {
    FILE *f = fopen("../datos/inventario.dat", "r+b");
    Producto prod;

    if (f == nullptr) {
        return false;
    }


    while (fread(&prod, sizeof(Producto), 1, f) == 1) {
        for (int i = 0; i < lenComandas; i++) {
            if (prod.codigo == comandas[i].codProd) {
                if (prod.stockActual >= comandas[i].cant) {
                    prod.stockActual -= comandas[i].cant;

                    fseek(f, -(long) sizeof(Producto), SEEK_CUR);
                    fwrite(&prod, sizeof(Producto), 1, f);

                    fseek(f, 0, SEEK_CUR);
                } else {
                    fclose(f);

                    return false;
                }
            }
        }
    }

    fclose(f); return true;
}
