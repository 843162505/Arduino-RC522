/*
     RC522门禁系统
     接线：
     RC522:     ARDUINO
     RST<------->D9
     SDA<------->D10
     MOSI<------>D11
     MISO<------>D12
     SCK<------->D13
     IRQ<------->不接
     3V3<------->3.3V（千万不能接5V）
     GND<------->GND
     Other:     ARDUINO
     BUZZER<---->D4
     RELAY<----->D5
     Button_write<------->D6
     WriteLight<----->D7
     Button_clear<------->D8
     GND<------->GND
*/
#include <SPI.h>
#include "RFID.h"
#include <EEPROM.h>
#define Button_write 6
#define Button_clear 8
#define WriteLight 7
#define BUZZER 4
#define RELAY 5
#define WriteWaitTime 15
unsigned char host[204][5];
unsigned char zero[5] = {0,0,0,0,0};

RFID rfid(10, 9);  //D10--读卡器SDA引脚、D5--读卡器RST引脚
boolean u = 0;//是否读到卡标记

void setup(){
  pinMode(Button_write, INPUT);//写卡按钮
	pinMode(Button_clear, INPUT);//复位按钮
  pinMode(WriteLight, OUTPUT);//写卡指示灯
  pinMode(BUZZER, OUTPUT);//蜂鸣器
  pinMode(RELAY, OUTPUT);//继电器
  digitalWrite(RELAY, HIGH);
  digitalWrite(WriteLight, HIGH);
  Serial.begin(9600);
  Cardarray();
  SPI.begin();
  rfid.init();
}
void loop() {
  Readcard();
  Writecard();
  clear();
}
void print() {
  Serial.print("The card's number is : ");
  Serial.print('\n');
  Serial.print("DEC:");
  Serial.print('\n');
  for (int i = 0; i < 4; i++) {
    Serial.print(rfid.serNum[i]);
    Serial.print(",");
  }
  Serial.print(rfid.serNum[4]);
  Serial.print('\n');
  Serial.print("HEX:");
  Serial.print('\n');
  for (int i = 0; i < 4; i++) {
    Serial.print("0x");
    Serial.print(rfid.serNum[i], HEX);
    Serial.print(",");
  }
  Serial.print("0x");
  Serial.print(rfid.serNum[4], HEX);
  Serial.print('\n');
}
void Readcard() {
  //找卡
  if (rfid.isCard()) {
  	Cardarray();
    Serial.print("Find the card!");
    Serial.print('\n');
    //读取卡序列号
    if (rfid.readCardSerial()) {
      print();
      int peopleNum_add = EEPROM.read(0);
      for (int i = 0; i < (peopleNum_add + 4) / 5; i++) {
        if (rfid.serNum[0, 1, 2, 3, 4] == host[i][0, 1, 2, 3, 4]) {
          Serial.print("yes");
          Serial.print('\n');
          digitalWrite(RELAY, LOW) ;//继电器闭合
          for (int i = 0; i < 80; i++) {
            u = 1;//这里为了标记读到了列表中的卡
            digitalWrite(BUZZER, HIGH);
            delay(1);
            digitalWrite(BUZZER, LOW);
            delay(1);
          }
          delay(998);
          digitalWrite(RELAY, HIGH);//继电器断开
        }
      }
    }
    if (!u) { //如果读到的卡不在列表中，那么执行下面程序
      Serial.print("no");
      Serial.print('\n');
      for (int i = 0; i < 100; i++) {
        digitalWrite(BUZZER, HIGH);
        delay(2);
        digitalWrite(BUZZER, LOW);
        delay(2);
      }
    }
    //选卡，可返回卡容量（锁定卡片，防止多数读取），去掉本行将连续读卡
    rfid.selectTag(rfid.serNum);
  }
  rfid.halt();
  u = 0; //为了下一次读卡准备
}
void Writecard() {
  if (digitalRead(Button_write)) {
  	Cardarray();
    for (int i = 0; i < WriteWaitTime; i++)
    {
      digitalWrite(WriteLight, !digitalRead(WriteLight));
      Serial.print(WriteWaitTime - i);
      Serial.print(" seconds left");
      Serial.print('\n');
      for (int o = 0; o < 33; o++)
      {
        if (rfid.isCard()) {
          Serial.print("Find the card!");
          Serial.print('\n');
          Serial.print("I am in the inside!");
          Serial.print('\n');
          //读取卡序列号
          if (rfid.readCardSerial()) {
            print();
            WriteEEPROM();
          }
          //选卡，可返回卡容量（锁定卡片，防止多数读取），去掉本行将连续读卡
          rfid.selectTag(rfid.serNum);
        }
        rfid.halt();
        delay(1);
      }
    }
    digitalWrite(WriteLight, HIGH);
  }
}
void WriteEEPROM() {
  //找卡
  Cardarray();
  int peopleNum_add = EEPROM.read(0);
  for (int i = 0; i < (peopleNum_add + 4) / 5; i++) {
    if (rfid.serNum[0, 1, 2, 3, 4] == host[i][0, 1, 2, 3, 4] or rfid.serNum[0, 1, 2, 3, 4] == zero[0,1,2,3,4]) {
      Serial.print("inside");
      Serial.print('\n');
      u = 1;//这里为了标记读到了列表中的卡
      for (int y = 0; y < 80; y++) {
        digitalWrite(BUZZER, HIGH);
        delay(1);
        digitalWrite(BUZZER, LOW);
        delay(1);
      }
    }
  }
  if (!u) { //如果读到的卡不在列表中，那么执行下面程序
    Serial.print("no inside");
    Serial.print('\n');
    if (peopleNum_add + 4 < EEPROM.length())
    {
      int o = 0;
      for (int i = peopleNum_add; i < peopleNum_add + 5; i++)
      {
        EEPROM.write(i, rfid.serNum[o]);
        o++;
        delay(10);
      }
      if (!o == 0)
      {
        for (int y = 0; y < 80; y++) {
          digitalWrite(BUZZER, HIGH);
          delay(1);
          digitalWrite(BUZZER, LOW);
          delay(1);
        }
        Serial.print("Write completion!");
        Serial.print('\n');
        EEPROM.write(0, EEPROM.read(0)+5);
      }
    }
  }
  rfid.halt();
  u = 0; //为了下一次读卡准备
}
void Cardarray() {
  int peopleNum_add = EEPROM.read(0);
  for (int k = 1, i = 0; i < 100; i++)
  {
    for (int j = 0; j < 5; j++)
    {
      //Serial.print(EEPROM.read(k));
      host[i][j] = EEPROM.read(k);
      k++;
    }
  }
  return host;
}
void clear(){
	if (digitalRead(Button_clear)) {
		digitalWrite(WriteLight, 0);
  	for (int i = 1; i < EEPROM.read(0)+5; i++)
  	{
  		EEPROM.write(i,0);
  	}
    EEPROM.write(0,1);
  	for (int y = 0; y < 80; y++) {
      digitalWrite(BUZZER, HIGH);
      delay(3);
      digitalWrite(BUZZER, LOW); 

      delay(3);
    }
    delay(3000);
    digitalWrite(WriteLight, 1);
    for (int y = 0; y < 80; y++) {
      digitalWrite(BUZZER, HIGH);
      delay(4);
      digitalWrite(BUZZER, LOW);
      delay(4);
    }
	}
}