#define ARDUINO_RUNNING_CORE 1

#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID ""
#define BLYNK_DEVICE_NAME ""
#define BLYNK_AUTH_TOKEN ""

#include "Arduino.h"
#include "config_pin.h"
#include "DHT.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>


int auto1 = 0;
float t = 0, h = 0;

char auth[] = "";
char ssid[] = "";
char pass[] = "";



DHT dht(DHTPIN, DHTTYPE);


void setup() {
  Serial.begin(115200);
  

  xTaskCreatePinnedToCore(Task_DHT11, "Task_DHT11", 8000, NULL, 2, NULL, ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(Task_Control, "Task_Control", 8000, NULL, 1, NULL, ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(Task_Blynk_Loop, "loop1", 4096, NULL, 1, NULL, ARDUINO_RUNNING_CORE);
  
  Serial.println("");
  Serial.println("Connected and Ready - Waiting for Button Press\n");
}

void Task_Blynk_Loop(void * pvParameters){
  (void)pvParameters;
  
  Blynk.begin(auth, ssid, pass);  
  while (Blynk.connected() == false) {
  }
  Serial.println();
  Serial.println("Blynk Connected");
  Serial.println("Blynk Loop Task Started");
  while (1) {
     Blynk.run();
     delay(1);
  }
}

void Task_DHT11(void *pvParameters)
{
    (void)pvParameters;

    dht.begin();

    while (1) // A Task shall never return or exit.
    {
        h = dht.readHumidity();
        t = dht.readTemperature();
        if (isnan(h) || isnan(t))
        {
            Serial.println(F("Failed to read from DHT sensor!"));
        }
        else
        {
            Serial.print(F("Humidity: "));
            Serial.print(h);
            Serial.print(F("%  Temperature: "));
            Serial.print(t);
            Serial.println(F("°C "));
            if(Blynk.connected() == true)
            {
              Blynk.virtualWrite(V0, int(t));
              Blynk.virtualWrite(V1, int(h));
            }
        }

        vTaskDelay(3000);
    }
}

void Task_Control(void *pvParameters)
{
    (void)pvParameters;

    pinMode(LED1_PIN, OUTPUT);
    pinMode(LED2_PIN, OUTPUT);
    pinMode(QUAT_PIN, OUTPUT);
    pinMode(MAYBOM_PIN, OUTPUT);
    pinMode(CD_PIN, INPUT);
    pinMode(AS_PIN, INPUT);
    pinMode(DOAM_PIN, INPUT);
    
    digitalWrite(QUAT_PIN, 1);
    digitalWrite(LED2_PIN, 1);
    digitalWrite(LED1_PIN, 1);
    digitalWrite(MAYBOM_PIN, 0);

    while (1)
    { 
        
        int s = analogRead(DOAM_PIN);
        int soil = 100 - map(s,0, 4095, 0, 100);
        Blynk.virtualWrite(V2, soil);

        int p = digitalRead(AS_PIN);
        int a = digitalRead(CD_PIN);
        Blynk.virtualWrite(V3, a);
        
        if(auto1 == 1)
        {  
          
          if(t > 32 )         digitalWrite(QUAT_PIN, 0);
          else                digitalWrite(QUAT_PIN, 1);
          Blynk.virtualWrite(V5, t > 32);
          
          if(soil < 30)       digitalWrite(MAYBOM_PIN, 1);
          else                digitalWrite(MAYBOM_PIN, 0);
          Blynk.virtualWrite(V8, soil < 30);
          
          if(p == 1)          digitalWrite(LED1_PIN, 0);
          else                digitalWrite(LED1_PIN, 1);
          Blynk.virtualWrite(V6, p == 1);
          
          if(a == 1)          digitalWrite(LED2_PIN, 0);
          else if(a == 0)     digitalWrite(LED2_PIN, 1);
          Blynk.virtualWrite(V7, a == 1);
        }
        vTaskDelay(2000);
    }
}

BLYNK_WRITE(V5)
{
  int b = param.asInt();
  Serial.println(b);
  if(auto1 == 0)
  {
    if(b == 1)digitalWrite(QUAT_PIN, 0);
    else digitalWrite(QUAT_PIN, 1); 
  }
  delay(200);  
}
BLYNK_WRITE(V4)
{
  auto1 = param.asInt();
  Serial.println(auto1);
  delay(200);
}


BLYNK_WRITE(V6)
{
  int b = param.asInt();
  if(auto1 == 0)
  {
  if(b == 1)digitalWrite(LED1_PIN, 0);
  else digitalWrite(LED1_PIN, 1);
  } 
}
BLYNK_WRITE(V7)
{
  int b = param.asInt();
  if(auto1 == 0)
  {
  if(b == 1)digitalWrite(LED2_PIN, 0);
  else digitalWrite(LED2_PIN, 1); 
  }
}
BLYNK_WRITE(V8)
{
  int b = param.asInt();
  if(auto1 == 0)
  {
      digitalWrite(MAYBOM_PIN, b);
  }
}

void loop() {
}
