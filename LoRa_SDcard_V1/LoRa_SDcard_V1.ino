#include <SPI.h>
#include <SD.h>
#include <LoRa.h>
#include <Arduino_LSM6DS3.h>

const int csPin = 4;     // LoRa radio chip select
const int resetPin = 2;  // LoRa radio reset
const int irqPin = 3;   // hardware interrupt
const int chipSelect = 10;
float x, y, z;
int mysensvals[4] = {,,,};
byte msgCount = 0;     // Message counter
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
Serial.print("Initializing SD card...");

  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("1. is a card inserted?");
    Serial.println("2. is your wiring correct?");
    Serial.println("3. did you change the chipSelect pin to match your shield or module?");
    Serial.println("Note: press reset button on the board and reopen this Serial Monitor after fixing your issue!");
    while (true);
  }

  Serial.println("initialization done.");

  //initialise sensors
  
//accelerometer setup
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");

    while (1);
  }

  Serial.print("Accelerometer sample rate = ");
  Serial.print(IMU.accelerationSampleRate());
  Serial.println(" Hz");
}


void loop() {

  //individual sensor codes go here
    if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(x, y, z);

    Serial.print(x);
    Serial.print('\t');
    Serial.print(y);
    Serial.print('\t');
    Serial.println(z);
  }
//sending packet data via LoRa
  Serial.print("Sending packet: ");
  Serial.println(msgCount);

  // Send packet
  LoRa.beginPacket();
  LoRa.print("Team 8 LoRa test: ");
  LoRa.print(msgCount);
  LoRa.endPacket();

  // Increment packet counter
  msgCount++;

  //Storing data in SD card
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(msgCount);
    dataFile.close();
    // print to the serial port too:
    Serial.println(msgCount);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }
  delay(5000);
}
