#include "elf_gen.h"

int GenerateElf(const char *filename, Buff *buff)
{
    if (filename     == nullptr) return ElfGen::FilenameIsNullptr;
    if (buff         == nullptr) return ElfGen::BuffIsNullptr;
    if (buff->buffer == nullptr) return ElfGen::BufferIsNullptr;

    Elf64_Half phdr_size = sizeof(Elf64_Phdr);
    Elf64_Half ehdr_size = sizeof(Elf64_Ehdr);

    Elf64_Off phdr_offset  = ehdr_size;
    Elf64_Off entry_offset = ehdr_size + phdr_size;

    assert(ehdr_size == 0x40);
    assert(phdr_size == 0x38);

    size_t elf_output_size = buff->size + ehdr_size + phdr_size;

    Elf64_Ehdr ehdr =
    {
      .e_ident =
      {

    /*[0]*/ ELFMAG0,
    /*[1]*/ ELFMAG1,
    /*[2]*/ ELFMAG2,
    /*[4]*/ ELFMAG3,

    /*[4]*/ ELFCLASS64,
    /*[5]*/ ELFDATA2LSB,
    /*[6]*/ EV_CURRENT,
    /*[7]*/ ELFOSABI_NONE,
    /*[8]*/ ELFABIVERSION,
    /*[9]*/ ELFPAD,

      },

      .e_type      = ET_EXEC,            // Object File Type
      .e_machine   = EM_X86_64,          // Target Architecture
      .e_version   = EV_CURRENT,         // Version of Object File
      .e_entry     = ELF_EPOINT + entry_offset,    // Entry Point
      .e_phoff     = phdr_offset,        // Program Header Offset
      .e_shoff     = SH_NONE,            // Section Header Offset
      .e_flags     = FLAGS_NONE,         // Processor Specific Flags
      .e_ehsize    = ehdr_size,          // Elf Header Size
      .e_phentsize = phdr_size,          // PH Entry Table Size
      .e_phnum     = 1,                  // A Number of Entries in PH
      .e_shentsize = SH_SIZE,            // SH Entry Table Size
      .e_shnum     = SH_NONE,            // A Number of Entries in SH
      .e_shstrndx  = SHN_UNDEF,          // Index of SH Entry Table

    };

    Elf64_Phdr phdr =
    {
        .p_type   = PT_LOAD,
        .p_flags  = PF_X | PF_R,
        .p_offset = 0,
        .p_vaddr  = ELF_EPOINT,
        .p_paddr  = ELF_EPOINT,
        .p_filesz = elf_output_size,
        .p_memsz  = elf_output_size,
        .p_align  = ELF_ALIGN,

    };

    FILE *file = fopen(filename, "w");
    if (file == nullptr) return ElfGen::OpeningFileFailed;

    size_t result = 0;

    result = fwrite(&ehdr, sizeof(char), ehdr_size, file);
    if (result != ehdr_size)  return ElfGen::EhdrWriteFailed;

    result = fwrite(&phdr, sizeof(char), phdr_size, file);
    if (result != phdr_size)  return ElfGen::PhdrWriteFailed;

    result = fwrite(buff->buffer, sizeof(char), buff->size, file);
    if (result != buff->size) return ElfGen::BuffWriteFailed;

    result = fclose(file);
    if (result != 0)          return ElfGen::ClosingFileFailed;

    static char format[] = "chmod +x %s";

    char cmd[ElfGen::CmdMaxSize] = {};
    snprintf(cmd, sizeof(cmd), format, filename);

    system(cmd);

    return 0;
}
