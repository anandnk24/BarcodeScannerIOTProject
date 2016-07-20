// This #include statement was automatically added by the Particle IDE.
#include "md5.h"

// This #include statement was automatically added by the Particle IDE.
#include "application.h"


#define MAX_BITS 100                 // max number of bits
#define WEIGAND_WAIT_TIME  3000      // time to wait for another weigand pulse.
unsigned char databits[MAX_BITS];    // stores all of the data bits
unsigned char bitCount;              // number of bits currently captured
unsigned char flagDone;              // goes low when data is currently being captured
unsigned int weigand_counter;        // countdown until we assume there are no more bits
unsigned long facilityCode=0;        // decoded facility code
unsigned long cardCode=0;            // decoded card code

//String hashArray[] = {"422478ec6d1218e76a0f51e2ef343f3b","e403c027c44f7b262a65950be0e7f69e","0c9f2b80a03450d11eadc6e8f7ac4c25","0039102352b8e6a9591f9e1aecf0266b","ed044180651185f3cc693ac2df8b4332","286f210770d62781b710a65be21b3d7d","98d569389e118633f469e6cb07af198f"};
//String nameArray[] = {"Ranny","Danny","Brett","Mike","Byron","David","Ben"};
String hashArray[] = {"5ece532b576a8f4c677f0f89bac52a65", "5a78f1d526e0a3c8b4f759968b660b8c", "eef6a11d6c02f4443cf9c5e91947536d", "09f4d591f9ff9e5e0bb6766b0188d017", "ff5f900bac84c058978c6b16f895131f", "37915daf6e872d58791b5e5cb144fc4a", "54bff62713e574c1097f56646402832a", "3ada268dec01dab0f6434c3ef50ec89f", "ec14fe5b3b9b9140e24d1e05046b3d85", "1e1aee7d1c2f371f8cd3442dcdc4e8cd", "15709800bdacf685676ca21dad559a45", "c16117de1309f508633dae03c0804a0b", "0ead0dd9d2b345a1fe8507437245d8f8", "3fb5bb321738f743b4dc240f7aa5ea23", "426ab2bdeb25e959cb46c7c717452d1e", "daa172021eb0b28d756615925486f129"};
String nameArray[] = {"Hallie Schwartz", "Emily King", "Pratik Prakash", "Arjit Jaiswal", "Jesse Shellabarger", "Aaron Leon", "Jessica Spratley", "Piyush Puri", "Mitch Laski", "Anand Kannappan", "Darren Jacoby", "Remy Bubulka", "Anurag Sharma", "Imani McLaurin", "Chad Kowalewski", "Jared Cohen"};
int ageArray[] = {0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1};

String md5(String line) {
  unsigned char result[16];

  MD5_CTX hash;
  MD5_Init(&hash);
  MD5_Update(&hash, line, line.length());
  MD5_Final(result, &hash);

  char buf[33];
  for (int i=0; i<16; i++)
    sprintf(buf+i*2, "%02x", result[i]);
  buf[32]=0;

  return String(buf);
}


// interrupt that happens when INTO goes low (0 bit)
void ISR_INT0()
{
  //Serial.print("0");   // uncomment this line to display raw binary
  bitCount++;
  flagDone = 0;
  weigand_counter = WEIGAND_WAIT_TIME;
}
// interrupt that happens when INT1 goes low (1 bit)
void ISR_INT1()
{
  //Serial.print("1");   // uncomment this line to display raw binary
  databits[bitCount] = 1;
  bitCount++;
  flagDone = 0;
  weigand_counter = WEIGAND_WAIT_TIME;
}
void setup()
{
  //pinMode(D7, OUTPUT);  // LED
  pinMode(D2, INPUT);     // DATA0 (INT0)
  pinMode(D3, INPUT);     // DATA1 (INT1)
  Serial.begin(9600);
  Serial.println("RFID Readers v2");
  // binds the ISR functions to the falling edge of INTO and INT1
  attachInterrupt(D2, ISR_INT0, FALLING);
  attachInterrupt(D3, ISR_INT1, FALLING);
  weigand_counter = WEIGAND_WAIT_TIME;
}
void loop()
{
  // This waits to make sure that there have been no more data pulses before processing data
  if (!flagDone) {
    if (--weigand_counter == 0)
      flagDone = 1;
  }
  // if we have bits and we the weigand counter went out
  if (bitCount > 0 && flagDone) {
    unsigned char i;
    //Serial.print("Read ");
    //Serial.print(bitCount);
    //Serial.print(" bits. ");
    // we will decode the bits differently depending on how many bits we have
    // see www.pagemac.com/azure/data_formats.php for mor info
    if (bitCount == 35)
    {
      // 35 bit HID Corporate 1000 format
      // facility code = bits 2 to 14
      for (i=2; i<14; i++)
      {
         facilityCode <<=1;
         facilityCode |= databits[i];
      }
      // card code = bits 15 to 34
      for (i=14; i<34; i++)
      {
         cardCode <<=1;
         cardCode |= databits[i];
      }
      noInterrupts();
    //   long hash = databits[0];
    //   for(int i = 1; i < arraySize(databits); i++) {
    //     hash << 1;
    //     hash |= databits[i];
    //   }
      String hash = md5(String(cardCode));
      for(int i = 0; i < arraySize(hashArray); i++){
          if(hashArray[i].compareTo(hash) == 0) {
            //Particle.publish("scanned", "data", PRIVATE);
            if(ageArray[i] == 1) {
              Particle.publish("scanned", "data", PRIVATE);
            }
            break;
          }
      }
    //   if(found == false){
    //       Particle.("Card Scanned","Card is not in database", 60, PRIVATE);
    //       Particle.publish("Hash code",hash);
    //   }
      //Serial.println(md5(String(facilityCode) + String(cardCode)));
      interrupts();

    }
    else if (bitCount == 26)
    {
      // standard 26 bit format
      // facility code = bits 2 to 9
      for (i=1; i<9; i++)
      {
         facilityCode <<=1;
         facilityCode |= databits[i];
      }
      // card code = bits 10 to 23
      for (i=9; i<25; i++)
      {
         cardCode <<=1;
         cardCode |= databits[i];
      }
    }
    else {
      // you can add other formats if you want!
    // Serial.println("Unable to decode.");
    }
     // cleanup and get ready for the next card
     bitCount = 0;
     facilityCode = 0;
     cardCode = 0;
     for (i=0; i<MAX_BITS; i++)
     {
       databits[i] = 0;
     }
  }
}