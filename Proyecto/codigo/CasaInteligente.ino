/*
 * ============================================================
 *  CASA INTELIGENTE - Control de Ambientes y Ventilador
 *  Organización Computacional — ECYS / USAC
 *  Hardware: Arduino Mega 2560
 * ============================================================
 *
 *  CAMBIO APROBADO:
 *    En lugar de múltiples LEDs de color fijo por ambiente,
 *    se usa UN LED RGB de cátodo común (4 pines) por ambiente.
 *    Cada modo asigna un color distinto a cada ambiente.
 *
 *  PINOUT GENERAL (Arduino Mega):
 *  ─────────────────────────────────────────────────────────
 *  Bluetooth HC-05/06  → Serial1 (TX1=18, RX1=19)
 *  LCD I2C             → SDA=20, SCL=21 (hardware I2C Mega)
 *  Servo (puerta)      → Pin 6
 *  Motor DC (ventilador) → Pin 7  (señal al transistor NPN)
 *  Botón puerta        → Pin 2   (con resistencia pulldown 10k)
 *
 *  LEDs de Estado:
 *    LED Azul  (sistema activo)   → Pin 22
 *    LED Verde (config OK)        → Pin 23
 *    LED Rojo  (error)            → Pin 24
 *
 *  LEDs RGB por ambiente (cátodo común):
 *    Sala     → R=25  G=26  B=27
 *    Comedor  → R=28  G=29  B=30
 *    Cocina   → R=31  G=32  B=33
 *    Baño     → R=34  G=35  B=36
 *    Habitación→ R=37  G=38  B=39
 *
 *  COMANDOS BLUETOOTH — MODOS GLOBALES:
 *    modo_fiesta | modo_relajado | modo_noche
 *    encender_todo | apagar_todo | estado
 *
 *  COMANDOS BLUETOOTH — CONTROL INDIVIDUAL POR AMBIENTE:
 *    sala:on      sala:off
 *    comedor:on   comedor:off
 *    cocina:on    cocina:off
 *    banio:on     banio:off
 *    habitacion:on   → enciende LED + VENTILADOR ON automático
 *    habitacion:off  → apaga LED  + VENTILADOR OFF automático
 *    ventilador:on   ventilador:off  (control manual independiente)
 *
 *  REGLA ESPECIAL:
 *    Al encender/apagar la habitación individualmente,
 *    el ventilador se sincroniza automáticamente con ese estado.
 *
 *  TABLA EEPROM:
 *    Dirección 0  → Modo guardado actual (0-6)
 *    Dirección 10 → Byte de validación (0xAB = datos válidos)
 *    Dirección 20 → Ventilador en modo_fiesta   (0/1)
 *    Dirección 21 → Ventilador en modo_relajado (0/1)
 *    Dirección 22 → Ventilador en modo_noche    (0/1)
 *    Dirección 23 → Ventilador en encender_todo (0/1)
 *    Dirección 24 → Ventilador en apagar_todo   (0/1)
 *    Dirección 30 → Bandera org cargado (0xCD = archivo .org OK)
 *    Dirección 40 → Estado individual LED Sala        (0/1)
 *    Dirección 41 → Estado individual LED Comedor     (0/1)
 *    Dirección 42 → Estado individual LED Cocina      (0/1)
 *    Dirección 43 → Estado individual LED Baño        (0/1)
 *    Dirección 44 → Estado individual LED Habitación  (0/1)
 *    Dirección 45 → Estado individual Ventilador      (0/1)
 * ============================================================
 */

#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// ─────────────────────────────────────────────
//  CONSTANTES DE PINES
// ─────────────────────────────────────────────

// LEDs de estado
#define PIN_LED_AZUL    22
#define PIN_LED_VERDE   23
#define PIN_LED_ROJO    24

// LEDs RGB — Sala
#define PIN_SALA_R      25
#define PIN_SALA_G      26
#define PIN_SALA_B      27

// LEDs RGB — Comedor
#define PIN_COMEDOR_R   28
#define PIN_COMEDOR_G   29
#define PIN_COMEDOR_B   30

// LEDs RGB — Cocina
#define PIN_COCINA_R    31
#define PIN_COCINA_G    32
#define PIN_COCINA_B    33

// LEDs RGB — Baño
#define PIN_BANIO_R     34
#define PIN_BANIO_G     35
#define PIN_BANIO_B     36

// LEDs RGB — Habitación
#define PIN_HABIT_R     37
#define PIN_HABIT_G     38
#define PIN_HABIT_B     39

// Actuadores
#define PIN_MOTOR       7   // Transistor NPN base → ventilador DC
#define PIN_SERVO       6   // Servomotor puerta
#define PIN_BOTON       2   // Push button puerta (pulldown externo)

// ─────────────────────────────────────────────
//  DIRECCIONES EEPROM
// ─────────────────────────────────────────────
#define EEPROM_MODO_ACTUAL   0
#define EEPROM_VALIDACION    10
#define EEPROM_FAN_FIESTA    20
#define EEPROM_FAN_RELAJADO  21
#define EEPROM_FAN_NOCHE     22
#define EEPROM_FAN_TODO_ON   23
#define EEPROM_FAN_TODO_OFF  24
#define EEPROM_ORG_FLAG      30
#define EEPROM_LED_SALA      40   // estado individual LED sala
#define EEPROM_LED_COMEDOR   41
#define EEPROM_LED_COCINA    42
#define EEPROM_LED_BANIO     43
#define EEPROM_LED_HABIT     44
#define EEPROM_FAN_ESTADO    45   // estado ventilador en modo individual
#define EEPROM_VALID_BYTE    0xAB
#define EEPROM_ORG_BYTE      0xCD

// ─────────────────────────────────────────────
//  MODOS DEL SISTEMA
// ─────────────────────────────────────────────
#define MODO_NINGUNO       0
#define MODO_FIESTA        1
#define MODO_RELAJADO      2
#define MODO_NOCHE         3
#define MODO_ENCENDER_TODO 4
#define MODO_APAGAR_TODO   5
#define MODO_INDIVIDUAL    6   // control por ambiente individual

// ─────────────────────────────────────────────
//  OBJETOS GLOBALES
// ─────────────────────────────────────────────
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo servoPuerta;

// ─────────────────────────────────────────────
//  VARIABLES DE ESTADO
// ─────────────────────────────────────────────
int  modoActual       = MODO_NINGUNO;
bool puertaAbierta    = false;
bool cargandoOrg      = false;   // bloquea Bluetooth durante carga .org
bool errorSistema     = false;

// Botón — debounce sin blocking
bool     estadoBotonPrev    = LOW;
uint32_t tiempoUltimoBoton  = 0;
#define  DEBOUNCE_MS         50

// Modo fiesta — parpadeo/alternancia LEDs sin blocking
uint32_t tiempoUltimoAlter  = 0;
bool     alterEstado         = false;
#define  INTERVALO_ALTER_MS  400

// LED verde — parpadeo éxito
bool     parpadeandoVerde    = false;
uint8_t  parpadeoCount       = 0;
uint32_t tiempoParpadeoVerde = 0;
#define  INTERVALO_PARPV_MS  300

// Recepción comandos Bluetooth
String cmdBT = "";

// Recepción archivo .org por USB serial
String lineaOrg   = "";
bool   orgIniciado = false;   // se encontró conf_ini
int    modoOrgActual = MODO_NINGUNO;

// Datos de modos cargados desde .org (ventilador)
// Los colores por modo son fijos en código; el ventilador puede venir del .org
bool fanPorModo[6] = {false, true, false, false, true, false};
// índices: 0=ninguno, 1=fiesta, 2=relajado, 3=noche, 4=enc_todo, 5=ap_todo

// ─────────────────────────────────────────────
//  ESTADO INDIVIDUAL DE AMBIENTES
// ─────────────────────────────────────────────
// Estado ON/OFF de cada ambiente (control individual)
bool ledSalaOn      = false;
bool ledComedorOn   = false;
bool ledCocinaOn    = false;
bool ledBanioOn     = false;
bool ledHabitOn     = false;
bool fanOn          = false;  // estado actual del ventilador

// Colores de cada ambiente cuando se encienden individualmente:
// Sala=Blanco, Comedor=Amarillo, Cocina=Cian, Baño=Azul, Habitación=Verde
// (R, G, B para cada ambiente)
const int COLOR_SALA_R=1, COLOR_SALA_G=1, COLOR_SALA_B=1;
const int COLOR_COM_R=1,  COLOR_COM_G=1,  COLOR_COM_B=0;
const int COLOR_COC_R=0,  COLOR_COC_G=1,  COLOR_COC_B=1;
const int COLOR_BAN_R=0,  COLOR_BAN_G=0,  COLOR_BAN_B=1;
const int COLOR_HAB_R=0,  COLOR_HAB_G=1,  COLOR_HAB_B=0;

// ─────────────────────────────────────────────
//  PROTOTIPOS
// ─────────────────────────────────────────────
void aplicarModo(int modo);
void setRGB(int pR, int pG, int pB, int r, int g, int b);
void apagarTodosRGB();
void encenderTodosRGB();
void alternarFiesta();
void procesarComandoBT(String cmd);
bool procesarControlIndividual(String cmd);   // NUEVO
void encenderAmbiente(const String& nombre, bool encender); // NUEVO
void procesarLineaOrg(String linea);
void finalizarCargaOrg(bool exito);
void lcdMostrar(const char* fila0, const char* fila1);
void iniciarParpadeVerde();
void tickParpadeVerde();
void guardarModoEEPROM(int modo);
int  leerModoEEPROM();
void guardarFanEEPROM();
void leerFanEEPROM();
void guardarEstadosIndividuales();  // NUEVO
void leerEstadosIndividuales();     // NUEVO
void manejarBoton();

// ─────────────────────────────────────────────
//  SETUP
// ─────────────────────────────────────────────
void setup() {
  // Seriales
  Serial.begin(9600);    // USB — carga archivo .org + monitor
  Serial1.begin(9600);   // Bluetooth HC-05/06 (pines 18/19 Mega)

  // LEDs de estado
  pinMode(PIN_LED_AZUL,  OUTPUT);
  pinMode(PIN_LED_VERDE, OUTPUT);
  pinMode(PIN_LED_ROJO,  OUTPUT);

  // LEDs RGB ambientes
  int rgbPins[] = {
    PIN_SALA_R,    PIN_SALA_G,    PIN_SALA_B,
    PIN_COMEDOR_R, PIN_COMEDOR_G, PIN_COMEDOR_B,
    PIN_COCINA_R,  PIN_COCINA_G,  PIN_COCINA_B,
    PIN_BANIO_R,   PIN_BANIO_G,   PIN_BANIO_B,
    PIN_HABIT_R,   PIN_HABIT_G,   PIN_HABIT_B
  };
  for (int i = 0; i < 15; i++) {
    pinMode(rgbPins[i], OUTPUT);
    digitalWrite(rgbPins[i], LOW);
  }

  // Motor DC
  pinMode(PIN_MOTOR, OUTPUT);
  digitalWrite(PIN_MOTOR, LOW);

  // Botón
  pinMode(PIN_BOTON, INPUT);  // resistencia pulldown externa 10k

  // Servo — puerta siempre cerrada al inicio
  servoPuerta.attach(PIN_SERVO);
  servoPuerta.write(0);   // 0° = cerrado
  puertaAbierta = false;

  // LCD
  lcd.init();
  lcd.backlight();

  // Leer EEPROM
  leerFanEEPROM();
  leerEstadosIndividuales();   // NUEVO: restaurar estados individuales
  int modoGuardado = leerModoEEPROM();

  // LED azul ON — sistema activo
  digitalWrite(PIN_LED_AZUL, HIGH);
  digitalWrite(PIN_LED_VERDE, LOW);
  digitalWrite(PIN_LED_ROJO, LOW);

  // Mostrar inicio
  lcdMostrar("Casa Inteligente", "Iniciando...");
  delay(1000);

  // Restaurar último modo si hay uno guardado válido
  if (modoGuardado != MODO_NINGUNO) {
    aplicarModo(modoGuardado);
  } else {
    lcdMostrar("Sistema listo", "Esperando cmd");
  }

  Serial.println(F("=== CASA INTELIGENTE LISTA ==="));
  Serial.println(F("Envie archivo .org por este puerto."));
  Serial.println(F("Modos: modo_fiesta|modo_relajado|modo_noche|encender_todo|apagar_todo|estado"));
  Serial.println(F("Individual: sala:on/off | comedor:on/off | cocina:on/off | banio:on/off | habitacion:on/off | ventilador:on/off"));
  Serial.println(F("NOTA: habitacion:on enciende ventilador automaticamente"));
}

// ─────────────────────────────────────────────
//  LOOP PRINCIPAL
// ─────────────────────────────────────────────
void loop() {
  uint32_t ahora = millis();

  // ── 1. Botón puerta (siempre activo) ──
  manejarBoton();

  // ── 2. Leer archivo .org desde USB Serial ──
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (lineaOrg.length() > 0) {
        procesarLineaOrg(lineaOrg);
        lineaOrg = "";
      }
    } else {
      lineaOrg += c;
    }
  }

  // ── 3. Leer comandos Bluetooth (solo si no cargando .org) ──
  if (!cargandoOrg) {
    while (Serial1.available()) {
      char c = Serial1.read();
      if (c == '\n' || c == '\r') {
        cmdBT.trim();
        if (cmdBT.length() > 0) {
          procesarComandoBT(cmdBT);
          cmdBT = "";
        }
      } else {
        cmdBT += c;
      }
    }
  }

  // ── 4. Animación modo FIESTA (alternancia LEDs) ──
  if (modoActual == MODO_FIESTA) {
    if (ahora - tiempoUltimoAlter >= INTERVALO_ALTER_MS) {
      tiempoUltimoAlter = ahora;
      alternarFiesta();
    }
  }

  // ── 5. Parpadeo LED verde (éxito config) ──
  tickParpadeVerde();
}

// ─────────────────────────────────────────────
//  MANEJO DE BOTÓN (debounce sin blocking)
// ─────────────────────────────────────────────
void manejarBoton() {
  bool estadoActual = digitalRead(PIN_BOTON);
  uint32_t ahora    = millis();

  if (estadoActual != estadoBotonPrev && (ahora - tiempoUltimoBoton) > DEBOUNCE_MS) {
    tiempoUltimoBoton = ahora;
    if (estadoActual == HIGH) {
      // Flanco de subida → toggle puerta
      puertaAbierta = !puertaAbierta;
      if (puertaAbierta) {
        servoPuerta.write(90);  // 90° = abierto
        lcdMostrar("Puerta: ABIERTA", "");
        Serial.println(F("Puerta abierta"));
        Serial1.println(F("Puerta: ABIERTA"));
      } else {
        servoPuerta.write(0);   // 0°  = cerrado
        lcdMostrar("Puerta: CERRADA", "");
        Serial.println(F("Puerta cerrada"));
        Serial1.println(F("Puerta: CERRADA"));
      }
      // Restaurar LCD al modo actual tras 1.5 s no bloqueante no es trivial;
      // se opta por reconmostar el modo en el siguiente comando.
    }
  }
  estadoBotonPrev = estadoActual;
}

// ─────────────────────────────────────────────
//  APLICAR MODO
// ─────────────────────────────────────────────
void aplicarModo(int modo) {
  modoActual   = modo;
  errorSistema = false;
  digitalWrite(PIN_LED_ROJO, LOW);
  // Apagar motor y LEDs antes de aplicar
  fanOn = false;
  digitalWrite(PIN_MOTOR, LOW);
  apagarTodosRGB();
  // Resetear estados individuales (el modo global los sobreescribe)
  ledSalaOn = ledComedorOn = ledCocinaOn = ledBanioOn = ledHabitOn = false;

  switch (modo) {

    case MODO_FIESTA:
      // Ventilador ON
      if (fanPorModo[MODO_FIESTA]) { fanOn = true; digitalWrite(PIN_MOTOR, HIGH); }
      // LEDs: la alternancia la hace el loop con alternarFiesta()
      // Iniciamos con la primera mitad encendida
      alterEstado = false;
      alternarFiesta();
      lcdMostrar("Modo: FIESTA.   ", "Fan:ON LEDs:Alt.");
      Serial.println(F("Modo FIESTA activado"));
      Serial1.println(F("OK: modo_fiesta"));
      break;

    case MODO_RELAJADO:
      if (fanPorModo[MODO_RELAJADO]) { fanOn = true; digitalWrite(PIN_MOTOR, HIGH); }
      lcdMostrar("Modo: RELAJADO. ", "Fan:OFF LEDs:OFF");
      Serial.println(F("Modo RELAJADO activado"));
      Serial1.println(F("OK: modo_relajado"));
      break;

    case MODO_NOCHE:
      if (fanPorModo[MODO_NOCHE]) { fanOn = true; digitalWrite(PIN_MOTOR, HIGH); }
      lcdMostrar("Modo: NOCHE.    ", "Fan:OFF LEDs:OFF");
      Serial.println(F("Modo NOCHE activado"));
      Serial1.println(F("OK: modo_noche"));
      break;

    case MODO_ENCENDER_TODO:
      if (fanPorModo[MODO_ENCENDER_TODO]) { fanOn = true; digitalWrite(PIN_MOTOR, HIGH); }
      encenderTodosRGB();
      // Marcar todos los ambientes como encendidos
      ledSalaOn = ledComedorOn = ledCocinaOn = ledBanioOn = ledHabitOn = true;
      lcdMostrar("LED'S: ON.      ", "Ventilador: ON. ");
      Serial.println(F("Modo ENCENDER TODO activado"));
      Serial1.println(F("OK: encender_todo"));
      break;

    case MODO_APAGAR_TODO:
      lcdMostrar("LED'S: OFF.     ", "Ventilador: OFF.");
      Serial.println(F("Modo APAGAR TODO activado"));
      Serial1.println(F("OK: apagar_todo"));
      break;

    default:
      return;  // No guardar modos desconocidos
  }

  guardarModoEEPROM(modo);
}

// ─────────────────────────────────────────────
//  COLORES RGB POR AMBIENTE (modo encender_todo)
//  Sala=blanco, Comedor=amarillo, Cocina=cian,
//  Baño=azul, Habitación=verde suave
// ─────────────────────────────────────────────
void encenderTodosRGB() {
  setRGB(PIN_SALA_R,    PIN_SALA_G,    PIN_SALA_B,    1, 1, 1);  // Blanco
  setRGB(PIN_COMEDOR_R, PIN_COMEDOR_G, PIN_COMEDOR_B, 1, 1, 0);  // Amarillo
  setRGB(PIN_COCINA_R,  PIN_COCINA_G,  PIN_COCINA_B,  0, 1, 1);  // Cian
  setRGB(PIN_BANIO_R,   PIN_BANIO_G,   PIN_BANIO_B,   0, 0, 1);  // Azul
  setRGB(PIN_HABIT_R,   PIN_HABIT_G,   PIN_HABIT_B,   0, 1, 0);  // Verde
}

void apagarTodosRGB() {
  setRGB(PIN_SALA_R,    PIN_SALA_G,    PIN_SALA_B,    0, 0, 0);
  setRGB(PIN_COMEDOR_R, PIN_COMEDOR_G, PIN_COMEDOR_B, 0, 0, 0);
  setRGB(PIN_COCINA_R,  PIN_COCINA_G,  PIN_COCINA_B,  0, 0, 0);
  setRGB(PIN_BANIO_R,   PIN_BANIO_G,   PIN_BANIO_B,   0, 0, 0);
  setRGB(PIN_HABIT_R,   PIN_HABIT_G,   PIN_HABIT_B,   0, 0, 0);
}

// ─────────────────────────────────────────────
//  ALTERNANCIA MODO FIESTA
//  Grupo A: Sala(rojo) + Cocina(verde)
//  Grupo B: Comedor(azul) + Baño(magenta) + Habitación(amarillo)
// ─────────────────────────────────────────────
void alternarFiesta() {
  alterEstado = !alterEstado;
  if (alterEstado) {
    // Grupo A encendido, Grupo B apagado
    setRGB(PIN_SALA_R,    PIN_SALA_G,    PIN_SALA_B,    1, 0, 0);  // Rojo
    setRGB(PIN_COCINA_R,  PIN_COCINA_G,  PIN_COCINA_B,  0, 1, 0);  // Verde
    setRGB(PIN_COMEDOR_R, PIN_COMEDOR_G, PIN_COMEDOR_B, 0, 0, 0);
    setRGB(PIN_BANIO_R,   PIN_BANIO_G,   PIN_BANIO_B,   0, 0, 0);
    setRGB(PIN_HABIT_R,   PIN_HABIT_G,   PIN_HABIT_B,   0, 0, 0);
  } else {
    // Grupo B encendido, Grupo A apagado
    setRGB(PIN_SALA_R,    PIN_SALA_G,    PIN_SALA_B,    0, 0, 0);
    setRGB(PIN_COCINA_R,  PIN_COCINA_G,  PIN_COCINA_B,  0, 0, 0);
    setRGB(PIN_COMEDOR_R, PIN_COMEDOR_G, PIN_COMEDOR_B, 0, 0, 1);  // Azul
    setRGB(PIN_BANIO_R,   PIN_BANIO_G,   PIN_BANIO_B,   1, 0, 1);  // Magenta
    setRGB(PIN_HABIT_R,   PIN_HABIT_G,   PIN_HABIT_B,   1, 1, 0);  // Amarillo
  }
}

// ─────────────────────────────────────────────
//  HELPER: set color de un LED RGB
// ─────────────────────────────────────────────
void setRGB(int pR, int pG, int pB, int r, int g, int b) {
  digitalWrite(pR, r ? HIGH : LOW);
  digitalWrite(pG, g ? HIGH : LOW);
  digitalWrite(pB, b ? HIGH : LOW);
}

// ─────────────────────────────────────────────
//  PROCESAR COMANDO BLUETOOTH
// ─────────────────────────────────────────────
void procesarComandoBT(String cmd) {
  cmd.trim();
  cmd.toLowerCase();

  Serial.print(F("CMD BT: "));
  Serial.println(cmd);

  if (cmd == "modo_fiesta") {
    aplicarModo(MODO_FIESTA);
  }
  else if (cmd == "modo_relajado") {
    aplicarModo(MODO_RELAJADO);
  }
  else if (cmd == "modo_noche") {
    aplicarModo(MODO_NOCHE);
  }
  else if (cmd == "encender_todo") {
    aplicarModo(MODO_ENCENDER_TODO);
  }
  else if (cmd == "apagar_todo") {
    aplicarModo(MODO_APAGAR_TODO);
  }
  else if (cmd == "estado") {
    // Responder con estado actual
    String resp = "Estado: ";
    switch (modoActual) {
      case MODO_FIESTA:        resp += "FIESTA";        break;
      case MODO_RELAJADO:      resp += "RELAJADO";      break;
      case MODO_NOCHE:         resp += "NOCHE";         break;
      case MODO_ENCENDER_TODO: resp += "ENCENDER_TODO"; break;
      case MODO_APAGAR_TODO:   resp += "APAGAR_TODO";   break;
      case MODO_INDIVIDUAL:    resp += "INDIVIDUAL";    break;
      default:                 resp += "NINGUNO";       break;
    }
    resp += " | Puerta: ";
    resp += puertaAbierta ? "ABIERTA" : "CERRADA";
    resp += " | Fan: ";
    resp += fanOn ? "ON" : "OFF";
    resp += "\nLEDs: Sala="; resp += ledSalaOn    ? "ON" : "OFF";
    resp += " Comedor=";     resp += ledComedorOn ? "ON" : "OFF";
    resp += " Cocina=";      resp += ledCocinaOn  ? "ON" : "OFF";
    resp += " Banio=";       resp += ledBanioOn   ? "ON" : "OFF";
    resp += " Habit=";       resp += ledHabitOn   ? "ON" : "OFF";
    Serial1.println(resp);
    Serial.println(resp);
    lcdMostrar("Estado actual:  ", modoActual == MODO_FIESTA ? "FIESTA          " :
               modoActual == MODO_RELAJADO ? "RELAJADO        " :
               modoActual == MODO_NOCHE    ? "NOCHE           " :
               modoActual == MODO_ENCENDER_TODO ? "TODO ON         " :
               modoActual == MODO_INDIVIDUAL    ? "INDIVIDUAL      " : "NINGUNO         ");
  }
  // ── Intentar control individual de ambiente ──
  else if (procesarControlIndividual(cmd)) {
    // ya fue procesado dentro de la función
  }
  else {
    // Comando inválido
    lcdMostrar("ERROR:          ", "Modo invalido   ");
    Serial1.println(F("ERROR: Modo invalido"));
    Serial.println(F("ERROR: Comando no reconocido"));
    digitalWrite(PIN_LED_ROJO, HIGH);
    errorSistema = true;
  }
}

// ─────────────────────────────────────────────
//  CONTROL INDIVIDUAL DE AMBIENTES
//  Formato del comando: "nombre:on" o "nombre:off"
//  Retorna true si el comando fue reconocido y ejecutado.
//  REGLA ESPECIAL: habitacion:on/off sincroniza el ventilador.
// ─────────────────────────────────────────────
bool procesarControlIndividual(String cmd) {
  int sep = cmd.indexOf(':');
  if (sep < 0) return false;

  String nombre = cmd.substring(0, sep);
  String accion = cmd.substring(sep + 1);
  nombre.trim();
  accion.trim();

  if (accion != "on" && accion != "off") return false;
  bool encender = (accion == "on");

  // ── Ventilador independiente ──
  if (nombre == "ventilador") {
    fanOn = encender;
    digitalWrite(PIN_MOTOR, fanOn ? HIGH : LOW);
    lcdMostrar(encender ? "Ventilador: ON  " : "Ventilador: OFF ",
               "Ctrl individual ");
    String r = "OK: ventilador:"; r += accion;
    Serial1.println(r);
    Serial.println(r);
    modoActual = MODO_INDIVIDUAL;
    guardarEstadosIndividuales();
    return true;
  }

  // ── Ambientes ──
  encenderAmbiente(nombre, encender);

  // Si el nombre no fue reconocido, la función devuelve false internamente.
  // Aquí detectamos eso verificando que el nombre sea válido:
  if (nombre != "sala"       && nombre != "comedor" &&
      nombre != "cocina"     && nombre != "banio"   &&
      nombre != "bano"       &&
      nombre != "habitacion" && nombre != "habitacion") {
    return false;
  }

  modoActual = MODO_INDIVIDUAL;
  guardarEstadosIndividuales();
  return true;
}

// ─────────────────────────────────────────────
//  ENCIENDE / APAGA UN AMBIENTE INDIVIDUAL
//  REGLA ESPECIAL: habitacion → sincroniza ventilador
// ─────────────────────────────────────────────
void encenderAmbiente(const String& nombre, bool encender) {
  char lcdL0[17];
  char lcdL1[17] = "Ctrl individual ";

  if (nombre == "sala") {
    ledSalaOn = encender;
    setRGB(PIN_SALA_R, PIN_SALA_G, PIN_SALA_B,
           encender ? COLOR_SALA_R : 0,
           encender ? COLOR_SALA_G : 0,
           encender ? COLOR_SALA_B : 0);
    snprintf(lcdL0, sizeof(lcdL0), "Sala: %-10s", encender ? "ON" : "OFF");
    String r = "OK: sala:"; r += encender ? "on" : "off";
    Serial1.println(r); Serial.println(r);

  } else if (nombre == "comedor") {
    ledComedorOn = encender;
    setRGB(PIN_COMEDOR_R, PIN_COMEDOR_G, PIN_COMEDOR_B,
           encender ? COLOR_COM_R : 0,
           encender ? COLOR_COM_G : 0,
           encender ? COLOR_COM_B : 0);
    snprintf(lcdL0, sizeof(lcdL0), "Comedor: %-7s", encender ? "ON" : "OFF");
    String r = "OK: comedor:"; r += encender ? "on" : "off";
    Serial1.println(r); Serial.println(r);

  } else if (nombre == "cocina") {
    ledCocinaOn = encender;
    setRGB(PIN_COCINA_R, PIN_COCINA_G, PIN_COCINA_B,
           encender ? COLOR_COC_R : 0,
           encender ? COLOR_COC_G : 0,
           encender ? COLOR_COC_B : 0);
    snprintf(lcdL0, sizeof(lcdL0), "Cocina: %-8s", encender ? "ON" : "OFF");
    String r = "OK: cocina:"; r += encender ? "on" : "off";
    Serial1.println(r); Serial.println(r);

  } else if (nombre == "banio" || nombre == "bano") {
    ledBanioOn = encender;
    setRGB(PIN_BANIO_R, PIN_BANIO_G, PIN_BANIO_B,
           encender ? COLOR_BAN_R : 0,
           encender ? COLOR_BAN_G : 0,
           encender ? COLOR_BAN_B : 0);
    snprintf(lcdL0, sizeof(lcdL0), "Banio: %-9s", encender ? "ON" : "OFF");
    String r = "OK: banio:"; r += encender ? "on" : "off";
    Serial1.println(r); Serial.println(r);

  } else if (nombre == "habitacion") {
    ledHabitOn = encender;
    setRGB(PIN_HABIT_R, PIN_HABIT_G, PIN_HABIT_B,
           encender ? COLOR_HAB_R : 0,
           encender ? COLOR_HAB_G : 0,
           encender ? COLOR_HAB_B : 0);

    // ── REGLA ESPECIAL: ventilador sincronizado con habitación ──
    fanOn = encender;
    digitalWrite(PIN_MOTOR, fanOn ? HIGH : LOW);

    if (encender) {
      snprintf(lcdL0, sizeof(lcdL0), "Habit+Fan: ON   ");
      Serial1.println(F("OK: habitacion:on | Fan:ON (auto)"));
      Serial.println(F("OK: habitacion:on -> ventilador ON automatico"));
    } else {
      snprintf(lcdL0, sizeof(lcdL0), "Habit+Fan: OFF  ");
      Serial1.println(F("OK: habitacion:off | Fan:OFF (auto)"));
      Serial.println(F("OK: habitacion:off -> ventilador OFF automatico"));
    }

  } else {
    // Nombre no reconocido — no modificar LCD
    return;
  }

  lcdMostrar(lcdL0, lcdL1);
}

// ─────────────────────────────────────────────
//  EEPROM — GUARDAR / LEER ESTADOS INDIVIDUALES
// ─────────────────────────────────────────────
void guardarEstadosIndividuales() {
  EEPROM.write(EEPROM_VALIDACION,   EEPROM_VALID_BYTE);
  EEPROM.write(EEPROM_LED_SALA,     ledSalaOn    ? 1 : 0);
  EEPROM.write(EEPROM_LED_COMEDOR,  ledComedorOn ? 1 : 0);
  EEPROM.write(EEPROM_LED_COCINA,   ledCocinaOn  ? 1 : 0);
  EEPROM.write(EEPROM_LED_BANIO,    ledBanioOn   ? 1 : 0);
  EEPROM.write(EEPROM_LED_HABIT,    ledHabitOn   ? 1 : 0);
  EEPROM.write(EEPROM_FAN_ESTADO,   fanOn        ? 1 : 0);
}

void leerEstadosIndividuales() {
  if (EEPROM.read(EEPROM_VALIDACION) != EEPROM_VALID_BYTE) return;
  ledSalaOn    = EEPROM.read(EEPROM_LED_SALA)    == 1;
  ledComedorOn = EEPROM.read(EEPROM_LED_COMEDOR) == 1;
  ledCocinaOn  = EEPROM.read(EEPROM_LED_COCINA)  == 1;
  ledBanioOn   = EEPROM.read(EEPROM_LED_BANIO)   == 1;
  ledHabitOn   = EEPROM.read(EEPROM_LED_HABIT)   == 1;
  fanOn        = EEPROM.read(EEPROM_FAN_ESTADO)  == 1;
}

// ─────────────────────────────────────────────
//  PROCESADOR DE ARCHIVO .ORG (línea por línea)
// ─────────────────────────────────────────────
void procesarLineaOrg(String linea) {
  linea.trim();

  // Ignorar líneas vacías
  if (linea.length() == 0) return;

  // Eliminar comentarios inline: todo lo que sigue a "//"
  int idxComentario = linea.indexOf("//");
  if (idxComentario == 0) return;            // Línea solo comentario
  if (idxComentario > 0) {
    linea = linea.substring(0, idxComentario);
    linea.trim();
    if (linea.length() == 0) return;
  }

  linea.toLowerCase();

  // ── conf_ini: inicio del archivo ──
  if (linea == "conf_ini") {
    if (cargandoOrg) {
      // Ya estábamos cargando, reiniciamos
    }
    cargandoOrg  = true;
    orgIniciado  = true;
    modoOrgActual = MODO_NINGUNO;
    errorSistema = false;
    digitalWrite(PIN_LED_ROJO,  LOW);
    digitalWrite(PIN_LED_VERDE, LOW);
    lcdMostrar("Cargando .org...", "");
    Serial.println(F("ORG: conf_ini detectado"));
    return;
  }

  // ── conf:fin: fin del archivo ──
  if (linea == "conf:fin") {
    if (!orgIniciado) {
      finalizarCargaOrg(false);
      return;
    }
    finalizarCargaOrg(true);
    return;
  }

  // Si no se recibió conf_ini aún, ignorar
  if (!cargandoOrg) return;

  // ── Declaración de modo ──
  if (linea == "modo_fiesta") {
    modoOrgActual = MODO_FIESTA;
    return;
  }
  if (linea == "modo_relajado") {
    modoOrgActual = MODO_RELAJADO;
    return;
  }
  if (linea == "modo_noche") {
    modoOrgActual = MODO_NOCHE;
    return;
  }
  if (linea == "encender_todo") {
    modoOrgActual = MODO_ENCENDER_TODO;
    return;
  }
  if (linea == "apagar_todo") {
    modoOrgActual = MODO_APAGAR_TODO;
    return;
  }

  // ── Parámetro ventilador ──
  if (linea.startsWith("ventilador:")) {
    if (modoOrgActual == MODO_NINGUNO) {
      finalizarCargaOrg(false);  // ventilador sin modo definido = error
      return;
    }
    String valor = linea.substring(11);
    valor.trim();
    if (valor == "on") {
      fanPorModo[modoOrgActual] = true;
    } else if (valor == "off") {
      fanPorModo[modoOrgActual] = false;
    } else {
      // Valor inválido
      finalizarCargaOrg(false);
      return;
    }
    Serial.print(F("ORG: Fan modo "));
    Serial.print(modoOrgActual);
    Serial.println(valor == "on" ? F(" = ON") : F(" = OFF"));
    return;
  }

  // ── Parámetros que se aceptan pero no requieren acción extra ──
  // (Mensaje en LCD, LED'S) — el sistema los ignora o los maneja implícitamente
  if (linea.startsWith("mensaje en lcd:")) return;
  if (linea.startsWith("led's:"))          return;

  // ── Línea desconocida = error de sintaxis ──
  Serial.print(F("ORG ERROR linea desconocida: "));
  Serial.println(linea);
  finalizarCargaOrg(false);
}

// ─────────────────────────────────────────────
//  FINALIZAR CARGA .ORG
// ─────────────────────────────────────────────
void finalizarCargaOrg(bool exito) {
  cargandoOrg  = false;
  orgIniciado  = false;
  modoOrgActual = MODO_NINGUNO;
  lineaOrg     = "";

  if (exito) {
    guardarFanEEPROM();
    EEPROM.write(EEPROM_ORG_FLAG, EEPROM_ORG_BYTE);
    lcdMostrar("Configuracion   ", "guardada OK     ");
    Serial.println(F("ORG: Carga exitosa. Configuracion guardada en EEPROM."));
    iniciarParpadeVerde();
  } else {
    lcdMostrar("Error en        ", "archivo .org    ");
    Serial.println(F("ORG ERROR: Sintaxis invalida o conf_ini/conf:fin faltante."));
    digitalWrite(PIN_LED_ROJO, HIGH);
    errorSistema = true;
  }
}

// ─────────────────────────────────────────────
//  EEPROM — GUARDAR / LEER MODO ACTUAL
// ─────────────────────────────────────────────
void guardarModoEEPROM(int modo) {
  // Validar rango
  if (modo < 0 || modo > 5) return;
  EEPROM.write(EEPROM_VALIDACION, EEPROM_VALID_BYTE);
  EEPROM.write(EEPROM_MODO_ACTUAL, (uint8_t)modo);
}

int leerModoEEPROM() {
  uint8_t validacion = EEPROM.read(EEPROM_VALIDACION);
  if (validacion != EEPROM_VALID_BYTE) return MODO_NINGUNO;
  uint8_t modo = EEPROM.read(EEPROM_MODO_ACTUAL);
  if (modo < 1 || modo > 5) return MODO_NINGUNO;
  return (int)modo;
}

// ─────────────────────────────────────────────
//  EEPROM — GUARDAR / LEER CONFIG VENTILADOR
// ─────────────────────────────────────────────
void guardarFanEEPROM() {
  EEPROM.write(EEPROM_FAN_FIESTA,   fanPorModo[MODO_FIESTA]        ? 1 : 0);
  EEPROM.write(EEPROM_FAN_RELAJADO, fanPorModo[MODO_RELAJADO]      ? 1 : 0);
  EEPROM.write(EEPROM_FAN_NOCHE,    fanPorModo[MODO_NOCHE]         ? 1 : 0);
  EEPROM.write(EEPROM_FAN_TODO_ON,  fanPorModo[MODO_ENCENDER_TODO] ? 1 : 0);
  EEPROM.write(EEPROM_FAN_TODO_OFF, fanPorModo[MODO_APAGAR_TODO]   ? 1 : 0);
}

void leerFanEEPROM() {
  uint8_t validacion = EEPROM.read(EEPROM_VALIDACION);
  if (validacion != EEPROM_VALID_BYTE) {
    // Valores por defecto del enunciado
    fanPorModo[MODO_FIESTA]        = true;
    fanPorModo[MODO_RELAJADO]      = false;
    fanPorModo[MODO_NOCHE]         = false;
    fanPorModo[MODO_ENCENDER_TODO] = true;
    fanPorModo[MODO_APAGAR_TODO]   = false;
    return;
  }
  fanPorModo[MODO_FIESTA]        = EEPROM.read(EEPROM_FAN_FIESTA)   == 1;
  fanPorModo[MODO_RELAJADO]      = EEPROM.read(EEPROM_FAN_RELAJADO) == 1;
  fanPorModo[MODO_NOCHE]         = EEPROM.read(EEPROM_FAN_NOCHE)    == 1;
  fanPorModo[MODO_ENCENDER_TODO] = EEPROM.read(EEPROM_FAN_TODO_ON)  == 1;
  fanPorModo[MODO_APAGAR_TODO]   = EEPROM.read(EEPROM_FAN_TODO_OFF) == 1;
}

// ─────────────────────────────────────────────
//  LCD — HELPER
// ─────────────────────────────────────────────
void lcdMostrar(const char* fila0, const char* fila1) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(fila0);
  lcd.setCursor(0, 1);
  lcd.print(fila1);
}

// ─────────────────────────────────────────────
//  LED VERDE — PARPADEO 3 VECES (sin blocking)
// ─────────────────────────────────────────────
void iniciarParpadeVerde() {
  parpadeandoVerde    = true;
  parpadeoCount       = 0;
  tiempoParpadeoVerde = millis();
  digitalWrite(PIN_LED_VERDE, HIGH);
}

void tickParpadeVerde() {
  if (!parpadeandoVerde) return;
  uint32_t ahora = millis();
  if (ahora - tiempoParpadeoVerde >= INTERVALO_PARPV_MS) {
    tiempoParpadeoVerde = ahora;
    parpadeoCount++;
    bool estado = (parpadeoCount % 2 == 0);  // par=OFF, impar=ON
    digitalWrite(PIN_LED_VERDE, estado ? HIGH : LOW);
    // 3 parpadeos completos = 6 cambios (ON-OFF x3)
    if (parpadeoCount >= 6) {
      parpadeandoVerde = false;
      digitalWrite(PIN_LED_VERDE, LOW);
    }
  }
}
