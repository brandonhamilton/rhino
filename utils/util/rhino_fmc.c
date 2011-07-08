#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>

#include "i2c.h"

#define GPIOEX "/sys/class/gpio/export"

#define FPGA_VCCO_AUX_EN "/sys/class/gpio/gpio100/direction"
#define FPGA_VCCO_AUX_RD "/sys/class/gpio/gpio100/value"

#define FMC0_SUPPLY_EN  "/sys/class/gpio/gpio102/direction"
#define FMC0_SUPPLY_RD  "/sys/class/gpio/gpio102/value"
#define FMC0_AUX_EN     "/sys/class/gpio/gpio103/direction"
#define FMC0_AUX_RD     "/sys/class/gpio/gpio103/value"
#define FMC1_SUPPLY_EN  "/sys/class/gpio/gpio104/direction"
#define FMC1_SUPPLY_RD  "/sys/class/gpio/gpio104/value"
#define FMC1_AUX_EN     "/sys/class/gpio/gpio105/direction"
#define FMC1_AUX_RD     "/sys/class/gpio/gpio105/value"

#define FMC0_GA0        "/sys/class/gpio/gpio140/direction"
#define FMC0_GA1        "/sys/class/gpio/gpio141/direction"
#define FMC0_nPRSNT_M2C "/sys/class/gpio/gpio142/value"

#define FMC1_GA0        "/sys/class/gpio/gpio155/direction"
#define FMC1_GA1        "/sys/class/gpio/gpio152/direction"
#define FMC1_nPRSNT_M2C "/sys/class/gpio/gpio153/value"

#define FMC_PG_C2M      "/sys/class/gpio/gpio143/direction"

int export_gpio(const char *path)
{
    struct stat bstat;
    int fex;
    if(stat(path,&bstat) == -1)
    {
        char gpio[4];
        strncpy(gpio,&path[20],3);
        gpio[3] = '\0';
        /* export gpio */
        fex = open(GPIOEX, O_WRONLY);
        if(fex != -1)
        {
            write(fex, gpio, 4);
            close(fex);
        }
        else
        {
            return 1;
        }
    }
    
    return 0;
}

int fmc_power_on(int fmc)
{
    char *aux;
    char *supply;
    int fid;
    
    if(fmc == 0)
    {
        aux = FMC0_AUX_EN;
        supply = FMC0_SUPPLY_EN;
    }
    else
    {
        aux = FMC1_AUX_EN;
        supply = FMC1_SUPPLY_EN;
    }
    
    fid = open(FPGA_VCCO_AUX_EN, O_RDWR);
    if(fid != -1)
    {
        write(fid, "high", 5);
        close(fid);
    }
    else
    {
        return 1;
    }
    
    if(export_gpio(FMC_PG_C2M))
    {
        return 1;
    }
    fid = open(FMC_PG_C2M, O_RDWR);
    if(fid == -1)
    {
        return 1;
    }
    write(fid, "high", 5);
    close(fid);
    
    if(export_gpio(aux))
    {
        return 1;
    }
    fid = open(aux, O_RDWR);
    if(fid == -1)
    {
        return 1;
    }
    write(fid, "high", 5);
    close(fid);
    
    if(export_gpio(supply))
    {
        return 1;
    }
    fid = open(supply, O_RDWR);
    if(fid == -1)
    {
        return 1;
    }
    write(fid, "high", 5);
    close(fid);
    
    return 0;
}

int fmc_power_off(int fmc)
{
    char *aux;
    char *supply;
    int fid;
    
    if(fmc == 0)
    {
        aux = FMC0_AUX_EN;
        supply = FMC0_SUPPLY_EN;
    }
    else
    {
        aux = FMC1_AUX_EN;
        supply = FMC1_SUPPLY_EN;
    }
    
    if(export_gpio(supply))
    {
        return 1;
    }
    fid = open(supply, O_RDWR);
    if(fid == -1)
    {
        return 1;
    }
    write(fid, "low", 4);
    close(fid);
    
    if(export_gpio(FMC_PG_C2M))
    {
        return 1;
    }
    fid = open(FMC_PG_C2M, O_RDWR);
    if(fid == -1)
    {
        return 1;
    }
    write(fid, "low", 4);
    close(fid);
    
    if(export_gpio(aux))
    {
        return 1;
    }
    fid = open(aux, O_RDWR);
    if(fid == -1)
    {
        return 1;
    }
    write(fid, "low", 4);
    close(fid);
    
    return 0;
}

void usage()
{
    printf("Usage: rhino_fmc FMCID [OPTIONS]\n");
}

void command_usage()
{
    printf("Usage: rhino_fmc FMCID -c [r|w] I2C_DEVICE REGISTER [VALUE]\n");
}

int main(int argc, char **argv)
{
    int enable_flag, disable_flag, address_flag, command_flag, write_flag;
    int GA;
    int option;
    int fmc;
    char *endptr;
    
    enable_flag = 0;
    disable_flag = 0;
    address_flag = 0;
    command_flag = 0;
    write_flag = 0;
    GA = 0;
    
    struct option opts[] = 
    {
        {"enable", no_argument, NULL, 'e'},
        {"disable", no_argument, NULL, 'd'},
        {"address", required_argument, NULL, 'a'},
        {0,0,0,0}
    };
    
    if(argc > 1)
    {
        fmc = strtol(argv[1], &endptr, 10);
        if(endptr == argv[1])
        {
            usage();
            return 1;
        }
        if(!(fmc == 0 || fmc == 1))
        {
            printf("Error: FMCID must be 0 or 1\n");
            return 1;
        }
    }
    else
    {
        usage();
        return 1;
    }
    
    while((option = getopt_long(argc, argv, "eda:c:", opts, NULL)) != -1)
    {
        switch(option)
        {
        case 'e':
            enable_flag = 1;
            break;
        case 'd':
            disable_flag = 1;
            break;
        case 'a':
            address_flag = 1;
            GA = strtol(optarg, &endptr, 0);
            if(endptr == optarg || GA > 3 || GA < 0)
            {
                printf("Error: invalid value for GA\n");
                return 1;
            }
            break;
        case 'c':
            command_flag = 1;
            if(optarg[0] == 'r')
            {
                write_flag = 0;
            }
            else if(optarg[0] == 'w')
            {
                write_flag = 1;
            }
            else
            {
                printf("Error: -c option requires 'r' or 'w' as an argument\n");
                return 1;
            }
            
            break;
        case '?':
            return 1;
        default:
            printf("Unknown option: %c\n",option);
        }
    }
    
    struct stat bstat;
    if(stat(FPGA_VCCO_AUX_EN,&bstat) == -1)
    {
        printf("Error: kernel does not support power management GPIO access\n");
        return 1;
    }
    
    if(enable_flag)
    {
        if(disable_flag || address_flag || command_flag)
        {
            printf("The -e option is mutually exlusive\n");
            return 1;
        }
        if(fmc_power_on(fmc) == 0)
        {
            printf("FMC %d powered on\n",fmc);
        }
        else
        {
            printf("Failed to power on FMC %d\n",fmc);
        }
    }
    else if(disable_flag)
    {
        if(address_flag || command_flag)
        {
            printf("The -d option is mutually exlusive\n");
            return 1;
        }
        if(fmc_power_off(fmc) == 0)
        {
            printf("FMC %d powered off\n",fmc);
        }
        else
        {
            printf("Failed to power off FMC %d\n",fmc);
        }
    }
    else if(address_flag)
    {
        if(command_flag)
        {
            printf("The -a option is mutually exlusive\n");
            return 1;
        }
        
        int fid;
        int GA0, GA1;
        char bit[5];
        GA0 = GA & 0x02;
        GA1 = GA & 0x01;
        
        char *ga0addr;
        char *ga1addr;
        
        if(fmc == 0)
        {
            ga0addr = FMC0_GA0;
            ga1addr = FMC0_GA1;
        }
        else
        {
            ga0addr = FMC1_GA0;
            ga1addr = FMC1_GA1;
        }
        
        if(export_gpio(ga0addr))
        {
            return 1;
        }
        fid = open(ga0addr, O_RDWR);
        if(fid == -1)
        {
            return 1;
        }
        if(GA0)
        {
            strcpy(bit,"high");
        }
        else
        {
            strcpy(bit,"low");
        }
        write(fid, bit, strlen(bit)+1);
        close(fid);
        
        if(export_gpio(ga1addr))
        {
            return 1;
        }
        fid = open(ga1addr, O_RDWR);
        if(fid == -1)
        {
            return 1;
        }
        if(GA1)
        {
            strcpy(bit,"high");
        }
        else
        {
            strcpy(bit,"low");
        }
        write(fid, bit, strlen(bit)+1);
        close(fid);
        
        printf("GA[0:1] set to 0x%02X\n",GA);
    }
    else if(command_flag)
    {
        unsigned char dev, reg, val;
        
        if(i2c_open("/dev/i2c-3"))
        {
            printf("Error: failed to open I2C\n");
            return 1;
        }
        
        optind++;  /* first non-option argument is the FMC number */
        
        if(optind < argc)
        {
            dev = (unsigned char)strtol(argv[optind], &endptr, 0);
            if(endptr == argv[optind])
            {
                printf("Error: invalid I2C device: %s\n", argv[optind]);
                return 1;
            }
        }
        else
        {
            command_usage();
            return 1;
        }
        
        optind++;
        
        if(optind < argc)
        {
            reg = (unsigned char)strtol(argv[optind], &endptr, 0);
            if(endptr == argv[optind])
            {
                printf("Error: invalid I2C register: %s\n", argv[optind]);
                return 1;
            }
        }
        else
        {
            command_usage();
            return 1;
        }
        
        if(write_flag)
        {
            optind++;
        
            if(optind < argc)
            {
                val = (unsigned char)strtol(argv[optind], &endptr, 0);
                if(endptr == argv[optind])
                {
                    printf("Error: invalid value: %s\n", argv[optind]);
                    return 1;
                }
                
                if(i2c_write(dev,reg,&val,1) == 1)
                {
                    printf("Wrote 0x%02X to register 0x%02X on device 0x%02X\n",val,reg,dev);
                }
                else
                {
                    printf("Error: failed to write to register\n");
                    return 1;
                }
            }
            else
            {
                command_usage();
                return 1;
            }
        }
        else
        {
            if(i2c_read(dev,reg,&val,1) == 1)
            {
                printf("Register 0x%02X on device 0x%02X = 0x%02X\n",reg,dev,val);
            }
            else
            {
                printf("Error: failed to read register\n");
                return 1;
            }
        }
        
        i2c_close();
    }
    else
    {
        char value;
        char *gpio;
        char *fmcaux;
        char *fmcsupply;
        int faux;
        char state;
        
        faux = open(FPGA_VCCO_AUX_RD, O_RDONLY);
        if(faux != -1)
        {
            read(faux, &state, 1);
            close(faux);
            if(state == '0')
            {
                printf("FMC supply power is off\n");
                return 0;
            }
        }
        
        if(fmc == 0)
        {
            gpio = FMC0_nPRSNT_M2C;
            fmcaux = FMC0_AUX_RD;
            fmcsupply = FMC0_SUPPLY_RD;
        }
        else
        {
            gpio = FMC1_nPRSNT_M2C;
            fmcaux = FMC1_AUX_RD;
            fmcsupply = FMC1_SUPPLY_RD;
        }
        
        if(export_gpio(gpio) || export_gpio(fmcaux) || export_gpio(fmcsupply))
        {
            printf("Error: failed to export GPIO\n");
            return 1;
        }
        
        int fid = open(gpio, O_RDWR);
        if(fid == -1)
        {
            printf("Error: failed to open GPIO\n");
            return 1;
        }

        if(read(fid, &value, 1) == 1)
        {
            switch(value)
            {
            case '0':
                printf("Card present in FMC slot %d\n",fmc);
                break;
            case '1':
                printf("No card present in FMC slot %d\n",fmc);
                break;
            }
        }
        else
        {
            printf("Error: failed to read GPIO\n");
        }
        close(fid);
        
        fid = open(fmcaux, O_RDWR);
        if(fid == -1)
        {
            printf("Error: failed to open GPIO\n");
            return 1;
        }

        if(read(fid, &value, 1) == 1)
        {
            switch(value)
            {
            case '0':
                printf("Auxillary power: off\n");
                break;
            case '1':
                printf("Auxillary power: on\n");
                break;
            }
        }
        else
        {
            printf("Error: failed to read GPIO\n");
        }
        close(fid);
        
        fid = open(fmcsupply, O_RDWR);
        if(fid == -1)
        {
            printf("Error: failed to open GPIO\n");
            return 1;
        }

        if(read(fid, &value, 1) == 1)
        {
            switch(value)
            {
            case '0':
                printf("Supply power: off\n");
                break;
            case '1':
                printf("Supply power: on\n");
                break;
            }
        }
        else
        {
            printf("Error: failed to read GPIO\n");
        }
        close(fid);
    }
    
    return 0;
}
