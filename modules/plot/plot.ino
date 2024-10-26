#include <heltec-eink-modules.h>
EInkDisplay_VisionMasterE290 display;

void setup() {
  display.setRotation(90);                // Rotate 90deg clocklwise - more room
  display.clearMemory();
  drawAxes();
  drawPlot();
  // drawTank(30);
  // display.update();
  // delay(2000);
  drawTank(70);
  display.update();
}

void loop()
{
}

void drawAxes() {
  display.drawRect(24, 14, 140, 101, BLACK);
  display.drawLine(19, 64, 24, 64, BLACK);
  display.setCursor(1, 60);
  display.print(" 50");
  display.drawLine(19, 64 - 50, 24, 64 - 50, BLACK);
  display.setCursor(1, 60 - 50);
  display.print("100");
  display.drawLine(19, 64 + 50, 24, 64 + 50, BLACK);
  display.setCursor(1, 60 + 50);
  display.print("  0");
  display.drawLine(21, 64 - 25, 24, 64 - 25, BLACK);
  display.setCursor(1, 60 - 25);
  display.print(" 75");
  display.drawLine(21, 64 + 25, 24, 64 + 25, BLACK);
  display.setCursor(1, 60 + 25);
  display.print(" 25");
}

void drawPlot() {
  unsigned char time_series[140] = {
    50, 52, 55, 57, 56, 58, 60, 63, 61, 60, 58, 59, 61, 63, 64, 62,
    61, 60, 59, 58, 60, 61, 63, 62, 64, 65, 67, 65, 66, 68, 67, 65,
    63, 62, 64, 66, 68, 67, 66, 64, 63, 62, 61, 60, 59, 61, 63, 62,
    61, 60, 59, 57, 56, 55, 53, 52, 51, 50, 52, 54, 56, 55, 57, 58,
    60, 62, 63, 61, 62, 63, 65, 66, 64, 63, 62, 61, 59, 57, 58, 56,
    55, 53, 51, 50, 48, 49, 51, 52, 54, 53, 52, 50, 48, 47, 49, 51,
    50, 52, 53, 54, 56, 58, 57, 59, 58, 57, 55, 56, 54, 53, 51, 50,
    52, 53, 51, 49, 47, 48, 46, 44, 43, 45, 47, 48, 49, 50, 51, 50,
    48, 49, 51, 52, 54, 55, 56, 57, 56, 55, 54, 52
  };
  for (int i = 1; i < 140; i++) {
    unsigned char offset_x = 24;
    unsigned char offset_y = 14;
    display.drawLine(i - 1 + offset_x, 100 - time_series[i - 1] + offset_y,
                     i + offset_x, 100 - time_series[i] + offset_y, BLACK);
  }
}

void drawTank(unsigned char percent) {
  display.drawRect(180, 60, 100, 50, BLACK);
  display.fillRect(180, 60 + 50 - percent / 2, 100, percent / 2, BLACK);
  display.setTextSize(3);
  String text = String(percent) + " %";
  display.setCursor(200, 30);
  display.print(text);

}
