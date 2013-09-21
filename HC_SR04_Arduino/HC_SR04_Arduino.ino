#define Trig 8
#define Echo 9

float Duration, Distance;

void setup(){
  Serial.begin(9600);
  pinMode(Trig,OUTPUT);
  pinMode(Echo,INPUT);
}

void loop(){
  //period > 60ms
  delay(100);
  //send a 20us (>10us) high pulse of Trig pin
  digitalWrite(Trig,LOW);
  digitalWrite(Trig,HIGH);
  delayMicroseconds(20);
  digitalWrite(Trig,LOW);
  //measure the width of high pulse of Echo pin
  Duration=pulseIn(Echo,HIGH);
  //calculate the distance
  Distance = round((Duration/2) / 29.1*100)/100.00;
  //print result
  Serial.print("Distance: ");
  Serial.print(Distance);
  Serial.println(" cm");
}
