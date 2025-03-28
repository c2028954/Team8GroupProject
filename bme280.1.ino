#include <Wire.h>
#include <SPI.h>   // for the SD card module
#include <SD.h>    // for the SD card
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <RTClib.h>

// Create BME280 instance
Adafruit_BME280 bme;

// SD card chip select pin (change if necessary)
const int chipSelect = 10;

// Create a file to store the data
File myFile;

// RTC instance
RTC_DS1307 rtc;

// Simple Moving Average Filter settings
#define NUM_SAMPLES 10
float temperatureReadings[NUM_SAMPLES]; // Array to store temperature readings
int tempIndex = 0; // Index for the current reading
float totalTemperature = 0; // Sum of the current window of readings
float averageTemperature = 0; // Average of the readings in the window

void setup() {
    Serial.begin(9600);
    while (!Serial);  // Wait for the serial port to connect - necessary for Leonardo only

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
        Serial.println("SD card initialization failed!");
        return;
    }
    Serial.println("SD card initialized successfully.");

    // Open file and write headers
    myFile = SD.open("DATA.txt", FILE_WRITE);
    if (myFile) {
        Serial.println("File created successfully, storing data...");
        myFile.println("Date,Time,Temperature (C)");
        myFile.close();
    } else {
        Serial.println("Unsuccessful data storage! Could not create file.");
    }

    // Initialize temperature readings
    for (int i = 0; i < NUM_SAMPLES; i++) {
        temperatureReadings[i] = 0; // Initialize the array to 0
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
    } else {
        Serial.println("Unsuccessful data storage! Could not open file for writing time.");
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
    float currentTemperature = bme.readTemperature();
    totalTemperature -= temperatureReadings[tempIndex]; // Subtract the oldest temperature
    temperatureReadings[tempIndex] = currentTemperature; // Update the newest temperature
    totalTemperature += temperatureReadings[tempIndex]; // Add the newest temperature
    tempIndex = (tempIndex + 1) % NUM_SAMPLES; // Update index to the next position

    averageTemperature = totalTemperature / NUM_SAMPLES; // Calculate the average temperature

    Serial.print("Average Temperature: "); 
    Serial.print(averageTemperature); 
    Serial.println(" *C");
    
    myFile = SD.open("DATA.txt", FILE_WRITE);
    if (myFile) {
        myFile.println(averageTemperature);
        myFile.close();
        // Print success message after successfully writing the temperature
        Serial.println("Temperature recorded successfully.");
    } else {
        Serial.println("Unsuccessful data storage! Could not write temperature data.");
    }
}

void loop() {
    loggingTime();
    loggingSensorData();
    delay(5000);  // Log data every 5 seconds
}
