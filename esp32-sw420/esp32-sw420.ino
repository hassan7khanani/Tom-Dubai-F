const int SW420_PIN = 4;  // Pin connected to SW420 sensor
const unsigned long VIBRATION_DURATION = 1000;  // Duration to detect continuous vibration (in milliseconds)

unsigned long vibrationStartTime = 0;  // Variable to store the start time of vibration

void setup() {
  Serial.begin(115200);
  pinMode(SW420_PIN, INPUT);
}

void loop() {
  int vibration = digitalRead(SW420_PIN);

  if (vibration == HIGH) {
    if (vibrationStartTime == 0) {
      vibrationStartTime = millis();  // Start timer if vibration is detected
    } else {
      unsigned long currentTime = millis();
      unsigned long duration = currentTime - vibrationStartTime;
      
      if (duration >= VIBRATION_DURATION) {
        Serial.println("Vibration detected!");
        vibrationStartTime = 0;  // Reset the start time
      }
    }
  } else {
    vibrationStartTime = 0;
    Serial.println("no");// Reset the start time if no vibration
  }
  
  delay(100);  // Delay for stability and to avoid false triggers
}
