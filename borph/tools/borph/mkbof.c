/**
 * mkbof
 * Utility to generate executable Borph Object Files (BOF)
 * Modified for RHINO Platform
 * Author: Hayden So, Brandon Hamilton
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <elf.h>

#include <linux/bof.h>
#include "elf_def.h"

struct symbol {
    char name[256];
    uint32_t mode;
    uint32_t loc;
    uint32_t len;
};

#define SYMTAB_SIZE 1024
struct symbol symtab[SYMTAB_SIZE];   
long numsym = 0;

uint32_t strtab_size;

FILE *felf, *fbof, *fbit, *fsym;
char * felf_tmpfilename;
long elf_size;
long bitfile_size;
Elf32_Ehdr ehdr;

int s_placed;
int s_placedloc;
int s_noconfig;
int s_verbose;
unsigned g_ekver;
unsigned g_addrClass;

#define BLOCK_SIZE 512
char block[BLOCK_SIZE];

/* copy all remaining bytes (starting from current possition of fromfile)
 * from @fromfile to fbof
 */
void copy_remain_file(FILE* fromfile)
{
    size_t size;
    while ((size = fread(&block, 1, BLOCK_SIZE, fromfile)) > 0) { 
	if ((size = fwrite(&block, 1, size, fbof)) < 0) {
	    perror("tmp file write error while copying\n");
	}
    }
    if (!feof(fromfile)) {
	if (!ferror(fromfile)) {
	    printf("huh?\n");
	} else {
	    perror("copy_remain_file: error while reading fromfile\n");
	}
    }
}

FILE* GenerateElfFile(void)
{
    size_t cnt;
    FILE* ret = NULL;
#ifdef WINDOWS
    felf_tmpfilename = _tempnam("","");/* Generate name in TMP dir */
    ret = fopen(felf_tmpfilename, "wb+");
#else
    ret = tmpfile();
#endif
    if (!ret) {
		printf("Error, cannot open tmpfile: %s\n", strerror(errno));
		goto out;
    }
    cnt = -1;
  	switch(g_addrClass)
  	{
  		case HAC_NETFPGA:
  			cnt = fwrite(default_x86_elf_bin, DEFAULT_X86_ELF_BIN_SIZE, 1, ret);
  			break;
  		case HAC_RHINO:
  			cnt = fwrite(default_arm_elf_bin, DEFAULT_ARM_ELF_BIN_SIZE, 1, ret);
  			break;
  		case HAC_ROACHV5:
  			cnt = fwrite(default_ppc_elf_bin, DEFAULT_PPC_ELF_BIN_SIZE, 1, ret);
  			break;
  	}
    
    if (cnt <= 0) {
		printf("Error, writing tmpfile: %s\b", strerror(errno));
		goto out;
    }
 out:
    return ret;
}

void FixEndian_bofhdr(struct bofhdr* bhdr, unsigned char oendian)
{
    if (oendian == ELFDATA2LSB) {
		bhdr->b_version = htole32(bhdr->b_version);
		bhdr->b_machine = htole16(bhdr->b_machine);
		bhdr->b_elfmachine = htole16(bhdr->b_elfmachine);
		bhdr->b_numchip = htole32(bhdr->b_numchip);
		bhdr->b_elfoff = htole32(bhdr->b_elfoff);
		bhdr->b_hwoff = htole32(bhdr->b_hwoff);
		bhdr->b_ekver = htole32(bhdr->b_ekver);
    } else if (oendian == ELFDATA2MSB) {
		bhdr->b_version = htobe32(bhdr->b_version);
		bhdr->b_machine = htobe16(bhdr->b_machine);
		bhdr->b_elfmachine = htole16(bhdr->b_elfmachine);
		bhdr->b_numchip = htobe32(bhdr->b_numchip);
		bhdr->b_elfoff = htobe32(bhdr->b_elfoff);
		bhdr->b_hwoff = htobe32(bhdr->b_hwoff);
		bhdr->b_ekver = htobe32(bhdr->b_ekver);
    } else {
		printf("Error: unknown endian %u\n", oendian);
    }
}

void FixEndian_hwrhdr(struct hwrhdr* hwrhdr, unsigned char oendian)
{
    if (oendian == ELFDATA2LSB) {
		hwrhdr->flag = htole32(hwrhdr->flag);
		hwrhdr->addr.class = htole16(hwrhdr->addr.class);
		hwrhdr->addr.addr = htole16(hwrhdr->addr.addr);
		hwrhdr->pl_off = htole32(hwrhdr->pl_off);
		hwrhdr->pl_len = htole32(hwrhdr->pl_len);
		hwrhdr->nr_symbol = htole32(hwrhdr->nr_symbol);
		hwrhdr->strtab_off = htole32(hwrhdr->strtab_off);
		hwrhdr->next_hwr = htole32(hwrhdr->next_hwr);
    } else if (oendian == ELFDATA2MSB) {
		hwrhdr->flag = htobe32(hwrhdr->flag);
		hwrhdr->addr.class = htobe16(hwrhdr->addr.class);
		hwrhdr->addr.addr = htobe16(hwrhdr->addr.addr);
		hwrhdr->pl_off = htobe32(hwrhdr->pl_off);
		hwrhdr->pl_len = htobe32(hwrhdr->pl_len);
		hwrhdr->nr_symbol = htobe32(hwrhdr->nr_symbol);
		hwrhdr->strtab_off = htobe32(hwrhdr->strtab_off);
		hwrhdr->next_hwr = htobe32(hwrhdr->next_hwr);
    } else {
		printf("Error: unknown endian %u\n", oendian);
    }
}

void FixEndian_ioreg(struct bofioreg* ior, unsigned char oendian)
{
    if (oendian == ELFDATA2LSB) {
		ior->name = htole16(ior->name);
		ior->mode = htole16(ior->mode);
		ior->loc = htole32(ior->loc);
		ior->len = htole32(ior->len);
    } else if (oendian == ELFDATA2MSB) {
		ior->name = htobe16(ior->name);
		ior->mode = htobe16(ior->mode);
		ior->loc = htobe32(ior->loc);
		ior->len = htobe32(ior->len);
    } else {
		printf("Error: unknown endian %u\n", oendian);
    }
}

void UsageExit()
{
    fprintf(stderr, "\nUsage: mkbof <options> bitfile\n\n");
    fprintf(stderr, "where options can be:\n");
    fprintf(stderr, 
	    "  -o filename    : output file name (default is bitfile.bof)\n"
	    "  -s filename    : symbol file name\n"
	    "  -t hwr_typ     : 1=HAC_BEE2FPGA, 3=HAC_ROACHV5, 4=NetFPGA, 5=RHINO\n"
	    "  -e filename    : embedded ELF file name\n"
	    "  -v             : be verbose\n"
	    "\n\n"
	    "mkbof version 2.0\n"
);
    exit(1);
}

int Initialize(int argc, char** argv)
{
    int i;
    char fbofname[256];
    char *felfname = NULL, *fbitname = NULL, *fsymname = NULL;
    if (argc < 4) {
		UsageExit();
    }
    fbofname[0] = 0;
    s_placed = 0;
    s_placedloc = 0;
    s_noconfig = 0;
    s_verbose = 0;
    g_ekver = 0x0000e001;
    g_addrClass = HAC_RHINO;
    for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
	    	switch (argv[i][1]) {
			    case 'o': 
					i += 1;
					if (i < argc) {
					    strcpy(fbofname, argv[i]);
					} else {
					    UsageExit();
					}
					break;
			    case 'e':
					i += 1;
					if (i < argc) {
					    felfname = argv[i];
					} else {
					    UsageExit();
					}
					break;
			    case 's':
					i += 1;
					if (i < argc) {
					    fsymname = argv[i];
					} else {
					    UsageExit();
					}
					break;
				case 't':
					i += 1;
					if (i < argc) {
					    g_addrClass = atoi(argv[i]);
					} else {
					    UsageExit();
					}
					break;
				case 'v':
					s_verbose = 1;
					break;
			    default: UsageExit();
			}
		} else {
	    	// assume dangling argument is bifile name
	    	fbitname = argv[i];
		}
    }

    if (fbofname[0] == 0) {
		sprintf(fbofname, "%s.bof", fbitname);
    }
    
    if (s_verbose) {
		printf("elf=%s, bit=%s, sym=%s, bof=%s\n", felfname, fbitname, fsymname, fbofname);
    }

    if (felfname == NULL) {
		felf = GenerateElfFile();
    } else {
		felf = fopen(felfname, "rb");
    }
    if (!felf) { 
		fprintf(stderr, "Error: Failed to open ELF file '%s'\n", felfname);
		return 1;
    } else {
		fseek(felf, 0, SEEK_END);
		elf_size = ftell(felf);
		if (s_verbose) {
	    	printf("ELF file size = %ld\n", elf_size);
		}
		fseek(felf, 0, SEEK_SET);
    }

    fbit = fopen(fbitname, "rb");
    if (!fbit) { 
		fprintf(stderr, "Error: Failed to open bit file '%s'\n", fbitname);
		return 1;
    } else {
		fseek(fbit, 0, SEEK_END);
		bitfile_size = ftell(fbit);
		if (s_verbose) {
	    	printf("bit file size = %ld\n", bitfile_size);
		}
		fseek(fbit, 0, SEEK_SET);
    }

    fsym = fopen(fsymname, "rb");
    if (!fsym) { 
		fprintf(stderr, "Error: Failed to open symbol file '%s'\n", fsymname);
		return 1;
    }

    fbof = fopen(fbofname, "wb");
    if (!fbof) {
		fprintf(stderr, "Error: Failed to create output file '%s'\n", fbofname);
		return 1;
    }

    return 0;
}

int ReadSymbolFile()
{
    size_t size;
    int len;
    struct symbol* sym;
    numsym = 0;
    strtab_size = 0;

    while (fgets(block, BLOCK_SIZE, fsym)) {
		len = strlen(block);
		if (block[len-1] != '\n') {
	    	fprintf(stderr, "line longer than %d bytes truncated\n", BLOCK_SIZE);
	    	return 1;
		}
		sym = &symtab[numsym];
		size = sscanf(block, "%s %x %x %x\n", sym->name, &sym->mode, &sym->loc, &sym->len);
		if (size < 4) {
	    	perror("error reading input file, parse error?\n");
	    	return 2;
		}
		if (s_verbose) {
	    	printf("Symbol: name=%s, mode=0x%08x, loc=0x%08x, len=%d\n", sym->name, sym->mode, sym->loc, sym->len);
		}
		strtab_size += strlen(sym->name) + 1;
		numsym += 1;
		if (numsym > SYMTAB_SIZE) {
	    	printf("INTERNAL ERROR: number of symbols exceeds maximum (%d)", SYMTAB_SIZE);
	    	return 2;
		}
    }
    if (ferror(fsym)) {
		perror("Error: failed to read symbol file");
		return 1;
    }

    return 0;
}

void WriteSymtab(unsigned char oendian)
{
    int i;
    uint16_t strtab_off = 0;
    struct bofioreg ior = {0,0,0,0};
    for (i = 0; i < numsym; i++) {
		ior.name = strtab_off;
		ior.mode = symtab[i].mode;
		ior.loc = symtab[i].loc;
		ior.len = symtab[i].len;
		if (s_verbose) {
		    printf("Writing symbol: name=%s, mode=0x%08x, loc=0x%08x, len=%d\n", symtab[i].name, ior.mode, ior.loc, ior.len);
		}
		FixEndian_ioreg(&ior, ELFDATA2LSB);
		strtab_off += strlen(symtab[i].name) + 1;
		if (fwrite(&ior, 1, sizeof(struct bofioreg), fbof) < 0) {
	    	perror("error writing ioreg to fbof\n");
	    	return;
		}
    }
    return;
}

void WriteStrtab()
{
    int i;
    char* p;
    for (i = 0; i < numsym; i++) {
		p = symtab[i].name;
		if (fwrite(p, 1, strlen(p) + 1, fbof) < 0) {
	    	perror("error writing string to fbof\n");
	    	return;
		}
    }
}

int main(int argc, char** argv)
{
    struct bofhdr bhdr;
    struct hwrhdr hwrhdr;
    unsigned char oendian;
    size_t size;
    int i, ret;

    if ((ret = Initialize(argc, argv)) > 0) {
		return ret;
    }

    /******************************************************
     * Read and save ELF Header
     ******************************************************/
    /* read elf header from felf
     * should check that felf is an ELF
     */
    size = fread(&ehdr, sizeof(Elf32_Ehdr), 1, felf);
    if (size < 1) {
		perror("error opening elf file\n");
		return 1;
    }

    /******************************************************
     * Read and save all symbols
     ******************************************************/
    if (ReadSymbolFile()) {
		return 1;
    }

    /******************************************************
     * Create BOF header
     ******************************************************/
    memset(&bhdr, 0, sizeof(struct bofhdr));
    // create a bof header, MUST be <= 52 bytes
    bhdr.ident[0] = 0x19;
    bhdr.ident[1] = 'B';
    bhdr.ident[2] = 'O';
    bhdr.ident[3] = 'F';
    oendian = bhdr.ident[BI_ENDIAN] = ehdr.e_ident[EI_DATA];
    bhdr.b_version = 6;
    if (g_addrClass == HAC_NETFPGA) {
		bhdr.b_machine = BM_NETFPGA;
    } else if (g_addrClass == HAC_ROACHV5) {
		bhdr.b_machine = BM_ROACH;
	} else if (g_addrClass == HAC_RHINO) {
		bhdr.b_machine = BM_RHINO;
    } else {
		bhdr.b_machine = BM_BEE2;
    }
    bhdr.b_elfmachine = ehdr.e_machine;
    bhdr.b_numchip = 1;
    bhdr.b_elfoff = elf_size;
    bhdr.b_hwoff = elf_size + sizeof(Elf32_Ehdr);
    bhdr.b_ekver = g_ekver;
    bhdr.load_err = 0;
    for (i = 0; i < 2; i++) {
		bhdr.pad[i] = 0xA5368120 + i;
    }
    FixEndian_bofhdr(&bhdr, ELFDATA2LSB);

    if (fwrite(&bhdr, sizeof(struct bofhdr), 1, fbof) < 1) {
		perror("out file write error\n");
		return -1;
    }

    /******************************************************
     * Stream the remaining of ELF file to BOF
     ******************************************************/
    copy_remain_file(felf);

    /******************************************************
     * Write original ELF header at end of ELF payload in BOF
     ******************************************************/
    if (fwrite(&ehdr, sizeof(Elf32_Ehdr), 1, fbof) < 1) {
		perror("bof file write error\n");
		return -1;
    }

    /******************************************************
     * Write hw_region header
     ******************************************************/
    hwrhdr.addr.class = g_addrClass;
    if (s_placed) {
		hwrhdr.flag = HFG_PLACED;
		hwrhdr.addr.addr = s_placedloc;
    } else {
		hwrhdr.flag = 0;
		hwrhdr.addr.addr = 0;
    }
    if (s_noconfig) {
		hwrhdr.flag |= HFG_NOCONFIG;
    }

    hwrhdr.nr_symbol = numsym;
    hwrhdr.strtab_off = 
	elf_size + sizeof(Elf32_Ehdr) +
	sizeof(struct hwrhdr) + numsym * sizeof(struct bofioreg);
    hwrhdr.pl_off = hwrhdr.strtab_off + strtab_size;
    hwrhdr.pl_len = bitfile_size;
    hwrhdr.next_hwr = 0;
    FixEndian_hwrhdr(&hwrhdr, ELFDATA2LSB);

    if (fwrite(&hwrhdr, sizeof(struct hwrhdr), 1, fbof) < 1) {
		perror("bof file write error while writing hwrhdr\n");
		return -1;
    }

    WriteSymtab(oendian);
    WriteStrtab();

    /******************************************************
     * Copy entire bit file
     ******************************************************/
    copy_remain_file(fbit);

    /******************************************************
     * Epiloque
     ******************************************************/    
#ifdef WINDOWS
    if( felf_tmpfilename )
    {
		_unlink(felf_tmpfilename);
		free(felf_tmpfilename);
    }
#endif
    fclose(fbit);
    fclose(felf);
    fclose(fbof);
    fclose(fsym);

    return 0;
}
