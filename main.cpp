#include <FEHUtility.h>
#include <FEHServo.h>
#include <FEHLCD.h>
#include <FEHIO.h>
#include <FEHMotor.h>

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
    leftMotor.SetPercent(-power);
    rightMotor.SetPercent(power);

    updateUI(power, power);

    Sleep(time);

    leftMotor.Stop();
    rightMotor.Stop();

    updateUI(0,0);
}

void driveForwards(float power, int counts) {
    rightEncoder.ResetCounts();
    leftEncoder.ResetCounts();

    rightMotor.SetPercent(power);
    leftMotor.SetPercent(-power);

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
    leftMotor.SetPercent(power);
    rightMotor.SetPercent(power);

    updateUI(power, -power);

    Sleep(time);

    leftMotor.Stop();
    rightMotor.Stop();

    updateUI(0,0);
}

void turnRight(float power, float time) {
    leftMotor.SetPercent(-power);
    rightMotor.SetPercent(-power);

    updateUI(-power, power);

    Sleep(time);

    leftMotor.Stop();
    rightMotor.Stop();

    updateUI(0,0);
}

bool changeOrientation(bool orientation) {
    bool newOrientation = orientation;
    if (orientation) {
        leftMotor.SetPercent(-50);
        rightMotor.SetPercent(-50);
        newOrientation = false;
        servo.SetDegree(0.0);
        Sleep(0.2);
        leftMotor.SetPercent(0);
        rightMotor.SetPercent(0);
    } else {
        leftMotor.SetPercent(50);
        rightMotor.SetPercent(50);
        newOrientation = true;
        servo.SetDegree(180.0);
        Sleep(0.2);
        leftMotor.SetPercent(0);
        rightMotor.SetPercent(0);
    }
    updateOrientation(0,0,newOrientation);
    return newOrientation;
}

int main(void) {
    initializeStartup();
    updateUI(0,0);
    bool orientation = true;

    servo.SetMin(1076);
    servo.SetMax(2090);

    
}
