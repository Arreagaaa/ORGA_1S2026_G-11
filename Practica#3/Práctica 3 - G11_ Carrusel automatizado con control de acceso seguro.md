# Práctica 3: Carrusel automatizado con control de acceso seguro

**Universidad San Carlos de Guatemala**  
**Facultad de Ingeniería — Escuela de Ingeniería en Ciencias y Sistemas**  
**Curso:** Organización Computacional  
**Semestre:** 1S 2026  
**Grupo:** 11  
**Sección:** C  

| No. | Integrante | Carné |
|:---:|------------|-------|
| 1 | Javier Rivas | 202303204 |
| 2 | Daniel Aceituno | 202300579 |
| 3 | Rene Gutiérrez | 202300540 |
| 4 | Ana Nufio | 202400452 |

**Fecha de entrega:** 18 de abril de 2026  
**Repositorio:** 

---

## Tabla de Contenidos

1. [Introducción](#introducción)
2. [Objetivos](#objetivos)
3. [Funciones Booleanas](#funciones-booleanas)
4. [Mapas de Karnaugh](#mapas-de-karnaugh)
5. [Diagramas del Diseño del Circuito](#diagramas-del-diseño-del-circuito)
6. [Equipo Utilizado](#equipo-utilizado)
7. [Presupuesto](#presupuesto)
8. [Conclusiones](#conclusiones)

---

## Introducción

El presente documento describe el diseño e implementación de un carrusel automatizado con control de acceso seguro, desarrollado para la empresa "TecnoParques S.A." El sistema integra lógica digital combinacional y secuencial para garantizar que solo usuarios autorizados puedan activar el carrusel, mediante un sistema de autenticación por contraseña de 4 bits, control bidireccional de motor DC y visualización en displays de 7 segmentos.

---

## Objetivos

### Objetivo general

Diseñar y construir un sistema digital completo que integre un módulo de autenticación con contraseña de 4 bits, un contador de errores con alarma, el control bidireccional de un motor DC mediante puente H, y dos contadores temporizados con displays de 7 segmentos, creando un carrusel automatizado con control de acceso seguro.

### Objetivos específicos

- Aplicar principios de lógica secuencial y combinacional utilizando flip-flops tipo D (74LS74), comparadores (7485) y decodificadores (7447/7448).
- Diseñar e implementar un sistema de autenticación digital con gestión de contraseñas, detección de errores y activación de alarmas.
- Integrar el puente H construido con transistores discretos (2N2222A) para el control bidireccional del motor DC.
- Utilizar el Arduino como generador de pulsos de reloj y controlador de la secuencia temporal del sistema.

---

## Funciones booleanas

### Módulo de autenticación

> _Describir las funciones lógicas del comparador de 4 bits. La señal de habilitación (ENABLE) se activa cuando la contraseña ingresada coincide con la almacenada._

Sean:
- **A[3:0]** = bits de la contraseña almacenada en flip-flops
- **B[3:0]** = bits de la contraseña ingresada por el usuario

La señal de igualdad del comparador 7485 se define como:

```
EQUAL = (A3 XNOR B3) AND (A2 XNOR B2) AND (A1 XNOR B1) AND (A0 XNOR B0)
```

Equivalentemente con XOR:

```
EQUAL = NOT(A3 XOR B3) AND NOT(A2 XOR B2) AND NOT(A1 XOR B1) AND NOT(A0 XOR B0)
```

### Módulo de contador de errores

> _El contador de errores se implementa con flip-flops D en cascada. Describir la lógica de incremento y la condición de activación de alarma._

Sean **Q1, Q0** los bits del contador de errores (cuenta de 0 a 3):

```
ALARMA = Q1 AND Q0    (se activa cuando el contador llega a 3 = 11 en binario)
```

Función de próximo estado del contador (incremento en 1):

```
D0 = NOT(Q0)
D1 = Q1 XOR Q0
```

### Módulo de control del motor (Puente H)

> _Describir las señales de control del puente H según la fase activa._

Sean:
- **FASE** = 0: giro en primera dirección (LED verde, 15 s)
- **FASE** = 1: giro en dirección contraria (LED rojo, 10 s)
- **EN** = señal de habilitación del comparador

```
IN1 = EN AND NOT(FASE)
IN2 = EN AND FASE
LED_VERDE = EN AND NOT(FASE)
LED_ROJO  = EN AND FASE
```

---

## Mapas de karnaugh

### Alarma del contador de errores

> _Simplificación de la función ALARMA en función de Q1, Q0 (contador de 2 bits que cuenta hasta 3)._

```
        Q0
Q1  |  0  |  1  |
 0  |  0  |  0  |
 1  |  0  |  1  |
```

**Resultado minimizado:** `ALARMA = Q1 · Q0`

> _Agregar aquí los mapas de Karnaugh adicionales que correspondan a su diseño específico._

---

## Diagramas del diseño del circuito

### Diagrama general del sistema
---

### Contraseña

![image](https://hackmd.io/_uploads/H1DCSOeTZg.png)

### Contador
![image](https://hackmd.io/_uploads/B1yBUdx6Zl.png)

### Carrusel
![image](https://hackmd.io/_uploads/HyAiUOlaWl.png)


---

### Módulo de autenticación

La contraseña se almacena en 4 flip-flops 74LS74 al presionar el botón "Guardar". El usuario ingresa su clave con un segundo dip-switch y presiona "Ingresar". El comparador 7485 valida la igualdad bit a bit y genera la señal `EQUAL` que habilita el sistema.

---

### Módulo de contador de errores y alarma

Cada intento incorrecto genera un pulso que incrementa el contador implementado con flip-flops D en cascada. El valor se muestra en un display de 7 segmentos mediante el decodificador 7447/7448. Al llegar a 3 errores se activa el buzzer/LED de alarma.

---

### Módulo de control del motor (Puente H)

El puente H está construido con transistores NPN (2N2222A) y PNP (2N2907), con diodos de protección 1N4007. Las señales digitales del Arduino controlan los transistores para invertir la polaridad del motor DC según la fase activa.

---

### Módulo de temporización y displays

El Arduino genera pulsos de reloj de 1 Hz. Un contador ascendente (0–15) gestiona la fase verde y un contador descendente (10–0) gestiona la fase roja. Las salidas pasan por decodificadores BCD-7 segmentos (7447/7448) que alimentan los displays.

---

## Componentes utilizado

| Cantidad | Componente | Código / Referencia |
|----------|------------|---------------------|
| 8 | Flip-Flop D doble | 74LS74 |
| 4 | Flip-Flop D séxtuple | 74LS174 |
| 1 | Comparador de 4 bits | 7485 |
| 2 | Decodificador BCD-7 seg | 7447 / 7448 |
| 1 | Multiplexor | 74157 |
| 2 | Compuerta AND | 7408 |
| 1 | Compuerta NOT | 7404 |
| 1 | Compuerta XOR | 7486 |
| 1 | Compuerta OR | 7432 |
| 1 | Driver de motor DC | L293D |
| 6 | Diodo rectificador | 1N4007 |
| 1 | Motor DC | — |
| 2 | Display 7 segmentos | Ánodo/Cátodo común |
| 1 | Buzzer | — |
| 1 | LED Verde | — |
| 1 | LED Rojo | — |
| 2 | Dip-Switch 4 posiciones | — |
| 3 | Push Button | — |
| 1 | Arduino Uno/Mega | — |
| 1 | Protoboard | — |
| — | Resistencias varias | 220Ω, 1kΩ, 10kΩ |
| — | Cables jumper | — |

---

## Presupuesto

Los componentes se dividen en dos categorías: los adquiridos específicamente para este proyecto y los que ya estaban disponibles de proyectos anteriores.

### Componentes comprados 

| Componente | Descripción | Cantidad | Precio Unitario (Q) | Total (Q) |
|------------|-------------|----------|---------------------|-----------|
| 74LS74 (7474) | Flip-Flop tipo D Dual | 8 | Q 8.00 | Q 64.00 |
| 74LS174 (74174) | Flip-Flop tipo D Hex | 4 | Q 17.00 | Q 68.00 |
| 1N4007 (4007) | Diodo Rectificador 1000V 1A | 6 | Q 1.00 | Q 6.00 |
| Pulsador (PUL) | Pulsador 2 pines 6x6x5mm | 3 | Q 1.00 | Q 3.00 |
| L293D (293D) | Driver de Motores DC 1A | 1 | Q 10.00 | Q 10.00 |
| **Subtotal compras** | | | | **Q 151.00** |

### Componentes que ya se disponia de ellos

| Componente | Código / Referencia | Cantidad |
|------------|---------------------|----------|
| Comparador de 4 bits | 7485 | 1 |
| Decodificador BCD-7 seg | 7447 / 7448 | 2 |
| Multiplexor | 74157 | 1 |
| Compuerta AND | 7408 | 2 |
| Compuerta NOT | 7404 | 1 |
| Compuerta XOR | 7486 | 1 |
| Compuerta OR | 7432 | 1 |
| Motor DC | — | 1 |
| Display 7 segmentos | — | 2 |
| Buzzer | — | 1 |
| LEDs (verde y rojo) | — | 2 |
| Dip-Switch 4 posiciones | — | 2 |
| Arduino Uno/Mega | — | 1 |
| Protoboard | — | 1 |
| Resistencias varias | 220Ω, 1kΩ, 10kΩ | — |
| Cables jumper | — | — |
| **Subtotal disponible** | | **Q 0.00** |

---

## Conclusiones

La implementación del sistema demostró que los flip-flops tipo D (74LS74) constituyen el bloque fundamental para cualquier sistema digital que requiera retención de estado. Su uso en cascada para construir contadores de errores y de tiempo permitió comprender que la lógica secuencial no es simplemente "memoria", sino un mecanismo preciso de avance de estados gobernado por ecuaciones de estado siguiente bien definidas. Por su parte, el comparador 7485 evidenció la potencia de los circuitos combinacionales especializados: con un único integrado fue posible realizar la validación completa de una contraseña de 4 bits, tarea que habría requerido múltiples compuertas si se hubiera construido desde cero. Finalmente, los decodificadores 7447/7448 pusieron de manifiesto la importancia de la capa de presentación en un sistema digital: convertir información binaria en un formato legible para el usuario (display de 7 segmentos) es tan esencial como la lógica interna que genera esa información.
 
 ---
 
El módulo de autenticación resultó ser el núcleo más delicado del proyecto, ya que involucra la correcta interacción entre circuitos de almacenamiento (flip-flops), validación (comparador) y conteo de fallos (contador en cascada). Se comprendió que el diseño de sistemas de seguridad digitales exige considerar no solo el camino exitoso (contraseña correcta), sino también los caminos de fallo: qué ocurre si se excede el límite de intentos, cómo se persiste la alarma una vez activada y bajo qué condición exacta debe desactivarse y reiniciarse el contador. Estos requerimientos, que parecen simples en el enunciado, exigieron un análisis cuidadoso de las señales de habilitación, reset y clock de cada flip-flop involucrado, reforzando la habilidad de traducir especificaciones funcionales en circuitos digitales concretos.
 
 ---
 
Construir el puente H con transistores discretos (2N2222A y 2N2907) en lugar de usar un integrado como el L293D reveló la complejidad real del control de potencia en sistemas embebidos. Se comprendió que el transistor, en este contexto, opera como un interruptor controlado por corriente de base, y que la correcta elección de resistencias de base determina la saturación del transistor y, con ello, la eficiencia de la conmutación. Igualmente importante fue la inclusión de los diodos de protección 1N4007 para suprimir los picos de tensión inductiva del motor al cortar la corriente, un aspecto que podría dañar irreversiblemente los transistores si se omite. La experiencia dejó en claro que el diseño digital no termina en las compuertas lógicas: integrar la etapa de potencia correctamente es fundamental para que el sistema sea robusto y funcione de forma estable en condiciones reales.
 
 ---
 
Delimitar estrictamente el rol del Arduino a la generación de reloj y al control temporal fue una decisión de diseño que fortaleció la comprensión de la arquitectura del sistema. Al prohibir que el Arduino ejecute la lógica de comparación o el conteo de errores, se hizo evidente la separación de responsabilidades entre el procesador de propósito general y los circuitos lógicos discretos: el Arduino es rápido y flexible para gestionar tiempos, pero es los flip-flops y comparadores quienes garantizan la integridad y persistencia del estado del sistema ante cualquier reinicio o interrupción del microcontrolador. Esta restricción también facilitó el depurado del proyecto, ya que cada módulo podía verificarse de forma independiente, evidenciando las ventajas del diseño modular en sistemas digitales complejos.

---

## Fotografías de la practica en físico 

![WhatsApp Image 2026-04-17 at 9.17.30 PM (1)](https://hackmd.io/_uploads/r1-tO_xpWx.jpg)

![WhatsApp Image 2026-04-17 at 9.17.30 PM (2)](https://hackmd.io/_uploads/BJc9dOlTbe.jpg)

![WhatsApp Image 2026-04-17 at 9.17.30 PM](https://hackmd.io/_uploads/r1J2_dl6bx.jpg)
