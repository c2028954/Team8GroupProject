//libraries that we will be using
#include <SPI.h>                     //used for the SD card
#include <SD.h>                     //SD library
#include <Wire.h>                  //used for I2C comms
#include <LoRa.h>                 // for LoRa
#include <Arduino_LSM6DS3.h>     //gyro and acc sensor library
#include <Adafruit_Sensor.h>    //general adafruit sensor library dependancy for the BME280 library
#include <Adafruit_BME280.h>   //BME280 library

float gx,gy,gz,ax,ay,az;
const int csPin = 4;         // LoRa radio chip select
const int resetPin = 2;     // LoRa radio reset
const int irqPin = 3;      // hardware interrupt
const int chipSelect = 10;// SD chip select pin
unsigned status;         // initialising a variable for the status of the BME sensor 
int Ppin = A1;          // pin used for the pressure sensor data in
float Pvalue;          // varaiable to store pressure sensor data value in
float mysense[8];       // array to display data of sensors
Adafruit_BME280 bme; // initialise a 'virtual' bme sensor to interact with via the code
void setup() {
  Serial.begin(9600);
    while (!Serial)
    ;
 // LORA MODULE SETUP
  LoRa.setPins(csPin, resetPin, irqPin);
  Serial.println("Initializing LoRa...");
  if (!LoRa.begin(866E6)) {
    Serial.println("LoRa initialization failed.");
    //while (1)
  }else{
  Serial.println("LoRa initialization done.");
  }
  //SD CARD SETUP
  Serial.println("Initializing SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println(" SD CARD initialization failed.");
    
    //while (true);
  }else{
  Serial.println("SD initialization done.");
  }
  //SENSOR SETUP

  //BME280 SETUP
  //Serial.println("point 1 reached");
  status = bme.begin(0x76);
  //Serial.println("point 2 reached");
  if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
        Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(),16);
        Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
        Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
        Serial.print("        ID of 0x60 represents a BME 280.\n");
        Serial.print("        ID of 0x61 represents a BME 680.\n");
        //while (1) delay(10);
    }
//LSM6DS3 SETUP
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    //while (1);
  }
  Serial.print("Accelerometer sample rate = ");
  Serial.print(IMU.accelerationSampleRate());
  Serial.println(" Hz");

//MPX7002DP SETUP
pinMode(Ppin,INPUT);
}
void loop() {
  // INDIVIDUAL SENSOR CODES GO RIGHT BELOW HERE 

  //pressure sensor code
  Pvalue = analogRead(Ppin);
  if(Pvalue>100){
  mysense[0] = Pvalue;
  }else{
    mysense[0] = 0;
  }
  //BME sensor code
  if(status){
  mysense[1] = bme.readTemperature();
  }else{
    mysense[1] = 0;
  }
  //accelerometer code
  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(ax, ay, az);
    mysense[2] = ax;
    mysense[3] = ay;
    mysense[4] = az;   
  }else{
    mysense[2] = mysense[3] = mysense[4] = 0;
  }
  //gyroscope code
  if (IMU.gyroscopeAvailable()) {
    IMU.readGyroscope(gx,gy,gz);
    mysense[5] = gx;
    mysense[6] = gy;
    mysense[7] = gz;    
  }else{
    mysense[5] = mysense[6] = mysense[7] = 0;
  }
  //sending packet data via LoRa
  Serial.print("Sending packet: ");
  

  // Send packet
  LoRa.beginPacket();
  for (int i = 0; i < 8; i++) {
        LoRa.print(mysense[i]); // Convert float to string and send
        if (i < 7) LoRa.print(","); // Separate values with commas
    }
  LoRa.endPacket();

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
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }
  for (int i = 0; i < 8; i++) {
        Serial.print(mysense[i]);
        if (i < 7) Serial.print(",");
    }
  Serial.println();
  delay(50); // miniumum this legally can be is 100 but who really cares about legality atm
}