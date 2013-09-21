#include <LiquidCrystal.h>
#include <Timer.h>

/////////////////////////// Constant //////////////////////////////////////////
//for DHT11
#define DHT11Pin 2
//for LCD1602
//Vss -> GND
//Vdd -> 5V
//V0 -> 6 (use pin6 to control light of LCD since I don't have suitable resister)
//Rs -> 3
//Rw -> 4
//E -> 5
//D4 -> 10
//D5 -> 11
//D6 -> 12
//D7 -> 13
#define V0 6
#define Rs 3
#define Rw 4
#define En 5
#define D4 10
#define D5 11
#define D6 12
#define D7 13
//circle for unit of temp
byte circle[8]={
  B00110,
  B01001,
  B01001,
  B00110,
  B00000,
  B00000,
  B00000,
};
//define lcd
LiquidCrystal lcd(Rs,Rw,En,D4,D5,D6,D7);
//for HC-SR04
#define Trig 8
#define Echo 9
//Timer
Timer t1;//for display
Timer t2;//for measure DHT11
Timer t3;//for HC-SR04
//////////////////////////////////////////////////////////////////////////////

///////////////////////// Veriable //////////////////////////////////////////
//temp and humi
int Result[3]={0};//store new result that is after convert
int chr[40]={0};//store result that is before convert
//distance
float Duration, Distance, VoiceSpeed;
/////////////////////// Veriable /////////////////////////////////////////////

void setup(){
  pinMode(Trig,OUTPUT);
  pinMode(Echo,INPUT);
  analogWrite(V0,2.5/5*255);
  lcd.begin(16,2);
  lcd.createChar(0,circle);
  lcd.setCursor(0,0);
  lcd.print("     Waiting");
  lcd.setCursor(0,1);
  lcd.print("     Waiting");
  t1.every(1000,displayToscreen);//1s reflash screen
  t2.every(1000,measureTemp);//1s measure temperature and hunidity
  t3.every(1000,measureDist);//0.5s measure distance
  Serial.begin(9600);
}

void loop(){
  t1.update();
  t2.update();
  t3.update();
}

void measureTemp(){
  //set the mode of pin to output
    pinMode(DHT11Pin,OUTPUT);
    //Start signal
    //low -> 20ms (>18ms)
    //high -> 40us (20us~40us)
    digitalWrite(DHT11Pin,LOW);
    delay(20);
    digitalWrite(DHT11Pin,HIGH);
    delayMicroseconds(40);
    //Set pin low to wait responds (set mode to input)
    digitalWrite(DHT11Pin,LOW);
    pinMode(DHT11Pin,INPUT);
    //responds: low -> 80us, high -> 80us
    //To avoid ignore data that is after the responds, use while loop to test responds
    int loopCount=1000;
    while(digitalRead(DHT11Pin)!=HIGH){
      if(loopCount--==0){
        Result[0]=0;
        Result[1]=0;
        Result[3]=0;
        return;
      }
    }
    loopCount=3000;
    while(digitalRead(DHT11Pin)!=LOW){
      if(loopCount--==0){
        Result[0]=1;
        Result[1]=1;
        Result[3]=1;
        return;
      }
    }
    //After get responds, begin to get data
    //low -> waiting
    //high -> If 26us~28us, it is "0". If 70us, it is "1". I use 50us as the threshold
    for(int i=0;i<40;i++){
      while(digitalRead(DHT11Pin)==LOW){}
      long time = micros();
      while(digitalRead(DHT11Pin)==HIGH){}
      if(micros()-time>50){
        chr[i]=1;
      }else{
        chr[i]=0;
      }
    }
    //Convert data to result
    //Result[0] -> humi
    //Result[1] -> Temp
    //Result[2] -> tol (if tol!=humi+Temp, result is wrong and Result=101)
    Result[0]=chr[0]*128+chr[1]*64+chr[2]*32+chr[3]*16+chr[4]*8+chr[5]*4+chr[6]*2+chr[7];
    Result[1]=chr[16]*128+chr[17]*64+chr[18]*32+chr[19]*16+chr[20]*8+chr[21]*4+chr[22]*2+chr[23];
    Result[2]=chr[32]*128+chr[33]*64+chr[34]*32+chr[35]*16+chr[36]*8+chr[37]*4+chr[38]*2+chr[39];
    if(Result[2]!=Result[0]+Result[1]){
      Result[0]=1;
      Result[1]=0;
      Result[2]=1;
    }
}

void measureDist(){
  //if the Temp is wrong, the distance will be 0
  if ((Result[0]==0 && Result[1]==0 && Result[2]==0) || (Result[0]==1 && Result[1]==1 && Result[2]==1) || (Result[0]==1 && Result[1]==0 && Result[2]==1)){
    Distance=0;
  }else{
    //if the Temp is correct, begin to measure distance
    //calculate voice speed according to http://zh.wikipedia.org/wiki/%E9%9F%B3%E9%80%9F
    VoiceSpeed=331+0.6*Result[1];//(m/s)
    //send a 50us (>10us) high pulse of Trig pin
    digitalWrite(Trig,LOW);
    digitalWrite(Trig,HIGH);
    delayMicroseconds(50);
    digitalWrite(Trig,LOW);
    //measure the width of high pulse of Echo pin
    Duration=pulseIn(Echo,HIGH);
    Serial.println(Duration);
    //calculate the distance
    Distance = round((Duration/2)*(VoiceSpeed/10000)*100)/100.00;
  }
}

void displayToscreen(){
  lcd.clear();
  lcd.setCursor(0,0);
  if ((Result[0]==0 && Result[1]==0 && Result[2]==0) || (Result[0]==1 && Result[1]==1 && Result[2]==1) || (Result[0]==1 && Result[1]==0 && Result[2]==1)){
    lcd.print("     Waiting");
  }else{
      lcd.print("   ");
      lcd.print(Result[0]);
      lcd.print(" %  ");
      lcd.print(Result[1]);
      lcd.write(byte(0));
      lcd.print("C");
  }
  lcd.setCursor(0,1);
  if(Distance==0){
    lcd.print("     Waiting");
  }else{
    lcd.print(" Dist: ");
    lcd.print(Distance);
    lcd.print(" cm");
  }
}
