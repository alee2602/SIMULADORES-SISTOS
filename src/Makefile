# ========== CONFIGURACIÓN ==========
CXX = g++
CXXFLAGS = -Wall -std=c++17

# Librerías necesarias
LIBS = -lSDL2 -lSDL2_ttf

# Directorios
SRC_DIR = .
OBJ_DIR = obj

# Archivos fuente
SRCS = $(wildcard $(SRC_DIR)/*.cpp)

# Archivos objeto en obj/
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

# Nombre del ejecutable
TARGET = simulator

# ========== REGLA PRINCIPAL ==========
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LIBS)

# ========== COMPILACIÓN INDIVIDUAL ==========
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ========== LIMPIEZA ==========
clean:
	rm -rf $(OBJ_DIR) $(TARGET)
