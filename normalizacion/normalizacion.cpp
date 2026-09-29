//
// Created by santi on 9/22/2026.
//

#include <stdio.h>
#include <string.h>
#include "../funciones/funciones.h"

#define RUTA_INPUT "../datos/"
#define RUTA_OUTPUT "../datos_de_uso/"

#define MAX_MOZOS 100
#define MAX_COMANDAS 500

// -- Por defecto / lectura original --
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
    int codigo;
    char nombreMozo[50];
    char password[20];
    float totalComision;
};

// Se remueve la fecha según especificación del TP
struct Comanda
{
    int codMozo;
    int codProd;
    int cant;
    float comision;
};

// Declaración de funciones
int archivoMozos(Mozo mozos[], int lenMozos);
void nombresAgrupados(ComandaHistorica array[], int len);
int llenarArrayComandas(ComandaHistorica comandas[]);
void arrayMozos(ComandaHistorica array[], int len, Mozo arrayMozo[], int &lenMozos);
void arrayComandas(ComandaHistorica array[], int len, Mozo mozos[], int lenMozos, Comanda arrayComanda[]);
void agruparPorFecha(ComandaHistorica array[], int len);
int archivosComandas(ComandaHistorica arrayHistorica[], int lenComandas, Mozo mozos[], int lenMozos);
int prunarInventario(Comanda comandas[], int lenComandas);

int main()
{
    ComandaHistorica comandas[MAX_COMANDAS];
    Mozo mozos[MAX_MOZOS];
    int lenMozos = 0;
    Comanda comandasNuevas[MAX_COMANDAS];

    int lenComandas = llenarArrayComandas(comandas);
    if (lenComandas < 0)
    {
        printf("\n Fallo al leer comandas_historicas.dat \n");
        return 1;
    }

    // 1. Agrupar por nombre para procesar la lista de mozos
    nombresAgrupados(comandas, lenComandas);
    arrayMozos(comandas, lenComandas, mozos, lenMozos);

    if (archivoMozos(mozos, lenMozos) < 0)
    {
        printf("\n Fallo al escribir mozos.dat \n");
        return 1;
    }

    // 2. Mapear ComandaHistorica a Comanda (sin fecha)
    arrayComandas(comandas, lenComandas, mozos, lenMozos, comandasNuevas);

    // 3. Agrupar por fecha para generar las planillas por día (comandas_dd-mm-aaaa.dat)
    agruparPorFecha(comandas, lenComandas);

    if (archivosComandas(comandas, lenComandas, mozos, lenMozos) < 0)
    {
        printf("\n Fallo al escribir los archivos de comandas por dia \n");
        return 1;
    }

    // 4. Actualizar el inventario con las ventas históricas
    int resultado = prunarInventario(comandasNuevas, lenComandas);

    if (resultado < 0)
    {
        printf("\n No se puede actualizar inventario.dat, no se pudo abrir el archivo \n");
        return 1;
    }

    if (resultado == 0)
    {
        printf("\n No se puede actualizar inventario.dat debido a que una comanda es mayor al inventario disponible. \n");
        return 1;
    }

    return 0;
}

// Agrupar por fecha.
void agruparPorFecha(ComandaHistorica array[], int len)
{
    for (int i = 0; i < len; i++)
    {
        for (int j = i + 1; j < len; j++)
        {
            if (strcmp(array[i].fecha, array[j].fecha) == 0)
            {
                i++;
                ComandaHistorica temp = array[i];
                array[i] = array[j];
                array[j] = temp;
            }
        }
    }
}

// Agrupar por nombre.
void nombresAgrupados(ComandaHistorica array[], int len)
{
    for (int i = 0; i < len; i++)
    {
        for (int j = i + 1; j < len; j++)
        {
            if (strcmp(array[i].nombreMozo, array[j].nombreMozo) == 0)
            {
                i++;
                ComandaHistorica temp = array[i];
                array[i] = array[j];
                array[j] = temp;
            }
        }
    }
}

// Cargar el archivo comandas historicas en un array.
int llenarArrayComandas(ComandaHistorica comandas[])
{
    FILE *f = fopen(RUTA_INPUT "comandas_historicas.dat", "rb");
    int len = 0;

    if (f == nullptr)
    {
        return -1;
    }

    while (len < MAX_COMANDAS && fread(&comandas[len], sizeof(ComandaHistorica), 1, f) == 1)
    {
        len++;
    }

    fclose(f);
    return len;
}

// Corte de control por nombre para crear el array de mozos y generar su contraseña cifrada.
void arrayMozos(ComandaHistorica array[], int len, Mozo arrayMozo[], int &lenMozos)
{
    lenMozos = 0;
    int i = 0;

    while (i < len)
    {
        char control[50];
        strcpy(control, array[i].nombreMozo);

        float totalComision = 0.0f;
        int cantidadComandas = 0;

        while (i < len && strcmp(array[i].nombreMozo, control) == 0)
        {
            totalComision += array[i].comision;
            cantidadComandas++;
            i++;
        }

        strcpy(arrayMozo[lenMozos].nombreMozo, control);
        arrayMozo[lenMozos].totalComision = totalComision;
        arrayMozo[lenMozos].codigo = lenMozos + 1;

        // --- Generación del Password ---
        // 1. Obtener primeros 4 caracteres del nombre
        char subNombre[5] = "";
        strncpy(subNombre, control, 4);
        subNombre[4] = '\0';

        // 2. Calcular promedio de comisión
        int promedioComision = 0;
        if (cantidadComandas > 0)
        {
            promedioComision = (int)(totalComision / cantidadComandas);
        }

        // 3. Armar password en texto plano (4 letras + promedio)
        sprintf(arrayMozo[lenMozos].password, "%s%d", subNombre, promedioComision);

        // 4. Aplicar la función de encriptación
        encriptado(arrayMozo[lenMozos].password);

        lenMozos++;
    }
}

// Pasa cada ComandaHistorica a Comanda (sin fecha).
void arrayComandas(ComandaHistorica array[], int len, Mozo mozos[], int lenMozos, Comanda arrayComanda[])
{
    for (int i = 0; i < len; i++)
    {
        int j = 0;
        while (j < lenMozos && strcmp(mozos[j].nombreMozo, array[i].nombreMozo) != 0)
        {
            j++;
        }

        arrayComanda[i].codMozo = mozos[j].codigo;
        arrayComanda[i].codProd = array[i].codigoProducto;
        arrayComanda[i].cant = array[i].cantidad;
        arrayComanda[i].comision = array[i].comision;
    }
}

// Crea el archivo mozos.dat final.
int archivoMozos(Mozo mozos[], int lenMozos)
{
    FILE *f = fopen(RUTA_OUTPUT "mozos.dat", "wb");

    if (f == nullptr)
    {
        return -1;
    }

    int escritos = fwrite(mozos, sizeof(Mozo), lenMozos, f);
    int errorCierre = fclose(f);

    if (escritos != lenMozos || errorCierre != 0)
    {
        return -1;
    }

    return lenMozos;
}

// Crea los archivos por fecha (comandas_dd-mm-aaaa.dat) omitiendo el campo fecha dentro de la struct Comanda.
int archivosComandas(ComandaHistorica arrayHistorica[], int lenComandas, Mozo mozos[], int lenMozos)
{
    int i = 0;

    while (i < lenComandas)
    {
        char controlFecha[11];
        strcpy(controlFecha, arrayHistorica[i].fecha);

        Comanda dia[MAX_COMANDAS];
        int lenDia = 0;

        char nombre[60];
        sprintf(nombre, RUTA_OUTPUT "comandas_%s.dat", controlFecha);

        FILE *f = fopen(nombre, "wb");

        if (f == nullptr)
        {
            return -1;
        }

        while (i < lenComandas && strcmp(arrayHistorica[i].fecha, controlFecha) == 0)
        {
            // Buscar el ID del mozo por su nombre
            int j = 0;
            while (j < lenMozos && strcmp(mozos[j].nombreMozo, arrayHistorica[i].nombreMozo) != 0)
            {
                j++;
            }

            dia[lenDia].codMozo = mozos[j].codigo;
            dia[lenDia].codProd = arrayHistorica[i].codigoProducto;
            dia[lenDia].cant = arrayHistorica[i].cantidad;
            dia[lenDia].comision = arrayHistorica[i].comision;

            lenDia++;
            i++;
        }

        // Ordenar por codMozo antes de guardar
        ordBurbujaGenerico(dia, lenDia, [](const Comanda &c)
                           { return c.codMozo; });

        fwrite(dia, sizeof(Comanda), lenDia, f);

        if (fclose(f) != 0)
        {
            return -1;
        }
    }

    return 0;
}

int prunarInventario(Comanda comandas[], int lenComandas)
{
    // 1. Leemos el inventario original desde la carpeta de datos (solo lectura)
    FILE *fEntrada = fopen(RUTA_INPUT "inventario.dat", "rb");
    if (fEntrada == nullptr)
    {
        return -1; // No se pudo abrir ../datos/inventario.dat
    }

    // 2. Creamos el nuevo inventario actualizado en datos_de_uso
    FILE *fSalida = fopen(RUTA_OUTPUT "inventario.dat", "wb");
    if (fSalida == nullptr)
    {
        fclose(fEntrada);
        return -1; // No se pudo crear ../datos_de_uso/inventario.dat
    }

    Producto prod;

    // 3. Recorremos producto por producto el inventario base
    while (fread(&prod, sizeof(Producto), 1, fEntrada) == 1)
    {
        int totalVendido = 0;

        // Sumamos todas las ventas asociadas a este producto
        for (int i = 0; i < lenComandas; i++)
        {
            if (prod.codigo == comandas[i].codProd)
            {
                totalVendido += comandas[i].cant;
            }
        }

        if (totalVendido > 0)
        {
            // Validamos que el stock actual alcance
            if (prod.stockActual >= totalVendido)
            {
                prod.stockActual -= totalVendido;
            }
            else
            {
                printf("\n[ERROR] El producto %d (%s) requiere %d unidades pero solo hay %d en stock.\n",
                       prod.codigo, prod.descripcion, totalVendido, prod.stockActual);

                fclose(fEntrada);
                fclose(fSalida);
                return 0; // Se cancela la generación por falta de stock
            }
        }

        // 4. Escribimos el producto (con stock restado o intacto) en datos_de_uso
        fwrite(&prod, sizeof(Producto), 1, fSalida);
    }

    fclose(fEntrada);
    fclose(fSalida);
    return 1; // Proceso completado exitosamente
}