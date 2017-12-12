/*********************************************************************
  This is an example for our Monochrome OLEDs based on SSD1306 drivers
  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/category/63_98
  This example is for a 128x64 size display using SPI to communicate
  4 or 5 pins are required to interface
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!
  Written by Limor Fried/Ladyada  for Adafruit Industries.
  BSD license, check license.txt for more information
  All text above, and the splash screen must be included in any redistribution



  Average: 324
  High: 476
  Low: 223

  rZero: 250.00

*********************************************************************/
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MQ135.h"
#define ANALOGPIN A1
MQ135 gasSensor = MQ135(ANALOGPIN);

// If using software SPI (the default case):
#define OLED_MOSI   9
#define OLED_CLK   10
#define OLED_DC    11
#define OLED_CS    12
#define OLED_RESET 13

#define BUZZER_PIN A4 

#define BUTTON_PIN 21
const int  buttonPin = 21;

int volume = 100;
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
/* Uncomment this block to use hardware SPI
  #define OLED_DC     6
  #define OLED_CS     7
  #define OLED_RESET  8
  Adafruit_SSD1306 display(OLED_DC, OLED_RESET, OLED_CS);
*/
#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2
#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16
#define LED 11
int reading;
int i;
int high;
int low = 800;
float secAvg;
int secAvgi;
float secAvgT;
int fiveSecAvg;
int fiveSecAvgi;
int fiveSecAvgT;
int totalAvg;
int totalAvgi;
int totalAvgT;
float ppm;
float rZero;
float rZeroAvg;
int rAvgCount;
float rZeroT;
int secPassed;

int displayState = 0;
int maxState = 4;
int buttonState = 0;
int cd = 1200;
int alertPPM = 2000;
int lowestCo2 = 2500;
int perfectCo2 = 200;

int lowestVOC = 1023;
int perfectVOC = 23;

float co21sec;
float co25sec;
float co21secT;
float co25secT;
int co2count;
int co25count;
static const unsigned char PROGMEM logo16_glcd_bmp[] =
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000
};
#define SSD1306_LCDHEIGHT 64
#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif
void setup()   {
  Serial.begin(9600);
  Serial.println("Begin Setup");
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC);
  // init done

  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10, 0);
  display.clearDisplay();
  display.println("");
  display.display();

  Serial.begin(115200);
  Serial.println("MiCS-5524 demo!");

  pinMode(LED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(buttonPin, INPUT);
  digitalWrite(buttonPin, HIGH);
  i = 0;
  //
  //  display.println("Hello?");
  //  display.display();

  splashScreen();
  delay(5000);

  Serial.write("Done Setup");

}
int j = 0;
int bFlag = 0;
void loop() {

  reading = analogRead(A0);

  buttonState = digitalRead(buttonPin);
  Serial.println(buttonState);
  if(buttonState == LOW) {
    Serial.println("Button - LOW");
    bFlag = 1;
  } else if(buttonState == HIGH) {
    Serial.println("Button - HIGH");
    if(bFlag == 1) {
      buttonPressed();
      bFlag = 0;
    }
  }

  ppm = gasSensor.getPPM();
  rZero = gasSensor.getRZero(); //Getting rZero of our sensor,  comment out after calibrating


  analogWrite(LED, reading);
  delay(100);
  maths();
  //averagesReadings();
  displayType();


  if (j >= 10) {
    digitalWrite(13, HIGH);
    Serial.print("CO2 ppm value : ");
    Serial.println(ppm);
    j = 0;
    secPassed++;
  }

  j++;

  //Comment out the following after rZero Obtained
  //if(secPassed >= 1800) { //Start Measuring rZero after 30 min
  addZeroAvg();
  //}

}

void buttonPressed() {
  Serial.write("Butt on");
  displayState++;
  if(displayState > maxState) {
    displayState = 0;
  }
}

void displayType() {

  if(co21sec >= alertPPM) {
    analogWrite(BUZZER_PIN, volume);
  } else {
    analogWrite(BUZZER_PIN, 0);
  }
  
  if(displayState == 0) {
    displayOne();
  } else if(displayState == 1) {
    displayTwo();
    displayState++;
  } else if(displayState ==2) {
    displayThree();
  } else if(displayState == 3) {
    displayFour();
  } else if(displayState == 4) {
    displayFive();
  } else {
    displayTwo();
  }
}

int k = 0;
void maths() {

  co2count++;
  co25count++;
  co25secT += ppm;
  co21secT += ppm;
  if (co2count >= 5) {
    co21sec = (co21secT / co2count);
    co21secT = 0;
    co2count = 0;
  }
  if(co25count >= 25) {
    co25sec = (co25secT / co25count);
    co25secT = 0;
    co25count = 0;
  }
  

  secAvgi++;
  secAvgT += reading;
  if (secAvgi > 10) {
    secAvg = (secAvgT / secAvgi);
    secAvgi = 0;
    secAvgT = 0;


    if (secAvg > high) {
      high = secAvg;
    }
    if (secAvg < low) {
      if (low > 0) {
        low = secAvg;
      }
    }
  }

  fiveSecAvgi++;
  fiveSecAvgT += reading;
  if (fiveSecAvgi > 50) {
    fiveSecAvg = (fiveSecAvgT / fiveSecAvgi);
    fiveSecAvgi = 0;
    fiveSecAvgT = 0;
  }

  totalAvgi++;
  totalAvgT += reading;

  i++;

  displayType();
  //averagesReadings();
  //displayReading();
  //delay(1000);
  k++;
  if (k >= 10) {
    displayType();
    //averagesReadings();
    k = 0;
  }
}

void splashScreen() {
  display.setTextSize(4);
  display.setTextColor(WHITE);
  display.setCursor(5, 15);
  display.clearDisplay();

  display.print("M.A.Q.");

  display.setTextSize(1);
  display.setCursor(8, 50);
  display.print("Breathing with You");
  display.setCursor(50, 50);
  display.display();
}

void designCode() {
  cd=1200;
 display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(30, 2);
  display.clearDisplay();

  display.print("M.A.Q.");

//  display.setTextSize(1);
//  display.setCursor(10, 22);
//  display.print("Breathing with You");

//  display.setTextSize(1);
//  display.setCursor(30, 22);
//  display.print("Air Quality");

  display.drawRect(0, 22, display.width() - 2, 16, WHITE);
  float pieFilling = ((1 - ((co21sec - 200) / 2300)) * 126);
  display.fillRect(0, 22, pieFilling, 16, WHITE);

  //200ppm is perfect
  //2500ppm is bad

  //((1 - ((ppm-200) / 2300)) * 126)


  //100 is good
  //1023 is worst
    display.drawRect(0, 44, display.width()-2, 16, WHITE);
    float pieFilling2 = ((1 - ((secAvg-80) / 943)) * 126);
    display.fillRect(0,44, pieFilling2,16,WHITE);


  display.display();
  

  display.display();
}

//
// Vertical Bar with quality level indicators
//
void displayOne() {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.clearDisplay();

  display.drawRect(0, 0, 16, display.height() - 2, WHITE);
  float pieFilling = ((1 - ((co21sec - perfectCo2) / lowestCo2)) * 62);
  float pieFilling2 = ((1 - ((co21sec - perfectVOC) / lowestVOC)) * 62);
  if(pieFilling2 < pieFilling) {
     pieFilling = pieFilling2;
  }
  display.fillRect(0, 62-pieFilling, 16, pieFilling, WHITE);

  display.setCursor(30, 2);
  display.print("Perfect");

  display.setCursor(30, 19);
  display.print("Good");

  display.setCursor(30, 34);
  display.print("Poor");

  display.setCursor(30, 50);
  display.print("Danger");

  display.display();
}

//
// Partial Circles for Co2 and VOCs
//
void displayTwo() {
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.clearDisplay();

  display.setCursor(20,0);
  display.print("CO2");

  display.setCursor(80,0);
  display.print("VOCs");

  int co2P = (1 - ((co21sec - perfectCo2) / lowestCo2));
  int vocP = (1 - ((fiveSecAvg - perfectVOC) / lowestVOC));

  display.setCursor((2*(display.width()/5)) - 5, display.height()/2);
  display.print(co2P);
  display.print("%");

  display.setCursor((5*(display.width()/5)) - 5, display.height()/2);
  display.print(vocP);
  display.print("%");
//
//  display.drawCircle(2*(display.width()/5), display.height()/2, 20, WHITE);
//  //display.drawCircle(4*(display.width()/5), display.height()/2, 40, WHITE);
   drawPieSlice(4*(display.width()/5),  display.height()/2, 20, WHITE, 0, (vocP*360));
   display.drawCircle(4*(display.width()/5),  display.height()/2, 20, WHITE);


  display.display();
}

//
// Simple horizontal bar for CO2
//
void displayThree() {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.clearDisplay();

  display.print("CO2");

  display.setCursor(70, 0);
  display.print(co21sec);
  display.print("PPM");

  display.drawRect(0, 35, display.width() - 2, 16, WHITE);
  float pieFilling = ((1 - ((co21sec - perfectCo2) / lowestCo2)) * 126);
  display.fillRect(0, 35, pieFilling, 16, WHITE);

  display.setTextSize(1);
  display.setCursor(2, 55);
  display.print(lowestCo2);

  display.setCursor(100,55);
  display.print(perfectCo2);

  display.display();
}

//
// Simple horizontal bar for VOCs
//
void displayFour() {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.clearDisplay();

  display.print("VOCs");

  display.setCursor(70, 0);
  display.print(secAvg);
  display.print("PPM");

  display.drawRect(0, 35, display.width() - 2, 16, WHITE);
  float pieFilling = ((1 - ((secAvg - perfectVOC) / lowestVOC)) * 126);
  display.fillRect(0, 35, pieFilling, 16, WHITE);

  display.setTextSize(1);
  display.setCursor(2, 55);
  display.print(lowestVOC);

  display.setCursor(115,55);
  display.print(perfectVOC);

  display.display();
}

void displayFive() {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.clearDisplay();

  display.print("      CO2: ");
  display.print(co21sec);
  display.println("PPM");
  display.print("5 avg CO2: ");
  display.print(co25sec);
  display.println("PPM");
  display.print("      VOC: ");
  display.print(secAvg);
  display.println("PPM");
  display.print("5 avg VOC: ");
  display.print(fiveSecAvg);
  display.println("PPM");
  
  display.display();
}

void realDisplay() {
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(30, 2);
  display.clearDisplay();

  display.print("M.A.Q.");

  display.setTextSize(1);
  display.setCursor(10, 22);
  display.print("Breathing with You");

  display.setCursor(30, 37);
  display.print("Air Quality");

  display.drawRect(0, 48, display.width() - 2, 16, WHITE);
  float pieFilling = ((1 - ((co21sec - 200) / 2300)) * 126);
  display.fillRect(0, 48, pieFilling, 16, WHITE);

  display.display();
}

void simpleDisplay() {
  display.setTextSize(5);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.clearDisplay();

  //display.drawRect(0, 48, display.width() - 2, 16, WHITE);
  //float pieFilling = ((1 - ((co21sec - 200) / 2300)) * 126);
  //display.fillRect(0, 48, pieFilling, 16, WHITE);

  if(co21sec >= 2500) {
   display.println("DEAD");
  } else if(co21sec >= 1500) {
   display.println("BAD");
  } else if(co21sec >= 1000) {
    display.println("Meh");
  } else {
    display.println("GOOD");
  }

  
  display.display();
}

void averagesReadings() {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.clearDisplay();

  display.print("Live PPM: ");
  display.println(reading);

  display.print("1 Sec Avg: ");
  display.println(secAvg);

  display.print("5 Sec Avg: ");
  display.println(fiveSecAvg);

  display.print("Total Avg: ");
  totalAvg = (totalAvgT / totalAvgi);
  display.println(totalAvg);

  display.print("High: ");
  display.println(high);

  display.print("Low: ");
  display.println(low);

  display.print("MQ135: ");
  display.println(ppm);

  display.print("rZero: ");
  display.println(rZeroAvg);

  display.display();

}
void countdown() {
  display.setTextSize(4); //set the size of the text
  display.setTextColor(WHITE); //color setting
  display.setCursor(20, 0); //The string will start at 10,0 (x,y)
  display.clearDisplay(); //Eraser any previous display on the screen
  display.println(); //Print the string here “Circuit Digest”

  if(cd > 600) {
    display.print(cd/600);
    display.print(":");
    display.println((cd%600)/10);
  } else if(cd > 0) {
    display.print(cd/600);
    display.print(":");
    display.println((cd%600)/10);
    if(cd%10 == 0) {
      analogWrite(BUZZER_PIN, volume);
    } else {
      analogWrite(BUZZER_PIN, 0);
    }
  } else {
    display.println("BOOM");
    analogWrite(BUZZER_PIN, volume);
  }


  display.display(); //send the text to the screen

  cd--;
}

void displayReading() {
  display.setTextSize(3); //set the size of the text
  display.setTextColor(WHITE); //color setting
  display.setCursor(0, 0); //The string will start at 10,0 (x,y)
  display.clearDisplay(); //Eraser any previous display on the screen
  display.println(reading); //Print the string here “Circuit Digest”



  display.setTextSize(2);
  display.setCursor(0, 30);
  display.print("High:");
  display.print(high);

  display.setCursor(0, 48);
  display.print("Low:");
  display.print(low);

  display.display(); //send the text to the screen
}

void addZeroAvg() {
  //  if(rAvgCount != 0) {
  //    float total = rZeroAvg * rAvgCount;
  //    total+=rZero;
  //    rAvgCount++;
  //    rZeroAvg = total/rAvgCount;
  //  } else {
  //    rAvgCount = 1;
  //    rZeroAvg = rZero;
  //  }

  rZeroT += rZero;
  rAvgCount++;
  rZeroAvg = rZeroT / rAvgCount;

}

void drawCircles(int x, int y, int radius, int color)
{
  for (int i=0; i<360; i++)  // a bigger radius might need more steps
  {
    double radians = i * PI / 180;
    double px = x + radius * cos(radians);
    double py = y + radius * sin(radians);
    display.drawPixel(px, py, color);
  }
}

void drawPieSlice(int x, int y, int radius, int color, int startAngle, int EndAngle)
{
  for (int i=startAngle; i<EndAngle; i++)
  {
    double radians = i * PI / 180;
    double px = x + radius * cos(radians);
    double py = y + radius * sin(radians);
    display.drawPixel(px, py, color);
  }
}
