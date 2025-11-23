#!/usr/bin/env python3

"""
compare_ppm.py - Compara dos imágenes PPM usando umbrales de tolerancia.

1. Diferencia Máxima por Píxel < 150
2. Error Cuadrático Medio (RMSE) < 10

Uso:
  ./compare_ppm.py <imagen_generada.ppm> <imagen_referencia.ppm>

Códigos de Salida:
  - 0: Éxito (las imágenes son aceptables)
  - 1: Fallo (las imágenes NO son aceptables)
  - 2: Error de uso (argumentos incorrectos)
  - 3: Error de archivo (no se puede leer, no es P3, dimensiones no coinciden)
"""

import sys
import math

# Umbrales de Aceptación
MAX_DIFF_THRESHOLD = 150.0
RMSE_THRESHOLD = 10.0

def parse_ppm_p3(filename):
    """Lee una imagen PPM y devuelve (ancho, alto, datos_bytes)."""
    try:
        # Abrir en modo texto 'r'
        with open(filename, 'r') as f:

            # Leer Magic Number (como texto)
            magic = f.readline().strip()
            if magic != 'P3':
                raise ValueError(f"No es un archivo P3 válido (magic number: {magic})")

            dims = ''
            while True:
                line = f.readline().strip()
                if not line.startswith('#'):
                    dims = line
                    break

            try:
                width, height = map(int, dims.split())
            except ValueError:
                raise ValueError(f"Cabecera de dimensiones inválida: {dims}")

            # Leer MaxVal (saltando comentarios)
            max_val_line = ''
            while True:
                line = f.readline().strip()
                if not line.startswith('#'):
                    max_val_line = line
                    break

            try:
                max_val = int(max_val_line)
            except ValueError:
                raise ValueError(f"Cabecera de MaxVal inválida: {max_val_line}")

            if max_val != 255:
                raise ValueError(f"Solo se soporta MaxVal=255 (encontrado: {max_val})")

            # Leer Datos
            all_values_str = f.read().split()

            expected_values = width * height * 3
            if len(all_values_str) != expected_values:
                raise ValueError(f"Datos de imagen incompletos. Esperados {expected_values} valores, leídos {len(all_values_str)}")

            # Convertir los strings a un array de bytes (enteros 0-255)
            # Usamos bytearray para que sea compatible con el bucle de data[index]

            try:
                data = bytearray(int(val) for val in all_values_str)
            except ValueError:
                raise ValueError("Se encontró un valor no numérico en los datos de píxeles.")

            return width, height, data

    except (IOError, FileNotFoundError) as e:
        print(f"Error: No se pudo leer el archivo '{filename}'. {e}", file=sys.stderr)
        sys.exit(3)
    except ValueError as e:
        print(f"Error: Fallo al parsear PPM '{filename}'. {e}", file=sys.stderr)
        sys.exit(3)

def main():
    # Validar Argumentos para que sea el número adecuado
    if len(sys.argv) != 3:
        print(f"Uso: {sys.argv[0]} <imagen1.ppm> <imagen2.ppm>", file=sys.stderr)
        sys.exit(2)

    file1_path = sys.argv[1]
    file2_path = sys.argv[2]

    # Parsear Imágenes
    print(f"INFO: Comparando '{file1_path}' (generada) vs '{file2_path}' (referencia)")

    w1, h1, data1 = parse_ppm_p3(file1_path)
    w2, h2, data2 = parse_ppm_p3(file2_path)

    # Validar Dimensiones
    if w1 != w2 or h1 != h2:
        print(f"Error: Las dimensiones no coinciden.", file=sys.stderr)
        print(f"  {file1_path}: {w1}x{h1}", file=sys.stderr)
        print(f"  {file2_path}: {w2}x{h2}", file=sys.stderr)
        sys.exit(3)

    num_pixels = w1 * h1
    max_pixel_diff = 0.0
    sum_squared_diff = 0.0

    # Calcular Métricas
    for i in range(num_pixels):
        idx = i * 3

        # Píxel 1 (generado)
        r1, g1, b1 = data1[idx], data1[idx+1], data1[idx+2]
        # Píxel 2 (referencia)
        r2, g2, b2 = data2[idx], data2[idx+1], data2[idx+2]

        # Fórmula de diferencia:
        pixel_diff = (abs(r1 - r2) + abs(g1 - g2) + abs(b1 - b2)) / 3.0

        # Actualizar MaxDiff
        if pixel_diff > max_pixel_diff:
            max_pixel_diff = pixel_diff

        # Sumar para RMSE
        sum_squared_diff += pixel_diff * pixel_diff

    # Calcular RMSE
    mse = sum_squared_diff / num_pixels
    rmse = math.sqrt(mse)

    # Comprobar Umbrales y Salir
    print(f"  Resultados de la Comparación:")
    print(f"    - Max Diff Píxel: {max_pixel_diff:.4f} (Umbral: < {MAX_DIFF_THRESHOLD})")
    print(f"    - RMSE:          {rmse:.4f} (Umbral: < {RMSE_THRESHOLD})")

    max_diff_ok = max_pixel_diff < MAX_DIFF_THRESHOLD
    rmse_ok = rmse < RMSE_THRESHOLD

    if max_diff_ok and rmse_ok:
        print("  VEREDICTO: ACEPTABLE (Ambos umbrales se cumplen)")
        sys.exit(0) # Éxito
    else:
        print("  VEREDICTO: FALLIDO (Al menos un umbral no se cumple)", file=sys.stderr)
        if not max_diff_ok:
            print(f"    - FALLO: Max Diff {max_pixel_diff:.4f} >= {MAX_DIFF_THRESHOLD}", file=sys.stderr)
        if not rmse_ok:
            print(f"    - FALLO: RMSE {rmse:.4f} >= {RMSE_THRESHOLD}", file=sys.stderr)
        sys.exit(1) # Fallo

if __name__ == "__main__":
    main()
