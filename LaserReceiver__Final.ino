#include <map>
#include <Wire.h>
#include <LCD_I2C.h>

#define LDR_PIN 35  
#define BUZZER_PIN 12  
#define Button 34
int LDR_Value = 0;
int threshold = 800;  

unsigned long Start = 0;
unsigned long End = 0;
unsigned long Signal_Len = 0;
int x = 0;  

unsigned long lastLaserTime = 0;
const unsigned long dotTime = 100;    
const unsigned long dashTime = 500;  
const unsigned long letterGap = 1000; 
const unsigned long wordGap = 2000;  

String morseInput = "";  
String sentence = "";    

std::map<String, char> morseMap = {
  {".-", 'A'},   {"-...", 'B'}, {"-.-.", 'C'}, {"-..", 'D'},  {".", 'E'},
  {"..-.", 'F'}, {"--.", 'G'},  {"....", 'H'}, {"..", 'I'},   {".---", 'J'},
  {"-.-", 'K'},  {".-..", 'L'}, {"--", 'M'},   {"-.", 'N'},   {"---", 'O'},
  {".--.", 'P'}, {"--.-", 'Q'}, {".-.", 'R'},  {"...", 'S'},  {"-", 'T'},
  {"..-", 'U'},  {"...-", 'V'}, {".--", 'W'},  {"-..-", 'X'}, {"-.--", 'Y'},
  {"--..", 'Z'}
};

LCD_I2C lcd(0x27, 16, 2);

int getStableLDR() {
  int sum = 0;
  for (int i = 0; i < 10; i++) {  
    sum += analogRead(LDR_PIN);
    delay(5);  
  }
  return sum / 10;  
}

void setup() {
  Serial.begin(9600);
  pinMode(BUZZER_PIN, OUTPUT);  
  pinMode(Button, INPUT);
  lcd.begin();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("Loading");
  delay(1000);
  lcd.clear();
}

void loop() {
  LDR_Value = getStableLDR();  

  if (LDR_Value <= threshold) {  
    tone(BUZZER_PIN, 1000);
    if (x == 0) {  
      x = 1;  
      Start = millis();  
    }
    lastLaserTime = millis();
  } 
  else {  
    noTone(BUZZER_PIN);
    if (x == 1) {  
      End = millis();
      Signal_Len = End - Start;

      if (Signal_Len < dotTime) {
        // Ignore noise
      } else if (Signal_Len < dashTime) {
        morseInput += ".";  
      } else {
        morseInput += "-";  
      }

      x = 0;  
    }
  }
  
  unsigned long timeSinceLastLaser = millis() - lastLaserTime;

  if (morseInput.length() > 0) {
    if (timeSinceLastLaser >= letterGap) {  
      decodeMorse();
      morseInput = "";  
    }
  }

  if (timeSinceLastLaser >= wordGap && sentence.length() > 0) {  
    Serial.print("Decoded Sentence: ");
    Serial.println(sentence);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(sentence);
    sentence = "";  
  }

  if(digitalRead(Button) == LOW) lcd.clear();
}

void decodeMorse() {
  if (morseMap.find(morseInput) != morseMap.end()) {
    char detectedChar = morseMap[morseInput];
    Serial.print("Decoded Letter: ");
    Serial.println(detectedChar);
    sentence += detectedChar;  
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(sentence);
  } else {
    Serial.println("Unknown Morse Code. Try again.");
  }
}

