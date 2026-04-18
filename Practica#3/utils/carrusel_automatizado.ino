/*
 * ============================================================
 *  PRÁCTICA 3 - Carrusel Automatizado con Control de Acceso
 *  Organización Computacional - USAC ECYS
 *  1S2026
 * ============================================================
 *
 *  ROL DEL ARDUINO (según enunciado):
 *   1) Generar señal de reloj de 1 Hz para los contadores
 *      externos (flip-flops 74LS74 / 74LS174).
 *   2) Controlar el puente H: dirección del motor DC.
 *   3) Leer la señal ENABLE del comparador 7485 (A=B).
 *   4) Leer la señal ALARMA del detector de 3 errores (AND Q1·Q0).
 *   5) Gestionar la secuencia de fases:
 *        Fase VERDE  → 15 s, motor dirección 1
 *        Fase ROJA   → 10 s, motor dirección 2
 *
 *  PROHIBIDO en el Arduino: comparar contraseña, contar errores,
 *  almacenar contraseña, tomar decisiones de autenticación.
 *
 * ============================================================
 *  MAPA DE PINES
 * ============================================================
 *
 *  SALIDAS:
 *   PIN 2  → CLK_OUT   : Reloj 1 Hz para flip-flops externos
 *   PIN 3  → IN1       : Puente H - control transistor Q1 (NPN izq.)
 *   PIN 4  → IN2       : Puente H - control transistor Q2 (NPN der.)
 *   PIN 5  → LED_VERDE : Fase 1 (15 s) - también va a CLR del contador ascendente
 *   PIN 6  → LED_ROJO  : Fase 2 (10 s) - también va a CLR del contador descendente
 *   PIN 7  → CLR_CONT  : Clear/Reset de los contadores externos de tiempo
 *                         (activo en BAJO → señal se invierte con NOT 7404)
 *
 *  ENTRADAS:
 *   PIN 8  → ENABLE    : Señal A=B del comparador 7485 (HIGH = contraseña correcta)
 *   PIN 9  → ALARMA    : Señal AND(Q1,Q0) del detector de 3 errores (HIGH = bloqueado)
 *
 * ============================================================
 */

// ── Pines de salida ─────────────────────────────────────────
const int PIN_CLK       = 2;   // Reloj 1 Hz → flip-flops externos
const int PIN_IN1       = 3;   // Puente H - transistor izquierdo
const int PIN_IN2       = 4;   // Puente H - transistor derecho
const int PIN_LED_VERDE = 5;   // LED verde (fase 1)
const int PIN_LED_ROJO  = 6;   // LED rojo  (fase 2)
const int PIN_CLR_CONT  = 7;   // Clear contadores de tiempo (LOW activo, invertir con 7404)

// ── Pines de entrada ────────────────────────────────────────
const int PIN_ENABLE    = 8;   // A=B del 7485 (HIGH = acceso válido)
const int PIN_ALARMA    = 9;   // AND(Q1,Q0) detector errores (HIGH = alarma)

// ── Constantes de tiempo ────────────────────────────────────
const unsigned long PERIODO_CLK_MS = 500;  // 500 ms → 1 Hz (toggle cada 500 ms)
const unsigned long TIEMPO_VERDE_MS = 15000UL;  // 15 segundos fase verde
const unsigned long TIEMPO_ROJO_MS  = 10000UL;  // 10 segundos fase roja

// ── Estados de la máquina de estados ────────────────────────
typedef enum {
  ESTADO_ESPERA,        // Esperando contraseña correcta (ENABLE = LOW o ALARMA = HIGH)
  ESTADO_INICIO_VERDE,  // Preparar fase verde
  ESTADO_FASE_VERDE,    // Motor girando dirección 1 durante 15 s
  ESTADO_INICIO_ROJO,   // Preparar fase roja
  ESTADO_FASE_ROJA,     // Motor girando dirección 2 durante 10 s
  ESTADO_FIN_CICLO      // Ciclo completo, detener motor y volver a espera
} Estado;

// ── Variables globales ───────────────────────────────────────
Estado estadoActual       = ESTADO_ESPERA;
unsigned long tInicioFase = 0;
unsigned long tUltimoCLK  = 0;
bool nivelCLK             = LOW;

// ── Prototipos ───────────────────────────────────────────────
void motorDetener();
void motorDireccion1();
void motorDireccion2();
void limpiarLEDs();
void resetContadoresTiempo();

// ============================================================
void setup() {
  // Salidas
  pinMode(PIN_CLK,       OUTPUT);
  pinMode(PIN_IN1,       OUTPUT);
  pinMode(PIN_IN2,       OUTPUT);
  pinMode(PIN_LED_VERDE, OUTPUT);
  pinMode(PIN_LED_ROJO,  OUTPUT);
  pinMode(PIN_CLR_CONT,  OUTPUT);

  // Entradas con pull-down externo (10k a GND en el circuito)
  pinMode(PIN_ENABLE,  INPUT);
  pinMode(PIN_ALARMA,  INPUT);

  // Estado inicial seguro
  motorDetener();
  limpiarLEDs();
  resetContadoresTiempo();

  // El CLK empieza en LOW
  digitalWrite(PIN_CLK, LOW);
  nivelCLK    = LOW;
  tUltimoCLK  = millis();

  Serial.begin(9600);
  Serial.println(F("=== Carrusel Automatizado - USAC ECYS 1S2026 ==="));
  Serial.println(F("Esperando autenticacion correcta..."));
}

// ============================================================
void loop() {
  unsigned long ahora = millis();

  // ── 1) GENERAR RELOJ 1 Hz (independiente del estado) ──────
  //    Toggle cada 500 ms para producir onda cuadrada de 1 Hz.
  //    El reloj corre SIEMPRE para alimentar los contadores
  //    externos de tiempo (se habilitan/deshabilitan con CLR_CONT).
  if (ahora - tUltimoCLK >= PERIODO_CLK_MS) {
    tUltimoCLK = ahora;
    nivelCLK   = !nivelCLK;
    digitalWrite(PIN_CLK, nivelCLK);
  }

  // ── 2) LEER SEÑALES DE HARDWARE ───────────────────────────
  bool enable = digitalRead(PIN_ENABLE);   // HIGH = contraseña OK
  bool alarma = digitalRead(PIN_ALARMA);   // HIGH = 3 errores, bloqueado

  // ── 3) MÁQUINA DE ESTADOS ─────────────────────────────────
  switch (estadoActual) {

    // ── ESPERA: motor parado, esperando ENABLE sin ALARMA ───
    case ESTADO_ESPERA:
      if (enable && !alarma) {
        Serial.println(F("[OK] Acceso valido. Iniciando secuencia del carrusel."));
        estadoActual = ESTADO_INICIO_VERDE;
      }
      break;

    // ── INICIO VERDE: configurar hardware para fase 1 ───────
    case ESTADO_INICIO_VERDE:
      resetContadoresTiempo();    // CLR contadores → los pone a 0 (o valor inicial)
      delay(5);                   // Pulso mínimo de reset
      digitalWrite(PIN_CLR_CONT, HIGH);  // Liberar clear → contadores habilitados

      motorDireccion1();
      digitalWrite(PIN_LED_VERDE, HIGH);
      digitalWrite(PIN_LED_ROJO,  LOW);

      tInicioFase  = millis();
      estadoActual = ESTADO_FASE_VERDE;
      Serial.println(F("[VERDE] Motor ON - Direccion 1 - 15 segundos"));
      break;

    // ── FASE VERDE: 15 s contando. Vigilar alarma ───────────
    case ESTADO_FASE_VERDE:
      // Si durante la fase se activa la alarma → parar todo
      if (alarma) {
        Serial.println(F("[ALARMA] Detectada durante fase verde. Deteniendo."));
        motorDetener();
        limpiarLEDs();
        resetContadoresTiempo();
        estadoActual = ESTADO_ESPERA;
        break;
      }
      if (ahora - tInicioFase >= TIEMPO_VERDE_MS) {
        estadoActual = ESTADO_INICIO_ROJO;
        Serial.println(F("[VERDE] Fase completa."));
      }
      break;

    // ── INICIO ROJO: configurar hardware para fase 2 ────────
    case ESTADO_INICIO_ROJO:
      resetContadoresTiempo();
      delay(5);
      digitalWrite(PIN_CLR_CONT, HIGH);

      motorDireccion2();
      digitalWrite(PIN_LED_VERDE, LOW);
      digitalWrite(PIN_LED_ROJO,  HIGH);

      tInicioFase  = millis();
      estadoActual = ESTADO_FASE_ROJA;
      Serial.println(F("[ROJO] Motor ON - Direccion 2 - 10 segundos"));
      break;

    // ── FASE ROJA: 10 s contando. Vigilar alarma ────────────
    case ESTADO_FASE_ROJA:
      if (alarma) {
        Serial.println(F("[ALARMA] Detectada durante fase roja. Deteniendo."));
        motorDetener();
        limpiarLEDs();
        resetContadoresTiempo();
        estadoActual = ESTADO_ESPERA;
        break;
      }
      if (ahora - tInicioFase >= TIEMPO_ROJO_MS) {
        estadoActual = ESTADO_FIN_CICLO;
        Serial.println(F("[ROJO] Fase completa."));
      }
      break;

    // ── FIN DE CICLO: detener motor y volver a espera ───────
    case ESTADO_FIN_CICLO:
      motorDetener();
      limpiarLEDs();
      resetContadoresTiempo();
      Serial.println(F("[FIN] Ciclo completo. Esperando nueva autenticacion..."));
      estadoActual = ESTADO_ESPERA;
      break;
  }
}

// ============================================================
//  FUNCIONES AUXILIARES
// ============================================================

/** Detiene el motor (ambas entradas del puente H en LOW) */
void motorDetener() {
  digitalWrite(PIN_IN1, LOW);
  digitalWrite(PIN_IN2, LOW);
}

/** Gira el motor en dirección 1 (fase verde) */
void motorDireccion1() {
  digitalWrite(PIN_IN1, HIGH);
  digitalWrite(PIN_IN2, LOW);
}

/** Gira el motor en dirección 2 (fase roja) */
void motorDireccion2() {
  digitalWrite(PIN_IN1, LOW);
  digitalWrite(PIN_IN2, HIGH);
}

/** Apaga ambos LEDs de fase */
void limpiarLEDs() {
  digitalWrite(PIN_LED_VERDE, LOW);
  digitalWrite(PIN_LED_ROJO,  LOW);
}

/**
 * Envía un pulso LOW en PIN_CLR_CONT para resetear los
 * contadores de tiempo externos (flip-flops).
 * Los CLR de los 74LS74/74LS174 son activos en BAJO.
 * El pin pasa por un NOT (7404) en el circuito, así que
 * LOW aquí → HIGH en CLR → no reset.
 * Para reset real: poner HIGH aquí → LOW en CLR del FF.
 *
 * NOTA: si se conecta directamente (sin inversor) al CLR
 * del flip-flop, invertir la lógica de esta función.
 */
void resetContadoresTiempo() {
  // Pulso de reset: CLR activo en bajo en el 74LS74
  // → Pin Arduino en HIGH pasa por NOT 7404 → LOW en CLR
  digitalWrite(PIN_CLR_CONT, HIGH);
  // Pequeño retardo para asegurar reset
  delayMicroseconds(100);
  // Liberar reset
  digitalWrite(PIN_CLR_CONT, LOW);
}
