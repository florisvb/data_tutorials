/*
 * This is a high speed data logger that automatically saves data to files,
 * periodically closing the files and opening a new one. Each time the teensy
 * is restarted it will start writing to a new directory, so that data is never
 * overwritten. Beware that teensy has a max file name length, so more than 
 * the code currently does not support having more than 99999999 files.
 * 
 * This code is based off of: 
 * https://github.com/MichaelStetner/TeensyDataLogger/blob/master/TeensyDataLogger.ino
 * And the following text is from that original code. The main changes are to automatically
 * save the data and start writing to a new file, and using a directory structure.
 * 
 * This is a high speed data logger. It acquires data and stores it on an SD 
 * card. In my tests using a Teensy 3.6 and a SanDisk Ultra 16GB SDHC micro SD 
 * card, I was able to sample an analog pin at 25 kHz.
 * 
 * It relies on the beta version of the SdFat library written by Bill Greiman,
 * which is available at https://github.com/greiman/SdFat-beta. I have tested
 * this sketch with revision ffbccb0 of SdFat-beta. This code was inpired by 
 * SdFat's LowLatencyLogger and TeensySdioDemo examples. It uses the same
 * binary file format as LowLatencyLogger, but with a bigger block size.
 * 
 * Here is how the code works. We have four buffers that are used to store the
 * data. The main loop is very simple: it checks to see if there are any full 
 * buffers, and if there are, it writes them to the SD card. When is the data 
 * acquired? Data is acquired in the function yield(), which is called 
 * whenever the Teensy is not busy with something else. yield() is called by 
 * the main loop whenever there is nothing to write to the SD card. yield() is 
 * also called by the SdFat library whenever it is waiting for the SD card.
 */

#include "SdFat.h"

// Pin with LED, which flashes whenever data is written to card, and does a 
// slow blink when recording has stopped.
const int LED_PIN = 13;

// 512 byte buffer for testing (probably want this to be bigger in practice)
const size_t BUF_DIM = 512;

// Sampling rate
const uint32_t sampleIntervalMillis = 10;

// Use total of four buffers.
const uint8_t BUFFER_BLOCK_COUNT = 4;

// Format for one data record
struct data_t {
  uint32_t time;
  uint32_t test1;
  char test2[24];
};
// Warning! The Teensy allocates memory in chunks of 4 bytes!
// sizeof(data_t) will always be a multiple of 4. For example, the following
// data record will have a size of 12 bytes, not 9:
// struct data_t {
//   uint32_t time; // 4 bytes
//   uint8_t  adc[5]; // 5 bytes
// }

// Use SdFatSdio (not SdFatSdioEX) because SdFatSdio spends more time in
// yield(). For more information, see the TeensySdioDemo example from the 
// SdFat library. 
SdFatSdio sd;

// File stuff
File file;
uint8_t dirname;
char dirname_char[8];
uint8_t fname = 1;
char fname_char[8];
char full_fname_char[16];
unsigned long fileSizeWrite = 4096; //each file will be this big (in bytes)

// Number of data records in a block.
const uint16_t DATA_DIM = (BUF_DIM - 4)/sizeof(data_t);

//Compute fill so block size is BUF_DIM bytes.  FILL_DIM may be zero.
const uint16_t FILL_DIM = BUF_DIM - 4 - DATA_DIM*sizeof(data_t);

// Format for one block of data
struct block_t {
  uint16_t count;
  uint16_t overrun;
  data_t data[DATA_DIM];
  uint8_t fill[FILL_DIM];
};

// Intialize all buffers
block_t block[BUFFER_BLOCK_COUNT];

// Initialize full queue
const uint8_t QUEUE_DIM = BUFFER_BLOCK_COUNT + 1;

// Index of last queue location.
const uint8_t QUEUE_LAST = QUEUE_DIM - 1;
block_t* curBlock = 0;
block_t* emptyStack[BUFFER_BLOCK_COUNT];
uint8_t emptyTop;
uint8_t minTop;

block_t* fullQueue[QUEUE_DIM];
uint8_t fullHead = 0;
uint8_t fullTail = 0;

uint32_t nextSampleMillis = 0;
bool fileIsClosing = false;
bool collectingData = false;
bool isSampling = false;
bool justSampled = false;


uint32_t test_millis = 0;
String str = "this is a test string";

//-----------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  while (!Serial) {
  }

  pinMode(LED_PIN, OUTPUT);

  // Put all the buffers on the empty stack.
  for (int i = 0; i < BUFFER_BLOCK_COUNT; i++) {
    emptyStack[i] = &block[i - 1];
  }
  emptyTop = BUFFER_BLOCK_COUNT;

  sd.begin();
  
  // every time teensy is rebooted, make a new directory
  dirname = 1;
  itoa(dirname, dirname_char, 10);
  while (sd.exists(dirname_char)){
    Serial.print("dirname exists: ");
    Serial.println(dirname_char);
    dirname ++;
    itoa(dirname, dirname_char, 10);
  }
  sd.mkdir(dirname_char);
  Serial.print("made directory: ");
  Serial.println(dirname_char);

  // get a new filename
  getFullFname();
  if (!file.open(full_fname_char, O_RDWR | O_CREAT)) {
    error("open failed");
  }
  else {
    Serial.print("Opened file for writing: ");
    Serial.println(full_fname_char);
  }
  
  Serial.print("Block size: ");
  Serial.println(BUF_DIM);
  Serial.print("Record size: ");
  Serial.println(sizeof(data_t));
  Serial.print("Records per block: ");
  Serial.println(DATA_DIM);
  Serial.print("Record bytes per block: ");
  Serial.println(DATA_DIM*sizeof(data_t));
  Serial.print("Fill bytes per block: ");
  Serial.println(FILL_DIM);
  Serial.println("Recording. Enter any key to stop.");
  delay(100);
  collectingData=true;
  nextSampleMillis = millis() + sampleIntervalMillis;
}
//-----------------------------------------------------------------------------
void loop() {
  
  if (file.size() > fileSizeWrite){
    fileIsClosing = true;
  }
  
  // Write the block at the tail of the full queue to the SD card
  if (fullHead == fullTail) { // full queue is empty
    if (fileIsClosing){
      file.close();
      Serial.println("File complete.");

      // open new file
      fileIsClosing = false;
      getFullFname();
      if (!file.open(full_fname_char, O_RDWR | O_CREAT)) {
        error("open failed");
      }
      else {
        Serial.print("Opened file for writing: ");
        Serial.println(full_fname_char);
      }
      
    } else {
      yield(); // acquire data etc.
    }
    
  } else { // full queue not empty
    // write buffer at the tail of the full queue and return it to the top of
    // the empty stack.
    digitalWrite(LED_PIN, HIGH);
    block_t* pBlock = fullQueue[fullTail];
    fullTail = fullTail < QUEUE_LAST ? fullTail + 1 : 0;
    if ((int)BUF_DIM != file.write(pBlock, BUF_DIM)) {
      error("write failed");
    }
    emptyStack[emptyTop++] = pBlock;
    digitalWrite(LED_PIN, LOW);
  }

}
//-----------------------------------------------------------------------------
void yield(){
  // This does the data collection. It is called whenever the teensy is not
  // doing something else. The SdFat library will call this when it is waiting
  // for the SD card to do its thing, and the loop() function will call this
  // when there is nothing to be written to the SD card.

  if (!collectingData || isSampling)
    return;
      
  isSampling = true;

  // If file is closing, add the current buffer to the head of the full queue
  // and skip data collection.
  if (fileIsClosing) {
    collectingData = true;
    isSampling = false;
    return;
  }
  
  // If we don't have a buffer for data, get one from the top of the empty 
  // stack.
  if (curBlock == 0) {
    curBlock = getEmptyBlock();
  }

  // If it's time, record one data sample.
  if (millis() >= nextSampleMillis) {
    if (justSampled) {
      error("rate too fast");
    }
    acquireData(&curBlock->data[curBlock->count++]);
    nextSampleMillis += sampleIntervalMillis;
    justSampled = true;
  } else {
    justSampled = false;
  }

  // If the current buffer is full, move it to the head of the full queue. We
  // will get a new buffer at the beginning of the next yield() call.
  if (curBlock->count == DATA_DIM) {
    putCurrentBlock();
  }

  isSampling = false;
}
//-----------------------------------------------------------------------------
block_t* getEmptyBlock() {
  /*
   * Takes a block form the top of the empty stack and returns it
   */
  block_t* blk = 0;
  if (emptyTop > 0) { // if there is a buffer in the empty stack
    blk = emptyStack[--emptyTop];
    blk->count = 0;
  } else { // no buffers in empty stack
    error("All buffers in use");
  }
  return blk;
}
//-----------------------------------------------------------------------------
void putCurrentBlock() {
  /*
   * Put the current block at the head of the queue to be written to card
   */
  fullQueue[fullHead] = curBlock;
  fullHead = fullHead < QUEUE_LAST ? fullHead + 1 : 0;
  curBlock = 0;
}
//-----------------------------------------------------------------------------
void error(String msg) {
  Serial.print("ERROR: ");
  Serial.println(msg);
  blinkForever();
}
//-----------------------------------------------------------------------------
void acquireData(data_t* data){
  data->time = millis();

  // these are constant values, in practice replace these with analog pin reads
  // or serial data
  str.toCharArray(data->test2, 24);
  data->test1 = 15;

  Serial.print(data->time);
  Serial.print(":  ");
  //Serial.println(data->wind);
  Serial.println(data->test2);
}
//-----------------------------------------------------------------------------
void blinkForever() {
  while (1) {
    digitalWrite(LED_PIN, HIGH);
    delay(1000);
    digitalWrite(LED_PIN, LOW);
    delay(1000);
  }
}

void getFullFname() {
  // every time teensy is rebooted, make a new directory
  itoa(fname, fname_char, 10);
  strcpy(full_fname_char, dirname_char);
  strcat(full_fname_char, "/");
  strcat(full_fname_char, fname_char);
  strcat(full_fname_char, ".bin");
  while (sd.exists(full_fname_char)){
    Serial.print("fname exists: ");
    Serial.println(full_fname_char);
    fname ++;
    itoa(fname, fname_char, 10);
    strcpy(full_fname_char, dirname_char);
    strcat(full_fname_char, "/");
    strcat(full_fname_char, fname_char);
    strcat(full_fname_char, ".bin");
  }
}
