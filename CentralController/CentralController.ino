#include <SoftwareSerial.h>
#include <FastLED.h>
#include <arduinoFFT.h>

#define SAMPLES 32
#define SAMPLING_FREQUENCY 480
#define NUM_LEDS 60

arduinoFFT FFT = arduinoFFT();

SoftwareSerial esp8266(9, 8);
CRGB leds[NUM_LEDS];

bool newData = false;
bool inProgC = false;
bool autoC = false;
bool spectrumC = false;
bool trailC = false;
bool led_on = false;

char recvData[32];
char command[32];
char param[32];

int rgb[3];
int rnd_rgb[3];
int atc[2];
int changed_leds = 0;
long average;
long dl_a = 0;
long dl_b = 0;
long ans = 0;
int peak_led = 0;
int leds_on = 0;
int spd = 5;
int len = 3;

byte average_i = 0;
byte led;

unsigned int sampling_period_us;
unsigned long microseconds;

double vReal[SAMPLES];
double vImag[SAMPLES];

CRGBArray = {[255, 0, 0], [0, 255, 0], [0, 0, 255], [255, 255, 0], [255, 40, 0], [255, 0, 255], [128, 0, 128], [255, 20, 147], [255, 215, 0], [135, 206, 250]};

void setup() {
  sampling_period_us = round(1000000*(1.0/SAMPLING_FREQUENCY));
  atc[0] = 60;
  atc[1] = 440;
  FastLED.addLeds<WS2811, 10, BRG>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();
  Serial.begin(115200);
  esp8266.begin(115200);
  pinMode(10, OUTPUT);
  pinMode(2, INPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(A5, INPUT);
  Serial.println(analogRead(A0));
  Serial.println("Starting");
  Serial.println(dataArray[0]);
  flowAnimation(0, 50, 0);
}

void loop() {
  averageLight();
  byte i;
  while(esp8266.available() > 0){
    char c = esp8266.read();
    if (c == '<'){
      i = 0;
      recvData[0] = '\0';
      inProgC = true;
    } else if (c == '>') {
      inProgC = false;
      newData = true;
      break;
    } else if (inProgC) {
      recvData[i] = c;
      i++;
      recvData[i] = '\0';
    }
  }
  if (newData){
    recvData[i] = '\0';
    //Serial.println(recvData);
    if (!strcmp(recvData, "lightInfo")) {
      esp8266.print("<light-");
      esp8266.print(average);
      esp8266.print('>');
    } else if (!strcmp(recvData, "spectrumctrl")) {
      FastLED.clear();
      FastLED.show();
      peak_led = 0;
      leds_on = 0;
      spectrumC = true;
      autoC = false;
      trailC = false;
      if (!spectrumC) {
        randomColor();
        ans = millis();
      }
    } else if (!strcmp(recvData, "solid")) {
      fill_solid(leds, NUM_LEDS, CRGB(rgb[0], rgb[1], rgb[2]));
      FastLED.show();
      autoC = false;
      spectrumC = false;
      trailC = false;
    } else if (!strcmp(recvData, "trail")) {
      ans = millis();
      dl_a = 0;
      trailC = true;
      autoC = false;
      spectrumC = false;
    } else if (!strcmp(recvData, "clear")) {
      FastLED.clear();
      FastLED.show();
      spectrumC = false;
      autoC = false;
      trailC = false;
      led_on = false;
    } else if (subchar(0, 8, "autoctrl")) {
      split('[', ']');
      splitATC(param, ',', strlen(param));
      Serial.print("Light on: ");
      Serial.println(atc[0]);
      Serial.print("Light off: ");
      Serial.println(atc[1]);
      FastLED.clear();
      if (led_on) {
        fill_solid(leds, NUM_LEDS, CRGB(rgb[0], rgb[1], rgb[2]));
        FastLED.show();
      } else {
        autoC = true;
        spectrumC = false;
        trailC = false;
      }
    } else if (subchar(0, 3, "RGB")) {
      split('[', ']');
      splitRGB(param, ',', strlen(param));
      if (spectrumC) {
        for (int i = 0; i < leds_on; i++) {
          leds[i] = CRGB(rgb[0], rgb[1], rgb[2]);
        }
      }
    } else if (subchar(0, 5, "speed")) {
      split('=', NULL);
      spd = atoi(param);
      Serial.print("Speed: ");
      Serial.println(spd);
    } else if (subchar(0, 3, "len")) {
      split('=', NULL);
      len = atoi(param);
      Serial.print("Length: ");
      Serial.println(len);
    }
    newData = false;
    recvData[0] = '\0';
  }
  if (autoC) {
    if (average < atc[0] and !led_on) {
      fill_solid(leds, NUM_LEDS, CRGB(round(rgb[0]/4),round(rgb[1]/4),round(rgb[2]/4)));
      FastLED.show();
      led_on = true;
    } else if (average > atc[1] and led_on) {
      FastLED.clear();
      FastLED.show();
      led_on = false;
    }
  } else if (spectrumC) {
    SpectrumControl();
  } else if (trailC) {
    trail(len, 0);
  }
}

bool subchar(int ns, int ne, char* comp) {
  if (ns < 16 or ne < 16) {
    command[0] = '\0';
    int x = 0;
    for (int i = ns; i < ne; i++) {
      command[x] = recvData[i];
      x++;
    }
    command[x] = '\0';
    if (!strcmp(command, comp)) {
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

void split(char cs, char ce) {
  param[0] = '\0';
  int n = 0;
  if (ce == NULL) {
    for (int i = indexof(cs)+1; i <= strlen(recvData); i++) {
      param[n] = recvData[i];
      n++;
    }
  } else {
    for (int i = indexof(cs); i <= indexof(ce); i++) {
      param[n] = recvData[i];
      n++;
    }
  }
  param[n] = '\0';
  //Serial.println(param);
}

int indexof(char c) {
  int ind;
  for (int i = 0; i < strlen(recvData); i++) {
    if (recvData[i] == c) {
      ind = i;
    }
  }
  return ind;
}

void trail(int head, int tail) {
  if (millis()-ans >= dl_a + spd) {
    dl_a += spd;
    for (int i = tail; i <= head; i++) {
      if (led - i >= 0 and led - i < NUM_LEDS) {
        if (i == head) {
          leds[led-i] = CRGB(0, 0, 0);
        } else {
          leds[led-i] = CRGB(round(rgb[0]/(i+1)), round(rgb[1]/(i+1)), round(rgb[2]/(i+1)));
        }
      } 
    }
    FastLED.show();
    led++;
    if (led == NUM_LEDS+3) {
      led = 0;
    }
  }
}

void splitRGB(char data[16], char split_ch, int data_size) {
  char temp_data[4];
  for (byte i = 0; i < data_size; i++){
    if (data[i+1] == '\0') {
      data[i] = '\0';
      break;
    }
    data[i] = data[i+1];
  }
  byte rgb_i = 0;
  byte temp_i = 0;
  for (byte i = 0; i < 17; i++) {
    if (data[i] == '\0' or data[i] == '\n') break;
    if (data[i] != split_ch and data[i] != ']') {
      temp_data[temp_i] = data[i];
      temp_i++;
    } else {
      temp_data[temp_i] = '\0';
      //Serial.println(atoi(temp_data));
      rgb[rgb_i] = atoi(temp_data);
      rgb_i++;
      temp_data[0] = '\0';
      temp_i = 0;
    }
  }
}

void splitATC(char data[16], char split_ch, int data_size) {
  char temp_data[4];
  for (byte i = 0; i < data_size; i++){
    if (data[i+1] == '\0') {
      data[i] = '\0';
      break;
    }
    data[i] = data[i+1];
  }
  byte atc_i = 0;
  byte temp_i = 0;
  for (byte i = 0; i < 17; i++) {
    if (data[i] == '\0' or data[i] == '\n') break;
    if (data[i] != split_ch and data[i] != ']') {
      temp_data[temp_i] = data[i];
      temp_i++;
    } else {
      temp_data[temp_i] = '\0';
      //Serial.println(atoi(temp_data));
      atc[atc_i] = atoi(temp_data);
      atc_i++;
      temp_data[0] = '\0';
      temp_i = 0;
    }
  }
}

void randomColor() {
  rnd_rgb[0] = random(0, rgb[0]);
  rnd_rgb[1] = random(0, rgb[1]);
  rnd_rgb[2] = random(0, rgb[2]);
}

void SpectrumControl() {
  for(int i=0; i<SAMPLES; i++){
      microseconds = micros();
      vReal[i] = analogRead(0);
      vImag[i] = 0;
      while(micros() < (microseconds + sampling_period_us)){
      }
  }
 
  FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);
    
  double peak = FFT.MajorPeak(vReal, SAMPLES, SAMPLING_FREQUENCY);
  //Serial.print("Peak Frequency: ");
  //Serial.println(peak);
  if (peak <= 220.0) {
    peak_led = (NUM_LEDS*round(vReal[round(peak/(SAMPLING_FREQUENCY/SAMPLES)*1.0)]))/750;
    Serial.print("Peak LED ");
    Serial.println(peak_led);
    //Serial.print("Intensity ");
    //Serial.println(vReal[round(peak/(SAMPLING_FREQUENCY/SAMPLES)*1.0)]);
  
    if (peak_led > 50) {
      peak_led = 50;
    }
      
    if (leds_on < peak_led) {
      for (int i = leds_on; i < peak_led; i++) {
        leds[i] = CRGB(rnd_rgb[0], rnd_rgb[1], rnd_rgb[2]);
        FastLED.show();
        Serial.print("Turning on led: ");
        Serial.println(i);
        delay(2);
      }
      leds_on = peak_led;
    }
  }
  if (millis()-ans >= dl_a + 2000) {
    dl_a += 2000;
    randomColor();
    fill_solid(leds, leds_on, CRGB(rnd_rgb[0], rnd_rgb[1], rnd_rgb[2]));
    FastLED.show();
    Serial.print("Leds changed: ");
    Serial.println(leds_on);
  }
  if (millis()-ans >= dl_a + 10) {
    dl_a += 10;
    Serial.println("10ms delay");
    if (leds_on > peak_led) {
      leds_on--;
      leds[leds_on] = CRGB(0,0,0);
      FastLED.show();
      Serial.print("Turning off led: ");
      Serial.println(leds_on);
    }
  }
}

void averageLight() {
  int lightLvl;
  average = 0;
  for (byte i = 0; i < 100; i++) {
    lightLvl = analogRead(A5);
    average = average + lightLvl;
  }
  average = average/100;
}

void flowAnimation(int r, int g, int b) {
  for (byte i = round(NUM_LEDS/2); i < NUM_LEDS; i++) {
    leds[i] = CRGB(r, g, b);
    leds[(NUM_LEDS-i)-1] = CRGB(r, g, b);
    FastLED.show();
    delay(30);
  }
  FastLED.clear();
  FastLED.show();
}
