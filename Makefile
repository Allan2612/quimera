# ===== Config =====
CC      := gcc
CSTD    := -std=c11
WARN    := -Wall -Wextra -Wpedantic
DBG     := -g
OPTDBG  := -O0
OPTREL  := -O2 -DNDEBUG
INC     := -I include
DEPSFLG := -MMD -MP

SRC_DIR := src
INC_DIR := include
TEST_DIR:= tests
OBJ_DIR := obj
BIN_DIR := bin

APP     := $(BIN_DIR)/Quimera
LIB     := $(BIN_DIR)/libquimera.a

# ===== Descubrir fuentes =====
# Todos los .c bajo src/
SRCS_ALL  := $(shell find $(SRC_DIR) -name '*.c')

# Biblioteca: todo src excepto main.c y (opcional) apps/*.c
SRCS_LIB  := $(filter-out $(SRC_DIR)/main.c,$(SRCS_ALL))
SRCS_LIB  := $(filter-out $(SRC_DIR)/apps/%.c,$(SRCS_LIB))

# Objeto de main
SRC_MAIN  := $(SRC_DIR)/main.c

# Tests: cada tests/*.c produce un ejecutable en bin/tests/<nombre>
TEST_SRCS := $(shell find $(TEST_DIR) -name '*.c' 2>/dev/null)
TEST_BINS := $(patsubst $(TEST_DIR)/%.c,$(BIN_DIR)/tests/%,$(TEST_SRCS))
TEST_OBJS := $(patsubst $(TEST_DIR)/%.c,$(OBJ_DIR)/tests/%.o,$(TEST_SRCS))

# Apps opcionales: cada src/apps/*.c produce bin/apps/<nombre>
APPS_SRCS := $(shell find $(SRC_DIR)/apps -name '*.c' 2>/dev/null)
APPS_BINS := $(patsubst $(SRC_DIR)/apps/%.c,$(BIN_DIR)/apps/%,$(APPS_SRCS))
APPS_OBJS := $(patsubst $(SRC_DIR)/apps/%.c,$(OBJ_DIR)/apps/%.o,$(APPS_SRCS))

# Objetos de la lib (mapeo src/...c -> obj/src/...o)
OBJS_LIB  := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/src/%.o,$(SRCS_LIB))
OBJ_MAIN  := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/src/%.o,$(SRC_MAIN))

# ===== Flags por modo =====
DEBUG_CFLAGS   := $(CSTD) $(WARN) $(DBG) $(OPTDBG) $(INC) $(DEPSFLG)
RELEASE_CFLAGS := $(CSTD) $(WARN) $(OPTREL) $(INC) $(DEPSFLG)

# ===== Utilidades =====
RM      := rm -rf
MKDIR_P := mkdir -p

# ===== Targets públicos =====
.PHONY: all debug release run clean tests test apps

all: debug

debug: CFLAGS := $(DEBUG_CFLAGS)
debug: $(APP) $(TEST_BINS) $(APPS_BINS)

release: CFLAGS := $(RELEASE_CFLAGS)
release: $(APP) $(TEST_BINS) $(APPS_BINS)

run: debug
	$(APP)

# Compila todos los tests (sin ejecutarlos)
tests: CFLAGS := $(DEBUG_CFLAGS)
tests: $(TEST_BINS)

# Ejecuta todos los tests (si existen)
test: tests
	@set -e; \
	for t in $(TEST_BINS); do \
	  echo "==> $$t"; "$$t"; \
	done; \
	echo "OK: $(words $(TEST_BINS)) tests"

# Limpieza
clean:
	-@$(RM) $(OBJ_DIR) $(BIN_DIR)

# ===== Reglas de construcción =====
# Ejecutable principal
$(APP): $(LIB) $(OBJ_MAIN) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

# Librería estática con todo el core (sin main ni apps)
$(LIB): $(OBJS_LIB) | $(BIN_DIR)
	ar rcs $@ $^

# Ejecutables de tests: bin/tests/<name> enlaza contra la lib + su objeto
$(BIN_DIR)/tests/%: $(OBJ_DIR)/tests/%.o $(LIB) | $(BIN_DIR)/tests
	$(CC) $(CFLAGS) $^ -o $@

# Ejecutables de apps opcionales: bin/apps/<name>
$(BIN_DIR)/apps/%: $(OBJ_DIR)/apps/%.o $(LIB) | $(BIN_DIR)/apps
	$(CC) $(CFLAGS) $^ -o $@

# Compilación de .c -> .o (src, tests, apps) con creación de subcarpetas
$(OBJ_DIR)/src/%.o: $(SRC_DIR)/%.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/tests/%.o: $(TEST_DIR)/%.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/apps/%.o: $(SRC_DIR)/apps/%.c
	@$(MKDIR_P) $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Carpetas de salida
$(BIN_DIR):
	@$(MKDIR_P) $(BIN_DIR)

$(BIN_DIR)/tests: | $(BIN_DIR)
	@$(MKDIR_P) $(BIN_DIR)/tests

$(BIN_DIR)/apps: | $(BIN_DIR)
	@$(MKDIR_P) $(BIN_DIR)/apps

# Dependencias de headers
DEPS := $(OBJS_LIB:.o=.d) $(OBJ_MAIN:.o=.d) $(TEST_OBJS:.o=.d) $(APPS_OBJS:.o=.d)
-include $(DEPS)
