# 1. Versión mínima de CMake (3.20 es un estándar seguro y moderno, 4.0 no existe aún comercialmente)
cmake_minimum_required(VERSION 3.20)

# 2. Nombre del proyecto
project(EstuMercado-Gestion-Escolar-CPP)

# 3. Estándar de C++ (Usamos el 17 o 14, ambos sirven para este proyecto)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 4. Configuración del ejecutable
# Sintaxis: add_executable(NombreDelEjecutable ArchivoFuente.cpp)
add_executable(EstuMercado Main.cpp)