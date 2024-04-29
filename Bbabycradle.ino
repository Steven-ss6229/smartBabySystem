#include <Servo.h>
#include <DHT.h>

// Define pin connections
#define RAIN_SENSOR_DIGITAL_PIN 2
#define RAIN_SENSOR_ANALOG_PIN A0
#define DHT_PIN 3
#define SOUND_SENSOR_PIN 4
#define SOUND_SENSOR_ANALOG_PIN A1
#define BUZZER_PIN 5
#define SERVO_PIN 6
#define RELAY_PIN 7

// DHT setup
#define DHT_TYPE DHT11 // Change to DHT22 if using DHT22
DHT dht(DHT_PIN, DHT_TYPE);
 
// Servo setup
Servo cradleServo;

void setup() {
  pinMode(RAIN_SENSOR_DIGITAL_PIN, INPUT);
  pinMode(SOUND_SENSOR_PIN, INPUT);
  pinMode(SOUND_SENSOR_ANALOG_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  
  Serial.begin(9600);
  dht.begin();
  cradleServo.attach(SERVO_PIN);
}

void loop() {
  // Read rain sensor
  int dampnessLevel = analogRead(RAIN_SENSOR_ANALOG_PIN);
  bool isDamp = digitalRead(RAIN_SENSOR_DIGITAL_PIN);
  if (!isDamp) {
    digitalWrite(BUZZER_PIN, HIGH);
    // Serial.print("Dampness Detected! Level: ");
    // Serial.println(dampnessLevel);
  } else {
    digitalWrite(BUZZER_PIN, LOW);
  }
  
  // Read temperature and humidity
  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();
  //Serial.print("damp bool: ");
  //Serial.print("Temperature: ");
  //Serial.print("C, Humidity: ");
  Serial.print(!isDamp);
  Serial.print(",");
  Serial.print(temp);
  Serial.print(",");
  Serial.print(humidity);
  Serial.print(",");
    // Serial.print(",");
  Serial.println(analogRead(SOUND_SENSOR_ANALOG_PIN));
  // Serial.print("\n");
  //Serial.println("%");

  // Read and display sound level
  int soundLevel = analogRead(SOUND_SENSOR_ANALOG_PIN);
  // Serial.print("Sound Level: ");
  //Serial.println(digitalRead(SOUND_SENSOR_PIN));

  // Control fan based on temperature
  if (temp > 30) { // Set threshold according to need
    digitalWrite(RELAY_PIN, HIGH);
  } else {
    digitalWrite(RELAY_PIN, LOW);
  }
  
  // Detect baby cry and swing cradle
  if (soundLevel>200) {
    cradleServo.write(10); // Rotate to 45 degrees
    cradleServo.write(15); // Rotate to 45 degrees
    delay(500); // Adjust time as needed
    cradleServo.write(20); // Rotate to 45 degrees
    cradleServo.write(25); // Rotate to 45 degrees
    delay(500); // Adjust time as needed
    cradleServo.write(30); // Rotate to 45 degrees
    cradleServo.write(35); // Rotate to 45 degrees


    // cradleServo.write(120); // Rotate to 45 degrees
    delay(1000); // Adjust time as needed
    cradleServo.write(0); // Rotate back


  }
  
  delay(1000); // Wait for a second before next loop iteration
}
