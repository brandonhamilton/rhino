#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>

#include "i2c.h"
#include "tc654.h"

#define FAN_CONTRLR_ADDR    0x1B    /* Fan speed controller */

char *duty_label[16] = {"30","34.67","39.33","44","48.67","53.33","58","62.67","67.33","72","76.67","81.33","86","90.67","95.33","100"};

void usage()
{
    printf("Usage: rhino_fan FAN_NUMBER [OPTIONS]\n");
}

int main(int argc, char **argv)
{
    int option;
    int fan, clear_flag, mode_flag, speed_flag, pulse_flag, fault_flag;
    unsigned char mode;
    unsigned char dutycycle;
    unsigned char pulses;
    unsigned short threshold;
    char *endptr;
    
    fan = 1;
    clear_flag = 0;
    mode_flag = 0;
    speed_flag = 0;
    pulse_flag = 0;
    fault_flag = 0;
    mode = 'a';
    dutycycle = 2;
    pulses = 2;
    threshold = 500;
    
    struct option opts[] = 
    {
        {"mode", required_argument, NULL, 'm'},
        {"speed", required_argument, NULL, 's'},
        {"clear", no_argument, NULL, 'c'},
        {"ppr", required_argument, NULL, 'p'},
        {"f", required_argument, NULL, 'f'},
        {0,0,0,0}
    };
    
    if(argc > 1)
    {
        fan = strtol(argv[1], &endptr, 10);
        if(endptr == argv[1])
        {
            usage();
            return 1;
        }
        if(!(fan == 1 || fan == 2))
        {
            printf("Error: FAN_NUMBER must be 1 or 2\n");
            return 1;
        }
    }
    else
    {
        usage();
        return 1;
    }
        
    while((option = getopt_long(argc, argv, "m:s:cp:f:", opts, NULL)) != -1)
    {
        switch(option)
        {
        case 'm':
            mode_flag = 1;
            mode = (unsigned char)optarg[0];
            if(mode != 'a' && mode != 'd')
            {
                printf("Error: MODE must be either 'd' or 'a'\n");
                return 1;
            }
            break;
        case 's':
            speed_flag = 1;
            dutycycle = (unsigned char)strtol(optarg, &endptr, 10);
            if(endptr == optarg || dutycycle > 15)
            {
                printf("Error: DUTYCYCLE must be in the range [0:15]\n");
                return 1;
            }
            break;
        case 'c':
            clear_flag = 1;
            break;
        case 'p':
            pulse_flag = 1;
            pulses = (unsigned char)strtol(optarg, &endptr, 10);
            if(endptr == optarg ||
               (pulses != 1 &&
                pulses != 2 &&
                pulses != 4 &&
                pulses != 8))
            {
                printf("Error: PULSES must be in the set (1,2,4,8)\n");
                return 1;
            }
            break;
        case 'f':
            fault_flag = 1;
            threshold = (unsigned short)strtol(optarg, &endptr, 10);
            if(endptr == optarg)
            {
                printf("Error: invalid value for THRESHOLD\n");
                return 1;
            }
            break;
        default:
            printf("Unknown option: %c\n",option);
        }
    }
    
    if(i2c_open("/dev/i2c-1"))
        return 1;
    
    if(mode_flag)
    {
        unsigned char config;
        char *cs;
        if(TC654readConfig(FAN_CONTRLR_ADDR, &config) == 0)
        {
            if(mode == 'd')
            {
                config = config | 0x20;
                cs = "digital";
            }
            else
            {
                config = config & 0xDF;
                cs = "analog";
            }
            
            if(TC654writeConfig(FAN_CONTRLR_ADDR, config) == 0)
            {
                printf("Fan PWM set to %s control\n", cs);
            }
            else
            {
                printf("Error setting fan PWM control\n");
                return 1;
            }
        }
    }
    
    if(speed_flag)
    {
        if(TC654writeDutyCycle(FAN_CONTRLR_ADDR, dutycycle) == 0)
        {
            printf("Fan duty cycle set to: %d = %s%%\n",dutycycle,duty_label[dutycycle]);
        }
        else
        {
            printf("Error setting duty cycle\n");
            return 1;
        }
    }
    
    if(pulse_flag)
    {
        unsigned char config;
        if(TC654readConfig(FAN_CONTRLR_ADDR, &config) == 0)
        {
            unsigned char bits;
            switch(pulses)
            {
            case 1:
                bits = 0;
                break;
            default:
            case 2:
                bits = 1;
                break;
            case 4:
                bits = 2;
                break;
            case 8:
                bits = 3;
                break;
            };
            if(fan == 1)
            {
                config = config & 0xF9;
                config = config | (bits << 1);
            }
            else
            {
                config = config & 0xE7;
                config = config | (bits << 3);
            }
            
            if(TC654writeConfig(FAN_CONTRLR_ADDR, config) == 0)
            {
                printf("Set fan %d PPR to %d\n",fan,pulses);
            }
            else
            {
                printf("Error setting fan %d PPR\n",fan);
                return 1;
            }
        }
    }
    
    if(fault_flag)
    {
        if(fan == 1)
        {
            if(TC654writeFault1(FAN_CONTRLR_ADDR, threshold) == 0)
            {
                printf("Set fan 1 fault threshold to %d\n",threshold);
            }
            else
            {
                printf("Error setting fan 1 fault threshold\n");
                return 1;
            }
        }
        else
        {
            if(TC654writeFault2(FAN_CONTRLR_ADDR, threshold) == 0)
            {
                printf("Set fan 2 fault threshold to %d\n",threshold);
            }
            else
            {
                printf("Error setting fan 2 fault threshold\n");
                return 1;
            }
        }
    }
    
    if(clear_flag)
    {
        unsigned char config;
        if(TC654readConfig(FAN_CONTRLR_ADDR, &config) == 0)
        {
            config = config | 0x80;
            
            if(TC654writeConfig(FAN_CONTRLR_ADDR, config) == 0)
            {
                config = config & 0x7F;
            
                if(TC654writeConfig(FAN_CONTRLR_ADDR, config) == 0)
                {
                    printf("Fan fault cleared\n");
                }
                else
                {
                    printf("Error clearing fan fault\n");
                    return 1;
                }
            }
            else
            {
                printf("Error clearing fan fault\n");
                return 1;
            }
        }
        else
        {
            printf("Error clearing fan fault\n");
            return 1;
        }
    }
    
    unsigned short rpm;
    unsigned char status;
    if(TC654readStatus(FAN_CONTRLR_ADDR, &status) == 0)
    {
        int fault1 = status & 0x01;
        int fault2 = status & 0x02;
        
        if(fan == 1)
        {
            if(TC654readRPM1(FAN_CONTRLR_ADDR, &rpm) == 0)
            {
                printf("Fan 1 RPM: %d",rpm);
                if(fault1)
                {
                    printf(" FAULT");
                }
                printf("\n");
            }
            else
            {
                printf("Error reading fan 1 RPM\n");
            }
        }
        else
        {
            if(TC654readRPM2(FAN_CONTRLR_ADDR, &rpm) == 0)
            {
                printf("Fan 2 RPM: %d",rpm);
                if(fault2)
                {
                    printf(" FAULT");
                }
                printf("\n");
            }
            else
            {
                printf("Error reading fan 2 RPM\n");
            }
        }
    }
    else
    {
        printf("Error reading fan status\n");
    }
    
    i2c_close();
    
    return 0;
}
