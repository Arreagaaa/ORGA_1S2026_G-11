# Práctica 2 — Lógica Binaria y Combinacional

**Universidad San Carlos de Guatemala**
Facultad de Ingeniería — Ingeniería en Ciencias y Sistemas
Organización Computacional
**Fecha:** 21/03/26

---

## Integrantes del Grupo #11

| No. | Nombre Completo | Carné    |
|:---:|-----------------|----------|
| 1   |Javier Rivas     |202303204 |
| 2   |Daniel Aceituno  |202300579 |
| 3   |Rene Guitiérrez  |202300540 |
| 4   |Ana Nufio        |202400452 |


---

## 1. Introducción

En el campo de la arquitectura computacional, la Unidad Aritmética Lógica (ALU) representa uno de los componentes fundamentales de cualquier procesador. Este elemento es responsable de ejecutar todas las operaciones aritméticas y lógicas que un sistema computacional necesita para procesar información de manera eficiente.

La presente práctica, denominada **LogicCalc**, tiene como propósito diseñar e implementar un prototipo funcional de ALU básica utilizando exclusivamente circuitos integrados TTL y compuertas lógicas combinacionales. El sistema es capaz de realizar operaciones aritméticas (suma, resta, multiplicación y potencia) y operaciones lógicas (AND, OR, NAND y XNOR), además de incorporar una unidad comparativa que determina el número mayor y menor entre dos operandos de 4 bits.

El diseño se basa en principios de **lógica combinacional pura**, garantizando que todas las salidas dependan exclusivamente de las entradas actuales, sin la presencia de elementos secuenciales como flip-flops o registros. Los resultados se presentan mediante displays de 7 segmentos y LEDs de salida, simulando el comportamiento interno de una ALU real dentro de un procesador.

---

## 2. Objetivos

### 2.1 Objetivo General

Diseñar e implementar una Unidad Aritmética Lógica (ALU) funcional denominada LogicCalc, capaz de ejecutar operaciones aritméticas y lógicas mediante compuertas lógicas combinacionales y circuitos integrados TTL, utilizando únicamente los componentes autorizados por la cátedra.

### 2.2 Objetivos Específicos

1. **Unidad Aritmética:** Implementar la unidad aritmética del LogicCalc capaz de realizar suma, resta, multiplicación y potencia sobre operandos de 4 bits, mostrando los resultados correctamente en dos displays de 7 segmentos, con manejo adecuado del acarreo y validación de operandos mediante complemento a dos.

2. **Unidad Lógica:** Desarrollar la unidad lógica del sistema que ejecute las operaciones AND, OR, NAND y XNOR de forma bit a bit entre los dos operandos de entrada, presentando los resultados de 4 bits a través de LEDs de salida, garantizando la exclusividad mutua con la unidad aritmética mediante señales de control.

3. **Unidad Comparativa:** Construir la unidad comparativa que determine y muestre el número mayor y el número menor entre los operandos A y B en dos displays de 7 segmentos independientes, manejando correctamente el caso en que ambos operandos sean iguales.

4. **Controlador:** Integrar un sistema controlador mediante multiplexores y demultiplexores que seleccione correctamente la operación a ejecutar a partir de las señales de control CBA de 3 bits, asegurando que las unidades aritmética, lógica y comparativa operen de manera coordinada e independiente.

---

## 3. Funciones Booleanas

### 3.1 Unidad Aritmética

**Suma (Full Adder de 4 bits):**
- Suma:    `Si = Ai XOR Bi XOR Ci-1`
- Acarreo: `Ci = (Ai AND Bi) OR (Ci-1 AND (Ai XOR Bi))`

**Resta (Complemento a 2):**
- `A - B = A + B' + 1`  *(donde B' es el complemento bit a bit de B)*
- Validación `A ≥ B`: se extrae de la señal de carry-out del sumador
- Si `A < B` → mostrar `EE` en el display

**Multiplicación (sumas parciales):**
- `P = A × B = Σ (Ai · B) << i`  para i = 0..3

**Potencia:**
- Si `B = 2`: `P = A²`  → A × A mediante sumadores en cascada
- Si `B = 3`: `P = A³`  → A × A × A mediante sumadores en cascada

---

### 3.2 Unidad Lógica

| Operación | Función Booleana       |
|:---------:|------------------------|
| AND       | `F = A AND B`  (bit a bit) |
| OR        | `F = A OR B`   (bit a bit) |
| NAND      | `F = NOT(A AND B)` (bit a bit) |
| XNOR      | `F = NOT(A XOR B)` (bit a bit) |

---

### 3.3 Unidad Comparativa

Usando el comparador **74LS85**:

- `A > B` → pin **GT** activo
- `A < B` → pin **LT** activo
- `A = B` → pin **EQ** activo → mostrar mismo número en ambos displays

---

### 3.4 Tabla de Selección de Operaciones (Controlador)

| C | B | A | Operación      |
|:-:|:-:|:-:|----------------|
| 0 | 0 | 0 | Suma           |
| 0 | 0 | 1 | Resta          |
| 0 | 1 | 0 | Multiplicación |
| 0 | 1 | 1 | Potencia       |
| 1 | 0 | 0 | AND            |
| 1 | 0 | 1 | OR             |
| 1 | 1 | 0 | NAND           |
| 1 | 1 | 1 | XNOR           |

---

## 4. Mapas de Karnaugh

## 4.1. Controlador de Operaciones
 
El controlador recibe los bits **C, B, A** para seleccionar la operación activa.
 
### Variables de entrada
 
| Bit | Rol |
|-----|-----|
| C   | Selecciona unidad: `0` = Aritmética, `1` = Lógica |
| B   | Subselección dentro de la unidad |
| A   | Subselección dentro de la unidad |
 
### Tabla de verdad del controlador
 
| # | C | B | A | Operación    | Salida activa |
|---|---|---|---|--------------|---------------|
| 0 | 0 | 0 | 0 | Suma         | S0            |
| 1 | 0 | 0 | 1 | Resta        | S1            |
| 2 | 0 | 1 | 0 | Multiplicación | S2          |
| 3 | 0 | 1 | 1 | Potencia     | S3            |
| 4 | 1 | 0 | 0 | AND          | S4            |
| 5 | 1 | 0 | 1 | OR           | S5            |
| 6 | 1 | 1 | 0 | NAND         | S6            |
| 7 | 1 | 1 | 1 | XNOR         | S7            |
 
> El 74138 realiza esta decodificación directamente con entradas C, B, A y salidas activas en bajo (`S0̄` a `S7̄`).
 
### K-Map — Señal LED Azul (Unidad Aritmética activa)
 
La señal `LED_AZUL = 1` cuando `C = 0`.
 
```
        BA
 C   | 00 | 01 | 11 | 10 |
-----|----|----|----|----|
  0  |  1 |  1 |  1 |  1 |
  1  |  0 |  0 |  0 |  0 |
```
 
**Función simplificada:**
```
LED_AZUL = C̄
```
 
### K-Map — Señal LED Amarillo (Unidad Lógica activa)
 
La señal `LED_AMARILLO = 1` cuando `C = 1`.
 
```
        BA
 C   | 00 | 01 | 11 | 10 |
-----|----|----|----|----|
  0  |  0 |  0 |  0 |  0 |
  1  |  1 |  1 |  1 |  1 |
```
 
**Función simplificada:**
```
LED_AMARILLO = C
```
 
---
 
## 4.2. Unidad Aritmética
 
### 4.2.1 Sumador — Full Adder de 1 bit
 
Variables de entrada: **A**, **B**, **Cin** (acarreo de entrada)  
Salidas: **Sum** (suma), **Cout** (acarreo de salida)
 
#### Tabla de verdad
 
| A | B | Cin | Sum | Cout |
|---|---|-----|-----|------|
| 0 | 0 |  0  |  0  |  0   |
| 0 | 0 |  1  |  1  |  0   |
| 0 | 1 |  0  |  1  |  0   |
| 0 | 1 |  1  |  0  |  1   |
| 1 | 0 |  0  |  1  |  0   |
| 1 | 0 |  1  |  0  |  1   |
| 1 | 1 |  0  |  0  |  1   |
| 1 | 1 |  1  |  1  |  1   |
 
#### K-Map — Sum
 
```
        B·Cin
  A  | 00 | 01 | 11 | 10 |
-----|----|----|----|----|
  0  |  0 |  1 |  0 |  1 |
  1  |  1 |  0 |  1 |  0 |
```
 
**Función simplificada:**
```
Sum = A ⊕ B ⊕ Cin
```
Implementación con 7486 (XOR): `Sum = (A XOR B) XOR Cin`
 
#### K-Map — Cout
 
```
        B·Cin
  A  | 00 | 01 | 11 | 10 |
-----|----|----|----|----|
  0  |  0 |  0 |  1 |  0 |
  1  |  0 |  1 |  1 |  1 |
```
 
Grupos:
- `{m3, m7}` → `B·Cin`
- `{m6, m7}` → `A·B`
- `{m5, m7}` → `A·Cin`
 
**Función simplificada:**
```
Cout = (A·B) + (A·Cin) + (B·Cin)
```
 
---
 
### 4.2.2 Restador — Detección de error "EE"
 
La resta se implementa como **A + complemento a 2 de B**.  
Se muestra `EE` en los displays cuando **A < B** (el resultado es negativo).
 
Variables de entrada: **Cout** (acarreo de salida del sumador), que indica si hubo préstamo.
 
#### Lógica de detección de error
 
Cuando se realiza `A + (~B) + 1`:
- Si `A ≥ B` → `Cout = 1` (sin error)
- Si `A < B` → `Cout = 0` (resultado negativo → mostrar "EE")
 
```
ERROR_EE = Cout̄
```
 
#### K-Map — Señal ERROR_EE (1 variable)
 
```
 Cout | ERROR_EE
------|----------
   0  |    1      ← Mostrar "EE"
   1  |    0      ← Mostrar resultado
```
 
**Función simplificada:**
```
ERROR_EE = C̄out
```

---
 
### 4.2.3 Potencia — Selección de exponente
 
La operación **Potencia** eleva `A` al cuadrado si `B = 2` (binario: `0010`) o al cubo si `B = 3` (binario: `0011`).
 
Variables relevantes de B: **B1** y **B0** (bits menos significativos de B).
 
#### Tabla de verdad
 
| B1 | B0 | Operación | Selección |
|----|----|-----------|-----------|
|  0 |  0 | No válido | —         |
|  0 |  1 | No válido | —         |
|  1 |  0 | A²        | SEL = 0   |
|  1 |  1 | A³        | SEL = 1   |
 
#### K-Map — Señal SEL_CUBO
 
```
        B0
 B1  |  0  |  1  |
-----|-----|-----|
  0  |  X  |  X  |   ← No-cares (B < 2 no se evalúa)
  1  |  0  |  1  |
```
 
**Función simplificada:**
```
SEL_CUBO = B1 · B0
```
 
---
 
## 4.3. Unidad Lógica
 
Todas las operaciones son **bit a bit** sobre los 4 bits de A y B.  
Se muestra un K-map representativo por bit (el mismo aplica a los 4 bits).
 
### 4.3.1 Operación AND
 
#### Tabla de verdad (1 bit)
 
| A | B | AND |
|---|---|-----|
| 0 | 0 |  0  |
| 0 | 1 |  0  |
| 1 | 0 |  0  |
| 1 | 1 |  1  |
 
#### K-Map — AND
 
```
     B
 A | 0 | 1 |
---|---|---|
 0 | 0 | 0 |
 1 | 0 | 1 |
```
 
Grupo: `{m3}` → `A·B`
 
**Función simplificada:**
```
AND = A · B
```
Implementación con **7408**.
 
---
 
### 4.3.2 Operación OR
 
#### Tabla de verdad (1 bit)
 
| A | B | OR |
|---|---|----|
| 0 | 0 |  0 |
| 0 | 1 |  1 |
| 1 | 0 |  1 |
| 1 | 1 |  1 |
 
#### K-Map — OR
 
```
     B
 A | 0 | 1 |
---|---|---|
 0 | 0 | 1 |
 1 | 1 | 1 |
```
 
Grupos:
- `{m1, m3}` → `B`
- `{m2, m3}` → `A`
 
**Función simplificada:**
```
OR = A + B
```
Implementación con **7432**.
 
---
 
### 4.3.3 Operación NAND
 
#### Tabla de verdad (1 bit)
 
| A | B | NAND |
|---|---|------|
| 0 | 0 |  1   |
| 0 | 1 |  1   |
| 1 | 0 |  1   |
| 1 | 1 |  0   |
 
#### K-Map — NAND
 
```
     B
 A | 0 | 1 |
---|---|---|
 0 | 1 | 1 |
 1 | 1 | 0 |
```
 
Grupos (para los unos):
- `{m0, m1}` → `Ā`
- `{m0, m2}` → `B̄`
 
**Función simplificada:**
```
NAND = Ā + B̄ = NOT(A · B)
```
Implementación con **7408** (AND) + **7404** (NOT).  
O bien usando De Morgan directamente: `Ā + B̄` con 7404 y 7432.
 
---
 
### 4.3.4 Operación XNOR
 
#### Tabla de verdad (1 bit)
 
| A | B | XNOR |
|---|---|------|
| 0 | 0 |  1   |
| 0 | 1 |  0   |
| 1 | 0 |  0   |
| 1 | 1 |  1   |
 
#### K-Map — XNOR
 
```
     B
 A | 0 | 1 |
---|---|---|
 0 | 1 | 0 |
 1 | 0 | 1 |
```
 
> No hay grupos posibles (tablero de ajedrez). La función es la negación de XOR.
 
**Función simplificada:**
```
XNOR = NOT(A ⊕ B) = (A · B) + (Ā · B̄)
```
Implementación con **7486** (XOR) + **7404** (NOT).
 
---
 
## 4.4. Controlador de Salida
 
El controlador determina qué salida se activa y cuál se bloquea (aritmética vs lógica son mutuamente excluyentes).
 
### K-Map — Habilitación de displays (salida aritmética)
 
La señal `EN_DISPLAY = 1` solo cuando la unidad aritmética está activa (`C = 0`) **y** la operación seleccionada es aritmética (minterms 0-3).
 
```
        BA
 C   | 00 | 01 | 11 | 10 |
-----|----|----|----|----|
  0  |  1 |  1 |  1 |  1 |
  1  |  0 |  0 |  0 |  0 |
```
 
**Función simplificada:**
```
EN_DISPLAY = C̄
```
 
### K-Map — Habilitación de LEDs lógicos (salida lógica)
 
```
        BA
 C   | 00 | 01 | 11 | 10 |
-----|----|----|----|----|
  0  |  0 |  0 |  0 |  0 |
  1  |  1 |  1 |  1 |  1 |
```
 
**Función simplificada:**
```
EN_LEDS = C
```
 
> Implementación: el bit `C` habilita directamente el 74138 (demux) y los multiplexores 74157 que enrutan la salida correcta.
 
---
 
## 5. Unidad Comparativa
 
El comparador utiliza el integrado **7485** que ya implementa internamente la comparación de 4 bits.  
Las salidas del 7485 son: `A>B`, `A=B`, `A<B`.
 
### Lógica de display Mayor / Menor
 
| A>B | A=B | A<B | Display Mayor | Display Menor |
|-----|-----|-----|---------------|---------------|
|  1  |  0  |  0  | Muestra A     | Muestra B     |
|  0  |  1  |  0  | Muestra A     | Muestra A     |
|  0  |  0  |  1  | Muestra B     | Muestra A     |
 
### K-Map — Selección MUX para Display Mayor (SEL_MAY)
 
`SEL_MAY = 0` → mostrar A, `SEL_MAY = 1` → mostrar B  
Solo importa la salida `A<B` del 7485.
 
```
 A<B | SEL_MAY (Display Mayor)
-----|------------------------
  0  |  0   ← Mostrar A (A≥B)
  1  |  1   ← Mostrar B (B es mayor)
```
 
**Función simplificada:**
```
SEL_MAY = (A < B)
```
 
### K-Map — Selección MUX para Display Menor (SEL_MEN)
 
`SEL_MEN = 0` → mostrar A, `SEL_MEN = 1` → mostrar B
 
```
 A>B | SEL_MEN (Display Menor)
-----|------------------------
  0  |  0   ← Mostrar A (A≤B)
  1  |  1   ← Mostrar B (B es menor)
```
 
**Función simplificada:**
```
SEL_MEN = (A > B)
```
 
> Implementación: Las señales `A>B` y `A<B` del pin 15 y pin 13 del **7485** controlan directamente los pines de selección de los multiplexores **74157** que enrutan A o B hacia los decoders **7447** y luego a los displays.

---

## 5. Diagramas de Diseño del Circuito y Placas

### 5.1 Diagrama de Bloques General

```
┌─────────────────────────────────────────────────┐
│                    USUARIO                       │
└──────────────────┬──────────────────────────────┘
                   │
          ┌────────┴────────┐
          ▼                 ▼
      [Número A]        [Número B]
          │                 │
          └────────┬────────┘
                   ▼
         [Entradas a la Unidad]
                   │
    ┌──────────────┼──────────────┐
    ▼              ▼              ▼
[Bloque       [Bloque        [Bloque
Aritmético]   Lógico]        Comparativo]
    │              │              │
    └──────────────┼──────────────┘
                   ▼
             [Controlador]
                   │
        ┌──────────┼──────────┐
        ▼          ▼          ▼
   [Salida     [Salida    [Salida
  Aritmética]  Lógica]   Comparativa]
        │                    │
        ▼               ┌────┴────┐
 [Conv. Bin→BCD]     [#Mayor] [#Menor]
        │               │        │
        ▼               ▼        ▼
   [Decoders]       [Decoder] [Decoder]
```

### 5.2 Esquema en Proteus

![738538c7-8562-489a-b927-f57eda2fafd6](https://hackmd.io/_uploads/SJkllSq9-e.jpg)

### 5.3 Fotografías del Montaje Físico

![bf89d5cf-8805-4737-8020-b137d158376f](https://hackmd.io/_uploads/r1khkHcqZl.jpg)

![eeea2989-2f93-4ec9-b6d7-87df3b452ab3](https://hackmd.io/_uploads/Ska2JBqcbg.jpg)

![4d812f21-3a33-4657-9614-be383510ceff](https://hackmd.io/_uploads/H1M0ySc5-l.jpg)

---

## 6. Equipo Utilizado

| Componente / Descripción            | Código    | Cantidad | Función en el Circuito                        |
|-------------------------------------|:---------:|:--------:|-----------------------------------------------|
| Compuerta Lógica AND                | 74LS08    | 1        | Operaciones AND y productos lógicos           |
| Compuerta Lógica NOT                | 74LS04    | 1        | Inversión de bits (complemento a 2)           |
| Compuerta Lógica OR                 | 74LS32    | 1        | Operación OR lógica                           |
| Compuerta Lógica XOR                | 74LS86    | 2        | XOR / XNOR y lógica de suma                  |
| Sumador de 4 bits                   | 74LS83    | 4        | Aritmética: suma, resta, multiplicación, potencia |
| Comparador de 4 bits                | 74LS85    | 1        | Unidad comparativa A vs B                    |
| Multiplexor 2 entradas              | 74LS157   | 3        | Selección de operación / controlador          |
| Demultiplexor / Decoder             | 74LS138   | 2        | Decodificación de señales de control          |
| Display 7 Segmentos Cátodo Común    | —         | 4        | Visualización de resultados numéricos         |
| Decoder Driver 7 Segmentos          | 74LS48    | 4        | Conversión BCD a 7 segmentos                 |
| LED Azul Difuso 5mm                 | —         | 3        | Indicador Unidad Aritmética activa            |
| LED Amarillo Difuso 5mm             | —         | 3        | Indicador Unidad Lógica activa               |
| LED Verde Difuso 5mm                | —         | 6        | Indicador de salidas lógicas (4 bits)        |
| Resistencia 330 Ohm 1/4W           | —         | 20       | Limitación de corriente en LEDs y displays    |
| Capacitor Electrolítico 10uF 16V    | —         | 4        | Desacoplamiento de alimentación              |
| DIP Switch 4 posiciones             | —         | 3        | Entradas A, B y señales de control CBA       |

---

## 7. Presupuesto

| Producto                                    | Cantidad | Precio Unitario | Total    |
|---------------------------------------------|:--------:|:---------------:|:--------:|
| Compuerta Lógica AND 74LS08                 | 1        | Q5.00           | Q5.00    |
| Compuerta Lógica NOT 74LS04                 | 1        | Q5.00           | Q5.00    |
| Compuerta Lógica OR 74LS32                  | 1        | Q5.00           | Q5.00    |
| Compuerta Lógica XOR 74LS86                 | 2        | Q6.00           | Q12.00   |
| Sumador 74LS83                              | 4        | Q15.00          | Q60.00   |
| Comparador 74LS85                           | 1        | Q11.00          | Q11.00   |
| Multiplexor 74LS157 (2 entradas)            | 3        | Q7.00           | Q21.00   |
| Decoder / Demux 74LS138                     | 2        | Q6.00           | Q12.00   |
| Display 7 Segmentos Cátodo Común Rojo       | 4        | Q5.00           | Q20.00   |
| Decodificador Driver 7 Seg. 74LS48          | 4        | Q9.00           | Q36.00   |
| LED Azul Difuso 5mm                         | 3        | Q1.00           | Q3.00    |
| LED Amarillo Difuso 5mm                     | 3        | Q1.00           | Q3.00    |
| LED Verde Difuso 5mm                        | 6        | Q1.00           | Q6.00    |
| Resistencia 330 Ohm 1/4W                   | 20       | Q0.50           | Q10.00   |
| Capacitor Electrolítico 10uF 16V            | 4        | Q1.50           | Q6.00    |
| DIP Switch de 4 Posiciones                  | 3        | Q3.75           | Q11.25   |
| **TOTAL**                                   |          |                 | **Q226.25** |

---

## 8. Conclusiones

### Conclusión 1

La implementación de la unidad aritmética permitió comprender en profundidad cómo los circuitos integrados TTL como el 74LS83 realizan sumas y restas binarias de manera eficiente mediante lógica combinacional. El manejo del acarreo y la representación en complemento a dos resultaron esenciales para garantizar la precisión de los resultados, evidenciando que los fundamentos teóricos de la aritmética binaria tienen una correspondencia directa y funcional en el hardware físico.

### Conclusión 2

El desarrollo de la unidad lógica demostró que las operaciones bit a bit como AND, OR, NAND y XNOR pueden implementarse de manera directa con compuertas básicas, sin necesidad de lógica adicional compleja. La restricción de mostrar resultados exclusivamente mediante LEDs y la exclusividad mutua con la unidad aritmética reforzó el concepto de diseño modular en sistemas digitales, donde cada bloque opera de forma independiente pero coordinada.

### Conclusión 3

La integración de la unidad comparativa utilizando el 74LS85 fue un ejercicio valioso para entender cómo los comparadores digitales determinan relaciones de magnitud entre operandos binarios. Ver los resultados de mayor y menor en displays de 7 segmentos conectados al decodificador 74LS48 consolidó el proceso completo de conversión desde valores binarios hasta una representación visual comprensible para el usuario.

### Conclusión 4

El diseño del controlador mediante multiplexores 74LS157 y demultiplexores 74LS138 fue la pieza central que unificó todos los módulos del LogicCalc. Esta experiencia permitió comprender cómo las señales de control CBA coordinan el funcionamiento de una ALU real, confirmando que la correcta selección y enrutamiento de señales es tan crítica como la lógica operativa de cada unidad, siendo determinante para el funcionamiento global del sistema.
