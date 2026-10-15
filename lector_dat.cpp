// Lector de archivos binarios con interfaz WinAPI. Solo abre archivos en lectura.
// MinGW (compilar por separado de los otros .cpp):
// g++ -std=c++11 -Wall -Wextra lector_dat.cpp -o lector_dat.exe -municode -mwindows -lcomctl32 -lcomdlg32 -lshell32
// Visual C++ (Developer Command Prompt):
// cl /EHsc /utf-8 lector_dat.cpp /link /SUBSYSTEM:WINDOWS

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
#endif

namespace LectorDat
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
    HWND control = CreateWindowExW(0, clase, textoControl, WS_CHILD | WS_VISIBLE | estilo,
                                  0, 0, 1, 1, ventana,
                                  reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
                                  GetModuleHandleW(nullptr), nullptr);
    SendMessageW(control, WM_SETFONT, reinterpret_cast<WPARAM>(fuente), TRUE);
    return control;
}

void distribuirControles(int ancho, int alto)
{
    MoveWindow(titulo, 16, 12, ancho - 32, 30, TRUE);
    MoveWindow(ayuda, 16, 46, ancho - 32, 22, TRUE);
    MoveWindow(abrir, 16, 78, 115, 30, TRUE);
    MoveWindow(etiquetaFormato, 150, 85, 62, 22, TRUE);
    MoveWindow(formatos, 212, 78, 340, 250, TRUE);
    MoveWindow(recargar, 568, 78, 100, 30, TRUE);
    MoveWindow(ruta, 16, 118, ancho - 32, 26, TRUE);
    MoveWindow(tabla, 16, 156, ancho - 32, alto - 234, TRUE);
    MoveWindow(anterior, 16, alto - 65, 105, 28, TRUE);
    MoveWindow(siguiente, 130, alto - 65, 105, 28, TRUE);
    MoveWindow(paginaTexto, 252, alto - 60, ancho - 268, 24, TRUE);
    MoveWindow(estado, 16, alto - 29, ancho - 32, 24, TRUE);
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
        formatos = crearControl(L"COMBOBOX", L"", WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL, FORMATOS);
        const wchar_t *opciones[] = {L"Hexadecimal / ASCII (cualquier binario)", L"Comanda", L"ComandaHistorica", L"Mozo", L"Producto"};
        const int tamanos[] = {16, sizeof(Comanda), sizeof(ComandaHistorica), sizeof(Mozo), sizeof(Producto)};
        for (int i = 0; i < 5; i++)
        {
            wstring opcion = wstring(opciones[i]) + L" (" + to_wstring(tamanos[i]) + L" bytes)";
            SendMessageW(formatos, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(opcion.c_str()));
        }
        SendMessageW(formatos, CB_SETCURSEL, HEXADECIMAL, 0);
        recargar = crearControl(L"BUTTON", L"Recargar", WS_TABSTOP | BS_PUSHBUTTON, RECARGAR);
        ruta = crearControl(L"EDIT", L"Ningun archivo abierto", ES_READONLY | ES_AUTOHSCROLL | WS_BORDER, 0);
        tabla = crearControl(WC_LISTVIEWW, L"", WS_TABSTOP | WS_BORDER | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL, TABLA);
        ListView_SetExtendedListViewStyle(tabla, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER);
        anterior = crearControl(L"BUTTON", L"< Anterior", WS_TABSTOP, ANTERIOR);
        siguiente = crearControl(L"BUTTON", L"Siguiente >", WS_TABSTOP, SIGUIENTE);
        paginaTexto = crearControl(L"STATIC", L"", 0);
        estado = crearControl(L"STATIC", L"", SS_PATHELLIPSIS);
        crearColumnas();
        mostrarPagina();
        DragAcceptFiles(hwnd, TRUE);
        return 0;
    }
    case WM_SIZE:
        distribuirControles(LOWORD(lParam), HIWORD(lParam));
        return 0;
    case WM_GETMINMAXINFO:
        reinterpret_cast<MINMAXINFO *>(lParam)->ptMinTrackSize = {920, 540};
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
        if (archivo != nullptr) { fclose(archivo); archivo = nullptr; }
        if (fuente != nullptr) DeleteObject(fuente);
        if (fuenteTitulo != nullptr) DeleteObject(fuenteTitulo);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, mensaje, wParam, lParam);
}
} // namespace LectorDat

int WINAPI wWinMain(HINSTANCE instancia, HINSTANCE, PWSTR, int mostrar)
{
    INITCOMMONCONTROLSEX controles = {sizeof(controles), ICC_LISTVIEW_CLASSES};
    if (!InitCommonControlsEx(&controles)) return 1;
    WNDCLASSW clase = {};
    clase.lpfnWndProc = LectorDat::procesarMensaje;
    clase.hInstance = instancia;
    clase.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    clase.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    clase.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    clase.lpszClassName = L"TP2026LectorDat";
    if (!RegisterClassW(&clase)) return 1;
    HWND ventana = CreateWindowExW(0, clase.lpszClassName, L"Lector DAT - Buffet Albert Einstein",
                                   WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1100, 680,
                                   nullptr, nullptr, instancia, nullptr);
    if (ventana == nullptr) return 1;
    ShowWindow(ventana, mostrar);
    UpdateWindow(ventana);

    int cantidadArgumentos = 0;
    LPWSTR *argumentos = CommandLineToArgvW(GetCommandLineW(), &cantidadArgumentos);
    if (argumentos != nullptr)
    {
        if (cantidadArgumentos > 1) LectorDat::abrirArchivo(argumentos[1]);
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
