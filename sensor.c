#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>

#define CS_MCP3208  6        // BCM_GPIO 25
#define SPI_CHANNEL_1 0
#define SPI_CHANNEL_2 1
#define SPI_SPEED   1000000 


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
	SENSOR_VALUE sensor_value;

	if(sensor_flag == 0)
	{
		if(wiringPiSetup() == -1)
	  	{
	    	fprintf (stdout, "Unable to start wiringPi: %s\n", strerror(errno));
	  	}
	 
	 	if(wiringPiSPISetup(SPI_CHANNEL_1, SPI_SPEED) == -1)
	 	{
	    	fprintf (stdout, "wiringPiSPISetup Failed: %s\n", strerror(errno));
	  	}
	}
	
  	pinMode(CS_MCP3208, OUTPUT);

	sensor_value.flameValue = read_mcp3208_adc(0);

	std::cout<< "flameValue : "<<sensor_value.flameValue<<std::endl;
		
	sensor_value.gasValue=0;
	sensor_value.check_flag=0;
	
	return 0;

}
