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

# ===== Unity =====
UNITY_DIR := $(TEST_DIR)/unity
UNITY_SRC := $(UNITY_DIR)/unity.c
UNITY_OBJ := $(OBJ_DIR)/unity/unity.o
INC      += -I $(UNITY_DIR)

# ===== Descubrir fuentes =====
# Todos los .c bajo src/
SRCS_ALL  := $(shell find $(SRC_DIR) -name '*.c')

# Biblioteca: todo src excepto main.c y (opcional) apps/*.c
SRCS_LIB  := $(filter-out $(SRC_DIR)/main.c,$(SRCS_ALL))
SRCS_LIB  := $(filter-out $(SRC_DIR)/apps/%.c,$(SRCS_LIB))

# Objeto de main
SRC_MAIN  := $(SRC_DIR)/main.c

# Solo los *_main.c serán ejecutables
TEST_RUNNERS := $(shell find $(TEST_DIR) -name '*_main.c' 2>/dev/null)

# Fuentes de test sin main (se compilan a .o y se linkean al runner)
TEST_SRCS := $(shell find $(TEST_DIR) -name '*.c' \
              -and -not -name 'unity.c' \
              -and -not -name '*_main.c' 2>/dev/null)

TEST_REL   := $(patsubst $(TEST_DIR)/%,%,$(TEST_SRCS))
TEST_OBJS  := $(addprefix $(OBJ_DIR)/tests/,$(TEST_REL:.c=.o))
TEST_BINS  := $(patsubst $(TEST_DIR)/%.c,$(BIN_DIR)/tests/%,$(TEST_RUNNERS))

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
	@fails=0; total=0; \
	for t in $(TEST_BINS); do \
	  echo "==> $$t"; "$$t"; ec=$$?; echo "   (exit=$$ec)"; \
	  if [ $$ec -ne 0 ]; then fails=$$((fails+1)); fi; \
	  total=$$((total+1)); \
	done; \
	echo "Ran $$total test executables; FAILS=$$fails"

# Targets para ejecutar solo un subconjunto de tests por patrón en el nombre
.PHONY: test-% testlex

# Uso: make test-lexer   (ejecuta solo los binarios de tests cuyo path contenga "lexer")
test-%: tests
	@pattern=$*; fails=0; total=0; \
	for t in $(TEST_BINS); do \
		echo "$$t" | grep -qi "$$pattern" || continue; \
		echo "==> $$t"; "$$t"; ec=$$?; echo "   (exit=$$ec)"; \
		if [ $$ec -ne 0 ]; then fails=$$((fails+1)); fi; \
		total=$$((total+1)); \
	done; \
		if [ $$total -eq 0 ]; then echo "No tests matched pattern '$$pattern'."; exit 1; fi; \
		echo "Ran $$total test executables matching '$$pattern'; FAILS=$$fails"


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

# Ejecutables de tests: cada *_main.c linkea con todos los test .o + lib + unity
$(BIN_DIR)/tests/%: $(OBJ_DIR)/tests/%.o $(TEST_OBJS) $(LIB) $(UNITY_OBJ)
	@$(MKDIR_P) $(dir $@)
	$(CC) $(CFLAGS) $^ -o $@

# Ejecutables de apps opcionales: bin/apps/<ruta>
$(BIN_DIR)/apps/%: $(OBJ_DIR)/apps/%.o $(LIB) | $(BIN_DIR)/apps
	@$(MKDIR_P) $(dir $@)
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

# Objeto de Unity
$(UNITY_OBJ): $(UNITY_SRC)
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
DEPS := $(OBJS_LIB:.o=.d) $(OBJ_MAIN:.o=.d) $(TEST_OBJS:.o=.d) $(APPS_OBJS:.o=.d) $(UNITY_OBJ:.o=.d)
-include $(DEPS)
