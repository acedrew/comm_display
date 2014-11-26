// This is a demonstration on how to use an input device to trigger changes on your neo pixels.
// You should wire a momentary push button to connect from ground to a digital IO pin.  When you
// press the button it will change to a new pixel animation.  Note that you need to press the
// button once to start the first animation!

#include <PS2Keyboard.h>
#include <SerialLCD.h>
#include <Wire.h>
#include <OctoWS2811.h>

const int ledsPerStrip = 144;

DMAMEM int displayMemory[ledsPerStrip*6];
int drawingMemory[ledsPerStrip*6];

const int config = WS2811_GRB | WS2811_800kHz;

OctoWS2811 leds(ledsPerStrip, displayMemory, drawingMemory, config);

#define KEYBOARD_DATA_PIN 0
#define KEYBOARD_IRQ_PIN 1
#define CENTER_PIXEL_COUNT 144
#define LEFT_PIXEL_COUNT 72
#define RIGHT_PIXEL_COUNT 72
#define BUFFER_SIZE 8

enum states { CONTROL_OPEN, LEFT_CONTROL, LEFT_SENDING, RIGHT_CONTROL, RIGHT_SENDING };
enum bufferPos { BUFFER_LEFT, BUFFER_RIGHT };

PS2Keyboard keyboard;
int i=0;

SerialLCD lcd(2,16,0x28,I2C);
char buffer[BUFFER_SIZE + 1];
int charPointer = 0;
int sendShift = 0;
int state = CONTROL_OPEN;
int rainbowColors[180];

void setup() {
  // Initialize LCD module
  lcd.init();
  for (int i=0; i<180; i++) {
    int hue = i * 2;
    int saturation = 100;
    int lightness = 50;
    // pre-compute the 180 rainbow colors
    rainbowColors[i] = makeColor(hue, saturation, lightness);
  }


  leds.begin();
  clearLeds();
  leds.show();

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

//writes zeroes to the entire strip buffer
void clearLeds() {
  for (i=0; i < ledsPerStrip * 3; i++) {
    leds.setPixel(i, 0);
  }
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
  clearLeds();
  for (int i=0; i<8; i++) {
    for (int j=0; j<8; j++) {
      int pos = sendShift + i*8+j;
      leds.setPixel( pos, (buffer[i] >> j) & 0x1 ? rainbowColors[pos] : 0
      );
    }
  }
  leds.show();
}
