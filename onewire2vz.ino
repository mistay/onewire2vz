
#include <EtherCard.h>
#define CS_PIN 10

#include <EEPROM.h>
#include <TrueRandom.h>
//#include "Timer.h"
//Timer t;


#include <OneWire.h>
OneWire ds(8);  // an pin 10
#define MAX_DS1820_SENSORS 12
byte addr[MAX_DS1820_SENSORS][8];
  
  
  
#include <avr/wdt.h>

// Universally administered and locally administered addresses are distinguished by setting the second least significant bit of the most significant byte of the address. If the bit is 0, the address is universally administered. If it is 1, the address is locally administered. In the example address 02-00-00-00-00-01 the most significant byte is 02h. The binary is 00000010 and the second least significant bit is 1. Therefore, it is a locally administered address.[3] The bit is 0 in all OUIs.
// so mac should be:
// x2-xx-xx-xx-xx-xx
// x6-xx-xx-xx-xx-xx
// xA-xx-xx-xx-xx-xx
// xE-xx-xx-xx-xx-xx
static byte mymac[] = { 0x06,0x02,0x03,0x04,0x05,0x06 };
char macstr[18];

byte Ethernet::buffer[700];
static uint32_t timer = 0;

const char website[] PROGMEM = "vz.langhofer.net";

int LED_LAN_RDY =2;
int LED_LAN_ROOCESSING =3;


int ResetGPIO = A3;


bool requestPending=false;
int heartbeatreset = 0;

/*
void displaystuff(char* releasedetails, char* ipaddress, char* macaddress)   {                
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  // init done
  
  // Clear the buffer.
  display.clearDisplay();


  // text display tests
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println(releasedetails);
  display.println(ipaddress);
  display.setTextSize(2);
  display.println(macaddress);
  display.display();
  delay(2000);
}
*/


static void my_callback (byte status, word off, word len) {
  requestPending=false;
  
  Serial.println(">>>");
  Ethernet::buffer[off+300] = 0;
  Serial.print((const char*) Ethernet::buffer + off);
  Serial.println("...");
  //wdt_reset();
}

uint16_t values[10];
uint8_t debounce[10];

String a,c;

int heartbeat=0;


void(* resetFunc) (void) = 0;

int HighByte, LowByte, TReading, SignBit, Tc_100, Whole, Fract;

void setup () {
  
  Serial.begin(115200);
  Serial.println(F("\nonewire2vz v20170107 starting..."));
  
  
  
  
  //displaystuff("s0vz v2 build 1702", "192.168.2.3", "13:22:23:12:23:42");
 
  
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
  pinMode(8, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);
  
  pinMode(A0, INPUT_PULLUP);
  pinMode(A1, INPUT_PULLUP);
  pinMode(A2, INPUT_PULLUP);
  pinMode(A3, INPUT_PULLUP);
  

  pinMode(LED_LAN_RDY, OUTPUT);
  pinMode(LED_LAN_ROOCESSING, OUTPUT);
  
  // LED test
  delay(200);
  digitalWrite(LED_LAN_RDY, HIGH);
  delay(200);
  digitalWrite(LED_LAN_RDY, LOW);
  digitalWrite(LED_LAN_ROOCESSING, HIGH);
  delay(200);
  digitalWrite(LED_LAN_ROOCESSING, LOW);
  digitalWrite(LED_LAN_RDY, LOW);
  
  
  
  
  
  
  if (EEPROM.read(1) != '#') {
    Serial.println(F("\nWriting EEProm..."));
  
    EEPROM.write(1, '#');
    
    for (int i = 3; i < 6; i++) {
      EEPROM.write(i, TrueRandom.randomByte());
    }
  }
  
  // read 3 last MAC octets
  for (int i = 3; i < 6; i++) {
      mymac[i] = EEPROM.read(i);
  }
  
Serial.print("MAC: ");
  for (int i = 0; i < 6; i++) {
    Serial.print(mymac[i], HEX);
        if (i<5)
          Serial.print(':');
    }
  Serial.println();
  
  Serial.print("Access Ethernet Controller... ");
  if (ether.begin(sizeof Ethernet::buffer, mymac, CS_PIN) == 0) {
    Serial.println(F("FAILED"));
  } else {
    Serial.println("DONE");
  }
  
  
  Serial.print("Requesting IP from DHCP Server... ");
  if (!ether.dhcpSetup()) {
    Serial.println(F("FAILED"));
  } else {
    Serial.println("DONE");
  }
  
  
  ether.printIp("IP:  ", ether.myip);
  ether.printIp("GW:  ", ether.gwip);  
  ether.printIp("DNS: ", ether.dnsip);  

  if (!ether.dnsLookup(website))
    Serial.println("DNS failed");
    
  ether.printIp("SRV: ", ether.hisip);
  
 
  
  heartbeat=0;
  
  a = "?mac=" + String(mymac[0], HEX) + ":" + String(mymac[1], HEX) + ":" + String(mymac[2], HEX) + 
  ":" + String(mymac[3], HEX) + ":" +String( mymac[4], HEX) + ":" + String(mymac[5], HEX);
  
  digitalWrite(LED_LAN_RDY, HIGH);
  // t.every(1000, takeReading); 

}

bool foo;
void blinkLED() {
  foo=!foo;
  if (foo)
    digitalWrite(LED_LAN_RDY,HIGH);
  else
    digitalWrite(LED_LAN_RDY,LOW);
  
}




int MAX_HEARTBEAT=200;


char b[150];
String d;
void loop () {
  
  buildsensorstring();
  
  
  ether.packetLoop(ether.packetReceive());
  
  if (millis() > timer) {
    timer = millis() + 10000;
    Serial.println();
    Serial.print("<<< REQ ");
    
    
     
     d.toCharArray(b, sizeof(b));



    ether.browseUrl(PSTR("/ow2vz.php?"), b, website, my_callback);
    
    Serial.print("/ow2vz.php?");
    Serial.println(d);
  }
}

void buildsensorstring() {
  
  byte i;
  byte present = 0;
  byte data[12];
  byte addr[MAX_DS1820_SENSORS][8];
  int sensor=0;
  
  d = "";
  for (sensor=0;sensor<MAX_DS1820_SENSORS;sensor++) {
    
  
    if ( !ds.search(addr[sensor])) {
      Serial.print("Keine weiteren Addressen bei Sensor");
      Serial.println (sensor); //\n");
      ds.reset_search();
      break;
    }
     
    if ( OneWire::crc8( addr[sensor], 7) != addr[sensor][7]) {
      Serial.print("CRC nicht gültig!\n");
      //return;
    }
     
    if ( addr[sensor][0] == 0x10) {
      //Serial.print("Gerät ist aus der DS18S20 Familie.\n");
      
    }
    else if ( addr[sensor][0] == 0x28) {
      //Serial.print("Gerät ist aus der D18s20S18B20 Familie.\n");
    }
    else {
      Serial.print("Gerätefamilie nicht erkannt : 0x");
      Serial.println(addr[sensor][0],HEX);
      //return;
    }
     
    ds.reset();
    ds.select(addr[sensor]);
    ds.write(0x44,1);         // start Konvertierung, mit power-on am Ende
     
    //delay(50);     // 750ms sollten ausreichen
    
    ether.packetLoop(ether.packetReceive());
    delay(750);
    ether.packetLoop(ether.packetReceive());
    
    
    // man sollte ein ds.depower() hier machen, aber ein reset tut das auch
     
    present = ds.reset();
    ds.select(addr[sensor]);    
    ds.write(0xBE);         // Wert lesen
     
     
    d += (sensor);
   
    d += "=";
    //d += ("");
    //for( i = 1; i < 4; i++)
     //{
       //if (addr[sensor][i]<0xff)
       //  d +="0";
         
       //d += String(addr[sensor][i], HEX);
       //Serial.print("e:");
       //Serial.println(addr[sensor][i], HEX);
       //Serial.print(addr[sensor][i], HEX);
       
       //if (i<6) d += ("-");
     //}

     //d += "=";
    
    Serial.print(" ");
    for ( i = 0; i < 9; i++) {           // 9 bytes
      data[i] = ds.read();
      Serial.print(data[i], HEX);
    }
    Serial.println();

    LowByte = data[0];
    HighByte = data[1];
    TReading = (HighByte << 8) + LowByte;
    SignBit = TReading & 0x8000;  // test most sig bit
    if (SignBit) // negative
    {
      TReading = (TReading ^ 0xffff) + 1; // 2's comp
    }
    
    Tc_100 = (6 * TReading) + TReading / 4;    // mal (100 * 0.0625) oder 6.25
  
    /* Für DS18S20 folgendes verwenden Tc_100 = (TReading*100/2);    */
  
    //Whole = Tc_100 / 100;  // Ganzzahlen und Brüche trennen
    //Fract = Tc_100 % 100;
    
    if (SignBit) // negative Werte ermitteln
    {
      d += "-";
    }
    d += Tc_100;
    
    d +="&";
    
    //d += (Whole);
    //d += ".";
  
    //if (Fract < 10)
    //{
     //d += "0";
    //}
    
    
    //d += Fract;
    
    //d += "&";
  }
  
  //Serial.println("debug");
  //Serial.println(d);
  //return d;
}

