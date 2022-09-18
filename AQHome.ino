#include <M5Stack.h>
#include "Wire.h"
#include "DHT.h"
#include "Adafruit_PM25AQI.h"
#include "Adafruit_CCS811.h"

#define SMALL 2
#define BIG 4

// defines - pins
#define DHT_PIN 3
#define DHTTYPE DHT11

typedef struct airdata {
  float temp; //temperature, Celsius
  float hum; // relative humidity %
  uint16_t co2; // Co2 equivalent (ppm)
  uint16_t pm10; // PM1
  uint16_t pm25; //PM2.5
  uint16_t pm100;
} airdata;

// initialize objects
DHT dht(DHT_PIN, DHTTYPE);
Adafruit_PM25AQI aqi = Adafruit_PM25AQI();
Adafruit_CCS811 ccs;

TFT_eSprite spr = TFT_eSprite(&M5.Lcd);


void setup() {

  Serial.begin(112500);
  M5.begin();
  M5.Power.begin();

  M5.Lcd.setTextSize(2);
  M5.Lcd.print("Hello World");

  // init dht11 sensor
  dht.begin();
  Serial.println("DHT11 test");

  // init ccs811 sensor
  if (!ccs.begin(0x5B)) {  // alt addres 0x5A
    Serial.println("failed to init ccs811");
  }

  // Wait one second for PM2.5 sensor to boot up!
  delay(1000);
  Serial2.begin(9600);  //  hookup TX of sensor to RX2

  // attempt to connect oto PM2.5 sensor
  if (!aqi.begin_UART(&Serial2)) {  // connect to the sensor over hardware serial
    Serial.println("Could not find PM 2.5 sensor!");
    while (1) delay(10);
  }

  // Create a sprite of defined size
  spr.createSprite(300, 180);
}

void loop() {
  airdata data;

  if (update_data(&data)) {
    Serial.println("data updated");
  }

  display_data(&data);

  delay(1000);
}


void display_data(airdata* data) {


  spr.fillSprite(TFT_BLACK);

  spr.drawString("TEMP", 40, 10, SMALL);
  spr.drawString(String(data->temp, 1), 30, 30, BIG);

  spr.drawString("HUM", 120, 10, SMALL);
  spr.drawString(String(data->hum, 1), 120, 30, BIG);

  spr.drawString("CO2", 200, 10, SMALL);
  spr.drawString(String(data->co2, 1), 200, 30, BIG);

  spr.drawString("PM1.0", 30, 100, SMALL);
  spr.drawString(String(data->pm10), 30, 120, BIG);

  spr.drawString("PM2.5", 120, 100, SMALL);
  spr.drawString(String(data->pm25), 120, 120, BIG);

  spr.drawString("PM10", 200, 100, SMALL);
  spr.drawString(String(data->pm100), 200, 120, BIG);


  spr.pushSprite(0, 0);
}

bool update_data(airdata* data_struct) {

  bool res = true;
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    res &= false;
  } else {
    data_struct->temp = t;
    data_struct->hum = h;
  }

  PM25_AQI_Data data;

  if (aqi.read(&data)) {
    data_struct->pm10 = data.pm10_standard;
    data_struct->pm25 = data.pm25_standard;
    data_struct->pm100 = data.pm100_standard;
  } else {
    res &= false;
  }

  if (ccs.available()) {
    if (!ccs.readData()) {
      data_struct->co2 = ccs.geteCO2();
    } else {
      res &= false;
    }
  }
  else {
    res &= false;
  }

  return res;
}