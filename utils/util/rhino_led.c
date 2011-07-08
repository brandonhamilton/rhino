#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#define GPIOEX "/sys/class/gpio/export"

#define U_LED_0 "/sys/class/gpio/gpio131/direction"
#define U_LED_1 "/sys/class/gpio/gpio130/direction"

int main(int argc, char **argv)
{
    if(argc != 3)
    {
        printf("Usage: rhino_led NUMBER [on|off]\n");
        return 1;
    }
    
    struct stat bstat;
    int fex;
    if(stat(U_LED_0,&bstat) == -1)
    {
        /* export gpio131 */
        fex = open(GPIOEX, O_WRONLY);
        if(fex != -1)
        {
            write(fex, "131", 4);
            close(fex);
        }
    }
    
    if(stat(U_LED_1,&bstat) == -1)   /* could not stat file */
    {
        /* export gpio130 */
        fex = open(GPIOEX, O_WRONLY);
        if(fex != -1)
        {
            write(fex, "130", 4);
            close(fex);
        }
    }
    
    int led_id;
    char state[5];
    
    led_id = strtol(argv[1],0,10);
    
    if(strcmp(argv[2],"on") == 0)
    {
        strcpy(state, "high");
    }
    else if(strcmp(argv[2], "off") == 0)
    {
        strcpy(state, "low");
    }
    else
    {
        printf("Error: LED state must be 'on' or 'off'\n");
        return 1;
    }
    
    int fled;
    
    switch(led_id)
    {
    case 0:
        fled = open(U_LED_0, O_RDWR);
        if(fled == -1)
        {
            return 1;
        }
        write(fled, state, strlen(state)+1);
        close(fled);
        break;
    case 1:
        fled = open(U_LED_1, O_RDWR);
        if(fled == -1)
        {
            return 1;
        }
        write(fled, state, strlen(state)+1);
        close(fled);
        break;
    default:
        printf("Error: LED number must be 0 or 1\n");
        return 1;
    }
    
    return 0;
}
