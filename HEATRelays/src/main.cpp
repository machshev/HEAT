#include <Arduino.h>

#include <CAN.h>
#include <SPI.h>
#include <math.h>

#define CAN_ID_HWTANK_TEMP     0xA1

#define RELAY_1                9
#define RELAY_2                8
#define RELAY_3                7
#define RELAY_4                6
#define RELAY_5                5
#define RELAY_6                4
#define RELAY_7                3
#define RELAY_8                0


const long mcp2515_can_freq = 500E3;
const long mcp2515_spi_freq = 10E6;
const long mcp2515_clock_freq = 16E6;

uint8_t rxbuf[8] = {0};

float hwtank_temp = 0;

struct ThermData {
  uint16_t therm1;
  uint16_t therm2;
  uint16_t therm3;
};


void packet_HWTANK_TEMP(uint8_t *msg){
  struct ThermData *data = (struct ThermData *) msg;

  float therm1 = ((float) data->therm1) / 100;
  float therm2 = ((float) data->therm2) / 100;
  float therm3 = ((float) data->therm3) / 100;

  hwtank_temp = (therm1 + therm2 + therm3) / 3;

  Serial.print(hwtank_temp);
  Serial.print(" T1[");
  Serial.print(therm1);
  Serial.print("] T2[");
  Serial.print(therm2);
  Serial.print("] T3[");
  Serial.print(therm3);
  Serial.println("] C");
}


void onReceive(int packetSize) {
  int byte_no = 0;

  Serial.print("packet with id 0x");
  Serial.println(CAN.packetId(), HEX);

  while (CAN.available()) {
    rxbuf[byte_no++] = (uint8_t) CAN.read();
  }

  switch (CAN.packetId()) {
    case CAN_ID_HWTANK_TEMP:
      packet_HWTANK_TEMP(rxbuf);
      break;
  }

}

void setup_relay(uint16_t pin) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
}

void setup_relays() {
    setup_relay(RELAY_1);
    setup_relay(RELAY_2);
    setup_relay(RELAY_3);
    setup_relay(RELAY_4);
    setup_relay(RELAY_5);
    setup_relay(RELAY_6);
    setup_relay(RELAY_7);
    setup_relay(RELAY_8);
}

void relay_on(uint16_t pin){
  digitalWrite(pin, LOW);
}

void relay_off(uint16_t pin){
  digitalWrite(pin, HIGH);
}

void pulse_relay(uint16_t pin){
  relay_on(pin);
  delay(500);
  relay_off(pin);
}

void relay_cycle_test() {
  pulse_relay(RELAY_1);
  pulse_relay(RELAY_2);
  pulse_relay(RELAY_3);
  pulse_relay(RELAY_4);
  pulse_relay(RELAY_5);
  pulse_relay(RELAY_6);
  pulse_relay(RELAY_7);
  pulse_relay(RELAY_8);
}

void setup() {
  Serial.begin(115200);

  setup_relays();

  //CAN.setPins(can_cs, can_irq);
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
  relay_cycle_test();
}
