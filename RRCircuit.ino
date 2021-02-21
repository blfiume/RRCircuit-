#include <Servo.h>

int servoAngle = 0;

const int Relay = 4;
const int Location0E = 22;
const int Location0T = 23;
const int Location1E = 24;
const int Location1T = 25;
const int Location2E = 26;
const int Location2T = 27;
const int Location3E = 28;
const int Location3T = 29;

bool LockCCW = false;
bool LockCW = false;
bool TrainAt0 = false;
bool TrainAt1 = false;
bool TrainAt2 = false;
bool TrainAt3 = false;
bool AcknowledgeLockCW = false;
bool AcknowledgeLockCCW = false;

void setup()
{

  Serial.begin(9600);
  pinMode(Location0E, INPUT);
  pinMode(Location1E, INPUT);
  pinMode(Location2E, INPUT);
  pinMode(Location3E, INPUT);
  pinMode(Location0T, OUTPUT);
  pinMode(Location1T, OUTPUT);
  pinMode(Location2T, OUTPUT);
  pinMode(Location3T, OUTPUT);
  pinMode(Relay, OUTPUT);

}


void loop()
{

  while (true) {
    printStatus();

    TrainAt0 = isOccupied(0, TrainAt0);
    TrainAt1 = isOccupied(1, TrainAt1);
    TrainAt2 = isOccupied(2, TrainAt2);
    TrainAt3 = isOccupied(3, TrainAt3);


    //Check if we need to look for the appproaching ccw trigger
    if (!LockCCW) {
      //if so, lets see if there is a train there
      if (TrainAt0 ) {

        Serial.println("Train at Zero");
        CCW();
        continue;
      }
    }
    else if (TrainAt0 )
      AcknowledgeLockCCW = true;

    if (TrainAt1) {
      PartialCCW();
      continue;
    }
    if (TrainAt2) {
      PartialCW();
      continue;
    }
    if (TrainAt3  ) {
      if (!LockCW) {
        CW();
        continue;
      }
      else {
        AcknowledgeLockCW = true;
      }
    }
    if  (LockCW && AcknowledgeLockCW) {
      if (!TrainAt3 ) {
        LockCW = false;
        AcknowledgeLockCW = false;
      
      }
    }
    if  (LockCCW && AcknowledgeLockCCW) {
      if (!TrainAt0 ) {
        LockCW = false;
        AcknowledgeLockCCW = false;

      }
    }
    printStatus();
  }

}
void printStatus() {
  Serial.println("Locks :");
  Serial.print("Lock CCW: ");
  Serial.println(LockCCW);
  Serial.print("Lock CW: ");
  Serial.println(LockCW);
  Serial.print("Acknowledge CCW: ");
  Serial.println(AcknowledgeLockCCW);
  Serial.print("Acknowledge CW: ");
  Serial.println(AcknowledgeLockCW);
  Serial.println("---------");
  Serial.println("0");
  Serial.println(isOccupied(0, false));
  Serial.println("1");
  Serial.println(isOccupied(1, false));
  Serial.println("2");
  Serial.println(isOccupied(2, false));
  Serial.println("3");
  Serial.println(isOccupied(3, false));

}
void DropRelay() {
  digitalWrite(Relay, HIGH);
  Serial.write("drop");
}

void ReleaseRelay() {
  digitalWrite(Relay, LOW);
  Serial.write("release");
}
void PartialCCW() {
  Serial.println("Counter Clockwise Partial Event Begin");
  
 
  DropRelay();
 
  bool a1b = true;
  bool a2b = true;
  bool a2EverActuated = false;
  while (a1b || a2b) {
    a1b = isOccupied(1, true);
    a2b = isOccupied(2, true);
    if (a2b) {
      a2EverActuated = true;
    }
    

  }
  Serial.println("Partial CCW event terminated");
  ReleaseRelay();
  LockCCW = false;
  if (a2EverActuated) {
    LockCW = true;
  }
}
void PartialCW() {
  Serial.println("Clockwise Partial Event Begin");

  DropRelay();

  bool a1b = true;
  bool a2b = true;
  bool a1EverActuated = false;
  bool isStop = false;
  while (a1b || a2b) {
    a1b = isOccupied(1, true);
    if (a1b) {
      a1EverActuated = true;
    }
    a2b = isOccupied(2, true);
    if (!isStop) {
      if (isOccupied(0, false)) {       
        isStop = true;
      }
    }

  }
  Serial.println("Partial CCW event terminated");
  ReleaseRelay();
  LockCW = false;
  if (a1EverActuated) {
    LockCCW = true;
  }

}
void CCW() {

 
  Serial.println("Counter Clockwise Event Begin");
  DropRelay();
  
  long time = millis();
  bool Timeout = true;
  Serial.println("In the loop time: ");
  Serial.println(time);
  
  while (millis() - time < 25000) {

    if (isOccupied(1, false)) {
      Timeout = false;
      bool a1b = true;
      bool a2b = true;
      bool isDropped = false;
     
      while (a1b || a2b) {
        a1b = isOccupied(1 , true);
        delay(500);        
        if (a1b)
          continue;
        a2b = isOccupied(2, true);
        delay(500);
      }

      ReleaseRelay();
      Serial.println("CounterClockwise Event terminate");

      LockCCW = false;
      LockCW = true;
      return;
    }
  }
  Serial.println("timeout");
  ReleaseRelay();
  LockCCW = true;

}
void CW() {
 

  Serial.println("Clockwise Event Begin");
  DropRelay();
 
  long time = millis();
  bool Timeout = true;
  Serial.println(time);
  while (millis() - time < 25000) {

    if (isOccupied(2, false)) {
      Timeout = false;
      bool a1b = isOccupied(1, false);
      bool a2b = true;
      bool isStop = false;
      while (a1b || a2b) {
        a1b = isOccupied(1, a1b);
        delay(500);
        if (!isStop) {
          if (isOccupied(0, false)) {          
            isStop = true;
          }
        }
        if (a1b)
          continue;
        a2b = isOccupied(2, a2b);
        delay(500);
        
      }
      ReleaseRelay();
     
      Serial.println("Clockwise Event terminate");
      LockCW = false;
      LockCCW = true;
      return;
    }
    delay(1000);
  }
  Serial.println("timeout");
  ReleaseRelay();
  LockCW = true;

}
bool isOccupied(int location, bool TrainPrev) {
  delay(50);
  int timeoutvariable = 450;
  int echo;
  int trigger;
  int threshold = 18;
  if (location == 0) {
    echo = 22;
    trigger = 23;
    //train preset 5...7 to be safe
    //train not present 11...13
    if (TrainPrev)
      threshold = 10;
    else threshold = 9;
  }
  else if (location == 1) {
    echo = 24;
    trigger = 25;
    //train preset 4...7 to be safe
    //train not present 16...13
    if (TrainPrev)
      threshold = 13;
    else threshold = 8;
  }
  else if (location == 2) {
    echo = 26;
    trigger = 27;
    //train preset 9...12 to be safe
    //train not present 26...20
    if (TrainPrev)
      threshold = 23;
    else threshold = 17;
  }
  else {
    echo = 28;
    trigger = 29;
    //train preset 6...9 to be safe
    //train not present 19...16
    if (TrainPrev)
      threshold = 14;
    else threshold = 12;
  }

  long startTime = millis();
  long stopTime = timeoutvariable + startTime;
  long distance = -1;
  while (stopTime >  millis()) {
    digitalWrite(trigger, LOW);
    delayMicroseconds(2);
    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(trigger, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigger, LOW);
    // Reads the echoPin, returns the sound wave travel time in microseconds
    long duration = pulseIn(echo, HIGH);

    // Calculating the distance
    distance = duration * 0.034 / 2;

    if (distance < threshold && distance != 0) {

      Serial.print("location: ");
      Serial.print(location);
      Serial.print("  train detected.  Reported Distance: ");
      Serial.println(distance);
      if (!TrainPrev) {
        Serial.println("No train detected here prev. lets double check ");
        delay (200);
        // Checking once again
        digitalWrite(trigger, LOW);
        delayMicroseconds(2);
        // Sets the trigPin on HIGH state for 10 micro seconds
        digitalWrite(trigger, HIGH);
        delayMicroseconds(10);
        digitalWrite(trigger, LOW);
        // Reads the echoPin, returns the sound wave travel time in microseconds
        duration = pulseIn(echo, HIGH);

        // Calculating the distance
        distance = duration * 0.034 / 2;

        if (distance < threshold && distance != 0) {

          Serial.print("location: ");
          Serial.print(location);
          Serial.print("  train detected.  Reported Distance: ");
          Serial.println(distance);
          Serial.println();
          //we really have a train
          return true;
        }
        return false;

      }
      else {
        delay(100);
        Serial.println("False Positive");
        return true;

      }
    }
    if (TrainPrev) {
      delay (200);
      // Checking once again
      digitalWrite(trigger, LOW);
      delayMicroseconds(2);
      // Sets the trigPin on HIGH state for 10 micro seconds
      digitalWrite(trigger, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigger, LOW);
      // Reads the echoPin, returns the sound wave travel time in microseconds
      duration = pulseIn(echo, HIGH);

      // Calculating the distance
      distance = duration * 0.034 / 2;

      if (distance < threshold && distance != 0) {

        Serial.print("location: ");
        Serial.print(location);
        Serial.print("  train still detected.  Reported Distance: ");
        Serial.println(distance);
        //we really have a train
        return true;
      }
      return false;

    }
    Serial.print("location: ");
    Serial.print(location);
    Serial.print(": No train detected");
    Serial.print(distance);
    return false;
  }

}
void Check2() {
  int  echo = 26;
  int trigger = 27;
  digitalWrite(trigger, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  int duration = pulseIn(echo, HIGH);

  // Calculating the distance
  int distance = duration * 0.034 / 2;
  Serial.print("Calibrrating!!!  : ");
  Serial.print("Reported Distance: ");
  Serial.println(distance);
}
