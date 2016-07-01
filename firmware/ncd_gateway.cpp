#include "ncd_gateway.h"
#include "S3B.h"
#include "spark_wiring_eeprom.h"

String firmware_version = "000027";
S3B sModule;
String eventReturns[5];
unsigned long tOut = 3000;

void init_gateway(){
    Particle.function("deviceComm", gatewayCommand);
    Particle.subscribe("ncd_deviceCom", commandHandler, MY_DEVICES);
    Particle.variable("ncd_version", firmware_version);
    Serial1.begin(115200);
    Wire.begin();
}
int hexToInt(String arg, byte bytes[], int length){
    arg.getBytes(bytes, length+1);
    for(int i=0;i<length;i++){
        if(bytes[i]>70) bytes[i] -= 32;
        if(bytes[i]>58) bytes[i] -= 7;
        bytes[i] -= 48;
    }
    return 1;
}

int gatewayCommand(String arg){
    int length = arg.length();
    
    byte buff[length+1];
    hexToInt(arg, buff, length);
    
    int ind = 0;
    byte bytes[(length/2)+1];
    
    for(int i=0;i<length;i+=2){
        if(i>0) ind=i/2;
        bytes[ind]=(buff[i] << 4)+buff[i+1];
    }
    return ncdApi(bytes);
}

void commandHandler(const char *event, const char *data){
    String newCommand = String(data);
    gatewayCommand(newCommand);
}
int ncdApi(byte packetBytes[]){
    int buffLen = 1;
    Serial.println(String(packetBytes[0]));
    switch(packetBytes[0]){
        case 185:
            {
                return firmware_version.toInt();
            }
        case 186:
            {
                //I2C bus scan
                int start = packetBytes[1]*32+1;
                int end = start+32;
                int addrStatus;
                int status = 0;
                for(start;start<end;start++){
                    Wire.beginTransmission(start);
                    addrStatus = Wire.endTransmission();
                    if(start+32 > end){
                        status = status << 1;
                    }
                    if(addrStatus > 0){
                        addrStatus = 1;
                    }
                    status+=addrStatus;
                }
                return status;
            }
        case 187:
            {
                //packet of packets
                int i=2;
                int max;
                byte status[packetBytes[1]];
                for(int pn = 0; pn<packetBytes[1]; pn++){
                    
                    max=i+packetBytes[i];
                    Serial.println(max);
                    byte intPacket[packetBytes[i]];
                    i++;
                    int ni=0;
                    for(i;i<=max;i++){
                        intPacket[ni]=packetBytes[i];
                        ni++;
                    }
                    status[pn]=ncdApi(intPacket);
                }
                return bytesToInt(status, packetBytes[1]);
            }
        case 188:
            {
                //plain i2c w/r command
                if(packetBytes[3] > 0){
                    buffLen = packetBytes[3];
                }
                byte buff[buffLen];
                i2c_command(packetBytes, buff);
                return bytesToInt(buff, buffLen);
            }
        case 189:
            {
                //masking command
                int addr = packetBytes[1];
                int maskOp = packetBytes[2];
                int maskedOffsets = packetBytes[3];
                int readCommandLen = packetBytes[4];
                int readLen = packetBytes[5];
                int readCommand[readCommandLen];
                
                array_slice(packetBytes, 6, readCommandLen, readCommand);
                
                int writeCommandLen = packetBytes[6+readCommandLen];
                int writeCommand[writeCommandLen];
                
                array_slice(packetBytes, 7+readCommandLen, writeCommandLen, writeCommand);
                
                
                int writeVals[writeCommandLen];
                int wi=0;
                for(wi; wi<maskedOffsets; wi++){
                    writeVals[wi]=writeCommand[wi];
                    Serial.println(writeVals[wi]);
                }
                
                writeCommandsI2C(addr, readCommand, readCommandLen);
                Wire.requestFrom(addr, readLen);
                for(int i=0;i<readLen;i++){
                    int current = Wire.read();
                    writeVals[wi] = mask(current, writeCommand[wi], maskOp);
                    Serial.println(current);
                    Serial.println(writeVals[wi]);
                    wi++;
                }
                
                return writeCommandsI2C(addr, writeVals, writeCommandLen);
            }
        case 190:
            {
                int delayTime = (packetBytes[1] << 8) + packetBytes[2];
                delay(delayTime);
                return 1;
            }
    }
    return 1;
}
int mask(int val, int mask, int type){
    switch(type){
        case 0:
            val |= mask;
            break;
        case 1:
            val ^= mask;
            break;
        case 2:
            val &= ~mask;
            break;
        case 3:
            val &= mask;
            break;
        case 4:
            val = val << mask;
            break;
        case 5:
            val = val >> mask;
    }
    return val;
}
void i2c_command(byte bytes[], byte *buff){
    if(bytes[0] == 188){
        int commands[bytes[2]];
        array_slice(bytes, 4, bytes[2], commands);
        int addr = bytes[1];
        if(bytes[3] == 0){
            //write command
            buff[0]=writeCommandsI2C(addr, commands, bytes[2]);
        }else{
            //read command
            writeCommandsI2C(addr, commands, bytes[2]);
            Wire.requestFrom(addr, bytes[3]);
            for(int i=0;i<bytes[3];i++){
                buff[i] = Wire.read();
            }
        }
    }
}
void array_slice(byte bytes[], int start, int len, byte *buff){
    int ni = 0;
    int end = start+len;
    for(int i=start; i<=end; i++){
        buff[ni] = bytes[i];
        ni++;
    }
}
void array_slice(byte bytes[], int start, int len, int *buff){
    int ni=0;
    int end = start+len;
    for(int i=start; i<=end; i++){
        buff[ni] = bytes[i];
        ni++;
    }
}
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
int writeCommandsI2C(int addr, int* commands, int commandsLen){
    Serial.printf("Running I2C Command, address: %i data: ",addr);
    Wire.beginTransmission(addr);
    for(int i = 0; i < commandsLen; i++){
        Wire.write(commands[i]);
        Serial.printf("%i, ",commands[i]);
    }
    int status = Wire.endTransmission();
    Serial.printf(" Status: %i \n", status);
    return status;
};
int bytesToInt(byte bytes[], int length){
    int ret = bytes[0];
    for(int i=1; i<length; i++){
        ret = ret << 8;
        ret += bytes[i];
    }
    return ret;
}
