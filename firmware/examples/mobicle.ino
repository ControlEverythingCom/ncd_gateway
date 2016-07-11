// This #include statement was automatically added by the Particle IDE.
#include "ncd_gateway/ncd_gateway.h"

// Data packet structure:
// [0] - Header: [186: Bus scan, 187: packet of packets, 188: i2c_read/i2c_write, 189: i2c_write_mask, 190: delay]

//Bus Scan (186)
// [1] - chunk: 0=1-32, 1=33-64, 2=65-96, 3=97-127
//-----------

//Packet of packets (187)
// [1] 1st packet length
// [2-[[1]+1]] 1st packet
//-----------

//i2c_read - i2c_write (188)
// [1] - Address: i2c chip address
// [2] - Write length: number of subsequent bytes to be written
// [3] - Bytes to be read back
// [4] - (val(2)+2) - Bytes to be written
//-----------

//i2c_write_mask (189)
// [1] - Address: i2c chip address
// [2] - Mask Op: 0 - |=, 1 - ^=, 2 - &= ~, 3 - &=, 4 - <<, 5 - >>
// [3] - Masked offsets: Read/Write offsets, first 4 bits as int determine offset for write, second 4 as int for read. 
// [4] - Read/write length: number of bytes for writing the read command
// [5] - Bytes to read back
// [6 - (val(4)+4)] - Bytes to be written to get initial values
// [(val(4)+4)+2] - Bytes to be read back for initial value
// [(val(4)+4)+3] - Write length
// [(val(4)+4)+4 - (val([val(3)+3)+2])+((val(3)+3)+2)] - Bytes to be written

//set only relay 1 to on
// i2c_write_mask:  i2c address:    Mask op:    Masked Offsets: Read command length:    Read command bytes: Read bytes: Write command length:   Write command bytes:    Bytes to read after write
// 1:               32:             0:          16:             1:                      10:                 1:          2:                      10:1:                   0
//------------

//Delay (190)
// [1] MSB
// [2] LSB
// ([1] << 8)+[2] = Delay in millis

//DO NOT REMOVE THIS LINE - DEFINITIONS

void setup() {
    init_gateway();
    //DO NOT REMOVE THIS LINE - SETUP
}

void loop() {
    //DO NOT REMOVE THIS LINE - LOOP
}

//DO NOT REMOVE THIS LINE - FUNCTIONS
