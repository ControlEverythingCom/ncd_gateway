

#include "spark_wiring.h"
#include "spark_wiring_constants.h"
#include "spark_wiring_usbserial.h"
#include "i2c_comm.h"

void commandHandler(const char *event, const char *data);

int gatewayCommand(String command);

int ncdApi(byte packetBytes[]);
void i2c_command(byte bytes[], byte *buff);

void array_slice(byte bytes[], int start, int len, byte *buff);
void array_slice(byte bytes[], int start, int len, int *buff);

void init_gateway();
