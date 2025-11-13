import os
import sys
import subprocess
import json

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
QUIMERA_PROJECT_DIR = r"C:\Users\alanv\Desktop\Allan\Trabajos\Paradigmas\Nueva_carpeta\Quimera_Lenguage"  # Carpeta del proyecto Quimera (donde está el Makefile)
MSYS_ROOT = r"C:\msys64"  # Ruta raíz de MSYS2
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
        self.setGeometry(200, 100, 1100, 650)
        self.setWindowIcon(QIcon("resources/icons/icono.png"))

        # === MENÚ SUPERIOR ===
        menu_bar = self.menuBar()
        self._create_menus(menu_bar)

        # === EDITOR PRINCIPAL ===
        self.editor = CodeEditor()
        self.editor.setPlaceholderText("Escribe tu código Quimera aquí...")
        self.highlighter = QuimeraHighlighter(self.editor.document())

        # === SALIDAS ===
        # Arriba: resultado "limpio" del programa (solo contenido de type=output)
        self.result_output = QTextEdit()
        self.result_output.setReadOnly(True)
        self.result_output.setPlaceholderText("Resultado del programa...")

        # Abajo: consola / logs (make, errores, diagnósticos, etc.)
        self.console_output = QTextEdit()
        self.console_output.setReadOnly(True)
        self.console_output.setPlaceholderText("Consola (compilación, errores, diagnósticos)...")

        # Splitter vertical para resultado y consola
        output_splitter = QSplitter(Qt.Vertical)
        output_splitter.addWidget(self.result_output)
        output_splitter.addWidget(self.console_output)
        output_splitter.setStretchFactor(0, 1)
        output_splitter.setStretchFactor(1, 1)

        # === BOTONES INFERIORES ===
        self.compile_button = QPushButton("Compilar")  # ejecuta `make test`
        self.run_button = QPushButton("Ejecutar")
        self.clear_code_button = QPushButton("Limpiar código")
        self.clear_output_button = QPushButton("Limpiar salida")

        self.compile_button.clicked.connect(self._compile)
        self.run_button.clicked.connect(self._run)
        self.clear_code_button.clicked.connect(self._clear_code)
        self.clear_output_button.clicked.connect(self._clear_output)

        # Estilos de botones (verde y rojo opacos)
        self.run_button.setStyleSheet("""
            QPushButton {
                background-color: #6EB581;
                color: white;
                font-weight: bold;
                padding: 10px;
            }
        """)

        self.clear_code_button.setStyleSheet("""
            QPushButton {
                background-color: #F5585D;
                color: white;
                font-weight: bold;
                padding: 10px;
            }
        """)
        self.compile_button.setStyleSheet("""
            QPushButton {
                background-color: #5868F5;
                color: white;
                font-weight: bold;
                padding: 10px;
            }
        """)
        self.clear_output_button.setStyleSheet("""
            QPushButton {
                background-color: #F5B258;
                color: white;
                font-weight: bold;
                padding: 10px;
            }
        """)

        # === LAYOUT PRINCIPAL ===
        central_widget = QWidget()
        self.setCentralWidget(central_widget)

        main_layout = QVBoxLayout()
        central_widget.setLayout(main_layout)

        # Parte superior: editor y outputs en un splitter horizontal
        top_splitter = QSplitter(Qt.Horizontal)
        top_splitter.addWidget(self.editor)
        top_splitter.addWidget(output_splitter)
        top_splitter.setStretchFactor(0, 3)
        top_splitter.setStretchFactor(1, 2)

        main_layout.addWidget(top_splitter)

        # Botones
        buttons_layout = QHBoxLayout()
        buttons_layout.addWidget(self.compile_button)
        buttons_layout.addWidget(self.run_button)
        buttons_layout.addWidget(self.clear_code_button)
        buttons_layout.addWidget(self.clear_output_button)
        main_layout.addLayout(buttons_layout)

        self.statusBar().showMessage("Listo")

    # ======================================================================
    # MENÚS
    # ======================================================================
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
            "Función sin retorno",
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

    # ======================================================================
    # UTILIDADES DE MENÚ
    # ======================================================================
    def _show_example(self, title: str, code: str):
        dlg = CodePreviewDialog(title, code, parent=self)
        dlg.exec_()

    def _show_reserved_words(self):
        reserved_words = (
            "If, ElseIf, Else, For, While, End, Function, Return, Void,\n"
            "Integer, Float, Char, String, Boolean, Null, List, Print, Read."
        )
        self._safe_log(f"Palabras reservadas:\n{reserved_words}")

    # ======================================================================
    # LOGGING SEGURO
    # ======================================================================
    def _safe_log(self, text: str):
        try:
            self.console_output.append(text)
        except Exception:
            print(text, flush=True)

    def _safe_result(self, text: str):
        try:
            self.result_output.append(text)
        except Exception:
            print(text, flush=True)

    # ======================================================================
    # COMPILAR (make test)
    # ======================================================================
    def _compile(self):
        proj = QUIMERA_PROJECT_DIR
        if not proj or not os.path.isdir(proj):
            self._safe_log(f"Carpeta de proyecto inválida: {proj}\n")
            return

        # Windows + MSYS2
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
            cp = subprocess.run(
                cmd,
                cwd=proj,
                capture_output=True,
                text=True,
                encoding="utf-8"
            )
        else:
            # Linux / macOS (o Windows sin MSYS2)
            self._safe_log("Ejecutando: make test\n")
            cp = subprocess.run(
                ["make", "test"],
                cwd=proj,
                capture_output=True,
                text=True,
                encoding="utf-8"
            )

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

    # ======================================================================
    # Procesador de líneas JSON de Quimera
    # ======================================================================
    def _handle_quimera_line(self, line: str, is_error: bool = False):
        line = line.strip()
        if not line:
            return

        # Solo nos interesan las líneas que parecen JSON
        if not line.startswith("{"):
            return

        try:
            data = json.loads(line)
        except json.JSONDecodeError:
            return

        msg_type = data.get("type")

        # 1) Salida normal del programa -> va ARRIBA
        if msg_type == "output":
            content = data.get("content", "")
            self._safe_result(str(content))
            return

        # 2) Error de runtime -> va ABAJO
        if msg_type == "runtime_error":
            msg = data.get("message", "Error en tiempo de ejecución")
            self._safe_log(f"[RUNTIME] {msg}")
            return

        # 3) Diagnósticos de lexer/parser/semántico/etc. -> ABAJO
        if msg_type == "diagnostic":
            stage = data.get("stage", "unknown")
            severity = data.get("severity", "error")
            code = data.get("code", 0)
            line_no = data.get("line", 0)
            col = data.get("column", 0)
            message = data.get("message", "")

            self._safe_log(
                f"[{stage.upper()} {severity.upper()} #{code}] "
                f"L{line_no}, C{col}: {message}"
            )

            # 🔴 Resaltar línea con error en el editor
            if isinstance(line_no, int) and line_no > 0:
                self.editor.set_error_line(line_no)
                self.editor.goto_line(line_no)

            return

        # 4) Formato viejo: {"severity":"error","code":2002,} -> ABAJO
        severity = data.get("severity")
        if severity:
            code = data.get("code", "")
            message = data.get("message", "")
            line_no = data.get("line")
            if line_no is not None:
                self._safe_log(
                    f"[{severity.upper()} #{code}] L{line_no}: {message}"
                )
                # También podemos resaltar aquí si viene línea
                try:
                    ln = int(line_no)
                    if ln > 0:
                        self.editor.set_error_line(ln)
                        self.editor.goto_line(ln)
                except (TypeError, ValueError):
                    pass
            else:
                self._safe_log(f"[{severity.upper()} #{code}]: {message}")
            return

    # ======================================================================
    # Ejecutar programa (bin/Quimera --interactive)
    # ======================================================================
    def _run(self):
        proj = QUIMERA_PROJECT_DIR
        if not proj or not os.path.isdir(proj):
            self._safe_log(f"Carpeta de proyecto inválida: {proj}\n")
            return

        # Limpiamos marca de error de ejecuciones previas
        self.editor.clear_error_line()

        # 1) Guardar el texto del editor en tmp/ide_input.qmr dentro del repo C
        tmp_dir = os.path.join(proj, "tmp")
        os.makedirs(tmp_dir, exist_ok=True)
        src_path = os.path.join(tmp_dir, "ide_input.qmr")
        with open(src_path, "w", encoding="utf-8") as f:
            f.write(self.editor.toPlainText())

        # Limpiar resultado anterior (dejamos historial de consola si quieres)
        self.result_output.clear()

        # 2) Preparar entorno MSYS2
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

        msys_proj = _win_to_msys(proj)

        # 2a) Compilar CLI (asegura bin/Quimera actualizado)
        cp1 = subprocess.run(
            [bash, "-lc", f'cd "{msys_proj}" && make cli'],
            capture_output=True,
            text=True,
            encoding="utf-8",
            env=env
        )
        if cp1.stdout:
            self._safe_log(cp1.stdout.rstrip() + "\n")
        if cp1.stderr:
            self._safe_log(cp1.stderr.rstrip() + "\n")

        if cp1.returncode != 0:
            self._safe_log(f"Falló la construcción del CLI (code={cp1.returncode})\n")
            self.statusBar().showMessage("CLI falló al compilar")
            return

        # 2b) Ejecutar Quimera en modo interactivo (lee tmp/ide_input.qmr)
        cp2 = subprocess.run(
            [bash, "-lc", f'cd "{msys_proj}" && bin/Quimera --interactive'],
            capture_output=True,
            text=True,
            encoding="utf-8",
            env=env
        )

        # 3) Procesar SOLO la salida JSON de Quimera
        if cp2.stdout:
            for line in cp2.stdout.splitlines():
                self._handle_quimera_line(line, is_error=False)

        if cp2.stderr:
            for line in cp2.stderr.splitlines():
                self._handle_quimera_line(line, is_error=True)

        # 4) Actualizar barra de estado según exit code
        if cp2.returncode == 0:
            self.statusBar().showMessage("Ejecución OK")
        else:
            self.statusBar().showMessage(f"Ejecución con errores (code={cp2.returncode})")

    # ======================================================================
    # Utilidades varias
    # ======================================================================
    def _clear_code(self):
        if QMessageBox.question(
            self,
            "Confirmar",
            "¿Deseas limpiar el código?",
            QMessageBox.Yes | QMessageBox.No
        ) == QMessageBox.Yes:
            self.editor.clear()
            self.editor.clear_error_line()
            self.statusBar().showMessage("Código limpiado")

    def _clear_output(self):
        self.result_output.clear()
        self.console_output.clear()
        self.statusBar().showMessage("Salida limpiada")
