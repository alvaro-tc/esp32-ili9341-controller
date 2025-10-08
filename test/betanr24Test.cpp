#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define NRF24_CE 6
#define NRF24_SCK 36
#define NRF24_MISO 37
#define NRF24_CSN 7
#define NRF24_MOSI 35

// Inicializar RF24 con CE y CSN (usaremos SPI por defecto)
RF24 radio(NRF24_CE, NRF24_CSN);

const uint64_t my_radio_pipe = 0xE8E8F0F0E1LL;

struct Data_to_be_sent {
  byte ch1;
};

Data_to_be_sent sent_data;

void setup() {
  Serial.begin(9600);

  // Inicializar SPI con pines personalizados para ESP32-S2
  SPI.begin(NRF24_SCK, NRF24_MISO, NRF24_MOSI, NRF24_CSN);

  // Inicializar radio
  if (!radio.begin()) {
    Serial.println("Error al inicializar NRF24");
    while (1);
  }

  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_LOW);
  radio.openWritingPipe(my_radio_pipe);

  sent_data.ch1 = 127;
}

void loop() {
  // Leer ADC ESP32-S2 (12-bit) - usar pin analógico válido
  sent_data.ch1 = map(analogRead(8), 0, 4095, 0, 255);
    Serial.print("Enviando ch1: "); Serial.println(sent_data.ch1);
  // Enviar
  if (!radio.write(&sent_data, sizeof(Data_to_be_sent))) {
    Serial.println("Error al enviar");
  }

  delay(10);
}
