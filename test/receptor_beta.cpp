#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <BTS7960.h>
#include <Servo.h>  // Biblioteca para el control del servomotor

#define L_EN 8
#define R_EN 7
#define L_PWM 5                             //pin 5 supports 980hz pwm frequency
#define R_PWM 6    

// Pines del servomotor
BTS7960 motor1(L_EN, R_EN, L_PWM, R_PWM); 
#define SERVO_PIN 3 

const uint64_t pipeIn = 0xE8E8F0F0E5LL; // Dirección de comunicación NRF24
RF24 radio(9, 10);                     // Pines CSN y CE

Servo servo;                           // Declaración del servomotor

// Estructura de datos recibidos (solo 4 canales ahora)
struct Received_data {
  byte ch1;  // Velocidad adelante (0-255)
  byte ch2;  // Velocidad atrás (0-255)
  byte ch3;  // Giro derecha (0-255)
  byte ch4;  // Giro izquierda (0-255)
  byte ch5;  // (no usado)
  byte ch6;  // (no usado)
  byte ch7;  // (no usado)
};

Received_data received_data;

// Variables para el control
int velocidadFinal = 0;
int direccionFinal = 90;  // Ángulo inicial del servomotor (posición neutra)
int topeVelocidad = 255;
int topeGiroSup = 120;  // Límite superior del servo (derecha)
int topeGiroInf = 60;   // Límite inferior del servo (izquierda)

void reset_the_Data() {
  // Valores predeterminados al perder señal
  received_data.ch1 = 0;   // Sin velocidad adelante
  received_data.ch2 = 0;   // Sin velocidad atrás
  received_data.ch3 = 0;   // Sin giro derecha
  received_data.ch4 = 0;   // Sin giro izquierda
}

void setup() {
  Serial.begin(9600);
  reset_the_Data();
  
  // Configuración del NRF24L01
  Serial.println();
  Serial.println(F("LGT RF_NANO v2.0 Test"));

  radio.begin();
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.openReadingPipe(1, pipeIn);
  radio.startListening();

  // Inicialización del servomotor
  servo.attach(SERVO_PIN);
  servo.write(direccionFinal); // Coloca el servomotor en la posición inicial
  motor1.begin();
  motor1.enable();
}

unsigned long lastRecvTime = 0;

void receive_the_data() {
  while (radio.available()) {
    radio.read(&received_data, sizeof(Received_data));
    lastRecvTime = millis();
  }
}

void loop() {
  // Recibir datos por radiofrecuencia
  receive_the_data();

  // Verificar si se perdió la señal
  unsigned long now = millis();
  if (now - lastRecvTime > 1000) {
    reset_the_Data();
  }

  // ========== CONTROL DE MOTOR ==========
  // ch1 = velocidad adelante (0-255)
  // ch2 = velocidad atrás (0-255)
  
  if (received_data.ch1 > 0 && received_data.ch2 == 0) {
    // Avanzar hacia adelante
    velocidadFinal = received_data.ch1;  // Usar directamente el valor recibido
    motor1.pwm = velocidadFinal;
    motor1.front();
  }
  else if (received_data.ch2 > 0 && received_data.ch1 == 0) {
    // Retroceder
    velocidadFinal = received_data.ch2;  // Usar directamente el valor recibido
    motor1.pwm = velocidadFinal;
    motor1.back();
  }
  else {
    // Detener motor (ambos canales en 0 o ambos activos)
    velocidadFinal = 0;
    motor1.stop();
  }

  // ========== CONTROL DE DIRECCIÓN (SERVO) ==========
  // ch3 = giro derecha (0-255)
  // ch4 = giro izquierda (0-255)
  
  if (received_data.ch3 > 0) {
    // Girar a la derecha - mapeo completo de 0 a 180 grados
    direccionFinal = map(received_data.ch3, 0, 255, 71, 180);
    servo.write(direccionFinal);
  }
  else if (received_data.ch4 > 0) {
    // Girar a la izquierda - mapeo completo de 0 a 180 grados
    direccionFinal = map(received_data.ch4, 0, 255, 71, 0);
    servo.write(direccionFinal);
  }
  else {
    // Posición neutra
    direccionFinal = 71;
    servo.write(direccionFinal);
  }

  // Depuración por Serial
  Serial.print("CH1 (Adelante): ");
  Serial.print(received_data.ch1);
  Serial.print(" | CH2 (Atrás): ");
  Serial.print(received_data.ch2);
  Serial.print(" | CH3 (Der): ");
  Serial.print(received_data.ch3);
  Serial.print(" | CH4 (Izq): ");
  Serial.print(received_data.ch4);
  Serial.print(" | Vel Final: ");
  Serial.print(velocidadFinal);
  Serial.print(" | Dir Final: ");
  Serial.println(direccionFinal);

  delay(10); // Pequeño delay para estabilidad
}
