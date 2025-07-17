# Compilador y banderas comunes
CC = gcc
CFLAGS = -g -Wall -Wextra -fdiagnostics-color=always
INCLUDES = -I./include -I./src -I./resources/icons
DEFINES = -D_DEBUG

# Archivos fuente
SRC_DIR = src
SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/gui_helper.c

# Directorio de salida
BUILD_DIR = build

# Configuración específica del sistema operativo
ifeq ($(OS),Windows_NT)
    # Windows
    TARGET = $(BUILD_DIR)/angelcode_font_generator.exe
    LDFLAGS = -LC:/raylib/raylib/src -lraylib -lgdi32 -lwinmm
    RAYLIB_INCLUDE = -IC:/raylib/raylib/src
else
    # Linux/macOS
    TARGET = $(BUILD_DIR)/angelcode_font_generator
    LDFLAGS = -lraylib -lm -lpthread -ldl
    RAYLIB_INCLUDE = -I/usr/local/include
endif

# Estándar C
C_STANDARD = -std=c17

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRCS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(C_STANDARD) $(DEFINES) $(INCLUDES) $(RAYLIB_INCLUDE) $^ -o $@ $(LDFLAGS)

clean:
	@rm -rf $(BUILD_DIR)