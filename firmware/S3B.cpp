#include "S3B.h"

bool S3B::transmit(byte *address, byte *data, int len){
    int tlen = len+14;
    byte payload[len+18];
    // len = len+14;
    
    byte lsb = (tlen & 255);
    byte msb = (tlen >> 8);
    
    // Serial.printf("MSB: %i \n", msb);
    // Serial.printf("LSB: %i \n", lsb);
    
    //build the packet
    payload[0] = startDelimiter;
    payload[1] = msb;
    payload[2] = lsb;
    payload[3] = frameType;
    payload[4] = frameID;
    //populate address in packet
    for(int i = 0; i < 8; i++){
        payload[5+i] = address[i];
    }
    payload[13] = reserved1;
    payload[14] = reserved2;
    payload[15] = bRadius;
    payload[16] = transmitOptions;
    //populate data in packet
    for(int i = 0; i < len; i++){
        payload[17+i] = data[i];
    }
    int c = 0;
    for(int i = 3; i < len+17; i++){
        c += payload[i];
    }
    byte checksum = 0xFF - (c & 0xFF);
    payload[len+17] = checksum;
    Serial1.write(payload, len+18);
    Serial.print("Sending: ");
    for(int i = 0; i < len+18; i++){
        Serial.printf("%x, ", payload[i]);
    }
    Serial.println();
    return true;
};

bool S3B::parseAddress(String addr, byte *buffer){
    int ind = 0;
    int pos = addr.indexOf('.');
    while(pos>-1){
        buffer[ind] = addr.substring(0, pos).toInt();
        addr = addr.substring(pos+1);
        pos = addr.indexOf('.');
        ind++;
    }
    buffer[ind] = addr.toInt();
    return true;
}

bool S3B::validateReceivedData(byte *s3bData, int len){
    if(len < 16){
        Serial.println("Invalid packet, too short");
        return false;
    }
    int c = 0;
    for(int i = 3; i < len -1; i++){
        c += s3bData[i];
    }
    byte checksum = 0xFF - (c & 0xFF);
    if(s3bData[len-1] == checksum){
        return true;
    }else{
        Serial.println("Invalid packet, checksum incorrect");
        return false;
    }
}

int S3B::getReceiveDataLength(byte *s3bData){
    int length = ((s3bData[1]*256)+s3bData[2])-12;
    return length;
}

int S3B::parseReceive(byte *s3bData, char *buffer, int len){
    int count = 0;
    for(int i = 15; i < len-1; i++){
        buffer[i-15] = (char)s3bData[i];
        count++;
    }
    return count;
}

