// This #include statement was automatically added by the Particle IDE.
#include "ncd_gateway.h"

// This #include statement was automatically added by the Particle IDE.
#include "i2c_comm.h"

// This #include statement was automatically added by the Particle IDE.
#include "S3B.h"

SYSTEM_THREAD(ENABLED);

S3B module;

byte S3BAddress[7][8] = {{0x00, 0x13, 0xA2, 0x00, 0x41, 0x02, 0x20, 0xD8}, 
                        {0x00, 0x13, 0xA2, 0x00, 0x41, 0x02, 0x20, 0x43}, 
                        {0x00, 0x13, 0xA2, 0x00, 0x40, 0xF2, 0x74, 0x79}, 
                        {0x00, 0x13, 0xA2, 0x00, 0x41, 0x02, 0x20, 0xD8}, 
                        {0x00, 0x13, 0xA2, 0x00, 0x41, 0x02, 0x20, 0xD8}, 
                        {0x00, 0x13, 0xA2, 0x00, 0x41, 0x02, 0x20, 0xD8},
                        {0x00, 0x13, 0xA2, 0x00, 0x41, 0x02, 0x20, 0xD8}};
                        
int relayStatus = 0;

void setup() {
    Particle.function("deviceComm", gatewayCommand);
    Serial1.begin(115200);
    Wire.begin();
    
  String initCommands = "i2c,32,write,0,0|6,0";
	byte initCommandBytes[initCommands.length()+1];
	initCommands.getBytes(initCommandBytes, initCommands.length()+1);
    module.transmit(S3BAddress[0], initCommandBytes, initCommands.length());
}

void loop() {
    //Receive S3B data
    if(Serial1.available()!=0){
        Serial.println("Got a command");
        byte startDelimiter = Serial1.read();
        if(startDelimiter == 0x7E){
            unsigned long startTime = millis();
            while(Serial1.available() < 2 && millis() <= tOut+startTime);
            if(Serial1.available() < 2){
                Serial.println("Timeout");
                return;
            }
            byte lenMSB = Serial1.read();
            byte lenLSB = Serial1.read();
            int newDataLength = (lenMSB*256)+lenLSB;
            while(Serial1.available() < newDataLength+1 && millis() <= tOut+startTime);
            if(Serial1.available() < newDataLength+1){
                Serial.println("Timeout2");
                Serial.printf("Serial1.available(): %i, expected %i \n", Serial1.available(), newDataLength+1);
                return;
            }
            //We have all our data.
            byte newData[newDataLength+4];
            newData[0] = startDelimiter;
            newData[1] = lenMSB;
            newData[2] = lenLSB;
            for(int i = 3; i < newDataLength+4; i++){
                newData[i] = Serial1.read();
            }
            //validate data
            if(!module.validateReceivedData(newData, newDataLength+4)){
                Serial.println("Invalid Data");
                return;
            }
            //get length of new data
            int receivedDataLength = module.getReceiveDataLength(newData);
            char receivedData[receivedDataLength];
            int validDataCount = module.parseReceive(newData, receivedData, newDataLength+4);
            if(validDataCount == receivedDataLength){

            }
            String stringCommand = String(receivedData);
            gatewayCommand(stringCommand);
        }else{
            Serial.printf("First byte not valid, it was: 0x%x \n", startDelimiter);
        }
    }
}

//Command format
//I2CAddress,cmd1|,cmd2|cmd3
