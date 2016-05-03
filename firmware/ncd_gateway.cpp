#include "master_gateway.h"
#include "S3B.h"

S3B sModule;

//i2c,32,writem,0,252|6,252
int gatewayCommand(String command){
    int s = command.indexOf(',');
    String type = command.substring(0, s);
    
    command = command.substring(s+1);
    s = command.indexOf(',');
    String address = command.substring(0, s);
    
    command = command.substring(s+1);
    s = command.indexOf(',');
    String func = command.substring(0, s);
    
    String args = command.substring(s+1);
    
    int response = 0;
    
    Serial.println(address);
    
    if(type.equals("i2c")){
        int addr = address.toInt();
        if(func.equals("read")){
            s = args.indexOf(',');
            int size = args.substring(0, s).toInt();
            args = args.substring(s+1);
            byte buff[size];
            readCommandI2C(addr, args, buff, size);
            
            Serial.print("Size: ");
            Serial.println(size);
            //maybe check if it's more than 4 bytes here and set an event since the client should already know to listen
            response = bytesToInt(buff, size);
        }
        else if(func.equals("write")){
            
            Serial.print(addr);
	        Serial.println(".");
	        
            Serial.print(args);
	        Serial.println(".");
	        
            response = writeCommandsI2C(addr, args);
        }
    }
    if(type.equals("s3b")){
        byte s3bAddress[8];
        sModule.parseAddress(address, s3bAddress);
        if(func.equals("read")){
            s = args.indexOf(',');
            int size = args.substring(0, s).toInt();
            args = args.substring(s+1);
            byte buff[size];
            // readCommandI2C(addr, args, buff, size);
            
            Serial.print("Size: ");
            Serial.println(size);
            //maybe check if it's more than 4 bytes here and set an event since the client should already know to listen
            response = bytesToInt(buff, size);
        }
        else if(func.equals("write")){
            Serial.println("S3B Write, here we go");
            // Serial.print(addr);
	        Serial.println(".");
	        
            Serial.print(args);
	        Serial.println(".");
	        
	        byte stringBytes[args.length()+1];
	        args.getBytes(stringBytes, args.length()+1);
	        
	        sModule.transmit(s3bAddress, stringBytes, args.length());
	        
        }
    }
    
    return response;
}

int bytesToInt(byte* b, int size){
    int val = 0;
    for(int i=3;i>-1;i--){ 
      val = val | b[i] << ((i)*8);
    }
    return val;
}
