/*******************************************************
 * Hayden So 12/08/2005
 *
 * A simply cross-machine byte ordering macro
 * It relies on predfined passed down from make
 * If it is not defined, intel i686 is assuemed
 *******************************************************/
#ifndef _HBYTEORDER_H_
#define _HBYTEORDER_H_
#include <stdint.h>

#if defined(__MACHINE_LE__) && defined(__MACHINE_BE__)
#  error Both __MACHINE_LE__ and __MACHINE_BE__ defined
#endif
#if !defined(__MACHINE_LE__) && !defined(__MACHINE_BE__)
#  if #cpu(i386)
#    define __MACHINE_LE__
#  else
#    warning Neither __MACHINE_LE__ or __MACHINE_BE__ defined
#    warning and we are not running on intel machine, assume BE
#    define __MACHINE_BE__
#  endif
#endif

uint32_t swap_byte32(uint32_t val)
{
    uint32_t tmp = 0;
    int i;
    for (i = 0; i < 4; i++) {
	tmp |= ((val & 0xFF) << ((4 - i - 1) << 3));
	val >>= 8;
    }
    return tmp;
}

uint16_t swap_byte16(uint16_t val)
{
    uint16_t tmp = 0;
    int i;
    for (i = 0; i < 2; i++) {
	tmp |= ((val & 0xFF) << ((2 - i - 1) << 3));
	val >>= 8;
    }
    return tmp;
}

#ifdef __MACHINE_LE__
#define letoh32(val) (val)
#define betoh32(val) swap_byte32(val)
#define letoh16(val) (val)
#define betoh16(val) swap_byte16(val)
#endif /* __MACHINE_LE__ */

#ifdef __MACHINE_BE__
#define htole32(val) swap_byte32(val)
#define htobe32(val) (val)
#define letoh32(val) swap_byte32(val)
#define betoh32(val) (val)
#define htole16(val) swap_byte16(val)
#define htobe16(val) (val)
#define letoh16(val) swap_byte16(val)
#define betoh16(val) (val)
#endif

#endif /* _HBYTEORDER_H_ */
/*******************************************************/
