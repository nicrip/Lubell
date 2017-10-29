/* PPSWaveSelect
* Play a waveform at the start of PPS signal.
* Note, there are small delays in play command - calibrate for this delay and you can compensate by setting the delay value in ms within files 'delay1.txt' - 'delay5.txt' on the SD card.
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
char delayfile1[20];
char delayfile2[20];
char delayfile3[20];
char delayfile4[20];
char delayfile5[20];
unsigned long delay1;
unsigned long delay2;
unsigned long delay3;
unsigned long delay4;
unsigned long delay5;
//////////////////////////////////// SETUP

void setup() {
  // set up Serial library at 9600 bps
  Serial.begin(9600);
  
  PgmPrintln("LFM Player on PPS trigger with selectable wave");
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
  strcpy_P(delayfile1, PSTR("delay1.txt"));
  strcpy_P(delayfile2, PSTR("delay2.txt"));
  strcpy_P(delayfile3, PSTR("delay3.txt"));
  strcpy_P(delayfile4, PSTR("delay4.txt"));
  strcpy_P(delayfile5, PSTR("delay5.txt"));
  
  Serial.print("For chirp1.wav - ");
  setupDelays(delayfile1, &delay1);
  Serial.print("For chirp2.wav - ");
  setupDelays(delayfile2, &delay2);
  Serial.print("For chirp3.wav - ");
  setupDelays(delayfile3, &delay3);
  Serial.print("For chirp4.wav - ");
  setupDelays(delayfile4, &delay4);
  Serial.print("For chirp5.wav - ");
  setupDelays(delayfile5, &delay5);
}

void setupDelays(char *delayname, unsigned long *delayval) {
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
  *delayval = atoi(st);
  Serial.print("Delay value is: ");
  printDouble(*delayval, 0);
  Serial.println("");
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
  int wave1State = digitalRead(A5);
  int wave2State = digitalRead(A4);
  int wave3State = digitalRead(A3);
  int wave4State = digitalRead(A2);
  int wave5State = digitalRead(A1);
  Serial.print("Triggered!!! Selected state: ");
  Serial.print(wave1State);
  Serial.print(wave2State);
  Serial.print(wave3State);
  Serial.print(wave4State);
  Serial.println(wave5State);
  if(wave1State==LOW){
    delay(delay1);
    playcomplete(filename1);
  } else if(wave2State==LOW) {
    delay(delay2);
    playcomplete(filename2);
  } else if(wave3State==LOW) {
    delay(delay3);
    playcomplete(filename3);
  } else if(wave4State==LOW) {
    delay(delay4);
    playcomplete(filename4);
  } else if(wave5State==LOW) {
    delay(delay5);
    playcomplete(filename5);
  }
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

void printDouble( double val, byte precision){
 // prints val with number of decimal places determine by precision
 // precision is a number from 0 to 6 indicating the desired decimial places
 // example: lcdPrintDouble( 3.1415, 2); // prints 3.14 (two decimal places)

 if(val < 0.0){
   Serial.print('-');
   val = -val;
 }

 Serial.print (int(val));  //prints the int part
 if( precision > 0) {
   Serial.print("."); // print the decimal point
   unsigned long frac;
   unsigned long mult = 1;
   byte padding = precision -1;
   while(precision--)
 mult *=10;

   if(val >= 0)
frac = (val - int(val)) * mult;
   else
frac = (int(val)- val ) * mult;
   unsigned long frac1 = frac;
   while( frac1 /= 10 )
padding--;
   while(  padding--)
Serial.print("0");
   Serial.print(frac,DEC) ;
 }
}
