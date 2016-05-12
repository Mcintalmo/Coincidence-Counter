//Preliminary Arudino code to monitor three RM-60 Geiger counters
//Will count hits in 5-second interval (adjustable)
//Will monitor coincidences between Geiger counters as well
//No logging of data (yet) - only prints to serial monitor (screen)
//No tagging of data with timestamps or GPS stamps (yet)

// Looking into the port on the RM60, connector tab up, from left to right:
// Ground - Positive - Signal - Ground
// Usually Black - Red - Green - White/Yellow (Not always)

//Josh Nelson, Christopher Gosch, James Flaten
//U of MN, Twin Cities - March 2016

// 03/21/16 - Alex McIntosh - Added print to SD card option
// 04/07/16 - Alex McIntosh - Replaced values with constants and variables

// Put Libraries/Global Variables here
#include <SD.h>;
#include <SPI.h>;
#include <SoftwareSerial.h>
#include <Adafruit_GPS.h>

#define GPSECHO  true

const int SENSORPIN1 = 4;
const int SENSORPIN2 = 5;
const int SENSORPIN3 = 6;
const int SDPIN = 7;
const boolean SDOUT = true;
const char HEADER[ ] = "Time, Alt, Lat, Long, S1, S2, S3, Coin12, Coin13, Coin23, Coin123";
const char SEPERATOR[ ] = ", ";

long unsigned int timer = 0; 
long unsigned int Logtime = 5000; // Logging time in milliseconds
long unsigned int LocalTime = 0;
long unsigned int LoopLog = 150;
long int counter1 = 0; // Local Counter for Geiger counter sensor 1 hits
long int counter2 = 0; // Local Counter for Geiger counter sensor 2 (on top of 1) hits
long int counter3 = 0; // Local Counter for Geiger counter sensor 3 (on side of 1) hits
long int totalcount1 = 0; // Counter for Geiger counter sensor 1 hits
long int totalcount2 = 0; // Counter for Geiger counter sensor 2 hits
long int totalcount3 = 0; // Counter for Geiger counter sensor 3 hits
boolean hit1 = 0; // Tells if sensor 1 was low during the current while loop. 1 if true, 0 if false
boolean hit2 = 0; // Tells if sensor 2 was low during the current while loop. 1 if true, 0 if false
boolean hit3 = 0; // Tells if sensor 3 was low during the current while loop. 1 if true, 0 if false
long int coincidencecount12 = 0; // Counter for coincident hits between sensors 1 and 2 
long int coincidencecount13 = 0; // Counter for coincident hits between sensors 1 and 3
long int coincidencecount23 = 0; // Counter for coincident hits between sensors 2 and 3
long int coincidencecount123 = 0; // Counter for coincident hits between sensors 1,2, and 3. These hits do not add to other coincidence counters
long int totalcoincidencecount12 = 0;
long int totalcoincidencecount13 = 0;
long int totalcoincidencecount23 = 0;
long int totalcoincidencecount123 = 0;
long int tempcounter = 0;
SoftwareSerial mySerial(8, 7);
Adafruit_GPS GPS(&mySerial);
char GPSTime[ ] = "";
char altitude[ ] = "";
char latitude[ ] = "";
char longitude[ ] = "";
char dataLogFile[ ] = "datalog.txt";

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  if(SDOUT){
    Serial.print("Initializing SD card...");

    // see if the card is present and can be initialized:
    if (!SD.begin(SDPIN)) {
      Serial.println("Card failed, or not present.");
    }
    else {
      Serial.println("Card initialized.");
    }
  
    // Open the file on the SD card . note that only one file can be 
    // open at a time,so you have to close this one before opening another.
    File dataFile = SD.open(dataLogFile, FILE_WRITE);

    // The file is available, write to it:
    if (dataFile) {
      dataFile.println(HEADER);
      dataFile.close();
    }
    // If the file isn't open, pop up an error:
    else {
      Serial.print("error opening "); 
      Serial.println(dataLogFile);
    }
  }
  
  // Add in sensor setup/initialization
}

void loop() {
  // Check for sensor hits. Increment a counter if you get hits
  timer = millis();
  
  while((millis()-timer) < Logtime)
  {
    LocalTime = micros();
    int sensor1 = digitalRead(SENSORPIN1); // Read in the pin for sensor 1. Duplicate for multiple geiger counters
    int sensor2 = digitalRead(SENSORPIN2); // Read in the pin for sensor 2. Duplicate for multiple geiger counters
    int sensor3 = digitalRead(SENSORPIN3); // Read in the pin for sensor 3. Duplicate for multiple geiger counters
        
    if(sensor1==LOW)
    {
      counter1++;
      totalcount1++;
      hit1 = true;
       //Serial.print ("S1 Hit ");
       //Serial.println(counter1);
    }

    if(sensor2==LOW)
    {
      counter2++;
      totalcount2++;
      hit2 = true;
       //Serial.print ("S2 Hit ");
       //Serial.println(counter2);
    }

    if(sensor3==LOW)
    {
      counter3++;
      totalcount3++;
      hit3 = true;
       //Serial.print ("S3 Hit ");
       //Serial.println(counter3);
    }

    if(hit1==1 && hit2==1 && hit3==1) // These if statements increment the coincidence counters
    {
      coincidencecount123++;
      totalcoincidencecount123++;
      //coincidencecount12++;  //No longer needed now that the logic (below) doesn't use ELSE-IF statements
      //coincidencecount23++;
      //coincidencecount13++;
      tempcounter++;
    }
    
    if(hit1==1 && hit2==1)
    {
      coincidencecount12++;
      totalcoincidencecount12++;
      tempcounter++;
    }
    
    if(hit1==1 && hit3==1)
    {
      coincidencecount13++;
      totalcoincidencecount13++;
      tempcounter++;
    }
    
    if(hit2==1 && hit3==1)
    {
      coincidencecount23++;
      totalcoincidencecount23++;
      tempcounter++;
    }
    
    hit1 = false;
    hit2 = false;
    hit3 = false;

    while((micros() - LocalTime) < LoopLog)
    {
      delayMicroseconds(5);  //slow code down if needed, to let Geiger counters reset
    }
  }
  
  if(SDOUT){
    // Open the file on the SD card.
    File dataFile = SD.open(dataLogFile, FILE_WRITE);

    // If the file is available, write to it
    // Note: Current HEADER = "Time, Alt, Lat, Long, S1, S2, S3, Coin12, Coin13, Coin23, Coin123"
    if (dataFile) {
      dataFile.print(Logtime / 1000.);
      dataFile.print(SEPERATOR);
      dataFile.print(altitude);
      dataFile.print(SEPERATOR);
      dataFile.print(latitude);
      dataFile.print(SEPERATOR);
      dataFile.print(longitude);
      dataFile.print(SEPERATOR);
      dataFile.print(counter1);
      dataFile.print(SEPERATOR);
      dataFile.print(counter2);
      dataFile.print(SEPERATOR);
      dataFile.print(counter3);
      dataFile.print(SEPERATOR);
      dataFile.print(coincidencecount12);
      dataFile.print(SEPERATOR);
      dataFile.print(coincidencecount13);
      dataFile.print(SEPERATOR);
      dataFile.print(coincidencecount23);
      dataFile.print(SEPERATOR);
      dataFile.print(coincidencecount123);
      dataFile.println();
      dataFile.close();
    }
    // If the file isn't open, pop up an error:
    else {
      Serial.print("ERROR opening ");
      Serial.println(dataLogFile);
    }
  }

  Serial.print("S1 Hit Count: ");
  Serial.println(counter1);
  Serial.print("S2 Hit Count: ");
  Serial.println(counter2);
  Serial.print("S3 Hit Count: ");
  Serial.println(counter3);
  Serial.print("Sensor 1&2 Coincidence Count: ");
  Serial.println(coincidencecount12);
  Serial.print("Sensor 1&3 Coincidence Count: ");
  Serial.println(coincidencecount13);
  Serial.print("Sensor 2&3 Coincidence Count: ");
  Serial.println(coincidencecount23);
  Serial.print("Sensor 1&2&3 Coincidence Count: ");
  Serial.println(coincidencecount123);
  Serial.print("Over the past ");
  Serial.print(Logtime/1000);
  Serial.println(" seconds.");
  Serial.print("Cumulative Count on 1 is: ");
  Serial.println(totalcount1);
  Serial.print("Cumulative Count on 2 is: ");
  Serial.println(totalcount2);
  Serial.print("Cumulative Count on 3 is: ");
  Serial.println(totalcount3);
  Serial.print("Total (Cumulative) Count is: ");
  Serial.println(totalcount1 + totalcount2 + totalcount3);
  Serial.print("Cumulative Sensor 1&2 Coincidence Count: ");
  Serial.println(totalcoincidencecount12);
  Serial.print("Cumulative Sensor 1&3 Coincidence Count: ");
  Serial.println(totalcoincidencecount13);
  Serial.print("Cumulative Sensor 2&3 Coincidence Count: ");
  Serial.println(totalcoincidencecount23);
  Serial.print("Cumulative Sensor 1&2&3 Coincidence Count: ");
  Serial.println(totalcoincidencecount123);
  Serial.print("Total (Cumulative) Coincidence Count: ");
  Serial.println(tempcounter);

  // Reset short-term counters but not cumulative counters
  counter1 = 0;
  counter2 = 0;
  counter3 = 0;
  coincidencecount12 = 0;
  coincidencecount13 = 0;
  coincidencecount23 = 0;
  coincidencecount123 = 0;
  //tempcounter = 0;
 
  // Add any logging/generic sensor reads here. You will miss any hits that occur during this process.
  // Example things would be SD logging, GPS, Accel, Magnetometer
  // See existing examples with those sensors from Arduino workshops for wiring/code.
}
