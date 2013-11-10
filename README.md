linux-rpi-audi-dis
==================

Linux kernel module for sending data to the DIS LCD in an Audi car from a Raspberry Pi.

This module is specific to the Raspberry Pi because it uses the GPIO pins.

It is intended to be used like this:

```
echo " INSERT   COIN" > /proc/dis
```

![Audi DIS showing INSERT COIN](https://raw.github.com/derpston/linux-rpi-audi-dis/master/img/dis-insert-coin.jpg "Audi DIS showing INSERT COIN")

References
==========

This project builds on the work done in [https://github.com/derpston/Audi-radio-DIS-reader](https://github.com/derpston/Audi-radio-DIS-reader) where the protocol/format was analysed and reverse engineered.

This project uses a heavily butchered version of the GPIO device driver built by Redrobes as described in [http://www.raspberrypi.org/phpBB3/viewtopic.php?f=33&t=32152](http://www.raspberrypi.org/phpBB3/viewtopic.php?f=33&t=32152).

Compiling
=========

Run `make`. Some dependencies are `build-essential` and some kernel header files from a `linux-source-...` package. There may be more - sorry, I didn't keep good track of dependencies while hacking on this. Contributions/issues welcome.

Electronic connections
======================

Connecting the Raspberry Pi's GPIO pins to the DIS is relatively straightforward but might take a while.

You'll need to tap or otherwise connect to the CLOCK, DATA and ENABLE lines between the radio and the DIS as described in [https://github.com/derpston/Audi-radio-DIS-reader](https://github.com/derpston/Audi-radio-DIS-reader). (includes pinout)

The radio/DIS system uses a 5V logic level and the Raspberry Pi uses a 3.3v logic level, so you'll need a level shifter/converter. I personally used three optoisolators because I wanted to keep the Pi isolated from the car's electronics as much as possible, but something like [https://www.sparkfun.com/products/11978](https://www.sparkfun.com/products/11978) would probably work very well. Bear in mind that you'll need to find a level shifter that has at least three lines in the same direction.

The pin mapping is:

* RPi GPIO 14 -> CLOCK
* RPi GPIO 15 -> DATA
* RPi GPIO 18 -> ENABLE

Be careful to make sure that the Raspberry Pi's ground is the same as the car's ground, otherwise you might blow something up.

This is what it looks like in a lab environment with a logic analyser instead of the car's DIS connections:
![RPi in the lab](https://raw.github.com/derpston/linux-rpi-audi-dis/master/img/rpi-lab.jpg "RPi in the lab")

Using
=====

Load the module:

```
# insmod dis.ko
#
```

Check that the module was loaded successfully:
```
# lsmod | grep dis
dis                     2793  0 
# dmesg | grep DIS
DIS LCD module loaded.
#
```

To send some text to the display:
```
$ echo FOO BAR > /proc/dis
$
```

To unload the module:
```
# rmmod dis
#
```

