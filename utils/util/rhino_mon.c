#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <string.h>

#include "i2c.h"
#include "ina219.h"
#include "max1668.h"

#define NCHANNELS 12

#define TEMP_MON_ADDR       0x18    /* Temperature monitor */
#define MON_12V_ADDR        0x40    /* 12V supply voltage */
#define MON_1V2_FPGA_ADDR   0x41    /* 1.2V FPGA voltage */
#define MON_1V5_FPGA_ADDR   0x42    /* 1.5V FPGA voltage */
#define MON_2V5_FPGA_ADDR   0x43    /* 2.5V FPGA voltage */
#define MON_3V3_FPGA_ADDR   0x44    /* 3.3V FPGA voltage */
#define MON_2V5_FMC_ADDR    0x45    /* 2.5V FMC voltage */
#define MON_3V3_FMC_ADDR    0x46    /* 3.3V FMC voltage */
#define MON_12V_FMC_ADDR    0x47    /* 12V FMC voltage */
#define MON_5V_PROC_ADDR    0x49    /* 5V Processor voltage */

int main(int argc, char **argv)
{
    int voltage_flag, current_flag, power_flag, daemon, delay;
    int option;
    char *endptr;
    char logfile[100];
    
    int channel[NCHANNELS] = {0,0,0,0,0,0,0,0,0,0,0,0};
    char *channel_name[NCHANNELS] = {"PROCT","FPGAT","AMBT","12V","PROC5V","FPGA1V2","FPGA1V5","FPGA2V5","FPGA3V3","FMC2V5","FMC3V3","FMC12V"};
    char *channel_label[NCHANNELS] = {"PROC","FPGA","AMB","12V","PROC_5V","FPGA_1.2V","FPGA_1.5V","FPGA_2.5V","FPGA_3.3V","FMC_2.5V","FMC_3.3V","FMC_12V"};
    unsigned char channel_reg[NCHANNELS] = {0,0,0,MON_12V_ADDR,MON_5V_PROC_ADDR,MON_1V2_FPGA_ADDR,MON_1V5_FPGA_ADDR,MON_2V5_FPGA_ADDR,MON_3V3_FPGA_ADDR,MON_2V5_FMC_ADDR,MON_3V3_FMC_ADDR,MON_12V_FMC_ADDR};
    
    voltage_flag = 0;
    current_flag = 0;
    power_flag = 0;
    daemon = 0;
    delay = 1;
    
    struct option opts[] = 
    {
        {"voltage", no_argument, NULL, 'v'},
        {"current", no_argument, NULL, 'c'},
        {"power", no_argument, NULL, 'p'},
        {"daemon", no_argument, NULL, 'd'},
        {"time", required_argument, NULL, 't'},
        {"log", required_argument, NULL, 'l'},
        {0,0,0,0}
    };
    
    strcpy(logfile, "/var/log/rhino_mon.log");
    
    while((option = getopt_long(argc, argv, "vcpdt:l:", opts, NULL)) != -1)
    {
        switch(option)
        {
        case 'v':
            voltage_flag = 1;
            break;
        case 'c':
            current_flag = 1;
            break;
        case 'p':
            power_flag = 1;
            break;
        case 'd':
            daemon = 1;
            break;
        case 't':
            delay = strtol(optarg, &endptr, 10);
            if(endptr == optarg || delay == 0)
            {
                delay = 1;
            }
            break;
        case 'l':
            strncpy(logfile, optarg, 100);
            break;
        default:
            printf("Unknown option: %c\n",option);
        }
    }
    
    if(voltage_flag == 0 && current_flag == 0 && power_flag == 0)
    {
        voltage_flag = 1;
    }
    
    int i;
    int nchannels = 0;
    
    for(; optind < argc; optind++)
    {
        for(i = 0; i < NCHANNELS; i++)
        {
            if(strcmp(argv[optind], channel_name[i]) == 0)
            {
                channel[i] = 1;
                nchannels++;
            }
        }
    }
    
    if(nchannels == 0)
    {
        for(i = 0; i < NCHANNELS; i++)
        {
            channel[i] = 1;
        }
        nchannels = NCHANNELS;
    }
    
    FILE *ofid = stdout;
    
    if(daemon)
    {
        pid_t pid, sid;
        
        pid = fork();
        
        if(pid < 0)
        {
            return 1;
        }
        if(pid > 0)
        {
            return 0;
        }
        
        umask(0);
        
        sid = setsid();
        if(sid < 0)
        {
            return 1;
        }
        
        if(chdir("/") < 0)
        {
            return 1;
        }
        
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        
        ofid = fopen(logfile,"w");
        
        if(!ofid)
        {
            return 1;
        }
    }
    
    if(i2c_open("/dev/i2c-1"))
        return 1;
        
    for(i = 3; i < NCHANNELS; i++)
    {
        if(channel[i])
        {
            INA219calibrate(channel_reg[i]);
        }
    }
    
    fprintf(ofid,"|");
    for(i = 0; i < NCHANNELS; i++)
    {
        if(channel[i])
        {
            fprintf(ofid,"%10s |",channel_label[i]);
        }
    }
    fprintf(ofid,"\n+");
    for(i = 0; i < nchannels; i++)
    {
        fprintf(ofid,"-----------+");
    }
    fprintf(ofid,"\n");
    
    char leader_val[40];
    strcpy(leader_val,"|");
    if(channel[0])
    {
        strcat(leader_val, "           |");
    }
    if(channel[1])
    {
        strcat(leader_val, "           |");
    }
    if(channel[2])
    {
        strcat(leader_val, "           |");
    }
    
    while(1)
    {
        char temp;
        unsigned short millivolts;
        short microamps;
        int microwatts;
        char leader[40];
        
        strcpy(leader, "");
        
        fprintf(ofid,"|");
        
        if(channel[0])
        {
            MAX1668readDX1Temp(TEMP_MON_ADDR, &temp);
            fprintf(ofid,"%9dC |",temp);
        }
        if(channel[1])
        {
            MAX1668readDX2Temp(TEMP_MON_ADDR, &temp);
            fprintf(ofid,"%9dC |",temp);
        }
        if(channel[2])
        {
            MAX1668readDX3Temp(TEMP_MON_ADDR, &temp);
            fprintf(ofid,"%9dC |",temp);
        }
        
        if(voltage_flag)
        {
            for(i = 3; i < NCHANNELS; i++)
            {
                if(channel[i])
                {
                    INA219readVoltage(channel_reg[i], &millivolts);
                    fprintf(ofid,"%4d.%03d V |",millivolts/1000,millivolts%1000);
                }
            }
            fprintf(ofid,"\n");
            strcpy(leader,leader_val);
        }
            
        if(current_flag)
        {
            fprintf(ofid,"%s",leader);
            for(i = 3; i < NCHANNELS; i++)
            {
                if(channel[i])
                {
                    INA219readCurrent(channel_reg[i], &microamps);
                    fprintf(ofid,"%4d.%01d  mA |",microamps/1000,microamps%1000/100);
                }
            }
            fprintf(ofid,"\n");
            strcpy(leader,leader_val);
        }
        
        if(power_flag)
        {
            fprintf(ofid,"%s",leader);
            for(i = 3; i < NCHANNELS; i++)
            {
                if(channel[i])
                {
                    INA219readPower(channel_reg[i], &microwatts);
                    fprintf(ofid,"%4d.%03dmW |",microwatts/1000,microwatts%1000);
                }
            }
            fprintf(ofid,"\n");
        }
        
        fprintf(ofid,"+");
        for(i = 0; i < nchannels; i++)
        {
            fprintf(ofid,"-----------+");
        }
        fprintf(ofid,"\n");
        fflush(ofid);
        
        sleep(delay);
    }
    
    return 0;
}
