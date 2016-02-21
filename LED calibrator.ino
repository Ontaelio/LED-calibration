/*******************************************************
 * The RGB LED Calibration Sketch
 * (c) 2016 Dmitry Reznikov, ontaelio - at - gmail.com
 * Use this as you like.
 *******************************************************/

#include <Wire.h>
#define ROUGH 0
#define FINE 1
#define RGBCMYW 2
#define RGB 3
#define CMY 4
#define WB 5
#define RAINBOW 6

long initValues[3], values[3], base[3], modifiers[3], ang;
uint32_t  rainbowDelay = 300; // the delay is later divided by 10
byte current[3], secondLEDon = 1, rainbowMode;;
byte red, green, blue;
int maxVal;
byte curLED, count, compareLED;

/*********************************************************
 *         SETUP SECTION                                 *
 *********************************************************/
 
//#define EXTERNAL_ADC // uncomment this if using second Arduino in slave mode

#define CALIBRATION_MODE ROUGH // select the mode
// valid mode names are: ROUGH FINE RGBCMYW RGB CMY WB RAINBOW

#define POWER_RESTRICTION 1023 // 1023 = 1 led; 2047 = 2 leds; 3072 and more = no restriction

#define COMPAREBUTTONDOWN 7 // Compare button 1
#define COMPAREBUTTONUP 12  // Compare button 2
#define SETBUTTON 8         // SET button
#define PRINTBUTTON 4       // PRINT button
#define HOLDBUTTON 4        // HOLD button (FINE mode)
#define OVERFLOWLED 13      // Overflow LED

byte outputPins[6] = {3, 5, 6, 9, 10, 11}; // PWM pins

// setRGBpoint (0, ...) for pins 3, 5, 6; setRGBpoint (1, ...) for pins 9, 10, 11.
// See the outputPins array above
void setRGBpoint(byte LED, byte red, byte green, byte blue)
{
  // this code is for common anode LEDs. If you use common cathode ones,
  // remove the '255-' bits.
  analogWrite(outputPins[LED*3], 255-red);
  analogWrite(outputPins[LED*3+1], 255-green);
  analogWrite(outputPins[LED*3+2], 255-blue);
}

//**** The following is related to ROUGH and FINE modes ****

#if CALIBRATION_MODE < 2

#define tableSize 13 // the number of colors you intend to calibrate
#define divider 10 // the fine-tuning precision

// the following table should be replaced with the results
// of the ROUGH calibration before doing the FINE one
byte RGBready[13][3] ={
{255, 0, 0}, // hour=0, min=0
{210, 44, 0}, // min=1
{183, 71, 0}, // hour=1
{149, 105, 0}, // min=2
{0, 136, 0}, // hour=2, min=3
{0, 164, 35}, // min=4
{0, 156, 98}, // hour=3
{0, 100, 155}, // min=5
{0, 0, 255}, // hour=4, min=6
{68, 0, 186}, // min=7
{145, 0, 110}, // hour=5
{198, 0, 57}, // min=8
{94, 78, 82} // min=9
};


// comments strings
char* comment[]={
 "hour=0, min=0",
 "min=1",
 "hour=1",
 "min=2",
 "hour=2, min=3",
 "min=4",
 "hour=3",
 "min=5",
 "hour=4, min=6",
 "min=7",
 "hour=5",
 "min=8",
 "min=9"
};

//**** The following is related to the rest of the modes ****

#else

// array of preset colors
byte CMYWready[7][3] = {
{255, 0, 0},    // red
{0, 255, 0},    // green
{0, 0, 255},    // blue
{127, 127, 0},  // yellow
{0, 127, 127},  // cyan
{127, 0, 127},  // magenta
{85, 85, 85}};  // white

// comments
char* CMYWcomment[]={
  "red",
  "green",
  "blue",
  "yellow",
  "cyan",
  "magenta",
  "white"
};

/*********************************************************
 *         SETUP SECTION ENDS HERE                       *
 *********************************************************/

// sine wave table
const byte lights[300] ={0,0,0,0,0,1,2,3,3,4,6,7,8,10,11,13,15,17,19,21,23,26,
28,31,33,36,39,42,45,48,51,54,58,61,64,68,71,75,78,82,86,90,93,97,101,
105,109,113,117,121,125,128,132,136,140,144,148,152,156,159,163,167,171,
174,178,181,185,188,191,195,198,201,204,207,210,213,216,218,221,223,226,
228,230,232,234,236,238,239,241,242,243,245,246,246,247,248,249,249,249,
249,250,249,249,249,249,248,247,246,246,245,243,242,241,239,238,236,234,
232,230,228,226,223,221,218,216,213,210,207,204,201,198,195,191,188,185,
181,178,174,171,167,163,159,156,152,148,144,140,136,132,128,124,121,117,
113,109,105,101,97,93,90,86,82,78,75,71,68,64,61,58,54,51,48,45,42,39,36,
33,31,28,26,23,21,19,17,15,13,11,10,8,7,6,4,3,3,2,1,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

// the function that shows rainbow after RGBCMYW modes
void showTwoFinalRainbows()
{
      // this will show synthetic sine wave colors on 2nd LED
      //setRGBpoint (1, lights[(ang+100)%300],
      //                lights[ang%300],
      //                lights[(ang+200)%300]);

      // first LED shows 'base' colors, sine wave
      //setRGBpoint (0, (lights[(ang+100)%300]*base[0])/255,
      //                (lights[ang%300]*base[1])/255,
      //                (lights[(ang+200)%300]*base[2])/255);

      // or it can show 'multiplier' colors, sine wave
      //setRGBpoint (0, (lights[(ang+100)%300]*modifiers[0])/100000,
      //                (lights[ang%300]*modifiers[1])/100000,
      //                (lights[(ang+200)%300]*modifiers[2])/100000);

      // second LED shows rainbow based on red, green, blue from the resulting 
      // RGBready table, sine wave
     // setRGBpoint (1, (lights[(ang+100)%300]*CMYWready[0][0])/255,
      //                (lights[ang%300]*CMYWready[1][1])/255,
      //                (lights[(ang+200)%300]*CMYWready[2][2])/255);

     //the following does the 'power-conscious HSV' rainbow with general values
     getHSVvalues();
     setRGBpoint(1, red, green, blue);

     // and here is the same 'power-conscious' HSV with base values applied
     setRGBpoint (0, map (red, 0, 255, 0, base[0]),
                     map (green, 0, 255, 0, base[1]),
                     map (blue, 0, 255, 0, base[2]));
}

// shows current lavues on test LED
void CMYWCurrentLED()
{
  setRGBpoint (0, current[0], current[1], current[2]);
}

// shows default values on reference LED
void CMYWCompareLED()
{
  setRGBpoint (1, CMYWready[compareLED][0], 
                  CMYWready[compareLED][1], 
                  CMYWready[compareLED][2]);
}

// prints results for RGBCMYW modes
 #if CALIBRATION_MODE < RAINBOW
void printCMYWresult()
{
  Serial.print("byte RGBready[");
  Serial.print(7);
  Serial.println("][3] ={");
  for (byte k=0; k<7; k++)
  {
    Serial.print("{");
    Serial.print(CMYWready[k][0]);
    Serial.print(", ");
    Serial.print(CMYWready[k][1]);
    Serial.print(", ");
    Serial.print(CMYWready[k][2]);
    if (k<6) Serial.print("},"); else Serial.print("}");
    Serial.print(" // ");
    Serial.println(CMYWcomment[k]);
  } 
  Serial.println("};");

  // calculate base values
  getCMYWbase();
  
  // calculate maximum among them 
  maxVal = getMax(getMax(base[0],base[1]),getMax(base[1],base[2]));
  
  // print base values relative to 255 
  Serial.print("byte baseRed = ");
  Serial.print(map (base[0], 0, maxVal, 0, 255));
  Serial.println(";");
  Serial.print("byte baseGreen = ");
  Serial.print(map (base[1], 0, maxVal, 0, 255));
  Serial.println(";");
  Serial.print("byte baseBlue = ");
  Serial.print(map (base[2], 0, maxVal, 0, 255));
  Serial.println(";");

  // print base values relative to 100000
  Serial.print("long multiplierRed = ");
  Serial.print(map (base[0], 0, maxVal, 0, 100000));
  Serial.println(";");
  Serial.print("long multiplierGreen = ");
  Serial.print(map (base[1], 0, maxVal, 0, 100000));
  Serial.println(";");
  Serial.print("long multiplierBlue = ");
  Serial.print(map (base[2], 0, maxVal, 0, 100000));
  Serial.println(";");

  modifiers[0] = map (base[0], 0, maxVal, 0, 100000);
  modifiers[1] = map (base[1], 0, maxVal, 0, 100000);
  modifiers[2] = map (base[2], 0, maxVal, 0, 100000);
  
  base[0] = map (base[0], 0, maxVal, 0, 255);
  base[1] = map (base[1], 0, maxVal, 0, 255);
  base[2] = map (base[2], 0, maxVal, 0, 255);
 
  while(!checkButton(PRINTBUTTON)) // show the rainbow until the button is pressed again
   {
      showTwoFinalRainbows();
      if (checkButton(COMPAREBUTTONUP)) rainbowDelay -= rainbowDelay/10;
      if (checkButton(COMPAREBUTTONDOWN)) rainbowDelay += rainbowDelay/10;
      if (rainbowDelay<10) rainbowDelay = 10;
      delay(rainbowDelay/10);
     ang++;
     ang%=300;                
   } 
}
 #endif

#endif

void setup()
{
  Wire.begin();        
  Serial.begin(9600);  
  pinMode (OVERFLOWLED, OUTPUT);
  pinMode (SETBUTTON, INPUT_PULLUP);
  pinMode (COMPAREBUTTONUP, INPUT_PULLUP);
  pinMode (COMPAREBUTTONDOWN, INPUT_PULLUP);
  pinMode (PRINTBUTTON, INPUT_PULLUP);
#if CALIBRATION_MODE == FINE
  setBaseValues();
#endif
}

/***********************************************************************
 *   ROUGH MODE                                                        *
 ***********************************************************************/

#if CALIBRATION_MODE == ROUGH
void loop()
{
  Serial.print(curLED);
  Serial.print(": ");
  
 getADCreadings();
 
checkRange();

for (byte k=0; k<3; k++)
{current[k] = values[k]/4;
 Serial.print(current[k]);
 Serial.print(" ");
}

Serial.print(" // ");
Serial.println(comment[curLED]);

showCurrentLED();
showCompareLED();

if (checkButton(PRINTBUTTON)) printTable();
if (checkButton(COMPAREBUTTONUP)) compareLED = (compareLED+1)%tableSize;
if (checkButton(COMPAREBUTTONDOWN)) compareLED = (compareLED+tableSize-1)%tableSize;
if (checkButton(SETBUTTON)) 
    {for (byte k=0; k<3; k++) RGBready[curLED][k] =  current[k];
    if ((curLED+1)==tableSize) printTable();
     curLED = (curLED+1)%tableSize;
     compareLED = curLED; 
    }
  delay(100);
}


/***********************************************************************
 *   FINE MODE                                                         *
 ***********************************************************************/

#elif CALIBRATION_MODE == FINE
void loop()
{
  Serial.print(curLED);
  Serial.print(": ");
#ifdef EXTERNAL_ADC
  count=0;
  Wire.requestFrom(12, 6);   // request 6 bytes from device 12 
  while(Wire.available())  
  { 
    byte a = Wire.read(); // 1st byte
    byte b = Wire.read(); // 2nd byte
    uint16_t c = (uint16_t)(a<<8) + b; // make a 10-bit integer out of them
    modifiers[count] = (int)(c - base[count]) / divider; // get a modifier
    values[count] = initValues[count] + modifiers[count]; // set values
    if (values[count]<0) values[count]=0; // don't let 'em go below zero
    Serial.print(modifiers[count]);         // print the modifier
    Serial.print(" ");
    count++; 
  }
 
#else

  values[0]=analogRead(A0);
  values[1]=analogRead(A1);
  values[2]=analogRead(A2);

  for (count=0; count<3; count++)
  {
    modifiers[count] = (int)(values[count] - base[count]) / divider; // get a modifier
    values[count] = initValues[count] + modifiers[count]; // set values
    if (values[count]<0) values[count]=0; // don't let 'em go below zero
    Serial.print(modifiers[count]);         // print the modifier
    Serial.print(" ");
  }
#endif

Serial.print("--- ");
checkRange();

for (byte k=0; k<3; k++)
{current[k] =  values[k]/4;
 Serial.print(current[k]);
 Serial.print(" ");
}

Serial.print(" // ");
Serial.println(comment[curLED]);

showCurrentLED();
showCompareLED();

while (checkButton(HOLDBUTTON)) {for (byte k=0; k<3; k++) RGBready[curLED][k] =  current[k]; setBaseValues();};
if (checkButton(COMPAREBUTTONUP)) compareLED = (compareLED+1)%tableSize;
if (checkButton(COMPAREBUTTONDOWN)) compareLED = (compareLED+tableSize-1)%tableSize;
if (checkButton(SETBUTTON)) 
    {for (byte k=0; k<3; k++) RGBready[curLED][k] =  current[k];
     if ((curLED+1)==tableSize) printTable();
     curLED = (curLED+1)%tableSize;
     compareLED = curLED; 
     setBaseValues();
    }
  delay(100);
}

void setBaseValues()
{
  for (uint8_t k=0; k<3; k++) initValues[k] = RGBready[curLED][k]*4;
  getADCreadings();
  base[0] = values[0];
  base[1] = values[1];
  base[2] = values[2];
  delay(100);
}

/***********************************************************************
 *   RGBCMYW MODE                                                      *
 *****************************************-*****************************/

#elif CALIBRATION_MODE == RGBCMYW
void loop()
{
  Serial.print(CMYWcomment[curLED]);
  Serial.print(": ");
  
getADCreadings();

checkRange();

for (byte k=0; k<3; k++)
{current[k] = values[k]/4;
 Serial.print(current[k]);
 Serial.print(" ");
}

Serial.println();

CMYWCurrentLED();
CMYWCompareLED();

if (checkButton(PRINTBUTTON)) printCMYWresult();
if (checkButton(COMPAREBUTTONUP)) compareLED = (compareLED+1)%7;
if (checkButton(COMPAREBUTTONDOWN)) compareLED = (compareLED+7-1)%7;
if (checkButton(SETBUTTON)) 
    {CMYWready[curLED][0]=current[0];
     CMYWready[curLED][1]=current[1];
     CMYWready[curLED][2]=current[2];
     if ((curLED+1)==7) printCMYWresult();
     curLED = (curLED+1)%7;
     compareLED = curLED; 
    }
 
  delay(100);
}

// calculate base values.
// this function differs between modes.
void getCMYWbase()
{
base[0] = (CMYWready[0][0]*2 + CMYWready[3][0]*2 + CMYWready[5][0]*2 + CMYWready[6][0]*3);
base[1] = (CMYWready[1][1]*2 + CMYWready[3][1]*2 + CMYWready[4][1]*2 + CMYWready[6][1]*3);
base[2] = (CMYWready[2][2]*2 + CMYWready[4][2]*2 + CMYWready[5][2]*2 + CMYWready[6][2]*3);
}


/***********************************************************************
 *   RGB MODE                                                          *
 ***********************************************************************/

#elif CALIBRATION_MODE == RGB
void loop()
{
  Serial.print(CMYWcomment[curLED]);
  Serial.print(": ");
  
getADCreadings();

checkRange();

for (byte k=0; k<3; k++)
{current[k] = values[k]/4;
 Serial.print(current[k]);
 Serial.print(" ");
}

Serial.println();

CMYWCurrentLED();
CMYWCompareLED();

if (checkButton(PRINTBUTTON)) printCMYWresult();
if (checkButton(COMPAREBUTTONUP)) compareLED = (compareLED+1)%3;
if (checkButton(COMPAREBUTTONDOWN)) compareLED = (compareLED+3-1)%3;
if (checkButton(SETBUTTON)) 
    {CMYWready[curLED][0]=current[0];
     CMYWready[curLED][1]=current[1];
     CMYWready[curLED][2]=current[2];
     if ((curLED+1)==3) printCMYWresult();
     curLED = (curLED+1)%3;
     compareLED = curLED; 
    }

  delay(100);
}

// calculate base values.
// this function differs between modes.
void getCMYWbase()
{
base[0] = CMYWready[0][0];
base[1] = CMYWready[1][1];
base[2] = CMYWready[2][2];
}

/***********************************************************************
 *   CMY MODE                                                          *
 ***********************************************************************/

#elif CALIBRATION_MODE == CMY
void loop()
{
  if (curLED==0) {curLED+=3; compareLED=3;};
  Serial.print(CMYWcomment[curLED]);
  Serial.print(": ");
  
getADCreadings();

checkRange();

for (byte k=0; k<3; k++)
{current[k] = values[k]/4;
 Serial.print(current[k]);
 Serial.print(" ");
}

Serial.println();

CMYWCurrentLED();
CMYWCompareLED();

if (checkButton(PRINTBUTTON)) printCMYWresult();
if (checkButton(COMPAREBUTTONUP)) compareLED = (compareLED+1)%3;
if (checkButton(COMPAREBUTTONDOWN)) compareLED = (compareLED+3-1)%3;
if (checkButton(SETBUTTON)) 
    {CMYWready[curLED][0]=current[0];
     CMYWready[curLED][1]=current[1];
     CMYWready[curLED][2]=current[2];
     if ((curLED+1)==6) printCMYWresult();
     curLED = (curLED+1)%3+3;
     compareLED = curLED; 
    }
 
  delay(100);
}

// calculate base values.
// this function differs between modes.
void getCMYWbase()
{
base[0] = (CMYWready[3][0]*2 + CMYWready[5][0]*2);
base[1] = (CMYWready[3][1]*2 + CMYWready[4][1]*2);
base[2] = (CMYWready[4][2]*2 + CMYWready[5][2]*2);
}

/***********************************************************************
 *   WHITE BALANCE MODE                                                *
 ***********************************************************************/

#elif CALIBRATION_MODE == WB
void loop()
{
  if (curLED==0) {curLED=6; compareLED=6;};
  Serial.print(CMYWcomment[curLED]);
  Serial.print(": ");
  
getADCreadings();

checkRange();

for (byte k=0; k<3; k++)
{current[k] = values[k]/4;
 Serial.print(current[k]);
 Serial.print(" ");
}

Serial.println();

CMYWCurrentLED();

// second LED will display CMY according to current White Balance setting
// or the basic white provided in the table
if (compareLED==6) CMYWCompareLED(); else
  {
    maxVal = getMax(getMax(current[0],current[1]),getMax(current[1],current[2]));
    base[0] = map (current[0], 0, maxVal, 0, 255);
    base[1] = map (current[1], 0, maxVal, 0, 255);
    base[2] = map (current[2], 0, maxVal, 0, 255);
    
    setRGBpoint(1, map (CMYWready[compareLED][0], 0, 255, 0, base[0]),
                   map (CMYWready[compareLED][1], 0, 255, 0, base[1]),
                   map (CMYWready[compareLED][2], 0, 255, 0, base[2]));
    
  }

if (checkButton(PRINTBUTTON)) printCMYWresult();
if (checkButton(COMPAREBUTTONUP)) compareLED = (compareLED+2)%4+3;
if (checkButton(COMPAREBUTTONDOWN)) compareLED = (compareLED)%4+3;
if (checkButton(SETBUTTON)) 
    {CMYWready[6][0]=current[0];
     CMYWready[6][1]=current[1];
     CMYWready[6][2]=current[2];
     printCMYWresult();
     compareLED = 6; 
    }
 
  delay(100);
}

// calculate base values.
// this function differs between modes.
void getCMYWbase()
{
base[0] = current[0];
base[1] = current[1];
base[2] = current[2];
}

/***********************************************************************
 *   RAINBOW MODE                                                      *
 ***********************************************************************/

#elif CALIBRATION_MODE == RAINBOW

void loop()
{
  
#ifdef EXTERNAL_ADC
  count=0;
  Wire.requestFrom(12, 6);    // request 6 bytes from slave device 12
  while(Wire.available())    
  { 
    byte a = Wire.read(); // receive a byte 
    byte b = Wire.read(); // receive a byte 
    values[count] = (uint16_t)(a<<8) + b;
    count++; 
  }

#else

  values[0]=analogRead(A0);
  values[1]=analogRead(A1);
  values[2]=analogRead(A2);

#endif

for (byte k=0; k<3; k++) current[k] = values[k]/4;

if (rainbowMode) {rainbowModeLED(); rainbowPlainLED();}
   else {
     getHSVvalues();
     if (secondLEDon) setRGBpoint (1, red, green, blue); else
                       setRGBpoint(1, 0, 0, 0);
     setRGBpoint (0, map (red, 0, 255, 0, current[0]),
                     map (green, 0, 255, 0, current[1]),
                     map (blue, 0, 255, 0, current[2]));
        }
ang++; if (ang>300) ang = 0;

if (checkButton(PRINTBUTTON)) 
{
  rainbowMode = !rainbowMode;
  
  Serial.print("byte red = ");
  Serial.print(current[0]);
  Serial.println(";");
  Serial.print("byte green = ");
  Serial.print(current[1]);
  Serial.println(";");
  Serial.print("byte blue = ");
  Serial.print(current[2]);
  Serial.println(";");

  if (rainbowMode) Serial.println("Now running: Sine Wave Rainbow mode"); 
           else Serial.println("Now running: HSV Rainbow mode"); 
}
 if (checkButton(COMPAREBUTTONUP)) rainbowDelay -= rainbowDelay/10;
      if (checkButton(COMPAREBUTTONDOWN)) rainbowDelay += rainbowDelay/10;
      if (rainbowDelay<10) rainbowDelay = 10;
if (checkButton(SETBUTTON)) secondLEDon = !secondLEDon;
   

delay(rainbowDelay/10);
}

// sine wave rainbow animation with adjusted values
void rainbowModeLED()
{
    setRGBpoint (0, (lights[(ang+100)%300]*current[0])/255,
                    (lights[ang%300]*current[1])/255,
                    (lights[(ang+200)%300]*current[2])/255);
}

// sine wave rainbow animation with default values
void rainbowPlainLED()
{
  if (secondLEDon) {
  // this shows synthetic colors
      setRGBpoint (1, lights[(ang+100)%300],
                   lights[ang%300],
                   lights[(ang+200)%300]);}
   else
   {
     setRGBpoint (1, 0, 0, 0);
   }
}

#endif


/***********************************************************************
 *   COMMON FUNCTIONS AND STUFF                                        *
 ***********************************************************************/

void checkRange()
{
if ((values[0]+values[1]+values[2]) > POWER_RESTRICTION)

{
  uint16_t valsum = (values[0]+values[1]+values[2]);
  values[0] = (values[0] * POWER_RESTRICTION) / valsum ;
  values[1] = (values[1] * POWER_RESTRICTION) / valsum ;
  values[2] = (values[2] * POWER_RESTRICTION) / valsum ;
  digitalWrite(OVERFLOWLED, HIGH);
}
else digitalWrite(OVERFLOWLED, LOW);
}

bool checkButton(byte button)
{
  uint16_t buttonCounter = 0;
  bool result = false;
  while (!digitalRead(button))
    {if (buttonCounter<100) buttonCounter++;
     delayMicroseconds(1);}
 if (buttonCounter>50) result = true;
 return result;
}

int getMax(int a,int b)
{return((a>b)?(a):(b));}

void getADCreadings()
{

#ifdef EXTERNAL_ADC // get results from the second Arduino
  count=0;
  Wire.requestFrom(12, 6);    // request 6 bytes from slave device 12
  while(Wire.available())    
  { 
    byte a = Wire.read(); // receive a byte 
    byte b = Wire.read(); // receive a byte 
    values[count] = (uint16_t)(a<<8) + b;
    Serial.print(values[count]);         // print the value
    Serial.print(" ");
    count++; 
  }

#else // get results from analog pins

  values[0]=analogRead(A0);
  values[1]=analogRead(A1);
  values[2]=analogRead(A2);

  Serial.print(values[0]);         // print the values
  Serial.print(" ");
  Serial.print(values[1]);
  Serial.print(" ");
  Serial.print(values[2]);
  Serial.print(" ");

#endif

Serial.print("--- ");
}

void getHSVvalues()
{
     
     if (ang<50) {red = 255; green = round(ang*5.1-0.01); blue = 0;} else
     if (ang<100) {red = round((100-ang)*5.1-0.01); green = 255; blue = 0;} else 
     if (ang<150) {red = 0, green = 255; blue = round((ang-100)*5.1-0.01);} else 
     if (ang<200) {red = 0, green = round((200-ang)*5.1-0.01); blue = 255;} else 
     if (ang<250) {red = round((ang-200)*5.1-0.01), green = 0; blue = 255;} else 
                 {red = 255, green = 0; blue = round((300-ang)*5.1-0.01);} 
}


#if CALIBRATION_MODE < 2

void showCurrentLED()
{
  setRGBpoint(0, current[0], 
                 current[1], 
                 current[2]);
}

void showCompareLED()
{
  setRGBpoint(1, RGBready[compareLED][0], 
                 RGBready[compareLED][1], 
                 RGBready[compareLED][2]);
}

void printTable()
{
  Serial.print("byte RGBready[");
  Serial.print(tableSize);
  Serial.println("][3] ={");
  for (byte k=0; k<tableSize; k++)
  {
    Serial.print("{");
    Serial.print(RGBready[k][0]);
    Serial.print(", ");
    Serial.print(RGBready[k][1]);
    Serial.print(", ");
    Serial.print(RGBready[k][2]);
    if (k<(tableSize-1)) Serial.print("},"); else Serial.print("}");
    Serial.print(" // ");
    Serial.println(comment[k]);
    
  } 
  Serial.println("};");
  while(!checkButton(PRINTBUTTON)); // delay until the button is pressed again
}

#endif

/*
// the following function is for testing purposes.
// I keep it here just in case
void testWrite()
{
  Serial.println();
  Serial.print("count: ");
  Serial.println(count);
  Serial.print("base: ");
  Serial.println(base[count]);
  Serial.print("values: ");
  Serial.println(values[count]);
  Serial.print("initValues: ");
  Serial.println(initValues[count]);
  Serial.print("modifiers: ");
  Serial.println(modifiers[count]);
}
*/
