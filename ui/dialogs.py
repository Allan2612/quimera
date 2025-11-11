from PyQt5.QtWidgets import QDialog, QVBoxLayout, QHBoxLayout, QPushButton, QTextEdit, QApplication, QMessageBox
from PyQt5.QtGui import QFont, QPalette, QColor
from PyQt5.QtCore import Qt
from ui.syntax_highlighter import QuimeraHighlighter


# === Subventana para mostrar ejemplos o código ===
class CodePreviewDialog(QDialog):
    def __init__(self, title: str, code: str, parent=None):
        super().__init__(parent)
        self.setWindowTitle(title)
        self.setMinimumSize(500, 400)

        layout = QVBoxLayout()
        self.setLayout(layout)

        # --- Editor de solo lectura con highlighter ---
        self.code_view = QTextEdit()
        self.code_view.setReadOnly(True)
        self.code_view.setFont(QFont("Consolas", 10))
        self.code_view.setPlainText(code)
        self.code_view.setLineWrapMode(QTextEdit.NoWrap)

        # 🌈 Aplicar la paleta oscura
        palette = QPalette()
        palette.setColor(QPalette.Base, QColor(43, 43, 43))
        palette.setColor(QPalette.Text, QColor(220, 220, 220))
        self.code_view.setPalette(palette)

        # 🌈 Activar el resaltador de Quimera
        self.highlighter = QuimeraHighlighter(self.code_view.document())

        layout.addWidget(self.code_view)

        # --- Botonera inferior ---
        button_layout = QHBoxLayout()
        layout.addLayout(button_layout)

        copy_button = QPushButton("📋 Copiar código")
        close_button = QPushButton("Cerrar")

        button_layout.addStretch()
        button_layout.addWidget(copy_button)
        button_layout.addWidget(close_button)

        copy_button.clicked.connect(self.copy_code)
        close_button.clicked.connect(self.close)

    def copy_code(self):
        clipboard = QApplication.clipboard()
        clipboard.setText(self.code_view.toPlainText())


# === Ventana “Acerca de” ===
def show_about(parent):
    about = QMessageBox(parent)
    about.setWindowTitle("Acerca de Quimera IDE")
    about.setTextFormat(Qt.RichText)
    about.setText(
        "<h3>Quimera IDE</h3>"
        "<p>Versión 1.0 — Proyecto académico para el curso Paradigmas de Programación</p>"
        "<p><b>Autores:</b> Raúl Bejarano Mora & Allan Vargas Torres</p>"
        "<p><b>Profesor:</b> Josías Ariel Chaves Murillo</p>"
        "<p><b>Universidad Nacional de Costa Rica — Sede Regional Brunca</b></p>"
    )
    about.setStandardButtons(QMessageBox.Ok)
    about.exec_()
