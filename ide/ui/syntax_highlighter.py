from PyQt5.QtGui import QSyntaxHighlighter, QTextCharFormat, QColor, QFont
from PyQt5.QtCore import QRegExp


class QuimeraHighlighter(QSyntaxHighlighter):
    """
    Coloreado de sintaxis completo del lenguaje Quimera (sensible a mayúsculas).
    """

    def __init__(self, document):
        super().__init__(document)
        self.rules = []

        # === Helper para crear formatos ===
        def make_format(color, bold=False, italic=False):
            fmt = QTextCharFormat()
            fmt.setForeground(QColor(color))
            if bold:
                fmt.setFontWeight(QFont.Bold)
            if italic:
                fmt.setFontItalic(True)
            return fmt

        # === PALETA (la misma del sitio quimera.avt.codes, variante oscura) ===
        fmt_control   = make_format("#C7A6F7", bold=True)   # If ElseIf Else End While For Return
        fmt_io        = make_format("#8CC4EA", bold=True)   # Print Read
        fmt_logic     = make_format("#EF9FCF", bold=True)   # And Or Not
        fmt_type      = make_format("#A3D39F")              # Integer Float String Char Boolean List
        fmt_atom      = make_format("#F3B57C")              # True False Null
        fmt_string    = make_format("#E8C47C")
        fmt_number    = make_format("#F4A8A1")
        fmt_comment   = make_format("#93928C", italic=True)
        fmt_operator  = make_format("#A5A49F")
        fmt_function  = make_format("#F1F0EC", bold=True)  # llamadas
        fmt_func_decl = make_format("#F1F0EC", bold=True)  # nombre declarado
        fmt_method    = make_format("#7FD8CD")              # .append .remove .size

        def add_words(words, fmt):
            for w in words:
                regex = QRegExp(rf"\b{w}\b")  # palabra completa, sensible a mayúsculas
                regex.setPatternSyntax(QRegExp.RegExp2)
                self.rules.append((regex, fmt))

        add_words(["If", "ElseIf", "Else", "End", "While", "For", "Return"], fmt_control)
        add_words(["Print", "Read"], fmt_io)
        add_words(["And", "Or", "Not"], fmt_logic)
        add_words(["Integer", "Float", "String", "Char", "Boolean", "List"], fmt_type)
        add_words(["True", "False", "Null"], fmt_atom)

        method_regex = QRegExp(r"\.(append|remove|size)\b")
        method_regex.setPatternSyntax(QRegExp.RegExp2)
        self.rules.append((method_regex, fmt_method))

        # === Operadores ===
        for op in [r"\+", "-", r"\*", "/", "=", "==", "!=", "<", ">", "<=", ">="]:
            r = QRegExp(op)
            r.setPatternSyntax(QRegExp.RegExp2)
            self.rules.append((r, fmt_operator))

        # === Cadenas ===
        self.rules.append((QRegExp(r'"[^"\\]*(\\.[^"\\]*)*"'), fmt_string))
        self.rules.append((QRegExp(r"'[^'\\]*(\\.[^'\\]*)*'"), fmt_string))

        # === Números ===
        self.rules.append((QRegExp(r"\b[0-9]+(\.[0-9]+)?\b"), fmt_number))

        # === Comentarios ===
        self.rules.append((QRegExp(r"//[^\n]*"), fmt_comment))
        self.rules.append((QRegExp(r"#.*"), fmt_comment))
        self.comment_start = QRegExp(r"/\*")
        self.comment_end = QRegExp(r"\*/")
        self.comment_format = fmt_comment

        # === Funciones declaradas (Function Nombre) ===
        self.func_decl_regex = QRegExp(r"\bFunction\s+([A-Za-z_][A-Za-z0-9_]*)")
        self.func_decl_regex.setPatternSyntax(QRegExp.RegExp2)

        # === Funciones llamadas (identificador seguido de paréntesis, que no sean palabras reservadas) ===
        func_call_regex = QRegExp(
            r"\b(?!If\b|ElseIf\b|Else\b|While\b|For\b|Function\b|Return\b)"
            r"[A-Za-z_][A-Za-z0-9_]*(?=\s*\()"
        )
        func_call_regex.setPatternSyntax(QRegExp.RegExp2)
        self.rules.append((func_call_regex, fmt_function))

        self.fmt_func_decl = fmt_func_decl

    # === Aplicar las reglas de coloreado ===
    def highlightBlock(self, text):
        # Aplicar reglas normales
        for pattern, fmt in self.rules:
            index = pattern.indexIn(text)
            while index >= 0:
                length = pattern.matchedLength()
                self.setFormat(index, length, fmt)
                index = pattern.indexIn(text, index + length)

        # Colorear nombre de función declarado después de "Function"
        start_index = self.func_decl_regex.indexIn(text)
        if start_index >= 0:
            func_name = self.func_decl_regex.cap(1)
            if func_name:
                name_index = text.index(func_name, start_index)
                self.setFormat(name_index, len(func_name), self.fmt_func_decl)

        # Comentarios multilínea
        self.setCurrentBlockState(0)
        start = 0
        if self.previousBlockState() != 1:
            start = self.comment_start.indexIn(text)
        while start >= 0:
            end = self.comment_end.indexIn(text, start)
            if end == -1:
                self.setCurrentBlockState(1)
                length = len(text) - start
            else:
                length = end - start + self.comment_end.matchedLength()
            self.setFormat(start, length, self.comment_format)
            start = self.comment_start.indexIn(text, start + length)
