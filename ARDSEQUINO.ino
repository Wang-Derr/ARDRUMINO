/*
 * This file is part of the ARDSEQUINO project.
 *
 * ARDSEQUINO is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * ARDSEQUINO is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with ARDSEQUINO. If not, see <https://www.gnu.org/licenses/>. 
 */

#include <Wire.h> // I2C library, required for communicating with SX1509 and LED backpack
#include <SparkFunSX1509.h> // SX1509_io expander library
#include <Adafruit_GFX.h> // Graphics library
#include <Adafruit_LEDBackpack.h>  // LED backpack library
#include <MIDI.h> // MIDI library
#include "ARDSEQUINO.h" // project header file

Adafruit_8x16matrix matrix = Adafruit_8x16matrix(); // initialize the LED panel
SX1509 SX1509_io; // initialize the SX1509 io expander
MIDI_CREATE_DEFAULT_INSTANCE(); // initialize MIDI comms

// encoder state vars
volatile bool prev_enc0_ch0_state;
volatile bool prev_enc0_ch1_state;
volatile bool prev_enc1_ch0_state;
volatile bool prev_enc1_ch1_state;
volatile bool prev_enc2_ch0_state;
volatile bool prev_enc2_ch1_state;

// interrupt flags
volatile bool sx1509_int_flag = false;
volatile bool sw0_flag = false;
volatile bool enc0_knob_flag = false;
volatile bool enc0_sw_flag = false;
volatile bool enc1_knob_flag = false;
volatile bool enc1_sw_flag = false;
volatile bool enc2_knob_flag = false;
volatile bool enc2_sw_flag = false;

// analog potentiometer position vars
analog_potentiometers_t anlg_pot[4];

// Debounce tracking vars
unsigned long sx1509_int_pin_last_trig = millis();
unsigned long NANO_sw0_last_trig = millis();
unsigned long enc0_sw_last_trig = millis();
unsigned long enc0_knob_last_trig = millis();
unsigned long enc1_sw_last_trig = millis();
unsigned long enc1_knob_last_trig = millis();
unsigned long enc2_sw_last_trig = millis();
unsigned long enc2_knob_last_trig = millis();
unsigned long prev_pot_time = millis();

// button hold duration tracking var
unsigned long sw0_last_pressed = millis();

// array for storing the debounce timing for all 16 SX1509 buttons
sx1509_pins_t sx1509_pin[SX1509_PIN_CT];

// array for storing the parameters of the 14 keys
sound_properties_t key_array[MAX_POLYPHONY];

// struct for storing Sequencer parameters
global_sequencer_menu_t global_seq;

// array stores sequence itself
uint16_t sequencer_array[MAX_SEQUENCER_LENGTH] = {0};

uint16_t prev_sequencer_step_val = 0;

// There are two menus, one for general sequencer control, one for key-specific parameter changes
uint8_t menu_mode = GLOBAL_SEQUENCER_MODE;

// BPM related vars
unsigned long prev_seq_time = millis(); // tracks last sequencer step timing
unsigned long prev_clock_tick = micros(); // tracks midi clock signal
unsigned long bpm_as_ms = 60000 / (global_seq.bpm + 45) / global_seq.npb; // converts BPM to ms per sequencer step
unsigned long us_per_tick = 60000000 / (global_seq.bpm + 45) / 4 / 24; // calculate microseconds per quarternote for midi clock sync based on bpm

// buffers for loading data stored in program memory
uint8_t digit0_buf[16];
uint8_t digit1_buf[16];
uint8_t digit2_buf[16];
uint8_t bitmap_buf[16];

void setup()
{
    MIDI.begin(MIDI_CHANNEL_OMNI); // Determines which MIDI channel to broadcast on
    Wire.begin(); // Enable I2C comms
    pinMode(LED_BUILTIN, OUTPUT); // onboard LED enabled for debug
    digitalWrite(LED_BUILTIN, LOW);
    
    // successful LED backpack init returns 1
    if (matrix.begin(HT16K33_ADDR) == false) {
        digitalWrite(LED_BUILTIN, HIGH); // Failing to communicate with the LED backpack will turn on the onboard LED
        while (1); // loop forever if we can't communicate with LED  backpack
    }

    // successful SX1509 init returns 1
    if (SX1509_io.begin(SX1509_ADDR) == false) {
        digitalWrite(LED_BUILTIN, HIGH); // Failing to communicate with the SX1509 will turn on the onboard LED
        while (1);
    } 

    // encoder init step
    pinMode(NANO_enc0_ch0, INPUT_PULLUP);
    pinMode(NANO_enc0_ch1, INPUT_PULLUP);
    pinMode(NANO_enc0_sw, INPUT_PULLUP);
    prev_enc0_ch0_state = digitalRead(NANO_enc0_ch0);
    prev_enc0_ch1_state = digitalRead(NANO_enc0_ch1);

    pinMode(NANO_enc1_ch0, INPUT_PULLUP);
    pinMode(NANO_enc1_ch1, INPUT_PULLUP);
    pinMode(NANO_enc1_sw, INPUT_PULLUP);
    prev_enc1_ch0_state = digitalRead(NANO_enc1_ch0);
    prev_enc1_ch1_state = digitalRead(NANO_enc1_ch1);

    pinMode(NANO_enc2_ch0, INPUT_PULLUP);
    pinMode(NANO_enc2_ch1, INPUT_PULLUP);
    pinMode(NANO_enc2_sw, INPUT_PULLUP);
    prev_enc2_ch0_state = digitalRead(NANO_enc2_ch0);
    prev_enc2_ch1_state = digitalRead(NANO_enc2_ch1);

    // one of the pins onboard the nano is used as a switch
    pinMode(NANO_sw0_pin, INPUT_PULLUP);

    // init the interrupt pin that the SX1509 relies on to report button presses
    pinMode(SX1509_int_pin, INPUT_PULLUP);

    // map the keys to their associated LED positions, may vary from user to user depending on how the keys are wired
    key_array[0].led_pos[0] = 0; // 0xxxxxx
    key_array[0].led_pos[1] = 6; // xxxxxxx

    key_array[1].led_pos[0] = 1; // x0xxxxx
    key_array[1].led_pos[1] = 6; // xxxxxxx

    key_array[2].led_pos[0] = 2; // xx0xxxx
    key_array[2].led_pos[1] = 6; // xxxxxxx

    key_array[3].led_pos[0] = 3; // xxx0xxx
    key_array[3].led_pos[1] = 6; // xxxxxxx

    key_array[11].led_pos[0] = 4; // xxxx0xx
    key_array[11].led_pos[1] = 6; // xxxxxxx

    key_array[12].led_pos[0] = 5; // xxxxx0x
    key_array[12].led_pos[1] = 6; // xxxxxxx

    key_array[13].led_pos[0] = 6; // xxxxxx0
    key_array[13].led_pos[1] = 6; // xxxxxxx

    key_array[4].led_pos[0] = 0; // xxxxxxx
    key_array[4].led_pos[1] = 7; // 0xxxxxx

    key_array[5].led_pos[0] = 1; // xxxxxxx
    key_array[5].led_pos[1] = 7; // x0xxxxx

    key_array[6].led_pos[0] = 2; // xxxxxxx
    key_array[6].led_pos[1] = 7; // xx0xxxx

    key_array[7].led_pos[0] = 3; // xxxxxxx
    key_array[7].led_pos[1] = 7; // xxx0xxx

    key_array[8].led_pos[0] = 4; // xxxxxxx
    key_array[8].led_pos[1] = 7; // xxxx0xx

    key_array[9].led_pos[0] = 5; // xxxxxxx
    key_array[9].led_pos[1] = 7; // xxxxx0x

    key_array[10].led_pos[0] = 6; // xxxxxxx
    key_array[10].led_pos[1] = 7; // xxxxxx0

    // init each button on the SX1509 and enable interrupts for them
    for (uint8_t i = 0; i < SX1509_PIN_CT; i++) {
        SX1509_io.pinMode(i, INPUT_PULLUP);
        SX1509_io.enableInterrupt(i, CHANGE);
    }

    // init the analog potentiometers
    anlg_pot[0].pinNum = NANO_pot_0;
    anlg_pot[0].state = analogRead(anlg_pot[0].pinNum);
    anlg_pot[0].bitmap = pot0_rotate_bmp;
    anlg_pot[1].pinNum = NANO_pot_1;
    anlg_pot[1].state = analogRead(anlg_pot[1].pinNum);
    anlg_pot[1].bitmap = pot1_rotate_bmp;
    anlg_pot[2].pinNum = NANO_pot_2;
    anlg_pot[2].state = analogRead(anlg_pot[2].pinNum);
    anlg_pot[2].bitmap = pot2_rotate_bmp;
    anlg_pot[3].pinNum = NANO_pot_3;
    anlg_pot[3].state = analogRead(anlg_pot[3].pinNum);
    anlg_pot[3].bitmap = pot3_rotate_bmp;

    // set the seed for random() to the value of a floating analog pin
    randomSeed(A7);

    // Run a bootup graphic on the LED backpack
    matrix.setTextSize(1);
    matrix.setTextWrap(false);  // text will scroll so disable wrapping
    matrix.setTextColor(LED_ON);
    matrix.setRotation(LED_ORIENTATION);
    for (int8_t x=7; x>=-60; x--) { // max x value found thru trial & error
        matrix.clear();
        matrix.setCursor(x,0);
        matrix.print("ARDSEQUINO");
        matrix.writeDisplay();
        delay(100);
    }

    // Init interrupts for both PCINTs and standard interrupt pins
    PCICR |= B00000101;
    PCMSK0 |= B00011111;
    PCMSK2 |= B11111000;
    attachInterrupt(digitalPinToInterrupt(SX1509_int_pin), sx1509_interrupt, FALLING);

    // default display image when boot up is complete
    matrix.clear();
    matrix.drawPixel(0, 0, LED_ON);
    matrix.drawPixel(14, 6, LED_ON);
    matrix.drawLine(10, 6, 10, 7, LED_ON);
    matrix.drawLine(8, 6, 8, 7, LED_ON);
    matrix.writeDisplay();
}

// Interrupt handling for digital inputs 8-12
ISR (PCINT0_vect)
{
    if ((digitalRead(NANO_enc2_ch0) != prev_enc2_ch0_state) || (digitalRead(NANO_enc2_ch1) != prev_enc2_ch1_state)) {
        enc2_knob_flag = true;
        prev_enc2_ch0_state = digitalRead(NANO_enc2_ch0);
        prev_enc2_ch1_state = digitalRead(NANO_enc2_ch1);
        enc2_knob_last_trig = millis();
    }

    if (digitalRead(NANO_enc2_sw) == LOW) {
        enc2_sw_flag = true;
    }

    if (digitalRead(NANO_enc1_sw) == LOW) {
        enc1_sw_flag = true;
    }

    if (digitalRead(NANO_sw0_pin) == LOW && sw0_flag == false) {

        sw0_flag = true;
    }
}

// Interrupt handling for digital inputs 3-7
ISR (PCINT2_vect)
{
    if ((digitalRead(NANO_enc0_ch0) != prev_enc0_ch0_state) || (digitalRead(NANO_enc0_ch1) != prev_enc0_ch1_state)) {
        enc0_knob_flag = true;
        prev_enc0_ch0_state = digitalRead(NANO_enc0_ch0);
        prev_enc0_ch1_state = digitalRead(NANO_enc0_ch1);
        enc0_knob_last_trig = millis();
    }

    if ((digitalRead(NANO_enc1_ch0) != prev_enc1_ch0_state) || (digitalRead(NANO_enc1_ch1) != prev_enc1_ch1_state)) {
        enc1_knob_flag = true;
        prev_enc1_ch0_state = digitalRead(NANO_enc1_ch0);
        prev_enc1_ch1_state = digitalRead(NANO_enc1_ch1);
        enc1_knob_last_trig = millis();
    }

    if (digitalRead(NANO_enc0_sw) == LOW) {
        enc0_sw_flag = true;
    }
}

/*
 * Function: sx1509_interrupt
 * Description: called by interrupt to set a flag that is checked in the main loop
 */

void sx1509_interrupt()
{
    sx1509_int_flag = true;
}

/*
 * Function: draw_image
 * Description: draws any given bitmap on the LED backpack
 * Input:
 *    *bitmap - an array of 16 uint8_t size elements that map out which LEDs on an LED backpack to turn on
 */

void draw_image(const uint8_t *bitmap)
{
    matrix.setRotation(LED_ORIENTATION);
    matrix.clear();
    matrix.drawBitmap(0, 0, bitmap, 16, 8, LED_ON);
    matrix.writeDisplay();  // write the changes we just made to the display
}

/*
 * Function: load_bitmap
 * Description: translates decimal values to a bitmap that can be displayed on the LED backpack
 * Input:
 *    numVal - decimal value that will be displayed on the LED backpack
 */

void load_bitmap(uint16_t numVal)
{
    PROGMEM_readAnything (&digit0[numVal % 10], digit0_buf);
    PROGMEM_readAnything (&digit1[(numVal / 10) % 10], digit1_buf);
    PROGMEM_readAnything (&digit2[(numVal / 100) % 10], digit2_buf);
    for (uint8_t i = 0; i < 16; i++) {
        bitmap_buf[i] = digit2_buf[i] | digit1_buf[i] | digit0_buf[i];
    }
    matrix.fillRect(4, 0, 11, 5, LED_OFF);
    matrix.drawBitmap(0, 0, bitmap_buf, 16, 8, LED_ON);
    matrix.writeDisplay();
}

/*
 * Function: bpm_direction
 * Description: Draws a line to denote the direction of the BPM
 */

void bpm_direction()
{
  if (!global_seq.direction) {
      matrix.drawLine(0, 2, 2, 2, LED_ON);
  } else {
      matrix.drawLine(0, 2, 2, 2, LED_OFF);
  }
  matrix.writeDisplay();
}

/*
 * Function: enc_16bit_val_calc
 * Description: Handler for dec/incrementing 16-bit values based on encoder inputs
 * Input:
 *    direction - true == clockwise, false == counter-clockwise
 *    ＊val - pointer to a param that is controlled by the encoder
 *    max_val - maximum value before wrapping around to minimum value
 *  　min_val - minimum value before wrapping around to maximum value
 */

void enc_16bit_val_calc(bool direction, uint16_t* val, uint16_t max_val, uint16_t min_val)
{
    if (direction) {
        if (*val < max_val) {
            *val += 1;
        } else {
            *val = min_val;
        }
    } else {
        if (*val > min_val) {
            *val -= 1;
        } else {
           *val = max_val;
        }
    }
}

/*
 * Function: enc_8bit_val_calc
 * Description: Handler for dec/incrementing 8-bit values based on encoder inputs
 * Input:
 *    direction - true == clockwise, false == counter-clockwise
 *    ＊val - pointer to a param that is controlled by the encoder
 *    max_val - maximum value before wrapping around to minimum value
 *  　min_val - minimum value before wrapping around to maximum value
 */

void enc_8bit_val_calc(bool direction, uint8_t* val, uint8_t max_val, uint8_t min_val)
{
    if (direction) {
        if (*val < max_val) {
            *val += 1;
        } else {
            *val = min_val;
        }
    } else {
        if (*val > min_val) {
            *val -= 1;
        } else {
           *val = max_val;
        }
    }
}

/*
 * Function: encoder_led_mapping
 * Description: maps encoder inputs to system parameters
 * Input:
 *    enc_num - There are 4 rotary encoders, valid values 0-3
 *    direction - true == clockwise, false == counter-clockwise
 */

void encoder_led_mapping(uint8_t enc_num, bool direction) // true for clockwise, false for counter clockwise
{
    if (menu_mode == GLOBAL_SEQUENCER_MODE) {
        switch(enc_num) {
            case KIT_ENCODER:
                if (digitalRead(NANO_sw0_pin) == LOW) {  // Change MIDI channel when shift is being held
                    matrix.fillRect(0, 0, 16, 6, LED_OFF); // Clear the top 6 LED rows, not run at the beginning of the func due to not all knobs being fully implemented at the moment
                    enc_8bit_val_calc(direction, &global_seq.midi_chan, MAX_MIDI_CHANNEL, 1);
                    load_bitmap(global_seq.midi_chan);
                } else { // Change program change bank
                    matrix.fillRect(0, 0, 16, 6, LED_OFF);
                    enc_8bit_val_calc(direction, &global_seq.PCNum, MAX_PC_BANK, 0);
                    load_bitmap(global_seq.PCNum);
                    MIDI.sendProgramChange(global_seq.PCNum, global_seq.midi_chan);
                }
                break;
            case SEQUENCE_LENGTH_ENCODER:
                if (digitalRead(NANO_sw0_pin) == LOW) { // Functionality TBD
                    // WIP - Have ideas for this knob? Feel free to make a suggestion or fork this repo to give it a shot yourself!
                } else { // Adjust sequencer length
                    matrix.fillRect(0, 0, 16, 6, LED_OFF);
                    enc_16bit_val_calc(direction, &global_seq.length, MAX_SEQUENCER_LENGTH, 1);
                    load_bitmap(global_seq.length);
                }
                break;
            case BPM_ENCODER:
                if (digitalRead(NANO_sw0_pin) == LOW) { // Adjusts the number of notes per beat when shift is held
                    matrix.fillRect(0, 0, 16, 6, LED_OFF);
                    enc_8bit_val_calc(direction, &global_seq.npb, MAX_NOTES_PER_BEAT, 1);
                    load_bitmap(global_seq.npb);
                    bpm_as_ms = 60000 / (global_seq.bpm + 45) / global_seq.npb;
                } else { // Adjust the BPM
                    matrix.fillRect(0, 0, 16, 6, LED_OFF);
                    enc_8bit_val_calc(direction, &global_seq.bpm, MAX_BPM, 0);
                    load_bitmap(global_seq.bpm + 45);
                    bpm_direction();
                    bpm_as_ms = 60000 / (global_seq.bpm + 45) / global_seq.npb;
                }
                break;
            default:
                break;
        }
    } else if (menu_mode == DETAILED_PARAM_MODE) {
        switch(enc_num) {
            case KIT_ENCODER:
                if (digitalRead(NANO_sw0_pin) == LOW) { // Set MIDI channel per button when shift is held
                    matrix.fillRect(0, 0, 16, 6, LED_OFF);
                    draw_image(enc0_alt_rotate_bmp);
                    enc_8bit_val_calc(direction, &key_array[global_seq.last_key].midi_chan, MAX_MIDI_CHANNEL, 1);
                    load_bitmap(key_array[global_seq.last_key].midi_chan);
                } else { // Select the MIDI note associated with a button
                    matrix.fillRect(0, 0, 16, 6, LED_OFF);
                    draw_image(enc0_rotate_bmp);
                    MIDI.sendNoteOff(key_array[global_seq.last_key].midi_note, 0, key_array[global_seq.last_key].midi_chan); // silence a key before switching to another
                    enc_8bit_val_calc(direction, &key_array[global_seq.last_key].midi_note, MAX_MIDI_NOTE, 0);
                    load_bitmap(key_array[global_seq.last_key].midi_note);
                }
                break;
            case SEQUENCE_LENGTH_ENCODER:
                if (digitalRead(NANO_sw0_pin) == LOW) { // Functionality TBD
                    // WIP - Have ideas for this knob? Feel free to make a suggestion or fork this repo to give it a shot yourself!
                } else { // Adjust the probability of this midi note in-sequence
                    matrix.fillRect(0, 0, 16, 6, LED_OFF);
                    draw_image(enc1_rotate_bmp);
                    enc_8bit_val_calc(direction, &key_array[global_seq.last_key].probability, MAX_PROBABILITY, 1);
                    load_bitmap(key_array[global_seq.last_key].probability);
                }
                break;
            case BPM_ENCODER:
                if (digitalRead(NANO_sw0_pin) == LOW) { // Functionality TBD
                    // WIP - Have ideas for this knob? Feel free to make a suggestion or fork this repo to give it a shot yourself!
                } else { // Set button specific BPM for polyrythms, WIP
                    // key specific bpm - Reach Goal
                    // draw_image(enc2_rotate_bmp);
                    // enc_8bit_val_calc(direction, &key_array[global_seq.last_key].local_bpm, MAX_BPM, 0);
                    // load_bitmap(key_array[global_seq.last_key].local_bpm);
                }
                break;
            default:
                break;
        }
    }
}

/*
 * Function: read_encoder
 * Description: read encoder inputs
 * Input:
 *    enc_ch0 - a digital pin associated with an encoder
 *    enc_ch1 - another digital pin associated with an encoder
 *    enc_num - There are 4 rotary encoders, valid values 0-3
 */

void read_encoder(uint8_t enc_ch0, uint8_t enc_ch1, uint8_t enc_num)
{
    static uint8_t old_AB[] = {3, 3, 3};
    static int8_t encval[] = {0, 0, 0};
    static const int8_t enc_states[] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};

    old_AB[enc_num] <<=2;

    if (digitalRead(enc_ch0)) old_AB[enc_num] |= 0x02;
    if (digitalRead(enc_ch1)) old_AB[enc_num] |= 0x01;

    encval[enc_num] += enc_states[(old_AB[enc_num] & 0x0f)];
    if (encval[enc_num] > 3 or encval[enc_num] < -3) {
        int8_t changevalue = (encval[enc_num] > 0) - (encval[enc_num] < 0);
        if (changevalue == 1) {
            encoder_led_mapping(enc_num, true);
        } else {
            encoder_led_mapping(enc_num, false);
        }
        encval[enc_num] = 0;
    }
}

/*
 * Function: manual_seq_control
 * Description: manually step thru the sequencer
 * Input:
 *    manual_direction - true == clockwise, false == counter-clockwise
 */

void manual_seq_control(bool manual_direction)
{
    if (menu_mode == GLOBAL_SEQUENCER_MODE) {
        if (manual_direction) {
            if (global_seq.step >= (global_seq.length - 1)) {
                global_seq.step = 0;
            } else {
                global_seq.step++;
            }
        } else {
            if (global_seq.step == 0) {
                global_seq.step = global_seq.length - 1;
            } else {
                global_seq.step--;
            }
        }
        display_global_sequencer();
    }
}

/*
 * Function: sx1509_midi_func
 * Description: handles button events related to playing midi notes
 * Input:
 *    pin_num - an SX1509 pin associated with playing midi notes
 */

void sx1509_midi_func(uint8_t pin_num)
{
    if ((millis() - sx1509_pin[pin_num].debounce) > sw_debounce_time) {
        if (SX1509_io.digitalRead(pin_num) == LOW) {
            if (global_seq.record) {
                sequencer_array[global_seq.step] ^= (1 << pin_num);
            }
            if (menu_mode == GLOBAL_SEQUENCER_MODE) {
                matrix.fillRect(0, 0, 16, 6, LED_OFF);     // clear sequencer portion of display
                matrix.drawPixel(global_seq.step % 16, global_seq.row % 6, LED_ON);
                matrix.drawPixel(key_array[pin_num].led_pos[0], key_array[pin_num].led_pos[1], LED_ON);
                matrix.writeDisplay();
            } else if (menu_mode == DETAILED_PARAM_MODE) {
                draw_image(key_bitmap[pin_num]);
            }
            if ((digitalRead(NANO_sw0_pin) == HIGH) && !key_array[pin_num].state) {
                MIDI.sendNoteOn(key_array[pin_num].midi_note, key_array[pin_num].volume, key_array[pin_num].midi_chan);
                key_array[pin_num].state = true;
            }
            global_seq.last_key = pin_num;
        } else if ((SX1509_io.digitalRead(pin_num)) == HIGH) {
            if (!(0x0001 & (sequencer_array[global_seq.step] >> pin_num)) && (menu_mode == GLOBAL_SEQUENCER_MODE)) {
                matrix.drawPixel(key_array[pin_num].led_pos[0], key_array[pin_num].led_pos[1], LED_OFF);
                matrix.writeDisplay();
            }
            if (key_array[pin_num].note_off && key_array[pin_num].state) {
                MIDI.sendNoteOff(key_array[pin_num].midi_note, 0, key_array[pin_num].midi_chan);
            }
            key_array[pin_num].state = false;
        }
        sx1509_pin[pin_num].debounce = millis();
    }
}

/*
 * Function: sx1509_input_handler
 * Description: Handles all inputs to the SX1509css
 */

void sx1509_input_handler()
{
    uint16_t intSrc = SX1509_io.interruptSource();

    if (intSrc == 0x4000) {
        if (SX1509_io.digitalRead(14) == LOW && ((millis() - sx1509_pin[14].debounce) > sw_debounce_time)) {
            if (menu_mode == GLOBAL_SEQUENCER_MODE) {
                if (digitalRead(NANO_sw0_pin) == LOW) {
                    manual_seq_control(false);
                } else {
                    if (global_seq.record) {
                        global_seq.record = false;
                        matrix.drawPixel(12, 7, LED_OFF);
                        global_seq.record_blink_flag = false;
                    } else {
                        global_seq.record = true;
                        matrix.drawPixel(12, 7, LED_ON);
                        global_seq.record_blink_flag = true;
                        global_seq.record_last_blink = millis();
                    }
                    matrix.writeDisplay();
                }
            }
            sx1509_pin[14].debounce = millis();
        }
    } else if (intSrc == 0x8000) {
        if (SX1509_io.digitalRead(15) == LOW && ((millis() - sx1509_pin[15].debounce) > sw_debounce_time)) {
            if (menu_mode == GLOBAL_SEQUENCER_MODE) {
                if (digitalRead(NANO_sw0_pin) == LOW) {
                    manual_seq_control(true);
                } else {
                    for (int i = 0; i < MAX_POLYPHONY; i++) {
                        MIDI.sendNoteOff(key_array[i].midi_note, 0, key_array[i].midi_chan);
                    }
                    if (global_seq.paused) {
                        global_seq.paused = false;
                        MIDI.sendStop();
                    } else {
                        global_seq.paused = true;
                        MIDI.sendStart();
                        prev_clock_tick = micros();
                    }
                }
            }
            sx1509_pin[15].debounce = millis();
        }
    } else {
        for (uint8_t i = 0; i < MAX_POLYPHONY; i++) {
            sx1509_midi_func(i);
        }
    }
}

/*
 * Function: analog_potentiometer_disp
 * Description: displays setting associated with a specified analog potentiometer on the LED backpack
 * Input:
 *    anlg_pin_num - an analog i/o pin number
 */

void analog_potentiometer_disp(int anlg_pin_num)
{
    draw_image(anlg_pot[anlg_pin_num].bitmap);
    int pot_level = (POT_MOD(analogRead(anlg_pot[anlg_pin_num].pinNum))/ 64);
    if (analogRead(anlg_pot[anlg_pin_num].pinNum) > 991) {
        matrix.drawLine(0, 0, 15, 0, LED_OFF);
    } else {
        matrix.drawLine(0, 0, pot_level, 0, LED_ON);
    }
    matrix.writeDisplay();
    anlg_pot[anlg_pin_num].state = analogRead(anlg_pot[anlg_pin_num].pinNum);
}

/*
 * Function: analog_potentiometer_handler
 * Description: Handles changes to the analog potentiometers
 */

void analog_potentiometer_handler(void)
{
    if ((millis() - prev_pot_time) > 50) {
        if (abs(analogRead(anlg_pot[0].pinNum) - anlg_pot[0].state) > 32) {
            analog_potentiometer_disp(0);
            key_array[global_seq.last_key].volume = POT_MOD(anlg_pot[0].state) / 8; // per key volume
        } else if (abs(analogRead(anlg_pot[1].pinNum) - anlg_pot[1].state) > 32) {
            analog_potentiometer_disp(1);
            global_seq.volume = POT_MOD(anlg_pot[1].state) / 8;
            MIDI.sendControlChange(7, global_seq.volume, global_seq.midi_chan); // global volume
        } else if (abs(analogRead(anlg_pot[2].pinNum) - anlg_pot[2].state) > 32) {
            analog_potentiometer_disp(2);
            global_seq.attack = POT_MOD(anlg_pot[2].state) / 8;
            MIDI.sendControlChange(73, global_seq.attack, global_seq.midi_chan); // global attack
        } else if (abs(analogRead(anlg_pot[3].pinNum) - anlg_pot[3].state) > 32) {
            analog_potentiometer_disp(3);
            global_seq.release = POT_MOD(anlg_pot[3].state) / 8;
            MIDI.sendControlChange(72, global_seq.release, global_seq.midi_chan); // global release
        }
        prev_pot_time = millis();
    }
}

/*
 * Function: global_sequencer_tracker
 * Description: tracks sequencer steps when sequencer is active
 * Input:
 *    direction - true == clockwise, false == counter-clockwise
 */

void global_sequencer_tracker(bool direction)
{
    if (direction) {
        if (global_seq.step >= (global_seq.length - 1)) {
            global_seq.step = 0;
        } else {
            global_seq.step++;
        }
    } else {
        if (global_seq.step == 0) {
            global_seq.step = global_seq.length - 1;
        } else {
            global_seq.step--;
        }
    }
}

/*
 * Function: draw_sequencer_pixel
 * Description: draw the sequencer progression on the LED backpack
 */

void draw_sequencer_pixel()
{
    for (uint8_t i = 0; i < MAX_POLYPHONY; i++) {
        if (0x0001 & (sequencer_array[global_seq.step] >> i)) {
            matrix.drawPixel(key_array[i].led_pos[0], key_array[i].led_pos[1], LED_ON);
        } else {
            matrix.drawPixel(key_array[i].led_pos[0], key_array[i].led_pos[1], LED_OFF);
        }
    }
    matrix.writeDisplay();
}

/*
 * Function: display_global_sequencer
 * Description: Displays the global sequencer menu on the LED backpack
 */

void display_global_sequencer()
{
    global_seq.page = global_seq.step / 96;
    global_seq.row = global_seq.step / 16;
    matrix.fillRect(0, 0, 16, 6, LED_OFF); // clear sequencer portion of display
    matrix.drawPixel(global_seq.step % 16, global_seq.row % 6, LED_ON);
    if (global_seq.page != global_seq.prev_page) { // this is for tracking which page of the sequencer the user is on
        switch(global_seq.page) {
            case 0:
                matrix.fillRect(14, 6, 2, 2, LED_OFF);
                matrix.drawPixel(14, 6, LED_ON);
                break;
            case 1:
                matrix.fillRect(14, 6, 2, 2, LED_OFF);
                matrix.drawLine(14, 6, 15, 6, LED_ON);
                break;
            case 2:
                matrix.fillRect(14, 6, 2, 2, LED_OFF);
                matrix.drawLine(14, 6, 15, 6, LED_ON);
                matrix.drawPixel(15, 7, LED_ON);
                break;
            case 3:
                matrix.fillRect(14, 6, 2, 2, LED_ON);
                break;
            default:
                break;
        }
        global_seq.prev_page = global_seq.page;
    }
    draw_sequencer_pixel();
    matrix.writeDisplay();
}

/*
 * Function: switch_menu_mode
 * Description: handles which menu the system is in
 */

void switch_menu_mode()
{
    matrix.clear();
    matrix.writeDisplay();
    if (menu_mode == GLOBAL_SEQUENCER_MODE) {
        menu_mode = DETAILED_PARAM_MODE;
        // display the detailed param mode interface
        draw_image(key_bitmap[global_seq.last_key]);
    } else if (menu_mode == DETAILED_PARAM_MODE) {
        menu_mode = GLOBAL_SEQUENCER_MODE;
        if (global_seq.paused) {
            matrix.drawLine(10, 6, 10, 7, LED_ON);
            matrix.drawLine(8, 6, 8, 7, LED_ON);
        }
        global_seq.prev_page = 5; // set it to an impossible value so that it triggers the if statement in display_global_sequencer(), a bit hacky
        display_global_sequencer();
    }
}

/*
 * Function: sequencer_handler
 * Description: handles changes to sequencer status
 */

void sequencer_handler()
{
    if (global_seq.record && (menu_mode == GLOBAL_SEQUENCER_MODE)) { // display a blinking dot if sequencer recording is on
        if ((millis() - global_seq.record_last_blink) >= 500) {
            if (global_seq.record_blink_flag) {
                matrix.drawPixel(12, 7, LED_OFF);
                global_seq.record_blink_flag = false;
            } else {
                matrix.drawPixel(12, 7, LED_ON);
                global_seq.record_blink_flag = true;
            }
            global_seq.record_last_blink = millis();
            matrix.writeDisplay();
        }
    }

    if ((menu_mode == GLOBAL_SEQUENCER_MODE) && (prev_sequencer_step_val != sequencer_array[global_seq.step])) { // if the sequencer step has changed, reflect that on the LED backpack
        draw_sequencer_pixel();
        matrix.writeDisplay();
        prev_sequencer_step_val = sequencer_array[global_seq.step];
    }

    if (global_seq.paused) { // if the sequencer is paused, draw two vertical parallel lines on the LED backpack to indicate it
        if (menu_mode == GLOBAL_SEQUENCER_MODE) {
            matrix.drawLine(10, 6, 10, 7, LED_ON);
            matrix.drawLine(8, 6, 8, 7, LED_ON);
            matrix.writeDisplay();
        }
        return;
    } else {
        matrix.fillRect(8, 6, 3, 2, LED_OFF);
    }
    //do math for bpm and transition LEDs to next step
    if (((micros() - prev_clock_tick) >= us_per_tick) || (micros() < prev_clock_tick)) {
        MIDI.sendClock();
        prev_clock_tick = micros();
    }
    if ((millis() - prev_seq_time) >= bpm_as_ms) {
        // increment sequencer steps
        global_sequencer_tracker(global_seq.direction);
        if (menu_mode == GLOBAL_SEQUENCER_MODE) {
            display_global_sequencer();
            for (uint8_t i = 0; i < MAX_POLYPHONY; i++) { // checks current sequencer step for any programmed midi notes
                if ((0x0001 & (sequencer_array[global_seq.step] >> i)) && ((uint8_t) random(1, 100) < key_array[i].probability)) {
                    MIDI.sendNoteOn(key_array[i].midi_note, key_array[i].volume, key_array[i].midi_chan);
                }
            }
        } else if (menu_mode == DETAILED_PARAM_MODE) {
            // WIP related to button-specific BPM
        }
        prev_seq_time = millis();
    }
}

/*
 * Function: sw0_func
 * Description: handles inputs on the one dedicated switch attached directly to the Arduino, in this case switches menu modes when tapped
 */

void sw0_func()
{
    sw0_flag = false;
    switch_menu_mode();
    uint16_t unstuck_sx1509_int = SX1509_io.interruptSource(); // sx1509 inputs seem to get overwhelmed when spammed rapidly
    NANO_sw0_last_trig = millis();
}

/*
 * Function: enc0_sw_func
 * Description: handles switch presses on encoder 0, in this case if the system is in menu DETAILED_PARAM_MODE, it resets parameters for the last pressed button
 */

void enc0_sw_func()
{
    if (menu_mode == DETAILED_PARAM_MODE) {
        draw_image(enc0_param_rst_bmp);
        key_array[global_seq.last_key].volume = 127;
        key_array[global_seq.last_key].midi_chan = global_seq.midi_chan;
        key_array[global_seq.last_key].probability = 100;
        key_array[global_seq.last_key].note_off = false;
        key_array[global_seq.last_key].state = false;
    } else { // TBD
        // WIP - Have ideas for this knob? Feel free to make a suggestion or fork this repo to give it a shot yourself!
    }
    enc0_sw_flag = false;
}

/*
 * Function: enc0_knob_func
 * Description: handles changes to encoder 0
 */

void enc0_knob_func()
{
    read_encoder(NANO_enc0_ch0, NANO_enc0_ch1, 0);
    enc0_knob_flag = false;
}

/*
 * Function: enc1_sw_func
 * Description: handles switch pressed on encoder 1, in this case if the system is in menu DETAILED_PARAM_MODE, it toggles note-off for the last pressed button on/off
 */

void enc1_sw_func()
{
    if (menu_mode == DETAILED_PARAM_MODE) {
        // toggle note-off on/off
        if (key_array[global_seq.last_key].note_off) {
            draw_image(enc1_note_off_dis_bmp);
            key_array[global_seq.last_key].note_off = false;
        } else {
            draw_image(enc1_note_off_en_bmp);
            key_array[global_seq.last_key].note_off = true;
        }
    } else { //TBD
        // WIP - Have ideas for this knob? Feel free to make a suggestion or fork this repo to give it a shot yourself!
    }
    enc1_sw_flag = false;
}

/*
 * Function: enc1_knob_func
 * Description: handles changes to encoder 1
 */

void enc1_knob_func()
{
    read_encoder(NANO_enc1_ch0, NANO_enc1_ch1, 1);
    enc1_knob_flag = false;
}

/*
 * Function: enc2_sw_func
 * Description: handles switch pressed on encoder 2, in this case if the system is in menu GLOBAL_SEQUENCER_MODE, it changes the direction of the sequencer, denoted by a negative/positive BPM
 */

void enc2_sw_func()
{
    if (menu_mode == GLOBAL_SEQUENCER_MODE) {
        // reverse sequencer
        matrix.fillRect(0, 0, 16, 6, LED_OFF);
        if (global_seq.direction) {
            global_seq.direction = false;
            load_bitmap(global_seq.bpm + 45);
            bpm_direction();
        } else {
            global_seq.direction = true;
            load_bitmap(global_seq.bpm + 45);
            bpm_direction();
        }
    } else if (menu_mode == DETAILED_PARAM_MODE) {
        // resync local BPM WIP
    }
    enc2_sw_flag = false;
}

/*
 * Function: enc2_knob_func
 * Description: handles changes to encoder 2
 */

void enc2_knob_func()
{
    read_encoder(NANO_enc2_ch0, NANO_enc2_ch1, 2);
    enc2_knob_flag = false;
}

void loop()
{
    // automatically assume that SW0 is being used as a shift key if any of the other buttons/encoders have been triggered as well
    static bool shift_op = enc0_sw_flag || enc0_knob_flag || enc1_sw_flag || enc1_knob_flag || enc2_sw_flag || enc2_knob_flag || sx1509_int_flag;
    sequencer_handler();
    analog_potentiometer_handler();

    // check for flags that would be set due to an interrupt being triggered
    if (sw0_flag && !shift_op) {
        if ((millis() - NANO_sw0_last_trig) > sw_debounce_time) {
            if (sw0_last_pressed == 0) {
                sw0_last_pressed = millis();
            }
            if (digitalRead(NANO_sw0_pin) == HIGH) {
                if ((millis() - sw0_last_pressed) < 200) { // somewhat arbitrary period of time to determine whether to interpret SW0 as shift or menu-change
                    sw0_func();
                    sw0_last_pressed = 0;
                } else {
                    sw0_flag = false;
                    sw0_last_pressed = 0;
                }
            }
            NANO_sw0_last_trig = millis();
        }
    }
    
    if (enc0_sw_flag) {
        if ((millis() - enc0_sw_last_trig) > sw_debounce_time) {
            enc0_sw_func();
            enc0_sw_last_trig = millis();
        }
    }

    if (enc0_knob_flag) {
        enc0_knob_func();
    }

    if (enc1_sw_flag) {
        if ((millis() - enc1_sw_last_trig) > sw_debounce_time) {
            enc1_sw_func();
            enc1_sw_last_trig = millis();
        }
    }

    if (enc1_knob_flag) {
        enc1_knob_func();
    }

    if (enc2_sw_flag) {
        if ((millis() - enc2_sw_last_trig) > sw_debounce_time) {
            enc2_sw_func();
            enc2_sw_last_trig = millis();
        }
    }

    if (enc2_knob_flag) {
        enc2_knob_func();
    }

    if (sx1509_int_flag) {
        if ((millis() - sx1509_int_pin_last_trig) > sw_debounce_time) {
            sx1509_int_flag = false;
            sx1509_input_handler();
            sx1509_int_pin_last_trig = millis();
        }
    }

    shift_op = false;
}