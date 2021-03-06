#include "spark_wiring.h"
#include "spark_wiring_constants.h"
#include "spark_wiring_usbserial.h"
#include "spark_wiring_usartserial.h"
#include "spark_wiring_i2c.h"

void commandHandler(const char *event, const char *data);

int gatewayCommand(String command);

int ncdApi(byte packetBytes[]);
int ncdApi(byte packetBytes[], bool ka);
void i2c_command(byte bytes[], byte *buff);
int writeCommandsI2C(int addr, int* commands, int commandsLen);
void array_slice(byte bytes[], int start, int len, byte *buff);
void array_slice(byte bytes[], int start, int len, int *buff);
int mask(int val, int mask, int type);

void init_gateway();
void loop_gateway();
int hexToInt(String arg, byte bytes[], int length);
int base64ToInt(String arg, byte buff[], int length);
int bytesToInt(byte bytes[], int length);
