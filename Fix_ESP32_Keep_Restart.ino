void setup() {
  Serial.begin(115200);
}

void loop() {
  Serial.println("ESP32 run normal ...");
  
  for (int i = 0; i < 10000; i++) {
    // Do something...

// REQUIRED: Add yield() if the loop takes more than a few milliseconds to run
    yield(); 
  }

  delay(10); 
}