//Include Header file
#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);                  //Declearing lcd pins
int pinLED = 9;                                         //Declearing LED pin for Android input

int PIN_OUT = 13;                                       //Define the LED Pin for MorseCode

#define UNIT_LENGTH    250                              //Define unit length in ms

const int ST_0 = 0;                                     //waiting Sync word
const int ST_1_CMD = 1;                                 //Waiting CMD
const int ST_2_LENGTH= 2;                               //Receiving Length for CMD_03_TEXT
const int ST_3_DATA= 3;                                 //Receiving Data for CMD_03_TEXT
const byte IGNORE_00 = 0x00;                            //There Variables are pre-decleared in android side
const byte SYNC_WORD = 0xFF;                            //For Perform a equality cheak;
const byte CMD_01_LEDON = 0x01;                         //This is Led On var
const byte CMD_02_LEDOFF= 0x02;                         //This is Led Off Var
const byte CMD_03_TEXT  = 0x03;                         //This var used to receive String
int cmdState;                                           //Uses as a var to store incoming operaton
int dataIndex;                                          //Used for index data

const int MAX_LENGTH = 16;                              //Decleared Maximum dataLength 
byte data[MAX_LENGTH];                                  //Decleared a arrey for receiving data
int dataLength;                                         //Used to store the dataLength of Receiving byte

char receiveMessage;                                    //Used to convert receivedData to receiveMessage for Morse
char arrReceiveMassage[20];                             //making an array of receivedmessage
int xi;                                                 //needed for arrReceiveMassage indexing 

//Build a struct with the morse code mapping
static const struct{
    const char letter, *code;
    } MorseMap[] =
        {   
            //Small letter
            { 'a', ".-" },{ 'b', "-..." },{ 'c', "-.-."},{ 'd', "-.." },{ 'e', "." },{ 'f', "..-." },{ 'g', "--." },{ 'h', "...." },{ 'i', ".." },
            { 'j', ".---" },{ 'k', ".-.-" },{ 'l', ".-.." },{ 'm', "--" },{ 'n', "-." },{ 'o', "---" },{ 'p', ".--." },{ 'q', "--.-" },
	        { 'r', ".-." },{ 's', "..." },{ 't', "-" },{ 'u', "..-" },{ 'v', "...-" },{ 'w', ".--" },{ 'x', "-..-" },{ 'y', "-.--" },{ 'z', "--.." },

            //Capital Letter
            { 'A', ".-" },{ 'B', "-..." },{ 'C', "-.-."},{ 'D', "-.." },{ 'E', "." },{ 'F', "..-." },{ 'G', "--." },{ 'H', "...." },{ 'I', ".." },
            { 'J', ".---" },{ 'K', ".-.-" },{ 'L', ".-.." },{ 'M', "--" },{ 'N', "-." },{ 'O', "---" },{ 'P', ".--." },{ 'Q', "--.-" },
	        { 'R', ".-." },{ 'S', "..." },{ 'T', "-" },{ 'U', "..-" },{ 'V', "...-" },{ 'W', ".--" },{ 'X', "-..-" },{ 'Y', "-.--" },{ 'Z', "--.." },

            //space
            { ' ', "     " }, //Gap between word, seven units 

            //Numbers
	        { '1', ".----" },{ '2', "..---" },{ '3', "...--" },{ '4', "....-" },{ '5', "....." },{ '6', "-...." },{ '7', "--..." },{ '8', "---.." },
	        { '9', "----." },{ '0', "-----" },

            //Symbols
	        { '.', "·–·–·–" },{ ',', "--..--" },{ '?', "..--.." },{ '!', "-.-.--" },{ ':', "---..." },{ ';', "-.-.-." },{ '(', "-.--." },
            { ')', "-.--.-" },{ '"', ".-..-." },{ '@', ".--.-." },{ '&', ".-..." },
        };

void setup() {
    Serial.begin(9600);
    pinMode(pinLED, OUTPUT);                            //Assign Android led pin as output pin
    digitalWrite(pinLED, LOW);                          //Assign android led pin as LOW
    pinMode( PIN_OUT, OUTPUT );                         //Assign Morse led as output pin
    digitalWrite( PIN_OUT, LOW );                       //Assign Morse led pin as LOW
  
    lcd.begin(16, 2);                                   // 16*2 LCD decleared
    lcd.print("Host Mode:");                            //Print the Line
}

void loop(){                             
    //Getting Data via Serial
    if(Serial.available()){                             //Cheak if the serial is available
        while(Serial.available() > 0){                  //If Serial is avaiable, then
            int byteIn = Serial.read();                 //Read data from serial and
            cmdHandle(byteIn);                          //Put data one bit by another -using loop
            arrReceiveMassage[xi] = receiveMessage;     //Making an array by received char
            xi++;
        }                                               //Unless Serial stop sendind data 
    }
    //Morse code Execution
    String morseWord = encode(arrReceiveMassage);

    //Gives the LED responce
    for(int i=0; i<=morseWord.length(); i++){
        switch( morseWord[i] ){
            case '.': //dit
                digitalWrite( PIN_OUT, HIGH );
                delay( UNIT_LENGTH );
                digitalWrite( PIN_OUT, LOW );
                delay( UNIT_LENGTH );
                break;

            case '-': //dah
                digitalWrite( PIN_OUT, HIGH );
                delay( UNIT_LENGTH*3 );
                digitalWrite( PIN_OUT, LOW );
                delay( UNIT_LENGTH );
                break;

            case ' ': //gap
                delay( UNIT_LENGTH );
        }
    }
}

//Function that handles all received bytes from phone
void cmdHandle(int incomingByte){
  
    //prevent from lost-sync
    if(incomingByte == SYNC_WORD){
        cmdState = ST_1_CMD;
        return;
    }
    if(incomingByte == IGNORE_00){
        return;
    }
    
    //Switch for use commend state
    switch(cmdState){
        case ST_1_CMD:{
            switch(incomingByte){
                case CMD_01_LEDON:
                    digitalWrite(pinLED, HIGH);
                    break;
                case CMD_02_LEDOFF:
                    digitalWrite(pinLED, LOW);
                    break;
                case CMD_03_TEXT:
                    for(int i=0; i < MAX_LENGTH; i++){
                        data[i] = 0;
                    }
                    lcd.setCursor(0, 1);
                    lcd.print("                ");
                    lcd.setCursor(0, 1);
            
                    cmdState = ST_2_LENGTH;
                    dataIndex = 0;
                    break;
                default:
                    cmdState = ST_0;
            }
        }
        break;
        case ST_2_LENGTH:{
            dataLength = incomingByte;
            if(dataLength > MAX_LENGTH){
                dataLength = MAX_LENGTH;
            }
            cmdState = ST_3_DATA;
        }
        break;
        case ST_3_DATA:{
            data[dataIndex] = incomingByte;
            dataIndex++;
          
            Serial.write(incomingByte);
            lcd.write(incomingByte);
          
            receiveMessage = (char)incomingByte;

            if(dataIndex==dataLength){
                cmdState = ST_0; 
            }
        }
        break;
    }
  
}

//Function to endone the String to morse code
String encode(const char *string){
    size_t i, j;
    String morseWord = "";
  
    for( i = 0; string[i]; ++i ){
        for( j = 0; j < sizeof MorseMap / sizeof *MorseMap; ++j ){
            if( toupper(string[i]) == MorseMap[j].letter ){
                morseWord += MorseMap[j].code;
                break;
            }
        }
        morseWord += " "; //Add tailing space to seperate the chars
    }
    return morseWord;  
}
