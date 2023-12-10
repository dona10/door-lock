// Team Number: 20
// Names: Dona Maria, Binh Nguyen
// NetId’s - dmari21, bnguye26
// Name of the project - Lock with an Alarm system
// Abstract:
// The “Lock with an alarm system” project enhances the safety of the residents and the efficiency of door locks. 
// The communication modules between the user and the lock system enables the user to open and close the lock and also gets notified if there is a potential intruder. 
// The key feature of this project includes the alarm system which is setup along with the lock. 
// The alarm system turns off to notify the residents inside the house that an unknown person is trying to unlock the door. 
// This project offers a practical solution to security and control access authorization.


#include <LiquidCrystal.h>
#include <TimeLib.h>


const int rs = 12, en = 11, d4 = 10, d5 = 9, d6 = 8, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
unsigned long startTime = millis();
unsigned long endTime = 0;
unsigned long warningTime = 0;
int state = 0;
int attempt = 0;
char OTP[5];
char newPW[5];

// text thats prints in the first row of the LCD
String messages[6] =
{ "   DB LOCK!    ",
  "door unlocking attempts     ",
  "OTP, press D to reset     ",
  "Set new password, press A.      ",
   "Remember your new password    ",
   "Password has to be 4 digits      " };



// text thats prints in the second row of the LCD
String infos[6] = {"  Dona, Binh  ","Attempts: ", "OTP: "," Door Unlocked  ", "Password: ", ""};
String info;


void setup() {
  int col = 16;
  int row = 2;
  lcd.begin(col, row);
  Serial.begin(9600);
}

// to remove the unwanted characters.
void cleanSerialBuffer() {
  while (Serial.available() > 0) {
    char garbage = Serial.read();
  }
}

// to display the information on the LCD
void displayInfo(int state){
    lcd.setCursor(0, 1);
    lcd.print(infos[state]);
  if (state == 1){
    lcd.print(attempt);
  } else if (state == 2){
    lcd.print(OTP);
  }else if (state == 4){
    lcd.print(newPW);
  }
}



// to print the message on the LCD
void displayText(String message, int delayTime) {
  int textLength = message.length();
  // Check if scrolling is needed
  if (textLength > 16) {
    static int textIndex = 0;
    lcd.clear();
    lcd.setCursor(0, 0);
    for (int i = 0; i < 16; ++i) {
      lcd.print(message[(textIndex + i) % textLength]);
    }
    if (millis() - startTime >= delayTime) {
      startTime = millis();
      ++textIndex;
      if (textIndex >= textLength) {
        textIndex = 0;
      }
    }
  } else {
    // If the message is shorter than the screen, display it without scrolling
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(message);
  }
}

// to reset the state of the LCD
void end(){
  if (state == 4){
    if (endTime + 10000 <= millis()){
      state = 0;
      endTime = 0;
    }
  }else if (state == 5){
        if (warningTime + 5000 <= millis()){
      state = 3;
      warningTime = 0;
    }
  }
}


void loop() {
  displayText(messages[state] , 300);
  displayInfo(state);
  end();
  delay(200);


  if (Serial.available() >= 5){
    unsigned long startTime = millis();
    char rxData[5];
    for (int i = 0; i < 5; i++) {
      rxData[i] = Serial.read();
    }
    // Separate the 1st and 2nd digits
    char firstDigit = rxData[0];
    char secondDigit = rxData[1];


    Serial.print("1st digt: ");
    Serial.print(firstDigit);
    Serial.print("  | ");
    Serial.println(rxData);


     switch (firstDigit) {
      // when the program starts
      case '0':
        attempt = 0;
        state = 0;
        break;
      // when the first try fails
      case '1':
        attempt = (int)(secondDigit - '0');
        state = 1;
        break;
      // when the three tries fail 
      case '2':
        strcpy(OTP, rxData + 1);
        state = 2;
        break;
      // after correctly typing in the otp  
      case '3':
        attempt = 0;
        state = 3;
        break;
      // after the user set up new password  
      case '4':
        strcpy(newPW, rxData + 1);
        state = 4;
        endTime = millis();
        break;
      // warning  
      case '5':
        state = 5;
        warningTime = millis();
        break;
      default:
        break;
    }
    // Reset rxData to '\0' after processing
    memset(rxData, '\0', sizeof(rxData));


    cleanSerialBuffer();
  }
}
