void setup() {
  pinMode(A0, INPUT_PULLUP);
  pinMode(A1, INPUT_PULLUP);
  pinMode(A2, OUTPUT);
  Serial.begin(9600);
}

int prev_a = 1;
int prev_b = 1;
void loop() {
  if (Serial.available()) {
    int ch = Serial.read();
    if (ch == '1') {
      digitalWrite(A2, 1);
    }
    if (ch == '0') {
      digitalWrite(A2, 0);
    }
  }
  int curr_a = digitalRead(A0);
  int curr_b = digitalRead(A1);
  if (prev_a != curr_a && !curr_a)
    Serial.write('1');
  if (prev_b != curr_b && !curr_b)
    Serial.write('0');
  prev_a = curr_a;
  prev_b = curr_b;
}
