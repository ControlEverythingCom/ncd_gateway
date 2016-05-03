
#include "spark_wiring.h"
#include "spark_wiring_constants.h"
#include "spark_wiring_usbserial.h"
#include "i2c_comm.h"

int bytesToInt(byte* b, int size);
int gatewayCommand(String command);
