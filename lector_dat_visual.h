#pragma once
// Implementacion compartida de los dos lectores visuales. C++11 / WinAPI.
#ifndef LECTOR_VARIANTE
#define LECTOR_VARIANTE 1
#endif
#define ES_NOCHE (LECTOR_VARIANTE == 1)
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shellapi.h>
#include <cwctype>
#include <uxtheme.h>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <vector>
#include "funciones/funciones.h"

#ifdef _MSC_VER
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "uxtheme.lib")
#endif

namespace LectorVisual
{
const int FILAS_POR_PAGINA = 200;
enum Formato { HEXADECIMAL, COMANDA, HISTORICA, MOZO, PRODUCTO };
enum Control { ABRIR = 100, FORMATOS, RECARGAR, ANTERIOR, SIGUIENTE, TABLA };

HWND ventana, titulo, ayuda, abrir, etiquetaFormato, formatos, recargar;
HWND ruta, tabla, anterior, siguiente, paginaTexto, estado;
HFONT fuente = nullptr, fuenteTitulo = nullptr;
FILE *archivo = nullptr;
wstring nombreArchivo;
long long tamanoArchivo = 0, pagina = 0;
Formato formato = HEXADECIMAL;
void animarContenido();

// Los campos char[N] pueden no terminar en '\0'. Se respeta siempre su limite.
template <size_t N>
wstring texto(const char (&campo)[N])
{
    int largo = 0;
    while (largo < static_cast<int>(N) && campo[largo] != '\0')
        largo++;
    if (largo == 0)
        return L"";

    UINT codificacion = CP_UTF8;
    DWORD opciones = MB_ERR_INVALID_CHARS;
    int cantidad = MultiByteToWideChar(codificacion, opciones, campo, largo, nullptr, 0);
    if (cantidad == 0)
    {
        codificacion = CP_ACP;
        opciones = 0;
        cantidad = MultiByteToWideChar(codificacion, opciones, campo, largo, nullptr, 0);
    }
    if (cantidad == 0)
        return L"[texto invalido]";
    wstring resultado(cantidad, L' ');
    MultiByteToWideChar(codificacion, opciones, campo, largo, &resultado[0], cantidad);
    for (size_t i = 0; i < resultado.size(); i++)
        if (resultado[i] < L' ')
            resultado[i] = L' ';
    return resultado;
}

wstring importe(float valor)
{
    wostringstream salida;
    salida << fixed << setprecision(2) << valor;
    return salida.str();
}

int tamanoRegistro()
{
    switch (formato)
    {
    case COMANDA: return sizeof(Comanda);
    case HISTORICA: return sizeof(ComandaHistorica);
    case MOZO: return sizeof(Mozo);
    case PRODUCTO: return sizeof(Producto);
    default: return 16; // Dieciseis bytes por fila en la vista hexadecimal.
    }
}

long long cantidadRegistros()
{
    int tamano = tamanoRegistro();
    return tamanoArchivo / tamano + (formato == HEXADECIMAL && tamanoArchivo % tamano != 0);
}

long long cantidadPaginas()
{
    long long cantidad = cantidadRegistros();
    return cantidad / FILAS_POR_PAGINA + (cantidad % FILAS_POR_PAGINA != 0);
}

Formato detectarFormato(const wstring &direccion)
{
    size_t separador = direccion.find_last_of(L"/\\");
    wstring nombre = direccion.substr(separador == wstring::npos ? 0 : separador + 1);
    for (size_t i = 0; i < nombre.size(); i++)
        nombre[i] = static_cast<wchar_t>(towlower(nombre[i]));
    if (nombre == L"comandas_historicas.dat") return HISTORICA;
    if (nombre == L"mozos.dat") return MOZO;
    if (nombre == L"inventario.dat") return PRODUCTO;
    if (nombre.find(L"comandas_") == 0) return COMANDA;
    return HEXADECIMAL;
}

// Los binarios no describen sus campos: cada formato usa las structs del TP.
bool leerFila(vector<wstring> &campos, long long offset)
{
    switch (formato)
    {
    case COMANDA:
    {
        Comanda dato = {};
        if (fread(&dato, sizeof(dato), 1, archivo) != 1) return false;
        campos = {to_wstring(dato.idMozo), to_wstring(dato.codigoProducto),
                  to_wstring(dato.cantidad), importe(dato.comision)};
        break;
    }
    case HISTORICA:
    {
        ComandaHistorica dato = {};
        if (fread(&dato, sizeof(dato), 1, archivo) != 1) return false;
        campos = {texto(dato.fecha), texto(dato.nombreMozo), to_wstring(dato.codigoProducto),
                  to_wstring(dato.cantidad), importe(dato.comision)};
        break;
    }
    case MOZO:
    {
        Mozo dato = {};
        if (fread(&dato, sizeof(dato), 1, archivo) != 1) return false;
        campos = {to_wstring(dato.idMozo), texto(dato.nombre), texto(dato.password),
                  importe(dato.totalComision)};
        break;
    }
    case PRODUCTO:
    {
        Producto dato = {};
        if (fread(&dato, sizeof(dato), 1, archivo) != 1) return false;
        campos = {to_wstring(dato.codigo), texto(dato.descripcion),
                  importe(dato.precio), to_wstring(dato.stockActual)};
        break;
    }
    default:
    {
        unsigned char bytes[16];
        size_t cantidad = fread(bytes, 1, sizeof(bytes), archivo);
        size_t esperada = static_cast<size_t>(tamanoArchivo - offset < 16 ? tamanoArchivo - offset : 16);
        if (cantidad != esperada || ferror(archivo)) return false;
        wostringstream posicion, hexadecimal;
        wstring ascii;
        posicion << uppercase << hex << setw(8) << setfill(L'0') << offset;
        hexadecimal << uppercase << hex << setfill(L'0');
        for (size_t i = 0; i < cantidad; i++)
        {
            if (i != 0) hexadecimal << L' ';
            hexadecimal << setw(2) << static_cast<int>(bytes[i]);
            ascii += bytes[i] >= 32 && bytes[i] <= 126 ? static_cast<wchar_t>(bytes[i]) : L'.';
        }
        campos = {posicion.str(), hexadecimal.str(), ascii};
    }
    }
    return true;
}

void crearColumnas()
{
    ListView_DeleteAllItems(tabla);
    while (ListView_DeleteColumn(tabla, 0)) {}
    vector<wstring> nombres;
    vector<int> anchos;
    switch (formato)
    {
    case COMANDA:
        nombres = {L"Registro", L"ID mozo", L"Codigo producto", L"Cantidad", L"Comision"};
        anchos = {85, 110, 160, 110, 140};
        break;
    case HISTORICA:
        nombres = {L"Registro", L"Fecha", L"Mozo", L"Codigo producto", L"Cantidad", L"Comision"};
        anchos = {85, 120, 240, 150, 100, 120};
        break;
    case MOZO:
        nombres = {L"Registro", L"ID mozo", L"Nombre", L"Clave cifrada", L"Comision acumulada"};
        anchos = {85, 100, 240, 180, 170};
        break;
    case PRODUCTO:
        nombres = {L"Registro", L"Codigo", L"Descripcion", L"Precio", L"Stock actual"};
        anchos = {85, 100, 320, 120, 120};
        break;
    default:
        nombres = {L"Fila", L"Offset (hex)", L"Bytes (hex)", L"ASCII"};
        anchos = {85, 130, 440, 190};
    }
    for (size_t i = 0; i < nombres.size(); i++)
    {
        LVCOLUMNW columna = {};
        columna.mask = LVCF_TEXT | LVCF_WIDTH;
        columna.pszText = &nombres[i][0];
        columna.cx = anchos[i];
        ListView_InsertColumn(tabla, static_cast<int>(i), &columna);
    }
}

void mostrarPagina()
{
    SendMessageW(tabla, WM_SETREDRAW, FALSE, 0);
    ListView_DeleteAllItems(tabla);
    bool lecturaCorrecta = true;
    if (archivo != nullptr)
    {
        clearerr(archivo);
        long long primero = pagina * FILAS_POR_PAGINA;
        lecturaCorrecta = _fseeki64(archivo, primero * tamanoRegistro(), SEEK_SET) == 0;
        for (int fila = 0; lecturaCorrecta && fila < FILAS_POR_PAGINA &&
                           primero + fila < cantidadRegistros(); fila++)
        {
            vector<wstring> campos;
            lecturaCorrecta = leerFila(campos, (primero + fila) * tamanoRegistro());
            if (!lecturaCorrecta) break;
            wstring numero = to_wstring(primero + fila + 1);
            LVITEMW item = {};
            item.mask = LVIF_TEXT;
            item.iItem = fila;
            item.pszText = &numero[0];
            ListView_InsertItem(tabla, &item);
            for (size_t col = 0; col < campos.size(); col++)
                ListView_SetItemText(tabla, fila, static_cast<int>(col + 1), &campos[col][0]);
        }
    }
    SendMessageW(tabla, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(tabla, nullptr, TRUE);

    long long paginas = cantidadPaginas();
    wstring textoPagina = L"Pagina " + to_wstring(paginas == 0 ? 0 : pagina + 1) +
                          L" de " + to_wstring(paginas) + L"   (200 filas por pagina)";
    SetWindowTextW(paginaTexto, textoPagina.c_str());
    EnableWindow(anterior, archivo != nullptr && pagina > 0);
    EnableWindow(siguiente, archivo != nullptr && pagina + 1 < paginas);
    EnableWindow(recargar, archivo != nullptr);
    if (archivo == nullptr)
        SetWindowTextW(estado, L"Abra un archivo .dat o arrastrelo sobre esta ventana.");
    else
    {
        wstring resumen = to_wstring(tamanoArchivo) + L" bytes | " +
                          to_wstring(cantidadRegistros()) + (formato == HEXADECIMAL ? L" filas" : L" registros");
        if (formato != HEXADECIMAL && tamanoArchivo % tamanoRegistro() != 0)
            resumen += L" | ATENCION: " + to_wstring(tamanoArchivo % tamanoRegistro()) +
                       L" bytes sobrantes. Revise el formato o use hexadecimal.";
        if (!lecturaCorrecta)
            resumen = L"ERROR al leer: el archivo pudo cambiar. Pulse Recargar.";
        SetWindowTextW(estado, resumen.c_str());
    }
    animarContenido();
}

bool abrirArchivo(const wstring &direccion, bool detectar = true)
{
    FILE *nuevo = _wfopen(direccion.c_str(), L"rb");
    if (nuevo == nullptr)
    {
        MessageBoxW(ventana, L"No se pudo abrir el archivo para lectura.", L"Lector DAT", MB_OK | MB_ICONERROR);
        return false;
    }
    long long bytes = -1;
    if (_fseeki64(nuevo, 0, SEEK_END) == 0)
        bytes = _ftelli64(nuevo);
    if (bytes < 0)
    {
        fclose(nuevo);
        MessageBoxW(ventana, L"No se pudo obtener el tamano del archivo.", L"Lector DAT", MB_OK | MB_ICONERROR);
        return false;
    }
    if (archivo != nullptr) fclose(archivo);
    archivo = nuevo;
    nombreArchivo = direccion;
    tamanoArchivo = bytes;
    pagina = 0;
    if (detectar) formato = detectarFormato(direccion);
    SendMessageW(formatos, CB_SETCURSEL, formato, 0);
    SetWindowTextW(ruta, direccion.c_str());
    crearColumnas();
    mostrarPagina();
    return true;
}

void elegirArchivo()
{
    wchar_t direccion[32768] = {};
    OPENFILENAMEW dialogo = {};
    dialogo.lStructSize = sizeof(dialogo);
    dialogo.hwndOwner = ventana;
    dialogo.lpstrFilter = L"Archivos DAT (*.dat)\0*.dat\0Todos los archivos (*.*)\0*.*\0";
    dialogo.lpstrFile = direccion;
    dialogo.nMaxFile = sizeof(direccion) / sizeof(direccion[0]);
    dialogo.lpstrTitle = L"Abrir archivo binario";
    dialogo.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_NOCHANGEDIR;
    if (GetOpenFileNameW(&dialogo))
        abrirArchivo(direccion);
}

HWND crearControl(const wchar_t *clase, const wchar_t *textoControl, DWORD estilo, int id = 0)
{
    if (wstring(clase) == L"BUTTON") estilo = (estilo & ~BS_TYPEMASK) | BS_OWNERDRAW;
    HWND control = CreateWindowExW(0, clase, textoControl, WS_CHILD | WS_VISIBLE | estilo,
                                  0, 0, 1, 1, ventana,
                                  reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
                                  GetModuleHandleW(nullptr), nullptr);
    SendMessageW(control, WM_SETFONT, reinterpret_cast<WPARAM>(fuente), TRUE);
    return control;
}

// Animaciones breves: entrada de la tabla, contadores y hover de botones.
// El temporizador se detiene al terminar; se respeta la preferencia de Windows.
const COLORREF FONDO = ES_NOCHE ? RGB(11, 18, 30) : RGB(247, 245, 239);
const COLORREF SUPERFICIE = ES_NOCHE ? RGB(19, 30, 46) : RGB(255, 254, 250);
const COLORREF LATERAL = ES_NOCHE ? RGB(15, 25, 40) : RGB(239, 235, 225);
const COLORREF TEXTO = ES_NOCHE ? RGB(232, 241, 247) : RGB(34, 57, 49);
const COLORREF SECUNDARIO = ES_NOCHE ? RGB(143, 164, 184) : RGB(114, 117, 105);
const COLORREF ACENTO = ES_NOCHE ? RGB(64, 224, 193) : RGB(171, 74, 47);
const COLORREF BORDE = ES_NOCHE ? RGB(35, 52, 71) : RGB(223, 220, 209);
HFONT fuenteGrande = nullptr, fuenteChica = nullptr, fuenteMono = nullptr;
HBRUSH pincelSuperficie = nullptr;
HIMAGELIST altoFilas = nullptr;
HWND vacio = nullptr;
BOOL permitirAnimacion = TRUE;
ULONGLONG inicioAnimacion = 0;
double progreso = 1, contadorDesde = 0, contadorHasta = 0;
struct BotonAnimado { HWND control; bool dentro; double hover; };
BotonAnimado botones[4] = {};

COLORREF mezclar(COLORREF a, COLORREF b, double t)
{
    return RGB(static_cast<int>(GetRValue(a) + (GetRValue(b) - GetRValue(a)) * t),
               static_cast<int>(GetGValue(a) + (GetGValue(b) - GetGValue(a)) * t),
               static_cast<int>(GetBValue(a) + (GetBValue(b) - GetBValue(a)) * t));
}

double suavizar(double t) { double inverso = 1 - t; return 1 - inverso * inverso * inverso; }

void panel(HDC dc, RECT rect, COLORREF color, COLORREF borde, int radio = 16)
{
    HBRUSH brocha = CreateSolidBrush(color);
    HPEN lapiz = CreatePen(PS_SOLID, 1, borde);
    HGDIOBJ anteriorBrocha = SelectObject(dc, brocha), anteriorLapiz = SelectObject(dc, lapiz);
    RoundRect(dc, rect.left, rect.top, rect.right, rect.bottom, radio, radio);
    SelectObject(dc, anteriorBrocha); SelectObject(dc, anteriorLapiz);
    DeleteObject(brocha); DeleteObject(lapiz);
}

void rotulo(HDC dc, const wstring &valor, RECT rect, HFONT font, COLORREF color,
            UINT opciones = DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS)
{
    HGDIOBJ anteriorFuente = SelectObject(dc, font);
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, color);
    DrawTextW(dc, valor.c_str(), static_cast<int>(valor.size()), &rect, opciones);
    SelectObject(dc, anteriorFuente);
}

wstring nombreCorto()
{
    if (nombreArchivo.empty()) return L"Sin archivo seleccionado";
    size_t corte = nombreArchivo.find_last_of(L"/\\");
    return nombreArchivo.substr(corte == wstring::npos ? 0 : corte + 1);
}

void distribuirControles(int ancho, int alto)
{
    int x = ES_NOCHE ? 240 : 32;
    int derecha = ancho - (ES_NOCHE ? 28 : 32);
    int yTabla = ES_NOCHE ? 286 : 334;
    int desplazamiento = static_cast<int>((1 - suavizar(progreso)) * 14);
    if (ES_NOCHE)
    {
        MoveWindow(abrir, 20, 150, 168, 42, TRUE);
        MoveWindow(formatos, 20, 246, 168, 250, TRUE);
        MoveWindow(recargar, 20, 306, 168, 38, TRUE);
        MoveWindow(ruta, x + 4, 246, derecha - x - 8, 25, TRUE);
    }
    else
    {
        MoveWindow(abrir, derecha - 164, 74, 164, 42, TRUE);
        MoveWindow(formatos, x + 80, 250, 325, 250, TRUE);
        MoveWindow(recargar, derecha - 120, 250, 120, 36, TRUE);
        MoveWindow(ruta, x + 4, 297, derecha - x - 8, 25, TRUE);
    }
    MoveWindow(tabla, x + 1, yTabla + desplazamiento, derecha - x - 2,
               alto - yTabla - 100 - desplazamiento, TRUE);
    int columnas = Header_GetItemCount(ListView_GetHeader(tabla));
    if (columnas > 0)
    {
        int ocupados = 0;
        for (int i = 0; i < columnas - 1; i++) ocupados += ListView_GetColumnWidth(tabla, i);
        int libre = derecha - x - 22 - ocupados;
        ListView_SetColumnWidth(tabla, columnas - 1, libre > 150 ? libre : 150);
    }
    MoveWindow(anterior, x, alto - 76, 104, 34, TRUE);
    MoveWindow(siguiente, x + 114, alto - 76, 104, 34, TRUE);
    if (vacio != nullptr)
    {
        int centro = yTabla + (alto - yTabla - 100) / 2;
        MoveWindow(vacio, x + 24, centro - 42, derecha - x - 48, 84, TRUE);
        SetWindowTextW(vacio, archivo == nullptr ? L"Arrastra un archivo .dat\npara explorar sus registros" :
            (tamanoArchivo == 0 ? L"Este archivo esta vacio" : L"No hay registros completos\nPrueba otro formato"));
        ShowWindow(vacio, cantidadRegistros() == 0 ? SW_SHOW : SW_HIDE);
    }
}

void animarContenido()
{
    double actual = contadorDesde + (contadorHasta - contadorDesde) * suavizar(progreso);
    contadorDesde = actual;
    contadorHasta = static_cast<double>(cantidadRegistros());
    progreso = permitirAnimacion ? 0 : 1;
    inicioAnimacion = GetTickCount64();
    if (ventana != nullptr)
    {
        if (permitirAnimacion) SetTimer(ventana, 1, 16, nullptr);
        RECT cliente; GetClientRect(ventana, &cliente);
        distribuirControles(cliente.right, cliente.bottom);
        InvalidateRect(ventana, nullptr, FALSE);
    }
}

LRESULT CALLBACK hoverBoton(HWND hwnd, UINT mensaje, WPARAM wParam, LPARAM lParam,
                           UINT_PTR id, DWORD_PTR)
{
    if (mensaje == WM_MOUSEMOVE && !botones[id].dentro)
    {
        botones[id].dentro = true;
        TRACKMOUSEEVENT seguimiento = {sizeof(seguimiento), TME_LEAVE, hwnd, 0};
        TrackMouseEvent(&seguimiento);
        SetTimer(ventana, 1, 16, nullptr);
    }
    if (mensaje == WM_MOUSELEAVE)
    {
        botones[id].dentro = false;
        SetTimer(ventana, 1, 16, nullptr);
    }
    if (mensaje == WM_NCDESTROY) RemoveWindowSubclass(hwnd, hoverBoton, id);
    return DefSubclassProc(hwnd, mensaje, wParam, lParam);
}

void actualizarAnimacion()
{
    bool seguir = false;
    if (progreso < 1)
    {
        progreso = permitirAnimacion ? static_cast<double>(GetTickCount64() - inicioAnimacion) / 360.0 : 1;
        if (progreso >= 1) progreso = 1;
        else seguir = true;
        RECT cliente; GetClientRect(ventana, &cliente);
        distribuirControles(cliente.right, cliente.bottom);
        InvalidateRect(ventana, nullptr, FALSE);
    }
    for (int i = 0; i < 4; i++)
    {
        double destino = botones[i].dentro && IsWindowEnabled(botones[i].control) ? 1 : 0;
        if (fabs(botones[i].hover - destino) > 0.01)
        {
            botones[i].hover += (destino - botones[i].hover) * (permitirAnimacion ? 0.25 : 1);
            seguir = true;
            InvalidateRect(botones[i].control, nullptr, FALSE);
        }
        else botones[i].hover = destino;
    }
    if (!seguir) KillTimer(ventana, 1);
}

void dibujarFondo(HDC dc)
{
    RECT cliente; GetClientRect(ventana, &cliente);
    int ancho = cliente.right, alto = cliente.bottom;
    HBRUSH fondo = CreateSolidBrush(FONDO);
    FillRect(dc, &cliente, fondo); DeleteObject(fondo);
    int x = ES_NOCHE ? 240 : 32, derecha = ancho - (ES_NOCHE ? 28 : 32);
    if (ES_NOCHE)
    {
        RECT barra = {0, 0, 208, alto};
        HBRUSH brocha = CreateSolidBrush(LATERAL); FillRect(dc, &barra, brocha); DeleteObject(brocha);
        panel(dc, {20, 28, 60, 68}, ACENTO, ACENTO, 12);
        rotulo(dc, L"D", {20, 28, 60, 68}, fuenteTitulo, RGB(8, 39, 34), DT_CENTER | DT_SINGLELINE | DT_VCENTER);
        rotulo(dc, L"DAT STUDIO", {73, 28, 198, 53}, fuente, TEXTO);
        rotulo(dc, L"N O C H E", {74, 51, 198, 70}, fuenteChica, SECUNDARIO);
        rotulo(dc, L"EXPLORADOR BINARIO", {20, 102, 198, 123}, fuenteChica, SECUNDARIO);
        rotulo(dc, L"ESTRUCTURA DEL ARCHIVO", {20, 218, 198, 237}, fuenteChica, SECUNDARIO);
        rotulo(dc, L"TIP", {20, alto - 143, 190, alto - 122}, fuenteChica, ACENTO);
        rotulo(dc, L"Tambien podes arrastrar", {20, alto - 117, 194, alto - 94}, fuenteChica, SECUNDARIO);
        rotulo(dc, L"un archivo a esta ventana.", {20, alto - 96, 194, alto - 73}, fuenteChica, SECUNDARIO);
        rotulo(dc, L"LOCAL / SOLO LECTURA", {20, alto - 44, 196, alto - 21}, fuenteChica, ACENTO);
        rotulo(dc, L"WORKSPACE / INSPECTOR", {x, 23, derecha, 44}, fuenteChica, ACENTO);
        rotulo(dc, L"Tus datos, en claro.", {x, 49, derecha, 92}, fuenteGrande, TEXTO);
        rotulo(dc, nombreCorto(), {x, 96, derecha, 119}, fuente, SECUNDARIO);
    }
    else
    {
        rotulo(dc, L"Archivo", {x, 17, 180, 52}, fuenteTitulo, TEXTO);
        rotulo(dc, L"/  PAPEL", {151, 20, 300, 49}, fuenteChica, SECUNDARIO);
        panel(dc, {derecha - 184, 20, derecha, 49}, LATERAL, BORDE, 24);
        rotulo(dc, L"LOCAL  /  SOLO LECTURA", {derecha - 180, 20, derecha - 4, 49}, fuenteChica, TEXTO,
               DT_CENTER | DT_SINGLELINE | DT_VCENTER);
        rotulo(dc, L"Cada registro cuenta.", {x, 67, derecha - 190, 110}, fuenteGrande, TEXTO);
        rotulo(dc, L"Explora tus archivos con una mirada mas clara.", {x, 112, derecha, 134}, fuente, SECUNDARIO);
        rotulo(dc, L"Formato", {x, 253, x + 75, 282}, fuente, SECUNDARIO);
    }
    int yTarjetas = ES_NOCHE ? 136 : 155;
    int espacio = 16, tarjetaAncho = (derecha - x - espacio * 2) / 3;
    const wchar_t *etiquetas[] = {formato == HEXADECIMAL ? L"BLOQUES DE 16 BYTES" : L"REGISTROS", L"TAMANO DEL ARCHIVO", L"ESTRUCTURA"};
    const wchar_t *tipos[] = {L"Hex / ASCII", L"Comanda", L"Historica", L"Mozo", L"Producto"};
    long long cantidadAnimada = static_cast<long long>(contadorDesde + (contadorHasta - contadorDesde) * suavizar(progreso));
    wstring valores[] = {archivo == nullptr ? L"--" : to_wstring(cantidadAnimada),
                        archivo == nullptr ? L"--" : to_wstring(tamanoArchivo) + L" B", tipos[formato]};
    for (int i = 0; i < 3; i++)
    {
        int izquierda = x + i * (tarjetaAncho + espacio);
        int fin = i == 2 ? derecha : izquierda + tarjetaAncho;
        panel(dc, {izquierda, yTarjetas, fin, yTarjetas + 86}, SUPERFICIE, BORDE);
        rotulo(dc, etiquetas[i], {izquierda + 18, yTarjetas + 11, fin - 12, yTarjetas + 31}, fuenteChica, SECUNDARIO);
        rotulo(dc, valores[i], {izquierda + 18, yTarjetas + 32, fin - 12, yTarjetas + 73},
               i == 2 ? fuenteTitulo : fuenteGrande, i == 0 ? ACENTO : TEXTO);
    }
    int yTabla = ES_NOCHE ? 286 : 334;
    panel(dc, {x - 1, yTabla - 2, derecha + 1, alto - 98}, SUPERFICIE, BORDE, 10);
    wchar_t paginaActual[256] = {}, estadoActual[1024] = {};
    GetWindowTextW(paginaTexto, paginaActual, 256); GetWindowTextW(estado, estadoActual, 1024);
    rotulo(dc, paginaActual, {x + 238, alto - 76, derecha, alto - 42}, fuenteChica, SECUNDARIO);
    rotulo(dc, estadoActual, {x, alto - 31, derecha, alto - 10}, fuenteChica, SECUNDARIO);
}

void pintarVentana()
{
    PAINTSTRUCT pintura;
    HDC dc = BeginPaint(ventana, &pintura);
    RECT cliente; GetClientRect(ventana, &cliente);
    if (cliente.right > 0 && cliente.bottom > 0)
    {
        HDC memoria = CreateCompatibleDC(dc);
        HBITMAP imagen = CreateCompatibleBitmap(dc, cliente.right, cliente.bottom);
        HGDIOBJ previa = SelectObject(memoria, imagen);
        dibujarFondo(memoria);
        BitBlt(dc, 0, 0, cliente.right, cliente.bottom, memoria, 0, 0, SRCCOPY);
        SelectObject(memoria, previa); DeleteObject(imagen); DeleteDC(memoria);
    }
    EndPaint(ventana, &pintura);
}

void dibujarControl(const DRAWITEMSTRUCT &item)
{
    if (item.CtlID == FORMATOS)
    {
        HBRUSH fondo = CreateSolidBrush((item.itemState & ODS_SELECTED) ? LATERAL : SUPERFICIE);
        FillRect(item.hDC, &item.rcItem, fondo); DeleteObject(fondo);
        if (item.itemID != static_cast<UINT>(-1))
        {
            wchar_t nombre[128] = {};
            SendMessageW(formatos, CB_GETLBTEXT, item.itemID, reinterpret_cast<LPARAM>(nombre));
            RECT rect = item.rcItem; rect.left += 10;
            rotulo(item.hDC, nombre, rect, fuente, TEXTO);
        }
        return;
    }
    double hover = 0;
    for (int i = 0; i < 4; i++) if (botones[i].control == item.hwndItem) hover = botones[i].hover;
    bool principal = item.CtlID == ABRIR, desactivado = (item.itemState & ODS_DISABLED) != 0;
    COLORREF base = principal ? ACENTO : SUPERFICIE;
    COLORREF color = mezclar(base, principal ? TEXTO : ACENTO, hover * 0.15);
    if (item.itemState & ODS_SELECTED) color = mezclar(color, FONDO, 0.20);
    if (desactivado) color = mezclar(SUPERFICIE, FONDO, 0.5);
    HBRUSH fondoBoton = CreateSolidBrush(ES_NOCHE && (item.CtlID == ABRIR || item.CtlID == RECARGAR) ? LATERAL : FONDO);
    FillRect(item.hDC, &item.rcItem, fondoBoton); DeleteObject(fondoBoton);
    RECT rect = item.rcItem; rect.right--; rect.bottom--;
    panel(item.hDC, rect, color, principal ? color : BORDE, 12);
    wchar_t nombre[128] = {}; GetWindowTextW(item.hwndItem, nombre, 128);
    COLORREF letra = principal ? (ES_NOCHE ? RGB(8, 39, 34) : RGB(255, 253, 248)) : TEXTO;
    rotulo(item.hDC, nombre, rect, fuente, desactivado ? SECUNDARIO : letra, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    if ((item.itemState & ODS_FOCUS) && !(item.itemState & ODS_NOFOCUSRECT))
    {
        InflateRect(&rect, -5, -5); DrawFocusRect(item.hDC, &rect);
    }
}

void dibujarCabecera(HWND cabecera, HDC dc)
{
    RECT cliente; GetClientRect(cabecera, &cliente);
    HBRUSH fondo = CreateSolidBrush(LATERAL); FillRect(dc, &cliente, fondo); DeleteObject(fondo);
    int cantidad = Header_GetItemCount(cabecera);
    for (int i = 0; i < cantidad; i++)
    {
        RECT rect; Header_GetItemRect(cabecera, i, &rect);
        wchar_t nombre[128] = {};
        HDITEMW columna = {}; columna.mask = HDI_TEXT; columna.pszText = nombre; columna.cchTextMax = 128;
        Header_GetItem(cabecera, i, &columna);
        rect.left += 10;
        rotulo(dc, nombre, rect, fuenteChica, SECUNDARIO);
    }
}

LRESULT CALLBACK cabeceraVisual(HWND hwnd, UINT mensaje, WPARAM wParam, LPARAM lParam, UINT_PTR id, DWORD_PTR)
{
    if (mensaje == WM_ERASEBKGND) return 1;
    if (mensaje == WM_PAINT)
    {
        PAINTSTRUCT pintura; HDC dc = BeginPaint(hwnd, &pintura);
        dibujarCabecera(hwnd, dc); EndPaint(hwnd, &pintura); return 0;
    }
    if (mensaje == WM_PRINTCLIENT)
    {
        dibujarCabecera(hwnd, reinterpret_cast<HDC>(wParam)); return 0;
    }
    if (mensaje == WM_NCDESTROY) RemoveWindowSubclass(hwnd, cabeceraVisual, id);
    return DefSubclassProc(hwnd, mensaje, wParam, lParam);
}

LRESULT decorarTabla(NMHDR *notificacion)
{
    if (notificacion->code != NM_CUSTOMDRAW) return 0;
    if (notificacion->hwndFrom == tabla)
    {
        NMLVCUSTOMDRAW *dibujo = reinterpret_cast<NMLVCUSTOMDRAW *>(notificacion);
        if (dibujo->nmcd.dwDrawStage == CDDS_PREPAINT) return CDRF_NOTIFYITEMDRAW;
        if (dibujo->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
        {
            bool seleccionado = ListView_GetItemState(tabla, static_cast<int>(dibujo->nmcd.dwItemSpec), LVIS_SELECTED) != 0;
            dibujo->clrText = TEXTO;
            dibujo->clrTextBk = seleccionado ? mezclar(SUPERFICIE, ACENTO, ES_NOCHE ? 0.18 : 0.12) :
                (dibujo->nmcd.dwItemSpec % 2 == 0 ? SUPERFICIE : mezclar(SUPERFICIE, FONDO, 0.48));
            dibujo->nmcd.uItemState &= ~CDIS_SELECTED;
            return CDRF_NEWFONT;
        }
    }
    if (notificacion->hwndFrom == ListView_GetHeader(tabla))
    {
        NMCUSTOMDRAW *dibujo = reinterpret_cast<NMCUSTOMDRAW *>(notificacion);
        if (dibujo->dwDrawStage == CDDS_PREPAINT) return CDRF_NOTIFYITEMDRAW;
        if (dibujo->dwDrawStage == CDDS_ITEMPREPAINT)
        {
            RECT rect = dibujo->rc;
            HBRUSH fondo = CreateSolidBrush(LATERAL); FillRect(dibujo->hdc, &rect, fondo); DeleteObject(fondo);
            wchar_t nombre[128] = {};
            HDITEMW columna = {}; columna.mask = HDI_TEXT; columna.pszText = nombre; columna.cchTextMax = 128;
            Header_GetItem(notificacion->hwndFrom, static_cast<int>(dibujo->dwItemSpec), &columna);
            rect.left += 12;
            rotulo(dibujo->hdc, nombre, rect, fuenteChica, SECUNDARIO);
            return CDRF_SKIPDEFAULT;
        }
    }
    return CDRF_DODEFAULT;
}

void inicializarEstilo()
{
    SystemParametersInfoW(SPI_GETCLIENTAREAANIMATION, 0, &permitirAnimacion, 0);
    HDC dc = GetDC(ventana); int dpi = GetDeviceCaps(dc, LOGPIXELSY); ReleaseDC(ventana, dc);
    fuenteGrande = CreateFontW(-MulDiv(27, dpi, 72), 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                              DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
    fuenteChica = CreateFontW(-MulDiv(9, dpi, 72), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
    fuenteMono = CreateFontW(-MulDiv(10, dpi, 72), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                            DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Consolas");
    pincelSuperficie = CreateSolidBrush(SUPERFICIE);
    HWND ocultos[] = {titulo, ayuda, etiquetaFormato, paginaTexto, estado};
    for (int i = 0; i < 5; i++) ShowWindow(ocultos[i], SW_HIDE);
    SetWindowTextW(abrir, ES_NOCHE ? L"+  Abrir archivo" : L"+  Importar archivo");
    SetWindowTextW(recargar, L"Recargar");
    SetWindowTheme(tabla, L"", L""); SetWindowTheme(ListView_GetHeader(tabla), L"", L"");
    SetWindowSubclass(ListView_GetHeader(tabla), cabeceraVisual, 1, 0);
    SetWindowTheme(ruta, L"", L"");
    SetWindowLongPtrW(ruta, GWL_STYLE, GetWindowLongPtrW(ruta, GWL_STYLE) & ~WS_BORDER);
    SetWindowPos(ruta, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
    ListView_SetBkColor(tabla, SUPERFICIE); ListView_SetTextBkColor(tabla, SUPERFICIE); ListView_SetTextColor(tabla, TEXTO);
    SendMessageW(tabla, WM_SETFONT, reinterpret_cast<WPARAM>(fuenteMono), TRUE);
    altoFilas = ImageList_Create(1, 30, ILC_COLOR32, 1, 1);
    ListView_SetImageList(tabla, altoFilas, LVSIL_SMALL);
    SendMessageW(formatos, CB_SETITEMHEIGHT, static_cast<WPARAM>(-1), 25);
    SendMessageW(formatos, CB_SETDROPPEDWIDTH, 325, 0);
    HWND listaBotones[] = {abrir, recargar, anterior, siguiente};
    for (int i = 0; i < 4; i++)
    {
        botones[i].control = listaBotones[i];
        SetWindowTheme(listaBotones[i], L"", L"");
        SetWindowSubclass(listaBotones[i], hoverBoton, i, 0);
    }
    vacio = crearControl(L"STATIC", L"", SS_CENTER);
    SendMessageW(vacio, WM_SETFONT, reinterpret_cast<WPARAM>(fuenteTitulo), TRUE);
    RECT cliente; GetClientRect(ventana, &cliente); distribuirControles(cliente.right, cliente.bottom);
    animarContenido();
}

void liberarEstilo()
{
    KillTimer(ventana, 1);
    if (altoFilas != nullptr) ImageList_Destroy(altoFilas);
    if (pincelSuperficie != nullptr) DeleteObject(pincelSuperficie);
    if (fuenteGrande != nullptr) DeleteObject(fuenteGrande);
    if (fuenteChica != nullptr) DeleteObject(fuenteChica);
    if (fuenteMono != nullptr) DeleteObject(fuenteMono);
}

LRESULT CALLBACK procesarMensaje(HWND hwnd, UINT mensaje, WPARAM wParam, LPARAM lParam)
{
    switch (mensaje)
    {
    case WM_CREATE:
    {
        ventana = hwnd;
        int dpi = 96;
        HDC contexto = GetDC(hwnd);
        if (contexto != nullptr)
        {
            dpi = GetDeviceCaps(contexto, LOGPIXELSY);
            ReleaseDC(hwnd, contexto);
        }
        fuente = CreateFontW(-MulDiv(10, dpi, 72), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
        fuenteTitulo = CreateFontW(-MulDiv(17, dpi, 72), 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                                   DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
        titulo = crearControl(L"STATIC", L"Lector de archivos binarios", 0);
        SendMessageW(titulo, WM_SETFONT, reinterpret_cast<WPARAM>(fuenteTitulo), TRUE);
        ayuda = crearControl(L"STATIC", L"Seleccione la estructura del archivo o use hexadecimal para inspeccionar sus bytes.", 0);
        abrir = crearControl(L"BUTTON", L"Abrir .dat...", WS_TABSTOP | BS_PUSHBUTTON, ABRIR);
        etiquetaFormato = crearControl(L"STATIC", L"Formato:", 0);
        formatos = crearControl(L"COMBOBOX", L"", WS_TABSTOP | CBS_DROPDOWNLIST | CBS_OWNERDRAWFIXED | CBS_HASSTRINGS | WS_VSCROLL, FORMATOS);
        const wchar_t *opciones[] = {L"Hexadecimal / ASCII", L"Comandas", L"Historicas", L"Mozos", L"Inventario"};
        for (int i = 0; i < 5; i++)
        {
            wstring opcion = opciones[i];
            SendMessageW(formatos, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(opcion.c_str()));
        }
        SendMessageW(formatos, CB_SETCURSEL, HEXADECIMAL, 0);
        recargar = crearControl(L"BUTTON", L"Recargar", WS_TABSTOP | BS_PUSHBUTTON, RECARGAR);
        ruta = crearControl(L"EDIT", L"Ningun archivo abierto", ES_READONLY | ES_AUTOHSCROLL | WS_BORDER, 0);
        tabla = crearControl(WC_LISTVIEWW, L"", WS_TABSTOP | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL, TABLA);
        ListView_SetExtendedListViewStyle(tabla, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
        anterior = crearControl(L"BUTTON", L"< Anterior", WS_TABSTOP, ANTERIOR);
        siguiente = crearControl(L"BUTTON", L"Siguiente >", WS_TABSTOP, SIGUIENTE);
        paginaTexto = crearControl(L"STATIC", L"", 0);
        estado = crearControl(L"STATIC", L"", SS_PATHELLIPSIS);
        crearColumnas();
        mostrarPagina();
        inicializarEstilo();
        DragAcceptFiles(hwnd, TRUE);
        return 0;
    }
    case WM_ERASEBKGND: return 1;
    case WM_PAINT: pintarVentana(); return 0;
    case WM_PRINTCLIENT: dibujarFondo(reinterpret_cast<HDC>(wParam)); return 0;
    case WM_TIMER: if (wParam == 1) actualizarAnimacion(); return 0;
    case WM_DRAWITEM: dibujarControl(*reinterpret_cast<DRAWITEMSTRUCT *>(lParam)); return TRUE;
    case WM_MEASUREITEM:
        reinterpret_cast<MEASUREITEMSTRUCT *>(lParam)->itemHeight = 30;
        return TRUE;
    case WM_NOTIFY: return decorarTabla(reinterpret_cast<NMHDR *>(lParam));
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX:
        SetTextColor(reinterpret_cast<HDC>(wParam), reinterpret_cast<HWND>(lParam) == vacio ? SECUNDARIO : TEXTO);
        SetBkColor(reinterpret_cast<HDC>(wParam), SUPERFICIE);
        return reinterpret_cast<LRESULT>(pincelSuperficie);
    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED) distribuirControles(LOWORD(lParam), HIWORD(lParam));
        return 0;
    case WM_GETMINMAXINFO:
        reinterpret_cast<MINMAXINFO *>(lParam)->ptMinTrackSize = {1040, 660};
        return 0;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case ABRIR: elegirArchivo(); break;
        case RECARGAR:
            if (archivo != nullptr) abrirArchivo(nombreArchivo, false);
            break;
        case FORMATOS:
            if (HIWORD(wParam) == CBN_SELCHANGE)
            {
                formato = static_cast<Formato>(SendMessageW(formatos, CB_GETCURSEL, 0, 0));
                pagina = 0;
                crearColumnas();
                mostrarPagina();
            }
            break;
        case ANTERIOR:
            if (pagina > 0) { pagina--; mostrarPagina(); }
            break;
        case SIGUIENTE:
            if (pagina + 1 < cantidadPaginas()) { pagina++; mostrarPagina(); }
            break;
        }
        return 0;
    case WM_DROPFILES:
    {
        HDROP soltado = reinterpret_cast<HDROP>(wParam);
        UINT largo = DragQueryFileW(soltado, 0, nullptr, 0);
        vector<wchar_t> direccion(largo + 1);
        if (DragQueryFileW(soltado, 0, direccion.data(), largo + 1))
            abrirArchivo(direccion.data());
        DragFinish(soltado);
        return 0;
    }
    case WM_DESTROY:
        liberarEstilo();
        if (archivo != nullptr) { fclose(archivo); archivo = nullptr; }
        if (fuente != nullptr) DeleteObject(fuente);
        if (fuenteTitulo != nullptr) DeleteObject(fuenteTitulo);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, mensaje, wParam, lParam);
}
} // namespace LectorVisual

int WINAPI wWinMain(HINSTANCE instancia, HINSTANCE, PWSTR, int mostrar)
{
    INITCOMMONCONTROLSEX controles = {sizeof(controles), ICC_LISTVIEW_CLASSES};
    if (!InitCommonControlsEx(&controles)) return 1;
    WNDCLASSW clase = {};
    clase.lpfnWndProc = LectorVisual::procesarMensaje;
    clase.hInstance = instancia;
    clase.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    clase.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    clase.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    clase.lpszClassName = ES_NOCHE ? L"TP2026LectorNoche" : L"TP2026LectorPapel";
    if (!RegisterClassW(&clase)) return 1;
    HWND ventana = CreateWindowExW(0, clase.lpszClassName, ES_NOCHE ? L"Noche - DAT Studio" : L"Papel - Archivo DAT",
                                   WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, 1240, 800,
                                   nullptr, nullptr, instancia, nullptr);
    if (ventana == nullptr) return 1;
    ShowWindow(ventana, mostrar);
    UpdateWindow(ventana);

    int cantidadArgumentos = 0;
    LPWSTR *argumentos = CommandLineToArgvW(GetCommandLineW(), &cantidadArgumentos);
    if (argumentos != nullptr)
    {
        if (cantidadArgumentos > 1) LectorVisual::abrirArchivo(argumentos[1]);
        LocalFree(argumentos);
    }
    MSG mensaje = {};
    int resultado;
    while ((resultado = GetMessageW(&mensaje, nullptr, 0, 0)) > 0)
    {
        if (!IsDialogMessageW(ventana, &mensaje))
        {
            TranslateMessage(&mensaje);
            DispatchMessageW(&mensaje);
        }
    }
    return resultado == -1 ? 1 : static_cast<int>(mensaje.wParam);
}
