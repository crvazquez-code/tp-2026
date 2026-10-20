#include <iostream>
#include <cstdio>
#include <cerrno>
#include <cstring>
#include "funciones/funciones.h"

#define RUTA "./datos_de_uso/"
using namespace std;

void nombrePlanillaDia(int dia, int mes, int anio, char nombre[])
{
    string diaTexto = to_string(dia);
    string mesTexto = to_string(mes);
    string anioTexto = to_string(anio);
    if (dia < 10)
        diaTexto = "0" + diaTexto;

    if (mes < 10)
        mesTexto = "0" + mesTexto;

    while (anioTexto.length() < 4)
        anioTexto = "0" + anioTexto;

    string nombreCompleto = string(RUTA) + "comandas_" + diaTexto + "-" + mesTexto + "-" + anioTexto + ".dat";

    strcpy(nombre, nombreCompleto.c_str());
}

int diasDelMes(int mes, int anio)
{
    int dias[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    bool bisiesto = anio % 400 == 0 || (anio % 4 == 0 && anio % 100 != 0);
    if (mes == 2 && bisiesto)
    {
        return 29;
    }
    return dias[mes - 1];
}

// Devuelve 1 si se leyo el dia, 0 si no existe y -1 si hubo un error.
int cargarDia(const char nombreArchivo[], Comanda *&comandas, int &len)
{
    comandas = nullptr;
    len = 0;
    FILE *archivo = fopen(nombreArchivo, "rb");
    if (archivo == nullptr)
    {
        if (errno == ENOENT)
        {
            return 0; // En caso de que no se encuentre
        }
        mostradorDetexto('e', "No se pudo abrir " + string(nombreArchivo) + ".");
        return -1;
    }

    int capacidad = 16;
    comandas = new Comanda[capacidad];
    Comanda comanda;
    bool valido = true;
    size_t bytesLeidos;
    // Leemos un registro por vez. La cantidad de bytes permite detectar uno incompleto.
    while ((bytesLeidos = fread(&comanda, 1, sizeof(Comanda), archivo)) == sizeof(Comanda))
    {
        if (len > 0 && comandas[len - 1].idMozo > comanda.idMozo)
        {
            valido = false;
            break;
        }
        if (len == capacidad)
        {
            // Duplicamos el espacio solo cuando el arreglo se llena.
            int nuevaCapacidad = capacidad * 2;
            Comanda *ampliadas = new Comanda[nuevaCapacidad];
            for (int i = 0; i < len; i++)
            {
                ampliadas[i] = comandas[i];
            }
            delete[] comandas;
            comandas = ampliadas;
            capacidad = nuevaCapacidad;
        }
        comandas[len++] = comanda;
    }
    bool errorLectura = ferror(archivo) != 0;
    fclose(archivo);

    if (!valido || errorLectura || bytesLeidos != 0)
    {
        mostradorDetexto('e', "No se pudo leer" + string(nombreArchivo) + ".");

        delete[] comandas;
        comandas = nullptr;
        len = 0;
        return -1;
    }
    return 1;
}

bool guardarSemana(const char nombreArchivo[], const Comanda comandas[], int len)
{
    FILE *archivo = fopen(nombreArchivo, "wb");
    if (archivo == nullptr)
    {
        mostradorDetexto('e', "No se pudo crear" + string(nombreArchivo) + ".");
        return false;
    }

    bool guardado = true;
    for (int i = 0; i < len && guardado; i++)
    {
        guardado = fwrite(&comandas[i], sizeof(Comanda), 1, archivo) == 1;
    }
    if (fclose(archivo) != 0)
    {
        guardado = false;
    }
    if (!guardado)
    {
        mostradorDetexto('e', "No se pudo completar la escritura de " + string(nombreArchivo) + ".");
    }
    return guardado;
}

int main()
{
    int semana, mes, anio;
    mostradorDetexto('t', "=== Cierre semanal ===");
    mostradorDetexto('t', "Semanas: 1 = dias 1-7, 2 = 8-14, 3 = 15-21, 4 = 22-28, 5 = 29-fin. ");
    mostradorDetexto('t', "Ingrese el numero de semana: ");

    if (!(cin >> semana))
    {
        mostradorDetexto('e', "Semana invalida.");
        return 1;
    }

    mostradorDetexto('t', "Ingrese el numero de mes: ");
    if (!(cin >> mes))
    {
        mostradorDetexto('e', "Mes invalido.");
        return 1;
    }

    mostradorDetexto('t', "Ingrese el año: ");
    if (!(cin >> anio) || semana < 1 || semana > 5 ||
        mes < 1 || mes > 12 || anio < 1 || anio > 9999)
    {
        mostradorDetexto('e', " Semana, mes o año invalido.");
        return 1;
    }

    int primerDia = (semana - 1) * 7 + 1;
    int ultimoDia = semana * 7;
    int finMes = diasDelMes(mes, anio);
    if (ultimoDia > finMes)
    {
        ultimoDia = finMes;
    }
    if (primerDia > ultimoDia)
    {
        mostradorDetexto('e', "Esa semana no tiene dias en el mes elegido.");
        return 1;
    }

    Comanda *comandasSemana = new Comanda[1];
    int lenSemana = 0;
    int diasEncontrados = 0;

    for (int dia = primerDia; dia <= ultimoDia; dia++)
    {
        // Intentamos leer cada dia; cargarDia devuelve 0 si el archivo no existe.
        char nombreDia[80];
        nombrePlanillaDia(dia, mes, anio, nombreDia);
        Comanda *comandasDia;
        int lenDia;
        int resultado = cargarDia(nombreDia, comandasDia, lenDia);
        if (resultado < 0)
        {
            delete[] comandasSemana;
            return 1;
        }
        if (resultado == 0)
        {
            continue;
        }
        diasEncontrados++;
        // Ambos arreglos ya estan ordenados: el apareo conserva ese orden.
        Comanda *combinadas = new Comanda[lenSemana + lenDia];
        int lenCombinadas;
        apareoGenerico(comandasSemana, lenSemana, comandasDia, lenDia,
                       combinadas, lenCombinadas, [](const Comanda &a, const Comanda &b)
                       { return a.idMozo - b.idMozo; });

        delete[] comandasSemana;
        delete[] comandasDia;
        comandasSemana = combinadas;
        lenSemana = lenCombinadas;
    }

    if (diasEncontrados == 0)
    {
        mostradorDetexto('e', "No se encontraron planillas para esa semana.");
        delete[] comandasSemana;
        return 1;
    }

    string mesTexto = to_string(mes);
    if (mes < 10)
    {
        mesTexto = "0" + mesTexto;
    }

    string nombreSemana = string(RUTA) + "comandas_semana_s" +
                          to_string(semana) + "-" + mesTexto + ".dat";

    bool guardado = guardarSemana(nombreSemana.c_str(), comandasSemana, lenSemana);
    delete[] comandasSemana;

    if (!guardado)
    {
        return 1;
    }
    if (!guardado)
    {
        return 1;
    }

    mostradorDetexto('t', "Cierre generado: " + string(nombreSemana));
    mostradorDetexto('t', "Dias incluidos: " + to_string(diasEncontrados) + ". Comandas: " + to_string(lenSemana) + ".");
    return 0;
}
