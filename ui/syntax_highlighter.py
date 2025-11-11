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

        # === PALETA DE COLORES (tema oscuro VSCode) ===
        fmt_keyword   = make_format("#569CD6", bold=True)   # Azul
        fmt_type      = make_format("#4EC9B0")              # Verde-azulado
        fmt_string    = make_format("#CE9178")              # Naranja
        fmt_number    = make_format("#B5CEA8")              # Verde claro
        fmt_comment   = make_format("#6A9955", italic=True) # Verde apagado
        fmt_operator  = make_format("#C586C0")              # Violeta
        fmt_function  = make_format("#DCDCAA")              # Amarillo (llamadas)
        fmt_func_decl = make_format("#FFD700", bold=True)   # Dorado (nombre declarado)
        fmt_end       = make_format("#9CDCFE", bold=True)   # Azul celeste

        # === PALABRAS RESERVADAS ===
        keywords = [
            "If", "ElseIf", "Else", "For", "While", "End",
            "Function", "Return", "Void",
            "Integer", "Float", "Char", "String", "Boolean", "Null",
            "List", "Print", "Read", "And", "Or", "Not", "True", "False"
        ]
        for kw in keywords:
            regex = QRegExp(rf"\b{kw}\b")  # sensible a mayúsculas
            regex.setPatternSyntax(QRegExp.RegExp2)
            self.rules.append((regex, fmt_keyword))

        # === END (prioridad alta) ===
        regex_end = QRegExp(r"\bEnd\b")
        regex_end.setPatternSyntax(QRegExp.RegExp2)
        self.rules.insert(0, (regex_end, fmt_end))

        # === Tipos de datos ===
        for t in ["Integer", "Float", "Char", "String", "Boolean", "List", "Matrix", "Null"]:
            regex = QRegExp(rf"\b{t}\b")
            regex.setPatternSyntax(QRegExp.RegExp2)
            self.rules.append((regex, fmt_type))

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
