#include "display.hpp"
#include <U8g2lib.h>

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

TaskHandle_t DisplayTask;

#define SLEEP_AFTER_MS 30000 // sleep after 30 seconds

const char* getStatusMessage(int status);

void centerPrintToScreen(const char *str, u8g2_uint_t y) {
    u8g2_uint_t width = u8g2.getStrWidth(str);
    u8g2.setCursor(64 - width / 2, y);  // Center calculation simplified
    u8g2.print(str);
}

void displayScaleStatus() {
    char buf[64];
    u8g2.setFont(u8g2_font_7x13_tr);
    centerPrintToScreen(getStatusMessage(scaleStatus), 14);

    snprintf(buf, sizeof(buf), "%3.1fg", scaleWeight - (scaleStatus == STATUS_EMPTY ? 0 : cupWeightEmpty));
    centerPrintToScreen(buf, 48);

}

const char* getStatusMessage(int status) {
    switch (status) {
        case STATUS_EMPTY:
            return "Weight:";
        case STATUS_GRINDING_IN_PROGRESS:
            return "Grinding...";
        case STATUS_GRINDING_FAILED:
            return "Grinding failed";
        case STATUS_GRINDING_FINISHED:
            return "Grinding finished";
        default:
            return "Unknown status";
    }
}

void updateDisplay(void *parameter) {
    char buf[64];
    unsigned long lastUpdate = 0;

    for (;;) {
        if (millis() - lastUpdate < 100) {  // Update every 100 ms
            vTaskDelay(10 / portTICK_PERIOD_MS);
            continue;
        }
        lastUpdate = millis();

        u8g2.clearBuffer();
        if (millis() - lastSignificantWeightChangeAt > SLEEP_AFTER_MS) {
            u8g2.sendBuffer();
            continue;
        }

        switch (scaleStatus) {
            case STATUS_EMPTY:
            case STATUS_GRINDING_IN_PROGRESS:
            case STATUS_GRINDING_FAILED:
            case STATUS_GRINDING_FINISHED:
                displayScaleStatus();
                break;
            default:
                u8g2.setFontPosTop();
                u8g2.drawStr(0, 20, "Init...");
                break;
        }
        u8g2.sendBuffer();
    }
}

void setupDisplay() {
    u8g2.begin();
    xTaskCreatePinnedToCore(
        updateDisplay, /* Function to implement the task */
        "Display", /* Name of the task */
        10000,  /* Stack size in words */
        NULL,  /* Task input parameter */
        0,  /* Priority of the task */
        &DisplayTask,  /* Task handle. */
        1); /* Core where the task should run */
}
