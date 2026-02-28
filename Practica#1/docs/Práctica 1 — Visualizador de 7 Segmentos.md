# Práctica 1 — Visualizador de 7 Segmentos
**Organización Computacional | Universidad San Carlos de Guatemala**
Facultad de Ingeniería — Ingeniería en Ciencias y Sistemas

---

> **Curso:** Organización Computacional
> **Sección:** C
> **Grupo:** 11
> **Fecha de entrega:** 27/02/2026

---

## Integrantes del Equipo 

| Nombre Completo | Carnet |
|----------------|-------|
|Daniel Aceituno |202300579 | 
|Javier Rivas |202303204 | 
|Rene Gutiérrez |202300540 |
|Ana Nufio |202400452 |

---

## Índice

[TOC]

---

## 1. Introducción

Esta documentación detalla el diseño, implementación y validación de un sistema de visualización mediante displays de 7 segmentos bidireccional (normal y espejo). Este proyecto integra conceptos fundamentales de lógica combinacional, electrónica digital y diseño de circuitos, aplicados a la simulación de señalización urbana inteligente que debe ser legible desde distintos puntos de vista en una intersección.

El sistema implementa dos tipos de displays:
- **Display de cátodo común** — para visualización frontal (normal), usando minterminos
- **Display de ánodo común** — para visualización trasera en espejo, usando maxterminos

Esto permite la lectura de una palabra personalizada de 8 caracteres (mínimo 2 números, máximo 1 repetición) desde direcciones opuestas. La lógica se implementa físicamente mediante compuertas transistorizadas con transistores 2N2222A sobre placas PCB para los segmentos a, c, d, f y g; y mediante compuertas TTL serie 74LS sobre protoboard para los segmentos b, e y el punto decimal (pt).

---

## 2. Objetivos

### 2.1 Objetivo General
Diseñar e implementar un sistema físico funcional de visualización mediante displays de 7 segmentos bidireccional (normal y espejo), aplicando lógica combinacional con compuertas transistorizadas en PCB y compuertas TTL en protoboard.

### 2.2 Objetivos Específicos
- Diseñar y simplificar las funciones booleanas de cada segmento mediante mapas de Karnaugh, utilizando términos mínimos para cátodo común y términos máximos para ánodo común.
- Implementar los segmentos a, c, d, f y g con compuertas lógicas transistorizadas (2N2222A) fabricadas sobre placas de circuito impreso (PCB) etiquetadas con formato G11_S1_2026.
- Implementar los segmentos b, e y punto decimal con compuertas TTL (74LS08, 74LS32, 74LS04) montadas en protoboard.

---

## 3. Marco Teórico

### 3.1 Display de 7 Segmentos

![14bb15d5-708c-4c28-ac00-914b1f35e5c6](https://hackmd.io/_uploads/B1FB7KJFbl.jpg)

Un display de 7 segmentos está formado por siete LEDs dispuestos en forma de "8", etiquetados **a, b, c, d, e, f, g**. Existen dos tipos:

| Tipo | Conexión común | Segmento ON | Lógica usada |
|------|---------------|-------------|--------------|
| **Cátodo Común** | Cátodos → GND | Señal **HIGH (1)** | **Términos mínimos** |
| **Ánodo Común** | Ánodos → VCC | Señal **LOW (0)** | **Términos máximos** |

### 3.2 Compuertas TTL — Serie 74LS

![dcde0c1d-79c0-459b-94a1-b5cecfb37469](https://hackmd.io/_uploads/SyiuQtJYbe.jpg)

| Compuerta | CI | Operación | Segmentos asignados |
|-----------|-----|-----------|---------------------|
| AND | 74LS08 | F = A·B | b, e |
| OR | 74LS32 | F = A+B | b, e |
| NOT | 74LS04 | F = Ā | Inversiones  |


### 3.3 Compuertas Transistorizadas (2N2222A)

Las compuertas lógicas se implementan con transistores NPN 2N2222A que funcionan como interruptores. Cuando la base recibe señal HIGH, el transistor conduce y activa la salida. Los segmentos a, c, d, f y g se implementan de esta forma sobre PCB.

**Configuración típica por transistor:**
- Resistencia de base: 10 kΩ
- Resistencia de colector: 1 kΩ
- Resistencia limitadora de LED: 220 Ω

![e47f08df-460d-41f3-99d3-aa8f24170496](https://hackmd.io/_uploads/r1Ap7t1F-x.jpg)

![esquematico_page-0001](https://hackmd.io/_uploads/HJGWGFkYWe.jpg)

### 3.4 Mapas de Karnaugh

Los mapas de Karnaugh permiten simplificar funciones booleanas de forma gráfica, agrupando celdas adyacentes con valor 1 (para minterms) o valor 0 (para maxterms). Para 3 variables de entrada (A, B, C) el mapa tiene 8 celdas (2³).

```
Estructura del mapa K — 3 variables (A, B, C):

        BC
A    | 00 | 01 | 11 | 10 |
-----|----|----|----|----|
 0   | m0 | m1 | m3 | m2 |
 1   | m4 | m5 | m7 | m6 |
```

---

## 4. Palabra Seleccionada

La palabra elegida para la practica es:

> ## ORGA1S26

### Validación de la palabra:

| Regla | ¿Cumple? |
|-------|----------|
| Exactamente 8 caracteres |  ✅ |
| Mínimo 2 números |   ✅ |
| Máximo 1 letra o número repetido | ✅ |

### Codificación de caracteres (3 bits → 8 posiciones):

| Posición | Carácter | A | B | C |
|----------|----------|---|---|---|
| 0 |O | 0 | 0 | 0 |
| 1 |R | 0 | 0 | 1 |
| 2 |G | 0 | 1 | 0 |
| 3 |A | 0 | 1 | 1 |
| 4 |1 | 1 | 0 | 0 |
| 5 |S | 1 | 0 | 1 |
| 6 |2 | 1 | 1 | 0 |
| 7 |6 | 1 | 1 | 1 |

---

## 5. Tabla de Verdad

![36be8e8b-c6fc-4719-9dd5-42d1067cbbd3](https://hackmd.io/_uploads/rkGmftyY-x.jpg)

---

## 6. Funciones Booleanas — Display Cátodo Común (Minterminos)

Las funciones se obtienen sumando los minterminos (posiciones donde el segmento = 1).

### Segmento a *(transistorizado — PCB)*
Simplificada: $a =$ B + C + Ā

### Segmento b *(TTL — protoboard)*
Simplificada: $b =$ C̄ + ĀB

### Segmento c *(transistorizado — PCB)*
Simplificada: $c =$ BC + ĀB + ĀC

### Segmento d *(transistorizado — PCB)*
Simplificada: $d =$ AC + BĈ + ĀĈ

### Segmento e *(TTL — protoboard)*
Simplificada: $e =$ AB + ĀC + ĀB

### Segmento f *(transistorizado — PCB)*
Simplificada: $f =$ C + Ā

### Segmento g *(transistorizado — PCB)*
Simplificada: $g =$ B + AC

---

## 7. Mapas de Karnaugh — Display Cátodo Común

### Segmento a:
```
     | 00  | 01  | 11  | 10  |
-----|-----|-----|-----|-----|
 0   |  1  |  1  |  1  |  1  |
 1   |  0  |  1  |  1  |  0  |
```

### Segmento b:
```
     | 00  | 01  | 11  | 10  |
-----|-----|-----|-----|-----|
 0   |  1  |  0  |  1  |  1  |
 1   |  1  |  0  |  1  |  0  |
```

### Segmento c:
```
     | 00  | 01  | 11  | 10  |
-----|-----|-----|-----|-----|
 0   |  1  |  0  |  1  |  1  |
 1   |  0  |  0  |  0  |  1  |
```

### Segmento d:
```
     | 00  | 01  | 11  | 10  |
-----|-----|-----|-----|-----|
 0   |  1  |  0  |  1  |  1  |
 1   |  0  |  0  |  1  |  1  |
```

### Segmento e:
```
     | 00  | 01  | 11  | 10  |
-----|-----|-----|-----|-----|
 0   |  1  |  1  |  0  |  1  |
 1   |  0  |  0  |  1  |  0  |
```

### Segmento f:
```
     | 00  | 01  | 11  | 10  |
-----|-----|-----|-----|-----|
 0   |  1  |  1  |  1  |  1  |
 1   |  0  |  1  |  1  |  0  |
```

### Segmento g:
```
     | 00  | 01  | 11  | 10  |
-----|-----|-----|-----|-----|
 0   |  0  |  0  |  1  |  1  |
 1   |  0  |  0  |  1  |  1  |
```

---

## 8. Funciones Booleanas — Display Ánodo Común (Maxterminos / Espejo)

Para el display de ánodo común se usan términos máximos (posiciones donde el segmento = 0 en cátodo común). La función de ánodo común equivale a aplicar NOT a cada salida del cátodo común.

### Segmento a (espejo):
Simplificada: $a' =$ B̄ · C̄ · A

### Segmento b (espejo):
Simplificada: $b' =$ C · (A + B̄)

### Segmento c (espejo):
Simplificada: $c' =$ (B̄+C̄)(A+B̄)(A+C̄)

### Segmento d (espejo):
Simplificada: $d' =$ (Ā+C̄)(B̄+C)(A+C)

### Segmento e (espejo):
Simplificada: $e' =$ (Ā+B̄)(A+C̄)(A+B̄)

### Segmento f (espejo):
Simplificada: $f' =$ C̄ · A

### Segmento g (espejo):
Simplificada: $g' =$ B̄ · (Ā+C̄)

---

## 9. Mapas de Karnaugh — Display Ánodo Común (Espejo)

### Segmento a':
```
     | 00  | 01  | 11  | 10  |
-----|-----|-----|-----|-----|
 0   |  0  |  0  |  0  |  0  |
 1   |  1  |  0  |  0  |  1  |
```

### Segmento b':
```
     | 00  | 01  | 11  | 10  |
-----|-----|-----|-----|-----|
 0   |  0  |  1  |  0  |  0  |
 1   |  0  |  1  |  0  |  1  |
```

### Segmento c':
```
     | 00  | 01  | 11  | 10  |
-----|-----|-----|-----|-----|
 0   |  0  |  1  |  0  |  0  |
 1   |  1  |  1  |  1  |  0  |
```

### Segmento d':
```
     | 00  | 01  | 11  | 10  |
-----|-----|-----|-----|-----|
 0   |  0  |  1  |  0  |  0  |
 1   |  1  |  1  |  0  |  0  |
```

### Segmento e':
```
     | 00  | 01  | 11  | 10  |
-----|-----|-----|-----|-----|
 0   |  0  |  0  |  1  |  0  |
 1   |  1  |  1  |  0  |  1  |
```

### Segmento f':
```
     | 00  | 01  | 11  | 10  |
-----|-----|-----|-----|-----|
 0   |  0  |  0  |  0  |  0  |
 1   |  1  |  0  |  0  |  1  |
```

### Segmento g':
```
     | 00  | 01  | 11  | 10  |
-----|-----|-----|-----|-----|
 0   |  1  |  1  |  0  |  0  |
 1   |  1  |  1  |  0  |  0  |
```
---

## 10. Implementación en Proteus

### 10.1 Circuito Schematic — Compuerta Transistorizada

![image](https://hackmd.io/_uploads/B17qgnJF-g.png)

### 10.2 Display Cátodo Común — Funcionando

![image](https://hackmd.io/_uploads/Hyq1jK1t-g.png)


### 10.3 Display Ánodo Común — Espejo Funcionando

![image](https://hackmd.io/_uploads/ryHViFyYWl.png)

### 10.4 Diseño PCB en Proteus

![3f23af9f-1d23-4549-897d-776dacb82c0f](https://hackmd.io/_uploads/BysQVKkYWg.jpg)

![a6bd97ef-8a6b-4e09-88df-6d82397df3a2](https://hackmd.io/_uploads/SJGE4YkKWl.jpg)

---

## 11. Implementación Física

### 11.1 Montaje en Protoboard

![c96abd8e-4d8a-4c7d-9c81-4eabc15b655b](https://hackmd.io/_uploads/SkKH4tkKWl.jpg)

El protoboard implementa los segmentos b, e y pt con compuertas TTL:

### 11.2 Placas PCB Fabricadas

![9f1ef491-f471-4f5b-99c1-7541266b2a4b](https://hackmd.io/_uploads/rkDHGqyFWx.jpg)

![72a281e8-92fc-4732-be80-b12707acfcce](https://hackmd.io/_uploads/BJAHMcJtWg.jpg)

---

## 12. Materiales y Presupuesto

### Lista de Componentes

| # | Componente | Especificación | Cant. Mín. | Cant. Máx. | Precio Unit. (Q) | Subtotal Mín. (Q) | Subtotal Máx. (Q) |
|---|-----------|---------------|:----------:|:----------:|:----------------:|:-----------------:|:-----------------:|
| **ILUMINACIÓN** | | | | | | | |
| 1 | LED rojo 5mm | 3 LEDs/seg × 7seg × 8char × 2 displays | 43 | 101 | Q 0.50 | Q 21.50 | Q 50.50 |
| **SEMICONDUCTORES** | | | | | | | |
| 2 | Transistor NPN | 2N2222A | 30 | 45 | Q 1.00 | Q 30.00 | Q 45.00 |
| **RESISTENCIAS** | | | | | | | |
| 3 | Resistencia 220 Ω | Limitadora de corriente LEDs | 43 | 101 | Q 0.15 | Q 6.45 | Q 15.15 |
| 4 | Resistencia 10 kΩ | Pull-down / base transistor | 10 | 20 | Q 0.15 | Q 1.50 | Q 3.00 |
| 5 | Resistencia 1 kΩ | Base transistor | 30 | 45 | Q 0.15 | Q 4.50 | Q 6.75 |
| **COMPUERTAS TTL (74LS)** | | | | | | | |
| 6 | 74LS08 — AND | 4 compuertas/CI | 1 | 3 | Q 6.00 | Q 6.00 | Q 18.00 |
| 7 | 74LS32 — OR | 4 compuertas/CI | 1 | 3 | Q 6.00 | Q 6.00 | Q 18.00 |
| 8 | 74LS04 — NOT | 6 por CI | 1 | 2 | Q 6.00 | Q 6.00 | Q 12.00 |
| **PCB Y PROTOBOARD** | | | | | | | |
| 9 | Placa PCB virgen | Para compuertas transistorizadas FR4 | 2 | 2 | Q 15.00 | Q 30.00 | Q 30.00 |
| 10 | Protoboard | 830 puntos | 1 | 2 | Q 35.00 | Q 35.00 | Q 70.00 |
| **CONEXIONES** | | | | | | | |
| 11 | Cables jumper M-M | Pack 40 unidades | 2 | 3 | Q 20.00 | Q 40.00 | Q 60.00 |
| 12 | Cables M-H | Pack 40 unidades | 1 | 2 | Q 20.00 | Q 20.00 | Q 40.00 |
| 13 | Terminal block | 2 pines | 6 | 8 | Q 3.00 | Q 18.00 | Q 24.00 |
| 14 | DIP Switch | 3 posiciones | 1 | 2 | Q 5.00 | Q 5.00 | Q 10.00 |
| **ALIMENTACIÓN** | | | | | | | |
| 15 | Fuente regulada 5V DC | Con cargador USB o AC | 1 | 1 | Q 45.00 | Q 45.00 | Q 45.00 |
| 16 | Batería 9V + regulador 7805 | Alternativa económica | 1 | 1 | Q 20.00 | Q 20.00 | Q 20.00 |
| **CONSUMIBLES** | | | | | | | |
| 17 | Estaño para soldar | Rollo 0.8mm | 1 | 1 | Q 25.00 | Q 25.00 | Q 25.00 |
| 18 | Flux desoxidante | Opcional | 1 | 1 | Q 15.00 | Q 15.00 | Q 15.00 |
| 19 | Cinta aislante | Kapton tape | 1 | 1 | Q 8.00 | Q 8.00 | Q 8.00 |
| | | | | | **TOTAL ESTIMADO** | **Q 343.95** | **Q 515.40** |

> Se tomo en cuenta en comprar un 10% extra de LEDs y resistencias como repuestos. La cantidad final de transistores y compuertas depende de la simplificación K-map de cada segmento.

---

## 13. Conclusiones

Al diseñar e implementar las funciones booleanas mediante mapas de Karnaugh se comprobó que la simplificación reduce significativamente el número de compuertas necesarias, logrando que el sistema de visualización funcione de manera óptima. El objetivo de aplicar lógica combinacional para controlar ambos displays fue alcanzado satisfactoriamente, demostrando la viabilidad del diseño propuesto.

La diferenciación entre términos mínimos para el display de cátodo común y términos máximos para el ánodo común fue esencial para garantizar la visualización correcta en modo espejo. Esto valida el objetivo de implementar ambos tipos de display con lógica complementaria, comprobando que la dualidad booleana tiene una aplicación directa y práctica en el hardware.

La fabricación de compuertas transistorizadas sobre placas PCB con transistores 2N2222A permitió alcanzar el objetivo de construir hardware discreto funcional. El proceso evidenció la relación directa entre la función booleana simplificada y el circuito físico, confirmando que la etapa de diseño teórico es determinante para el éxito de la implementación.

---
