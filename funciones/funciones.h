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

// ====== ORDENAMIENTO ======

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
bool leerRegistroPorIndice(string nombreArchivo, int indice, T &registroDestino)
{
    FILE *f = fopen(nombreArchivo.c_str(), "rb");
    if (f == NULL)
    {
        return false;
    }

    fseek(f, indice * sizeof(T), SEEK_SET);
    size_t leidos = fread(&registroDestino, sizeof(T), 1, f);
    fclose(f);

    return (leidos == 1);
}

template <typename T>
bool modificarRegistro(string nombreArchivo, int indice, const T &nuevoRegistro)
{
    // "rb+" abre el archivo para lectura y escritura binaria sin borrar su contenido previo
    FILE *f = fopen(nombreArchivo.c_str(), "rb+");
    if (f == NULL)
    {
        mostradorDetexto('e', "No se pudo abrir el archivo para edición");
        return false;
    }

    // Mover el cursor hasta la posición exacta del registro
    fseek(f, indice * sizeof(T), SEEK_SET);

    // Escribir el registro actualizado sobrescribiendo el anterior
    fwrite(&nuevoRegistro, sizeof(T), 1, f);

    fclose(f);
    return true;
}

template <typename T, typename Comparador>
long busquedaBinariaArchivoGenerica(string nombre, int clave, T &registro, Comparador comparar)
{
    FILE *f = fopen(nombre.c_str(), "rb");
    if (f == NULL)
        return -1;

    fseek(f, 0, SEEK_END);
    long n = ftell(f) / sizeof(T);
    long pri = 0, ult = n - 1, pos = -1;

    while (pri <= ult && pos == -1)
    {
        long med = pri + (ult - pri) / 2;
        fseek(f, med * sizeof(T), SEEK_SET);

        if (fread(&registro, sizeof(T), 1, f) != 1)
            break;

        int resultado = comparar(clave, registro);
        if (resultado == 0)
            pos = med;
        else if (resultado < 0)
            ult = med - 1;
        else
            pri = med + 1;
    }

    fclose(f);
    return pos;
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
    const size_t cantidadEsperada = len;
    // abro el archivo en modo escritura binaria
    FILE *archivo = fopen(nombreArchivo.c_str(), "wb");

    if (archivo == NULL)
    {
        cout << "[Error]: No se pudo abrir/crear el archivo para escritura." << endl;
        return false;
    }

    // Escribe todo el arreglo de una sola vez
    size_t elementosEscritos = fwrite(arr, sizeof(T), cantidadEsperada, archivo);

    fclose(archivo);

    // Verifica que se hayan escrito todos los elementos
    return elementosEscritos == cantidadEsperada;
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
