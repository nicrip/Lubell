/* pps_chirp_select
* Play a selected waveform at the start of PPS signal.
* We purposefully delay the play command to account for unique per-file delays in WAV file loading:
*   - chirp1.wav - chirp5.wav is played at the start of a PPS trigger into pin 7 (selected through analog pins 5-1)
*   - each waveform is purposefully delayed so that it starts playing after the number of microseconds specified in delay.txt
*   - changing WAV files changes the amount of time it takes to load each WAV file; we automatically acount for this variability by purposefully delaying the play
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
char filename1[20];
char filename2[20];
char filename3[20];
char filename4[20];
char filename5[20];
char delayfile[20];
unsigned long delayTime;
unsigned long t0,t1,dt,delayDiff;
double avgLoadTime1 = 0;
double avgLoadTime2 = 0;
double avgLoadTime3 = 0;
double avgLoadTime4 = 0;
double avgLoadTime5 = 0;
double avgLoadTime;
//////////////////////////////////// SETUP

void setup() {
  // set up Serial library at 9600 bps
  Serial.begin(9600);
  
  PgmPrintln("WAV Player on PPS trigger with selectable wave");
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
  pinMode(A5, INPUT);
  digitalWrite(A5, HIGH);
  pinMode(A4, INPUT);
  digitalWrite(A4, HIGH);
  pinMode(A3, INPUT);
  digitalWrite(A3, HIGH);
  pinMode(A2, INPUT);
  digitalWrite(A2, HIGH);
  pinMode(A1, INPUT);
  digitalWrite(A1, HIGH);
  strcpy_P(filename1, PSTR("chirp1.wav"));
  strcpy_P(filename2, PSTR("chirp2.wav"));
  strcpy_P(filename3, PSTR("chirp3.wav"));
  strcpy_P(filename4, PSTR("chirp4.wav"));
  strcpy_P(filename5, PSTR("chirp5.wav"));
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
unsigned digit = 0;

void loop() {
  int oldPPSState=PPSState;
  PPSState = digitalRead(PPSPin);
  
  int wave1State = digitalRead(A5);
  int wave2State = digitalRead(A4);
  int wave3State = digitalRead(A3);
  int wave4State = digitalRead(A2);
  int wave5State = digitalRead(A1);
  
  if(PPSState==HIGH & oldPPSState==LOW){
    Serial.print("Triggered!!! Selected state: ");
    Serial.print(wave1State);
    Serial.print(wave2State);
    Serial.print(wave3State);
    Serial.print(wave4State);
    Serial.println(wave5State);
    if(wave1State==LOW){
      playcomplete(filename1, 1);
    } else if(wave2State==LOW) {
      playcomplete(filename2, 2);
    } else if(wave3State==LOW) {
      playcomplete(filename3, 3);
    } else if(wave4State==LOW) {
      playcomplete(filename4, 4);
    } else if(wave5State==LOW) {
      playcomplete(filename5, 5);
    }
  }
}

/*
* Play a file and wait for it to complete
*/
void playcomplete(char *name, unsigned int select) {
  playfile(name, select);
  while (wave.isplaying);
  // see if an error occurred while playing
  sdErrorCheck();
}

/*
* Open and start playing a WAV file
*/
void playfile(char *name, unsigned int select){
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
  getAverageLoadTime(dt, select);
  delaymicros(static_cast<unsigned long>(delayTime-avgLoadTime));
  // ok time to play!
  wave.play();
}

/*
* Get moving average of load times
*/
void getAverageLoadTime(unsigned long dt, unsigned int select){
  if (select == 1) {
    avgLoadTime = avgLoadTime1;
  } else if (select == 2) {
    avgLoadTime = avgLoadTime2;
  } else if (select == 3) {
    avgLoadTime = avgLoadTime3;
  } else if (select == 4) {
    avgLoadTime = avgLoadTime4;
  } else if (select == 5) {
    avgLoadTime = avgLoadTime5;
  }
  if (avgLoadTime == 0) {
    avgLoadTime = static_cast<double>(dt);
  } else {
    avgLoadTime = 0.9*avgLoadTime + 0.1*static_cast<double>(dt);
  }
  Serial.print("Instant/Average load time for selection ");
  Serial.print(select);
  Serial.print(" (us): ");
  Serial.print(dt);
  Serial.print(" / ");
  Serial.println(static_cast<unsigned long>(avgLoadTime));
  if (select == 1) {
    avgLoadTime1 = avgLoadTime;
  } else if (select == 2) {
    avgLoadTime2 = avgLoadTime;
  } else if (select == 3) {
    avgLoadTime3 = avgLoadTime;
  } else if (select == 4) {
    avgLoadTime4 = avgLoadTime;
  } else if (select == 5) {
    avgLoadTime5 = avgLoadTime;
  }
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
