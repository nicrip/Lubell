/* pps_chirp
* Play a waveform at the start of PPS signal.
* We purposefully delay the play command to account for unique per-file delays in WAV file loading:
*   - chirp1.wav is played at the start of a PPS trigger into pin 7
*   - this waveform is purposefully delayed so that it starts playing after the number of microseconds specified in delay.txt
*   - changing chirp1.wav changes the amount of time it takes to load the WAV file; we automatically acount for this variability by purposefully delaying the play
*/
#include <WaveHC.h>
#include <WaveUtil.h>

// put pi in flash memory
// const char pi[] PROGMEM = "3.1415926535897932384626433832795028841971693993751058209749";
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
char delayfile[20];
unsigned long delayTime;
unsigned long t0,t1,dt,delayDiff;
double avgLoadTime = 0;
//////////////////////////////////// SETUP

void setup() {
  // set up Serial library at 9600 bps
  Serial.begin(9600);
  
  PgmPrintln("WAV Player on PPS trigger");
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
  strcpy_P(delayfile, PSTR("delay.txt"));
  setupDelay(delayfile, &delayTime);
  strcpy_P(filename, PSTR("chirp1.wav"));
}

void setupDelay(char *delayname, unsigned long *delayval) {
  char ch;
  char st[20];
  int i = 0;
  
  if (!file.open(root, delayname)){
    PgmPrint("Could not open file ");
    Serial.print(delayname);
    return;
  }
  while (file.read(&ch, 1) == 1) {
    st[i] = ch;
    i++;
  }
  *delayval = strtoul(st, NULL, 0);
  Serial.print("Delay value is: ");
  Serial.print(*delayval);
  Serial.println("");
}

/////////////////////////////////// LOOP
void loop() {
  int oldPPSState=PPSState;
  PPSState = digitalRead(PPSPin);
  
  if(PPSState==LOW & oldPPSState==HIGH){
    //DEBUG PRINT - uncomment for debugging
    /*Serial.println("Triggered!!!");*/
    playcomplete(filename);
  }
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
  if (wave.isplaying) wave.stop();  // already playing something, so stop it!
    
  t0 = micros();                    // time how long it takes to load the file - start timer
  if (!file.open(root, name)){
    PgmPrint("Could not open file ");
    Serial.print(name);
    return;
  }
  if (!wave.create(file)){
    PgmPrintln("Not a valid WAV");
    return;
  }
  wave.load();                      // each WAV file takes a unique amount of time to load
  t1 = micros();                    // time how long it takes to load the file - stop timer
  dt = t1-t0;
  getAverageLoadTime(dt);
  delaymicros(static_cast<unsigned long>(delayTime-avgLoadTime));
  // ok time to play!
  wave.play();
}

/*
* Get moving average of load times
*/
void getAverageLoadTime(unsigned long dt){
  if (avgLoadTime == 0) {
    avgLoadTime = static_cast<double>(dt);
  } else {
    avgLoadTime = 0.9*avgLoadTime + 0.1*static_cast<double>(dt);
  }
  //DEBUG PRINT - uncomment for debugging
  /*Serial.print("Instant/Average load time (us): ");
  Serial.print(dt);
  Serial.print(" / ");
  Serial.println(static_cast<unsigned long>(avgLoadTime));*/
}

/*
* Delay in microseconds by delaying in milliseconds + microseconds remainder
*/
void delaymicros(unsigned long d){
  delay(d/1000);
  delayMicroseconds(d%1000); 
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

int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}
