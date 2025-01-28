/*
 * This file is part of the ARDSEQUINO project.
 *
 * ARDSEQUINO is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * ARDSEQUINO is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with ARDSEQUINO. If not, see <https://www.gnu.org/licenses/>. 
 */

#ifndef ARDSEQUINO_H
#define ARDSEQUINO_H

// uncomment if potentiometer polarity is flipped, comment out otherwise
#define FLIPPED_POTS

// uncommment if LED backpack is upsidedown, comment out otherwise
#define FLIPPED_LEDS

#ifdef FLIPPED_POTS
#define POT_MOD(x) (1023 - x)
#endif // FLIPPED_POTENTIOMETERS

#ifndef FLIPPED_POTS
#define POT_MOD(x) (x)
#endif // FLIPPED_POTENTIOMETERS

#ifdef FLIPPED_LEDS
#define LED_ORIENTATION 3
#endif // FLIPPED_LEDS

#ifndef FLIPPED_LEDS
#define LED_ORIENTATION 1
#endif // FLIPPED_LEDS

#define GLOBAL_SEQUENCER_MODE 0
#define DETAILED_PARAM_MODE 1

#define KIT_ENCODER 0
#define SEQUENCE_LENGTH_ENCODER 1
#define BPM_ENCODER 2

#define MAX_SEQUENCER_LENGTH 384
#define MAX_POLYPHONY 14
#define MAX_BPM 255
#define MAX_PC_BANK 31
#define MAX_MIDI_CHANNEL 16
#define MAX_NOTES_PER_BEAT 8
#define MAX_MIDI_NOTE 127
#define MAX_PROBABILITY 100

#define DEFAULT_MIDI_CHANNEL 8

#define SX1509_PIN_CT 16

template <typename T> void PROGMEM_readAnything (const T * sce, T& dest)
{
  memcpy_P (&dest, sce, sizeof (T));
}

template< typename T, size_t N > size_t ArraySize (T (&) [N]){ return N; }

// global debounce var
extern const unsigned long sw_debounce_time = 30;

// analog potentiometer assignment
extern const uint8_t NANO_pot_0 = A0;
extern const uint8_t NANO_pot_1 = A1;
extern const uint8_t NANO_pot_2 = A2;
extern const uint8_t NANO_pot_3 = A3;

// of the 17 key switches, one is connected directly to the Nano while the rest are routed through the SX1509 GPIO expander
extern const uint8_t NANO_sw0_pin = 12;

// encoder defintions
extern const uint8_t NANO_enc0_ch0 = 4;
extern const uint8_t NANO_enc0_ch1 = 3;
extern const uint8_t NANO_enc0_sw = 5;

extern const uint8_t NANO_enc1_ch0 = 7;
extern const uint8_t NANO_enc1_ch1 = 6;
extern const uint8_t NANO_enc1_sw = 8;

extern const uint8_t NANO_enc2_ch0 = 10;
extern const uint8_t NANO_enc2_ch1 = 9;
extern const uint8_t NANO_enc2_sw = 11;

// SX1509 related defintions
extern const uint8_t SX1509_int_pin = 2;
extern const uint8_t SX1509_ADDR = 0x3E;
extern const uint8_t HT16K33_ADDR = 0x70;

// Begin volatile declarations for vars accessed by interrupts

// encoder state vars
extern volatile bool prev_enc0_ch0_state;
extern volatile bool prev_enc0_ch1_state;
extern volatile bool prev_enc1_ch0_state;
extern volatile bool prev_enc1_ch1_state;
extern volatile bool prev_enc2_ch0_state;
extern volatile bool prev_enc2_ch1_state;

// interrupt flags
extern volatile bool sx1509_int_flag;
extern volatile bool sw0_flag;
extern volatile bool enc0_knob_flag;
extern volatile bool enc0_sw_flag;
extern volatile bool enc1_knob_flag;
extern volatile bool enc1_sw_flag;
extern volatile bool enc2_knob_flag;
extern volatile bool enc2_sw_flag;

// Each of the 14 keys/buttons will own unique sets of these properties
typedef struct sound_properties {
    uint8_t midi_note = 0; // 0-127
    uint8_t volume = 127; // 0-127
    uint8_t midi_chan = DEFAULT_MIDI_CHANNEL; // 1-16
    uint8_t probability = 100; // 0-100
    bool note_off = false; // true == note-off is enabled
    bool state = false; // true == button is actively pressed
    uint8_t led_pos[2] = {0, 0}; // {x, y} mapping of button to LED backpack
} sound_properties_t;

typedef struct sx1509_pins {
    unsigned long debounce = 0;
} sx1509_pins_t;

typedef struct analog_potentiometers {
    int pinNum;
    int state;
    uint8_t* bitmap;
} analog_potentiometers_t;

typedef struct global_sequencer_menu {
    uint8_t bpm = 45; // 0-255
    uint16_t step = 0; // 0-479
    uint8_t row = 0; // 0-29
    bool direction = true;
    uint8_t page = 0; // 0-4
    uint8_t prev_page = 0; // 0-4
    uint16_t length = 8; // 1-480
    uint8_t npb = 1; // notes per beat (can be viewed as note length per sequencer step i.e. 1 npb = 1/4 notes, 2 npb = 1/8, 4 npb = 1/16 notes, etc. Similar to DAWs, each beat is treated as a 1/4 note in length)
    uint8_t midi_chan = DEFAULT_MIDI_CHANNEL; // 1-16
    uint8_t volume = 64; // 0-127
    uint8_t attack = 0; // 0-127
    uint8_t release = 0; // 0-127
    uint8_t PCNum = 0; // 0-31
    bool record = false;
    bool paused = true;
    unsigned long record_last_blink = 0;
    bool record_blink_flag = false;
    uint8_t last_key = 0;
} global_sequencer_menu_t;

// standalone bitmaps
extern const PROGMEM uint8_t enc0_bmp[] =
    { // placeholder, not in use
        B11101110, B11101110,
        B10001010, B10001010,
        B11001010, B10001010,
        B10001010, B10001010,
        B11101010, B11101110,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    },
    enc0_param_rst_bmp[] =
    {
        B11001100, B11011000,
        B10101010, B10101000,
        B11001100, B10101000,
        B10001010, B10101000,
        B00000000, B00000000,
        B11101101, B11000000,
        B10001000, B10000000,
        B10011000, B10000000,
    },
    enc1_bmp[] =
    { // placeholder, not in use
        B11101110, B11101000,
        B10001010, B10001000,
        B11001010, B10001000,
        B10001010, B10001000,
        B11101010, B11101000,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    },
    enc1_note_off_en_bmp[] =
    {
        B01110011, B10110110,
        B01010010, B10100100,
        B01010010, B10110110,
        B01010011, B10100100,
        B00000000, B00000000,
        B00001110, B11100000,
        B00001010, B10100000,
        B00001110, B10100000,
    },
    enc1_note_off_dis_bmp[] =
    {
        B01110011, B10110110,
        B01010010, B10100100,
        B01010010, B10110110,
        B01010011, B10100100,
        B00000000, B11011000,
        B00001110, B10010000,
        B00001010, B11011000,
        B00001110, B10010000,
    },
    enc0_rotate_bmp[] =
    {
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
        B10101011, B10000000,
        B11001001, B00000000,
        B10101001, B00000000,
    },
    enc0_alt_rotate_bmp[] =
    {
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
        B11010000, B10000000,
        B10011101, B11011100,
        B11010101, B01010100,
    },
    enc1_rotate_bmp[] =
    {
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
        B11000000, B01000000,
        B11011011, B01100000,
        B10010011, B01100000,
    },
    enc2_rotate_bmp[] =
    {
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
        B10011000, B00000000,
        B11011011, B11100000,
        B11010010, B10100000,
    },
    pot0_rotate_bmp[] =
    {
        B00000000, B00000000,
        B00000000, B00000000,
        B11101010, B11101000,
        B01001010, B10101000,
        B01000100, B11101110,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    },
    pot1_rotate_bmp[] =
    {
        B00000000, B00000000,
        B00000000, B00000000,
        B00010101, B11010000,
        B00010101, B01010000,
        B00001001, B11011100,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    },
    pot2_rotate_bmp[] =
    {
        B00000000, B00000000,
        B00000000, B00000000,
        B00001001, B11010100,
        B00011100, B10011000,
        B00010100, B10010100,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    },
    pot3_rotate_bmp[] =
    {
        B00000000, B00000000,
        B00000000, B00000000,
        B00011101, B11010000,
        B00010001, B10010000,
        B00010001, B11011100,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    };

// The following bitmaps are stored in an array so that they can be index-mapped
// bitmaps for numbers in the ones place
extern const PROGMEM uint8_t digit0[10][16] {
    {
        B00000000, B00001110,
        B00000000, B00001010,
        B00000000, B00001010,
        B00000000, B00001010,
        B00000000, B00001110,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    },
    {
        B00000000, B00000100,
        B00000000, B00001100,
        B00000000, B00000100,
        B00000000, B00000100,
        B00000000, B00001110,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    },
    {
        B00000000, B00001110,
        B00000000, B00000010,
        B00000000, B00001110,
        B00000000, B00001000,
        B00000000, B00001110,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    },
    {
        B00000000, B00001110,
        B00000000, B00000010,
        B00000000, B00000110,
        B00000000, B00000010,
        B00000000, B00001110,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    },
    {
        B00000000, B00001010,
        B00000000, B00001010,
        B00000000, B00001110,
        B00000000, B00000010,
        B00000000, B00000010,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    },
    {
        B00000000, B00001110,
        B00000000, B00001000,
        B00000000, B00001110,
        B00000000, B00000010,
        B00000000, B00001110,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    },
    {
        B00000000, B00001000,
        B00000000, B00001000,
        B00000000, B00001110,
        B00000000, B00001010,
        B00000000, B00001110,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    },
    {
        B00000000, B00001110,
        B00000000, B00000010,
        B00000000, B00000010,
        B00000000, B00000010,
        B00000000, B00000010,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    },
    {
        B00000000, B00001110,
        B00000000, B00001010,
        B00000000, B00001110,
        B00000000, B00001010,
        B00000000, B00001110,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    },
    {
        B00000000, B00001110,
        B00000000, B00001010,
        B00000000, B00001110,
        B00000000, B00000010,
        B00000000, B00000010,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    }
};

// bitmaps for numbers in the tens place
extern const PROGMEM uint8_t digit1[10][16] {
    {
        B00000000, B11100000,
        B00000000, B10100000,
        B00000000, B10100000,
        B00000000, B10100000,
        B00000000, B11100000,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    },
    {
        B00000000, B01000000,
        B00000000, B11000000,
        B00000000, B01000000,
        B00000000, B01000000,
        B00000000, B11100000,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    },
    {
        B00000000, B11100000,
        B00000000, B00100000,
        B00000000, B11100000,
        B00000000, B10000000,
        B00000000, B11100000,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    },
    {
        B00000000, B11100000,
        B00000000, B00100000,
        B00000000, B01100000,
        B00000000, B00100000,
        B00000000, B11100000,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    },
    {
        B00000000, B10100000,
        B00000000, B10100000,
        B00000000, B11100000,
        B00000000, B00100000,
        B00000000, B00100000,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    },
    {
        B00000000, B11100000,
        B00000000, B10000000,
        B00000000, B11100000,
        B00000000, B00100000,
        B00000000, B11100000,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    },
    {
        B00000000, B10000000,
        B00000000, B10000000,
        B00000000, B11100000,
        B00000000, B10100000,
        B00000000, B11100000,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    },
    {
        B00000000, B11100000,
        B00000000, B00100000,
        B00000000, B00100000,
        B00000000, B00100000,
        B00000000, B00100000,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    },
    {
        B00000000, B11100000,
        B00000000, B10100000,
        B00000000, B11100000,
        B00000000, B10100000,
        B00000000, B11100000,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    },
    {
        B00000000, B11100000,
        B00000000, B10100000,
        B00000000, B11100000,
        B00000000, B00100000,
        B00000000, B00100000,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    }
};

// bitmaps for numbers in the hundreds place
extern const PROGMEM uint8_t digit2[10][16] {
    {
        B00001110, B00000000,
        B00001010, B00000000,
        B00001010, B00000000,
        B00001010, B00000000,
        B00001110, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    },
    {
        B00000100, B00000000,
        B00001100, B00000000,
        B00000100, B00000000,
        B00000100, B00000000,
        B00001110, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    },
    {
        B00001110, B00000000,
        B00000010, B00000000,
        B00001110, B00000000,
        B00001000, B00000000,
        B00001110, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    },
    {
        B00001110, B00000000,
        B00000010, B00000000,
        B00000110, B00000000,
        B00000010, B00000000,
        B00001110, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    },
    {
        B00001010, B00000000,
        B00001010, B00000000,
        B00001110, B00000000,
        B00000010, B00000000,
        B00000010, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    }
};

// bitmaps for each of 14 midi-note playing keys
extern const PROGMEM uint8_t key_bitmap[14][16] {
    {
        B10101110, B10101110,
        B10101000, B10101010,
        B11001100, B01001010,
        B10101000, B01001010,
        B10101110, B01001110,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    },
    {
        B10101110, B10100100,
        B10101000, B10101100,
        B11001100, B01000100,
        B10101000, B01000100,
        B10101110, B01001110,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    },
    {
        B10101110, B10101110,
        B10101000, B10100010,
        B11001100, B01001110,
        B10101000, B01001000,
        B10101110, B01001110,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    },
    {
        B10101110, B10101110,
        B10101000, B10100010,
        B11001100, B01000110,
        B10101000, B01000010,
        B10101110, B01001110,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    },
    {
        B10101110, B10101010,
        B10101000, B10101010,
        B11001100, B01001110,
        B10101000, B01000010,
        B10101110, B01000010,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    },
    {
        B10101110, B10101110,
        B10101000, B10101000,
        B11001100, B01001110,
        B10101000, B01000010,
        B10101110, B01001110,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    },
    {
        B10101110, B10101000,
        B10101000, B10101000,
        B11001100, B01001110,
        B10101000, B01001010,
        B10101110, B01001110,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    },
    {
        B10101110, B10101110,
        B10101000, B10100010,
        B11001100, B01000010,
        B10101000, B01000010,
        B10101110, B01000010,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    },
    {
        B10101110, B10101110,
        B10101000, B10101010,
        B11001100, B01001110,
        B10101000, B01001010,
        B10101110, B01001110,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    },
    {
        B10101110, B10101110,
        B10101000, B10101010,
        B11001100, B01001110,
        B10101000, B01000010,
        B10101110, B01000010,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    },
    {
        B10101110, B10101011,
        B10101000, B10101011,
        B11001100, B01001011,
        B10101000, B01001011,
        B10101110, B01001011,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    },
    {
        B10101110, B10101010,
        B10101000, B10101010,
        B11001100, B01001010,
        B10101000, B01001010,
        B10101110, B01001010,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    },
    {
        B10101110, B10101011,
        B10101000, B10101001,
        B11001100, B01001011,
        B10101000, B01001010,
        B10101110, B01001011,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    },
    {
        B10101110, B10101011,
        B10101000, B10101001,
        B11001100, B01001011,
        B10101000, B01001001,
        B10101110, B01001011,
        B00000000, B00000000,
        B00000000, B00000000,
        B00000000, B00000000,
    }
};

#endif  // ARDSEQUINO_H