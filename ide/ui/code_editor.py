from PyQt5.QtWidgets import QPlainTextEdit, QWidget, QTextEdit
from PyQt5.QtGui import QColor, QPainter, QFont, QTextFormat, QTextCursor
from PyQt5.QtCore import Qt, QRect, QSize


class LineNumberArea(QWidget):
    """Área lateral que muestra los números de línea."""
    def __init__(self, editor):
        super().__init__(editor)
        self.code_editor = editor

    def sizeHint(self):
        return QSize(self.code_editor.line_number_area_width(), 0)

    def paintEvent(self, event):
        self.code_editor.line_number_area_paint_event(event)


class CodeEditor(QPlainTextEdit):
    """
    Editor de código con:
    - Números de línea
    - Resaltado de línea actual (azul tenue)
    - Resaltado de línea con error (rojo tenue)
    """
    def __init__(self, parent=None):
        super().__init__(parent)

        # Configuración base
        self.setFont(QFont("Consolas", 10))
        self.setLineWrapMode(QPlainTextEdit.NoWrap)
        self.setStyleSheet("""
            QPlainTextEdit {
                background-color: #2b2b2b;
                color: #dddddd;
                border: 1px solid #3c3c3c;
                border-radius: 4px;
            }
        """)

        # Área de números de línea
        self.line_number_area = LineNumberArea(self)

        # Línea de error (None = sin error marcado, 1-based)
        self._error_line = None

        # Conexiones para actualizar números y resaltado
        self.blockCountChanged.connect(self.update_line_number_area_width)
        self.updateRequest.connect(self.update_line_number_area)
        self.cursorPositionChanged.connect(self._update_highlight_selections)
        self.textChanged.connect(self._on_text_changed)

        # Inicializar márgenes y resaltado
        self.update_line_number_area_width(0)
        self._update_highlight_selections()

    # ==========================================================
    #   NÚMEROS DE LÍNEA
    # ==========================================================
    def line_number_area_width(self):
        """Calcula el ancho necesario para mostrar los números de línea."""
        digits = len(str(max(1, self.blockCount())))
        # 10px de padding + ancho de '9' * dígitos
        space = 10 + self.fontMetrics().width('9') * digits
        return space

    def update_line_number_area_width(self, _):
        """Ajusta el margen izquierdo para dejar espacio al área de líneas."""
        self.setViewportMargins(self.line_number_area_width(), 0, 0, 0)

    def update_line_number_area(self, rect, dy):
        """Actualiza/redibuja el área de números de línea cuando se hace scroll o cambia el texto."""
        if dy:
            self.line_number_area.scroll(0, dy)
        else:
            self.line_number_area.update(
                0, rect.y(),
                self.line_number_area.width(),
                rect.height()
            )

        if rect.contains(self.viewport().rect()):
            self.update_line_number_area_width(0)

    def resizeEvent(self, event):
        """Reposiciona el área de números de línea al cambiar el tamaño del editor."""
        super().resizeEvent(event)
        cr = self.contentsRect()
        self.line_number_area.setGeometry(
            QRect(cr.left(), cr.top(),
                  self.line_number_area_width(), cr.height())
        )

    def line_number_area_paint_event(self, event):
        """Dibuja los números de línea."""
        painter = QPainter(self.line_number_area)
        painter.fillRect(event.rect(), QColor(40, 40, 40))  # fondo gris oscuro

        block = self.firstVisibleBlock()
        block_number = block.blockNumber()
        top = int(self.blockBoundingGeometry(block).translated(self.contentOffset()).top())
        bottom = top + int(self.blockBoundingRect(block).height())

        while block.isValid() and top <= event.rect().bottom():
            if block.isVisible() and bottom >= event.rect().top():
                number = str(block_number + 1)
                painter.setPen(QColor(130, 130, 130))
                painter.drawText(
                    0, top,
                    self.line_number_area.width() - 4,
                    self.fontMetrics().height(),
                    Qt.AlignRight,
                    number
                )

            block = block.next()
            top = bottom
            bottom = top + int(self.blockBoundingRect(block).height())
            block_number += 1

    # ==========================================================
    #   RESALTADO DE LÍNEAS (ACTUAL + ERROR)
    # ==========================================================
    def _update_highlight_selections(self):
        """
        Crea las selecciones extra:
        - Línea actual (azul tenue)
        - Línea con error (rojo tenue), si hay.
        """
        extra_selections = []

        # 1) Línea actual (donde está el cursor)
        if not self.isReadOnly():
            selection = QTextEdit.ExtraSelection()
            line_color = QColor(45, 45, 60)  # azul/gris tenue
            selection.format.setBackground(line_color)
            selection.format.setProperty(QTextFormat.FullWidthSelection, True)

            cursor = self.textCursor()
            cursor.clearSelection()
            selection.cursor = cursor

            extra_selections.append(selection)

        # 2) Línea con error (si hay)
        if self._error_line is not None and self._error_line > 0:
            block = self.document().findBlockByNumber(self._error_line - 1)
            if block.isValid():
                error_sel = QTextEdit.ExtraSelection()
                error_color = QColor(80, 30, 30)  # rojo muy tenue
                error_sel.format.setBackground(error_color)
                error_sel.format.setProperty(QTextFormat.FullWidthSelection, True)

                c = QTextCursor(block)
                c.clearSelection()
                error_sel.cursor = c

                extra_selections.append(error_sel)

        self.setExtraSelections(extra_selections)

    def highlight_current_line(self):
        """API simple por si la quieres llamar desde fuera."""
        self._update_highlight_selections()

    # ---------- API para marcar / limpiar línea con error ----------
    def highlight_error_line(self, line_number: int | None):
        """
        Marca una línea como línea de error (1-based).
        Si line_number es None o <= 0, limpia el error.
        """
        if not line_number or line_number <= 0:
            self._error_line = None
        else:
            self._error_line = int(line_number)
        self._update_highlight_selections()

    def clear_error_highlight(self):
        """Limpia manualmente el resaltado de error."""
        if self._error_line is not None:
            self._error_line = None
            self._update_highlight_selections()

    def _on_text_changed(self):
        """
        Cuando el usuario modifica el texto, eliminamos
        el resaltado de error (para que no se quede "pegado").
        """
        if self._error_line is not None:
            self._error_line = None
            self._update_highlight_selections()

    # ==========================================================
    #   API COMPATIBLE CON IDEWindow
    # ==========================================================
    def set_error_line(self, line: int):
        """Compatibilidad: usado por IDEWindow._handle_quimera_line."""
        self.highlight_error_line(line)

    def clear_error_line(self):
        """Compatibilidad: usado por IDEWindow._run y _clear_code."""
        self.clear_error_highlight()

    def goto_line(self, line: int):
        """Compatibilidad: mueve el cursor a una línea (1-based) y centra."""
        if not line or line <= 0:
            return

        cursor = self.textCursor()
        cursor.movePosition(QTextCursor.Start)
        for _ in range(line - 1):
            if not cursor.movePosition(QTextCursor.Down):
                break
        cursor.movePosition(QTextCursor.StartOfLine)
        self.setTextCursor(cursor)
        self.centerCursor()
