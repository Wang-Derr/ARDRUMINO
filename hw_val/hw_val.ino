/*
 * This file is part of the ARDSEQUINO project.
 *
 * ARDSEQUINO is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * ARDSEQUINO is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with ARDSEQUINO. If not, see <https://www.gnu.org/licenses/>. 
 */

#include <Wire.h> // I2C library
#include <SparkFunSX1509.h> // SX1509_io expander library
#include <Adafruit_GFX.h> // Graphics library
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>  // LED backpack library
#include "hw_val.h"

Adafruit_8x16matrix matrix = Adafruit_8x16matrix();
SX1509 SX1509_io;

volatile bool prev_enc_0_ch0_state;
volatile bool prev_enc_0_ch1_state;
volatile bool prev_enc_0_sw_state;

volatile bool prev_enc_1_ch0_state;
volatile bool prev_enc_1_ch1_state;
volatile bool prev_enc_1_sw_state;

volatile bool prev_enc_2_ch0_state;
volatile bool prev_enc_2_ch1_state;
volatile bool prev_enc_2_sw_state;

volatile bool prev_sw_0_state;

volatile int8_t enc_draw_pos;

volatile bool enc_usage_tracker[3];

volatile bool former_enc_direction;

volatile int prev_anlg_pot0_state;

volatile int prev_anlg_pot1_state;

volatile int prev_anlg_pot2_state;

volatile int prev_anlg_pot3_state;

volatile unsigned long prev_pot_time;

void setup()
{
    Wire.begin();
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW); // Start it as low

    memset(enc_usage_tracker, false, sizeof(enc_usage_tracker));

    former_enc_direction = true;
    
    // Call matrix.begin(<address>) to initialize the HT16K33. If it
    // successfully communicates, it'll return 1.
    if (matrix.begin(HT16K33_ADDR) == false) {
        digitalWrite(LED_BUILTIN, HIGH); // If we failed to communicate, turn the pin 13 LED on
        while (1); // If we fail to communicate, loop forever.
    }

    // Call SX1509_io.begin(<address>) to initialize the SX1509. If it
    // successfully communicates, it'll return 1.
    if (SX1509_io.begin(SX1509_ADDR) == false) {
        digitalWrite(LED_BUILTIN, HIGH); // If we failed to communicate, turn the pin 13 LED on
        while (1); // If we fail to communicate, loop forever.
    }

    

    pinMode(NANO_enc0_ch0, INPUT_PULLUP);
    pinMode(NANO_enc0_ch1, INPUT_PULLUP);
    pinMode(NANO_enc0_sw, INPUT_PULLUP);

    pinMode(NANO_enc1_ch0, INPUT_PULLUP);
    pinMode(NANO_enc1_ch1, INPUT_PULLUP);
    pinMode(NANO_enc1_sw, INPUT_PULLUP);

    pinMode(NANO_enc2_ch0, INPUT_PULLUP);
    pinMode(NANO_enc2_ch1, INPUT_PULLUP);
    pinMode(NANO_enc2_sw, INPUT_PULLUP);

    pinMode(NANO_sw0_pin, INPUT_PULLUP);

    pinMode(SX1509_int_pin, INPUT_PULLUP);
    

    prev_enc_0_ch0_state = digitalRead(NANO_enc0_ch0);
    prev_enc_0_ch1_state = digitalRead(NANO_enc0_ch1);
    prev_enc_0_sw_state = digitalRead(NANO_enc0_sw);

    prev_enc_1_ch0_state = digitalRead(NANO_enc1_ch0);
    prev_enc_1_ch1_state = digitalRead(NANO_enc1_ch1);
    prev_enc_1_sw_state = digitalRead(NANO_enc1_sw);

    prev_enc_2_ch0_state = digitalRead(NANO_enc2_ch0);
    prev_enc_2_ch1_state = digitalRead(NANO_enc2_ch1);
    prev_enc_2_sw_state = digitalRead(NANO_enc2_sw);

    prev_sw_0_state = digitalRead(NANO_sw0_pin);

    // Use a pull-up resistor on the button's input pin. When
    // the button is pressed, the pin will be read as LOW:
    SX1509_io.pinMode(SX1509_sw0_pin, INPUT_PULLUP);
    SX1509_io.enableInterrupt(SX1509_sw0_pin, RISING);
    SX1509_io.pinMode(SX1509_sw1_pin, INPUT_PULLUP);
    SX1509_io.enableInterrupt(SX1509_sw1_pin, RISING);
    SX1509_io.pinMode(SX1509_sw2_pin, INPUT_PULLUP);
    SX1509_io.enableInterrupt(SX1509_sw2_pin, RISING);
    SX1509_io.pinMode(SX1509_sw3_pin, INPUT_PULLUP);
    SX1509_io.enableInterrupt(SX1509_sw3_pin, RISING);
    SX1509_io.pinMode(SX1509_sw4_pin, INPUT_PULLUP);
    SX1509_io.enableInterrupt(SX1509_sw4_pin, RISING);
    SX1509_io.pinMode(SX1509_sw5_pin, INPUT_PULLUP);
    SX1509_io.enableInterrupt(SX1509_sw5_pin, RISING);
    SX1509_io.pinMode(SX1509_sw6_pin, INPUT_PULLUP);
    SX1509_io.enableInterrupt(SX1509_sw6_pin, RISING);
    SX1509_io.pinMode(SX1509_sw7_pin, INPUT_PULLUP);
    SX1509_io.enableInterrupt(SX1509_sw7_pin, RISING);
    SX1509_io.pinMode(SX1509_sw8_pin, INPUT_PULLUP);
    SX1509_io.enableInterrupt(SX1509_sw8_pin, RISING);
    SX1509_io.pinMode(SX1509_sw9_pin, INPUT_PULLUP);
    SX1509_io.enableInterrupt(SX1509_sw9_pin, RISING);
    SX1509_io.pinMode(SX1509_sw10_pin, INPUT_PULLUP);
    SX1509_io.enableInterrupt(SX1509_sw10_pin, RISING);
    SX1509_io.pinMode(SX1509_sw11_pin, INPUT_PULLUP);
    SX1509_io.enableInterrupt(SX1509_sw11_pin, RISING);
    SX1509_io.pinMode(SX1509_sw12_pin, INPUT_PULLUP);
    SX1509_io.enableInterrupt(SX1509_sw12_pin, RISING);
    SX1509_io.pinMode(SX1509_sw13_pin, INPUT_PULLUP);
    SX1509_io.enableInterrupt(SX1509_sw13_pin, RISING);
    SX1509_io.pinMode(SX1509_sw14_pin, INPUT_PULLUP);
    SX1509_io.enableInterrupt(SX1509_sw14_pin, RISING);
    SX1509_io.pinMode(SX1509_sw15_pin, INPUT_PULLUP);
    SX1509_io.enableInterrupt(SX1509_sw15_pin, RISING);

    prev_anlg_pot0_state = analogRead(A0);
    prev_anlg_pot1_state = analogRead(A1);
    prev_anlg_pot2_state = analogRead(A2);
    prev_anlg_pot3_state = analogRead(A3);

    // Run a "Bootup Graphic" on the LED backpack
    matrix.setTextSize(1);
    matrix.setTextWrap(false);  // we dont want text to wrap so it scrolls nicely
    matrix.setTextColor(LED_ON);
    matrix.setRotation(3);
    for (int8_t x=7; x>=-105; x--) { // max x value found thru trial & error
        matrix.clear();
        matrix.setCursor(x,0);
        matrix.print("Bootup Successful!");
        matrix.writeDisplay();
        delay(100);
    }
    PCICR |= B00000101;
    PCMSK0 |= B00011111;
    PCMSK2 |= B11111000;
    attachInterrupt(digitalPinToInterrupt(SX1509_int_pin), sx1509_interrupt, FALLING);
}

void draw_image(const uint8_t *bitmap)
{
    matrix.setRotation(3);
    matrix.clear();
    matrix.drawBitmap(0, 0, bitmap, 16, 8, LED_ON);
    matrix.writeDisplay();  // write the changes we just made to the display
}

ISR (PCINT0_vect)
{
    interrupts();

    if ((digitalRead(NANO_enc2_ch0) != prev_enc_2_ch0_state) || (digitalRead(NANO_enc2_ch1) != prev_enc_2_ch1_state)) {
        read_encoder(NANO_enc2_ch0, NANO_enc2_ch1, 2);
        prev_enc_2_ch0_state = digitalRead(NANO_enc2_ch0);
        prev_enc_2_ch1_state = digitalRead(NANO_enc2_ch1);
    }

    if (digitalRead(NANO_enc2_sw) != prev_enc_2_sw_state) {
        memset(enc_usage_tracker, false, sizeof(enc_usage_tracker));
        draw_image(enc2_bmp);
        prev_enc_2_sw_state = digitalRead(NANO_enc2_sw);
    }

    if (digitalRead(NANO_enc1_sw) != prev_enc_1_sw_state) {
        memset(enc_usage_tracker, false, sizeof(enc_usage_tracker));
        draw_image(enc1_bmp);
        prev_enc_1_sw_state = digitalRead(NANO_enc1_sw);
    }

    if (digitalRead(NANO_sw0_pin) != prev_sw_0_state) {
        memset(enc_usage_tracker, false, sizeof(enc_usage_tracker));
        draw_image(sw0_bmp);
        prev_sw_0_state = digitalRead(NANO_sw0_pin);
    }
}

ISR (PCINT2_vect)
{
    interrupts();
    
    if ((digitalRead(NANO_enc0_ch0) != prev_enc_0_ch0_state) || (digitalRead(NANO_enc0_ch1) != prev_enc_0_ch1_state)) {
        read_encoder(NANO_enc0_ch0, NANO_enc0_ch1, 0);
        prev_enc_0_ch0_state = digitalRead(NANO_enc0_ch0);
        prev_enc_0_ch1_state = digitalRead(NANO_enc0_ch1);
    }

    if ((digitalRead(NANO_enc1_ch0) != prev_enc_1_ch0_state) || (digitalRead(NANO_enc1_ch1) != prev_enc_1_ch1_state)) {
        read_encoder(NANO_enc1_ch0, NANO_enc1_ch1, 1);
        prev_enc_1_ch0_state = digitalRead(NANO_enc1_ch0);
        prev_enc_1_ch1_state = digitalRead(NANO_enc1_ch1);
    }

    if (digitalRead(NANO_enc0_sw) != prev_enc_0_sw_state) {
        memset(enc_usage_tracker, false, sizeof(enc_usage_tracker));
        draw_image(enc0_bmp);
        prev_enc_0_sw_state = digitalRead(NANO_enc0_sw);
    }
}

void encoder_led_mapping(uint8_t enc_num, bool direction) // true for clockwise, false for counter clockwise
{
    static uint8_t x;
    static uint8_t y;
    if (direction) {
        if (enc_usage_tracker[enc_num]) {
            if (direction == former_enc_direction) {
                if (enc_draw_pos < ((int8_t)sizeof(enc_map_table)/2)) {
                    matrix.drawPixel(enc_map_table[enc_draw_pos][0], enc_map_table[enc_draw_pos][1], LED_ON); // draw next pixel
                    matrix.writeDisplay();
                    enc_draw_pos++;
                }
            } else {
                if (enc_draw_pos < ((int8_t)(sizeof(enc_map_table)/2) - 1)) {
                    enc_draw_pos++;
                    matrix.drawPixel(enc_map_table[enc_draw_pos][0], enc_map_table[enc_draw_pos][1], LED_OFF); // erase previous pixel
                    matrix.writeDisplay();
                } else {
                    former_enc_direction = direction;
                    enc_draw_pos = 0;
                }
            }
        } else {
            enc_draw_pos = 0;
            switch (enc_num) {
                case 0:
                    draw_image(enc0_rotate_bmp);
                    break;
                case 1:
                    draw_image(enc1_rotate_bmp);
                    break;
                case 2:
                    draw_image(enc2_rotate_bmp);
                    break;
                default:
                    draw_image(err_bmp);
                    break;
            }
            matrix.drawPixel(enc_map_table[enc_draw_pos][0], enc_map_table[enc_draw_pos][1], LED_ON); // draw first pixel
            matrix.writeDisplay();
            memset(enc_usage_tracker, false, sizeof(enc_usage_tracker));
            enc_usage_tracker[enc_num] = true;
            former_enc_direction = direction;
        }
    } else {
        if (enc_usage_tracker[enc_num]) {
            if (direction == former_enc_direction) {
                if (enc_draw_pos > -1) {
                    matrix.drawPixel(enc_map_table[enc_draw_pos][0], enc_map_table[enc_draw_pos][1], LED_ON); // draw next pixel
                    matrix.writeDisplay();
                    enc_draw_pos--;
                }
            } else {
                if (enc_draw_pos > 0) {
                    enc_draw_pos--;
                    matrix.drawPixel(enc_map_table[enc_draw_pos][0], enc_map_table[enc_draw_pos][1], LED_OFF); // erase previous pixel
                    matrix.writeDisplay();
                } else {
                    former_enc_direction = direction;
                    enc_draw_pos = (int8_t)(sizeof(enc_map_table)/2) - 1;
                }
            }
        } else {
            enc_draw_pos = ((int8_t)sizeof(enc_map_table)/2) - 1;
            switch (enc_num) {
                case 0:
                    draw_image(enc0_rotate_bmp);
                    break;
                case 1:
                    draw_image(enc1_rotate_bmp);
                    break;
                case 2:
                    draw_image(enc2_rotate_bmp);
                    break;
                default:
                    draw_image(err_bmp);
                    break;
            }
            matrix.drawPixel(enc_map_table[enc_draw_pos][0], enc_map_table[enc_draw_pos][1], LED_ON); // draw first pixel
            matrix.writeDisplay();
            memset(enc_usage_tracker, false, sizeof(enc_usage_tracker));
            enc_usage_tracker[enc_num] = true;
            former_enc_direction = direction;
        }
    }
}

void read_encoder(int enc_ch0, int enc_ch1, uint8_t enc_num)
{
    static uint8_t old_AB[] = {3, 3, 3};
    static int8_t encval[] = {0, 0, 0};
    static const int8_t enc_states[] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};

    old_AB[enc_num] <<=2;

    if (digitalRead(enc_ch0)) old_AB[enc_num] |= 0x02;
    if (digitalRead(enc_ch1)) old_AB[enc_num] |= 0x01;

    encval[enc_num] += enc_states[(old_AB[enc_num] & 0x0f)];
    if (encval[enc_num] > 3 or encval[enc_num] < -3) {
        int changevalue = (encval[enc_num] > 0) - (encval[enc_num] < 0);
        if (changevalue == 1) {
            encoder_led_mapping(enc_num, true);
        } else {
            encoder_led_mapping(enc_num, false);
        }
        encval[enc_num] = 0;
    }
}

void sx1509_interrupt() {
    interrupts();
    memset(enc_usage_tracker, false, sizeof(enc_usage_tracker));
    uint16_t intSrc = SX1509_io.interruptSource();
    if (intSrc == 1) {
        draw_image(sw_1_bmp);
    } else if (intSrc == 2) {
        draw_image(sw_2_bmp);
    } else if (intSrc == 4) {
        draw_image(sw_3_bmp);
    } else if (intSrc == 8) {
        draw_image(sw_4_bmp);
    } else if (intSrc == 16) {
        draw_image(sw_5_bmp);
    } else if (intSrc == 32) {
        draw_image(sw_6_bmp);
    } else if (intSrc == 64) {
        draw_image(sw_7_bmp);
    } else if (intSrc == 128) {
        draw_image(sw_8_bmp);
    } else if (intSrc == 256) {
        draw_image(sw_9_bmp);
    } else if (intSrc == 512) {
        draw_image(sw_10_bmp);
    } else if (intSrc == 1024) {
        draw_image(sw_11_bmp);
    } else if (intSrc == 2048) {
        draw_image(sw_12_bmp);
    } else if (intSrc == 4096) {
        draw_image(sw_13_bmp);
    } else if (intSrc == 8192) {
        draw_image(sw_14_bmp);
    } else if (intSrc == 16384) {
        draw_image(sw_15_bmp);
    } else if (intSrc == 32768) {
        draw_image(sw_16_bmp);
    } else {
        draw_image(err_bmp);
    }
}

void analog_potentiometer_led(void)
{
    static int pot_level;
    if ((millis() - prev_pot_time) > 50) {
        if (abs(analogRead(A0) - prev_anlg_pot0_state) > 32) {
            draw_image(pot0_rotate_bmp);
            pot_level = 15 - (analogRead(A0)/ 64); //polarity of my Potentiometers is reversed...
            if (analogRead(A0) > 991) {
                matrix.drawLine(0, 0, 15, 0, LED_OFF);
            } else {
                matrix.drawLine(0, 0, pot_level, 0, LED_ON);
            }
            matrix.writeDisplay();
            prev_anlg_pot0_state = analogRead(A0);
        } else if (abs(analogRead(A1) - prev_anlg_pot1_state) > 32) {
            draw_image(pot1_rotate_bmp);
            pot_level = 15 - (analogRead(A1)/ 64);
            if (analogRead(A1) > 991) {
                matrix.drawLine(0, 0, 15, 0, LED_OFF);
            } else {
                matrix.drawLine(0, 0, pot_level, 0, LED_ON);
            }
            matrix.writeDisplay();
            prev_anlg_pot1_state = analogRead(A1);
        } else if (abs(analogRead(A2) - prev_anlg_pot2_state) > 32) {
            draw_image(pot2_rotate_bmp);
            pot_level = 15 - (analogRead(A2)/ 64);
            if (analogRead(A2) > 991) {
                matrix.drawLine(0, 0, 15, 0, LED_OFF);
            } else {
                matrix.drawLine(0, 0, pot_level, 0, LED_ON);
            }
            matrix.writeDisplay();
            prev_anlg_pot2_state = analogRead(A2);
        } else if (abs(analogRead(A3) - prev_anlg_pot3_state) > 32) {
            draw_image(pot3_rotate_bmp);
            pot_level = 15 - (analogRead(A3)/ 64);
            if (analogRead(A3) > 991) {
                matrix.drawLine(0, 0, 15, 0, LED_OFF);
            } else {
                matrix.drawLine(0, 0, pot_level, 0, LED_ON);
            }
            matrix.writeDisplay();
            prev_anlg_pot3_state = analogRead(A3);
        }
        prev_pot_time = millis();
    }
}

void loop() {
    analog_potentiometer_led();
}