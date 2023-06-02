int vibr_pin = 23;
int LED_Pin = 2;
int count = 0;

void setup() {
  Serial.begin(9600);
  pinMode(vibr_pin, INPUT);
  pinMode(LED_Pin, OUTPUT);
}

void loop() {
  int val = digitalRead(vibr_pin);
  
  if (val == 1) {
    count++;
    Serial.println(count);
    if (count >= 3) {
      Serial.println("Vibration detected");
      count = 0;  // Reset the count
    }
    digitalWrite(LED_Pin, HIGH);
    delay(1000);
    digitalWrite(LED_Pin, LOW);
    delay(1000);
  } else {
    count = 0;  // Reset the count if no vibration
    Serial.println("No vibration");
    digitalWrite(LED_Pin, LOW);
  }
}
