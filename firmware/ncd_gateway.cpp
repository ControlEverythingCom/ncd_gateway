#include "master_gateway.h"

int byte[]::toInt();

//i2c,32,writem,0,252|6,252
int gatewayCommand(String command){
    int s = command.indexOf(',');
    String type = command.substring(0, s);
    
    command = command.substring(s+1);
    s = command.indexOf(',');
    int addr = command.substring(0, s).toInt();
    
    command = command.substring(s+1);
    s = command.indexOf(',');
    String func = command.substring(0, s);
    
    String args = command.substring(s+1);
    
    int response = 0;
    
    Serial.print(bytesToInt(btest, 6));
    
    if(type.equals("i2c")){
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
	        
            response = writeCommandI2C(addr, args);
        }
        else if(func.equals("writem")){
            
            Serial.print(addr);
	        Serial.println(".");
	        
            Serial.print(args);
	        Serial.println(".");
            
            response = writeCommandsI2C(addr, args);
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
