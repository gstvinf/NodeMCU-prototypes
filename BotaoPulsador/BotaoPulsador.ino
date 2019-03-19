const int buttonPin = 5;
const int ledPin = 4;

bool ledStatus = false;
int buttonState = 0;
int lastButtonState = 1;

void setup() {
  pinMode(buttonPin, INPUT); 
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200); 
}

void loop(){
  lastButtonState = buttonState;
  buttonState = digitalRead(buttonPin);

  if (lastButtonState == HIGH && buttonState == LOW) {
    ledStatus = !ledStatus;
  }

  if (ledStatus) {
    digitalWrite(ledPin, HIGH);
  } else {
    digitalWrite(ledPin, LOW);
  }

  delay(10);
}
