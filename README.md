

# Proyecto Quimera IDE

Este proyecto corresponde a un entorno de desarrollo integrado (IDE) para el lenguaje **Quimera**. El IDE se construye utilizando **PyQt5** para la interfaz gráfica y permite editar, compilar y ejecutar código en el lenguaje Quimera.

## Requisitos

### **Python**
Se recomienda instalar **Python 3.12**. Puedes descargarlo desde [python.org](https://www.python.org/downloads/).

### **Librerías necesarias**
Para que el proyecto funcione correctamente, se deben instalar las librerías requeridas usando **pip**.

1. **Crea un entorno virtual** (recomendado):
   ```bash
   python -m venv .venv
   ```

2. **Activa el entorno virtual**:
   - En **Windows**:
     ```bash
     .\.venv\Scripts\activate
     ```
   - En **Linux/macOS**:
     ```bash
     source .venv/bin/activate
     ```

3. **Instalar PyQt5**:
   ```bash
      pip install PyQt5
   ```

4. **Instala MSYS2** (si estás en Windows y necesitas usar herramientas como `make`):
   - Descarga desde [MSYS2](https://www.msys2.org/).
   - Instala las herramientas necesarias (`make`, `bash`, etc.):
     ```bash
     pacman -S make gcc
     ```

## Configuración del Proyecto

1. **Configuración de directorios**:
   - El archivo `QUIMERA_PROJECT_DIR` debe apuntar a la ubicación de tu proyecto Quimera, donde se encuentra el archivo **Makefile**.
   - **Ejemplo (Windows)**: 
     ```python
     QUIMERA_PROJECT_DIR = r"C:
utal\proyecto\Quimera_Lenguage"
     MSYS_ROOT = r"C:\msys64"
     ```

2. **Ejecutar el Proyecto**:
   - Después de configurar el entorno, puedes ejecutar la aplicación IDE ejecutando el siguiente comando:
     ```bash
     python main.py
     ```

## Estructura del Proyecto

- **`ui/`**: Contiene todos los archivos relacionados con la interfaz gráfica del IDE.
  - `ide_window.py`: Ventana principal del IDE.
  - `dialogs.py`: Ventanas modales (ej. para ver código).
  - `syntax_highlighter.py`: Resaltado de sintaxis del lenguaje Quimera.
  - `theme.py`: Definición del tema oscuro.
  - `code_editor.py`: Editor de código con soporte para números de línea y resaltado de la línea actual.
- **`main.py`**: Entrada principal de la aplicación.

## Problemas Comunes

1. **Faltan librerías o módulos**:
   - Asegúrate de que las dependencias estén correctamente instaladas en el entorno virtual.
   - Si no se pueden instalar, verifica la versión de Python y la compatibilidad de las librerías.

2. **Problemas al ejecutar en Windows**:
   - Verifica que **MSYS2** y **make** estén correctamente configurados. Si usas herramientas de línea de comandos como `bash`, asegúrate de tener las variables de entorno correctamente definidas.
