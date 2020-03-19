
#include <FastLED.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#define NUM_LEDS 4
#define DATA_PIN 8
#include <Servo.h>
Servo xservo;
Servo yservo;

int xpos = 90;
int ypos = 90;
int xposup = 150;
int xposdown = 80;
int yposup = 150;
int yposdown = 80;

int servodiff = 1;
int servoDelay = 30;
CRGB leds[NUM_LEDS];
const uint64_t pipeIn = 0xE8E8F0F0E1LL; //Remember that this code is the same as in the transmitter

RF24 radio(9, 10);
float leftSpeed = 0;
float rightSpeed = 0;
//We could use up to 32 channels
struct MyData {
  byte throttle; //We define each byte of data input, in this case just 6 channels
  byte yaw;
  byte pitch;
  byte roll;
  byte AUX1;
  byte AUX2;
  byte AUX3;
  byte AUX4;
};

MyData data;

long prev = 0;
long noww = 0;

void resetData()
{
  //We define the inicial value of each data input
  //3 potenciometers will be in the middle position so 127 is the middle from 254
  data.throttle = 125;
  data.yaw = 129;
  data.pitch = 127;
  data.roll = 132;
  data.AUX1 = 0;
  data.AUX2 = 0;

}

/**************************************************/

void setup()
{
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  Serial.begin(250000); //Set the speed to 9600 bauds if you want.
  //You should always have the same speed selected in the serial monitor
  resetData();
  radio.begin();
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);

  radio.openReadingPipe(1, pipeIn);
  //we start the radio comunication
  radio.startListening();

  FastLED.addLeds<WS2811, DATA_PIN, GRB>(leds, NUM_LEDS);

  xservo.attach(A0);
  yservo.attach(A1);

  leds[0] = CRGB::Cyan;
  leds[1] = CRGB::Cyan;
  leds[2] = CRGB::Red;
  leds[3] = CRGB::Red;
  FastLED.show();
  delay(1000);
  leds[0] = CRGB::Black;
  leds[1] = CRGB::Black;
  leds[2] = CRGB::Black;
  leds[3] = CRGB::Black;
  FastLED.show();

}

/**************************************************/

unsigned long lastRecvTime = 0;

void recvData()
{
  while ( radio.available() ) {
    radio.read(&data, sizeof(MyData));
    lastRecvTime = millis(); //here we receive the data
  }
}

/**************************************************/
void blinkLED() {
  noww = millis();
  if (noww - prev >= 3000) {
    leds[0] = CRGB::Cyan;
    leds[1] = CRGB::Cyan;
    leds[2] = CRGB::Red;
    leds[3] = CRGB::Red;
    FastLED.show();
    delay(75);
    leds[0] = CRGB::Black;
    leds[1] = CRGB::Black;
    leds[2] = CRGB::Black;
    leds[3] = CRGB::Black;
    FastLED.show();
    prev = millis();
  }
}


void loop()
{
  blinkLED();
  recvData();
  unsigned long now = millis();
  //Here we check if we've lost signal, if we did we reset the values
  if ( now - lastRecvTime > 1000 ) {
    // Signal lost?
    resetData();
  }

  Serial.print("Throttle: "); Serial.print(data.throttle);  Serial.print("    ");
  Serial.print("Yaw: ");      Serial.print(data.yaw);       Serial.print("    ");
  Serial.print("Pitch: ");    Serial.print(data.pitch);     Serial.print("    ");
  Serial.print("Roll: ");     Serial.print(data.roll);      Serial.print("    ");
  Serial.print("Aux1: ");     Serial.print(data.AUX1);      Serial.print("    ");
  Serial.print("Aux2: ");     Serial.print(data.AUX2);      Serial.print("    ");
  Serial.print("Aux3: ");     Serial.print(data.AUX3);      Serial.print("    ");
  Serial.print("Aux4: ");     Serial.print(data.AUX4);      Serial.print("\n");
  //Serial.println(millis());

  if (data.throttle < 125) {
    digitalWrite(6, HIGH);
    digitalWrite(7, LOW);
    leftSpeed = (129 - data.throttle) * 1.9;
    analogWrite(5, int(leftSpeed));

  } else if (data.throttle > 135) {
    digitalWrite(6, LOW);
    digitalWrite(7, HIGH);
    leftSpeed = ((data.throttle - 130) * 1.9);
    analogWrite(5, int(leftSpeed));

  } else {
    digitalWrite(6, LOW);
    digitalWrite(7, LOW);
    digitalWrite(5, 0);

  }

  if (data.pitch < 125) {
    digitalWrite(4, HIGH);
    digitalWrite(2, LOW);
    rightSpeed = (129 - data.pitch) * 1.9;
    analogWrite(3, int(rightSpeed));

  } else if (data.pitch > 135) {
    digitalWrite(4, LOW);
    digitalWrite(2, HIGH);
    rightSpeed = ((data.pitch - 130) * 1.9);
    analogWrite(3, int(rightSpeed));

  } else {
    digitalWrite(2, LOW);
    digitalWrite(4, LOW);
    digitalWrite(3, 0);

  }

  if (data.yaw < 80) {
    if (xpos < xposdown) {

    } else {
      
      xservo.attach(A0);
      xpos = xpos - servodiff;
      xservo.write(xpos);
      delay(servoDelay);
    }


  } else if (data.yaw > 200) {
    if (xpos > xposup) {

    } else {
      
      xservo.attach(A0);
      xpos = xpos + servodiff;
      xservo.write(xpos);
      delay(servoDelay);
    }

  } else if (data.yaw > 81 && data.yaw < 199) {
    xservo.detach();
   
  }


  if (data.roll < 80) {
    
    
    if (xpos > xposup) {

    } else {
     
      yservo.attach(A1);
      ypos = ypos + servodiff;
      yservo.write(ypos);
      delay(servoDelay);
    }
    
    
    
  } else if (data.roll > 200) {


    if (ypos < yposdown) {

    } else {
    
      yservo.attach(A1);
      ypos = ypos - servodiff;
      yservo.write(ypos);
      delay(servoDelay);
    }


  } else if (data.roll > 81 && data.roll < 199) {
    yservo.detach();
   
  }



}



void demoGo() {
  digitalWrite(6, HIGH);
  digitalWrite(7, LOW);
  analogWrite(5, 255);
  digitalWrite(4, HIGH);
  digitalWrite(2, LOW);
  analogWrite(3, 255);
  delay(1000);
  digitalWrite(6, LOW);
  digitalWrite(7, LOW);
  analogWrite(5, 0);
  digitalWrite(4, LOW);
  digitalWrite(2, LOW);
  analogWrite(3, 0);
}



/**************************************************/
