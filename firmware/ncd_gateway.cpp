#include "ncd_gateway.h"
#include "S3B.h"

S3B sModule;
String eventReturns[5];

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
        if(func.equals("read")){
            s = args.indexOf(',');
            int size = args.substring(0, s).toInt();
            args = args.substring(s+1);
            byte buff[size];
            int addr = address.toInt();
            readCommandI2C(addr, args, buff, size);
            Serial.println(size);
            //maybe check if it's more than 4 bytes here and set an event since the client should already know to listen
            
            if(size>3){
                byte rebytes[4] = {0xff,0xff,0xff,0x00};
                String byteString = "";
                int i;
                for(i=0;i<size;i++){
                    if(!byteString.equals("")){
                        byteString = byteString + ",";
                    }
                    byteString = byteString + String(buff[i]);
                }
                rebytes[3] = setEventReturn(byteString);
                response = bytesToInt(rebytes, 4);
            }else{
                response = bytesToInt(buff, size);
            }
        }
        else if(func.equals("write")){
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
