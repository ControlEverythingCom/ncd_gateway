#include "ncd_gateway.h"
#include "spark_wiring_eeprom.h"

String firmware_version = "000029";
String eventReturns[5];
unsigned long tOut = 3000;

int SerialLock = 0;

void init_gateway(){
    Particle.function("deviceComm", gatewayCommand);
    Particle.subscribe("ncd_deviceCom", commandHandler, MY_DEVICES);
    Particle.variable("ncd_version", firmware_version);
    Wire.begin();
}

void loop_gateway(){
    if(Serial.available() > 0){
        //Serial.println('input');
        int rlen = Serial.available();
        byte buff[rlen];
        for(int rb=0; rb<rlen; rb++){
            buff[rb] = Serial.read();
            
        }
        int resLen = ncdApi(buff, true);
        if(resLen > 0){
            byte retBuff[resLen];
            for(int resb=0; resb<resLen; resb++){
                retBuff[resb] = Serial1.read();
                Serial.print(retBuff[resb]);
                Serial.print(' ');
            }
            Serial.write(retBuff, resLen);
            //return;
        }
        //Serial1.end();
        //Serial.println('fin');
    }
};
int hexToInt(String arg, byte bytes[], int length){
    arg.getBytes(bytes, length+1);
    for(int i=0;i<length;i++){
        if(bytes[i]>70) bytes[i] -= 32;
        if(bytes[i]>58) bytes[i] -= 7;
        bytes[i] -= 48;
    }
    return 1;
}
int base64ToInt(String arg, byte buff[], int length){
    byte bytes[length];
    arg.getBytes(bytes, length+1);
    int buffInd=0;
    int cbits=0;
    int cByte;
    for(int i=0;i<length;i++){
        cByte = bytes[i];
        if(cByte==43) cByte = 58;
        if(cByte==47) cByte = 59;
        if(cByte<65) cByte += 75;
        if(cByte<97) cByte += 6;
        cByte -= 71;
        switch(cbits){
            case 0:
                buff[buffInd] = cByte << 2;
                cbits = 6;
                break;
            case 2:
                buff[buffInd] = buff[buffInd]+cByte;
                buffInd++;
                cbits = 0;
                break;
            case 4:
                buff[buffInd] = buff[buffInd] + (cByte >> 2);
                buffInd++;
                buff[buffInd] = (cByte & 3) << 6;
                cbits = 2;
                break;
            case 6:
                buff[buffInd] = buff[buffInd] + (cByte >> 4);
                buffInd++;
                buff[buffInd] = (cByte & 15) << 4;
                cbits = 4;
                break;
        }
    }
    return 1;
}
int gatewayCommand(String arg){
    int length = arg.length();
    byte buff[length];
    base64ToInt(arg, buff, length);
    return ncdApi(buff);
}

void commandHandler(const char *event, const char *data){
    String newCommand = String(data);
    gatewayCommand(newCommand);
}
int ncdApi(byte packetBytes[]){
    return ncdApi(packetBytes, false);
}
int ncdApi(byte packetBytes[], bool ka){
    int buffLen = 1;
    switch(packetBytes[0]){
        case 184:
            {
                
                //byte pins[20] = {D0, D1, D2, D3, D4, D5, D6, D7, A0, A1, A2, A3, A4, A5, A6, A7, DAC1, DAC2, WKP, RX, TX};
                
                if(packetBytes[1]==1){
                    return digitalRead(packetBytes[2]);
                }else if(packetBytes[1] == 2){
                    digitalWrite(packetBytes[2], packetBytes[3]);
                    return 1;
                }
                //just read the status of the available GPIOs and return them
                 //int status = 0;
                 //for(int i=0;i<20;i++){
                    //status = status << 1;
                    //status += digitalRead(pins[i]);
                 //}
                 //return status;
                 return 1;
            }
        case 185:
            {
                return firmware_version.toInt();
            }
        case 186:
            {
                //I2C bus scan
                //Serial.println(String(packetBytes[1]));
                //Serial.println(String(packetBytes[2]));
                int start = packetBytes[1]*32+1;
                int end = start+32;
                int addrStatus;
                int status = 0;
                for(;start<end;start++){
                    Wire.beginTransmission(start);
                    addrStatus = Wire.endTransmission();
                    if(start+32 > end){
                        status = status << 1;
                    }
                    if(addrStatus > 0){
                        addrStatus = 1;
                    }else{
                        Serial.print("Device found on: ");
                        Serial.println(start);
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
                    for(;i<=max;i++){
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
                for(; wi<maskedOffsets; wi++){
                    writeVals[wi]=writeCommand[wi];
                    //Serial.println(writeVals[wi]);
                }
                
                writeCommandsI2C(addr, readCommand, readCommandLen);
                Wire.requestFrom(addr, readLen);
                for(int i=0;i<readLen;i++){
                    int current = Wire.read();
                    writeVals[wi] = mask(current, writeCommand[wi], maskOp);
                    //Serial.println(current);
                    //Serial.println(writeVals[wi]);
                    wi++;
                }
                
                return writeCommandsI2C(addr, writeVals, writeCommandLen);
            }
        case 190:
            {
                int delayTime;
                //if(packetBytes[1] == 2){
                    delayTime = (packetBytes[2] << 8) + packetBytes[3];
                //}else{
                   // delayTime = packetBytes[2];
               // }
                Serial.printf("Delaying for %i ",delayTime);
                delay(delayTime);
                return 1;
            }
        case 169:
            {
                
                Serial1.begin(115200);
                byte buff[packetBytes[1]]; 
                for(int i = 0; i<packetBytes[1]; i++){
                    buff[i] = packetBytes[i+2];
                    
                }
                Serial1.write(buff, packetBytes[1]);
                delay(2);
                int rlen = Serial1.available();
                if(rlen>0){
                    byte ret[rlen];
                    for(int rb = 0; rb<rlen; rb++){
                        ret[rb] = Serial1.read();
                    }
                    if(!ka) Serial1.end();
                    return 0;
                    //return bytesToInt(ret, rlen);
                }
                if(!ka) Serial1.end();
                return 0;
            }
        case 170:
            {
                while(SerialLock>0 && (millis()-SerialLock)<100){
                    delay(50);
                }
                SerialLock = millis();
                Serial1.begin(115200);
                int rlen=0;
                //SINGLE_THREADED_BLOCK(){
                    Serial1.write(packetBytes, packetBytes[1]+3);
                    //int SerialTO = millis()+10;
                   // while(rlen<1 && millis()>SerialTO){
                   delay(15);
                        rlen = Serial1.available();
                   // }
                    if(!ka && rlen>0){
                        byte ret[rlen];
                        for(int rb = 0; rb<rlen; rb++){
                            ret[rb] = Serial1.read();
                        }
                        return bytesToInt(ret, rlen);
                    }
                //}
                //set flag
                
                if(!ka){
                    SerialLock = 0;
                    //Serial1.end();
                }
                return rlen;
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
