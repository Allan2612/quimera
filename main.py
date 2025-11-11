import sys
from PyQt5.QtWidgets import QApplication
from ui.ide_window import IDEWindow
from ui.theme import apply_dark_palette

def main():
    app = QApplication(sys.argv)
    apply_dark_palette(app) 
    window = IDEWindow()
    window.show()
    sys.exit(app.exec_())

if __name__ == "__main__":
    main()
