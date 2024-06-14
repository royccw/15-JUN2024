#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"

// Which pin on the Arduino is connected to the NeoPixels?
#define LED_PIN 3 // On Trinket or Gemma, suggest changing this to 1
// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 8 // Popular NeoPixel ring size
#define DELAYVAL 500 // Time (in milliseconds) to pause between pixels
Adafruit_NeoPixel pixels(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
//Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800);
#define BRIGHTNESS  10 //0~255

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define BUZZER  10
#define DHTPIN  9     // Digital pin connected to the DHT sensor
#define WATER_PIN A3
#define IR_PIN  A2

#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);
float Humi, temp, initTemp = 0;
char chBuffer[30];
char floatString[10];

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

char message[]="Welcome ";
int8_t x, minX;

bool oled_display = false;
bool scroll_repeat = false;

void setup() {

  pinMode(BUZZER, OUTPUT);
  Serial.begin(115200);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    //for(;;); // Don't proceed, loop forever
  }

  dht.begin();
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.setBrightness(BRIGHTNESS);
  pixels.clear(); // Set all pixel colors to 'off'
  pixels.show();

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.clearDisplay();
  //display.display();
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.setCursor(20,15);
  display.print("HELLO");
  display.display();
  delay(4000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setTextWrap(true);
  x = display.width();
  minX = -12 * strlen(message);  // 12 = 6 pixels/character * text size 2
  Serial.print("minX=");
  Serial.println(minX);
  display.display();

  delay(1000);
  initTemp = dht.readTemperature();
  Humi = dht.readHumidity();
  Serial.print("Initial Temp=");
  Serial.println(initTemp);

}

void OLED_message(void) {

//  Serial.println(x);
  display.clearDisplay();//Clear buffer and screen
  display.setTextColor(WHITE);//Use white pixels
  display.setTextSize(2);//Set 3 times normal text size
  display.setTextWrap(false);//Turn text wrapping off
  display.setCursor(x, 16);//Position cursor
  display.print(message);//Scrolling message

  display.display();//Send the above to the display
  x = x - 10;//scroll speed faster with larger value  
//  if (x < minX) x = display.width();//If text length less than 128 pixels, use screen size as length to base scrolling maths on
  if (x < minX){
    oled_display = false;

  }
}

uint8_t  Waitcount=0;
uint8_t BUZZER_wait=0,BUZZER_repeat=0;

float t_diff,lasttemp=0;

void loop() {

//  if (digitalRead(IR_PIN) == LOW){
  if (analogRead(A2) < 255){
    Waitcount++;
  if (oled_display == false ) {
      oled_display = true;
      x = display.width();
    }
  }else{
    Waitcount = 0;
  }
  if (Waitcount >= 33) {// @ 5second
    Waitcount = 0;
  }
  if (oled_display == true )  OLED_message();

  if (analogRead(A3) < 255){
    for(int i=0; i<LED_COUNT; i++) { // For each pixel...
        // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
        // Here we're using a moderately bright green color:
        pixels.setPixelColor(i, pixels.Color(255, 0, 0));
        pixels.show();   // Send the updated pixel colors to the hardware.
      }
  }else{
    delay(200);
    pixels.clear();
    pixels.show();   // Send the updated pixel colors to the hardware.
  }

  temp = dht.readTemperature();
  t_diff = temp - initTemp;
  if ((t_diff < 0 ) || (t_diff >= 1.5) ) initTemp = temp;

  if ((t_diff >= 0.8) && !BUZZER_wait && !BUZZER_repeat){
    BUZZER_wait=40;//@8 sec wait
    BUZZER_repeat = 1;
    tone(BUZZER, 1500,300);
  }
  if (BUZZER_wait){
    BUZZER_wait--;
  }else{
    BUZZER_repeat = 0;
  }
  delay(200);
}
