#include "ncd_gateway.h"
#include "S3B.h"
#include "spark_wiring_eeprom.h"
#include "SparkJson/SparkJson.h"

S3B sModule;
String eventReturns[5];

int initCommands(JsonObject& root);

int i2c(JsonObject& root);
int s3b(JsonObject& root);


int sendEvent(String key){
    int i = key.toInt();
    String eventName = "";
    eventName = "eventReturn_"+key;
    Particle.publish(eventName, eventReturns[i], 60, PRIVATE);
    eventReturns[i] = "";
    return 1;
};
int setEventReturn(String value){
    int index = 0;
    while(eventReturns[index].length() > 1){
        index++;
    }
    eventReturns[index] = value;
    return index;
};

//i2c,32,writem,0,252|6,252

//S3B command format
//rs,0.19.162.0.65.2.32.216,wi,0,252|6,252

int gatewayCommand(String command){
    
    DynamicJsonBuffer jsonBuffer;
        
    Serial.println("Received: "+command);
    
    char commandData[command.length()+1];
    command.toCharArray(commandData, command.length()+1);
    
    //Parse incomming JSON
    JsonObject& root = jsonBuffer.parseObject(commandData);
    
    //Check to see if parsing Json object was successful.
    if(!root.success()){
        Serial.println("Failed to parse JSON");
        return 0;
    }
    for(int i = 0; i < root["root"].size(); i++){
        //Check type
        const char* typeChar = root["root"][i]["type"];
        String type = String(typeChar);
        Serial.println("type: "+type);
        
        if(type.equals("init")){
            initCommands(root["root"][i]);
        }
        if(type.equals("i2c")){
            Serial.println("I2C Command");
            i2c(root["root"][i]);
        }
        if(type.equals("s3b")){
            s3b(root["root"][i]);
        }
    }
    Serial.println("Done");
    return 0;
}

int bytesToInt(byte* b, int size){
    int val = 0;
    for(int i=3;i>-1;i--){ 
      val = val | b[i] << ((i)*8);
    }
    return val;
}

void dpm(String name, String arg){
    Serial.print(name);
    Serial.print(": ");
    Serial.println(arg);
}
void dpm(String name, int arg){
    Serial.print(name);
    Serial.print(": ");
    Serial.println(arg);
}

int i2c(JsonObject& root){
    int commandsRan = 0;
    for(int i = 0; i < root["args"].size(); i++){
        int addr = root["args"][i]["address"];
        Serial.printf("i2c sending command to address: %i data: ", addr);
        for(int j = 0; j < root["args"][i]["commands"].size(); j++){
            int data[root["args"][i]["commands"][j].size()];
            for(int k = 0; k < root["args"][i]["commands"][j].size(); k++){
                data[k] = root["args"][i]["commands"][j][k];
                Serial.printf("%i, ", data[k]);
            }
            Serial.println();
            writeCommandsI2C(addr, data, root["args"][i]["commands"][j].size());
            commandsRan++;
        }
    }
    Serial.printf("i2c Ran %i commands \n",commandsRan);
    return commandsRan;
}

int s3b(JsonObject& root){
    
}

//Returns the number of init commands stored to memory
//Format init,i2c,addr,write,arg1,arg2|i2c,addr,write,arg1,arg2
int initCommands(JsonObject& root){
    Serial.println("Init Command");
    
    char initCommandsToStore[255];
    root.printTo(initCommandsToStore, 255);
    String initCommandString = String(initCommandsToStore);
    Serial.printf("Length of initCommandString: %i \n", initCommandString.length());

    Serial.print("initCommand: ");
    Serial.println(initCommandsToStore);
    
    int sInd = 0;
    EEPROM.write(sInd, initCommandString.length());
    sInd++;
    for(int i = 0; i < initCommandString.length(); i++){
        EEPROM.write(sInd, initCommandsToStore[i]);
        sInd++;
    }
    Serial.printf("stored %i chars \n", sInd);
}

//Returns the number of init commands ran.
int runInitCommands(){
    if(EEPROM.read(0) == 255){
        Serial.println("No Init Commands Stored");
        return 0;
    }
    int numberOfCommandsRan = 0;
    
    int ind = 0;
    int commandStringLen = EEPROM.read(ind);
    ind++;
    char commandString[commandStringLen+1];
    for(int i = 0; i < commandStringLen; i++){
        commandString[i] = EEPROM.read(ind);
        ind++;
    }
    Serial.print("Stored Init: ");
    Serial.println(commandString);
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(commandString);
    
    //Check to see if parsing Json object was successful.
    if(!root.success()){
        Serial.println("Failed to parse init JSON");
        return 0;
    }else{
        Serial.println("Parse init JSON success");
    }
    i2c(root);
    return numberOfCommandsRan;
}

int clearInit(String args){
    for(int i = 0; i < 2047; i++){
        EEPROM.write(i, 255);
    }
    return 1;
}

