/**
 *
 * 
 *
**/

#include <SD.h>

#define ERROR_LED 2
#define STATUS_LED 3
#define CS 10
#define CARD_DETECT 8
#define WRITE_PROTECTION 9

char outputFileName[10];
boolean wasError = false;
String content;

void blinkSDCardGood();

void setup() {
 analogReference(EXTERNAL);
 Serial.begin(9600);
 setupIO();
 setupSDCard(); 
 content = String("");
 content.reserve(16);
}

boolean cardPresentWriteable() {
  pinMode(CARD_DETECT, INPUT);
  digitalWrite(CARD_DETECT, HIGH);
  pinMode(WRITE_PROTECTION, INPUT);
  digitalWrite(WRITE_PROTECTION, HIGH);
  boolean ret = digitalRead(CARD_DETECT) || digitalRead(WRITE_PROTECTION);
  digitalWrite(CARD_DETECT, LOW);
  digitalWrite(WRITE_PROTECTION, LOW);
  pinMode(CARD_DETECT, OUTPUT);
  pinMode(WRITE_PROTECTION, OUTPUT);
  return ret;
}

void loop() {
  if(cardPresentWriteable()) {
    setError();
    delay(500);
  } else {
    if(wasError) {
      delay(500);
      digitalWrite(ERROR_LED, LOW);
      Serial.println("Reinitialising card...");

      if(setupSDCard()) {
        wasError = false;
      }
    } else {
      File outputFile = SD.open(outputFileName, FILE_WRITE);
      if(outputFile) {
        digitalWrite(STATUS_LED, HIGH);
        outputFile.println(getContent());
        outputFile.close();
        digitalWrite(STATUS_LED, LOW);
        delay(1000);
      } else {
        setError();
      }
    }
  }
}
  

String getContent() {
  content = "";
  content.concat(map(analogRead(A0), 0, 1023, 0, 2500));
  content.concat(", ");
  content.concat(map(analogRead(A1), 0, 1023, 0, 2500));
  content.concat(", ");
  content.concat(map(analogRead(A2), 0, 1023, 0, 2500));
  Serial.println(content);
  return content;
}

void setupIO() {
 pinMode(CS, OUTPUT);
 pinMode(ERROR_LED, OUTPUT);
 pinMode(STATUS_LED, OUTPUT);
}

boolean setupSDCard() {
 if(!SD.begin(CS)) {
   setError();
   Serial.println("Could not initialize card!");
 } else {
   Serial.println("SD card initialized.");
   if(setupOutputFile()) {
     blinkSDCardGood();
     return true;
   }
 }
}

boolean findFreeFileName() {
  unsigned int i = 1;
  char fileNameChar[10];
  boolean loopFileNames = true;
  String fileName;
  while(loopFileNames) {
    fileName = String("") + i + ".txt";
    Serial.print("Trying output file name ");
    fileName.toCharArray(fileNameChar, 9);
    Serial.print(fileNameChar);
    if(SD.exists(fileNameChar)) {
      Serial.println(" - already exists");
      i++;
      if(0 == i) {
        Serial.println("ERROR: Could not find a name for a new file. All 65535 filenames are taken. Clean up your SD card.");
        setError();
        return false;
      }
    } else {
      Serial.println(" - available");
      fileName.toCharArray(outputFileName, 9);
      loopFileNames = false;
    }
  }
  return true;
}

boolean setupOutputFile() {
  if(!findFreeFileName()) {
    return false;
  }
  File outputFile = SD.open(outputFileName, FILE_WRITE);
  if(!outputFile) {
    Serial.print("Failed to create file ");
    Serial.println(outputFileName);
    Serial.println("giving up");
    setError();
    return false;
  } else {
    outputFile.close();
    Serial.print("Logging to file ");
    Serial.println(outputFileName);
  }
  return true;
}

void blinkSDCardGood() {
   digitalWrite(ERROR_LED, LOW);
   digitalWrite(STATUS_LED, HIGH);
   delay(100);
   digitalWrite(STATUS_LED, LOW);
   delay(100);
   digitalWrite(STATUS_LED, HIGH);
   delay(100);
   digitalWrite(STATUS_LED, LOW);
}

void setError() {
   wasError = true;
   digitalWrite(ERROR_LED, HIGH);
}


