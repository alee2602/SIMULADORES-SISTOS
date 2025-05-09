# SIMULADORES-SISTOS

Inicializar wsl
```bash
wsl
```

Instalar SDL2 como interfaz gráfica
```bash
sudo apt update
sudo apt install libsdl2-dev libsdl2-ttf-dev
```

Compilar el proyecto
```bash
g++ main.cpp -o simulator -lSDL2 -lSDL2_ttf
```
Ejecutar el proyecto
```bash
./simulator
```