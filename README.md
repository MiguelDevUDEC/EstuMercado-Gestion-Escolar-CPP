# 🛒 Gestión de Tienda Escolar "EstuMercado"

> **Proyecto Final - Programación** > Ingeniería de Sistemas | Universidad de Cartagena

Este repositorio contiene el código fuente de **EstuMercado**, una aplicación de consola desarrollada en C++ para la administración eficiente de una tienda escolar. El sistema implementa **persistencia de datos mediante archivos binarios**, permitiendo gestionar estudiantes, inventario, transacciones financieras y reportes.

---

## 📋 Características Principales

El sistema cumple con los requerimientos funcionales definidos en el proyecto final, operando a través de un menú interactivo modular:

### 👤 Módulo de Estudiantes
* **Registro:** Captura de cédula, nombre, grado y saldo inicial (Validación de saldo mín. $5.000 y cédula única).
* **Gestión de Saldos:** Recargas de saldo con validación de topes (máx. $500.000).
* **Eliminación:** Borrado lógico/físico de estudiantes, generando reporte automático de retiros.

### 📦 Módulo de Inventario
* **Gestión de Productos:** Registro de productos con código único, nombre, precio y stock.
* **Validaciones:** Control de duplicados por código.

### 💰 Módulo de Transacciones
* **Sistema de Compras:** Proceso transaccional que:
    * Verifica existencia de usuario y producto.
    * Valida disponibilidad de stock y suficiencia de saldo.
    * Actualiza automáticamente el inventario y el saldo del cliente.
    * Registra la transacción en un historial histórico.

### 📊 Módulo de Consultas y Reportes
* Consulta individual de estudiante (Datos + Historial de Compras).
* Listado general de productos en inventario.
* Reporte de riesgo: Estudiantes con saldo bajo (< $5.000).
* Generación de archivo plano (`retiros.txt`) con estudiantes eliminados.

---

## 🛠️ Tecnologías y Conceptos Aplicados

Este proyecto demuestra el dominio de las siguientes competencias de programación en C++:

* **Persistencia de Datos:** Uso avanzado de la librería `<fstream>` para manipulación de archivos binarios (`.dat`) y de texto (`.txt`).
* **Estructuras de Datos:** Uso de `structs` con arreglos de caracteres (`char[]`) para asegurar la integridad en la escritura binaria.
* **Programación Modular:** División del problema en funciones específicas con paso de parámetros por valor y referencia.
* **Lógica de Negocio:** Implementación de validaciones robustas para garantizar la integridad de la información (Unicidad, rangos, estados).

---

## 🚀 Cómo Ejecutar el Proyecto

### Requisitos Previos
* Compilador compatible con C++14 o superior (G++, MinGW, Clang).
* Entorno de desarrollo recomendado: **CLion**, **Dev-C++** o **VS Code**.
* CMake 3.20+ (Opcional, si usas CLion/VS Code).

### Compilación Manual (Terminal)
Si no usas un IDE con botón de "Run", puedes compilarlo desde la terminal:

```bash
g++ Main.cpp -o EstuMercado
./EstuMercado