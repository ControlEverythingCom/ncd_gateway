#include "spark_wiring.h"
#include "spark_wiring_constants.h"
#include "spark_wiring_usbserial.h"
#include "spark_wiring_i2c.h"

int processCommandI2C(int addr, String command);
int writeCommandI2C(int addr, String command);
int writeCommandsI2C(int addr, String command);
int writeCommandsI2C(int addr, int* commands, int commandsLen);
int readCommandI2C(int addr, String command, byte *buf, int size);
int clearInitCommands();
