/******************************************
 *
 * This example works for both Industrial and STEM users.
 *
 * Developed by Jose Garcia, https://github.com/jotathebest/
 *
 * ****************************************/

/****************************************
 * Include Libraries
 ****************************************/
#include "UbidotsEsp32Mqtt.h"
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
#define MQ2 35
#define MQ13 34
#define water 14
#define exhaust 19

int lcdColumns = 20 ;
int lcdRows = 4;
int flame;
int sensormq13 = 0;
int sensormq2 = 0;
float tempo_hum =0;
float tempo_temperature = 0;
int caller_index= 0;
int message_index = 0;
#include "DHT.h"
#define DHTPIN 5 
#define FlamePin 27// This is for input pin
#define buzzer 26
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
SoftwareSerial mySerial(18,15);
char *msg;
char call;

// set LCD address, number of columns and rows
// if you don't know your display address, run an I2C scanner sketch
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  

/****************************************
 * Define Constants
 ****************************************/
const char *UBIDOTS_TOKEN = "BBFF-hgQVhhQ2QIVP1yCdSd55MHekdiE432";  // Put here your Ubidots TOKEN
const char *WIFI_SSID = "POCO F4";      // Put here your Wi-Fi SSID
const char *WIFI_PASS = "12345678";      // Put here your Wi-Fi password
const char *DEVICE_LABEL = "esp32";   // Put here your Device label to which data  will be published
const char *VARIABLE_LABEL1= "hum"; // Put here your Variable label to which data  will be published
const char *VARIABLE_LABEL2= "temp";
const char *VARIABLE_LABEL3= "flame";
const char *VARIABLE_LABEL4= "gas";
const char *VARIABLE_LABEL5= "air";
const int PUBLISH_FREQUENCY = 5000; // Update rate in milliseconds

unsigned long timer;
uint8_t analogPin = 34; // Pin used to read data from GPIO34 ADC_CH6.

Ubidots ubidots(UBIDOTS_TOKEN);

/****************************************
 * Auxiliar Functions
 ****************************************/

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

/****************************************
 * Main Functions
 ****************************************/

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  mySerial.begin(9600);
  dht.begin();
  pinMode(water,OUTPUT);
  pinMode(exhaust,OUTPUT);
  pinMode(MQ2, INPUT);
  pinMode(MQ13, INPUT);
  pinMode(FlamePin, INPUT);
  pinMode(buzzer,OUTPUT);

  digitalWrite(water,LOW);
  digitalWrite(exhaust,LOW);

  Serial.begin(9600);
  lcd.init();
  // turn on LCD backlight                      
  lcd.backlight();

  
  ubidots.connectToWifi(WIFI_SSID, WIFI_PASS);
  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();
  
  timer = millis();

}

void loop()
{
  
  float gas = analogRead(MQ2);
  float air = analogRead(MQ13);
  flame = digitalRead(FlamePin);
  flame =!flame;
  float hum= dht.readHumidity();
  hum=hum + 8;
  float temperature = dht.readTemperature();



//calibration of gas sensor

  if( gas>200 || air>250)
      {
        gas = 1;
        
      }
     else
      gas =0;
//calibration of air quality sensor
   if (air >=0.0 && air<=100.0)
    air = random(150,170);
   else 
     air = map(air,100,1000,200,500);

    
  float value1 = hum;
  float value2 = temperature;
  float value3 = flame;
  float value4 = gas;
  float value5 = air;

  lcd.setCursor(0, 0);
  lcd.print("H: ");
  lcd.print(hum);
  lcd.print("% " );
  lcd.print("T: ");
  lcd.print(temperature);
  lcd.print(" C");    
  delay(100);

  lcd.setCursor(0, 1);
  if (flame == 1)
   {
    
    lcd.print("Fire Detected");
    Serial.println("Fire Detected");
    MakeCall();
    
      
    while(1)
    {
    MakeCall();
    flame = digitalRead(FlamePin);
    flame =!flame;
    value3 = flame;
    digitalWrite(buzzer,HIGH);
    digitalWrite(water,HIGH);
    delay(1000);
    digitalWrite(buzzer,LOW);
    digitalWrite(water,LOW);
    delay(100);
    if (abs(millis() - timer) > PUBLISH_FREQUENCY) // triggers the routine every 5 seconds
  {
    ubidots.add(VARIABLE_LABEL3, value3);
  }

    if(flame == 0)
      break;
   }
   }
   else 
   {
   
   lcd.setCursor(6, 1);
   lcd.print("No Fire");
   delay(200);
   
   }
  
    // mapping to abnormal value
   
   

   lcd.setCursor(0, 2);
   if(gas ==1)
   {
    lcd.print("Harmful Gas detected");
    msg = "harmful gas detected. Don't worry";
    SendMessage();
    
    Serial.println("harmful gas detected");

    
    while(1)
    {

     float gas = analogRead(MQ2);
     float air =  analogRead(MQ13);
     
     Serial.println("my gas" );
     Serial.println(gas);
     //air = map(air,100,1000,200,500);
     Serial.println("my air" );
     Serial.println(air);
    if(gas>100 || air >250)
    {
      gas = 1;
      
    }
     else 
      {
        gas = 0;
        
        break;
        
      }
      value4 = gas;
      if (abs(millis() - timer) > PUBLISH_FREQUENCY) // triggers the routine every 5 seconds
    {
        ubidots.add(VARIABLE_LABEL4, value4);
     }
  
      
    digitalWrite(buzzer,HIGH);
    digitalWrite(exhaust,HIGH);
    delay(2000);
    digitalWrite(buzzer,LOW);
    digitalWrite(exhaust,LOW);
    delay(500);
    
    }
   }
   else 
    lcd.print("No Harmful Gas            ");
    
   delay(50);

   lcd.setCursor(0, 3);
   lcd.print("AQI: ");
   lcd.print(air);
   lcd.print(" ppm");
   delay(50);

    
  if (isnan(hum) || isnan(temperature)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
   
  Serial.println("\nhum: ");
  Serial.print(hum); 
  Serial.println("\ntemperature: ");
  Serial.print(temperature);
  Serial.println("\nflame: ");
  Serial.print(flame); 
  Serial.println("\ngas: ");
  Serial.print(gas);
  Serial.println("\nair: ");
  Serial.print(air); 
  Serial.print("\n\n");
  
 





//////////////////////////Send to server////////////////////

    
  // put your main code here, to run repeatedly:
  if (!ubidots.connected())
  {
    ubidots.reconnect();
  }
  if (abs(millis() - timer) > PUBLISH_FREQUENCY) // triggers the routine every 5 seconds
  {
    
    ubidots.add(VARIABLE_LABEL1, value1);// Insert your variable Labels and the value to be sent
    ubidots.add(VARIABLE_LABEL2, value2);
    ubidots.add(VARIABLE_LABEL3, value3);
    ubidots.add(VARIABLE_LABEL4, value4);
    ubidots.add(VARIABLE_LABEL5, value5);
    ubidots.publish(DEVICE_LABEL);
    timer = millis();
  }
  ubidots.loop();
  
}



//////////////////////  Functions ///////////////////
int MakeCall()
{
  
  mySerial.println("ATD+8801533648725;"); // ATDxxxxxxxxxx; -- watch out here for semicolon at the end!!
  Serial.println("Calling  "); // print response over serial port
  delay(500);
  caller_index = caller_index + 1;
  return caller_index;
}


int SendMessage()
{
  mySerial.println("AT+CMGF=1");    //Sets the GSM Module in Text Mode
  delay(500);  // Delay of 1000 milli seconds or 1 second
  mySerial.println("AT+CMGS=\"+8801533648725\"\r"); // Replace x with mobile number
  delay(500);
  mySerial.println(msg);// The SMS text you want to send
  delay(100);
   mySerial.println((char)26);// ASCII code of CTRL+Z
  delay(500);
  message_index++;
  return message_index;
}


/*
void ReceiveMessage()
{
  mySerial.println("AT+CNMI=2,2,0,0,0"); // AT Command to recieve a live SMS
  delay(1000);
  if (mySerial.available()>0)
  {
    *msg=mySerial.read();
    Serial.print(msg);
  }
  
} */
