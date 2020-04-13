#include <Arduino.h>

#include <CAN.h>
#include <SPI.h>
#include <math.h>

#define CAN_ID_HWTANK_TEMP     0xA1

/*
Junction box (right to left):
 1. Hot water enable from programmer
 2. Thermostat Downstairs
 3. Thermostat Upstairs
 4. Hot water Valve req
 5. Rads Valve req
 6. Hot water Valve open
 7. Rads Valve open
 8. Boiler Request
 9. Pump request from Boiler
 10. Pump on
*/

#define RELAY_1                9  // Hot Water Valve (request) [4]
#define RELAY_2                8  // Rads Valve (request) [5]
#define RELAY_3                7  // Boiler (request) [8]
#define RELAY_4                6  // NC
#define RELAY_5                5  // NC
#define RELAY_6                4  // NC
#define RELAY_7                3  // NC
#define RELAY_8                0  // NC

#define S_1                    A7  // Thermostat Down stairs [2]
#define S_2                    A6  // Thermostat Up stairs [3]
#define S_3                    A5  // Hot Water Valve open [6]
#define S_4                    A4  // Rads Valve Open [7]
#define S_5                    A3  // NC
#define S_6                    A2  // Boiler request check [8]
#define S_7                    A1  // Pump request from Boiler [9]
#define S_8                    A0  // Hot Water (Programmer Enable) [1]

#define TRUE_THRESHOULD        200

const long mcp2515_spi_freq = 10E6;
const long mcp2515_can_freq = 500E3;
const long mcp2515_clock_freq = 16E6;

uint8_t rxbuf[8] = {0};
float hwtank_temp = 0;

bool mains_states[8] = {0};

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

  // TODO: do something more sensible than just average
  // ignore sensors with errors
  // - Range errors: sensible tank temperatures
  // - One value deviates too much from the others
  hwtank_temp = (therm1 + therm2 + therm3) / 3;
}


void onReceive(int packetSize) {
  int byte_no = 0;

  // Serial.print("packet with id 0x");
  // Serial.println(CAN.packetId(), HEX);

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

void read_states() {
  mains_states[0] = analogRead(S_1) < TRUE_THRESHOULD;
  mains_states[1] = analogRead(S_2) < TRUE_THRESHOULD;
  mains_states[2] = analogRead(S_3) < TRUE_THRESHOULD;
  mains_states[3] = analogRead(S_4) < TRUE_THRESHOULD;
  mains_states[4] = analogRead(S_5) < TRUE_THRESHOULD;
  mains_states[5] = analogRead(S_6) < TRUE_THRESHOULD;
  mains_states[6] = analogRead(S_7) < TRUE_THRESHOULD;
  mains_states[7] = analogRead(S_8) < TRUE_THRESHOULD;

  Serial.print(hwtank_temp);
  Serial.print(' ');
  Serial.print(mains_states[0]);
  Serial.print(' ');
  Serial.print(mains_states[1]);
  Serial.print(' ');
  Serial.print(mains_states[2]);
  Serial.print(' ');
  Serial.print(mains_states[3]);
  Serial.print(' ');
  Serial.print(mains_states[4]);
  Serial.print(' ');
  Serial.print(mains_states[5]);
  Serial.print(' ');
  Serial.print(mains_states[6]);
  Serial.print(' ');
  Serial.println(mains_states[7]);
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
  read_states();

  // Hot water valve control
  bool hot_water_enable = mains_states[7];

  if (hot_water_enable) {
    if (hwtank_temp < 40) relay_on(RELAY_1); // Hot water valve on
    if (hwtank_temp > 45) relay_off(RELAY_1); // Hot water valve off
  }

  // Rads valve control

  bool downstairs_rads_req = mains_states[0];
  bool upstairs_rads_req = mains_states[1];

  if (downstairs_rads_req || upstairs_rads_req){
    relay_on(RELAY_2); // Rads valve on
  } else {
    relay_off(RELAY_2); // Rads valve off
  }

  // Boiler control

  bool hot_water_valve_on = mains_states[2];
  bool rads_valve_on = mains_states[3];

  if (hot_water_valve_on || rads_valve_on){
    relay_on(RELAY_3); // Boiler on
  } else {
    relay_off(RELAY_3); // Boiler off
  }

  delay(500);
}
