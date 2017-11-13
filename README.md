# Lubell
Arduino Code for Custom Lubell Pingers

## Dependencies

* [Modified WaveHC library](https://github.com/pvazteixeira/WaveHC) - clone this library into your sketchbook's libraries directory (e.g. `git clone https://github.com/pvazteixeira/WaveHC.git $HOME/sketchbook/libraries/WaveHC`).


## Examples

* `pps_chirp` - plays a hard-coded WAV file a configurable amount of time after a PPS trigger. The code should adjust for the (file-dependent) time required to seek and load the waveform.
