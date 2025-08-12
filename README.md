# Crosshair Overlay Personalizable

## Descripción

Esta es una aplicación para Windows escrita en C++ que muestra una mira (crosshair) personalizable en el centro de la pantalla. La mira es una superposición (overlay) que se mantiene visible encima de todas las demás ventanas, incluyendo juegos en pantalla completa.

La aplicación cuenta con un panel de control que permite modificar la apariencia de la mira en tiempo real.

---

## Características Principales

* **Siempre Visible:** La mira se muestra siempre en primer plano, ideal para juegos que no tienen una mira personalizable.
* **Panel de Control:** Una interfaz gráfica para ajustar todos los parámetros de la mira.
* **Alta Personalización:**
    * **Forma:** Elige entre una cruz, un círculo o un punto.
    * **Tamaño y Grosor:** Ajusta las dimensiones de la mira con deslizadores o cajas de texto.
    * **Color:** Selecciona cualquier color usando el selector de colores de Windows.
    * **Punto Central:** Activa o desactiva un punto en el centro para mayor visibilidad.
* **Sistema de Presets:**
    * Guarda y carga hasta 5 configuraciones (presets) diferentes.
    * Restaura cualquier preset a sus valores por defecto.
* **Guardado Automático:** Toda tu configuración, incluyendo los presets y colores personalizados, se guarda automáticamente al cerrar la aplicación y se carga al iniciarla.
* **Logo e Información:** El archivo ejecutable incluye un ícono y metadatos de versión para una apariencia más profesional.

---

## Archivos del Proyecto

Para compilar y ejecutar el proyecto, necesitas los siguientes tres archivos en la misma carpeta:

1.  `crosshairV2.cpp`: El código fuente principal de la aplicación.
2.  `resources.rc`: El archivo de recursos que define el ícono y la información de versión del programa.
3.  `logo.ico`: El archivo de ícono que se usará para el ejecutable.

---

## Instrucciones de Compilación

### Prerrequisitos

* Un compilador de C++ para Windows. Las siguientes instrucciones son para **MinGW-w64 (g++)**.

### Pasos para Compilar

Se recomienda compilar en dos pasos para asegurar que los recursos se enlacen correctamente. Abre una terminal (CMD o PowerShell) en la carpeta del proyecto y ejecuta los siguientes comandos:

1.  **Compilar los recursos:** Este comando convierte el archivo `.rc` a un formato que el compilador entiende.
    ```bash
    windres resources.rc -o resources.o
    ```

2.  **Compilar el código y enlazar todo:** Este comando compila el código C++, lo une con los recursos del paso anterior y crea el archivo `CrosshairV2.exe` final.
    ```bash
    g++ crosshairV2.cpp resources.o -o CrosshairV2.exe -lgdi32 -lcomdlg32 -lshlwapi -mwindows
    ```

---

## Uso

1.  Ejecuta el archivo `CrosshairV2.exe` generado.
2.  Aparecerá la mira en el centro de la pantalla y el "Panel de Ajustes".
3.  Usa el panel para personalizar la mira a tu gusto. Los cambios se reflejarán en tiempo real.
4.  Para cerrar la aplicación, simplemente cierra el "Panel de Ajustes".

---

## Información Adicional

* **Ubicación de los Ajustes:** La configuración de los presets se guarda en el archivo `settings.dat`, ubicado en la carpeta `%AppData%\CrosshairOverlay`.
* **Falsos Positivos de Antivirus:** Debido a que la aplicación crea una superposición en pantalla (un comportamiento similar al de algunos tipos de malware), es posible que algunos antivirus la detecten como una amenaza (falso positivo). El código es 100% seguro y no realiza ninguna acción maliciosa.

---

## Créditos

Creado por **DonMiñoquito**.
