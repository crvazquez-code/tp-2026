#pragma once

#include <iostream>
#include <cstdio>
#include <string>

using namespace std;

// STRUCT
//  -- Por defecto / lectura original --
struct ComandaHistorica
{
    char fecha[11];
    char nombreMozo[50];
    int codigoProducto;
    int cantidad;
    float comision;
};

struct Producto
{
    int codigo;
    char descripcion[50];
    float precio;
    int stockActual;
};

// -- Propios del sistema (acorde al enunciado) --
struct Mozo
{
    int idMozo;
    char nombre[50];
    char password[20];
    float totalComision;
};

// Se remueve la fecha según especificación del TP
struct Comanda
{
    int idMozo;
    int codigoProducto;
    int cantidad;
    float comision;
};

// =========== APAREO ===========
template <typename T1, typename T2, typename TOut, typename Comp>
void apareoGenerico(T1 arrA[], int lenA, T2 arrB[], int lenB, TOut arrC[], int &lenC, Comp comp)
{
    int i = 0, j = 0;
    lenC = 0;
    while (i < lenA && j < lenB)
    {
        int relacion = comp(arrA[i], arrB[j]);
        if (relacion <= 0)
        {
            arrC[lenC++] = arrA[i++];
        }
        else
        {
            arrC[lenC++] = arrB[j++];
        }
    }

    while (i < lenA)
    {
        arrC[lenC++] = arrA[i++];
    }

    while (j < lenB)
    {
        arrC[lenC++] = arrB[j++];
    }
}

// ======= CORTE CONTROL =======
template <typename T, typename ClaveCorte, typename ValProcRegistro, typename ProcesarReg, typename OperacionFin>
void corteControlGenerico(T arr[], int lenA, ClaveCorte claveCorte, ValProcRegistro valProcRegistro, ProcesarReg procRegistro, OperacionFin operacionFinCorte)
{
    int i = 0;

    while (i < lenA)
    {
        int corte = claveCorte(i, arr, lenA);
        auto estructura = valProcRegistro(arr, i, corte);
        int inicioCorte = i;

        while (i < lenA && corte == claveCorte(i, arr, lenA))
        {
            procRegistro(estructura, arr[i]);
            i++;
        }
        operacionFinCorte(arr, inicioCorte, i);
    }
}

// ====== ORDENAMIENTO ======
template <typename T, typename ClaveOrden>
void ordSeleccionGenerico(T arr[], int len, ClaveOrden claveOrden)
{
    for (int i = 0; i < len; i++)
    {
        auto claveMin = claveOrden(arr[i]);
        int posMin = i;
        for (int j = i + 1; j < len; j++)
        {
            if (claveOrden(arr[j]) < claveMin)
            {
                claveMin = claveOrden(arr[j]);
                posMin = j;
            }
        }
        if (posMin != i)
        {
            auto temp = arr[i];
            arr[i] = arr[posMin];
            arr[posMin] = temp;
        }
    }
}

template <typename T, typename ClaveOrden>
void ordBurbujaGenerico(T arr[], int len, ClaveOrden claveOrden)
{
    for (int i = 0; i < len - 1; i++)
    {
        for (int j = 0; j < len - i - 1; j++)
        {
            if (claveOrden(arr[j]) > claveOrden(arr[j + 1]))
            {
                auto temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

template <typename T, typename ClaveOrden>
void ordInsercionGenerico(T arr[], int len, ClaveOrden claveOrden)
{
    for (int i = 1; i < len; i++)
    {
        auto key = arr[i];
        int j = i - 1;
        while (j >= 0 && claveOrden(arr[j]) > claveOrden(key))
        {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

// ====== BUSQUEDA ======

template <typename T, typename ClaveBusqueda, typename Comparador>
int busquedaSecuencialGenerico(T arr[], int len, ClaveBusqueda claveBusqueda, Comparador comparador)
{
    for (int i = 0; i < len; i++)
    {
        if (comparador(claveBusqueda, arr[i]) == 0)
        {
            return i;
        }
    }
    return -1;
}

template <typename T, typename ClaveBusqueda, typename Comparador>

int busquedaBinariaGenerico(T arr[], int len, ClaveBusqueda claveBusqueda, Comparador comparador)

{

    int left = 0;

    int right = len - 1;

    while (left <= right)

    {

        int mid = left + (right - left) / 2;

        int compResult = comparador(claveBusqueda, arr[mid]);

        if (compResult == 0)

        {

            return mid;
        }

        else if (compResult < 0)

        {

            right = mid - 1;
        }

        else

        {

            left = mid + 1;
        }
    }

    return -1;
}

// ====== ARCHIVOS ======

template <typename T>
long obtenerCantidadRegistros(string nombreArchivo)
{
    FILE *f = fopen(nombreArchivo.c_str(), "rb"); // Modo lectura binaria
    if (f == NULL)
    {
        return -1;
    }

    fseek(f, 0, SEEK_END);
    long tamanoTotal = ftell(f);
    fclose(f);

    return tamanoTotal / sizeof(T);
}

template <typename T>
T *leerArchivoGenerico(string nombreArchivo, int largoArchivo)
{
    FILE *archivo = fopen(nombreArchivo.c_str(), "rb");

    if (archivo == NULL)
    {
        cout << "[Warn]: No se encontro un archivo" << endl;
        return NULL;
    }

    T *archivoRam = new T[largoArchivo];

    fseek(archivo, 0, SEEK_SET);

    int i = 0;

    fread(archivoRam, sizeof(T), largoArchivo, archivo);

    fclose(archivo);

    return archivoRam;
}

template <typename T>
bool reemplazarDataArchivoGenerico(string nombreArchivo, T arr[], int len)
{
    // abro el archivo en modo escritura binaria
    FILE *archivo = fopen(nombreArchivo.c_str(), "wb");

    if (archivo == NULL)
    {
        cout << "[Error]: No se pudo abrir/crear el archivo para escritura." << endl;
        return false;
    }

    // Escribe todo el arreglo de una sola vez
    size_t elementosEscritos = fwrite(arr, sizeof(T), len, archivo);

    fclose(archivo);

    // Verifica que se hayan escrito todos los elementos
    return elementosEscritos == static_cast<size_t>(len);
}

// ===== ENCRIPTACION ====

void intAChar(int numero, char texto[])
{
    int i = 0;

    if (numero == 0)
    {
        texto[0] = '0';
        texto[1] = '\0';
        return;
    }

    while (numero > 0)
    {
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
    for (int j = 0; j < i / 2; j++)
    {
        char aux = texto[j];
        texto[j] = texto[i - 1 - j];
        texto[i - 1 - j] = aux;
    }
}

// shift de +5 (osea K = 5)
void encriptado(char contrasenia[])
{
    for (int i = 0; contrasenia[i] != '\0'; i++)
    {
        contrasenia[i] += 5;
    }
}

// mensajes en consola
void mostradorDetexto(char type, string texto, bool debug = false)
{
    switch (type)
    {
    case 'd':
        if (debug)
            cout << "[DEBUG]" + texto << endl;
        break;
    case 'w':
        cout << "[WARNING]" + texto << endl;
        break;
    case 'e':
        cout << "[ERROR]" + texto << endl;
        break;
    default:
        cout << texto << endl;
        break;
    }
}
