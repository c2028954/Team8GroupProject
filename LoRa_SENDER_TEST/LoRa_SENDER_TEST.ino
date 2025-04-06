

// Include required libraries
#include <SPI.h>
#include <LoRa.h>

// Define the pins used by the LoRa module
const int csPin = 4;     // LoRa radio chip select
const int resetPin = 2;  // LoRa radio reset
const int irqPin = 3;    // Must be a hardware interrupt pin

// Message counter
byte msgCount = 0;

void setup() {

  Serial.begin(9600);
  while (!Serial)
    ;

  // Setup LoRa module
  LoRa.setPins(csPin, resetPin, irqPin);

  Serial.println("LoRa Sender Test");



  if (!LoRa.begin(866E6)) {
    Serial.println("Starting LoRa failed!");
    while (1)
      ;
  }
}

void loop() {

  Serial.print("Sending packet: ");
  Serial.println(msgCount);

  // Send packet
  LoRa.beginPacket();
  LoRa.print("Team 8 LoRa test: ");
  LoRa.print(msgCount);
  LoRa.endPacket();

  // Increment packet counter
  msgCount++;

  // 5-second delay
  delay(5000);
}