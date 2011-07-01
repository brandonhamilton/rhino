GPMC Test
---------

Author: AWL
Date: 1/7/2011


Instuctions
-----------

* Copy gpmc_test.bof to Rhino file system
* ./gpmc_test.bof &  to run application
* Find out the <pid> for the running gpmc_test process

The gateware implements four memory regions. Note all memory access is 16 bits

Name     Acces GPMC Address  Number of Bytes
VERSION    R    0x08000000       0x02   MSB = type and LSB = version number
reg_led    R/W  0x08800000       0x02   LSB is mapped to LED[7..0]
reg_fmc    R/W  0x09000000       0x02   Writes to half fmc0_LA_{P/N} and read back from the other half see .ucf for pins
reg_file   R/W  0x09800000       0x7f   64 x 16 Registers
reg_word   R/W  0x0A000000       0x04   32 bit register consists of 2 16 bit registers


In Borph linux, the kernel handles the mapping between a file located in /proc/<pid>/hw/ioreg/ and the GPMC memory address

/> echo -e -n "\x<byte:0>\x<byte:1> .... \x<byte:n-1>" > /proc/<pid>/hw/ioreg/reg_file to write upto 64 bytes to reg file

/> od -x /proc/<pid>/hw/ioreg/VERSION to read the version information