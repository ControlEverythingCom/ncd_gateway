#include "i2c_comm.h"

#include "master_gateway.h"

int processCommandI2C(int addr, String command){
    dpm("Command", command);
    int pos = command.indexOf(',');
    if(pos<0){
        dpm("Single Byte", command.toInt());
        Wire.beginTransmission(addr);
        Wire.write(command.toInt());
        return Wire.endTransmission();
    }
    String byte = "";
    bool open = false;
    while(pos>-1){
        byte = command.substring(0, pos);
        command = command.substring(pos+1);
        pos = command.indexOf(',');
        if(byte.equals("delay")){
            int mils = command.toInt();
            dpm("Delay", mils);
            delay(mils);
            return 0;
        }
        if(!open){
            Wire.beginTransmission(addr);
            open = true;
        }
        dpm("Writing", byte);
        Wire.write(byte.toInt());
    }
    dpm("Writing", command);
    Wire.write(command.toInt());
    return Wire.endTransmission();
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
        status=1;
        current = command.substring(0, del);
        command = command.substring(del+1);
        del = command.indexOf('|');
        
        dpm("Trying", current);
        while (tries<3 && status!=0){
            
            dpm("Try", tries);
            status = processCommandI2C(addr, current);
            tries++;
        }
    }
    tries=0;
    status=1;
    while (tries<3 && status!=0){
        dpm("Current", command);
        status = processCommandI2C(addr, command);
        tries++;
    }
    return status;
};

int writeCommandI2C(int addr, String command){
    int tries=0;
    int status=1;
    while (tries<3 && status!=0) {
        dpm("Single command", command);
        status = processCommandI2C(addr, command);
        tries++;
    }
    return status;
};

int readCommandI2C(int addr, String command, byte* buf, int size){
    int status = writeCommandI2C(addr, command);
    Wire.requestFrom(addr, size);
    int ind=0;
    while(ind<size){
        byte bv = Wire.read();
        buf[ind] = bv;
        ind++;
    }
    return status;
}
