#include <Arduino.h>

#include <CAN.h>
#include <SPI.h>
#include <math.h>

# define CAN_ID_HWTANK_TEMP     0xA1

const int fixed_res = 6800;
const int therm_res = 10000;
const int therm_b = 3950;
const int adc_max = 1023;

const int thermPin1 = A0;
const int thermPin2 = A1;
const int thermPin3 = A2;

const int thermEnablePin1 = 7;
const int thermEnablePin2 = 6;
const int thermEnablePin3 = 5;

const int can_cs = 10;
const int can_irq = 2;
const int can_packet_size = 6;

struct ThermData {
  uint16_t therm1;
  uint16_t therm2;
  uint16_t therm3;
};

const long mcp2515_can_freq = 500E3;
const long mcp2515_spi_freq = 10E6;
const long mcp2515_clock_freq = 16E6;


float get_therm_value(int pin, int enable_pin) {
  float value = 0;

  digitalWrite(enable_pin, HIGH);
  value = analogRead(pin);
  digitalWrite(enable_pin, LOW);

  value = fixed_res * (value / (adc_max - value));

  value = log(value / therm_res) / therm_b;
  value += 1 / (25 + 273.15);
  value = 1 / value;
  value -= 273.15;

  return value;
}

void transmit_can_packet(float therm1, float therm2, float therm3) {
  //CAN.wakeup();
  uint8_t * buff;
  struct ThermData data;

  data.therm1 = (int) (therm1 * 100);
  data.therm2 = (int) (therm2 * 100);
  data.therm3 = (int) (therm3 * 100);

  buff = (uint8_t *) &data;
  
  CAN.beginPacket(CAN_ID_HWTANK_TEMP);
  CAN.write(buff[0]);
  CAN.write(buff[1]);
  CAN.write(buff[2]);
  CAN.write(buff[3]);
  CAN.write(buff[4]);
  CAN.write(buff[5]);
  CAN.endPacket();

  //CAN.sleep();
}

void setup() {
  Serial.begin(115200);

  pinMode(thermEnablePin1, OUTPUT);
  pinMode(thermEnablePin2, OUTPUT);
  pinMode(thermEnablePin3, OUTPUT);

  //CAN.setPins(can_cs, can_irq);
  //CAN.setSPIFrequency(mcp2515_spi_freq);
  //CAN.setClockFrequency(mcp2515_clock_freq);

  while (!CAN.begin(mcp2515_can_freq)) {
    Serial.println("Starting CAN failed!");

    delay(500);
  }
  
  Serial.println("CAN Initialised");
}

void loop() {
  float therm1 = get_therm_value(thermPin1, thermEnablePin1);
  float therm2 = get_therm_value(thermPin2, thermEnablePin2);
  float therm3 = get_therm_value(thermPin3, thermEnablePin3);

  Serial.print("therm1 = ");
  Serial.print(therm1);
  Serial.print("|");
  Serial.print((int) (therm1 * 100));
  Serial.print("\t therm2 = ");
  Serial.print(therm2);
  Serial.print("|");
  Serial.print((int) (therm2 * 100));
  Serial.print("\t therm3 = ");
  Serial.print(therm3);;
  Serial.print("|");
  Serial.println((int) (therm3 * 100));

  transmit_can_packet(therm1, therm2, therm3);
  delay(500);
}
