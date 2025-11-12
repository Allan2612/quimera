import os, sys, subprocess
from PyQt5.QtWidgets import (
    QMainWindow, QWidget, QVBoxLayout, QHBoxLayout,
    QPushButton, QTextEdit, QMenuBar, QAction, QSplitter,
    QMessageBox, QFileDialog
)
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QIcon

from ui.dialogs import CodePreviewDialog, show_about
from ui.syntax_highlighter import QuimeraHighlighter
from ui.code_editor import CodeEditor

# ================================================================================================================================
#  CONFIGURACIÓN (EDITAS SOLO ESTO)
# ================================================================================================================================
QUIMERA_PROJECT_DIR = r"C:\Users\alanv\Desktop\Allan\Trabajos\Paradigmas\Nueva carpeta\Quimera_Lenguage"  # Carpeta del proyecto Quimera (donde está el Makefile)
MSYS_ROOT = r"C:\msys64"                                                                    # Ruta raíz de MSYS2 
# ================================================================================================================================
def _win_to_msys(path: str) -> str:
    """C:\foo\bar -> /c/foo/bar (para bash de MSYS2)"""
    p = os.path.abspath(path)
    drive, rest = os.path.splitdrive(p)  # "C:", "\Users\..."
    drive = drive[:-1].lower()
    rest = rest.replace("\\", "/")
    return f"/{drive}{rest}"

class IDEWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Quimera IDE")
        self.setGeometry(200, 100, 900, 600)
        self.setWindowIcon(QIcon("resources/icons/icono.png"))

        # === MENÚ SUPERIOR ===
        menu_bar = self.menuBar()
        self._create_menus(menu_bar)

        # === COMPONENTES PRINCIPALES ===
        self.editor = CodeEditor()
        self.editor.setPlaceholderText("Escribe tu código Quimera aquí...")
        self.highlighter = QuimeraHighlighter(self.editor.document())

        self.output = QTextEdit()
        self.output.setReadOnly(True)
        self.output.setPlaceholderText("Salida de compilación / tests...")

        # === BOTONES INFERIORES ===
        self.compile_button = QPushButton("Compilar")  # ahora ejecuta `make test`
        self.run_button = QPushButton("Ejecutar")
        self.clear_code_button = QPushButton("Limpiar código")
        self.clear_output_button = QPushButton("Limpiar salida")

        self.compile_button.clicked.connect(self._compile)
        self.run_button.clicked.connect(self._run)
        self.clear_code_button.clicked.connect(self._clear_code)
        self.clear_output_button.clicked.connect(self._clear_output)

        # Layout principal
        central_widget = QWidget()
        self.setCentralWidget(central_widget)

        # Layout principal (QVBoxLayout) que tendrá dos secciones
        main_layout = QVBoxLayout()  
        central_widget.setLayout(main_layout)

        # Primera sección: QHBoxLayout para editor a la izquierda y output a la derecha
        hbox_layout = QHBoxLayout()  # QHBoxLayout para horizontal (input y output)
        hbox_layout.addWidget(self.editor)
        hbox_layout.addWidget(self.output)
        main_layout.addLayout(hbox_layout)

        # Segunda sección: QHBoxLayout para los botones (alineados horizontalmente)
        buttons_layout = QHBoxLayout()  # QHBoxLayout para los botones horizontales
        buttons_layout.addWidget(self.compile_button)
        buttons_layout.addWidget(self.run_button)
        buttons_layout.addWidget(self.clear_code_button)
        buttons_layout.addWidget(self.clear_output_button)
        
        # Agregar el layout de botones al final
        main_layout.addLayout(buttons_layout)

        self.statusBar().showMessage("Listo")

    def _create_menus(self, menu_bar: QMenuBar):
        # === PALABRAS RESERVADAS ===
        words_menu = menu_bar.addMenu("Palabras reservadas")
        words_action = QAction("Ver todas", self)
        words_action.triggered.connect(self._show_reserved_words)
        words_menu.addAction(words_action)

        # === SINTAXIS ===
        syntax_menu = menu_bar.addMenu("Sintaxis")

        # --- Submenú CONTROL ---
        control_menu = syntax_menu.addMenu("Control de flujo")

        control_menu.addAction(
            "If / ElseIf / Else",
            lambda: self._show_example(
                "Condicional If / ElseIf / Else",
                (
                    "If (x > 0)\n"
                    "    Print(\"Positivo\");\n"
                    "ElseIf (x == 0)\n"
                    "    Print(\"Neutro\");\n"
                    "Else\n"
                    "    Print(\"Negativo\");\n"
                    "End\n"
                ),
            ),
        )

        control_menu.addAction(
            "While",
            lambda: self._show_example(
                "Bucle While",
                (
                    "Integer i = 0;\n"
                    "While (i < 5)\n"
                    "    Print(i);\n"
                    "    i = i + 1;\n"
                    "End\n"
                ),
            ),
        )

        control_menu.addAction(
            "For",
            lambda: self._show_example(
                "Bucle For",
                (
                    "For (i = 0 : i < 10 : i = i + 1)\n"
                    "    Print(i);\n"
                    "End\n"
                ),
            ),
        )

        # --- Submenú FUNCIONES ---
        func_menu = syntax_menu.addMenu("Funciones")

        func_menu.addAction(
            "Declaración básica",
            lambda: self._show_example(
                "Función sin retorno",
                (
                    "Function Saludo()\n"
                    "    Print(\"Hola desde Quimera!\");\n"
                    "End\n"
                ),
            ),
        )

        func_menu.addAction(
            "Función con retorno",
            lambda: self._show_example(
                "Función con retorno",
                (
                    "Function Suma(Integer a, Integer b)\n"
                    "    Return a + b;\n"
                    "End\n"
                ),
            ),
        )

        # --- Submenú OPERACIONES ---
        op_menu = syntax_menu.addMenu("Operaciones")

        op_menu.addAction(
            "Aritméticas",
            lambda: self._show_example(
                "Operaciones aritméticas",
                (
                    "Integer a = 5 + 2;\n"
                    "Integer b = a * 3;\n"
                    "Float c = b / 2.0;\n"
                    "Print(c);\n"
                ),
            ),
        )

        op_menu.addAction(
            "Lógicas",
            lambda: self._show_example(
                "Operaciones lógicas",
                (
                    "Boolean activo = True;\n"
                    "If (activo And Not terminado)\n"
                    "    Print(\"En ejecución\");\n"
                    "End\n"
                ),
            ),
        )

        # === TIPOS DE DATOS ===
        types_menu = menu_bar.addMenu("Tipos de datos")

        types_menu.addAction(
            "Simples",
            lambda: self._show_example(
                "Tipos simples",
                (
                    "Tipos básicos soportados:\n"
                    "- Integer: números enteros.\n"
                    "- Float: números decimales.\n"
                    "- Char: un solo carácter.\n"
                    "- String: texto.\n"
                    "- Boolean: valores lógicos (True / False).\n"
                    "- Null: valor nulo o indefinido.\n"
                ),
            ),
        )

        types_menu.addAction(
            "Compuestos",
            lambda: self._show_example(
                "Tipos compuestos",
                (
                    "Tipos compuestos de Quimera:\n"
                    "- List: colección dinámica de elementos.\n"
                    "- Matrix: estructura bidimensional de datos.\n"
                    "\nEjemplo:\n"
                    "List numeros = [1, 2, 3, 4];\n"
                    "Print(numeros[2]);\n"
                ),
            ),
        )

        # === AYUDA / ACERCA DE ===
        help_menu = menu_bar.addMenu("Ayuda")
        about_action = QAction("Acerca de Quimera", self)
        about_action.triggered.connect(lambda: show_about(self))
        help_menu.addAction(about_action)

    def _show_example(self, title, code):
        dialog = CodePreviewDialog(title, code, self)
        dialog.exec_()
        self.statusBar().showMessage(f"Ejemplo: {title}")

    # =========================
    #  Acciones principales
    # =========================
    def _safe_log(self, text: str):
        try:
            self.output.append(text)
        except Exception:
            print(text, flush=True)

    def _compile(self):
        """Botón Compilar: ejecuta `make test` en QUIMERA_PROJECT_DIR"""
        proj = QUIMERA_PROJECT_DIR
        if not proj or not os.path.isdir(proj):
            self._safe_log(f"Carpeta de proyecto inválida: {proj}\n")
            return

        # --- Windows + MSYS2 ---
        if sys.platform.startswith("win") and MSYS_ROOT:
            bash = os.path.join(MSYS_ROOT, "usr", "bin", "bash.exe")
            env = os.environ.copy()
            env.setdefault("MSYSTEM", "UCRT64")
            env["CHERE_INVOKING"] = "1"
            env["PATH"] = os.pathsep.join([
                os.path.join(MSYS_ROOT, "ucrt64", "bin"),
                os.path.join(MSYS_ROOT, "usr", "bin"),
                env.get("PATH", "")
            ])
            msys_proj = _win_to_msys(proj)
            cmd = [bash, "-lc", f'cd "{msys_proj}" && make test']
            self._safe_log(f"Ejecutando (MSYS2): bash -lc 'cd {msys_proj} && make test'\n")
            cp = subprocess.run(cmd, capture_output=True, text=True, encoding="utf-8", env=env)
        else:
            # --- Linux / macOS (o Windows sin MSYS2) ---
            self._safe_log("Ejecutando: make test\n")
            cp = subprocess.run(["make", "test"], cwd=proj,
                                capture_output=True, text=True, encoding="utf-8")

        if cp.stdout:
            self._safe_log(cp.stdout.rstrip() + "\n")
        if cp.stderr:
            self._safe_log(cp.stderr.rstrip() + "\n")

        if cp.returncode == 0:
            self._safe_log("Tests OK (make test)\n")
            self.statusBar().showMessage("Tests OK")
        else:
            self._safe_log(f"Tests FALLARON (code={cp.returncode})\n")
            self.statusBar().showMessage("Tests fallaron")

    def _run(self):
        proj = QUIMERA_PROJECT_DIR
        if not proj or not os.path.isdir(proj):
            self._safe_log(f"Carpeta de proyecto inválida: {proj}\n")
            return

        # 1) Guardar el texto del editor en tmp/ide_input.qmr dentro del repo C
        tmp_dir = os.path.join(proj, "tmp")
        os.makedirs(tmp_dir, exist_ok=True)
        src_path = os.path.join(tmp_dir, "ide_input.qmr")
        with open(src_path, "w", encoding="utf-8") as f:
            f.write(self.editor.toPlainText())

        # 2) Ejecutar make (Windows + MSYS2)
        if not MSYS_ROOT or not os.path.isdir(MSYS_ROOT):
            self._safe_log("MSYS2 no configurado. Define MSYS_ROOT = r\"C:\\msys64\".\n")
            return

        bash = os.path.join(MSYS_ROOT, "usr", "bin", "bash.exe")
        env = os.environ.copy()
        env.setdefault("MSYSTEM", "UCRT64")
        env["CHERE_INVOKING"] = "1"
        env["PATH"] = os.pathsep.join([
            os.path.join(MSYS_ROOT, "ucrt64", "bin"),
            os.path.join(MSYS_ROOT, "usr", "bin"),
            env.get("PATH", "")
        ])

        # Conversión C:\...\a\Rutas -> /c/... para bash
        def to_msys(p: str) -> str:
            d, r = os.path.splitdrive(os.path.abspath(p))
            return f"/{d[:-1].lower()}{r.replace('\\', '/')}"

        msys_proj = to_msys(proj)
        msys_src  = to_msys(src_path)

        # 2a) Compilar CLI (asegura bin/Quimera actualizado)
        cp1 = subprocess.run(
            [bash, "-lc", f'cd "{msys_proj}" && make cli'],
            capture_output=True, text=True, encoding="utf-8", env=env
        )
        if cp1.returncode != 0:
            self._safe_log(f"Falló la construcción del CLI (code={cp1.returncode})\n")
            self.statusBar().showMessage("CLI falló al compilar")
            return

        # 2b) Ejecutar lexer con dump de tokens
        cp2 = subprocess.run(
            [bash, "-lc", f'cd "{msys_proj}" && make run-lex-dump FILE="{msys_src}"'],
            capture_output=True, text=True, encoding="utf-8", env=env
        )

        # === 3) Filtrar y mostrar solo errores importantes o el output del programa ===
        errors_found = False

        # Filtrar errores semánticos/interpretación
        for line in cp2.stdout.splitlines():
            if "error" in line.lower():  # Buscamos la palabra "error" en cualquier lugar
                self._safe_log(line.strip())  # Solo muestra el error
                errors_found = True

        # Mostrar el output generado por el programa (lo que hizo el `print()`)
        output_found = False
        for line in cp2.stdout.splitlines():
            if "error" not in line.lower():  # Ignorar las líneas de error
                self._safe_log(line.strip())  # Mostrar el output real del programa
                output_found = True

        # Si no se encontró error, muestra "Ejecución exitosa"
        if cp2.returncode == 0 and not errors_found and output_found:
            self.statusBar().showMessage("Ejecución OK")
        else:
            self._safe_log(f"Ejecución con errores (code={cp2.returncode})\n")
            self.statusBar().showMessage("Ejecución con errores")




    def _clear_code(self):
        if QMessageBox.question(self, "Confirmar", "¿Deseas limpiar el código?",
                                QMessageBox.Yes | QMessageBox.No) == QMessageBox.Yes:
            self.editor.clear()
            self.statusBar().showMessage("Código limpiado")

    def _clear_output(self):
            self.output.clear()
            self.statusBar().showMessage("Salida limpiada")

    def _show_reserved_words(self):
        text = (
            "If, ElseIf, Else, For, While, End, Function, Return, Void,\n"
            "Integer, Float, Char, String, Boolean, Null, List, Print, Read."
        )
        self._show_example("Palabras reservadas", text)
