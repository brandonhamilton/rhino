/**
 * showbof
 * Utility to analyse Borph Object Files (BOF)
 * Modified for RHINO Platform
 * Author: Hayden So, Brandon Hamilton
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <endian.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <linux/bof.h>
#include "hbyteorder.h"

FILE* f;
unsigned hwr_foff;
struct bofhdr bhdr;
char* filename;
unsigned char fendian;

char* machine_names[] = {"UNKNOWN", "", "BEE2", "ROACH", "NETFPGA", "RHINO"};
char* elfmachine_names[95] = {
"EM_NONE", "EM_M32", "EM_SPARC", "EM_386",
"EM_68K",  "EM_88K", "EM_INVAL6", "EM_860",
"EM_MIPS", "EM_S370", "EM_MIPS_RS3_LE", "EM_INVAL11",
"EM_INVAL12", "EM_INVAL13", "EM_INVAL14", "EM_PARISC",
"EM_INVAL16", "EM_VPP500", "EM_SPARC32PLUS", "EM_960",
"EM_PPC", "EM_PPC64", "EM_S390", "EM_INVAL23",
"EM_INVAL24", "EM_INVAL25", "EM_INVAL26", "EM_INVAL27",
"EM_INVAL28", "EM_INVAL29", "EM_INVAL30", "EM_INVAL31",
"EM_INVAL32", "EM_INVAL33", "EM_INVAL34", "EM_INVAL35",
"EM_V800", "EM_FR20", "EM_RH32", "EM_RCE",
"EM_ARM", "EM_FAKE_ALPHA", "EM_SH", "EM_SPARCV9",
"EM_TRICORE", "EM_ARC", "EM_H8_300", "EM_H8_300H",
"EM_H8S", "EM_H8_500", "EM_IA_64", "EM_MIPS_X",
"EM_COLDFIRE", "EM_68HC12", "EM_MMA", "EM_PCP",
"EM_NCPU", "EM_NDR1", "EM_STARCORE", "EM_ME16",
"EM_ST100", "EM_TINYJ", "EM_X86_64", "EM_PDSP",
"EM_INVAL64", "EM_INVAL65", "EM_FX66", "EM_ST9PLUS",
"EM_ST7", "EM_68HC16", "EM_68HC11", "EM_68HC08",
"EM_68HC05", "EM_SVX", "EM_ST19", "EM_VAX",
"EM_CRIS", "EM_JAVELIN", "EM_FIREPATH", "EM_ZSP",
"EM_MMIX", "EM_HUANY", "EM_PRISM", "EM_AVR",
"EM_FR30", "EM_D10V", "EM_D30V", "EM_V850",
"EM_M32R", "EM_MN10300", "EM_MN10200", "EM_PJ",
"EM_OPENRISC", "EM_ARC_A5", "EM_XTENSA"
};


char* MachineName(uint16_t m)
{
    if (m < 4) {
	return machine_names[m];
    }
    return machine_names[0];
}

char* ElfMachineName(uint16_t m)
{
    if (m < 95) {
	return elfmachine_names[m];
    }
    return elfmachine_names[0];
}

uint16_t __num16(uint16_t n)
{
    if (fendian == BOFDATA2LSB) {
	return letoh16(n);
    }
    return betoh16(n);
}

uint32_t __num32(uint32_t n)
{
    if (fendian == BOFDATA2LSB) {
	return letoh32(n);
    }
    return betoh32(n);
}


void FixEndian_bofhdr(struct bofhdr* bhdr, unsigned char fendian)
{
    if (fendian == BOFDATA2LSB) {
	bhdr->b_version = letoh32(bhdr->b_version);
	bhdr->b_machine = letoh16(bhdr->b_machine);
	bhdr->b_elfmachine = letoh16(bhdr->b_elfmachine);
	bhdr->b_numchip = letoh32(bhdr->b_numchip);
	bhdr->b_elfoff = letoh32(bhdr->b_elfoff);
	bhdr->b_hwoff = letoh32(bhdr->b_hwoff);
    } else if (fendian == BOFDATA2MSB) {
	bhdr->b_version = betoh32(bhdr->b_version);
	bhdr->b_machine = betoh16(bhdr->b_machine);
	bhdr->b_elfmachine = betoh16(bhdr->b_elfmachine);
	bhdr->b_numchip = betoh32(bhdr->b_numchip);
	bhdr->b_elfoff = betoh32(bhdr->b_elfoff);
	bhdr->b_hwoff = betoh32(bhdr->b_hwoff);
    } else {
	printf("unknown endian %u\n", fendian);
    }
}

void FixEndian_hwrhdr(struct hwrhdr* hwrhdr, unsigned char fendian)
{
    if (fendian == BOFDATA2LSB) {
	hwrhdr->flag = letoh32(hwrhdr->flag);
	hwrhdr->addr.class = letoh16(hwrhdr->addr.class);
	hwrhdr->addr.addr = letoh16(hwrhdr->addr.addr);
	hwrhdr->pl_off = letoh32(hwrhdr->pl_off);
	hwrhdr->pl_len = letoh32(hwrhdr->pl_len);
	hwrhdr->nr_symbol = letoh32(hwrhdr->nr_symbol);
	hwrhdr->strtab_off = letoh32(hwrhdr->strtab_off);
	hwrhdr->next_hwr = letoh32(hwrhdr->next_hwr);
    } else if (fendian == BOFDATA2MSB) {
	hwrhdr->flag = betoh32(hwrhdr->flag);
	hwrhdr->addr.class = betoh16(hwrhdr->addr.class);
	hwrhdr->addr.addr = betoh16(hwrhdr->addr.addr);
	hwrhdr->pl_off = betoh32(hwrhdr->pl_off);
	hwrhdr->pl_len = betoh32(hwrhdr->pl_len);
	hwrhdr->nr_symbol = betoh32(hwrhdr->nr_symbol);
	hwrhdr->strtab_off = betoh32(hwrhdr->strtab_off);
	hwrhdr->next_hwr = betoh32(hwrhdr->next_hwr);
    } else {
	printf("unkown undian %u\n", fendian);
    }
}

void FixEndian_ioreg(struct bofioreg* ior, unsigned char fendian)
{
    if (fendian == BOFDATA2LSB) {
	ior->name = letoh16(ior->name);
	ior->mode = letoh16(ior->mode);
	ior->loc = letoh32(ior->loc);
	ior->len = letoh32(ior->len);
    } else if (fendian == BOFDATA2MSB) {
	ior->name = betoh16(ior->name);
	ior->mode = betoh16(ior->mode);
	ior->loc = betoh32(ior->loc);
	ior->len = betoh32(ior->len);
    } else {
	printf("unkown undian %u\n", fendian);
    }
}

void Usage(void)
{
    printf("Usage: showbof bof_file\n");
    exit(1);
}

void Initialize(int argc, char** argv)
{
    if (argc < 2) {
	Usage();
    }
    filename = argv[1];

    f = fopen(filename, "rb");
    if (!f) {
	printf("Error opening %s -- %s\n", filename, strerror(errno));
    }
}

void PrintBOFHeader(void)
{
    int i;
    printf("ident[]:");
    for (i = 0; i < 16; i++) printf(" %02x", bhdr.ident[i]);
    printf("\n");
    printf("version: %d\n", bhdr.b_version);
    printf("machine: %s\n", MachineName(bhdr.b_machine));
    printf("elfmachine: %s\n", ElfMachineName(bhdr.b_elfmachine));
    printf("numchip: %d\n", bhdr.b_numchip);
    printf("elfoff: 0x%x\n", bhdr.b_elfoff);
    printf("hwoff: 0x%x\n", bhdr.b_hwoff);
}

void SprintHWRAddr(char* buf, struct hwr_addr* a)
{
    char cbuf[256];
    if (a->class == HAC_BEE2FPGA) {
	strcpy(cbuf, "BEE2FPGA");
    } else if (a->class == HAC_PRMOD) {
	strcpy(cbuf, "PRMOD");
    } else if (a->class == HAC_ROACHV5) {
	strcpy(cbuf, "ROACHV5");
    } else if (a->class == HAC_NETFPGA) {
    strcpy(cbuf, "NETFPGA");
    } else if (a->class == HAC_RHINO) {
    strcpy(cbuf, "RHINO");
    } else {
	sprintf(cbuf, "unknown class (%d)\n", a->class);
    }
    sprintf(buf, "class: %s, addr: %d", cbuf, a->addr);
}

void PrintSymbols(struct hwrhdr* hwrhdr)
{
    unsigned strtab_foff, strtab_len;
    char* strtab;
    struct bofioreg bofioreg;
    int i;

    if (hwrhdr->nr_symbol == 0) {
	printf("  no symbol\n");
	return;
    }
    strtab_foff = 
	hwr_foff + sizeof(struct hwrhdr) + 
	hwrhdr->nr_symbol * sizeof(struct bofioreg);
    strtab_len = hwrhdr->pl_off - hwrhdr->strtab_off;
    if (strtab_len <= 0) {
	printf("Negative strtab_len %d\n", strtab_len);
	exit(1);
    }
    strtab = (char*) malloc(strtab_len);
    if (!strtab) {
	printf("Error mallocing strtab\n");
	exit(1);
    }
    if (fseek(f, strtab_foff, SEEK_SET)) {
	printf("Error seeking file into strtab at 0x%x: %s\n",
	       strtab_foff, strerror(errno));
	exit(1);
    }
    if (fread(strtab, strtab_len, 1, f) < 1) {
	printf("Error reading strtab: %s\n", strerror(errno));
	exit(1);
    }
    if (fseek(f, hwr_foff + sizeof(struct hwrhdr), SEEK_SET)) {
	printf("Error seeking file into ioreg at 0x%x: %s\n",
	       hwr_foff + sizeof(struct hwrhdr), strerror(errno));
	exit(1);
    }
    printf("      type    Name                    Mode        loc         len\n");
    for (i = 0; i < hwrhdr->nr_symbol; i++) {
	if (fread(&bofioreg, sizeof(struct bofioreg), 1, f) < 1) {
	    printf("Error reading ioreg: %s\n", strerror(errno));
	    exit(1);
	}
	FixEndian_ioreg(&bofioreg, fendian);
	printf("  %.2d: ioreg   ", i);
	printf("%-24s0x%-8x  0x%-8x  %-d\n", 
	       strtab + bofioreg.name, bofioreg.mode, 
	       bofioreg.loc, bofioreg.len);
    }
    free(strtab);
}

void PrintHWRHdr(struct hwrhdr* hwrhdr)
{
    char buf[256];
    unsigned num;
    printf("  flag: %s\n", hwrhdr->flag & HFG_PLACED?"HFG_PLACED":"NONE");
    SprintHWRAddr(buf, &hwrhdr->addr);
    printf("  addr: %s\n", buf);
    printf("  pl_off: 0x%x (%d)\n", hwrhdr->pl_off, hwrhdr->pl_off);
    printf("  pl_len: 0x%x (%d)\n", hwrhdr->pl_len, hwrhdr->pl_len);
    printf("  nr_symbol: %d\n", hwrhdr->nr_symbol);
    num = hwrhdr->strtab_off; 
    printf("  strtab_off: 0x%x (%d)\n", num, num);
    num = hwrhdr->next_hwr;
    printf("  next_hwr: 0x%x (%d)\n", num, num);
    printf("  << symbols >>\n");
    PrintSymbols(hwrhdr);
}

/* Squeeze some more info from xilinx bitstream if possible */
void PrintHWRPayload(struct hwrhdr* hwrhdr)
{
    uint32_t pl_foff;
    char* pl, *p, *m;
    int pagesize;

    printf("  << payload >>\n");
    pl_foff = hwr_foff + sizeof(struct hwrhdr) + 
	hwrhdr->nr_symbol * sizeof(struct bofioreg) + 
	(hwrhdr->pl_off - hwrhdr->strtab_off);
    pagesize = getpagesize();
    m = mmap(NULL, 4096, PROT_READ, MAP_PRIVATE, fileno(f),
	     (pl_foff / pagesize) * pagesize);
    if (m == MAP_FAILED) {
	printf("pl_len=%d, pl_foff=%d\n", hwrhdr->pl_len, pl_foff);
	printf("Error mmapping payload at: %s\n",
	       strerror(errno));
	exit(1);
    }
    pl = m + (pl_foff % pagesize);

    p = memchr(pl, 'a', 200);
    if (!p) {
	printf("can't find first info field");
	return;
    }

    printf("  Filename: %s\n", p+3);
    p += *(p+2) + 3;

    printf("  Device  : %s\n", p+3);
    p += *(p+2) + 3;

    printf("  Date    : %s\n", p+3);
    p += *(p+2) + 3;

    printf("  Time    : %s\n", p+3);
    p += *(p+2) + 3;

    munmap(m, 4096);
}

void PrintHWR()
{
    int i;
    struct hwrhdr hwrhdr;

    if (bhdr.b_numchip == 0) {
	printf("** no hwr in file **\n");
	return;
    }
    hwr_foff = bhdr.b_hwoff;

    for (i = 0 ; i < bhdr.b_numchip; i++) {
	printf("** hwr #%d **\n", i);
	if (fseek(f, hwr_foff, SEEK_SET)) {
	    printf("Error seeking file to hwoff=0x%x: %s\n", 
		   hwr_foff, strerror(errno));
	    exit(1);
	}
	if (fread(&hwrhdr, sizeof(struct hwrhdr), 1, f) < 1) {
	    printf("Error reading hwrhdr: %s\n", strerror(errno));
	    exit(1);
	}
	FixEndian_hwrhdr(&hwrhdr, fendian);

	PrintHWRHdr(&hwrhdr);
	PrintHWRPayload(&hwrhdr);

	hwr_foff += sizeof(struct hwrhdr) + 
	    hwrhdr.nr_symbol * sizeof(struct bofioreg) + 
	    (hwrhdr.pl_off - hwrhdr.strtab_off) + hwrhdr.pl_len;
    }
}


int main(int argc, char** argv)
{
    int ret;

    Initialize(argc, argv);

    ret = fread(&bhdr, sizeof(struct bofhdr), 1, f);
    if (ret < 1) {
	printf("Error parsing %s: %s\n", filename, strerror(errno));
	return 1;
    }

    /************* print header **************/
    if (bhdr.ident[0] != 0x19 || bhdr.ident[1] != 'B' ||
	bhdr.ident[2] != 'O' || bhdr.ident[3] != 'F') {
	printf("Not a BOF file\n");
	return 1;
    }
    fendian = bhdr.ident[BI_ENDIAN];
    if (fendian != BOFDATA2LSB && fendian != BOFDATA2MSB) {
	printf("Unknown endianess %u\n", fendian);
	return 1;
    }

    FixEndian_bofhdr(&bhdr, fendian);

    printf("Filename: %s\n", filename);
    PrintBOFHeader();
    PrintHWR();


    fclose(f);
    return 0;
}
