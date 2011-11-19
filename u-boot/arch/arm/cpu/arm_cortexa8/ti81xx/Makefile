#
# Copyright (C) 2009, Texas Instruments, Incorporated
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

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(SOC).a

SOBJS	:= lowlevel_init.o
SOBJS	+= cache.o

COBJS	+= mem.o
COBJS	+= timer.o
COBJS	+= syslib.o
COBJS	+= sys_info.o

COBJS-$(CONFIG_NAND_TI81XX)	+= elm.o

SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS) $(COBJS-y) $(SOBJS))

$(LIB):	$(obj).depend $(OBJS) $(SOBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS) $(SOBJS)

clean:
	rm -f $(SOBJS) $(OBJS)

distclean:	clean
	rm -f $(LIB) core *.bak .depend

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################

