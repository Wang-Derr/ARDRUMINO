# ARD*SEQU*INO

An open source Arduino based MIDI sequencer and sample player.

Demo video coming soon!

## Description

The **ARD***SEQU***INO** project includes both software and hardware components. A rudimentary understanding of programming Arduinos is strongly recommended, the software may work right out of the box but will most likely require some tweaking to function correctly (more on this further down). An intermediate-level understanding of circuitry is required because the hardware is self-assembled.

My goal for this project was to create a rudimentary MIDI controller that would allow me to interface with a WAV trigger in preparation for future more in-depth MIDI controller projects. However, I ended up expanding this project to include a step sequencer that I could also use to control other MIDI compatible devices. With some minor tweaks to the electrical schematic (and possibly minor Arduino code surgery), this device could also just serve as a standalone sequencer without a need for a WAV trigger board (it would simply lack the ability to play sounds on its own but would knock down the price).

![Rotating gif](assets/Gifs/rotate_flat.gif)

This device is a sequencer with a built-in sample player. The specifications for this device are as follows:
- (up-to) 384 step sequencer
- LED display, can toggle between two modes: sequencer and parameter menu
- MIDI CC for volume
- MIDI CC for Attack
- MIDI CC for Release
- Global MIDI channel control
- BPM control (45-300), reversable as well
  - programmable note per beat division
- MIDI program change bank control (0-31 by default but expandable in software, limited to 32 because of the WAV Trigger)
- 14 voices (midi-note assignable keys)
- Per key volume control
- Per key MIDI channel control
- Per key sequence probability control
- Full MIDI output capabilities
- Limited MIDI input capabilities, supports control of the WAV Trigger via MIDI input but not control of the sequencer

## Required Arduino Libraries

- https://github.com/adafruit/Adafruit-GFX-Library
- https://github.com/adafruit/Adafruit_LED_Backpack
- https://github.com/FortySevenEffects/arduino_midi_library
- https://github.com/sparkfun/SX1509_IO-Expander

*can be installed directly through the Arduino IDE built-in library manager*

## Required Hardware

- 1x [Arduino Nano](https://store.arduino.cc/products/arduino-nano/)
- 1x [HT16K33 Backpack 16x8 LED Matrix](https://www.adafruit.com/product/2044)
- 1x [SX1509 GPIO expander](https://www.sparkfun.com/products/13601)
- 2x [6N138 Octocouplers](https://www.amazon.com/gp/product/B09C8T7V6V)
- 2x [MIDI DIN sockets](https://www.amazon.com/gp/product/B01GBT9RC0)
- 1x [WAV Trigger](https://www.robertsonics.com/wav-trigger/)
- 4x [10k Ohm rotary potentiometers](https://www.amazon.com/gp/product/B00MCK7JMS)
- 3x [5-pin rotary encoders](https://www.amazon.com/gp/product/B07DM2YMT4)
- 17x [mechanical switches](https://www.amazon.com/gp/product/B0BXZXZX74)
- 2x 1N4148 diodes
- 3x 220 Ohm resistors
- 2x 470 Ohm resistors
- 2x 10k Ohm resistors

*Links for parts are just examples, feel free to purchase from your preferred vendor*

## Optional (but recommended) 3D modeled enclosure

You can find the `.stl` files for the enclosure [here](https://www.printables.com/model/1103677-ardsequino-enclosure)!

You will also need the following hardware to secure the circuit and enclosure:
- 17x key caps (I printed [these](https://www.printables.com/model/67474-flat-mx-keycap))
- 9x M3 8mm screws
- 9x M3 nuts
- 4x M2 6mm screws
- 4x M2 nuts

## Assembly Instructions and User Manual

![Assembly gif](assets/Gifs/ardsequino_explode.gif)

Please follow [this guide](./UserManual.md) for both assembly instructions and the user manual!

## Known issues

Not working right off the bat? You may need to tweak the software so that it works correctly for your build. You can find information on that in the guide linked above as well!

- Simultaneous midi note playing (via the keys, sequencer works fine,) can be wonky; likely a limitation based on the way the GPIO expander works or my implementation of the code to read it.
- Feel free to report an Issue or make a Pull Request if you run into additional problems
- Software debouncing is a bit iffy, still may run into occasional ghost presses.

## Future Plans

- Alternate sequencer interface for individually tracking each key
- Tap Tempo
- Per key sequencer control
- per key time signature control
- swing
- Per key BPM control (polyrythms?)
- key-hold midi-note rolls
- Sequencer setting saving

## Version Log

### V1.1

- Fix for global MIDI setting not applying to each key.

### V1.0

Official release!
- updated circuit schematic.
- Cleaned up.
- Added comments to the code.
- All features targeted for 1.0 fully functioning! (I think)
  - (up-to) 384 step sequencer
  - LED display, can toggle between two modes: sequencer and parameter menu
  - MIDI CC for volume
  - MIDI CC for Attack
  - MIDI CC for Release
  - Global MIDI channel control
  - BPM control (45-300), reversable as well
    - programmable note per beat division
  - MIDI program change bank control (0-31 by default but expandable in software, limited to 32 because of the WAV Trigger)
  - 14 voices (midi-note assignable keys)
  - Per key volume control
  - Per key MIDI channel control
  - Per key sequence probability control
  - Full MIDI output capabilities
  - Limited MIDI input capabilities, supports control of the WAV Trigger via MIDI input but not control of the sequencer
  - More detailed descriptions on these features can be found in the [user manual](./UserManual.md).
- Complete documentation include build instructions and user manual (probably a lot of typos).
- Maybe more to come?

### V0.1

First release containing all basic functionality I had planned for this device. Works, but needs work.
- functioning 384 step sequencer.
- Potentiometer work to control track volume, global volume, global attack, and global release.
- kit select for the WAV trigger.
- MIDI channel controls.
- Keys can play midi notes (but issues with simultaneous playing, being investigated).

*If you have any questions feel free to submit an issue.*