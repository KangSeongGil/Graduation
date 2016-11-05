#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>

#define CS_MCP3208  6        // BCM_GPIO 25
#define SPI_CHANNEL_1 0
#define SPI_CHANNEL_2 1
#define SPI_SPEED   1000000 

const int LEDPIN=4;
#define MAXTIMINGS	85
#define DHTPIN		7
int th_data[5] = {0, 0, 0, 0, 0};

typedef struct sensorValue
{
	int check_flag;
	int flameValue;
	int gasValue;
}SENSOR_VALUE;



int read_mcp3208_adc(unsigned char adcChannel)
{
  unsigned char buff[3];
  int adcValue = 0;

  buff[0] = 0x06 | ((adcChannel & 0x07) >> 7);
  buff[1] = ((adcChannel & 0x07) << 6);
  buff[2] = 0x00;

  digitalWrite(CS_MCP3208, 0);  // Low : CS Active

  wiringPiSPIDataRW(SPI_CHANNEL_1, buff, 3);

  buff[1] = 0x0F & buff[1];
  adcValue = ( buff[1] << 8) | buff[2];

  digitalWrite(CS_MCP3208, 1);  // High : CS Inactive

  return adcValue;
}

int main(void)
{
	if(1)
	{
		if (wiringPiSetup () == -1)
			fprintf (stdout, "wiringPiSPISetup Failed: \n");
		printf("wiringPi setup end\n");

		if(wiringPiSPISetup(SPI_CHANNEL_1, SPI_SPEED) == -1)
			fprintf (stdout, "wiringPiSPISetup Failed\n");
		printf("wiringPiSPISetup\n");
	}

	pinMode(LEDPIN, OUTPUT);
	digitalWrite(LEDPIN, HIGH);
	return 0;
}
