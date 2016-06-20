// This #include statement was automatically added by the Particle IDE.
#include "ncd_gateway/ncd_gateway.h"

int testRelay(String arg)
//S3B module;

byte receiveBuffer[256];


// Data packet structure:
// [0] - Header: [188: i2c_read/i2c_write, 189: i2c_write_mask, 2: s3b]

//i2c_read - i2c_write
// [1] - Address: i2c chip address
// [2] - Write length: number of subsequent bytes to be written
// [3] - Bytes to be read back
// [4] - (val(2)+2) - Bytes to be written

//i2c_write_mask
// [1] - Address: i2c chip address
// [2] - Mask Op: 0=&, 1=|, 2=^, 3=<<, 4=>>
// [3] - Masked offsets: Read/Write offsets, first 4 bits as int determine offset for write, second 4 as int for read. 
// [4] - Read/write length: number of bytes for writing the read command
// [5] - Bytes to read back
// [6 - (val(4)+4)] - Bytes to be written to get initial values
// [(val(4)+4)+2] - Bytes to be read back for initial value
// [(val(4)+4)+3] - Write length
// [(val(4)+4)+4 - (val([val(3)+3)+2])+((val(3)+3)+2)] - Bytes to be written

//need to add mask offset for write bytes (to ignore register byte
//Add byte after head for port

//set only relay 1 to on
// i2c_write_mask:  i2c address:    Mask op:    Masked Offsets: Read command length:    Read command bytes: Read bytes: Write command length:   Write command bytes:    Bytes to read after write
// 1:               32:             0:          16:             1:                      10:                 1:          2:                      10:1:                   0

void setup() {
    init_gateway();
    Particle.function("relayTest", testRelay);
}


int testRelay(String arg){
    if(arg.equals("init")){
        byte packet[20] = {0xBB, 0x02, 0x06, 0xBC, 0x20, 0x02, 0x00, 0x00, 0xF0, 0x06, 0xBC, 0x20, 0x02, 0x00, 0x06, 0xF0};
        //byte packet[8] = {0xBC, 0x20, 0x04, 0x00, 0x00, 0xF0};
        ncdApi(packet);
        return 1;
    }
    if(arg.equals("on")){
        byte packet[6] = {0xBC, 0x20, 0x02, 0x00, 0x0a, 0x01};
        ncdApi(packet);
        return 2;
    }
    if(arg.equals("off")){
        byte packet[6] = {0xBC, 0x20, 0x02, 0x00, 0x0a, 0x00};
        ncdApi(packet);
        return 3;
    }
    if(arg.equals("toggle")){
        byte packet[10] = {0xBD, 0x20, 0x02, 0x01, 0x01, 0x01, 0x0a, 0x02, 0x0a, 0xFF};
        ncdApi(packet);
        return 4;
    }
}

void loop() {
    //Receive S3B data
    // if(Serial1.available()!=0){
    //     Serial.println("Got a command");
    //     byte startDelimiter = Serial1.read();
    //     if(startDelimiter == 0x7E){
    //         unsigned long startTime = millis();
    //         while(Serial1.available() < 2 && millis() <= tOut+startTime);
    //         if(Serial1.available() < 2){
    //             Serial.println("Timeout");
    //             return;
    //         }
    //         byte lenMSB = Serial1.read();
    //         byte lenLSB = Serial1.read();
    //         int newDataLength = (lenMSB*256)+lenLSB;
            
    //         int count = 0;
    //         while(count != newDataLength+1 && millis() <= tOut+startTime){
    //             if(Serial1.available() != 0){
    //                 receiveBuffer[count] = Serial1.read();
    //                 count++;
    //             }
    //         }
    //         if(count < newDataLength+1){
    //             Serial.println("Timeout2");
    //             Serial.printf("Received Bytes: %i, expected %i \n", count, newDataLength+1);
    //             return;
    //         }
    //         Serial.printf("Received %i bytes \n", count);
    //         //We have all our data.
    //         byte newData[newDataLength+4];
    //         newData[0] = startDelimiter;
    //         newData[1] = lenMSB;
    //         newData[2] = lenLSB;
    //         for(int i = 3; i < newDataLength+4; i++){
    //             newData[i] = receiveBuffer[i-3];
    //         }
    //         Serial.print("Received: ");
    //         for(int i = 0; i < sizeof(newData); i++){
    //             Serial.printf("%x, ", newData[i]);
    //         }
    //         Serial.println();
    //         //validate data
    //         if(!module.validateReceivedData(newData, newDataLength+4)){
    //             Serial.println("Invalid Data");
    //             return;
    //         }
    //         //get length of new data
    //         int receivedDataLength = module.getReceiveDataLength(newData);
    //         char receivedData[receivedDataLength];
    //         int validDataCount = module.parseReceive(newData, receivedData, newDataLength+4);
    //         if(validDataCount == receivedDataLength){

    //         }
    //         String stringCommand = String(receivedData);
    //         Serial.println("Command: "+stringCommand);
    //         gatewayCommand(stringCommand);
    //     }else{
    //         Serial.printf("First byte not valid, it was: 0x%x \n", startDelimiter);
    //     }
    // }
}

