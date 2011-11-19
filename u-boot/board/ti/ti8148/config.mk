#
# Copyright (C) 2009 Texas Instruments Incorporated - http://www.ti.com/
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation version 2.
#
# This program is distributed "as is" WITHOUT ANY WARRANTY of any
# kind, whether express or implied; without even the implied warranty
# of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#

sinclude $(OBJTREE)/board/$(BOARDDIR)/config.tmp
# This will be used by mkimage extension to select header for image
TI_DEVICE = ti81xx
# ROM code will load u-boot to this address
TI_LOAD_ADDR = 0x40300000
# TEXT_BASE for the 2 stages is different and comes from the Makefile
CROSS_COMPILE:= arm-none-linux-gnueabi-
LDSCRIPT:=board/ti/ti8148/u-boot.lds

