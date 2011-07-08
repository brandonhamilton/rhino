#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#include <string.h>

#define FPGA_VCCINT_EN "/sys/class/gpio/gpio99/direction"
#define FPGA_VCCO_AUX_EN "/sys/class/gpio/gpio100/direction"
#define FPGA_VCCMGT_EN "/sys/class/gpio/gpio101/direction"

#define FPGA_VCCINT_RD "/sys/class/gpio/gpio99/value"
#define FPGA_VCCO_AUX_RD "/sys/class/gpio/gpio100/value"
#define FPGA_VCCMGT_RD "/sys/class/gpio/gpio101/value"

int main(int argc, char **argv)
{
    int all_flag, int_flag, aux_flag, mgt_flag;
    char state[5];
 
    all_flag = 0;
    int_flag = 0;
    aux_flag = 0;
    mgt_flag = 0;
    
    struct option opts[] = 
    {
        {"all", no_argument, &all_flag, 1},
        {"vccint", no_argument, &int_flag, 1},
        {"vccaux", no_argument, &aux_flag, 1},
        {"vccmgt", no_argument, &mgt_flag, 1},
        {0,0,0,0}
    };
    
    while(getopt_long_only(argc, argv, "", opts, NULL) != -1);
            
    if(optind == 1)
    {
        all_flag = 1;
    }
    if(all_flag)
    {
        int_flag = 1;
        aux_flag = 1;
        mgt_flag = 1;
    }
    
    struct stat bstat;
    if(int_flag && (stat(FPGA_VCCINT_EN,&bstat) == -1))
    {
        printf("Error: kernel does not support power management GPIO access\n");
        return 1;
    }
    if(aux_flag && (stat(FPGA_VCCO_AUX_EN,&bstat) == -1))
    {
        printf("Error: kernel does not support power management GPIO access\n");
        return 1;
    }
    if(mgt_flag && (stat(FPGA_VCCMGT_EN,&bstat) == -1))
    {
        printf("Error: kernel does not support power management GPIO access\n");
        return 1;
    }
    
    int fint, faux, fmgt;
        
    if(optind < argc)
    {
        if(strcmp(argv[optind],"on") == 0)
        {
            strcpy(state, "high");
        }
        else if(strcmp(argv[optind], "off") == 0)
        {
            strcpy(state, "low");
        }
        else
        {
            printf("Error: power state must be 'on' or 'off'\n");
            return 1;
        }
        
        if(int_flag)
        {
            fint = open(FPGA_VCCINT_EN, O_WRONLY);
            if(fint != -1)
            {
                write(fint, state, strlen(state)+1);
                close(fint);
            }
        }
        if(aux_flag)
        {
            faux = open(FPGA_VCCO_AUX_EN, O_WRONLY);
            if(faux != -1)
            {
                write(faux, state, strlen(state)+1);
                close(faux);
            }
        }
        if(mgt_flag)
        {
            fmgt = open(FPGA_VCCMGT_EN, O_WRONLY);
            if(fmgt != -1)
            {
                write(fmgt, state, strlen(state)+1);
                close(fmgt);
            }
        }
    }
    
    if(int_flag)
    {
        fint = open(FPGA_VCCINT_RD, O_RDONLY);
        if(fint != -1)
        {
            read(fint, state, 1);
            if(state[0] == '0')
            {
                strcpy(state,"off");
            }
            else
            {
                strcpy(state,"on");
            }
            close(fint);
            printf("FPGA_VCCINT   : %s\n", state);
        }
    }
    if(aux_flag)
    {
        faux = open(FPGA_VCCO_AUX_RD, O_RDONLY);
        if(faux != -1)
        {
            read(faux, state, 1);
            if(state[0] == '0')
            {
                strcpy(state,"off");
            }
            else
            {
                strcpy(state,"on");
            }
            close(faux);
            printf("FPGA_VCCO_AUX : %s\n", state);
        }
    }
    if(mgt_flag)
    {
        fmgt = open(FPGA_VCCMGT_RD, O_RDONLY);
        if(fmgt != -1)
        {
            read(fmgt, state, 1);
            if(state[0] == '0')
            {
                strcpy(state,"off");
            }
            else
            {
                strcpy(state,"on");
            }
            close(fmgt);
            printf("FPGA_VCCMGT   : %s\n", state);
        }
    }
    
    return 0;
}
