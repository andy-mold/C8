#include <FEHUtility.h>
#include <FEHServo.h>
#include <FEHLCD.h>
#include <FEHIO.h>
#include <FEHMotor.h>
#include <FEHRCS.h>
#include <FEHBattery.h>

#define SCREENWIDTH 320
#define SCREENHEIGHT 240

//front of robot marked with electrical tape
//no grey tape
FEHMotor leftMotor(FEHMotor::Motor2, 7.2);
//grey tape
FEHMotor rightMotor(FEHMotor::Motor0, 7.2);

DigitalEncoder leftEncoder(FEHIO::P0_1);
DigitalEncoder rightEncoder(FEHIO::P0_2);

AnalogInputPin cdsCell(FEHIO::P0_0);

FEHServo servo(FEHServo::Servo0);

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
    LCD.SetFontColor(BLUE);

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

    LCD.SetFontColor(BLUE);
    LCD.WriteAt("CDS:",columnWidth+textBuffer,textBuffer+rowHeight);
    LCD.WriteAt(cdsCell.Value(), columnWidth+textBuffer+50, textBuffer+rowHeight);

    LCD.WriteAt("Color: Blue",columnWidth+textBuffer,textBuffer+rowHeight*2);
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

void driveForwards(float power, int counts) {
    float maxVoltage = 11.5;
    float actualPower = maxVoltage/Battery.Voltage()*power;

    rightEncoder.ResetCounts();
    leftEncoder.ResetCounts();

    rightMotor.SetPercent(actualPower);
    leftMotor.SetPercent(-actualPower);

    while((leftEncoder.Counts() + rightEncoder.Counts()) / 2. < counts);

    rightMotor.Stop();
    leftMotor.Stop();
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
        Sleep(0.15);
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
    servo.SetDegree(155.0);
}

void driveForwardsDebug(float power, float time) {
    float maxVoltage = 11.5;
    float actualPower = maxVoltage/Battery.Voltage()*power;

    leftMotor.SetPercent(-actualPower);
    rightMotor.SetPercent(actualPower);

    Sleep(time);

    leftMotor.Stop();
    rightMotor.Stop();
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
        Sleep(0.15);
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

    LCD.WriteAt("1s", screenWidth / 2 + textBuffer, textBuffer);
    LCD.WriteAt("3s", screenWidth / 2 + textBuffer, textBuffer + screenHeight / 4);
    LCD.WriteAt("5s", screenWidth / 2 + textBuffer, textBuffer + screenHeight / 2);
    LCD.WriteAt("7s", screenWidth / 2 + textBuffer, textBuffer + screenHeight / 2 + screenHeight / 4);

    bool exit = false;
    bool orientation = true;

    float time = 1.0;
    float power = 20.0;

    while(!exit) {
        float x_position, y_position;
        float x_trash, y_trash;

        LCD.ClearBuffer();
        while(!LCD.Touch(&x_position,&y_position)) {};
        while(LCD.Touch(&x_trash,&y_trash)) {};

        if (x_position < 100 && y_position < 120) {
            driveForwardsDebug(power, time);
        } else if (x_position < 100) {
            driveForwardsDebug(-power, time);
        } else if (x_position < screenWidth/2 && y_position < screenHeight/4) {
            power = 20.0;
        } else if (x_position < screenWidth/2 && y_position < screenHeight/2) {
            power = 50.0;
        } else if (x_position < screenWidth/2 && y_position < screenHeight/4 + screenHeight/2) {
            power = 70.0;
        } else if (x_position < screenWidth/2) {
            power = 90.0;
        } else if (x_position < screenWidth - columnWidth && y_position < screenHeight/4) {
            time = 1.0;
        } else if (x_position < screenWidth - columnWidth && y_position < screenHeight/2) {
            time = 3.0;
        } else if (x_position < screenWidth - columnWidth && y_position < screenHeight/4 + screenHeight/2) {
            time = 5.0;
        } else if (x_position < screenWidth - columnWidth) {
            time = 7.0;
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
    servo.SetMin(571);
    servo.SetMax(1819);
    servo.SetDegree(180);

    runDebugMenu();
    /*

    //Initialize RCS
    RCS.InitializeTouchMenu("C8LAaK2zD");

    //robot starts run in closed position (closed = true)
    bool orientation = true;

    //startWithLight();

    //Fuel Lever Code

    driveForwardsTime(50, 2.4);
    turnLeft(20, 2.5);
    updateUI(0,0);

    
    if (RCS.GetCorrectLever() == 0) {
        //A
        driveForwardsTime(30, 0.2);
    } else if (RCS.GetCorrectLever() == 1) {
        //A1
        driveForwardsTime(50, 0.6);
    } else if (RCS.GetCorrectLever() == 2) {
        //B
        driveForwardsTime(50, 1.0);
    }

    orientation = changeOrientation(orientation);

    driveForwardsTime(-20, 2.6);
    driveForwardsTime(20, 2.6);

    
    orientation = changeOrientation(orientation);
    driveForwardsTime(20, 0.4);
    orientation = changeOrientation(orientation);

    driveForwardsTime(-20, 3.0);
    driveForwardsTime(20, 2.6);
    */





    //return to closed
    //Sleep(5.0);
    //orientation = changeOrientation(orientation);
    
    //set to loose
    servo.SetDegree(160);
    
}
