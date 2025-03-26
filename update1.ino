int sensorPin = A1; // Analog pin connected to Vout of MPXV7002DP
float sensorValue;
float voltage;
float pressure_kPa;
float pressure_Pa;
float airspeed;
float dynamicPressure;
const float Vsupply = 5.0;  // Supply voltage (V)
const float rho = 1.225;    // Air density (kg/m³) at sea level

void setup() {
  Serial.begin(9600);
  pinMode(sensorPin, INPUT);
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

  delay(500); // Small delay for readability
}
 
