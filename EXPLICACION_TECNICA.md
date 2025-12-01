# 📘 Documentación Técnica: Sistema EstuMercado

> **Arquitectura del Sistema:** Aplicación de consola modular en C++ con persistencia de datos en archivos binarios.

Este documento detalla las decisiones de diseño, las estructuras de datos y los algoritmos utilizados para cumplir con los requerimientos del proyecto final de Programación.

---

## 1. Diseño de Datos (Structs)

Para garantizar la compatibilidad con la escritura binaria, se utilizaron estructuras de tamaño fijo.

### ¿Por qué `char[]` y no `std::string`?
En C++, `std::string` es un objeto dinámico que gestiona memoria en el Heap. Si intentamos guardar un `string` directamente en un archivo binario, guardaríamos un **puntero de memoria** (una dirección RAM) y no el texto real. Al cerrar el programa, esa dirección pierde validez.

**Solución:** Utilizamos arreglos de caracteres (`char[50]`).
* **Ventaja:** Reservan un bloque de bytes contiguos y fijos (ej: 50 bytes).
* **Resultado:** Al usar `write`, se vuelca el contenido exacto del texto al disco, permitiendo recuperarlo posteriormente sin errores.

```cpp
struct Estudiante {
    char cedula[15];    // Llave primaria
    char nombre[50];
    char grado[10];
    double saldo;       // Precisión para manejo financiero
    bool eliminado;     // Flag de estado
};
```
## 2. Persistencia Binaria (`<fstream>`)

A diferencia de los archivos de texto (`.txt`), los archivos binarios (`.dat`) almacenan la información tal como está en la memoria RAM (bytes crudos), lo que los hace más eficientes y seguros.

### Escritura y Casting
Para escribir una estructura completa en el disco, usamos la función `write`. Esta función espera un puntero de tipo `char*`.

```cpp
// reinterpret_cast: "Engaña" al compilador para tratar la estructura como un simple bloque de bytes.
archivo.write(reinterpret_cast<char*>(&estudiante), sizeof(Estudiante));
```
## 3. Algoritmos Clave (Lógica de Negocio)

### A. Actualización de Registros (Update)
Para modificar el saldo de un estudiante o el stock de un producto sin tener que borrar y volver a crear todo el archivo, utilizamos punteros de archivo:

1.  **Búsqueda:** Leemos secuencialmente (`read`) registro por registro hasta encontrar la cédula o código buscado.
2.  **Marcado (`tellg`):** Una vez encontrado, usamos `tellg()` para saber en qué byte del archivo estamos.
    * *Nota:* Como acabamos de leer, el puntero está al **final** del registro.
3.  **Retroceso (`seekp`):** Retrocedemos exactamente el tamaño de la estructura (`sizeof(Estudiante)`) para ubicar el puntero de escritura al **inicio** del registro que queremos modificar.
4.  **Sobreescritura:** Usamos `write` con los datos actualizados. Esto "chanca" (sobrescribe) los bytes viejos con los nuevos.

```cpp
long pos = archivo.tellg();              // 1. Obtener posición actual (final del registro)
archivo.seekp(pos - sizeof(Estudiante)); // 2. Retroceder un registro
archivo.write(...)                       // 3. Sobreescribir con datos nuevos
```

### B. Eliminación Física (Delete)

En archivos binarios secuenciales no es posible "borrar" un registro dejando un hueco vacío en el medio. Por ello, implementamos el algoritmo de **Archivo Temporal**, que reconstruye el archivo excluyendo el registro deseado.

**Pasos del Algoritmo:**

1.  **Apertura Dual:** Abrimos el archivo original `estudiantes.dat` en modo **Lectura** y creamos un archivo auxiliar `temp.dat` en modo **Escritura**.
2.  **Barrido Completo:** Leemos todos los registros del original uno por uno dentro de un ciclo `while`.
3.  **Filtrado Selectivo:**
    * **Si la cédula NO coincide:** Copiamos el registro intacto al archivo `temp.dat`.
    * **Si la cédula SÍ coincide:** "Ignoramos" el registro (no lo escribimos en el temporal). *En este paso aprovechamos para guardar sus datos en el reporte de texto `retiros.txt` antes de que desaparezca.*
4.  **Cierre y Reemplazo:**
    * Cerramos ambos flujos de datos.
    * Borramos el archivo original usando `remove()`.
    * Renombramos `temp.dat` a `estudiantes.dat` usando `rename()`.

```cpp
// Lógica simplificada:
while (archivoOriginal.read(...)) {
    if (strcmp(est.cedula, cedulaEliminar) != 0) {
        archivoTemp.write(...); // Copiar solo si no es el eliminado
    }
}
// Al final, el archivoTemp contiene todo MENOS el registro borrado.
```

## 4. Validaciones de Integridad (Reglas de Negocio)

El sistema no solo almacena datos, sino que garantiza su coherencia mediante validaciones estrictas implementadas antes de cualquier operación de escritura en disco:

* **Integridad Referencial:**
    * No se permite registrar una **Compra** si el *Estudiante* o el *Producto* no existen en la base de datos binaria.
* **Unicidad de Claves (Primary Keys):**
    * Al registrar un *Estudiante*, se verifica que la cédula no exista previamente.
    * Al agregar un *Producto*, se verifica que el código sea único.
* **Control de Inventario:**
    * Se impide realizar una venta si la cantidad solicitada supera el `stock` disponible (`stock <= 0`).
* **Modelo Financiero Prepago:**
    * La validación `if (estudiante.saldo < producto.precio)` asegura que **nunca existan saldos negativos**. Esto simplifica el modelo al eliminar la necesidad de gestionar cuentas por cobrar o deudas.

---

**Universidad de Cartagena - Ingeniería de Sistemas**
**Proyecto Final de Programación**