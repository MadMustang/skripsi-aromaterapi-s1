//Auto On/Off Testing 10 Second time//


int led = 3;
int in1 = 7;

void setup() {
  pinMode(in1, OUTPUT);
  digitalWrite(in1, HIGH);
  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH);
}

void loop() {
  digitalWrite(in1, LOW);
  delay(10000);  
  digitalWrite(led, LOW);
  
  digitalWrite(in1, HIGH);
  delay(10000);
  digitalWrite(led, HIGH);
}
