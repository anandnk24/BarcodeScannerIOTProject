// This #include statement was automatically added by the Particle IDE.
#include "md5/md5.h"
#include "application.h"
​
​
 
#define MAX_BITS 100                 // max number of bits 
#define WEIGAND_WAIT_TIME  3000      // time to wait for another weigand pulse.  
 
unsigned char databits[MAX_BITS];    // stores all of the data bits
unsigned char bitCount;              // number of bits currently captured
unsigned char flagDone;              // goes low when data is currently being captured
unsigned int weigand_counter;        // countdown until we assume there are no more bits
 
unsigned long facilityCode=0;        // decoded facility code
unsigned long cardCode=0;            // decoded card code
​
String hashArray[] = {"422478ec6d1218e76a0f51e2ef343f3b","e403c027c44f7b262a65950be0e7f69e","0c9f2b80a03450d11eadc6e8f7ac4c25","0039102352b8e6a9591f9e1aecf0266b","ed044180651185f3cc693ac2df8b4332","286f210770d62781b710a65be21b3d7d","98d569389e118633f469e6cb07af198f"};
String nameArray[] = {"Ranny","Danny","Brett","Mike","Byron","David","Ben"};

// String hashArray[] = {"22619", "22616", "22628", "22617", "18812", "18862", "18805", "22626", "22637", "19519", "20347", "22635", "21876", "22621", "22608"};
// String nameArray[]​ = {"Hallie Schwartz", "Emily King", "Pratik Prakash", "Arjit Jaiswal", "Jesse Shellabarger", "Aaron Leon", "Jessica Spratley", "Piyush Puri", "Mitch Laski", "Anand Kannappan", "Darren Jacoby", "Remy Bubulka", "Anurag Sharma", "Imani McLaurin", "Chad Kowalewski"};
// int ageArray[] = {0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, };
​
String md5(String line) {
  unsigned char result[16];
​
  MD5_CTX hash;
  MD5_Init(&hash);
  MD5_Update(&hash, line, line.length());
  MD5_Final(result, &hash);
​
  char buf[33];
  for (int i=0; i<16; i++)
    sprintf(buf+i*2, "%02x", result[i]);
  buf[32]=0;
​
  return String(buf);
}
​
​
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
  Serial.println("RFID Readers");
 
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
      
      bool found = false;;
      String hash = md5(String(facilityCode) + String(cardCode));
      for(int i = 0; i < arraySize(hashArray); i++){
          if(hashArray[i].compareTo(hash) == 0) {
            Particle.publish("rfid-slack-post",nameArray[i] + " Badged in!!!!!!!!!!!!!!!!!!", 60, PRIVATE);
            found = true;
            break;
          }
      }
      if(found == false){
          Particle.publish("rfid-slack-post","Card is not in database", 60, PRIVATE);
          Particle.publish("Hash code",hash);
      }
         
      //Serial.println(md5(String(facilityCode) + String(cardCode)));
     
      interrupts();
​
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