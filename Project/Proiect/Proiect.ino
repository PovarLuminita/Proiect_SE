#include <AM2302-Sensor.h>
#include <LiquidCrystal_I2C.h>


constexpr unsigned int SENSOR_PIN {4U}; //D4 - data
// declaration of the sensor variable
AM2302::AM2302_Sensor am2302{SENSOR_PIN};
// declaration of the LCD variable
LiquidCrystal_I2C lcd (0x27, 16,2);

void setup() {
   // Initialize the LCD connected 
   lcd.init();
   // Turn on the backlight on LCD. 
   lcd.backlight();
   lcd.setCursor(0,0);
  // segin Serial print
   Serial.begin(115200);

   while (!Serial) {
      yield();
   }

   // set pin and check for sensor
   if (am2302.begin()) {
      // this delay is needed to receive valid data,
      // when the loop directly read again
      delay(3000);
   }
   
}

void loop() {
   auto status = am2302.read();
   // put your main code here, to run repeatedly:
   Serial.print("Temperature: ");
   Serial.println(am2302.get_Temperature());

   Serial.print("Humidity:    ");
   Serial.println(am2302.get_Humidity());
   lcd.clear();
   lcd.setCursor(0, 0);
   lcd.print("Hum: ");
   lcd.print(am2302.get_Humidity());
   lcd.setCursor (0, 1);
   lcd.print("Temp: ");
   lcd.print(am2302.get_Temperature());
   delay(2000);
}