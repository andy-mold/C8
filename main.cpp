#include <FEHUtility.h>
#include <FEHServo.h>
#include <FEHLCD.h>
#include <FEHIO.h>
#include <FEHMotor.h>

#define SCREENWIDTH 320
#define SCREENHEIGHT 240

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

    LCD.WriteAt("LD",textBuffer,textBuffer);
    LCD.WriteAt("RD",screenWidth-columnWidth+textBuffer,textBuffer);

    LCD.DrawHorizontalLine(rowHeight,0,columnWidth);
    LCD.DrawHorizontalLine(rowHeight,screenWidth-columnWidth,screenWidth);

    LCD.DrawHorizontalLine(rowHeight,columnWidth,columnWidth+middleWidth);
    LCD.WriteAt("F/B",columnWidth+textBuffer,textBuffer);
    LCD.WriteAt("L/R",screenWidth-columnWidth-45,textBuffer);

    LCD.DrawRectangle(columnWidth*2,textBuffer,checkboxLength,checkboxLength);
    LCD.DrawRectangle(screenWidth-columnWidth*2-checkboxLength,textBuffer,checkboxLength,checkboxLength);
    LCD.SetFontColor(GREEN);
    LCD.FillRectangle(columnWidth*2,textBuffer,checkboxLength,checkboxLength);
}

void uptdateUI() {

}

int main(void) {
    initializeStartup();
    initializeUI();
}