#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>

#define CS_MCP3208  5       // BCM_GPIO 25
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

void read_th_data() {
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
        th_data[0]=-1000;
        th_data[2]=-1000;
        printf("Data not good, skip\n");
    }
}

SENSOR_VALUE readSensor(int sensor_flag)
{
    SENSOR_VALUE sensor_value;

    if(sensor_flag == 0)
    {
        if(wiringPiSPISetup(SPI_CHANNEL_1, SPI_SPEED) == -1)
            fprintf (stdout, "wiringPiSPISetup Failed: %s\n", strerror(errno));
        std::cout<<"wiringPiSPISetup"<<std::endl;
    }

    pinMode(CS_MCP3208, OUTPUT);

    sensor_value.flameValue = read_mcp3208_adc(0);
    //delay(500);
    sensor_value.gasValue = read_mcp3208_adc(1);

    std::cout<< "flameValue : "<<sensor_value.flameValue<<std::endl;
    std::cout<< "gasValue : "<<sensor_value.gasValue<<std::endl;
    read_th_data();


    sensor_value.check_flag=0;

    return sensor_value;

}
