//
// created by cristien vazquez 28/09/2026
//

#include <iostream>
#include <cstring>
#include <cstdio>
#include "funciones/funciones.h"

using namespace std;

// variables de entorno
bool DEBUG = false;
string DIRECTORIO = "./datos_de_uso/";
string TEXTO_DE_BIENVENIDA = "===============================\n"
                             "    Bienvenido al modulo  \n"
                             "            Ventas\n"
                             "===============================";
string OPCIONES_DEL_MENU_PRINCIPAL = "===============================\n"
                                     "    Selecciona una opcion  \n"
                                     "       del menu pricipal\n"
                                     "===============================\n"
                                     "1- loguear Mozo\n"
                                     "2- finalizar dia";

string OPCIONES_DEL_MENU_MOZO = "===============================\n"
                                "    Selecciona una opcion  \n"
                                "       del menu mozo\n"
                                "===============================\n"
                                "1- Cargar comanda\n"
                                "2- Cambiar Mozo\n"
                                "3- finalizar dia";
string DESPEDIDA = "===============================\n"
                   "      Muchas Gracias por  \n"
                   "        usar el modulo  \n"
                   "           ventas  \n"
                   "===============================\n";

string PEDIDO_DE_FECHA = "Por favor ingresa el dia de hoy respetando el formato DD-MM-YYYY(20-01-2026)";
string RESULTADO_PEDIDO_FECHA = "la Fecha se ingreso Correctamente:";
string CREDENCIALES_INCORRECTAS = "!Ups parece que el mozo o la contraseña es invalida";
string FECHA_INCORRECTA = "!Ups parece que la fecha es invalida,(Debe ser DD-MM-YYYY)";
string SOLICITAR_NOMBRE = "por favor escribe el nombre del mozo (maximo 50 caracteres)";
string SOLICITAR_PASSWORD = "por favor escribe tu contraseña (maximo 20 caracteres)";
string OPCION_INCORRECTA = "Opcion incorrecta por favor elija una opcion del menu";
string DESLOGUEANDO_MOZO = "volviendo al menu principal";

float IMPUESTO = 0.01; // 21% de impuesto

// valida Formato YYYY-MM-DD
bool validarFecha(char fecha[])
{

    for (int i = 0; i < 10; i++)
    {

        // debug cout << fecha[i] << "_" << i << endl;
        if (i == 2 || i == 5)
        {
            if (fecha[i] != '-')
            {
                return false;
            }
        }
        else
        {

            if (!isdigit(fecha[i]))
            {
                return false;
            }
        }
    }
    return true;
}

Comanda *traerComandaDelDia(char fecha[], long len)
{
    mostradorDetexto('d', "Cargando Comandas", DEBUG);
    string direccionArchivo = DIRECTORIO + "comandas_" + fecha + ".dat";

    Comanda *comandasDelDia = leerArchivoGenerico<Comanda>(direccionArchivo, len);
    mostradorDetexto('d', "Comandas Cargadas correctamente", DEBUG);

    return comandasDelDia;
};

// login mozo
bool loginMozo(Mozo mozos[], int len, char nombre[], char contrasenia[], int &indMozo)
{
    mostradorDetexto('d', "Logueando Mozo", DEBUG);
    encriptado(contrasenia);

    for (int i = 0; i < len; i++)
    {
        if (strcmp(mozos[i].nombre, nombre) == 0)
        {
            mostradorDetexto('d', "usuario encontrado pass=" + string(contrasenia), DEBUG);
            if (strcmp(mozos[i].password, contrasenia) == 0)
            {
                indMozo = i;
                return true;
            }
        }
    }
    mostradorDetexto('e', CREDENCIALES_INCORRECTAS, DEBUG);
    return false;
}

Comanda *cargarComandaNueva(Comanda *comandasDelDia, long &len, Comanda nuevaComanda)
{
    mostradorDetexto('d', "agregando comanda", DEBUG);

    Comanda *nuevoArreglo = new Comanda[len + 1];
    int i = len - 1;
    // arranco desde atras para ir comaprando
    while (i >= 0 && comandasDelDia[i].idMozo > nuevaComanda.idMozo)
    {
        nuevoArreglo[i + 1] = comandasDelDia[i];
        i--;
    }
    // como queda un hueco pongo el elemento
    nuevoArreglo[i + 1] = nuevaComanda;

    // termino de agregar los elementos restantes
    while (i >= 0)
    {
        nuevoArreglo[i] = comandasDelDia[i];
        i--;
    }

    mostradorDetexto('d', "se agrego correctamente la comanda", DEBUG);

    len++;
    mostradorDetexto('d', "borrando puntero anterior", DEBUG);

    delete[] comandasDelDia;
    return nuevoArreglo;
}

int buscarProducto(Producto inventario[], int len, int codigoProducto)
{
    mostradorDetexto('d', "buscando un Producto", DEBUG);

    int indice = busquedaBinariaGenerico(inventario, len, codigoProducto, [](int clave, const Producto &p)
                                         { return clave - p.codigo; });
    if (indice == -1)
    {
        mostradorDetexto('w', "Producto no encontrado");
    }
    else
    {
        mostradorDetexto('d', "devolviendo indice=>" + to_string(indice), DEBUG);
    }
    return indice;
}

float calcularComision(Producto &producto, int cantidad)
{
    float valorTotal = producto.precio * cantidad;
    producto.stockActual -= cantidad;
    return valorTotal * IMPUESTO;
}

void ReescribirComandasDias(Comanda *comandasDelDia, int len, char fecha[])
{
    string direccionArchivo = DIRECTORIO + "commandas_" + fecha + ".dat";

    bool guardado = reemplazarDataArchivoGenerico(direccionArchivo, comandasDelDia, len);
    if (!guardado)
    {
        mostradorDetexto('e', "fallo al guardar las comandas del dia.");
    }
    else
    {
        mostradorDetexto('t', "Comandas del dia guardadas exitosamente.");
    }
}

int main()
{
    mostradorDetexto('t', TEXTO_DE_BIENVENIDA);

    // inicializo archivos en memoria con los que voy a trabajar
    mostradorDetexto('d', "Cargando variables iniciales", DEBUG);
    char fechaDelDia[11];
    char nombreMozo[50];
    char password[20];
    int salirMenu = 0, opcionesDelMenuPrincipal = 0, opcionesDelMenuMozo = 0, mozoLogueado = 0, indMozo = -1;

    long largoInventario = obtenerCantidadRegistros<Producto>(DIRECTORIO + "inventario.dat");
    if (largoInventario == -1)
    {
        mostradorDetexto('e', "!Ups no encontre productos que vender");
        return 1;
    }
    Producto *inventario = leerArchivoGenerico<Producto>(DIRECTORIO + "inventario.dat", largoInventario);

    int largoMozos = obtenerCantidadRegistros<Mozo>(DIRECTORIO + "mozos.dat");

    if (largoMozos == -1)
    {
        mostradorDetexto('e', "!Ups no encontre mozos en sistema");
        return 1;
    }

    Mozo *mozos = leerArchivoGenerico<Mozo>(DIRECTORIO + "mozos.dat", largoMozos);
    mostradorDetexto('d', "cant mozos:" + to_string(largoMozos), DEBUG);
    for (size_t i = 0; i < largoMozos; i++)
    {
        mostradorDetexto('d', "nombre mozo: " + string(mozos[i].nombre), DEBUG);
        mostradorDetexto('d', "pass mozo: " + string(mozos[i].password), DEBUG);
        mostradorDetexto('d', "cod mozo: " + to_string(mozos[i].idMozo), DEBUG);
    }

    mostradorDetexto('d', "No se Encontro el archivo Comanda", DEBUG);

    Comanda *comandasDelDia = new Comanda[0];

    mostradorDetexto('t', PEDIDO_DE_FECHA);
    cin.getline(fechaDelDia, 11);

    if (!validarFecha(fechaDelDia))
    {
        mostradorDetexto('e', FECHA_INCORRECTA);
        return 1;
    }

    mostradorDetexto('t', RESULTADO_PEDIDO_FECHA + fechaDelDia);
    // pido el dia para cargar comanda

    // cargo comanda del dia
    string rutaComandas = DIRECTORIO + "comandas_" + string(fechaDelDia) + ".dat";
    mostradorDetexto('d', "Ruta a buscar: " + rutaComandas, DEBUG);
    long largoDeComandas = obtenerCantidadRegistros<Comanda>(rutaComandas);
    if (largoDeComandas != -1)
    {
        Comanda *punteroAux = comandasDelDia;
        comandasDelDia = traerComandaDelDia(fechaDelDia, largoDeComandas);
        delete[] punteroAux;
        ;
    }
    else
    {
        largoDeComandas = 0;
    }

    mostradorDetexto('d', "comanda en Memoria largo =" + to_string(largoDeComandas), DEBUG);

    // menu mozo
    while (salirMenu == 0)
    {
        if (mozoLogueado == 1)
        {
            opcionesDelMenuPrincipal = 0;

            mostradorDetexto('t', OPCIONES_DEL_MENU_MOZO);
            cin >> opcionesDelMenuMozo;
            if (opcionesDelMenuMozo == 1)
            {
                // agregar comnada producto
                char nombreProducto;
                Comanda nuevaComanda;
                nuevaComanda.idMozo = mozos[indMozo].idMozo;
                mostradorDetexto('t', "Ingrese el codigo del producto");
                cin >> nuevaComanda.codigoProducto;

                int indiceProducto = buscarProducto(inventario, largoInventario, nuevaComanda.codigoProducto);
                if (indiceProducto != -1)
                {
                    Producto producto = inventario[indiceProducto];

                    mostradorDetexto('t', "ingrese la cantidad de unidades (max= " + to_string(producto.stockActual) + ")");
                    cin >> nuevaComanda.cantidad;
                    if (nuevaComanda.cantidad > producto.stockActual)
                    {
                        mostradorDetexto('t', "exediste la cantidad de productos en stock");
                    }
                    else
                    {
                        float comision = calcularComision(producto, nuevaComanda.cantidad);
                        nuevaComanda.comision = comision;
                        mozos[indMozo].totalComision += comision;
                        inventario[indiceProducto] = producto;
                        comandasDelDia = cargarComandaNueva(comandasDelDia, largoDeComandas, nuevaComanda);

                        mostradorDetexto('t', "producto cargado correctamente");
                    }
                }
                else
                {
                    mostradorDetexto('t', "producto no encontrado");
                }
            }
            else if (opcionesDelMenuMozo == 2)
            {
                mozoLogueado = 0;
                mostradorDetexto('t', DESLOGUEANDO_MOZO);
            }
            else if (opcionesDelMenuMozo == 3)
            {
                // terminar menu
                salirMenu = 1;
                mostradorDetexto('t', DESPEDIDA);
            }
            else
            {
                mostradorDetexto('t', OPCION_INCORRECTA);
            }
        }
        else
        {
            mostradorDetexto('t', OPCIONES_DEL_MENU_PRINCIPAL);
            cin >> opcionesDelMenuPrincipal;

            if (opcionesDelMenuPrincipal == 1)
            {
                // logueo a mozo
                mostradorDetexto('t', SOLICITAR_NOMBRE);
                cin.ignore();
                cin.getline(nombreMozo, 50);
                mostradorDetexto('t', SOLICITAR_PASSWORD);
                cin.getline(password, 20);
                if (loginMozo(mozos, largoMozos, nombreMozo, password, indMozo))
                {
                    mostradorDetexto('t', "Hola " + string(nombreMozo) + ", un gusto volver a verte");
                    mozoLogueado = 1;
                }
                else
                {
                    mostradorDetexto('e', CREDENCIALES_INCORRECTAS);
                }
            }
            else if (opcionesDelMenuPrincipal == 2)
            {
                // terminar menu
                salirMenu = 1;
                mostradorDetexto('t', DESPEDIDA);
            }
            else
            {
                mostradorDetexto('t', OPCION_INCORRECTA);
            }
        }
    }

    // modificar archivos
    reemplazarDataArchivoGenerico<Comanda>(DIRECTORIO + "comandas_" + string(fechaDelDia) + ".dat", comandasDelDia, largoDeComandas);
    reemplazarDataArchivoGenerico<Producto>(DIRECTORIO + "inventario.dat", inventario, largoInventario);

    return 0;
}