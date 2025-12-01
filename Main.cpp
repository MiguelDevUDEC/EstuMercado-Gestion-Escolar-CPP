/*
    PROYECTO FINAL: Gestión de Tienda Escolar "EstuMercado"
    Versión: 2.0 (Con interfaz a color)
*/

#include <iostream>
#include <fstream>
#include <cstring>
#include <iomanip>
#include <ctime>
#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

// --- COLORES ANSI ---
const char* RESET   = "\033[0m";
const char* ROJO    = "\033[31m";
const char* VERDE   = "\033[32m";
const char* AMARILLO = "\033[33m";
const char* CYAN    = "\033[36m";
const char* BOLD    = "\033[1m";

void configurarConsola() {
#ifdef _WIN32
    // Obtiene el control de la salida de la consola
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;

    // Habilita el modo de "Secuencias Virtuales" (Colores ANSI)
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return;

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);

    // Configura la codificación para que salgan tildes y ñ (UTF-8)
    SetConsoleOutputCP(CP_UTF8);
#endif
}

struct Estudiante {
    char cedula[15];
    char nombre[50];
    char grado[10];
    double saldo;
    bool eliminado;
};

struct Producto {
    char codigo[15];
    char nombre[50];
    double precio;
    int stock;
};

struct Compra {
    char fecha[20];
    char nombreProducto[50];
    double valor;
    char cedulaEstudiante[15];
};

// Prototipos
void menuPrincipal();
void registrarEstudiante();
void agregarProducto();
void realizarCompra();
void recargarSaldo();
void eliminarEstudiante();
void menuConsultas();
bool existeEstudiante(const char* cedula);
bool existeProducto(const char* codigo);
void obtenerFechaActual(char* buffer);
void reporteEstudianteIndividual();
void reporteProductos();
void reporteBajoSaldo();
void generarReporteRetiros(Estudiante est);

const char* FILE_ESTUDIANTES = "estudiantes.dat";
const char* FILE_PRODUCTOS = "productos.dat";
const char* FILE_COMPRAS = "compras.dat";
const char* FILE_RETIROS = "retiros.txt";

int main() {
    configurarConsola();
    cout << fixed << setprecision(0);
    menuPrincipal();
    return 0;
}

void menuPrincipal() {
    int opcion;
    do {
        // system("cls"); // Descomenta si usas Windows CMD antiguo
        cout << "\n" << BOLD << CYAN << "=== TIENDA ESCOLAR 'ESTUMERCADO' ===" << RESET << endl;
        cout << "1. Registrar estudiante" << endl;
        cout << "2. Agregar producto al inventario" << endl;
        cout << "3. Realizar compra" << endl;
        cout << "4. Recargar saldo estudiante" << endl;
        cout << "5. Eliminar estudiante" << endl;
        cout << "6. Consultas y Reportes" << endl;
        cout << ROJO << "7. Salir" << RESET << endl;
        cout << "Seleccione una opcion: ";
        cin >> opcion;
        cin.ignore();

        switch(opcion) {
            case 1: registrarEstudiante(); break;
            case 2: agregarProducto(); break;
            case 3: realizarCompra(); break;
            case 4: recargarSaldo(); break;
            case 5: eliminarEstudiante(); break;
            case 6: menuConsultas(); break;
            case 7: cout << AMARILLO << "Saliendo del sistema..." << RESET << endl; break;
            default: cout << ROJO << "Opcion no valida." << RESET << endl;
        }
    } while(opcion != 7);
}

void registrarEstudiante() {
    ofstream archivo(FILE_ESTUDIANTES, ios::binary | ios::app);
    Estudiante est;

    cout << "\n" << CYAN << "--- REGISTRO DE ESTUDIANTE ---" << RESET << endl;
    cout << "Cedula: "; cin.getline(est.cedula, 15);

    if (existeEstudiante(est.cedula)) {
        cout << ROJO << "[!] Error: Ya existe un estudiante con esa cedula." << RESET << endl;
        return;
    }

    cout << "Nombre Completo: "; cin.getline(est.nombre, 50);
    cout << "Grado: "; cin.getline(est.grado, 10);

    do {
        cout << "Saldo Inicial (min $5000): ";
        cin >> est.saldo;
        if(est.saldo < 5000) cout << AMARILLO << "El saldo debe ser mayor o igual a 5000.\n" << RESET;
    } while (est.saldo < 5000);

    est.eliminado = false;
    archivo.write(reinterpret_cast<char*>(&est), sizeof(Estudiante));
    archivo.close();
    cout << VERDE << ">> Estudiante registrado exitosamente." << RESET << endl;
}

void agregarProducto() {
    ofstream archivo(FILE_PRODUCTOS, ios::binary | ios::app);
    Producto prod;

    cout << "\n" << CYAN << "--- NUEVO PRODUCTO ---" << RESET << endl;
    cout << "Codigo: "; cin.getline(prod.codigo, 15);

    if (existeProducto(prod.codigo)) {
        cout << ROJO << "[!] Error: Ya existe un producto con este codigo." << RESET << endl;
        return;
    }

    cout << "Nombre del Producto: "; cin.getline(prod.nombre, 50);
    cout << "Precio: "; cin >> prod.precio;
    cout << "Cantidad en Stock: "; cin >> prod.stock;

    archivo.write(reinterpret_cast<char*>(&prod), sizeof(Producto));
    archivo.close();
    cout << VERDE << ">> Producto agregado al inventario." << RESET << endl;
}

void realizarCompra() {
    fstream fEst(FILE_ESTUDIANTES, ios::binary | ios::in | ios::out);
    fstream fProd(FILE_PRODUCTOS, ios::binary | ios::in | ios::out);
    ofstream fCompra(FILE_COMPRAS, ios::binary | ios::app);

    if(!fEst.is_open() || !fProd.is_open()) {
        cout << ROJO << "Error: Faltan archivos de datos." << RESET << endl;
        return;
    }

    char cedula[15], codigo[15];
    Estudiante est;
    Producto prod;
    bool estEncontrado = false, prodEncontrado = false;
    long posEst = 0, posProd = 0;

    cout << "\n" << CYAN << "--- REALIZAR COMPRA ---" << RESET << endl;

    cout << "Cedula del Estudiante: "; cin.getline(cedula, 15);
    while(fEst.read(reinterpret_cast<char*>(&est), sizeof(Estudiante))) {
        if(strcmp(est.cedula, cedula) == 0) {
            estEncontrado = true;
            posEst = fEst.tellg();
            posEst -= sizeof(Estudiante);
            break;
        }
    }

    if(!estEncontrado) { cout << ROJO << "[!] Estudiante no encontrado." << RESET << endl; return; }

    cout << "Codigo del Producto: "; cin.getline(codigo, 15);
    while(fProd.read(reinterpret_cast<char*>(&prod), sizeof(Producto))) {
        if(strcmp(prod.codigo, codigo) == 0) {
            prodEncontrado = true;
            posProd = fProd.tellg();
            posProd -= sizeof(Producto);
            break;
        }
    }

    if(!prodEncontrado) { cout << ROJO << "[!] Producto no encontrado." << RESET << endl; return; }

    if(prod.stock <= 0) {
        cout << ROJO << "[!] Error: Producto agotado." << RESET << endl;
        return;
    }
    if(est.saldo < prod.precio) {
        cout << ROJO << "[!] Error: Saldo insuficiente. Saldo actual: $" << est.saldo << RESET << endl;
        return;
    }

    est.saldo -= prod.precio;
    fEst.seekp(posEst);
    fEst.write(reinterpret_cast<char*>(&est), sizeof(Estudiante));

    prod.stock--;
    fProd.seekp(posProd);
    fProd.write(reinterpret_cast<char*>(&prod), sizeof(Producto));

    Compra nuevaCompra;
    obtenerFechaActual(nuevaCompra.fecha);
    strcpy(nuevaCompra.nombreProducto, prod.nombre);
    nuevaCompra.valor = prod.precio;
    strcpy(nuevaCompra.cedulaEstudiante, est.cedula);

    fCompra.write(reinterpret_cast<char*>(&nuevaCompra), sizeof(Compra));

    cout << VERDE << ">> Compra exitosa! Nuevo saldo: $" << est.saldo << RESET << endl;

    fEst.close(); fProd.close(); fCompra.close();
}

void recargarSaldo() {
    fstream archivo(FILE_ESTUDIANTES, ios::binary | ios::in | ios::out);
    if(!archivo.is_open()) return;

    char cedula[15];
    double monto;
    Estudiante est;
    bool encontrado = false;

    cout << "\n" << CYAN << "--- RECARGAR SALDO ---" << RESET << endl;
    cout << "Cedula: "; cin.getline(cedula, 15);

    while(archivo.read(reinterpret_cast<char*>(&est), sizeof(Estudiante))) {
        if(strcmp(est.cedula, cedula) == 0) {
            encontrado = true;
            cout << "Valor a recargar (max $500,000): "; cin >> monto;
            if(monto > 500000 || monto <= 0) {
                cout << ROJO << "[!] Error: Monto invalido." << RESET << endl;
                return;
            }

            est.saldo += monto;
            long pos = archivo.tellg();
            archivo.seekp(pos - sizeof(Estudiante));
            archivo.write(reinterpret_cast<char*>(&est), sizeof(Estudiante));

            cout << VERDE << ">> Recarga exitosa. Nuevo saldo: $" << est.saldo << RESET << endl;
            break;
        }
    }

    if(!encontrado) cout << ROJO << "[!] Estudiante no encontrado." << RESET << endl;
    archivo.close();
}

void eliminarEstudiante() {
    ifstream archivoOriginal(FILE_ESTUDIANTES, ios::binary);
    ofstream archivoTemp("temp.dat", ios::binary);
    if(!archivoOriginal.is_open()) return;

    char cedula[15];
    Estudiante est;
    bool encontrado = false;

    cout << "\n" << ROJO << "--- ELIMINAR ESTUDIANTE ---" << RESET << endl;
    cout << "Cedula a eliminar: "; cin.getline(cedula, 15);

    while(archivoOriginal.read(reinterpret_cast<char*>(&est), sizeof(Estudiante))) {
        if(strcmp(est.cedula, cedula) == 0) {
            encontrado = true;
            generarReporteRetiros(est);
            cout << VERDE << ">> Estudiante eliminado y enviado a reporte de retiros." << RESET << endl;
        } else {
            archivoTemp.write(reinterpret_cast<char*>(&est), sizeof(Estudiante));
        }
    }

    archivoOriginal.close();
    archivoTemp.close();
    remove(FILE_ESTUDIANTES);
    rename("temp.dat", FILE_ESTUDIANTES);

    if(!encontrado) cout << AMARILLO << "[!] No se encontro la cedula." << RESET << endl;
}

void generarReporteRetiros(Estudiante est) {
    ofstream archivo(FILE_RETIROS, ios::app);
    archivo << "Cedula: " << est.cedula << " | Nombre: " << est.nombre << " | Saldo devuelto: $" << est.saldo << endl;
    archivo.close();
}

void menuConsultas() {
    int op;
    cout << "\n" << CYAN << "--- CONSULTAS ---" << RESET << endl;
    cout << "1. Consultar un estudiante" << endl;
    cout << "2. Listar todos los productos" << endl;
    cout << "3. Estudiantes con saldo < $5000" << endl;
    cout << "4. Ver reporte de retiros" << endl;
    cout << "Opcion: "; cin >> op; cin.ignore();

    switch(op) {
        case 1: reporteEstudianteIndividual(); break;
        case 2: reporteProductos(); break;
        case 3: reporteBajoSaldo(); break;
        case 4: cout << VERDE << "Reporte en: " << FILE_RETIROS << RESET << endl; break;
    }
}

void reporteEstudianteIndividual() {
    ifstream fEst(FILE_ESTUDIANTES, ios::binary);
    ifstream fComp(FILE_COMPRAS, ios::binary);
    char cedula[15];
    Estudiante est;
    Compra comp;
    bool encontrado = false;

    cout << "Ingrese Cedula: "; cin.getline(cedula, 15);

    while(fEst.read(reinterpret_cast<char*>(&est), sizeof(Estudiante))) {
        if(strcmp(est.cedula, cedula) == 0) {
            cout << "\n" << BOLD << "DATOS DEL ESTUDIANTE:" << RESET << endl;
            cout << "Nombre: " << est.nombre << endl;
            cout << "Grado: " << est.grado << endl;
            cout << "Saldo: $" << est.saldo << endl;
            encontrado = true;
            break;
        }
    }

    if(!encontrado) { cout << ROJO << "Estudiante no existe." << RESET << endl; return; }

    cout << "\n" << BOLD << "HISTORIAL DE COMPRAS:" << RESET << endl;
    cout << left << setw(15) << "FECHA" << setw(30) << "PRODUCTO" << "VALOR" << endl;
    cout << "----------------------------------------------------" << endl;

    while(fComp.read(reinterpret_cast<char*>(&comp), sizeof(Compra))) {
        if(strcmp(comp.cedulaEstudiante, cedula) == 0) {
            cout << left << setw(15) << comp.fecha << setw(30) << comp.nombreProducto << "$" << comp.valor << endl;
        }
    }
}

void reporteProductos() {
    ifstream archivo(FILE_PRODUCTOS, ios::binary);
    Producto p;
    cout << "\n" << BOLD << "--- INVENTARIO ---" << RESET << endl;
    cout << left << setw(10) << "CODIGO" << setw(30) << "NOMBRE" << setw(10) << "PRECIO" << "STOCK" << endl;
    while(archivo.read(reinterpret_cast<char*>(&p), sizeof(Producto))) {
        cout << left << setw(10) << p.codigo << setw(30) << p.nombre << "$" << setw(9) << p.precio << p.stock << endl;
    }
    archivo.close();
}

void reporteBajoSaldo() {
    ifstream archivo(FILE_ESTUDIANTES, ios::binary);
    Estudiante est;
    cout << "\n" << ROJO << "--- ESTUDIANTES CON SALDO BAJO (< $5000) ---" << RESET << endl;
    while(archivo.read(reinterpret_cast<char*>(&est), sizeof(Estudiante))) {
        if(est.saldo < 5000) {
            cout << "- " << est.nombre << " (Saldo: $" << est.saldo << ")" << endl;
        }
    }
    archivo.close();
}

bool existeEstudiante(const char* cedula) {
    ifstream archivo(FILE_ESTUDIANTES, ios::binary);
    if(!archivo.is_open()) return false;
    Estudiante est;
    while(archivo.read(reinterpret_cast<char*>(&est), sizeof(Estudiante))) {
        if(strcmp(est.cedula, cedula) == 0) return true;
    }
    return false;
}

bool existeProducto(const char* codigo) {
    ifstream archivo(FILE_PRODUCTOS, ios::binary);
    if(!archivo.is_open()) return false;
    Producto prod;
    while(archivo.read(reinterpret_cast<char*>(&prod), sizeof(Producto))) {
        if(strcmp(prod.codigo, codigo) == 0) return true;
    }
    return false;
}

void obtenerFechaActual(char* buffer) {
    time_t t = time(0);
    struct tm * now = localtime(&t);
    strftime(buffer, 20, "%Y-%m-%d", now);
}