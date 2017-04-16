// Step Sizes
float stepSize = 1;
const unsigned int stepFull[2] = {LOW, LOW};
const unsigned int stepHalf[2] = {HIGH, LOW};
const unsigned int stepQuarter[2] = {LOW, HIGH};
const unsigned int stepEighth[2] = {HIGH, HIGH};

// X-axis Stepper
const unsigned int x_stepPin = 2;
const unsigned int x_directionPin = 3;
const unsigned int x_microstep1Pin = 4;
const unsigned int x_microstep2Pin = 5;
const unsigned int x_enablePin = 6;

// X-axis limit switches
const unsigned int x_leftLimitSwitchPin = 7;
const unsigned int x_rightLimitSwitchPin = 8;

// Y-axis stepper
const unsigned int y_stepPin = 9;
const unsigned int y_directionPin = 10;
const unsigned int y_microstep1Pin = 11;
const unsigned int y_microstep2Pin = 12;
const unsigned int y_enablePin = 13;

// Maximums
bool maximumStepsRequired = true;
float x_maximumSteps = 0;
float y_maximumSteps = 0;

// Position
float x_position = 0;
float y_position = 0;

/*******************************************************************
 * Main Setup
 ******************************************************************/

void setup() {
  xSetup();
  ySetup();

  Serial.begin(9600);
}

void xSetup() {
  // Stepper
  pinMode(x_stepPin, OUTPUT);
  pinMode(x_directionPin, OUTPUT);
  pinMode(x_microstep1Pin, OUTPUT);
  pinMode(x_microstep2Pin, OUTPUT);
  pinMode(x_enablePin, OUTPUT);

  // Limit Switches
  pinMode(x_leftLimitSwitchPin, INPUT);
  pinMode(x_rightLimitSwitchPin, INPUT);
}

void ySetup() {
  // Stepper
  pinMode(y_stepPin, OUTPUT);
  pinMode(y_directionPin, OUTPUT);
  pinMode(y_microstep1Pin, OUTPUT);
  pinMode(y_microstep2Pin, OUTPUT);
  pinMode(y_enablePin, OUTPUT);  

  // Limit Switches
  // - not hooked up yet
  // - TODO

}

/*******************************************************************
 * Main Loop
 ******************************************************************/

void loop() {
  if (maximumStepsRequired) {
    findMinimumAndMaximum();
    maximumStepsRequired = false;
    
    // TEMP
    delay(3000);
    simpleDemo();
  }

//  while (Serial.available()) {
//    Serial.println("Options:");
//    Serial.println("1. Demo");
//    
//  }
}

void findMinimumAndMaximum() {
  setStepType(stepFull);
  enableSteppers(true);

  Serial.print("\n\nFind X-min… ");
  findMinimumX();
  Serial.print("Found");

  Serial.print("\nFind Y-min… ");
  findMinimumY();
  Serial.print("Found");

  Serial.print("\nMeasure X-max… ");
  measureMaximumX();
  Serial.print(x_maximumSteps);

  Serial.print("\nMeasure Y-max… ");
  measureMaximumY();
  Serial.print(y_maximumSteps);

  enableSteppers(false);
}

void enableSteppers(bool enable) {
  if (enable) {
    digitalWrite(x_enablePin, LOW);
    digitalWrite(y_enablePin, LOW);
  } else {
    digitalWrite(x_enablePin, HIGH);
    digitalWrite(y_enablePin, HIGH);
  }
}

void setStepType(const unsigned int type[2]) {
  // X-axis
  digitalWrite(x_microstep1Pin, type[0]);
  digitalWrite(x_microstep2Pin, type[1]);

  // Y-axis
  digitalWrite(y_microstep1Pin, type[0]);
  digitalWrite(y_microstep2Pin, type[1]);

  // Change the position increment size.
  if (type == stepFull) {
    stepSize = 1.0;
  } else if (type == stepHalf) {
    stepSize = 0.5;
  } else if (type == stepQuarter) {
    stepSize = 0.25;
  } else if (type == stepEighth) {
    stepSize = 0.125;
  }
}

void simpleDemo() {
  enableSteppers(true);

  // set origin of workpiece.
  // never operate on min and max, or within 100 steps of them, as the limit switches are approximately 100-ish steps wide.
  // TODO
  
  setStepType(stepFull);
  
  Serial.print("\n\nGoto Maximum - 200… ");
  gotoPosition(x_maximumSteps - 200, y_maximumSteps - 200);
  delay(3000);

  Serial.print("\n\nGoto Half… ");
  gotoPosition(ceil(x_maximumSteps / 2), ceil(y_maximumSteps / 2));
  delay(3000);

  Serial.print("\n\nGoto Minimum + 200");
  gotoPosition(200, 200);
  delay(3000);

  Serial.print("\n\nExplore… ");
  gotoPosition(450, 450);
  delay(300);
  gotoPosition(400, 400);
  delay(300);
  gotoPosition(350, 350);
  delay(300);
  gotoPosition(400, 400);
  delay(300);

  setStepType(stepHalf);
  gotoPosition(450, 450);
  delay(300);
  gotoPosition(400, 400);
  delay(300);
  gotoPosition(350, 350);
  delay(300);
  gotoPosition(400, 400);
  delay(300);

  setStepType(stepQuarter);
  gotoPosition(450, 450);
  delay(300);
  gotoPosition(400, 400);
  delay(300);
  gotoPosition(350, 350);
  delay(300);
  gotoPosition(400, 400);
  delay(300);

  setStepType(stepEighth);
  gotoPosition(450, 450);
  delay(300);
  gotoPosition(400, 400);
  delay(300);
  gotoPosition(350, 350);
  delay(300);
  gotoPosition(400, 400);
  delay(300);

  setStepType(stepFull);
  gotoPosition(200, 200);

  Serial.print("Finished\n");
  enableSteppers(false);
}

void gotoPosition(const float x, const float y) {
  while (x_position != x || y_position != y) {
    float dx = x - x_position;
    float dy = y - y_position;

    x_step(dx);
    y_step(dy);
  }
}

/*******************************************************************
 * Step
 ******************************************************************/

void x_step(float dx) {
  if (dx < 0 && !atMinimumLimitX()) {
    x_setNegativeDirection();
    stepForPin(x_stepPin);
    x_position -= stepSize;
  } else if (dx < 0 && atMinimumLimitX()) {
    Serial.println("dX cannot change due to limit.");
    Serial.println(dx);
    Serial.println(x_position);
    delay(9000);
  } else if (dx > 0 && !atMaximumLimitX()) {
    x_setPositiveDirection();
    stepForPin(x_stepPin);
    x_position += stepSize;
  }
}

// Ideal: step(xAxis, directionNegative, stepFull); < should include limit detection, and position update.
void stepForPin(unsigned int stepPin) {
  digitalWrite(stepPin, HIGH);
  delay(1);
  digitalWrite(stepPin, LOW);
  delay(1);
}

void y_step(float dy) {
  if (dy < 0) {
    y_setNegativeDirection();
    stepForPin(y_stepPin);
    y_position -= stepSize;
  } else if (dy > 0) {
    y_setPositiveDirection();
    stepForPin(y_stepPin);
    y_position += stepSize;
  }
}

/*******************************************************************
 * Directions
 ******************************************************************/

void x_setNegativeDirection() {
  digitalWrite(x_directionPin, HIGH);
}

void x_setPositiveDirection() {
  digitalWrite(x_directionPin, LOW);
}

void y_setNegativeDirection() {
  digitalWrite(y_directionPin, HIGH);
}

void y_setPositiveDirection() {
  digitalWrite(y_directionPin, LOW);
}

/*******************************************************************
 * Find X
 ******************************************************************/

void findMinimumX() {
  setStepType(stepFull);
  x_setNegativeDirection();

  while ( ! atMinimumLimitX()) {
    stepForPin(x_stepPin);
  }

  x_position = 0;
}

bool atMinimumLimitX() {
  return (digitalRead(x_leftLimitSwitchPin) == HIGH);
}

void measureMaximumX() {
  setStepType(stepFull);
  x_setPositiveDirection();

  while ( ! atMaximumLimitX()) {
    stepForPin(x_stepPin);
    x_maximumSteps += stepSize;
    x_position += stepSize;
  }
}

bool atMaximumLimitX() {
  return (digitalRead(x_rightLimitSwitchPin) == HIGH);
}

/*******************************************************************
 * Find Y
 ******************************************************************/

void findMinimumY() {
  setStepType(stepFull);
  // TEMP
  y_setNegativeDirection();

  for(int y = 1; y < 500; y++) {
    stepForPin(y_stepPin);
  }

  y_position = 0;
}

void measureMaximumY() {
  setStepType(stepFull);
  // TEMP
  y_setPositiveDirection();

  // TEMP
  y_maximumSteps = 2000;

  for (int y = 1; y < y_maximumSteps; y++) {
    stepForPin(y_stepPin);
    y_position += stepSize;
  }
}

