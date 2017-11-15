# Lubell
Arduino Code for Custom Lubell Pingers

## Dependencies

* [Modified WaveHC library](https://github.com/pvazteixeira/WaveHC) - clone this library into your sketchbook's libraries directory (e.g. `git clone https://github.com/pvazteixeira/WaveHC.git $HOME/sketchbook/libraries/WaveHC`).


## Examples

* `pps_chirp` - plays a hard-coded WAV file (chirp1.wav) a configurable amount of time (through delay.txt) after a PPS trigger. The code should adjust for the (file-dependent) time required to seek and load the waveform.
* `pps_chirp_select` - plays a hard-coded WAV file (chirp1.wav - chirp5.wav), selectable through analog pins 1-5, a configurable amount of time (through delay.txt)  after a PPS trigger. The code should adjust for the (file-dependent) time required to seek and load the waveform.

## Notes

* We trigger the waveform on the PPS falling edge, which occurs at some time after the second; by doing so, we can delay the onset of the waveform to the start of the second, and catch the next trigger, as long as the length of the waveform is less than the width of the PPS pulse.
* The PPS pulse width is consistent for a given GPS type, but varies between different GPS types.
* `pps_chirp` - there is a play delay caused by the loop code; this has been measured to 848 us.
* `pps_chirp_select` - there is a play delay caused by the loop code; this has been measured to 885 us.
* GPS18xLVC - PPS rising edge occurs at the start of the second, and PPS falling edge occurs exactly 100 ms after the start of the second.
  * `pps_chirp` - The delay time should be set to (900000-848)=899152 us.
  * `pps_chirp_select` - The delay time should be set to (900000-885)=899115 us.
