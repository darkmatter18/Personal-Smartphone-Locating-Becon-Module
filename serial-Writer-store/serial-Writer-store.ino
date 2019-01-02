#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
int pinLED = 13;

const int ST_0 = 0;        //waiting Sync word
const int ST_1_CMD = 1;    //Waiting CMD
const int ST_2_LENGTH = 2; //Receiving Length for CMD_03_TEXT
const int ST_3_DATA = 3;   //Receiving Data for CMD_03_TEXT
const byte IGNORE_00 = 0x00;
const byte SYNC_WORD = 0xFF;
const byte CMD_01_LEDON = 0x01;
const byte CMD_02_LEDOFF = 0x02;
const byte CMD_03_TEXT = 0x03;
int cmdState;
int dataIndex;

const int MAX_LENGTH = 16;
byte data[MAX_LENGTH];
int dataLength;

bool textReceived = false;      //Checks if any text is received or not
char receivedMsg[MAX_LENGTH];   //loads the received msg

void setup(){
    Serial.begin(9600);
    pinMode(pinLED, OUTPUT);
    digitalWrite(pinLED, LOW);

    lcd.begin(16, 2);
    lcd.print("USB Host Mode:");
}

void loop(){

    if (Serial.available()){
        //lcd.setCursor(0, 1);
        //lcd.print("                ");
        //delay(100);
        //lcd.setCursor(0, 1);
        while (Serial.available() > 0){
            int byteIn = Serial.read();
            Serial.write(byteIn);
            //lcd.write(byteIn);
            cmdHandle(byteIn);
        }
    }

    if(textReceived){
        //Transmit
        Serial.write(receivedMsg);
        textReceived = false; // Make textReceived false on completion of transmittion
    }
}

void cmdHandle(int incomingByte){

    //prevent from lost-sync
    if (incomingByte == SYNC_WORD){
        cmdState = ST_1_CMD;
        return;
    }
    if (incomingByte == IGNORE_00){
        return;
    }

    switch (cmdState){
        case ST_1_CMD:{
            
            switch (incomingByte){
            
                case CMD_01_LEDON:
                    digitalWrite(pinLED, HIGH);
                    break;

                case CMD_02_LEDOFF:
                    digitalWrite(pinLED, LOW);
                    break;
                
                case CMD_03_TEXT:
                    for (int i = 0; i < MAX_LENGTH; i++){
                        data[i] = 0;
                        receivedMsg[i] = '\0';
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
            
            if (dataLength > MAX_LENGTH){
                dataLength = MAX_LENGTH;
            }
            cmdState = ST_3_DATA;
        }
        break;
    
        case ST_3_DATA:{
            data[dataIndex] = incomingByte;
            receivedMsg[dataIndex] = (char)incomingByte;
            dataIndex++;

            //Serial.write(incomingByte);
            lcd.write(incomingByte);

            if (dataIndex == dataLength){
                cmdState = ST_0;
            }

            textReceived = true;
        }
        break;
    }
}
