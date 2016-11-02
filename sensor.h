#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>

#define CS_MCP3208 5
#define TH_SPI_CHANNEL 0
#define GAS_SPI_CHANNEL 1
#define SPI_SPEED 1000000


#define MAXTIMINGS	85
#define DHTPIN		7
int th_data[5] = {0, 0, 0, 0, 0};

typedef struct sensorValue
{
	int flameValue;
	int gasValue;
	int th_data[5];
}SENSOR_VALUE;

void checkGasState(int *gasTracker)
{
    int i=0;
    if(gasTracker[0]!=-200)
    {
        int flag=0;
        for(i=0;i<=4;i++)
        {
            if (gasTracker[i]>=300 && flag < 5)flag++;
            std::cout<<"------------------------------------------------"<<std::endl;
            std::cout<<"gasTracker value:"<<gasTracker[i]<<std::endl;
            std::cout<<"gasTracker flag  value:"<<flag<<std::endl;
            std::cout<<"------------------------------------------------"<<std::endl;
        }

        if(flag>2&&flag<4) sensorDev.gas_state=1;
        else if(flag==5) sensorDev.gas_state=2;
        else  if(flag<1)sensorDev.gas_state=0;
    }
    for(i=0;i<4;i++)
    {
        gasTracker[i]=gasTracker[i+1];
    }
}

void checkFlameState(int *flameTracker)
{
    int i;
    if(flameTracker[0]!=-200)
    {   int flag=0;
        for(i=0;i<=4;i++)
        {
            if (flameTracker[i]<700 && flag < 5 )flag++;
        }

        if(flag>2&&flag<4) sensorDev.flame_state=1;
        else if(flag==5) sensorDev.flame_state=2;
        else if(flag<1) sensorDev.flame_state=0;
    }

    for(i=0;i<4;i++)
    {
        flameTracker[i]=flameTracker[i+1];
    }
}

void checkHumiState(int *humTracker)
{
    if(humTracker[5] != -1000)
    {
        if(humTracker[0]==-200)
        {
            int i=0;
            for(;i<=4;i++)
            {
                humTracker[i]=humTracker[i+1];

            }
        }
        else
        {
            int avr=0,i;
            for(i=0;i<=4;i++)
            {
                if(avr-humTracker[i]>10 || humTracker[i]-avr > 10)
                    avr+=avr;
                else
                    avr+=humTracker[i];
            }
            avr=avr/5;

            if(avr-humTracker[5]>=4)
            {
                if(humTracker[6]==6)
                {
                    sensorDev.hum_state=2;
                }
                else if (humTracker[6]>2 && humTracker[6]<5)
                {
                    sensorDev.hum_state=1;
                    humTracker[6]++;
                }
                else
                {
                    humTracker[6]++;
                }
            }
            else if(avr-humTracker[5]<4)
            {
                if(humTracker[6]==0)
                {
                    sensorDev.hum_state=0;
                }
                else if (humTracker[6]>2 && humTracker[6]<5)
                {
                    sensorDev.hum_state=1;
                    humTracker[6]--;
                }
                else
                {
                    humTracker[6]--;
                }

                for(i=0;i<3;i++)
                {
                    humTracker[i] = humTracker[i+1];
                }

            }
        }
    }
}

void checkTempState(int *tempTracker)
{

    if(tempTracker[5] != -1000)
    {
        if(tempTracker[0]==-200)
        {
            int i=0;
            for(;i<=4;i++)
            {
                tempTracker[i]=tempTracker[i+1];
            }
        }
        else
        {
            int avr=0,i;
            for(i=0;i<=4;i++)
            {
                avr+=tempTracker[i];
            }
            avr=avr/5;

            if(tempTracker[5]-avr>=2)
            {
                if(tempTracker[6]==6)
                {
                    sensorDev.temp_state=2;
                }
                else if (tempTracker[6]>2 && tempTracker[6]<5)
                {
                    sensorDev.temp_state=1;
                    tempTracker[6]++;
                }
                else
                {
                    tempTracker[6]++;
                }
            }
            else if(tempTracker[5]-avr<2)
            {
                if(tempTracker[6]==0)
                {
                    sensorDev.temp_state=0;
                }
                else if (tempTracker[6]>2 && tempTracker[6]<5)
                {
                    sensorDev.temp_state=1;
                    tempTracker[6]--;
                }
                else
                {
                    tempTracker[6]--;
                }

                for(i=0;i<3;i++)
                {
                    tempTracker[i] =  tempTracker[i+1];
                }
            }
        }
        std::cout<<"temp state : "<<sensorDev.temp_state<<std::endl;
    }
}


void read_th_data() 
{
	uint8_t laststate = HIGH;
	uint8_t counter = 0;
	uint8_t j = 0;
	uint8_t i;
	float c;

	th_data[0] = th_data[1] = th_data[2] = th_data[3] = th_data[4] = 0;

	pinMode(DHTPIN, OUTPUT);
	digitalWrite(DHTPIN, LOW);
	delay(18);

	digitalWrite(DHTPIN, HIGH);
	delayMicroseconds(40);

	pinMode(DHTPIN, INPUT);

	for(i = 0; i < MAXTIMINGS; i++) {
		counter = 0;
		while(digitalRead(DHTPIN) == laststate) {
			counter++;
			delayMicroseconds(1);
			if(counter == 255) {
				break;
			}
		}
		laststate = digitalRead(DHTPIN);

		if(counter == 255) {
			break;
		}

		if((i >= 4) && (i % 2 == 0)) {
			th_data[j / 8] <<= 1;
			if(counter > 16) {
				th_data[j / 8] |= 1;
			}
			j++;
		}
	}

	if((j >= 40) && (th_data[4] == ((th_data[0] + th_data[1] + th_data[2] + th_data[3]) & 0xFF))) {
		c = th_data[2] * 9. / 5. + 32;
		printf("Humidity = %d.%d %% Temperature = %d.%d *C (%.1f *F)\n", th_data[0], th_data[1], th_data[2], th_data[3], c);
	}
	else {
		printf("Data not good, skip\n");
	}
}//th



int read_mcp3208_adc(unsigned char adcChannel) 
{
	unsigned char buff[3];
	int adcValue = 0;
	buff[0] = 0x06 | ((adcChannel & 0x07) >> 2);
	buff[1] = ((adcChannel & 0x07) << 6);
	buff[2] = 0x00;
	digitalWrite(CS_MCP3208, 0);
	wiringPiSPIDataRW(SPI_CHANNEL, buff, 3);
	buff[1] = 0x0F & buff[1];
	adcValue = (buff[1] << 8) | buff[2];
	digitalWrite(CS_MCP3208, 1);

	return adcValue;
}//flame


SENSOR_VALUE readSensor(void)
{

	int flameValue = 0;
	int gasValue=0;

	if(wiringPiSetup() == -1) return 1;

	if(wiringPiSPISetup(TH_SPI_CHANNEL, SPI_SPEED) == -1) return 2;
	pinMode(CS_MCP3208, OUTPUT);
	flameValue = read_mcp3208_adc(0);
	printf("adc0 Value = %u\n", flameValue);
	delay(100);

	if(wiringPiSPISetup(GAS_SPI_CHANNEL, GAS_SPI_SPEED) == -1) return 3;
	gasValue = read_mcp3208_adc(1);
	printf("adc0 Value = %u\n", gasValue);
	delay(100);

	read_th_data();
	
	SENSOR_VALUE.flameValue = flameValue;
	SENSOR_VALUE.gasValue = gasValue;
	SENSOR_VALUE.th_data = th_data;

	return 0;

}
