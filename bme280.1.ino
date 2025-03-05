#include <Wire.h>
#include <SPI.h>   // for the SD card module
#include <SD.h>    // for the SD card
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <RTClib.h>

// Create BME280 instance
Adafruit_BME280 bme;

// SD card chip select pin (change if necessary)
const int chipSelect = 4;

// Create a file to store the data
File myFile;

// RTC instance
RTC_DS1307 rtc;

void setup() {
    Serial.begin(9600);
    while (!Serial);
    
    // Initialize BME280
    if (!bme.begin(0x76)) {  // Default I2C address is 0x76; change if necessary
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        while (1);
    }
    
    // Initialize RTC
    if (!rtc.begin()) {
        Serial.println("Couldn't find RTC");
        while (1);
    }
    if (!rtc.isrunning()) {
        Serial.println("RTC is NOT running, setting time...");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
    
    // Initialize SD card
    Serial.print("Initializing SD card...");
    if (!SD.begin(chipSelect)) {
        Serial.println("Initialization failed!");
        return;
    }
    Serial.println("Initialization done.");
    
    // Open file and write headers
    myFile = SD.open("DATA.txt", FILE_WRITE);
    if (myFile) {
        Serial.println("File opened successfully");
        myFile.println("Date,Time,Temperature (C)");
        myFile.close();
    }
}

void loggingTime() {
    DateTime now = rtc.now();
    myFile = SD.open("DATA.txt", FILE_WRITE);
    if (myFile) {
        myFile.print(now.year(), DEC);
        myFile.print('/');
        myFile.print(now.month(), DEC);
        myFile.print('/');
        myFile.print(now.day(), DEC);
        myFile.print(',');
        myFile.print(now.hour(), DEC);
        myFile.print(':');
        myFile.print(now.minute(), DEC);
        myFile.print(':');
        myFile.print(now.second(), DEC);
        myFile.print(',');
    }
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(' ');
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.println(now.second(), DEC);
    myFile.close();
}

void loggingSensorData() {
    float temperature = bme.readTemperature();
    
    Serial.print("Temperature: "); Serial.print(temperature); Serial.println(" *C");
    
    myFile = SD.open("DATA.txt", FILE_WRITE);
    if (myFile) {
        myFile.println(temperature);
        myFile.close();
    }
}

void loop() {
    loggingTime();
    loggingSensorData();
    delay(5000);  // Log data every 5 seconds
}
