#define CLK 18
#define DT 19

int counter = 0;
int currentStateCLK;
int lastStateCLK;

void setup() {
  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  Serial.begin(9600);
  lastStateCLK = digitalRead(CLK);
}

void loop() {
  currentStateCLK = digitalRead(CLK);
  float degree;

  if (currentStateCLK != lastStateCLK) {
    if (digitalRead(DT) != currentStateCLK) {
      counter++;
    } else {
      counter--;
    }
    if(counter < 0){
      int ch = counter*-1;
      counter = ch;
    }
    degree = (counter * 360)/30;
    if(degree > 360){
      int ch;
      ch = degree - (counter/30)*360;
      degree = ch;
    }
    Serial.print("Degree: ");
    Serial.print(degree);
    Serial.print(" | Read: ");
    Serial.println(counter);
  }
  lastStateCLK = currentStateCLK;
}
