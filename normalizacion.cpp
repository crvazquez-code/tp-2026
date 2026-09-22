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
    int codigo;
    char nombreMozo[50];
    char contrasenia[20];
    float totalComision;
};


int archivoMozos();

int main() {
    archivoMozos();

    return 0;
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


int archivoMozos() {
    Mozo mozos[100];
    int lenMozos = arrayMozos(mozos);

    if (lenMozos < 0) {
        return -1;
    }

    // Mostrar los mozos
    //printf("\n===== MOZOS =====\n");

    //for (int i = 0; i < lenMozos; i++) {
    //    printf("Mozo %d\n", i);
    //    printf("ID: %d\n", mozos[i].codigo);
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

