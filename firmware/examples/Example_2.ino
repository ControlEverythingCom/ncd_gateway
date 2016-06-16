/*
 *  You MUST include the sparkjson library in your app for this to validate!!!
 *
 */

// This #include statement was automatically added by the Particle IDE.
#include "SparkJson/SparkJson.h"

// This #include statement was automatically added by the Particle IDE.
#include "ncd_gateway/ncd_gateway.h"

// This #include statement was automatically added by the Particle IDE.
#include "ncd_gateway/i2c_comm.h"

// This #include statement was automatically added by the Particle IDE.
#include "ncd_gateway/S3B.h"

unsigned long tOut = 3000;

void commandHandler(const char *event, const char *data);

S3B module;

byte receiveBuffer[256];
                        

void setup() {
    Particle.function("deviceComm", gatewayCommand);
    Particle.subscribe("deviceCom", commandHandler);
    Particle.function("clearInit", clearInit);
    Serial1.begin(115200);
    Wire.begin();
    int initCommandsRan = runInitCommands();
    Serial.printf("Ran %i init commands \n", initCommandsRan);
}

void commandHandler(const char *event, const char *data){
    Serial.println("Got Event");
    String newCommand = String(data);
    gatewayCommand(newCommand);
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
            
            int count = 0;
            while(count != newDataLength+1 && millis() <= tOut+startTime){
                if(Serial1.available() != 0){
                    receiveBuffer[count] = Serial1.read();
                    count++;
                }
            }
            if(count < newDataLength+1){
                Serial.println("Timeout2");
                Serial.printf("Received Bytes: %i, expected %i \n", count, newDataLength+1);
                return;
            }
            Serial.printf("Received %i bytes \n", count);
            //We have all our data.
            byte newData[newDataLength+4];
            newData[0] = startDelimiter;
            newData[1] = lenMSB;
            newData[2] = lenLSB;
            for(int i = 3; i < newDataLength+4; i++){
                newData[i] = receiveBuffer[i-3];
            }
            Serial.print("Received: ");
            for(int i = 0; i < sizeof(newData); i++){
                Serial.printf("%x, ", newData[i]);
            }
            Serial.println();
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
            Serial.println("Command: "+stringCommand);
            gatewayCommand(stringCommand);
        }else{
            Serial.printf("First byte not valid, it was: 0x%x \n", startDelimiter);
        }
    }
}

