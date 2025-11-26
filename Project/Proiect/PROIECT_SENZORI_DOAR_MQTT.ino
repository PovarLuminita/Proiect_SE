#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h> 

//--------------------------------
#include <AM2302-Sensor.h>
#include <LiquidCrystal_I2C.h>


constexpr unsigned int SENSOR_PIN {4U}; //D4 - data
// declaration of the sensor variable
AM2302::AM2302_Sensor am2302{SENSOR_PIN};
// declaration of the LCD variable
LiquidCrystal_I2C lcd (0x27, 16,2);

///-------------------------------------
const char* ssid = "Lumi25";
const char* password = "25042002";
const char* mqtt_server = "c525a9c992e64a03b77bbd47916513c1.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;  // ← ADĂUGAT
const char* mqtt_user = "admin";  // ← ADĂUGAT
const char* mqtt_password = "Pass123@";  // ← ADĂUGAT


WiFiClientSecure espClient;  // ← SCHIMBAT
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)


//LDR (fotorezistor)
const int LDR_PIN = 34; // GPIO34 (ADC1) - input only

//WATER LEVEL SENSOR 
const int WATER_SENSOR_PIN = 32; // GPIO32 (ADC1)


void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(2, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(2, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";  // ← SCHIMBAT din ESP8266
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("Connected to HiveMQ!");
      // Once connected, publish an announcement...
      client.publish("IoT/mqtt", "IoT");
      // ... and resubscribe
      client.subscribe("IoT/mqtt");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


///------------------------------------------------------------

void setup() {
   // Initialize the LCD connected 
   lcd.init();
   // Turn on the backlight on LCD. 
   lcd.backlight();
   lcd.setCursor(0,0);
  // segin Serial print
   pinMode(2, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
   Serial.begin(115200);
   setup_wifi();
   espClient.setInsecure();  // Skip certificate verification
   client.setServer(mqtt_server, 8883);
   client.setCallback(callback);

   while (!Serial) {
      yield();
   }

      // ADC setup for LDR
   analogReadResolution(12); // 0..4095
   analogSetPinAttenuation(LDR_PIN, ADC_11db); // citire pana la ~3.3V
   analogSetPinAttenuation(WATER_SENSOR_PIN, ADC_11db); // read up to ~3.3V
   // set pin and check for sensor
   if (am2302.begin()) {
      // this delay is needed to receive valid data,
      // when the loop directly read again
      delay(3000);
   }
   
}

void loop() {
   lcd.clear();
   
 if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    auto status = am2302.read();
    float temperature = am2302.get_Temperature();
    String temp = String(temperature, 2);
    Serial.print("Temperature: ");
    Serial.println(temp);
    client.publish("IoT-Temp", temp.c_str());
    
    float humidity = am2302.get_Humidity();
    String hum = String(humidity, 2);  
    Serial.print("Humidity: ");
    Serial.println(hum);
    client.publish("IoT-Hum", hum.c_str());

 // --- LDR read & print (doar pe Serial acum) ---
    int ldrRaw = analogRead(LDR_PIN); // 0..4095
    float ldrPct = (ldrRaw / 4095.0) * 100.0;
    String ldr = String(ldrPct, 2);
    Serial.print("LDR raw: ");
    Serial.print(ldrRaw);
    Serial.print("  (%) ");
    Serial.println(ldrPct, 1);
    client.publish("IoT-LDR", ldr.c_str());


    int waterRaw = analogRead(WATER_SENSOR_PIN); // 0..4095
    float waterPct = (waterRaw / 4095.0f) * 100.0f; // percent (0..100)
    String wl = String(waterPct, 2);
    Serial.print("WATER raw: ");
    Serial.print(waterRaw);
    Serial.print("  (%) ");
    Serial.println(waterPct, 1);
     client.publish("IoT-WL", wl.c_str());

  }
  lcd.setCursor(0, 0);
  lcd.print("T:"); 
  lcd.print(am2302.get_Temperature(),1);

  lcd.print("   ");                 // clear leftover digits if any
  lcd.setCursor(8, 0);              // column 8 to place H:
  lcd.print("H:");
  lcd.print(am2302.get_Humidity());
  

  // Line 1: LDR and Water
    lcd.setCursor(0, 1);
    lcd.print("L:");
    float ldrPct = (analogRead(LDR_PIN) / 4095.0) * 100.0;
    lcd.print(ldrPct, 1);

    lcd.print("   ");
    lcd.setCursor(8, 1);
    lcd.print("W:");
    float waterPct = (analogRead(WATER_SENSOR_PIN) / 4095.0f) * 100.0f; // percent (0..100)
    lcd.print(waterPct, 1);
    lcd.print("   ");

  delay(200);
}