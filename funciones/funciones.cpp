#include <iostream>

//===========APAREO ===========
template <typename T1, typename T2, typename TOut, typename Comp>

void apareoGenerico(T1 arrA[], int lenA, T2 arrB[], int lenB, TOut arrC[], int &lenC, Comp comp)
{
    int i = 0, j = 0;
    lenC = 0;
    // mergeo 2 arr
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

    // meto los sobrantes
    while (i < lenA)
    {
        arrC[lenC++] = arrA[i++];
    }

    while (j < lenB)
    {
        arrC[lenC++] = arrB[j++];
    }

    return;
}

//======= CORTE CONTROL =======
template <typename T, typename ClaveCorte, typename ValProcRegistro, typename ProcesarReg, typename OperacionFin>

void corteControlGenerico(T arr[], int lenA, ClaveCorte claveCorte, ValProcRegistro valProcRegistro, ProcesarReg procRegistro, OperacionFin operacionFinCorte)
{
    int i = 0;

    while (i < lenA)
    {
        // busco el corte
        int corte = claveCorte(i, arr, lenA);
        auto estructura = valProcRegistro(arr, i, corte);
        while (i < lenA && corte != claveCorte(j, arr, lenA))
        {
            i++;
            // proceso el registro
            procRegistro(estructura, arr[i]);
        }
        // hago la operacion de fin
        operacionFinCorte(arr, i, j);
    }
    return;
}

// ====== ORDENAMIENTO ======
template <typename T, typename ClaveOrden>
void ordSeleccionGenerico(T arr[], int len, ClaveOrden claveOrden)
{
    for (int i = 0; i < len; i++)
    {
        auto claveMin = claveOrden(arr[i]);
        for (j = i + 1; j < len; j++)
        {
            if (claveOrden(arr[j]) < claveMin)
            {
                auto temp = arr[i];
                claveMin = claveOrden(arr[j]);
                arr[i] = arr[j];
                arr[j] = temp;
            }
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
        if (comparador(claveBusqueda, arr[i]) == 0) // si el comparador devuelve 0, significa que se encontró el elemento
        {
            return i; // retorna el indice del elemento encontrado
        }
    }
    return -1; // retorna -1 si no se encuentra el elemento
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
            return mid; // retorna el indice del elemento encontrado
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
    return -1; // retorna -1 si no se encuentra el elemento
}
