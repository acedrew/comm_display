// This is a demonstration on how to use an input device to trigger changes on your neo pixels.
// You should wire a momentary push button to connect from ground to a digital IO pin.  When you
// press the button it will change to a new pixel animation.  Note that you need to press the
// button once to start the first animation!

#include <Adafruit_NeoPixel.h>
#include <PS2Keyboard.h>
#include <SerialLCD.h>
#include <Wire.h>

#define CENTER_PIXEL_PIN 6    // Digital IO pin connected to the NeoPixels.
#define LEFT_PIXEL_PIN 1
#define RIGHT_PIXEL_PIN 0
#define KEYBOARD_DATA_PIN 8
#define KEYBOARD_IRQ_PIN 5
#define CENTER_PIXEL_COUNT 144
#define LEFT_PIXEL_COUNT 72
#define RIGHT_PIXEL_COUNT 72
#define BUFFER_SIZE 8

enum states { CONTROL_OPEN, LEFT_CONTROL, LEFT_SENDING, RIGHT_CONTROL, RIGHT_SENDING };
enum bufferPos { BUFFER_LEFT, BUFFER_RIGHT };

PS2Keyboard keyboard;
int i=0;

// Parameter 1 = number of pixels in strip,  neopixel stick has 8
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream, correct for neopixel stick
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip), correct for neopixel stick
Adafruit_NeoPixel c_strip = Adafruit_NeoPixel(CENTER_PIXEL_COUNT, CENTER_PIXEL_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel l_strip = Adafruit_NeoPixel(LEFT_PIXEL_COUNT, LEFT_PIXEL_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel r_strip = Adafruit_NeoPixel(RIGHT_PIXEL_COUNT, RIGHT_PIXEL_PIN, NEO_GRB + NEO_KHZ800);

SerialLCD lcd(2,16,0x28,I2C);
char buffer[BUFFER_SIZE + 1];
int charPointer = 0;
int sendShift = 0;
int state = CONTROL_OPEN;

void setup() {
  // Initialize LCD module
  lcd.init();

  c_strip.begin();
  l_strip.begin();
  r_strip.begin();
  c_strip.show(); // Initialize all pixels to 'off'
  l_strip.show(); // Initialize all pixels to 'off'
  r_strip.show(); // Initialize all pixels to 'off'

  keyboard.begin(KEYBOARD_DATA_PIN, KEYBOARD_IRQ_PIN);
  Serial.begin(9600);
  Serial.println("Keyboard Test:");
  
  lcd.clear();
  lcd.setContrast(40);
  
  for (int i=0; i<=BUFFER_SIZE; i++) buffer[i]=0;
}

void loop() {
  switch (state) {
    case CONTROL_OPEN:
    case RIGHT_CONTROL: rightControl(); break;
    case LEFT_CONTROL: rightControl(); break;
    case RIGHT_SENDING: rightSending(); break;
    case LEFT_SENDING: leftSending(); break;
  }
//  delay(20);
}

void rightControl() {
  if (keyboard.available()) {
    char c = keyboard.read();

    if (c == PS2_BACKSPACE) {
      if (charPointer > 0)
        buffer[--charPointer] = 0;
    } else if (c == PS2_ENTER) {
      if (state == LEFT_CONTROL) {
        sendShift = CENTER_PIXEL_COUNT - 64;
        state = LEFT_SENDING;
      } else {
        sendShift = 0;
        state = RIGHT_SENDING;
      }
    } else {
      if (charPointer < BUFFER_SIZE)
        buffer[charPointer++] = c;
    }

    lcd.clear();
    
    lcd.home();
    lcd.print(buffer);
    
    bitLights(buffer);
  }
}

void rightSending() {
  sendShift++;
  if (sendShift + 8 * BUFFER_SIZE > CENTER_PIXEL_COUNT) {
    state = LEFT_CONTROL;
    return;
  }
  bitLights(buffer);
}

void leftSending() {
  sendShift--;
  if (sendShift == 0) {
    state = RIGHT_CONTROL;
    return;
  }
  bitLights(buffer);
}

void bitLights(char *buffer) {
  l_strip.clear();
  for (int i=0; i<8; i++) {
    for (int j=0; j<8; j++) {
      int pos = sendShift + i*8+j;
      l_strip.setPixelColor( pos, (buffer[i] >> j) & 0x1 ? Wheel(pos+charPointer*i) : c_strip.Color(0,0,0) );
    }
  }
  l_strip.show();
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
   return c_strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
   return c_strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
   WheelPos -= 170;
   return c_strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}\
