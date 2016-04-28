#include "i2c_comm.h"

int processCommandI2C(int addr, String command){
     //copy command into a char array
    char * cmdstr = new char[command.length() + 1];
    strcpy(cmdstr, command.c_str());
    
    //initialize variable
    int c = 0;
    
    //grab first part of string
    char * p = strtok(cmdstr, ",");
    
    Serial.print(addr);
    Serial.println(".");
    
    Serial.print(command);
    Serial.println(".");
    
    Wire.beginTransmission(addr);
    //loop through commands (',' delimited), using the first as the address and subsequent ones as commands
    while (p != NULL) {
        //copy command into int
        c = atoi(p);
        Wire.write(c);
        Serial.print("Command: ");
        Serial.println(c);
        //grab next part of string
        p = strtok(NULL, ",");
    }
    
    Serial.print("commands written");
    Serial.println(".");
    //end i2c transmission
    int status = Wire.endTransmission();
    
    
    Serial.print("status: ");
    Serial.println(status);
    delete cmdstr;
    
    return status;
}

int writeCommandsI2C(int addr, String command){
    int tries=0;
    int status=1;
    //char * cmdstr;
    //char * p;
    int del = command.indexOf('|');
    String current;
    if (del == -1) return writeCommandI2C(addr, command);
    while (del > 0) {
        tries=0;
        current = command.substring(0, del);
        command = command.substring(del+1);
        del = command.indexOf('|');
        while (tries<3 && status!=0){
            status = processCommandI2C(addr, current);
            tries++;
        }
    }
    tries=0;
    status=1;
    while (tries<3 && status!=0){
        status = processCommandI2C(addr, command);
        tries++;
    }
    return status;
};

int writeCommandI2C(int addr, String command){
    int tries=0;
    int status=1;
    while (tries<3 && status!=0) {
        status = processCommandI2C(addr, command);
        tries++;
    }
    return status;
};

int readCommandI2C(int addr, String command, byte* buf, int size){
    int s = command.indexOf(',');
    s = command.indexOf(',');
    int regAddr = command.substring(0, s).toInt();
    
    Wire.beginTransmission(addr);
    Wire.write(regAddr);
    int status = Wire.endTransmission();
    Wire.requestFrom(addr, size);
    int ind=0;
    while(ind<size){
        byte bv = Wire.read();
        buf[ind] = bv;
        ind++;
    }
    return status;
}
