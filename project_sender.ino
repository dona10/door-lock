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


#include <Keypad.h>

// KEYPAD setup
const int ROW_NUM = 4;     //four rows
const int COLUMN_NUM = 4;  //four columns
char keys[ROW_NUM][COLUMN_NUM] = {  // sets up the each key on the keypad
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};
// to set up the row and column in the keypad
byte pin_rows[ROW_NUM] = { 10, 9, 8, 7 };  
byte pin_column[COLUMN_NUM] = { 6, 5, 4, 3 };
Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM);


// usable var for millis()
const int onDuration = 2000;
const int periodDuration = 5000;
unsigned long lastPeriodStart;
unsigned long prevLockTime;
unsigned long waitTime;
unsigned long lightTime;


// output pin vars
const int red = 11;
const int green = 12;
const int buzzer = 13;


// mutable vars
const String password = "1111";  // initial password, change if needed
int state = 1;
int attempt;
String input_password;
char data[5];
char otp[4];
bool isLocked = true;


void setup() {
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(buzzer, OUTPUT);
  input_password.reserve(4);
  attempt = 0;
  Serial.begin(9600);
  waitTime = millis();
  lightTime = 0;
}


// function for LED flashing, 4 modes
void LED(int n) {
  if (n == 1) {  // (1) green on for 2 secs
    digitalWrite(green, HIGH);
    delay(2000);
    digitalWrite(green, LOW);
  } else if (n == 2) {  //(2) red on for 2 secs
    digitalWrite(red, HIGH);
    delay(2000);
    digitalWrite(red, LOW);
  }else if (n == 3) {  //green blink
    for (int i = 0; i < 5; i++) {
      digitalWrite(green, HIGH);
      delay(200);
      digitalWrite(green, LOW);
      delay(200);
    }
  } else if (n == 4) {  //red blink
    for (int i = 0; i < 5; i++) {
      digitalWrite(red, HIGH);
      delay(200);
      digitalWrite(red, LOW);
      delay(200);
    }
  }
}


// function to show the door is unlocked
void unlock() {
  isLocked = false;
  prevLockTime = millis();
}


// function called when password is correctly inputed
void correct() {
  attempt = 0; // reset # attempt
  unlock();  // door should be unlocked
  LED(1);  // indication LED flash (green)
  input_password = "";
  state = 1;  
  Serial.write("00000"); // sent state 0 to Serial
}


// function called when password is NOT correctly inputed
void incorrect() {
  // # attempts increase still unlocked or 3
  attempt++;
  char data[5] = "";
  sprintf(data, "1%d000",attempt); // each fail attempt sent data to serial as state 1
  Serial.write(data);
  LED(2); // indication LED flash
  input_password = ""; // reset input string
}


// function called when fail 3 attempts, generating OTP
void OTPgen() {
  // generating OTP as a 4 digits string
  const char alphanum[] = "0123456789";
  int string_length = sizeof(alphanum) - 1;
  char data[5] = "";
  char otp[5] = "";
  for (int i = 0; i < 4; i++)
    otp[i] = alphanum[rand() % string_length];
 
  // sent data as state 2, and OTP to be displayed on LCD
  sprintf(data, "2%s",otp);
  Serial.write(data);
  password = String(otp);
  password.trim();
}


// function called only when in state 3, the door is unlocked until a new password is set
// NO delay() used
void waitLED (){
  lightTime = waitTime;
  if ( millis() > lightTime + 500 ){
    if (digitalRead(green) == HIGH){
      digitalWrite(green, LOW);
    }else{
      digitalWrite(green, HIGH);
    }
    waitTime = millis() + 500;
  }    
}


// function called when 3 attempts fail, arlarm sound until OTP is verified
void alarm (){
    if (millis() - lastPeriodStart >= periodDuration) {
    lastPeriodStart += periodDuration;
    tone(buzzer, 800, onDuration);
  }
}


// function called when door are unlocked for 10 secs, then locked automatically
void autolock (){
  if (state != 3){
  if (isLocked == false){
    if (millis() - prevLockTime == 10000 ){
      isLocked = true;
      LED(3);
      LED(2);
    }
  }
  }
}

//function called when press "B", LED indicate if the door is currently locked or not
void checkLock (){
  if (isLocked == false){
    LED(1);
  }else{
    LED(2);
  }
}


// MAIN LOOP
void loop() {

    //alway checks for autolock
    autolock();

    // state 3: a user has to set a new password, or door will be unlocked indefinitely
    if(state ==3){
      unlock();
      waitLED();
    }

  // if 3 fail attempts, changes to state 2 and generate OTP
  if (attempt == 3 && state == 1) {
    OTPgen();
    state = 2;
  }

  //sound alarm if in state 2
  if (state == 2){
    alarm();
  }

  char key = keypad.getKey();

  //CHECK KEY
  if (key) {
    // "B" to check status of lock
    if (key == 'B'){
      checkLock();
    }
    
    // only append to input of key is digit
    if (isDigit(key)) {       // check that key is digit
      input_password += key;  // append new character to input password string
    }


    // beside  "B", macro keys activated when at least 1 digit pressed before.
    else if (input_password.length() > 0) {
      if (key == 'C') { // to erase input_password
        input_password = "";  // clear input password
      }
      else if (key == 'A' && state != 2) {
        if (password == "" && state == 3) { // to reset the new password
          if (input_password.length() != 4){
            LED(4);
            Serial.write("50000");
            input_password = "";
          }else{ // to check if the current passcode typed by the user is equal to the correct passcode
          password = input_password;
          char data[5] = "";
          char newP[5] = "";
          password.toCharArray(newP, 5);
          sprintf(data, "4%s", newP);
          Serial.write(data);
          state = 1;
          attempt = 0;
          input_password = "";
          LED(3);
          }
        }
        // to do the functions if it is incorrect or correct
        else if (password == input_password && state == 1) {
          correct();
        } else {
          incorrect();
        }
      // to reset the password 
      } else if (key == 'D' && state == 2) {
        if (input_password == password) {
          state = 3;
          input_password = "";
          password = "";
          Serial.write("30000");
          LED(3);
        } else {
          state = 2;
          input_password = "";
          LED(4);
        }
      }
    }else{LED(4);}
  }
}
