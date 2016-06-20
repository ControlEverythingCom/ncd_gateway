#include "i2c_comm.h"
#include "ncd_gateway.h"

String ncd_firmware = "gateway:v1.0";

void setup() {
    init_gateway();
}
