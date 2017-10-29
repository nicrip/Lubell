/* PPSWave
* Play a waveform at the start of PPS signal.
* Note, there are small delays in play command - remember to account for this delay in your receiving code!!!
*/
#include <WaveHC.h>
#include <WaveUtil.h>

// put pi in flash memory
//const char pi[] PROGMEM = "3.1415926535897932384626433832795028841971693993751058209749";
SdReader card;    // This object holds the information for the card
FatVolume vol;    // This holds the information for the partition on the card
FatReader root;   // This holds the information for the volumes root directory
FatReader file;   // This object represent the WAV file for a pi digit or period
WaveHC wave;      // This is the only wave (audio) object, since we will only play one at a time
/*
* Define macro to put error messages in flash memory
*/
#define error(msg) error_P(PSTR(msg))

const int PPSPin = 7;  // the number of the PPS Pin
const int ledPin = 13; // board-mounted LED pin - DOESN'T WORK: PIN IS RESERVED BY WAVESHIELD!!! 
int PPSState = 0;      // variable for reading the PPS status
char filename[20];
//////////////////////////////////// SETUP

void setup() {
  // set up Serial library at 9600 bps
  Serial.begin(9600);
  
  PgmPrintln("LFM Player on PPS trigger");
  Serial.print("Free Ram: ");
  Serial.println(freeRam());
  
  if (!card.init()) {
    error("Card init. failed!");
  }
  if (!vol.init(card)) {
    error("No partition!");
  }
  if (!root.openRoot(vol)) {
    error("Couldn’t open dir");
  }
  
  PgmPrintln("Files found:");
  root.ls();
  pinMode(PPSPin, INPUT);
  strcpy_P(filename, PSTR("chirp1.wav"));
}

/////////////////////////////////// LOOP
unsigned digit = 0;

void loop() {
  // get next character from flash memory
  //char c = pgm_read_byte(&pi[digit++]);
  
  //if (c == 0) {
    //digit = 0;
    //Serial.println();
    //return;
  //}
  
  int oldPPSState=PPSState;
  PPSState = digitalRead(PPSPin);
  
  if(PPSState==HIGH & oldPPSState==LOW){
    speaknum(digit);
  }
}

/////////////////////////////////// HELPERS
void speaknum(unsigned c) {
  Serial.println("Triggered!!!");
  playcomplete(filename);
}

/*
* print error message and halt
*/
void error_P(const char *str) {
  PgmPrint("Error: ");
  SerialPrint_P(str);
  sdErrorCheck();
  while(1);
}

/*
* print error message and halt if SD I/O error
*/
void sdErrorCheck(void) {
  if (!card.errorCode()) return;
  PgmPrint("\r\nSD I/O error: ");
  Serial.print(card.errorCode(), HEX);
  PgmPrint(", ");
  Serial.println(card.errorData(), HEX);
  while(1);
}

/*
* Play a file and wait for it to complete
*/
void playcomplete(char *name) {
  playfile(name);
  while (wave.isplaying);
  // see if an error occurred while playing
  sdErrorCheck();
}
/*
* Open and start playing a WAV file
*/
void playfile(char *name){
  if (wave.isplaying) {// already playing something, so stop it!
    wave.stop(); // stop it
  }
  if (!file.open(root, name)){
    PgmPrint("Could not open file ");
    Serial.print(name);
    return;
  }
  if (!wave.create(file)){
    PgmPrintln("Not a valid WAV");
    return;
  }
  // ok time to play!
  wave.play();
}

int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}
