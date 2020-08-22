#include <Arduino.h>

#include <CAN.h>
#include <SPI.h>
#include <math.h>

#define CAN_ID_HWTANK_TEMP          0xA1
#define CAN_ID_CONTROLLER_STATES    0xA2
#define CAN_ID_HOTWATER_SETPOINT    0x51

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

uint8_t relay_ids[8] = {
  9,  // Hot Water Valve (request) [4]
  8,  // Rads Valve (request) [5]
  7,  // Boiler (request) [8]
  6,  // NC
  5,  // NC
  4,  // NC
  3,  // NC
  0,  // NC
};

uint8_t mains_ids[8] = {
  A7,  // Thermostat Down stairs [2]
  A6,  // Thermostat Up stairs [3]
  A5,  // Hot Water Valve open [6]
  A4,  // Rads Valve Open [7]
  A3,  // NC
  A2,  // Boiler request check [8]
  A1,  // Pump request from Boiler [9]
  A0,  // Hot Water (Programmer Enable) [1]
};

#define TRUE_THRESHOULD        200

const long mcp2515_spi_freq = 10E6;
const long mcp2515_can_freq = 500E3;
const long mcp2515_clock_freq = 16E6;

uint8_t rxbuff[8] = {0};
float hwtank_temp = 0;

float hotwater_ON = 40;
float hotwater_OFF = 45;

struct RelayStates {
  bool R1_hot_water_valve_open_req;
  bool R2_rads_valve_open_req;
  bool R3_boiler_req;
  bool R4_NA;
  bool R5_NA;
  bool R6_NA;
  bool R7_NA;
  bool R8_NA;
};

struct MainsStates {
  bool S1_thermostat_downstairs;
  bool S2_thermostat_upstairs;
  bool S3_hot_water_valve_opened;
  bool S4_rads_valve_opened;
  bool S5_NA;
  bool S6_boiler_request_check;
  bool S7_pump_req_from_boiler;
  bool S8_hot_water_enable_programmer;
};

struct ThermData {
  uint16_t therm1;
  uint16_t therm2;
  uint16_t therm3;
};

struct TempSetPoint {
  uint8_t on_temp;
  uint8_t off_temp;
};

struct RelayStates relay_states;
struct MainsStates mains_states;

void packet_HWTANK_TEMP(uint8_t *msg) {
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

void packet_HOTWATER_SETPOINT(uint8_t *msg) {
  struct TempSetPoint *data = (struct TempSetPoint *) msg;

  hotwater_ON = ((float) data->on_temp) / 2;
  hotwater_OFF = ((float) data->off_temp) / 2;
}

void onReceive(int packetSize) {
  int byte_no = 0;

  // Serial.print("packet with id 0x");
  // Serial.println(CAN.packetId(), HEX);

  while (CAN.available()) {
    rxbuff[byte_no++] = (uint8_t) CAN.read();
  }

  switch (CAN.packetId()) {
    case CAN_ID_HWTANK_TEMP:
      packet_HWTANK_TEMP(rxbuff);
      break;

    case CAN_ID_HOTWATER_SETPOINT:
      packet_HOTWATER_SETPOINT(rxbuff);
      break;
  }

}

void transmit_status_can_packet() {
  //CAN.wakeup();
  CAN.beginPacket(CAN_ID_CONTROLLER_STATES);
  CAN.write(hwtank_temp);
  CAN.write(*(uint8_t *) &mains_states);
  CAN.write(*(uint8_t *) &relay_states);
  CAN.write(hotwater_ON * 2);
  CAN.write(hotwater_OFF * 2);
  CAN.endPacket();
  //CAN.sleep();
}

void setup_relay(uint16_t pin) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
}

void setup_relays() {
  for(int i=0; i < 8; i++){
    setup_relay(relay_ids[i]);
  }
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
  for(int i=0; i < 8; i++){
    pulse_relay(relay_ids[i]);
  }
}

void read_states() {
  mains_states.S1_thermostat_downstairs = analogRead(mains_ids[0]) < TRUE_THRESHOULD;
  mains_states.S2_thermostat_upstairs = analogRead(mains_ids[1]) < TRUE_THRESHOULD;
  mains_states.S3_hot_water_valve_opened = analogRead(mains_ids[2]) < TRUE_THRESHOULD;
  mains_states.S4_rads_valve_opened = analogRead(mains_ids[3]) < TRUE_THRESHOULD;
  mains_states.S5_NA = analogRead(mains_ids[4]) < TRUE_THRESHOULD;
  mains_states.S6_boiler_request_check = analogRead(mains_ids[5]) < TRUE_THRESHOULD;
  mains_states.S7_pump_req_from_boiler = analogRead(mains_ids[6]) < TRUE_THRESHOULD;
  mains_states.S8_hot_water_enable_programmer = analogRead(mains_ids[7]) < TRUE_THRESHOULD;

  relay_states.R1_hot_water_valve_open_req = digitalRead(relay_ids[0]);
  relay_states.R2_rads_valve_open_req = digitalRead(relay_ids[1]);
  relay_states.R3_boiler_req = digitalRead(relay_ids[2]);
  relay_states.R4_NA = digitalRead(relay_ids[3]);
  relay_states.R5_NA = digitalRead(relay_ids[4]);
  relay_states.R6_NA = digitalRead(relay_ids[5]);
  relay_states.R7_NA = digitalRead(relay_ids[6]);
  relay_states.R8_NA = digitalRead(relay_ids[7]);
  
  Serial.print(hwtank_temp);
  Serial.print(' ');
  Serial.print(*(uint8_t *) &mains_states);
  Serial.println(*(uint8_t *) &relay_states);
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
  if (mains_states.S8_hot_water_enable_programmer) {
    if (hwtank_temp < hotwater_ON) relay_on(relay_ids[0]); // Hot water valve on
    if (hwtank_temp > hotwater_OFF) relay_off(relay_ids[0]); // Hot water valve off
  } else {
    relay_off(relay_ids[0]); // Hot water valve off
  }

  // Rads valve control
  if (mains_states.S1_thermostat_downstairs || mains_states.S2_thermostat_upstairs){
    relay_on(relay_ids[1]); // Rads valve on
  } else {
    relay_off(relay_ids[1]); // Rads valve off
  }

  // Boiler control
  if (mains_states.S3_hot_water_valve_opened || mains_states.S4_rads_valve_opened){
    relay_on(relay_ids[2]); // Boiler on
  } else {
    relay_off(relay_ids[2]); // Boiler off
  }

  transmit_status_can_packet();

  delay(500);
}
