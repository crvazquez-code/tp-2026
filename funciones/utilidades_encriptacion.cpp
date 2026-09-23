
void intAChar(int numero, char texto[]) {
    int i = 0;

    if (numero == 0) {
        texto[0] = '0';
        texto[1] = '\0';
        return;
    }

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

// shift de +5 (osea K = 5)
void encriptado(char contrasenia[]) {
    for (int i = 0; contrasenia[i] != '\0'; i++) {
        contrasenia[i] += 5;
    }
}