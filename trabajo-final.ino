#include <LiquidCrystal.h>
#include <LiquidMenu.h>
#include <Keypad.h>
#include "DHT.h"
#include <stdio.h>
#include "AsyncTaskLib.h"
#include "DHTStable.h"
#include <Adafruit_Sensor.h>

#include <EEPROM.h>

/*
Sistema  administrador de temperatura

Con el sistema de administracion de temperatura es posible
gestionar una temperatura maximoa y minima con el fin de
generar una alerta si la temperatura del sensor sobrepasa
dichos limites.
*/

// Variables a modificar
int THTempHigh = 29;
int THTemLow = 26;
int THLuzHigh = 500;
int THLuzLow = 100;

int nuevoValor = 0;

const int rs = 23, en = 33, d4 = 25, d5 = 27, d6 = 29, d7 = 31;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

unsigned short analogReading = 0;
unsigned short lastAnalogReading = 0;

unsigned int period_check = 1000;
unsigned long lastMs_check = 0;

unsigned int period_nextScreen = 5000;
unsigned long lastMs_nextScreen = 0;
unsigned long lastMs_previousScreen = 0;

int posicion = 0;

/*
MENU principal

1. THTempHigh
2. THTemLow
3. THLuzHihg
4. THLuzHiLow
5. Reset
*/

LiquidLine welcome_line1(0, 0, "Menu");
LiquidLine welcome_line2(0, 1, "Arrba(A) Abjo(B)");
LiquidScreen screen(welcome_line1, welcome_line2);

LiquidLine menu_linea1(0, 0, "1. THTempHigh");
LiquidLine menu_linea2(0, 1, "2. THTemLow");
LiquidScreen screen2(menu_linea1, menu_linea2);

LiquidLine menu_linea3(0, 0, "3. THLuzHihg");
LiquidLine menu_linea4(0, 1, "4. THLuzHiLow");
LiquidScreen screen3(menu_linea3, menu_linea4);

LiquidLine menu_linea5(0, 0, "4. THLuzHiLow");
LiquidLine menu_linea6(0, 1, "5. Reset");
LiquidScreen screen4(menu_linea5, menu_linea6);

// LiquidLine analogReading_line(0, 0, "Analog: ", analogReading);
// LiquidScreen secondary_screen(analogReading_line);

/*
MENU secundario

int THTempHigh = 29;
int THTemLow = 26;
int THLuzHigh = 500;
int THLuzLow = 100;
*/

LiquidLine submenu_linea1(0, 0, "THTempHigh");
LiquidLine submenu_linea2(0, 1, THTempHigh);
LiquidScreen subscreen(submenu_linea1, submenu_linea2);

LiquidLine submenu_linea3(0, 0, "THTemLow");
LiquidLine submenu_linea4(0, 1, THTemLow);
LiquidScreen subscreen2(submenu_linea3, submenu_linea4);

LiquidLine submenu_linea5(0, 0, "THLuzHigh");
LiquidLine submenu_linea6(0, 1, THLuzHigh);
LiquidScreen subscreen3(submenu_linea5, submenu_linea6);

LiquidLine submenu_linea7(0, 0, "THLuzLow");
LiquidLine submenu_linea8(0, 1, THLuzLow);
LiquidScreen subscreen4(submenu_linea7, submenu_linea8);

LiquidMenu menu(lcd);

// 4 filas
const byte ROWS = 4;

// 4 columnas
const byte COLS = 4;

// Define los simbolos de los botones del teclado
char hexaKeys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'N' },
  { 'N', '0', 'N', 'D' }
};

// Conecta las filas del teclado
byte rowPins[ROWS] = { 3, 2, 1, 0 };

// Conecta las columans del teclado
byte colPins[COLS] = { 7, 6, 5, 4 };

// Inicializa la instancia de la clase NewKeypad
Keypad teclado = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);


DHTStable DHT;

#define DHT11_PIN 10

// Numero del pushbutton pin
#define buttonPin = 2;

// Variable para leer el estado del pushbutton
int buttonState = 0;

int duracion = 500;
int retardo = 1000;

#define LED_RED 14
#define LED_GREEN 15
#define LED_BLUE 16
#define BUZZER 35
#define Do 261
#define Re 293
#define Mi 329
#define Fa 349
#define Sol 392
#define La 440
#define Si 493

// Contraseña a verificar
String Password = "12345";

// booleano por
bool PasswordCorrecto = false;

// Booleano que activa el sonido del buzzer
bool bloqueado = false;

byte sonidoTocado = 0;
int melodia[] = {
  Do,
  Do,
  Fa,
  Mi,
  Si,
  Do,
  Do,
  La,
  Sol,
  Re,
};

// array con la duracion de cada nota
int duraciones[] = {
  16,
  16,
  8,
  6,
  32,
  8,
  8,
  8,
  16,
  16,
};

bool contraseniaCorrecta = false;

AsyncTask asyncTask1(1000);

int temperature = 0;

/*
Funcion inicializadora
*/
void setup() {
  lcd.begin(16, 2);
  Serial.begin(9600);

  asyncTask1.OnFinish = obtenerTemperatura;
  asyncTask1.Start();

  // Agregar menu principal
  menu.add_screen(screen);
  menu.add_screen(screen2);
  menu.add_screen(screen3);
  menu.add_screen(screen4);

  // Agregar menu secundario
  menu.add_screen(subscreen);
  menu.add_screen(subscreen2);
  menu.add_screen(subscreen3);
  menu.add_screen(subscreen4);
}

/*
Funcion que obtiene el nuevo valor por teclado
iterando para procesar los valores por teclado
concatenandolos en un unico valor

Retorna: entero
valor concatenado
*/
int obtenerNuevoValorTecleado() {
  int nuevoValor = 0;
  bool salir = false;
  int numeroTeclasPresionadas = 0;
  int i = 0;
  do {
    char tecla_presionada = teclado.waitForKey();
    if (tecla_presionada >= '0' && tecla_presionada <= '9') {
      Serial.println(tecla_presionada);
      lcd.setCursor(i, 1);
      lcd.println(tecla_presionada);
      nuevoValor *= 10;
      nuevoValor += tecla_presionada - '0';
      numeroTeclasPresionadas++;
      i++;
    }
    if (tecla_presionada == 'D') {
      salir = true;
    }
  } while (salir != true && numeroTeclasPresionadas < 4);
  return nuevoValor;
}

/*
Funcion que obbtiene la contrasena por teclado
esperando a que el usuario ingrese los 5 valores
para validar con la contrasena correcta

Retorno: bool
true si la contrasena es correcta
false si la contrasena es incorrecta
*/
bool obtenerContraseniaTecleado() {
  int intentos = 0;
  int numeroTeclasPresionadas = 0;
  String contrasenia = "";
  int i = 0;
  do {
    lcd.setCursor(0, 0);
    lcd.println("Contrasenia");
    numeroTeclasPresionadas = 0;
    contrasenia = "";
    i = 0;
    do {
      char tecla_presionada = teclado.waitForKey();
      lcd.setCursor(i, 1);
      lcd.println('*');
      Serial.println(tecla_presionada);
      contrasenia = contrasenia + tecla_presionada;
      numeroTeclasPresionadas++;
      i++;
    } while (numeroTeclasPresionadas < 5);
    lcd.clear();
    lcd.setCursor(0, 1);
    i = 0;
    if (contrasenia == Password) {
      return true;
    } else {
      lcd.println("Incorrecta");
      color(255, 0, 0);
      delay(2000);
      lcd.clear();
    }
    intentos++;
  } while (intentos < 3);
  bloqueado = true;
  return false;
}

/*
Funcion para generar los colores

Retorna: no
*/
void color(unsigned char red, unsigned char green, unsigned char blue) {
  analogWrite(LED_RED, red);
  analogWrite(LED_BLUE, blue);
  analogWrite(LED_GREEN, green);
}

/*
Funcion para verificar en que rango esta la temperatura
y encender el color del led y el buzzer

Retorna: no
*/
void medirTemperatura() {
  asyncTask1.Update();

  if (temperature > THTempHigh) {
    color(255, 0, 0);
    buzzerTone();
  } else if (temperature < THTemLow) {
    color(0, 0, 255);
  } else {
    color(0, 255, 0);
  }
}

/*
Funcion para leer los valores almacenados
en la memoria EEPROM al inicio del programa

Retorna: no
*/
void LeerEEPROM() {
  if (EEPROM.read(0) == 255 && EEPROM.read(1) == 255 && EEPROM.read(2) == 255 && EEPROM.read(3) == 255) {
    EEPROM.update(0, THTempHigh);
    EEPROM.update(1, THTemLow);
    EEPROM.update(2, THLuzHigh);
    EEPROM.update(3, THLuzLow);
  } else {
    THTempHigh = EEPROM.read(0);
    THTemLow = EEPROM.read(1);
    THLuzHigh = EEPROM.read(2);
    THLuzLow = EEPROM.read(3);
  }
}

/*
Funcion que controla el menu a mostrar
por la pantalla lcd de acuerdo a la tecla
presionada

Retorna: no
*/
void iniciarMenu() {
  if (posicion == 0) {
    menu.change_screen(&screen);
    LeerEEPROM();
  }
  char tecla_presionada = teclado.waitForKey();

  if (tecla_presionada == 'A') {
    if (posicion > 0 && posicion < 4) {
      lastMs_nextScreen = millis();
      menu.previous_screen();
      posicion--;
    }
  }

  if (tecla_presionada == 'B') {
    if (posicion < 3) {
      lastMs_previousScreen = millis();
      menu.next_screen();
      posicion++;
    }
  }

  // Menu secundario
  if (tecla_presionada == '1') {
    lastMs_previousScreen = millis();
    menu.change_screen(&subscreen);
    THTempHigh = obtenerNuevoValorTecleado();
    EEPROM.update(0, THTempHigh);
    posicion = 0;
  }

  if (tecla_presionada == '2') {
    lastMs_previousScreen = millis();
    menu.change_screen(&subscreen2);
    THTemLow = obtenerNuevoValorTecleado();
    EEPROM.update(1, THTemLow);
    posicion = 0;
  }

  if (tecla_presionada == '3') {
    lastMs_previousScreen = millis();
    menu.change_screen(&subscreen3);
    THLuzHigh = obtenerNuevoValorTecleado();
    EEPROM.update(2, THLuzHigh);
    posicion = 0;
  }

  if (tecla_presionada == '4') {
    lastMs_previousScreen = millis();
    menu.change_screen(&subscreen4);
    THLuzLow = obtenerNuevoValorTecleado();
    EEPROM.update(3, THLuzLow);
    posicion = 0;
  }

  if (tecla_presionada == '5') {
    lastMs_previousScreen = millis();
    menu.change_screen(&screen);
    EEPROM.update(0, 29);
    EEPROM.update(1, 26);
    EEPROM.update(2, 500);
    EEPROM.update(3, 100);
    posicion = 0;
  }

  if (tecla_presionada == '6') {
    lcd.clear();
    medirTemperatura();
    posicion = 6;
  }
}

/*
Funcion que obtiene el valor de la temperatura
actual e imprime por pantalla lcd su valor

Retorna: no
*/
void obtenerTemperatura() {
  asyncTask1.Start();
  temperature = DHT.getTemperature();
  lcd.setCursor(0, 1);
  lcd.print("Humedad : " + temperature);
}

/*
Funcion que ejecuta el sonido definido
en el buzzer

Retorna: no
*/
void buzzerTone() {
  for (int i = 0; i < 9; i++) {
    int duracion = 1500 / duraciones[i];
    tone(BUZZER, melodia[i], duracion);
    int pausa = duracion * 1.30;
    delay(pausa);
  }
  noTone(BUZZER);
}

/*
Funcion que verifica el valor obtenido por la funcion
obtenerContraseniaTecleado para mostar por pantalla
lcd el texto "bloqueado" e iniciar el sonido del buzzer

Retorna: no
*/
void verificarContrasenia() {
  if (sonidoTocado == 0) {
    PasswordCorrecto = obtenerContraseniaTecleado();
    if (bloqueado) {
      lcd.clear();
      lcd.println("Bloqueado");
      buzzerTone();
    }
    sonidoTocado = 1;
  }
}

/*
Funcion ciclica que permite detectar cambios y responder a ellos

Retorna: no
*/
void loop() {
  int chk = DHT.read11(DHT11_PIN);

  if (PasswordCorrecto) {
    iniciarMenu();
  } else {
    verificarContrasenia();
  }
}
