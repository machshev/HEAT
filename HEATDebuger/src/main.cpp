#include <Arduino.h>

#include <CAN.h>
#include <SPI.h>
#include <math.h>

const long mcp2515_spi_freq = 10E6;
const long mcp2515_can_freq = 500E3;
const long mcp2515_clock_freq = 16E6;

uint8_t rxbuff[8] = {0};

void onReceive(int packetSize) {
  Serial.print(CAN.packetId(), HEX);

  while (CAN.available()) {
    Serial.print(CAN.read(), HEX);
  }

  Serial.println("");
}

void setup() {
  Serial.begin(115200);

  CAN.setSPIFrequency(mcp2515_spi_freq);
  CAN.setClockFrequency(mcp2515_clock_freq);

  while (!CAN.begin(mcp2515_can_freq)) {
    Serial.println("Starting CAN failed!");

    delay(500);
  }

  // register the receive callback
  CAN.onReceive(onReceive);

  Serial.println("CAN Initialised");
}

void loop() {
}
