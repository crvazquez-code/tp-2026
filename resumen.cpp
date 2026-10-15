//
// resumen.cpp
// Arma el resumen del cierre: junta la planilla semanal por mozo
// (corte de control, porque ya viene ordenada por idMozo) e imprime
// cuanto vendio y cuanta comision le toca a cada uno, y el total del buffet.
//

#include <stdio.h>
#include <iomanip>
#include "funciones/funciones.h"

using namespace std;

#define RUTA "./datos_de_uso/"
#define MAX_MOZOS 100

// Busca el nombre del mozo por su id. Si no lo encuentra, devuelve "Desconocido".
const char *buscarNombreMozo(Mozo mozos[], int lenMozos, int idMozo)
{
    int indice = busquedaSecuencialGenerico(mozos, lenMozos, idMozo, [](int clave, const Mozo &m)
                                            { return clave - m.idMozo; });

    if (indice == -1)
    {
        return "Desconocido";
    }

    return mozos[indice].nombre;
}

// Imprime la linea del resumen para un mozo.
void mostrarResumenMozo(int idMozo, const char *nombre, int cantidadProductos, float comision)
{
    cout << left << setw(4) << idMozo << setw(20) << nombre << setw(12) << cantidadProductos << "$" << fixed << setprecision(2) << comision << endl;
}

int main()
{
    int semana, mes;

    mostradorDetexto('t', "=== Resumen del cierre semanal ===");
    mostradorDetexto('t', "Ingrese el numero de semana: ");
    cin >> semana;
    mostradorDetexto('t', "Ingrese el numero de mes:");
    cin >> mes;

    if (semana < 1 || semana > 5 || mes < 1 || mes > 12)
    {
        mostradorDetexto('e', "Semana o mes invalido.\n");
        return 1;
    }

    string nombreArchivo;
    nombreArchivo = string(RUTA) + "comandas_semana_s" + to_string(semana) + "-" + to_string(mes) + ".dat";
    mostradorDetexto('t', nombreArchivo);

    FILE *f = fopen(nombreArchivo.c_str(), "rb");
    if (f == nullptr)
    {
        mostradorDetexto('e', "No existe el archivo del resumen. Corre cierre.cpp primero.");
        return 1;
    }

    // Cargamos mozos.dat para poder mostrar el nombre junto al id.
    // Si no se encuentra, igual podemos mostrar el resumen solo con el id.
    Mozo mozos[MAX_MOZOS];
    int lenMozos = 0;

    FILE *fMozos = fopen(RUTA "mozos.dat", "rb");
    if (fMozos != nullptr)
    {
        while (lenMozos < MAX_MOZOS && fread(&mozos[lenMozos], sizeof(Mozo), 1, fMozos) == 1)
        {
            lenMozos++;
        }
        fclose(fMozos);
    }
    else
    {
        mostradorDetexto('w', "No se encontro mozos.dat, el resumen va a mostrar solo el numero de mozo.");
    }

    cout << left << setw(4) << "ID" << setw(20) << "Mozo" << setw(12) << "Productos" << "Comision" << endl;
    cout << "-----------------------------------------------" << endl;

    // Corte de control por idMozo: la planilla semanal ya viene ordenada por mozo,
    // asi que no hace falta cargarla entera en memoria ni volver a ordenarla.
    Comanda c;
    int idMozoActual = 0;
    int cantidadMozoActual = 0;
    float comisionMozoActual = 0.0f;
    bool hayMozoAbierto = false;

    int totalProductosBuffet = 0;
    float totalComisionBuffet = 0.0f;

    while (fread(&c, sizeof(Comanda), 1, f) == 1)
    {
        if (hayMozoAbierto && c.idMozo != idMozoActual)
        {
            // Cambio de mozo: cerramos el corte anterior y lo mostramos.
            mostrarResumenMozo(idMozoActual, buscarNombreMozo(mozos, lenMozos, idMozoActual),
                               cantidadMozoActual, comisionMozoActual);
            cantidadMozoActual = 0;
            comisionMozoActual = 0.0f;
        }

        idMozoActual = c.idMozo;
        hayMozoAbierto = true;
        cantidadMozoActual += c.cantidad;
        comisionMozoActual += c.comision;

        totalProductosBuffet += c.cantidad;
        totalComisionBuffet += c.comision;
    }

    fclose(f);

    // Caso raro: el ultimo mozo del archivo no tiene un "cambio" que lo dispare,
    // asi que lo mostramos aparte, afuera del while.
    if (hayMozoAbierto)
    {
        mostrarResumenMozo(idMozoActual, buscarNombreMozo(mozos, lenMozos, idMozoActual),
                           cantidadMozoActual, comisionMozoActual);
    }
    else
    {
        mostradorDetexto('t', "(la planilla semanal esta vacia)");
    }

    cout << "-----------------------------------------------" << endl;
    cout << "TOTAL de productos vendidos por el buffet: " << totalProductosBuffet << endl;
    cout << "TOTAL de comisiones a pagar: $" << fixed << setprecision(2) << totalComisionBuffet << endl;

    return 0;
}
