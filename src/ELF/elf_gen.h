#ifndef ELF_GEN_H_
#define ELF_GEN_H_

#include <stddef.h>
#include <assert.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

struct Buff
{
    const char *buffer;
    size_t size;
};

/* Type for a 16-bit quantity.  */
typedef uint16_t Elf64_Half;

/* Types for signed and unsigned 32-bit quantities.  */
typedef uint32_t Elf64_Word;
typedef int32_t  Elf64_Sword;

/* Types for signed and unsigned 64-bit quantities.  */
typedef uint64_t Elf64_Xword;
typedef int64_t  Elf64_Sxword;

/* Type of addresses.  */
typedef uint64_t Elf64_Addr;

/* Type of file offsets.  */
typedef uint64_t Elf64_Off;

/* Type for section indices, which are 16-bit quantities.  */
typedef uint16_t Elf64_Section;

/* Type for version symbol information.  */
typedef Elf64_Half Elf64_Versym;

enum ElfConstants
{
    // Subsidiary
    EI_NIDENT = 16, // Number of magic bytes

    // Magic
    ELFMAG0     = 0x7F, // 0
    ELFMAG1     = 'E',  // 1
    ELFMAG2     = 'L',  // 2
    ELFMAG3     = 'F',  // 3

    // Class
    // Data
    // Version
    // OS/ABI - UNIX System V ABI
    // ABI Version
    // Byte index of padding bytes

    ELFCLASS64    = 2,  // 4
    ELFDATA2LSB   = 1,  // 5
    EV_CURRENT    = 1,  // 6
    ELFOSABI_NONE = 0,  // 7
    ELFABIVERSION = 0,  // 8
    ELFPAD        = 0,  // 9


    // Type - Exectutable
    // Machine - AMD x86-64 architecture
    // Version
    // Entry point address
    // Start of program headers
    // Start of section headers
    // Flags
    // Size of this header
    // sizeof(Elf64_Ehdr)
    // Size of program headers
    // Number of program headers
    // Size of section headers
    // Number of section headers
    // Alignment
    // Section header string table index

    ET_EXEC       = 2,
    EM_X86_64     = 62,
 // EV_CURRENT    = 1,
    ELF_EPOINT    = 0x401000,
 // sizeof(Elf64_Ehdr)
    SH_NONE   = 0,
    FLAGS_NONE    = 0,
 // sizeof(Elf64_Phdr)
 // 1
    SH_SIZE   = 0,
 // SH_NONE   = 0,
    ELF_ALIGN = 0x1,
    SHN_UNDEF = 0,


    PT_LOAD   = 1,

    PF_X      = 0x1,
    PF_W      = 0x2,
    PF_R      = 0x4,

};

// ELF-64 header
typedef struct // __attribute__((__packed__))
{
    unsigned char e_ident[EI_NIDENT]; /* Magic number and other info */
    Elf64_Half e_type;                /* Object file type */
    Elf64_Half e_machine;             /* Architecture */
    Elf64_Word e_version;             /* Object file version */
    Elf64_Addr e_entry;               /* Entry point virtual address */
    Elf64_Off  e_phoff;               /* Program header table file offset */
    Elf64_Off  e_shoff;               /* Section header table file offset */
    Elf64_Word e_flags;               /* Processor-specific flags */
    Elf64_Half e_ehsize;              /* ELF header size in bytes */
    Elf64_Half e_phentsize;           /* Program header table entry size */
    Elf64_Half e_phnum;               /* Program header table entry count */
    Elf64_Half e_shentsize;           /* Section header table entry size */
    Elf64_Half e_shnum;               /* Section header table entry count */
    Elf64_Half e_shstrndx;            /* Section header string table index */

} Elf64_Ehdr;

// Program header
typedef struct // __attribute__((__packed__))
{
    uint32_t   p_type;
    uint32_t   p_flags;
    Elf64_Off  p_offset;
    Elf64_Addr p_vaddr;
    Elf64_Addr p_paddr;
    uint64_t   p_filesz;
    uint64_t   p_memsz;
    uint64_t   p_align;

} Elf64_Phdr;

// // Section header
// typedef struct
// {
//   Elf64_Word  sh_name;      /* Section name (string tbl index) */
//   Elf64_Word  sh_type;      /* Section type */
//   Elf64_Xword sh_flags;     /* Section flags */
//   Elf64_Addr  sh_addr;      /* Section virtual addr at execution */
//   Elf64_Off   sh_offset;    /* Section file offset */
//   Elf64_Xword sh_size;      /* Section size in bytes */
//   Elf64_Word  sh_link;      /* Link to another section */
//   Elf64_Word  sh_info;      /* Additional section information */
//   Elf64_Xword sh_addralign; /* Section alignment */
//   Elf64_Xword sh_entsize;   /* Entry size if section holds table */
// } Elf64_Shdr;

namespace ElfGen
{

    enum Constants
    {
        BuffMaxSize = 0x20,
    };

    enum Errors
    {

        // NO ERRORS

        FilenameIsNullptr = 0x900,
        BuffIsNullptr     = 0x901,
        OpeningFileFailed = 0x902,

        EhdrWriteFailed   = 0x903,
        PhdrWriteFailed   = 0x904,
        BuffWriteFailed   = 0x905,

        BufferIsNullptr   = 0x906,

    };


} // namespace ElfGen

int GenerateElf(const char *filename, Buff *buff);

#endif // ELF_GEN_H_
