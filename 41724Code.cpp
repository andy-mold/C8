#include <FEHUtility.h>
#include <FEHServo.h>
#include <FEHLCD.h>
#include <FEHIO.h>
#include <FEHMotor.h>
#include <FEHRCS.h>
#include <FEHBattery.h>
#include <cmath>
#include <iostream>

#define SCREENWIDTH 320
#define SCREENHEIGHT 240

//front of robot marked with electrical tape
//no grey tape
FEHMotor leftMotor(FEHMotor::Motor2, 7.2);
//grey tape
FEHMotor rightMotor(FEHMotor::Motor0, 7.2);

DigitalEncoder leftEncoder(FEHIO::P0_3, FEHIO::RisingEdge);
DigitalEncoder rightEncoder(FEHIO::P3_5, FEHIO::RisingEdge);

AnalogInputPin cdsCell(FEHIO::P0_0);

FEHServo servo(FEHServo::Servo0);
FEHServo luggageServo(FEHServo::Servo2);

void initializeStartup() {
    LCD.Clear();
    LCD.SetBackgroundColor(BLACK);
    LCD.SetFontColor(SCARLET);

    LCD.WriteAt("Touch Screen to Begin.",20,100);

    bool start = false;
    float x_position, y_position;
    float x_trash, y_trash;

    LCD.ClearBuffer();
    while(!LCD.Touch(&x_position,&y_position)) {};
    while(LCD.Touch(&x_trash,&y_trash)) {};

    return;
}

void initializeUI() {
    int screenWidth = SCREENWIDTH;
    int screenHeight = SCREENHEIGHT;
    int columnWidth = 50;
    int textBuffer = 5;
    int rowHeight = 25;
    int middleWidth = 220;
    int checkboxLength = 15;

    LCD.Clear();
    LCD.SetFontColor(WHITE);

    LCD.DrawRectangle(1,0,screenWidth-2,screenHeight-2);

    LCD.DrawRectangle(1,0,columnWidth-1,screenWidth-2);
    LCD.DrawRectangle(screenWidth-columnWidth,0,columnWidth-1,screenWidth-2);

    LCD.WriteAt("FL",textBuffer,textBuffer);
    LCD.WriteAt("BR",screenWidth-columnWidth+textBuffer,textBuffer);

    LCD.DrawHorizontalLine(rowHeight,0,columnWidth);
    LCD.DrawHorizontalLine(rowHeight,screenWidth-columnWidth,screenWidth);

    LCD.DrawHorizontalLine(rowHeight,columnWidth,columnWidth+middleWidth);
    LCD.WriteAt("F/B",columnWidth+textBuffer,textBuffer);
    LCD.WriteAt("L/R",screenWidth-columnWidth-45,textBuffer);

    LCD.DrawRectangle(columnWidth*2,textBuffer,checkboxLength,checkboxLength);
    LCD.DrawRectangle(screenWidth-columnWidth*2-checkboxLength,textBuffer,checkboxLength,checkboxLength);

    LCD.DrawHorizontalLine(rowHeight*2, columnWidth, columnWidth+middleWidth);
    LCD.DrawHorizontalLine(rowHeight*3, columnWidth, columnWidth+middleWidth);
    LCD.DrawHorizontalLine(rowHeight*4, columnWidth, columnWidth+middleWidth);
    
}

int getLightInput() {
    float input = cdsCell.Value();
    Sleep(0.1);
    float input2 = cdsCell.Value();
    Sleep(0.1);
    float input3 = cdsCell.Value();

    float average = (input + input2 + input3)/3;

    if (average < 1.0) {
        //red
        return 1;
    }
    else if (average < 2.0) {
        //blue
        return 2;
    }
    else {
        return 0;
    }
}

void updateUI(float leftPower, float rightPower) {
    initializeUI();

    int screenWidth = SCREENWIDTH;
    int screenHeight = SCREENHEIGHT;
    int columnWidth = 50;
    int textBuffer = 5;
    int rowHeight = 25;
    int middleWidth = 220;
    int checkboxLength = 15;

    LCD.SetFontColor(YELLOW);

    LCD.DrawRectangle(1, rowHeight + (screenHeight-rowHeight)/2, columnWidth-1, -1*((screenHeight-rowHeight)/2)*leftPower/100.0);
    LCD.DrawRectangle(screenWidth-columnWidth, rowHeight + (screenHeight-rowHeight)/2, columnWidth-1, -1*((screenHeight-rowHeight)/2)*rightPower/100.0);

    LCD.SetFontColor(WHITE);
    LCD.WriteAt("CDS:",columnWidth+textBuffer,textBuffer+rowHeight);
    LCD.WriteAt(cdsCell.Value(), columnWidth+textBuffer+50, textBuffer+rowHeight);

    if (getLightInput() == 1) {
        LCD.WriteAt("Color: Red",columnWidth+textBuffer,textBuffer+rowHeight*2);
    } else if (getLightInput() == 2) { 
        LCD.WriteAt("Color: Blue",columnWidth+textBuffer,textBuffer+rowHeight*2);
    } else {
        LCD.WriteAt("Color: None",columnWidth+textBuffer,textBuffer+rowHeight*2);
    }


    LCD.WriteAt(RCS.GetCorrectLever(),columnWidth+textBuffer,textBuffer+rowHeight*3);
}

void updateOrientation(float leftPower, float rightPower, bool orientation) {
    updateUI(leftPower, rightPower);

    int screenWidth = SCREENWIDTH;
    int screenHeight = SCREENHEIGHT;
    int columnWidth = 50;
    int textBuffer = 5;
    int rowHeight = 25;
    int middleWidth = 220;
    int checkboxLength = 15;

    LCD.SetFontColor(GREEN);
    if (orientation == true) {
        LCD.FillRectangle(columnWidth*2,textBuffer,checkboxLength,checkboxLength);
    } else {
        LCD.FillRectangle(screenWidth-columnWidth*2-checkboxLength,textBuffer,checkboxLength,checkboxLength);
    }
}

void driveForwardsTime(float power, float time) {
    float maxVoltage = 11.5;
    float actualPower = maxVoltage/Battery.Voltage()*power;

    leftMotor.SetPercent(-actualPower);
    rightMotor.SetPercent(actualPower);

    updateUI(actualPower, actualPower);

    Sleep(time);

    leftMotor.Stop();
    rightMotor.Stop();

    updateUI(0,0);
}

int max(int a, int b) {
    if (a > b) {
        return a;
    } else {
        return b;
    }
}

void driveForwards(float power, float inches) {
    //Wheel Circumference = 8.64in
    //Counts = 60
    float circumference = 8.64;
    float counts = (inches/circumference) * 30.0;

    LCD.Clear();
    float maxVoltage = 11.5;
    float actualPower = maxVoltage/Battery.Voltage()*power;

    rightEncoder.ResetCounts();
    leftEncoder.ResetCounts();

    rightMotor.SetPercent(actualPower);
    leftMotor.SetPercent(-actualPower);

    while(max(leftEncoder.Counts(), rightEncoder.Counts()) < counts) {
        LCD.Clear();
        LCD.WriteAt(leftEncoder.Counts(), 5, 5);
        LCD.WriteAt(rightEncoder.Counts(), 5, 30);
        Sleep(0.05);
    }

    rightMotor.Stop();
    leftMotor.Stop();
}

void driveForwardsPID(float inches) {
    rightEncoder.ResetCounts();
    leftEncoder.ResetCounts();

    float expectedVelocity = 10.0;
    float countsPerRotation = 30.0;
    float wheelCircumference = 8.64;
    int counts = (inches/wheelCircumference) * 30.0;
    float distancePerCount = wheelCircumference / countsPerRotation;
    float time1, time2, leftCounts1, leftCounts2, rightCounts1, rightCounts2, timeChange, leftCountsChange, rightCountsChange, actualVelocity, leftError, leftErrorSum = 0, lastLeftError, rightError, rightErrorSum = 0, lastRightError;
    float pTerm, iTerm, dTerm;
    float leftMotorPower = 20.0, rightMotorPower = 20.0;
    float pConstant = 0.75, dConstant = 0.2, iConstant = 0.08;
    bool firstTime = true;
    time1 = TimeNow();
    leftCounts1 = leftEncoder.Counts();
    rightCounts1 = rightEncoder.Counts();
    leftMotor.SetPercent(-leftMotorPower);
    rightMotor.SetPercent(rightMotorPower);
    Sleep(0.3);

    while((leftEncoder.Counts() + rightEncoder.Counts()) / 2.0 < counts) {
        leftCounts2 = leftEncoder.Counts();
        rightCounts2 = rightEncoder.Counts();
        time2 = TimeNow();

        timeChange = time2-time1;
        leftCountsChange = leftCounts2-leftCounts1;
        rightCountsChange = rightCounts2-rightCounts1;

        //Left
        actualVelocity = distancePerCount * (leftCountsChange / timeChange);
        leftError = expectedVelocity - actualVelocity;
        leftErrorSum += leftError;

        pTerm = leftError * pConstant;
        iTerm = leftErrorSum * iConstant;
        if (!firstTime) {
            dTerm = (leftError-lastLeftError) * dConstant;
        }

        leftMotorPower += pTerm + iTerm + dTerm;
        leftMotor.SetPercent(-leftMotorPower);
        

        //Right
        actualVelocity = distancePerCount * (rightCountsChange / timeChange);
        rightError = expectedVelocity - actualVelocity;
        rightErrorSum += rightError;

        pTerm = rightError * pConstant;
        iTerm = rightErrorSum * iConstant;
        if (!firstTime) {
            dTerm = (rightError-lastRightError) * dConstant;
        } else {
            firstTime = false;
        }

        rightMotorPower += pTerm + iTerm + dTerm;
        rightMotor.SetPercent(rightMotorPower);

        time1 = time2;
        leftCounts1 = leftCounts2;
        rightCounts1 = rightCounts2;
        lastLeftError = leftError;
        lastRightError = rightError;
        Sleep(0.3);

    }

    leftMotor.Stop();
    rightMotor.Stop();

}

void driveBackwardsPID(float inches) {
    rightEncoder.ResetCounts();
    leftEncoder.ResetCounts();

    float expectedVelocity = 10.0;
    float countsPerRotation = 60.0;
    float wheelCircumference = 8.64;
    int counts = (inches/wheelCircumference) * 60.0;
    float distancePerCount = wheelCircumference / countsPerRotation;
    float time1, time2, leftCounts1, leftCounts2, rightCounts1, rightCounts2, timeChange, leftCountsChange, rightCountsChange, actualVelocity, leftError, leftErrorSum = 0, lastLeftError, rightError, rightErrorSum = 0, lastRightError;
    float pTerm, iTerm, dTerm;
    float leftMotorPower = 20.0, rightMotorPower = 20.0;
    float pConstant = 0.75, dConstant = 0.25, iConstant = 0.08;
    bool firstTime = true;
    time1 = TimeNow();
    leftCounts1 = leftEncoder.Counts();
    rightCounts1 = rightEncoder.Counts();
    leftMotor.SetPercent(leftMotorPower);
    rightMotor.SetPercent(-rightMotorPower);
    Sleep(0.1);

    while((leftEncoder.Counts() + rightEncoder.Counts()) / 2.0 < counts) {
        leftCounts2 = leftEncoder.Counts();
        rightCounts2 =
        
        rightEncoder.Counts();
        time2 = TimeNow();

        timeChange = time2-time1;
        leftCountsChange = leftCounts2-leftCounts1;
        rightCountsChange = rightCounts2-rightCounts1;

        //Left
        actualVelocity = distancePerCount * (leftCountsChange / timeChange);
        leftError = expectedVelocity - actualVelocity;
        leftErrorSum += leftError;

        pTerm = leftError * pConstant;
        iTerm = leftErrorSum * iConstant;
        if (!firstTime) {
            dTerm = (leftError-lastLeftError) * dConstant;
        } else {
            firstTime = false;
        }

        leftMotorPower += pTerm + iTerm + dTerm;
        leftMotor.SetPercent(leftMotorPower);
        

        //Right
        actualVelocity = distancePerCount * (rightCountsChange / timeChange);
        rightError = expectedVelocity - actualVelocity;
        rightErrorSum += rightError;

        pTerm = rightError * pConstant;
        iTerm = rightErrorSum * iConstant;
        if (!firstTime) {
            dTerm = (rightError-lastRightError) * dConstant;
        } else {
            firstTime = false;
        }

        rightMotorPower += pTerm + iTerm + dTerm;
        rightMotor.SetPercent(-rightMotorPower);

        time1 = time2;
        leftCounts1 = leftCounts2;
        rightCounts1 = rightCounts2;
        lastLeftError = leftError;
        lastRightError = rightError;
        Sleep(0.1);

    }

    leftMotor.Stop();
    rightMotor.Stop();

}

void startWithLight() {
    bool waiting = true;

    while (waiting) {
        int light = getLightInput();
        if (light == 1) {
            updateUI(0,0);
            return;
        }
        Sleep(1.0);
        updateUI(0,0);
    }
}

void turnLeft(float power, float time) {
    float maxVoltage = 11.5;
    float actualPower = maxVoltage/Battery.Voltage()*power;

    leftMotor.SetPercent(actualPower);
    rightMotor.SetPercent(actualPower);

    updateUI(actualPower, -actualPower);

    Sleep(time);

    leftMotor.Stop();
    rightMotor.Stop();

    updateUI(0,0);
}

void turnRight(float power, float time) {
    float maxVoltage = 11.5;
    float actualPower = maxVoltage/Battery.Voltage()*power;

    leftMotor.SetPercent(-actualPower);
    rightMotor.SetPercent(-actualPower);

    updateUI(-actualPower, actualPower);

    Sleep(time);

    leftMotor.Stop();
    rightMotor.Stop();

    updateUI(0,0);
}

bool changeOrientation(bool orientation) {
    bool newOrientation = orientation;

    float maxVoltage = 11.5;
    float actualPower = maxVoltage/Battery.Voltage()*85;

    if (orientation) {
        leftMotor.SetPercent(-actualPower);
        rightMotor.SetPercent(-actualPower);
        newOrientation = false;
        servo.SetDegree(0.0);
        Sleep(0.18);
        leftMotor.SetPercent(0);
        rightMotor.SetPercent(0);
        
    } else {
        leftMotor.SetPercent(actualPower);
        rightMotor.SetPercent(actualPower);
        newOrientation = true;
        servo.SetDegree(180.0);
        Sleep(0.18);
        leftMotor.SetPercent(0);
        rightMotor.SetPercent(0);
    }
    updateOrientation(0,0,newOrientation);
    return newOrientation;
}

void calibrateServo() {
    bool orientation = true;

    orientation = changeOrientation(orientation);
    Sleep(2.0);
    orientation = changeOrientation(orientation);
    Sleep(2.0);
    servo.SetDegree(160.0);
}

void driveForwardsDebug(float power, float inches) {
    //Wheel Circumference = 8.64in
    //Counts = 60
    float circumference = 8.64;
    float counts = (inches/circumference) * 30.0;

    float maxVoltage = 11.5;
    float actualPower = maxVoltage/Battery.Voltage()*power;

    rightEncoder.ResetCounts();
    leftEncoder.ResetCounts();

    rightMotor.SetPercent(actualPower);
    leftMotor.SetPercent(-actualPower);

    while((leftEncoder.Counts() + rightEncoder.Counts()) / 2.0 < counts) {
        Sleep(0.1);
    }

    rightMotor.Stop();
    leftMotor.Stop();
}

bool changeOrientationDebug(bool orientation) {
    bool newOrientation = orientation;

    float maxVoltage = 11.5;
    float actualPower = maxVoltage/Battery.Voltage()*85;

    if (orientation) {
        leftMotor.SetPercent(-actualPower);
        rightMotor.SetPercent(-actualPower);
        newOrientation = false;
        servo.SetDegree(0.0);
        Sleep(0.18);
        leftMotor.SetPercent(0);
        rightMotor.SetPercent(0);
        
    } else {
        leftMotor.SetPercent(actualPower);
        rightMotor.SetPercent(actualPower);
        newOrientation = true;
        servo.SetDegree(180.0);
        Sleep(0.18);
        leftMotor.SetPercent(0);
        rightMotor.SetPercent(0);
    }
    return newOrientation;
}

void runDebugMenu() {
    int screenWidth = SCREENWIDTH;
    int screenHeight = SCREENHEIGHT;
    int columnWidth = 100;
    int textBuffer = 5;
    int rowHeight = 25;
    int middleWidth = 220;
    int checkboxLength = 15;

    LCD.Clear();
    LCD.SetFontColor(BLUE);

    LCD.DrawRectangle(1,0,screenWidth-2,screenHeight-2);

    LCD.DrawRectangle(1,0,columnWidth-1,screenWidth-2);
    LCD.DrawRectangle(screenWidth-columnWidth,0,columnWidth-1,screenWidth-2);

    LCD.DrawHorizontalLine(screenHeight/2, 0, screenWidth);
    LCD.DrawHorizontalLine(screenHeight/4, columnWidth, screenWidth-columnWidth);
    LCD.DrawHorizontalLine(screenHeight - screenHeight/4, columnWidth, screenWidth-columnWidth);
    LCD.DrawVerticalLine(screenWidth/2, 0, screenHeight);

    LCD.WriteAt("F/R", textBuffer, textBuffer);
    LCD.WriteAt("B/L", textBuffer, textBuffer + screenHeight / 2);
    LCD.WriteAt("Orient", screenWidth - columnWidth + textBuffer, textBuffer);
    LCD.WriteAt("Quit", screenWidth - columnWidth + textBuffer, screenHeight / 2 + textBuffer);

    LCD.WriteAt("20%", columnWidth + textBuffer, textBuffer);
    LCD.WriteAt("50%", columnWidth + textBuffer, textBuffer + screenHeight / 4);
    LCD.WriteAt("70%", columnWidth + textBuffer, textBuffer + screenHeight / 2);
    LCD.WriteAt("90%", columnWidth + textBuffer, textBuffer + screenHeight / 2 + screenHeight / 4);

    LCD.WriteAt("3in", screenWidth / 2 + textBuffer, textBuffer);
    LCD.WriteAt("5in", screenWidth / 2 + textBuffer, textBuffer + screenHeight / 4);
    LCD.WriteAt("10in", screenWidth / 2 + textBuffer, textBuffer + screenHeight / 2);
    LCD.WriteAt("15in", screenWidth / 2 + textBuffer, textBuffer + screenHeight / 2 + screenHeight / 4);

    bool exit = false;
    bool orientation = true;

    float distance = 3.0;
    float power = 20.0;

    while(!exit) {
        float x_position, y_position;
        float x_trash, y_trash;

        LCD.ClearBuffer();
        while(!LCD.Touch(&x_position,&y_position)) {};
        while(LCD.Touch(&x_trash,&y_trash)) {};

        if (x_position < 100 && y_position < 120) {
            driveForwardsDebug(power, distance);
        } else if (x_position < 100) {
            driveForwardsDebug(-power, distance);
        } else if (x_position < screenWidth/2 && y_position < screenHeight/4) {
            power = 20.0;
        } else if (x_position < screenWidth/2 && y_position < screenHeight/2) {
            power = 50.0;
        } else if (x_position < screenWidth/2 && y_position < screenHeight/4 + screenHeight/2) {
            power = 70.0;
        } else if (x_position < screenWidth/2) {
            power = 90.0;
        } else if (x_position < screenWidth - columnWidth && y_position < screenHeight/4) {
            distance = 3.0;
        } else if (x_position < screenWidth - columnWidth && y_position < screenHeight/2) {
            distance = 5.0;
        } else if (x_position < screenWidth - columnWidth && y_position < screenHeight/4 + screenHeight/2) {
            distance = 10.0;
        } else if (x_position < screenWidth - columnWidth) {
            distance = 15.0;
        } else if (y_position < screenHeight/2) {
            orientation = changeOrientationDebug(orientation);
        } else {
            exit = true;
        }
    }
    LCD.Clear();
}

int main(void) {
    //startup ui
    initializeStartup();
    updateUI(0,0);

    //set servo values
    servo.SetMin(558);
    servo.SetMax(1771);
    servo.SetDegree(180);

    luggageServo.SetMin(652);
    //neutral 1409
    luggageServo.SetMax(1657);
    luggageServo.SetDegree(0);

    //Initialize RCS
    RCS.InitializeTouchMenu("C8LAaK2zD");

    //robot starts run in closed position (closed = true)
    bool orientation = true;

    /*
    Code for full run starts here. In driving function, power is the first parameter, and time/distance (in) is the second parameter.
    Positive power values will drive forwards or right depending on orientation, and negative values will drive backwards/left
    */
    
    //Start With Lights
    startWithLight();
    //luggageServo.SetDegree(135);
    driveForwardsTime(-20, 0.02);

    //Lever Setup
    driveForwards(50, 18);
    turnLeft(20, 1.4);
    
    //Align With Lever
    if (RCS.GetCorrectLever() == 0) {
        //A
    } else if (RCS.GetCorrectLever() == 1) {
        //A1
        driveForwards(30, 3.5);
    } else if (RCS.GetCorrectLever() == 2) {
        //B
        driveForwards(30, 7);
    }

    //Push Lever Down
    orientation = changeOrientation(orientation);

    driveForwardsTime(-30, 1);
    driveForwardsTime(30, 0.8);

    //Align for lever up
    //orientation = changeOrientation(orientation);
    //driveForwards(20, 1);
    //orientation = changeOrientation(orientation);

    //Push lever up
    //driveForwardsTime(-30, 1);
    //driveForwardsTime(30, 0.8);

    Sleep(1.0);
    if (RCS.GetCorrectLever() != 2) {
        orientation = changeOrientation(orientation);
    }

    //Reset after levers
    if (RCS.GetCorrectLever() == 0) {
        //A
        driveForwards(30, 7);
        Sleep(2.0);
        orientation = changeOrientation(orientation);
    } else if (RCS.GetCorrectLever() == 1) {
        //A1
        driveForwards(30, 3.5);
        Sleep(2.0);
        orientation = changeOrientation(orientation);
    } else if (RCS.GetCorrectLever() == 2) {
        //B
    }

    //Up steep ramp
    driveForwards(85, 22);
    
    //To luggage
    Sleep(2.0);
    orientation = changeOrientation(orientation);
    driveForwards(-50, 8);

    //luggage
    Sleep(2.0);
    orientation = changeOrientation(orientation);
    driveForwards(-30, 2);
    //driveForwardsTime(-30, 1.0);
    luggageServo.SetDegree(180);
    Sleep(1.0);
    driveForwards(30, 2);
    //driveForwardsTime(30, 1.0);
    luggageServo.SetDegree(0);
    orientation = changeOrientation(orientation);

    //buttons
    driveForwards(30, 1);
    orientation = changeOrientation(orientation);


    //Detect light
    bool detected = false;
    int currentLight;
    int finalLight;
    double startTime = TimeNow();
    double finalTime;

    leftMotor.SetPercent(-20);
    rightMotor.SetPercent(20);
    while(!detected && finalTime-startTime < 10.0) {   
        currentLight = getLightInput();
        finalTime = TimeNow();

        if (currentLight == 1 || currentLight == 2) {
            detected = true;
            finalLight = getLightInput();
        }
    }
    leftMotor.Stop();
    rightMotor.Stop();

    driveForwards(-30, 5);
    

    /*
    //Get aligned with correct button
    orientation = changeOrientation(orientation);
    if (finalLight == 1) {
        //red
        driveForwards(-30, 5.74);
    } else if (finalLight == 2) {
        //blue
        driveForwards(-30, 9.12);
    }

    //Run into button
    Sleep(2.0);
    orientation = changeOrientation(orientation);
    driveForwards(30, 1);
    Sleep(2.0);
            driveForwards(-50, 13.78);
    orientation = changeOrientation(orientation);

    //Get to passport stamp
    if (finalLight == 2) {
        //blue
        driveForwards(-50, 1);
        orientation = changeOrientation(orientation);
    } else if (finalLight == 1) {
        //red
        driveForwards(-50, 4.65);
        orientation = changeOrientation(orientation);
    }

    //flip stamp
    driveForwards(30, 4);
    driveForwards(-30, 5);
    

    //Get aligned with steep ramp
    Sleep(2.0);
    orientation = changeOrientation(orientation);
    driveForwards(50, 17.64);
    Sleep(2.0);
    orientation = changeOrientation(orientation);
    
    //Back down ramp
    driveForwards(-50, 25);
    Sleep(2.0);

    //Realign with correct lever
    if (RCS.GetCorrectLever() == 0) {
        //A
        orientation = changeOrientation(orientation);
        driveForwards(-30, 1);
        Sleep(2.0);
        orientation = changeOrientation(orientation);
    } else if (RCS.GetCorrectLever() == 1) {
        //A1
        orientation = changeOrientation(orientation);
        driveForwards(-30, 1);
        Sleep(2.0);
        orientation = changeOrientation(orientation);
    } else if (RCS.GetCorrectLever() == 2) {
        //B
        orientation = changeOrientation(orientation);
        driveForwards(-30, 1);
        Sleep(2.0);
        orientation = changeOrientation(orientation);
    }

    //Flip lever back up
    driveForwardsTime(-30, 0.5);
    driveForwardsTime(30, 0.5);

    //Recorrect after levers
    Sleep(2.0);
    orientation = changeOrientation(orientation);
    
    if (RCS.GetCorrectLever() == 1) {
        //A1
        orientation = changeOrientation(orientation);
        driveForwards(-30, 1);
        Sleep(2.0);
        orientation = changeOrientation(orientation);
    } else if (RCS.GetCorrectLever() == 2) {
        //B
        orientation = changeOrientation(orientation);
        driveForwards(-30, 1);
        Sleep(2.0);
        orientation = changeOrientation(orientation);
    }

    //Back to start button
    turnRight(20, 1.8);
    driveForwardsTime(-50, 10); //extra distance to be safe
    */
    
    //Return to loose
    servo.SetDegree(160);
    
}
