/*

SSD1306 - Screen module

Copyright (C) 2018 by Xose Pérez <xose dot perez at gmail dot com>


This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <Wire.h>
#include "SSD1306Wire.h"
#include "OLEDDisplay.h"
#include "images.h"
#include "fonts.h"
#include "configuration.h"

#define SCREEN_HEADER_HEIGHT    14
#define SCREEN_POSITION_HEIGHT  15
#define SCREEN_SCROLLVIEW_POS   30

#define SCREEN_LINE_COUNT       4
#define SCREEN_BUFFER_SIZE      40

SSD1306Wire * display;

char screen_buffer[SCREEN_LINE_COUNT][SCREEN_BUFFER_SIZE] = {0};

void screen_ShowPosition() 
{
    if(!display) return;

    char buffer[40];

    snprintf(buffer, sizeof(buffer), "%.2f %.2f %.1f", gps_latitude(), gps_longitude(), gps_altitude());
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    display->drawString(0, SCREEN_POSITION_HEIGHT, buffer);

    // Satellite count
    display->setTextAlignment(TEXT_ALIGN_RIGHT);
    display->drawString(display->getWidth() - SATELLITE_IMAGE_WIDTH - 4, SCREEN_POSITION_HEIGHT, itoa(gps_sats(), buffer, 10));
    display->drawXbm(display->getWidth() - SATELLITE_IMAGE_WIDTH, SCREEN_POSITION_HEIGHT, SATELLITE_IMAGE_WIDTH, SATELLITE_IMAGE_HEIGHT, SATELLITE_IMAGE);
}

void screen_ShowHeader() 
{
    if(!display) return;

    char buffer[20];

    // clock
    gps_time(buffer, sizeof(buffer));
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    display->drawString(0, 0, buffer);

    // Message count
    snprintf(buffer, sizeof(buffer), "#%03d", ttn_get_count() % 1000);
    display->setTextAlignment(TEXT_ALIGN_CENTER);
    display->drawString(display->getWidth() / 2, 0, buffer);

     #ifdef T_BEAM_V10
    // Datetime (if the axp192 PMIC is present, alternate between powerstats and time)
    if(axp192_found && millis()%8000 < 3000)
    {
        snprintf(buffer, sizeof(buffer), "%.1fV %.0fmA", axp.getBattVoltage()/1000, axp.getBattChargeCurrent() - axp.getBattDischargeCurrent());
    } 
    else 
    {
        gps_time(buffer, sizeof(buffer));
    }
    #endif

    #ifdef T_BEAM_V12
        snprintf(buffer, sizeof(buffer), "%s %d%%", power_getChargerStatus(),power_getBatteryLevel());  
    #endif

    display->setTextAlignment(TEXT_ALIGN_RIGHT);
    display->drawString(display->getWidth() - 1, 0, buffer);
}

void screen_show_logo() 
{
    if(!display) return;

    uint8_t x = (display->getWidth() - TTN_IMAGE_WIDTH) / 2;
    uint8_t y = SCREEN_HEADER_HEIGHT + (display->getHeight() - SCREEN_HEADER_HEIGHT - TTN_IMAGE_HEIGHT) / 2 + 1;
    display->drawXbm(x, y, TTN_IMAGE_WIDTH, TTN_IMAGE_HEIGHT, TTN_IMAGE);
}

void screen_off() 
{
    if(!display) return;

    display->displayOff();
}

void screen_on() 
{
    if(!display) return;

    display->displayOn();
}

void screen_clear() 
{
    if(!display) return;

    display->clear();
}

void screen_print(const char * text, uint8_t x, uint8_t y, uint8_t alignment) 
{
    DEBUG_MSG(text);

    if(!display) return;

    display->setTextAlignment((OLEDDISPLAY_TEXT_ALIGNMENT) alignment);
    display->drawString(x, y, text);
}

void screen_print(const char * text, uint8_t x, uint8_t y) 
{
    screen_print(text, x, y, TEXT_ALIGN_LEFT);
}

void screen_print(const char * text) 
{
    if(!display) return;

    strncpy(screen_buffer[0], screen_buffer[1], SCREEN_BUFFER_SIZE - 1);
    strncpy(screen_buffer[1], screen_buffer[2], SCREEN_BUFFER_SIZE - 1);
    strncpy(screen_buffer[2], screen_buffer[3], SCREEN_BUFFER_SIZE - 1);
    strncpy(screen_buffer[3], text, SCREEN_BUFFER_SIZE - 1);
    screen_buffer[3][SCREEN_BUFFER_SIZE - 1] = '\0';
}

void screen_ShowScrollView() 
{
    if(!display) return;

    // Draw the string at the current line
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    display->drawString(0, SCREEN_SCROLLVIEW_POS, screen_buffer[0]);
    display->drawString(0, SCREEN_SCROLLVIEW_POS + 8, screen_buffer[1]);
    display->drawString(0, SCREEN_SCROLLVIEW_POS + 16, screen_buffer[2]);
    display->drawString(0, SCREEN_SCROLLVIEW_POS + 24, screen_buffer[3]);
}

void screen_update() 
{
    if (display) display->display();
}

void screen_setup() 
{
    // Display instance
    display = new SSD1306Wire(SSD1306_ADDRESS, I2C_SDA, I2C_SCL);
    display->init();
    display->flipScreenVertically();
    display->setFont(Custom_ArialMT_Plain_10);
}

void screen_loop() 
{
    if(!display) return;

    #ifdef T_BEAM_V10
    if (axp192_found && pmu_irq) {
        pmu_irq = false;
        axp.readIRQ();
        if (axp.isChargingIRQ()) {
            baChStatus = "Charging";
        } else {
            baChStatus = "No Charging";
        }
        if (axp.isVbusRemoveIRQ()) {
            baChStatus = "No Charging";
        }
        Serial.println(baChStatus);
        digitalWrite(2, !digitalRead(2));
        axp.clearIRQ();
    }
    #endif

    #ifdef T_BEAM_V12
    
    #endif

    display->clear();
    screen_ShowHeader();
    screen_ShowPosition();
    screen_ShowScrollView();
    display->display();
}
