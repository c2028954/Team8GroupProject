#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <LoRa.h>
#include <Arduino_LSM6DS3.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
const int csPin = 4;     // LoRa radio chip select
const int resetPin = 2;  // LoRa radio reset
const int irqPin = 3;   // hardware interrupt
const int chipSelect = 10; // SD chip select pin
unsigned status;
float x, y, z;
int Ppin = A1;
float Pvalue;
int mysense[8]; // array to display data of sensors
byte msgCount = 0;     // Message counter
Adafruit_BME280 bme;
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
Serial.println("Initializing SD card...");

  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("1. is a card inserted?");
    Serial.println("2. is your wiring correct?");
    Serial.println("3. did you change the chipSelect pin to match your shield or module?");
    Serial.println("Note: press reset button on the board and reopen this Serial Monitor after fixing your issue!");
    while (true);
  }

  Serial.println("SD initialization done.");

  //initialise sensors

  //BME280 temp sense set up
  Serial.println("point 1 reached");
  status = bme.begin(0x76);
  Serial.println("point 2 reached");
  if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
        Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(),16);
        Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
        Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
        Serial.print("        ID of 0x60 represents a BME 280.\n");
        Serial.print("        ID of 0x61 represents a BME 680.\n");
        //while (1) delay(10);
    }
//accelerometer setup
  /*if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");

    while (1);
  }

  Serial.print("Accelerometer sample rate = ");
  Serial.print(IMU.accelerationSampleRate());
  Serial.println(" Hz");
}*/
//Temp sensor setup

pinMode(Ppin,INPUT);
}
void loop() {

  //individual sensor codes go here
  //pressure sensor code
  Pvalue = analogRead(Ppin);
  mysense[0] = Pvalue;

  //BME sensor code
  mysense[1] = bme.readTemperature();


    /*if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(x, y, z);

    mysense[2] = x;
    mysense[3] = y;
    mysense[4] = z;
  }*/
//sending packet data via LoRa
  Serial.print("Sending packet: ");
  Serial.println(msgCount);

  // Send packet
  LoRa.beginPacket();
  for (int i = 0; i < 8; i++) {
        LoRa.print(mysense[i]); // Convert float to string and send
        if (i < 7) LoRa.print(","); // Separate values with commas
    }
  LoRa.endPacket();

  // Increment packet counter
  msgCount++;

  //Storing data in SD card
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    for (int i = 0; i < 8; i++) {
        dataFile.print(mysense[i]); // Convert float to string and send
        if (i < 7) dataFile.print(","); // Separate values with commas
    }
    dataFile.println();
    dataFile.close();
    // print to the serial port too:
    Serial.println(msgCount);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }
  Serial.println(mysense[0]);
  delay(50);
}
