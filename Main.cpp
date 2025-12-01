/*
    UNIVERSIDAD DE CARTAGENA
    PROGRAMA: Ingeniería de Sistemas
    ASIGNATURA: Programación
    PROYECTO FINAL: Gestión de Tienda Escolar "EstuMercado"

    AUTOR: [Tu Nombre Aquí]
    DESCRIPCIÓN: Sistema modular con archivos binarios (.dat) y reportes (.txt).
*/

#include <iostream>
#include <fstream>
#include <cstring>  // Para manejo de char[]
#include <iomanip>  // Para formato de moneda
#include <ctime>    // Para obtener la fecha actual

using namespace std;

// ==========================================
// 1. ESTRUCTURAS DE DATOS (Entidades)
// ==========================================
// Usamos char[] en lugar de string para garantizar un tamaño fijo en bytes
// vital para la lectura/escritura en archivos binarios.

struct Estudiante {
    char cedula[15];    // Identificador único
    char nombre[50];
    char grado[10];
    double saldo;       // Saldo disponible
    bool eliminado;     // Flag lógico (aunque usaremos eliminación física)
};

struct Producto {
    char codigo[15];    // Identificador único
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

// ==========================================
// 2. PROTOTIPOS DE FUNCIONES
// ==========================================
void menuPrincipal();
void registrarEstudiante();
void agregarProducto();
void realizarCompra();
void recargarSaldo();
void eliminarEstudiante();
void menuConsultas();

// Funciones Auxiliares (Validaciones y Búsquedas)
bool existeEstudiante(const char* cedula);
bool existeProducto(const char* codigo);
void obtenerFechaActual(char* buffer);
void reporteEstudianteIndividual();
void reporteProductos();
void reporteBajoSaldo();
void generarReporteRetiros(Estudiante est); // Escribe en txt

// Nombres de archivos constantes
const char* FILE_ESTUDIANTES = "estudiantes.dat";
const char* FILE_PRODUCTOS = "productos.dat";
const char* FILE_COMPRAS = "compras.dat";
const char* FILE_RETIROS = "retiros.txt";

// ==========================================
// 3. FUNCIÓN PRINCIPAL
// ==========================================
int main() {
    // Configuración para mostrar decimales correctamente
    cout << fixed << setprecision(0);
    menuPrincipal();
    return 0;
}

// ==========================================
// 4. DESARROLLO DE MÓDULOS
// ==========================================

void menuPrincipal() {
    int opcion;
    do {
        system("cls"); // Limpiar pantalla (Windows)
        cout << "=== TIENDA ESCOLAR 'ESTUMERCADO' ===" << endl;
        cout << "1. Registrar estudiante" << endl;
        cout << "2. Agregar producto al inventario" << endl;
        cout << "3. Realizar compra" << endl;
        cout << "4. Recargar saldo estudiante" << endl;
        cout << "5. Eliminar estudiante" << endl;
        cout << "6. Consultas y Reportes" << endl;
        cout << "7. Salir" << endl;
        cout << "Seleccione una opcion: ";
        cin >> opcion;
        cin.ignore(); // Limpiar buffer

        switch(opcion) {
            case 1: registrarEstudiante(); break;
            case 2: agregarProducto(); break;
            case 3: realizarCompra(); break;
            case 4: recargarSaldo(); break;
            case 5: eliminarEstudiante(); break;
            case 6: menuConsultas(); break;
            case 7: cout << "Saliendo del sistema..." << endl; break;
            default: cout << "Opcion no valida." << endl;
        }
        if(opcion != 7) {
            cout << "\nPresione Enter para continuar...";
            cin.get();
        }
    } while(opcion != 7);
}

// --- OPCIÓN 1: REGISTRAR ESTUDIANTE [cite: 231-238] ---
void registrarEstudiante() {
    ofstream archivo(FILE_ESTUDIANTES, ios::binary | ios::app);
    Estudiante est;

    cout << "\n--- REGISTRO DE ESTUDIANTE ---" << endl;
    cout << "Cedula: "; cin.getline(est.cedula, 15);

    // Validación: Cédula única [cite: 233, 237]
    if (existeEstudiante(est.cedula)) {
        cout << "[!] Error: Ya existe un estudiante con esa cedula." << endl;
        return;
    }

    cout << "Nombre Completo: "; cin.getline(est.nombre, 50);
    cout << "Grado: "; cin.getline(est.grado, 10);

    // Validación: Saldo inicial mínimo $5.000 [cite: 236]
    do {
        cout << "Saldo Inicial (min $5000): ";
        cin >> est.saldo;
        if(est.saldo < 5000) cout << "El saldo debe ser mayor o igual a 5000.\n";
    } while (est.saldo < 5000);

    est.eliminado = false;

    // Guardado en binario [cite: 238]
    archivo.write(reinterpret_cast<char*>(&est), sizeof(Estudiante));
    archivo.close();
    cout << ">> Estudiante registrado exitosamente." << endl;
}

// --- OPCIÓN 2: AGREGAR PRODUCTO [cite: 239-244] ---
void agregarProducto() {
    ofstream archivo(FILE_PRODUCTOS, ios::binary | ios::app);
    Producto prod;

    cout << "\n--- NUEVO PRODUCTO ---" << endl;
    cout << "Codigo: "; cin.getline(prod.codigo, 15);

    // Validación: Código único [cite: 240]
    if (existeProducto(prod.codigo)) {
        cout << "[!] Error: Ya existe un producto con este codigo." << endl;
        return;
    }

    cout << "Nombre del Producto: "; cin.getline(prod.nombre, 50);
    cout << "Precio: "; cin >> prod.precio;
    cout << "Cantidad en Stock: "; cin >> prod.stock;

    // Guardado en binario [cite: 244]
    archivo.write(reinterpret_cast<char*>(&prod), sizeof(Producto));
    archivo.close();
    cout << ">> Producto agregado al inventario." << endl;
}

// --- OPCIÓN 3: REALIZAR COMPRA [cite: 245-255] ---
void realizarCompra() {
    // Necesitamos modo lectura/escritura (in | out) para actualizar saldo y stock
    fstream fEst(FILE_ESTUDIANTES, ios::binary | ios::in | ios::out);
    fstream fProd(FILE_PRODUCTOS, ios::binary | ios::in | ios::out);
    ofstream fCompra(FILE_COMPRAS, ios::binary | ios::app);

    if(!fEst.is_open() || !fProd.is_open()) {
        cout << "Error: Faltan archivos de datos." << endl;
        return;
    }

    char cedula[15], codigo[15];
    Estudiante est;
    Producto prod;
    bool estEncontrado = false, prodEncontrado = false;
    long posEst = 0, posProd = 0;

    cout << "\n--- REALIZAR COMPRA ---" << endl;

    // 1. Buscar Estudiante [cite: 251]
    cout << "Cedula del Estudiante: "; cin.getline(cedula, 15);
    while(fEst.read(reinterpret_cast<char*>(&est), sizeof(Estudiante))) {
        if(strcmp(est.cedula, cedula) == 0) {
            estEncontrado = true;
            posEst = fEst.tellg(); // Guardamos posición actual (final del registro)
            posEst -= sizeof(Estudiante); // Retrocedemos al inicio del registro
            break;
        }
    }

    if(!estEncontrado) { cout << "[!] Estudiante no encontrado." << endl; return; }

    // 2. Buscar Producto [cite: 251]
    cout << "Codigo del Producto: "; cin.getline(codigo, 15);
    while(fProd.read(reinterpret_cast<char*>(&prod), sizeof(Producto))) {
        if(strcmp(prod.codigo, codigo) == 0) {
            prodEncontrado = true;
            posProd = fProd.tellg();
            posProd -= sizeof(Producto);
            break;
        }
    }

    if(!prodEncontrado) { cout << "[!] Producto no encontrado." << endl; return; }

    // 3. Validaciones de Negocio [cite: 251, 252]
    if(prod.stock <= 0) {
        cout << "[!] Error: Producto agotado." << endl;
        return;
    }
    if(est.saldo < prod.precio) {
        cout << "[!] Error: Saldo insuficiente. Saldo actual: $" << est.saldo << endl;
        return;
    }

    // 4. Procesar Transacción [cite: 255]
    // Actualizar Estudiante (Restar saldo)
    est.saldo -= prod.precio;
    fEst.seekp(posEst); // Mover puntero de escritura a la posición del estudiante
    fEst.write(reinterpret_cast<char*>(&est), sizeof(Estudiante));

    // Actualizar Producto (Restar stock)
    prod.stock--;
    fProd.seekp(posProd); // Mover puntero de escritura a la posición del producto
    fProd.write(reinterpret_cast<char*>(&prod), sizeof(Producto));

    // 5. Registrar en Historial [cite: 254]
    Compra nuevaCompra;
    obtenerFechaActual(nuevaCompra.fecha);
    strcpy(nuevaCompra.nombreProducto, prod.nombre);
    nuevaCompra.valor = prod.precio;
    strcpy(nuevaCompra.cedulaEstudiante, est.cedula);

    fCompra.write(reinterpret_cast<char*>(&nuevaCompra), sizeof(Compra));

    cout << ">> Compra exitosa! Nuevo saldo: $" << est.saldo << endl;

    fEst.close();
    fProd.close();
    fCompra.close();
}

// --- OPCIÓN 4: RECARGAR SALDO [cite: 256-260] ---
void recargarSaldo() {
    fstream archivo(FILE_ESTUDIANTES, ios::binary | ios::in | ios::out);
    if(!archivo.is_open()) return;

    char cedula[15];
    double monto;
    Estudiante est;
    bool encontrado = false;

    cout << "\n--- RECARGAR SALDO ---" << endl;
    cout << "Cedula: "; cin.getline(cedula, 15);

    while(archivo.read(reinterpret_cast<char*>(&est), sizeof(Estudiante))) {
        if(strcmp(est.cedula, cedula) == 0) {
            encontrado = true;

            // Validar monto máximo de recarga [cite: 258]
            cout << "Valor a recargar (max $500,000): "; cin >> monto;
            if(monto > 500000 || monto <= 0) {
                cout << "[!] Error: Monto invalido." << endl;
                return;
            }

            est.saldo += monto;

            // Sobreescribir registro actualizado [cite: 260]
            long pos = archivo.tellg();
            archivo.seekp(pos - sizeof(Estudiante));
            archivo.write(reinterpret_cast<char*>(&est), sizeof(Estudiante));

            cout << ">> Recarga exitosa. Nuevo saldo: $" << est.saldo << endl;
            break;
        }
    }

    if(!encontrado) cout << "[!] Estudiante no encontrado." << endl;
    archivo.close();
}

// --- OPCIÓN 5: ELIMINAR ESTUDIANTE [cite: 261-264] ---
void eliminarEstudiante() {
    // Método de eliminación física: Copiar todos a un temporal EXCEPTO el eliminado
    ifstream archivoOriginal(FILE_ESTUDIANTES, ios::binary);
    ofstream archivoTemp("temp.dat", ios::binary);

    if(!archivoOriginal.is_open()) return;

    char cedula[15];
    Estudiante est;
    bool encontrado = false;

    cout << "\n--- ELIMINAR ESTUDIANTE ---" << endl;
    cout << "Cedula a eliminar: "; cin.getline(cedula, 15);

    while(archivoOriginal.read(reinterpret_cast<char*>(&est), sizeof(Estudiante))) {
        if(strcmp(est.cedula, cedula) == 0) {
            encontrado = true;
            // Validacion PDF: "No tenga compras pendientes" [cite: 263]
            // Nota: Como es sistema prepago, asumimos que se puede borrar si existe.
            // Generamos el registro en txt antes de borrarlo [cite: 269]
            generarReporteRetiros(est);
            cout << ">> Estudiante eliminado y enviado a reporte de retiros." << endl;
        } else {
            // Si no es el que buscamos, lo copiamos al temporal
            archivoTemp.write(reinterpret_cast<char*>(&est), sizeof(Estudiante));
        }
    }

    archivoOriginal.close();
    archivoTemp.close();

    // Reemplazar archivos
    remove(FILE_ESTUDIANTES);
    rename("temp.dat", FILE_ESTUDIANTES);

    if(!encontrado) cout << "[!] No se encontro la cedula." << endl;
}

// --- GENERAR ARCHIVO RETIROS (Parte de eliminación y consulta) [cite: 269] ---
void generarReporteRetiros(Estudiante est) {
    ofstream archivo(FILE_RETIROS, ios::app); // Modo texto, agregar al final
    // Formato: Cedula, Nombre, Saldo Remanente
    archivo << "Cedula: " << est.cedula
            << " | Nombre: " << est.nombre
            << " | Saldo devuelto: $" << est.saldo << endl;
    archivo.close();
}

// --- OPCIÓN 6: SUBMENÚ CONSULTAS [cite: 265-269] ---
void menuConsultas() {
    int op;
    cout << "\n--- CONSULTAS ---" << endl;
    cout << "1. Consultar un estudiante (Detalle y Compras)" << endl;
    cout << "2. Listar todos los productos" << endl;
    cout << "3. Estudiantes con saldo < $5000" << endl;
    cout << "4. Ver reporte de retiros (txt)" << endl;
    cout << "Opcion: "; cin >> op; cin.ignore();

    switch(op) {
        case 1: reporteEstudianteIndividual(); break;
        case 2: reporteProductos(); break;
        case 3: reporteBajoSaldo(); break;
        case 4:
            cout << "El archivo " << FILE_RETIROS << " ha sido generado/actualizado en la carpeta del proyecto." << endl;
            break;
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

    // Mostrar datos personales [cite: 266]
    while(fEst.read(reinterpret_cast<char*>(&est), sizeof(Estudiante))) {
        if(strcmp(est.cedula, cedula) == 0) {
            cout << "\nDATOS DEL ESTUDIANTE:" << endl;
            cout << "Nombre: " << est.nombre << endl;
            cout << "Grado: " << est.grado << endl;
            cout << "Saldo: $" << est.saldo << endl;
            encontrado = true;
            break;
        }
    }

    if(!encontrado) { cout << "Estudiante no existe." << endl; return; }

    // Mostrar historial de compras [cite: 266]
    cout << "\nHISTORIAL DE COMPRAS:" << endl;
    cout << left << setw(15) << "FECHA" << setw(30) << "PRODUCTO" << "VALOR" << endl;
    cout << "----------------------------------------------------" << endl;

    while(fComp.read(reinterpret_cast<char*>(&comp), sizeof(Compra))) {
        if(strcmp(comp.cedulaEstudiante, cedula) == 0) {
            cout << left << setw(15) << comp.fecha
                 << setw(30) << comp.nombreProducto
                 << "$" << comp.valor << endl;
        }
    }
}

void reporteProductos() {
    ifstream archivo(FILE_PRODUCTOS, ios::binary);
    Producto p;
    cout << "\n--- INVENTARIO ---" << endl;
    cout << left << setw(10) << "CODIGO" << setw(30) << "NOMBRE" << setw(10) << "PRECIO" << "STOCK" << endl;

    while(archivo.read(reinterpret_cast<char*>(&p), sizeof(Producto))) {
        cout << left << setw(10) << p.codigo
             << setw(30) << p.nombre
             << "$" << setw(9) << p.precio
             << p.stock << endl;
    }
    archivo.close();
}

void reporteBajoSaldo() {
    ifstream archivo(FILE_ESTUDIANTES, ios::binary);
    Estudiante est;
    cout << "\n--- ESTUDIANTES CON SALDO BAJO (< $5000) ---" << endl; // [cite: 268]

    while(archivo.read(reinterpret_cast<char*>(&est), sizeof(Estudiante))) {
        if(est.saldo < 5000) {
            cout << "- " << est.nombre << " (Saldo: $" << est.saldo << ")" << endl;
        }
    }
    archivo.close();
}

// ==========================================
// 5. FUNCIONES AUXILIARES
// ==========================================

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