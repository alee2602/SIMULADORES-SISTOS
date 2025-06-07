# SIMULADORES-SISTOS

Este proyecto incluye dos simuladores interactivos desarrollados con Qt que permiten visualizar conceptos fundamentales de los sistemas operativos:

- **Simulador de Calendarización de Procesos**  
- **Simulador de Sincronización (Mutex/Semáforos)**

---

## Objetivo

Brindar una herramienta visual e interactiva que permita comprender y analizar el comportamiento de los algoritmos de calendarización de procesos y los mecanismos de sincronización mediante primitivas como mutex y semáforos.

---

## Tecnologías Usadas

- **Lenguaje:** C++
- **Framework GUI:** Qt 5/6
- **Construcción:** CMake

---

## Simulador de Calendarización

### Algoritmos implementados:

- FIFO (First In, First Out)
- SJF (Shortest Job First)
- SRTF (Shortest Remaining Time First)
- Round Robin (con quantum configurable)
- Priority (con envejecimiento opcional y configurable)

### Características:

- Carga de procesos desde archivo `.txt`
- Simulación paso a paso
- Comparación de algoritmos en paralelo
- Visualización de Gantt y métricas como tiempo de espera y turnaround
- Parámetros configurables: `Quantum` y `Aging`
- Permite ejecutar múltiples algoritmos de forma secuencial para análisis comparativo

---

## Simulador de Sincronización

### Mecanismos disponibles:

- **Mutex**
- **Semáforos**

### Características:

- Visualización gráfica de procesos/hilos activos
- Representación del acceso a la sección crítica
- Opciones para elegir el mecanismo de sincronización
- Muestra del estado de los procesos (Assigned, Waiting)

## Estructura del Proyecto
```bash

SIMULADORES-SISTOS
│
│
├──────qt/
│    │   
│    │   CMakeLists.txt
│    │   ganttchartwidget.cpp
│    │   ganttchartwidget.h
│    │   loader.cpp
│    │   loader.h
│    │   main.cpp
│    │   processsimulator.cpp
│    │   processsimulator.h
│    │   scheduler.cpp
│    │   scheduler.h
│    │   synchronizer.cpp
│    │   synchronizer.h
│    │   utils.h
│    │
│    ├───data/
│    │       actions.txt
│    │       processes.txt
│    │       processes_5.txt
│    │       resources.txt
│
└── README.md

```

## Ejecución

1. Clona el repositorio
```bash
https://github.com/alee2602/SIMULADORES-SISTOS.git
```
2. Accede a la carperta 'qt'
```bash
cd qt
```

### Construcción del proyecto

3. Elimina el directorio 'build' si existe para una compilación limpia sin residuos de compilaciones anteriores.
```bash
rm -rf build
```
4. Vuelve a generar el directorio y accede a él
```bash
mkdir build && cd build
```
5. Genera los archivos de construcción con CMake
```bash
cmake ..
```
6. Compila el proyecto
```bash
make
```
7. Ejecuta el simulador
```bash
./bin/ProcessSimulator
```

## Autores
- [Paula Barillas - 22764](https://github.com/paulabaal12)
- [Mónica Salvatierra - 22249](https://github.com/alee2602)