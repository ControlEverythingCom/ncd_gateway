// This #include statement was automatically added by the Particle IDE.
#include "i2c_comm.h"

// This #include statement was automatically added by the Particle IDE.
#include "master_gateway.h"

String ncd_firmware = "gateway:v1.0";

void setup() {
    Wire.begin();
    Serial.begin(115200);
    Particle.function("deviceComm", gatewayCommand);
    Particle.variable("ncd_firmware", ncd_firmware);
}
