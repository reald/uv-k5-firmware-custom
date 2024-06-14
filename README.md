# Open re-implementation of the Quansheng UV-K5/K6/5R v2.1.27 firmware with 2m ARDF support

This is all based on [egzumer custom firmware](https://github.com/egzumer/uv-k5-firmware-custom)!

Some features have been added to make it possible to use the radio as an amateur radio direction finding (ARDF) receiver on the 2m band.

Understand this as a *proof of concept*! This is experimental software, there is absolutely no warranty! Use it at your OWN RISK as a 
base for own experiments.

Despite all the limitations this is one of the quickest and cheapest ways to get a usable ARDF receiver for the 2m band!


> [!WARNING]  
> Use this firmware at your own risk (entirely). There is absolutely no guarantee that it will work in any way shape or form on your radio(s), it may even brick your radio(s), in which case, you'd need to buy another radio.
Anyway, have fun.


## Table of Contents

* [Main Features](#main-features)
* [Manual](#manual)
* [Manual for ARDF](#specific-ardf-features)
* [Headphones](#headphones)
* [Radio Performance](#radio-performance)
* [User Customization](#user-customization)
* [Compiler](#compiler)
* [Building](#building)
* [Credits](#credits)
* [Other sources of information](#other-sources-of-information)
* [License](#license)
* [Example changes/updates](#example-changesupdates)

## Main features:
* ARDF features
   * manual gain control
   * small bandwidth modes
   * active fox and remaining cycle time display
   * beep signal before end of fox cycle
* many of OneOfEleven mods:
   * AM fix, huge improvement in reception quality
   * long press buttons functions replicating F+ action
   * fast scanning
   * channel name editing in the menu
   * channel name + frequency display option
   * shortcut for scan-list assignment (long press `5 NOAA`)
   * scan-list toggle (long press `* Scan` while scanning)
   * configurable button function selectable from menu
   * battery percentage/voltage on status bar, selectable from menu
   * longer backlight times
   * mic bar
   * RSSI s-meter
   * more frequency steps
   * squelch more sensitive
* fagci spectrum analyzer (**F+5** to turn on)
* some other mods introduced by egzumer:
   * SSB demodulation (adopted from fagci)
   * backlight dimming
   * battery voltage calibration from menu
   * better battery percentage calculation, selectable for 1600mAh or 2200mAh
   * more configurable button functions
   * long press MENU as another configurable button
   * better DCS/CTCSS scanning in the menu (`* SCAN` while in RX DCS/CTCSS menu item)
   * Piotr022 style s-meter
   * restore initial freq/channel when scanning stopped with EXIT, remember last found transmission with MENU button
   * reordered and renamed menu entries
   * LCD interference crash fix
   * many others...

## Manual

Up to date manual is available in the [Wiki section](https://github.com/egzumer/uv-k5-firmware-custom/wiki)

## Specific ARDF Features

Download a [precompiled release](https://github.com/reald/uv-k5-firmware-custom/releases) or compile with ENABLE_ARDF=1 and preferable with ENABLE_PREVENT_TX=1. 
To get enough free flash space disable some unneeded TX features (e.g. ENABLE_DTMF_CALLING=0). 

### Flashing

For flashing [k5prog](https://github.com/nica-f/k5prog) can be used:

1. Backup your device!
1. Press PTT while turning the radio on. It will not work if cable is already plugged. White LED must be on.
1. Connect the programming cable
1. Run `k5prog -b firmware.bin -F -YYY`

The `firmware.packed.bin` file can be flashed with the official flash tool. If device does not boot after flashing something was wrong. Try flashing
another firmware or a different flash tool.

### Usage

#### Quick Setup ####

Turn the power knob to switch the device on. 

ARDF Mode can be disabled/enabled in the menu (ARDF = OFF/ON). This enables **manual gain control** 
by pressing **UP/DOWN keys**. The index reaches from 0 to 19, default is 15.

Gain steps should be roughly 5dB but they are uncalibrated. Expect surprises.

- Disable squelch (Menu: Sql = 0)
- Disable dual watch (Menu: RxMode = MAIN ONLY).
- Adjust clock correction if necessary (menu "ClkCor", details below).

- Select number of foxes in menu "NumFox" (default = 5, min = 1, max = 10).
- Choose duration time of one fox transmission in menu "FoxDur". Default is 60s. 
Modify with UP/DOWN key in 0.1s steps or enter value as *5 digit number* in 1/100s resolution. Confirm with menu button (min = 001.00s, max = 999.99s).
- Enter beep signal before end of fox cycle time (menu "EndSig", 0s = off, 1..30s).
- Activate VFO A (long pressing key "2 A/B" toggles between both VFOs.
- Enter the frequency of the foxes using the number keys. If memory mode is active long press "3 VFO MR" to change to frequency mode.
- Choose the modulation type of the foxes in menu "Demod" (e.g. AM).
- Select a narrow bandwidth mode in menu "BW W/N": "U 1K7" is 1.7 kHz, (this is the smallest possible value; shown as "U-") or "U 2K5" which is 2.5 kHz (shown as "N-" on the ARDF screen).
- Switch to VFO B by long pressing "2 A/B". This VFO will mostly be used for the return beacon.
- Configure frequency, modulation and bandwidth the same way as for VFO A.
- Change back to VFO A by long pressing "2 A/B".

- Unscrew antenna and add a directional antenna with good front/back ratio.
- Select a medium volume and try not to change it. Volume adjustment should be done with gain control (UP/DOWN keys).
- Listen for foxes, synchronize fox number and timer in menu with "ActFox" and "TiRst" options (details below).
- Start hunting foxes and have fun!

#### Main Screen ####

<img src="/images/ardf_mainscreen.jpg" width=800 />

This pictures shows the radio screen if ARDF mode is enabled. The manual gain index value can be chosen with UP/DOWN keys from 0 (silent) to 19 (max gain).
RSSI_max is the highest RSSI in the last half second. This is an uncalibrated raw value. (The RSSI value next to the S-meter shows roughly dBm.)
The **bold line** below show the settings for the **active VFO**:
VFO "A", demodulation "USB", smallest bandwidth mode "U-" (1.7kHz) and
receiving frequency "144.001 MHz". For this VFO the gain remember mode (details below) is active so the gain index history for up to 5 foxes is shown.
The active fox number is bold.

The **last line** shows the configuration for the **inactive VFO**. It is recommended to put the fox settings in VFO A and the return beacon on VFO B.
By long pressing "2 A/B" key can quickly be switched between both configurations.

On system boot the device starts with active fox number 1 and full duration time left. In menu "ActFox" the active fox can be changed. 
The timer can be reset in menu "TiRst". Select "TiRst" with "M"-key, a triangle appears. Another press on "M"-key will reset the timer.
Use this for synchronization. Active fox and timer are shown on the top in the status bar if the menu is opened.

To get a notice if the fox cycle is ending soon a beep sound can be played. The number of seconds for this tone before
the ending of a fox cycle can be configured in menu "EndSig" (1..30s or "off" for no signal). Please read the [headphone section](#headphones)
before activating this feature with headphones.


#### Gain Remember Mode ####
This firmware supports **gain remember** for manual gain control. If gain remember is activated, the
**manual gain index** is handled **for every fox separately**. At the **beginning of a fox cycle** the **last gain
index for this fox** is **restored from the last cycle**. The manual gain index history for up to 5 foxes is shown 
in the lower part of the screen. 

If gain remember is **off**, there is only one gain index for all foxes independent from any fox cycle times.
This feature can be **configured for both VFOs separately** (menu "GainRe": Off / VFO A / VFO B / BOTH).

A usage scenario would be to configure the fox frequency on VFO A with gain remember mode on. Put the return beacon 
frequency on VFO B without gain remember on. (The return beacon is transmitting permanently so there is no need for
different gains in different time slots.)

You can quickly switch between both VFOs by long pressing "2 (A/B)" or a configured function key (see below).

If gain remember is not activated on the actual VFO, no gain index history is shown in the lower part of the screen.


#### Clock Correction ####
The microcontroller of the radio has no crystal oscillator and runs on an internal 48 MHz resonator only.
So all timers in the CPU are not very precise. This leads to a heavily drifting clock. The firmware provides a correction
mechanism to reach acceptable ranges. Start ARDF mode and stop **how many seconds really pass** until **the radio tells 1 hour is up**. 
(Useful settings: NumFox = 10, FoxDur = 60.00s, sync to reference clock.) The formula for the correction value is:

$$ CorrectionValue = 6000 - (StoppedSeconds * 100)/60 $$

E.g. if the radio finished 60 one minute cycles in 59 min and 36 seconds = 59 * 60 + 36 = 3576s the calculation would be:

$$ CorrectionValue = 6000 - (3576 * 100)/60 = 40 $$

Enter this value in menu "ClkCor" using UP/DOWN keys (allowed range: -500 ... 500). The value is stored in eeprom.


#### Function keys ####
Two different ARDF actions can be mapped to function keys: 
* Set manual gain to a medium index value (10). It is recommended to configure "ARDF Set Med.Gain" to short press on F2 key, menu: F2Shrt) 
* Enable/Disable ARDF function (e.g. for long press on F1 key / menu: F1Long = "ARDF off/on").

It is recommended to configure "Switch VFO" to F1 short (menu F1Shrt).


#### Chirp ####
There is a [chirp](https://chirpmyradio.com/projects/chirp/wiki/Home) driver in folder [chirp_module/](chirp_module/) for this firmware. 
The radio (including ARDF settings) can be programmed only with this driver and not with the standard UV-K5 driver.

Chirp does not support separate settings for modulation and bandwidth, bandwidth is selected implicitly by the modulation. 
This chirp driver uses in AM mode already "narrow" bandwidth and "U1K7" (1.7kHz) for "NAM" and "USB".

 
#### Notes ####
* The lowest possible frequency of the receiver chip is 18 MHz. So this is NOT usable on 80 m without hardware modifications.
* If ARDF mode is active any TX functionality is disabled. However it is recommended to compile with ENABLE_PREVENT_TX=1 to 
permanently disable TX. This is done for official releases. ARDF receivers with a builtin TX functionality are not permitted 
in official competitions anyway. 
* You can glue an arrow on the volume knob to keep the position under control (simply cut a triangle from a sticker).
* It is possible (but not recommended because it is more complicated) to use memory mode instead of frequency mode on each VFO. 
If memory mode is active on the current VFO, frequency and memory number are are displayed alternately one after each other. 
Number keys change the memory number (enter 3 digits)  and not the frequency. You can switch between memory mode and 
frequency mode by long pressing "3 VFO/MR" key.
* If you modify settings in memory mode don´t forget to save them before switching off (menu "ChSave").


#### Troubleshooting ####
* The gain index history is flickering.
  * Disable dual watch (RxMode = MAIN ONLY).
* The entered frequency is modified.
  * Decrease frequency step in menu "Step" to desired channel raster.
* Chirp does read different channel settings than in my radio.
  * If you changed channel settings on the radio you have to store them first (menu "ChSave").

## Headphones

Headphones can be connected to the 2.5mm jack. The audio signal is between the tip and sleeve of the jack, but
there are some limitations:

> [!WARNING]
> **There is a DC voltage of around 4V between tip and sleeve**. Use a DC blocker before connecting your headphones to the device!
> Your **headphones might be destroyed** otherwise!

To make it even worse the DC voltage is switched off and on when playing signal tones (fox cycle end warning beep). This leads to
voltage peaks and crackling noises in your headphones.

I use this circuit for removing the DC voltage from the audio signal:
<img src="/images/headphone_adapter.png" width=800 />

The idea is to use a high pass to get rid of the DC voltage. The 10k resistor is for capacitor charging if no headphone is connected and can be left out.

My headphone channels are connected in a row, this leads to a resistance of around 32 ohm. By experiment capacitor values of around 1uF fit for this
configuration. The edge frequency is 

$$ f_{cut} = \frac {1} {2 \cdot \pi \cdot R \cdot C} = \frac {1} {2 \cdot \pi \cdot 32 \cdot 1\mu } = 4.97 ~kHz $$

and seems much too high. However the output amplifier of the radio is very powerful so some attenuation is desired. 
Try inline capacitor values of 0.1uF .. 10uF as start and adapt it to your headphones.

This might not be the best solution so don´t blame me for any damages. Better solutions are welcome!

> [!Warning]
> Use cheap headphones or some you can go without only!!!


## Radio performance

Please note that the Quansheng UV-Kx radios are not professional quality transceivers, their
performance is strictly limited. The RX front end has no track-tuned band pass filtering
at all, and so are wide band/wide open to any and all signals over a large frequency range.

Using the radio in high intensity RF environments will most likely make reception anything but
easy (AM mode will suffer far more than FM ever will), the receiver simply doesn't have a
great dynamic range, which results in distorted AM audio with stronger RX'ed signals.
There is nothing more anyone can do in firmware/software to improve that, once the RX gain
adjustment I do (AM fix) reaches the hardwares limit, your AM RX audio will be all but
non-existent (just like Quansheng's firmware).
On the other hand, FM RX audio will/should be fine.

But, they are nice toys for the price, fun to play with.

## User customization

You can customize the firmware by enabling/disabling various compile options, this allows
us to remove certain firmware features in order to make room in the flash for others.
You'll find the options at the top of "Makefile" ('0' = disable, '1' = enable) ..


|Build option | Description |
| --- | ---- |
|🧰 **STOCK QUANSHENG FEATURES**||
| ENABLE_UART | without this you can't configure radio via PC ! |
| ENABLE_AIRCOPY | easier to just enter frequency with butts |
| ENABLE_FMRADIO | WBFM VHF broadcast band receiver |
| ENABLE_NOAA | everything NOAA (only of any use in the USA) |
| ENABLE_VOICE | want to hear voices ? |
| ENABLE_VOX | |
| ENABLE_ALARM | TX alarms |
| ENABLE_TX1750 | side key 1750Hz TX tone (older style repeater access)|
| ENABLE_PWRON_PASSWORD | power-on password stuff |
| ENABLE_DTMF_CALLING | DTMF calling fuctionality, sending calls, receiving calls, group calls, contacts list etc. |
| ENABLE_FLASHLIGHT | enable top flashlight LED (on, blink, SOS) |
|🧰 **CUSTOM MODS**||
| ENABLE_BIG_FREQ | big font frequencies (like original QS firmware) |
| ENABLE_SMALL_BOLD | bold channel name/no. (when name + freq channel display mode) |
| ENABLE_CUSTOM_MENU_LAYOUT | changes how the menu looks like |
| ENABLE_KEEP_MEM_NAME | maintain channel name when (re)saving memory channel|
| ENABLE_WIDE_RX | full 18MHz to 1300MHz RX (though front-end/PA not designed for full range)|
| ENABLE_TX_WHEN_AM | allow TX (always FM) when RX is set to AM|
| ENABLE_F_CAL_MENU | enable the radios hidden frequency calibration menu |
| ENABLE_CTCSS_TAIL_PHASE_SHIFT | standard CTCSS tail phase shift rather than QS's own 55Hz tone method|
| ENABLE_BOOT_BEEPS | gives user audio feedback on volume knob position at boot-up |
| ENABLE_SHOW_CHARGE_LEVEL | show the charge level when the radio is on charge |
| ENABLE_REVERSE_BAT_SYMBOL | mirror the battery symbol on the status bar (+ pole on the right) |
| ENABLE_NO_CODE_SCAN_TIMEOUT | disable 32-sec CTCSS/DCS scan timeout (press exit butt instead of time-out to end scan) |
| ENABLE_AM_FIX | dynamically adjust the front end gains when in AM mode to help prevent AM demodulator saturation, ignore the on-screen RSSI level (for now) |
| ENABLE_AM_FIX_SHOW_DATA | show debug data for the AM fix |
| ENABLE_SQUELCH_MORE_SENSITIVE | make squelch levels a little bit more sensitive - I plan to let user adjust the values themselves |
| ENABLE_FASTER_CHANNEL_SCAN | increases the channel scan speed, but the squelch is also made more twitchy |
| ENABLE_RSSI_BAR | enable a dBm/Sn RSSI bar graph level in place of the little antenna symbols |
| ENABLE_AUDIO_BAR | experimental, display an audio bar level when TX'ing |
| ENABLE_COPY_CHAN_TO_VFO | copy current channel settings into frequency mode. Long press `1 BAND` when in channel mode |
| ENABLE_SPECTRUM | fagci spectrum analyzer, activated with `F` + `5 NOAA`|
| ENABLE_REDUCE_LOW_MID_TX_POWER | makes medium and low power settings even lower |
| ENABLE_BYP_RAW_DEMODULATORS | additional BYP (bypass?) and RAW demodulation options, proved not to be very useful, but it is there if you want to experiment |
| ENABLE_BLMIN_TMP_OFF | additional function for configurable buttons that toggles `BLMin` on and off wihout saving it to the EEPROM |
| ENABLE_SCAN_RANGES | scan range mode for frequency scanning, see wiki for instructions (radio operation -> frequency scanning) |
| ENABLE_PREVENT_TX | prevent TX in any case |
| ENABLE_ARDF | enable ARDF features |
|🧰 **DEBUGGING** ||
| ENABLE_AM_FIX_SHOW_DATA| displays settings used by  AM-fix when AM transmission is received |
| ENABLE_AGC_SHOW_DATA | displays AGC settings |
| ENABLE_UART_RW_BK_REGS | adds 2 extra commands that allow to read and write BK4819 registers |
|🧰 **COMPILER/LINKER OPTIONS**||
| ENABLE_CLANG | **experimental, builds with clang instead of gcc (LTO will be disabled if you enable this) |
| ENABLE_SWD | only needed if using CPU's SWD port (debugging/programming) |
| ENABLE_OVERLAY | cpu FLASH stuff, not needed |
| ENABLE_LTO | reduces size of compiled firmware but might break EEPROM reads (OVERLAY will be disabled if you enable this) |

## Compiler

arm-none-eabi GCC version 10.3.1 is recommended, which is the current version on Ubuntu 22.04.03 LTS.
Other versions may generate a flash file that is too big.
You can get an appropriate version from: https://developer.arm.com/downloads/-/gnu-rm

clang may be used but isn't fully supported. Resulting binaries may also be bigger.
You can get it from: https://releases.llvm.org/download.html

## Building

### Compile from source

1. Install gcc-arm-none-eabi 10.3.1 or later
1. Open Makefile
1. Edit build options, save Makefile changes
1. Run `make` in main project folder

If everything is ok `firmware.bin` (unmodified) and `firmware.packed.bin` (obscurified for official flash tool) can be found in the main folder.

### Docker build method

If you have docker installed you can use [compile-with-docker.bat](./compile-with-docker.bat) (Windows) or [compile-with-docker.sh](./compile-with-docker.sh) (Linux/Mac), the output files are created in `compiled-firmware` folder. This method gives significantly smaller binaries, I've seen differences up to 1kb, so it can fit more functionalities this way. The challenge can be (or not) installing docker itself.

### Windows environment build method

1. Open windows command line and run:
    ```
    winget install -e -h git.git Python.Python.3.8 GnuWin32.Make
    winget install -e -h Arm.GnuArmEmbeddedToolchain -v "10 2021.10"
    ```
2. Close command line, open a new one and run:
    ```
    pip install --user --upgrade pip
    pip install crcmod
    mkdir c:\projects & cd /D c:/projects
    git clone https://github.com/egzumer/uv-k5-firmware-custom.git
    ```
3. From now on you can build the firmware by going to `c:\projects\uv-k5-firmware-custom` and running `win_make.bat` or by running a command line:
    ```
    cd /D c:\projects\uv-k5-firmware-custom
    win_make.bat
    ```
4. To reset the repository and pull new changes run (!!! it will delete all your changes !!!):
    ```
    cd /D c:\projects\uv-k5-firmware-custom
    git reset --hard & git clean -fd & git pull
    ```

I've left some notes in the win_make.bat file to maybe help with stuff.




## Credits

Many thanks to various people on Telegram for putting up with me during this effort and helping:

* [OneOfEleven](https://github.com/OneOfEleven)
* [DualTachyon](https://github.com/DualTachyon)
* [Mikhail](https://github.com/fagci)
* [Andrej](https://github.com/Tunas1337)
* [Manuel](https://github.com/manujedi)
* @wagner
* @Lohtse Shar
* [@Matoz](https://github.com/spm81)
* @Davide
* @Ismo OH2FTG
* [OneOfEleven](https://github.com/OneOfEleven)
* @d1ced95
* and others I forget
* [egzumer](https://github.com/egzumer)

## Other sources of information

[ludwich66 - Quansheng UV-K5 Wiki](https://github.com/ludwich66/Quansheng_UV-K5_Wiki/wiki)<br>
[amnemonic - tools and sources of information](https://github.com/amnemonic/Quansheng_UV-K5_Firmware)

## License

Copyright 2023 Dual Tachyon
https://github.com/DualTachyon

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

## Example of ARDF changes/updates

<p float="left">
  <img src="/images/ardf2.jpg" width=300 />
  <img src="/images/ardf3.jpg" width=300 />
  <img src="/images/ardf6.jpg" width=300 />
  <img src="/images/ardf5.jpg" width=300 />
  <img src="/images/ardf4.jpg" width=300 />
  <img src="/images/ardf_sticker.jpg" width=300 />

</p>

