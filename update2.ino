#include <SD.h>
#include <math.h>  // Include for sqrt()

int sensorPin = A1; // Analog pin connected to Vout of MPXV7002DP
float sensorValue;
float voltage;
float pressure_kPa;
float pressure_Pa;
float airspeed;
float dynamicPressure;
const float Vsupply = 5.0;  // Supply voltage (V)
const float rho = 1.225;    // Air density (kg/m³) at sea level
const int chipSelect = 10;

void setup() {
  Serial.begin(9600);
  pinMode(sensorPin, INPUT);

  // Initialize SD card
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
}

void loop() {
  // Read raw ADC value (0-1023)
  sensorValue = analogRead(sensorPin);

  // Convert ADC value to voltage
  voltage = (sensorValue / 1023.0) * Vsupply;

  // Convert voltage to differential pressure (kPa)
  pressure_kPa = ((voltage / Vsupply) - 0.5) * 5.0;

  // Convert kPa to Pascals (1 kPa = 1000 Pa)
  pressure_Pa = pressure_kPa * 1000;

  // Ensure pressure is positive before calculating airspeed (avoid sqrt of negative values)
  if (pressure_Pa > 0) {
    airspeed = sqrt((2 * pressure_Pa) / rho);
  } else {
    airspeed = 0.0; // No airflow
  }

  // Compute dynamic pressure
  dynamicPressure = 0.5 * rho * airspeed * airspeed;

  // Print values to Serial Monitor
  Serial.print("Voltage: ");
  Serial.print(voltage, 3);
  Serial.print(" V, Pressure: ");
  Serial.print(pressure_kPa, 3);
  Serial.print(" kPa, Airspeed: ");
  Serial.print(airspeed, 3);
  Serial.print(" m/s, Dynamic Pressure: ");
  Serial.print(dynamicPressure, 3);
  Serial.println(" Pa");

  // Storing data in SD card
  File dataFile = SD.open("datalog.txt", FILE_WRITE);  // Open or create the file on SD card
  if (dataFile) { // If the file is open
    dataFile.print(voltage);
    dataFile.print(", ");
    dataFile.print(pressure_kPa);
    dataFile.print(", ");
    dataFile.print(airspeed);
    dataFile.print(", ");
    dataFile.print(dynamicPressure);
    dataFile.println();  // Add a newline for each data entry

    dataFile.close();  // Close the file
    Serial.println("Data logged to SD card");
  } else {
    Serial.println("Error opening datalog.txt");
  }

  delay(500); // Small delay for readability
}

