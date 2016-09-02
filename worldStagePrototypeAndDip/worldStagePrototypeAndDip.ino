#include "FastLED.h"
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>   

/* SWITCHES */
#define S1 8
#define S2 22
#define S3 21
#define S4 20

/* LED */
#define LED_1_DATA 14
#define LED_1_CLOCK 15

#define LED_2_DATA 16
#define LED_2_CLOCK 17

#define LED_3_DATA 18
#define LED_3_CLOCK 19

#define LED_4_DATA 2
#define LED_4_CLOCK 3

#define LED_5_DATA 4
#define LED_5_CLOCK 5

#define LED_6_DATA 6
#define LED_6_CLOCK 7

/* GLOBALS */
#define LEDS_PER_COLUMN 48
#define TOTAL_LEDS 288

  int test_c = 0;
int test_v = 0;
int fadeValue = 3;

/* POT */
int potPin = A9;
int brightnessTestMode = 0;

// State of each switch (0 or 1)
int s1state;
int s2state;
int s3state;
int s4state;

// Our number
int x;

EthernetUDP Udp;

char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; 

const int LED_IN_BUFFER_SIZE= 864;
char ledBuffer[864]; 

int currentModule;
byte macs[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};

byte mac[] = { 0x04, 0xE9, 0xE5, 0x03, 0x18, 0x01 };

IPAddress ip;
unsigned int localPort = 8888;
//char replyBuffer[] = {'0'};     

const unsigned int resetPin = 9;
CRGB leds[288];

int state = 1;

void testLEDs();

void setup() {
  delay(2000);
  digitalWrite(resetPin, LOW);
  pinMode(resetPin, OUTPUT);
  delayMicroseconds(10);
  pinMode(resetPin, INPUT);

    // pins for switches are inputs
  pinMode(S1, INPUT_PULLUP);
  pinMode(S2, INPUT_PULLUP);
  pinMode(S3, INPUT_PULLUP);
  pinMode(S4, INPUT_PULLUP);
  delay(20);

  s1state = digitalReadFast(S4);
  s2state = digitalReadFast(S3);
  s3state = digitalReadFast(S2);
  s4state = digitalReadFast(S1);

  bitWrite(x, 0, !s1state);
  bitWrite(x, 1, !s2state);
  bitWrite(x, 2, !s3state);
  bitWrite(x, 3, !s4state);
  
  currentModule = x;
  
  mac[5] = macs[currentModule];
  ip = IPAddress(192, 168, 2, currentModule+100);
  Ethernet.begin(mac, ip);
  delay(200);
  
  Udp.begin(localPort);
  Serial.begin(9600);

  LEDS.addLeds<WS2803, LED_1_DATA, LED_1_CLOCK, RGB, DATA_RATE_MHZ(1)>(leds, LEDS_PER_COLUMN*0, LEDS_PER_COLUMN);
  LEDS.addLeds<WS2803, LED_2_DATA, LED_2_CLOCK, RGB, DATA_RATE_MHZ(1)>(leds, LEDS_PER_COLUMN*1, LEDS_PER_COLUMN);
  LEDS.addLeds<WS2803, LED_3_DATA, LED_3_CLOCK, RGB, DATA_RATE_MHZ(1)>(leds, LEDS_PER_COLUMN*2, LEDS_PER_COLUMN);
  LEDS.addLeds<WS2803, LED_4_DATA, LED_4_CLOCK, RGB, DATA_RATE_MHZ(1)>(leds, LEDS_PER_COLUMN*3, LEDS_PER_COLUMN);
  LEDS.addLeds<WS2803, LED_5_DATA, LED_5_CLOCK, RGB, DATA_RATE_MHZ(1)>(leds, LEDS_PER_COLUMN*4, LEDS_PER_COLUMN);
  LEDS.addLeds<WS2803, LED_6_DATA, LED_6_CLOCK, RGB, DATA_RATE_MHZ(1)>(leds, LEDS_PER_COLUMN*5, LEDS_PER_COLUMN);
  
  for (int i = 0; i < TOTAL_LEDS; i++) {
    leds[i] = CRGB(0, 0, 0);
  }
  FastLED.show();
  
}

void loop() {
  
  brightnessTestMode = map(analogRead(potPin), 0, 1023, 10, 255);
  FastLED.setBrightness(brightnessTestMode);
  if (currentModule == 0) {
    testLEDs();
  } else {
    int packetSize = Udp.parsePacket();
    if (packetSize == 2) {
      Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
      Udp.write((char)(currentModule+100));
      Udp.endPacket();
      state = 2;
    } else if (packetSize == 864) {
      Udp.read((char*)ledBuffer, 864);    
      memcpy(leds, ledBuffer, 864);
    } else if (packetSize == 1 || (state == 1 && packetSize != 2)) {
      FastLED.show();
    } 
  }
}

void testLEDs() {
  CRGB current_value;

  switch (test_c) {
      case 0:
        current_value = CRGB(test_v, 0, 0);
        break;
      case 1:
        current_value = CRGB(0, test_v, 0);
        break;
      case 2:
        current_value = CRGB(0, 0, test_v);
        break;   
      case 3:
        current_value = CRGB(test_v, test_v, test_v);
        break;   
    } 
    
  for (int i = 0; i < TOTAL_LEDS; i++) {
        leds[i] = current_value;
  }
  
  FastLED.show();
  
  test_v = test_v + fadeValue;

  if (test_v >= 255) {
    test_v = 255;
    fadeValue = fadeValue *-1;
  } else if (test_v <= 0) {
    test_v = 0;
    fadeValue = fadeValue *-1;
    test_c++;
  }

  if (test_c > 3) { 
    test_c = 0;
  }
  
  
}
