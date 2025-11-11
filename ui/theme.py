from PyQt5.QtGui import QPalette, QColor

def apply_dark_palette(app):
    dark_palette = QPalette()

    dark_palette.setColor(QPalette.Window, QColor(30, 30, 30))
    dark_palette.setColor(QPalette.WindowText, QColor(220, 220, 220))
    dark_palette.setColor(QPalette.Base, QColor(40, 40, 40))
    dark_palette.setColor(QPalette.AlternateBase, QColor(60, 60, 60))
    dark_palette.setColor(QPalette.ToolTipBase, QColor(255, 255, 220))
    dark_palette.setColor(QPalette.ToolTipText, QColor(0, 0, 0))
    dark_palette.setColor(QPalette.Text, QColor(220, 220, 220))
    dark_palette.setColor(QPalette.Button, QColor(45, 45, 45))
    dark_palette.setColor(QPalette.ButtonText, QColor(230, 230, 230))
    dark_palette.setColor(QPalette.BrightText, QColor(255, 0, 0))
    dark_palette.setColor(QPalette.Link, QColor(0, 122, 204))
    dark_palette.setColor(QPalette.Highlight, QColor(80, 120, 200))
    dark_palette.setColor(QPalette.HighlightedText, QColor(0, 0, 0))

    app.setPalette(dark_palette)

    app.setStyleSheet("""
        QMainWindow, QDialog, QMessageBox {
            background-color: #1e1e1e;
            color: #e0e0e0;
        }
        QTextEdit {
            background-color: #2b2b2b;
            color: #dddddd;
            font-family: Consolas, monospace;
            border: 1px solid #3c3c3c;
            border-radius: 4px;
        }
        QPushButton {
            background-color: #3a3a3a;
            color: #e0e0e0;
            padding: 6px 12px;
            border-radius: 5px;
        }
        QPushButton:hover {
            background-color: #505050;
        }
        QPushButton:pressed {
            background-color: #606060;
        }
        QMenuBar {
            background-color: #2d2d2d;
            color: #cccccc;
        }
        QMenuBar::item:selected {
            background: #444444;
        }
        QMenu {
            background-color: #2d2d2d;
            color: #cccccc;
            border: 1px solid #3c3c3c;
        }
        QMenu::item:selected {
            background-color: #505050;
        }
        QMessageBox QPushButton {
            background-color: #3a3a3a;
            color: #e0e0e0;
            border-radius: 4px;
            padding: 4px 10px;
        }
        QMessageBox QPushButton:hover {
            background-color: #505050;
        }
    """)
    