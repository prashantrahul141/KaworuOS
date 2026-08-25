#ifndef ALLOW_ELF_INTERNAL_INCLUDE
	#error "do not include elf_internal directly, include elf.h instead"
#endif

#include "types.h"
#include "common_defs.h"

/* executable headers ------------------ */

constexpr UNUSED usize ELF_HEADER_IDENT_SIZE = 16;
constexpr usize ELF_HEADER_MAGIC_SIZE = 4;

typedef enum : u8 {
	ELF_HEADER_IDENT_CLASS_NONE = 0,
	ELF_HEADER_IDENT_CLASS_CLASS32 = 1,
	ELF_HEADER_IDENT_CLASS_CLASS64 = 2,
} ElfHeaderIdentClass;

typedef enum : u8 {
	ELF_HEADER_IDENT_DATA_NONE = 0,
	ELF_HEADER_IDENT_DATA_LSB = 1,
	ELF_HEADER_IDENT_DATA_MSB = 1,
} ElfHeaderIdentData;

typedef enum : u8 {
	ELF_HEADER_IDENT_OSABI_NONE = 0,
	ELF_HEADER_IDENT_OSABI_HPUX = 1,
	ELF_HEADER_IDENT_OSABI_NETBSD = 2,
	ELF_HEADER_IDENT_OSABI_LINUX = 3,
	ELF_HEADER_IDENT_OSABI_SOLARIS = 6,
	ELF_HEADER_IDENT_OSABI_AIX = 7,
	ELF_HEADER_IDENT_OSABI_IRIX = 8,
	ELF_HEADER_IDENT_OSABI_FREEBSD = 9,
	ELF_HEADER_IDENT_OSABI_TRU64 = 10,
	ELF_HEADER_IDENT_OSABI_MODESTO = 11,
	ELF_HEADER_IDENT_OSABI_OPENBSD = 12,
	ELF_HEADER_IDENT_OSABI_OPENVMS = 13,
	ELF_HEADER_IDENT_OSABI_NSK = 14
} ElfHeaderIdentOSABI;

typedef enum : u16 {
	ELF_HEADER_TYPE_NONE = 0, /* No file type */
	ELF_HEADER_TYPE_REL = 1, /* Relocatable file */
	ELF_HEADER_TYPE_EXEC = 2, /* Executable file */
	ELF_HEADER_TYPE_DYN = 3, /* Shared object file */
	ELF_HEADER_TYPE_CORE = 4, /* Core file */
	ELF_HEADER_TYPE_NUM = 5, /* Number of defined types */
	ELF_HEADER_TYPE_LOOS = 0xfe00, /* OS-specific range start */
	ELF_HEADER_TYPE_HIOS = 0xfeff, /* OS-specific range end */
	ELF_HEADER_TYPE_LOPROC = 0xff00, /* Processor-specific range start */
	ELF_HEADER_TYPE_HIPROC = 0xffff, /* Processor-specific range end */
} ElfHeaderType;

typedef enum : u16 {
	ELF_HEADER_MACHINE_AARCH64 = 183, /* ARM AARCH64 */
} ElfHeaderMachine;

typedef enum : u32 {
	ELF_HEADER_VERSION_NONE = 0, /* Invalid ELF version */
	ELF_HEADER_VERSION_CURRENT = 1, /* Current version */
	ELF_HEADER_VERSION_NUM = 2,
} ElfHeaderVersion;

typedef struct {
	u8 ident_magic[ELF_HEADER_MAGIC_SIZE]; /* magic */
	ElfHeaderIdentClass ident_class; /* class */
	ElfHeaderIdentData ident_data; /* lsb or msb */
	u8 ident_version; /* version */
	ElfHeaderIdentOSABI ident_os_abi; /* os/abi */
	u8 pad[7];
	u8 ident_abi_version;
	ElfHeaderType type; /* Object file type */
	ElfHeaderMachine machine; /* Architecture */
	ElfHeaderVersion version; /* Object file version */
	u64 entry; /* Entry point virtual address */
	u64 program_header_off; /* Program header table file offset */
	u64 section_header_off; /* Section header table file offset */
	u32 flags; /* Processor-specific flags */
	u16 executable_header_size; /* ELF header size in bytes */
	u16 program_header_table_entry_size; /* Program header table entry size
					      */
	u16 program_header_table_entry_count; /* Program header table entry
						 count */
	u16 section_header_table_entry_size; /* Section header table entry size
					      */
	u16 section_header_table_entry_count; /* Section header table entry
						 count */
	u16 section_header_string_table_index; /* Section header string table
						  index */
} PACKED ElfHeader;
static_assert(sizeof(ElfHeader) == 64, "elfheader is not 64 bytes");

/* section headers ------------------ */

typedef enum {
	ELF_SEC_HEADER_TYPE_NULL = 0, /* Section header table entry unused */
	ELF_SEC_HEADER_TYPE_PROGBITS = 1, /* Program data */
	ELF_SEC_HEADER_TYPE_SYMTAB = 2, /* Symbol table */
	ELF_SEC_HEADER_TYPE_STRTAB = 3, /* String table */
	ELF_SEC_HEADER_TYPE_RELA = 4, /* Relocation entries with addends */
	ELF_SEC_HEADER_TYPE_HASH = 5, /* Symbol hash table */
	ELF_SEC_HEADER_TYPE_DYNAMIC = 6, /* Dynamic linking information */
	ELF_SEC_HEADER_TYPE_NOTE = 7, /* Notes */
	ELF_SEC_HEADER_TYPE_NOBITS = 8, /* Program space with no data (bss) */
	ELF_SEC_HEADER_TYPE_REL = 9, /* Relocation entries, no addends */
	ELF_SEC_HEADER_TYPE_SHLIB = 10, /* Reserved */
	ELF_SEC_HEADER_TYPE_DYNSYM = 11, /* Dynamic linker symbol table */
	ELF_SEC_HEADER_TYPE_INIT_ARRAY = 14, /* Array of constructors */
	ELF_SEC_HEADER_TYPE_FINI_ARRAY = 15, /* Array of destructors */
	ELF_SEC_HEADER_TYPE_PREINIT_ARRAY = 16, /* Array of pre-constructors */
	ELF_SEC_HEADER_TYPE_GROUP = 17, /* Section group */
	ELF_SEC_HEADER_TYPE_SYMTAB_SHNDX = 18, /* Extended section indeces */
	ELF_SEC_HEADER_TYPE_NUM = 19, /* Number of defined types.  */
	ELF_SEC_HEADER_TYPE_LOOS = 0x60000000, /* Start OS-specific.  */
	ELF_SEC_HEADER_TYPE_GNU_ATTRIBUTES = 0x6ffffff5, /* Object attributes.
							  */
	ELF_SEC_HEADER_TYPE_GNU_HASH = 0x6ffffff6, /* GNU-style hash table.  */
	ELF_SEC_HEADER_TYPE_GNU_LIBLIST = 0x6ffffff7, /* Prelink library list */
	ELF_SEC_HEADER_TYPE_CHECKSUM = 0x6ffffff8, /* Checksum for DSO content.
						    */
	ELF_SEC_HEADER_TYPE_LOSUNW = 0x6ffffffa, /* Sun-specific low bound.  */
	ELF_SEC_HEADER_TYPE_SUNW_move = 0x6ffffffa,
	ELF_SEC_HEADER_TYPE_SUNW_COMDAT = 0x6ffffffb,
	ELF_SEC_HEADER_TYPE_SUNW_syminfo = 0x6ffffffc,
	ELF_SEC_HEADER_TYPE_GNU_verdef = 0x6ffffffd, /* Version definition
							section. */
	ELF_SEC_HEADER_TYPE_GNU_verneed = 0x6ffffffe, /* Version needs section.
						       */
	ELF_SEC_HEADER_TYPE_GNU_versym = 0x6fffffff, /* Version symbol table. */
	ELF_SEC_HEADER_TYPE_HISUNW = 0x6fffffff, /* Sun-specific high bound.  */
	ELF_SEC_HEADER_TYPE_HIOS = 0x6fffffff, /* End OS-specific type */
	ELF_SEC_HEADER_TYPE_LOPROC = 0x70000000, /* Start of processor-specific
						  */
	ELF_SEC_HEADER_TYPE_HIPROC = 0x7fffffff, /* End of processor-specific */
	ELF_SEC_HEADER_TYPE_LOUSER = 0x80000000, /* Start of
						    application-specific */
	ELF_SEC_HEADER_TYPE_HIUSER = 0x8fffffff, /* End of application-specific
						  */
} ElfSectionHeaderType;

typedef enum {
	ELF_SEC_HEADER_FLAG_WRITE = (1 << 0), /* Writable */
	ELF_SEC_HEADER_FLAG_ALLOC = (1 << 1), /* Occupies memory during
						 execution */
	ELF_SEC_HEADER_FLAG_EXECINSTR = (1 << 2), /* Executable */
	ELF_SEC_HEADER_FLAG_MERGE = (1 << 4), /* Might be merged */
	ELF_SEC_HEADER_FLAG_STRINGS = (1 << 5), /* Contains nul-terminated
						   strings */
	ELF_SEC_HEADER_FLAG_INFO_LINK = (1 << 6), /* `sh_info' contains SHT
						     index */
	ELF_SEC_HEADER_FLAG_LINK_ORDER = (1 << 7), /* Preserve order after
						      combining */
	ELF_SEC_HEADER_FLAG_OS_NONCONFORMING = (1 << 8), /* Non-standard OS
							    specific handling
							    required */
	ELF_SEC_HEADER_FLAG_GROUP = (1 << 9), /* Section is member of a group.
					       */
	ELF_SEC_HEADER_FLAG_TLS = (1 << 10), /* Section hold thread-local data.
					      */
	ELF_SEC_HEADER_FLAG_COMPRESSED = (1 << 11), /* Section with compressed
						       data. */
	ELF_SEC_HEADER_FLAG_MASKOS = 0x0ff00000, /* OS-specific.  */
	ELF_SEC_HEADER_FLAG_MASKPROC = 0xf0000000, /* Processor-specific */
	ELF_SEC_HEADER_FLAG_ORDERED = (1 << 30), /* Special ordering requirement
						    (Solaris).  */
	ELF_SEC_HEADER_FLAG_EXCLUDE = (1U << 31), /* Section is excluded unless
						     referenced or allocated
						     (Solaris).*/
} ElfSectionHeaderFlags;

typedef struct {
	u32 name; /* Section name (string tbl index) */
	ElfSectionHeaderType type; /* Section type */
	ElfSectionHeaderFlags flags; /* Section flags */
	u64 addr; /* Section virtual addr at execution */
	u64 offset; /* Section file offset */
	u64 size; /* Section size in bytes */
	u32 link; /* Link to another section */
	u32 info; /* Additional section information */
	u64 addr_align; /* Section alignment */
	u64 entry_size; /* Entry size if section holds table */
} PACKED ElfSectionHeader;

/* program headers ------------------ */

typedef enum {
	ELF_PROGRAM_HEADER_TYPE_NULL = 0, /* Program header table entry unused
					   */
	ELF_PROGRAM_HEADER_TYPE_LOAD = 1, /* Loadable program segment */
	ELF_PROGRAM_HEADER_TYPE_DYNAMIC = 2, /* Dynamic linking information */
	ELF_PROGRAM_HEADER_TYPE_INTERP = 3, /* Program interpreter */
	ELF_PROGRAM_HEADER_TYPE_NOTE = 4, /* Auxiliary information */
	ELF_PROGRAM_HEADER_TYPE_SHLIB = 5, /* Reserved */
	ELF_PROGRAM_HEADER_TYPE_PHDR = 6, /* Entry for header table itself */
	ELF_PROGRAM_HEADER_TYPE_TLS = 7, /* Thread-local storage segment */
	ELF_PROGRAM_HEADER_TYPE_NUM = 8, /* Number of defined types */
	ELF_PROGRAM_HEADER_TYPE_LOOS = 0x60000000, /* Start of OS-specific */
	ELF_PROGRAM_HEADER_TYPE_GNU_EH_FRAME = 0x6474e550, /* GCC .eh_frame_hdr
							      segment */
	ELF_PROGRAM_HEADER_TYPE_GNU_STACK = 0x6474e551, /* Indicates stack
							   executability */
	ELF_PROGRAM_HEADER_TYPE_GNU_RELRO = 0x6474e552, /* Read-only after
							   relocation */
	ELF_PROGRAM_HEADER_TYPE_LOSUNW = 0x6ffffffa,
	ELF_PROGRAM_HEADER_TYPE_SUNWBSS = 0x6ffffffa, /* Sun Specific segment */
	ELF_PROGRAM_HEADER_TYPE_SUNWSTACK = 0x6ffffffb, /* Stack segment */
	ELF_PROGRAM_HEADER_TYPE_HISUNW = 0x6fffffff,
	ELF_PROGRAM_HEADER_TYPE_HIOS = 0x6fffffff, /* End of OS-specific */
	ELF_PROGRAM_HEADER_TYPE_LOPROC = 0x70000000, /* Start of
							processor-specific */
	ELF_PROGRAM_HEADER_TYPE_HIPROC = 0x7fffffff, /* End of
							processor-specific */
} ElfProgramHeaderTypes;

typedef enum {
	PF_X = (1 << 0), /* Segment is executable */
	PF_W = (1 << 1), /* Segment is writable */
	PF_R = (1 << 2), /* Segment is readable */
	PF_MASKOS = 0x0ff00000, /* OS-specific */
	PF_MASKPROC = 0xf0000000, /* Processor-specific */
} ElfProgramHeaderFlags;

typedef struct {
	ElfProgramHeaderTypes type; /* Segment type */
	ElfProgramHeaderFlags flags; /* Segment flags */
	u64 offset; /* Segment file offset */
	u64 vaddr; /* Segment virtual address */
	u64 paddr; /* Segment physical address */
	u64 filesz; /* Segment size in file */
	u64 memsz; /* Segment size in memory */
	u64 align; /* Segment alignment */
} PACKED ElfProgramHeader;

/* symbols ----------- */

typedef struct {
	u32 name; /* Symbol name (string tbl index) */
	u8 info; /* Symbol type and binding */
	u8 other; /* Symbol visibility */
	u64 section_index; /* Section index */
	u64 value; /* Symbol value */
	u64 size; /* Symbol size */
} PACKED ElfSymbol;

typedef enum {
	ELF_SYMBOL_INFO_ST_BIN_LOCAL = 0, /* Local symbol */
	ELF_SYMBOL_INFO_ST_BIN_GLOBAL = 1, /* Global symbol */
	ELF_SYMBOL_INFO_ST_BIN_WEAK = 2, /* Weak symbol */
	ELF_SYMBOL_INFO_ST_BIN_NUM = 3, /* Number of defined types.  */
	ELF_SYMBOL_INFO_ST_BIN_LOOS = 10, /* Start of OS-specific */
	ELF_SYMBOL_INFO_ST_BIN_GNU_UNIQUE = 10, /* Unique symbol.  */
	ELF_SYMBOL_INFO_ST_BIN_HIOS = 12, /* End of OS-specific */
	ELF_SYMBOL_INFO_ST_BIN_LOPROC = 13, /* Start of processor-specific */
	ELF_SYMBOL_INFO_ST_BIN_HIPROC = 15, /* End of processor-specific */
} ElfSymbolInfoStBinds;

typedef enum {
	ELF_SYMBOL_INFO_ST_TYPE_NOTYPE = 0, /* Symbol type is unspecified */
	ELF_SYMBOL_INFO_ST_TYPE_OBJECT = 1, /* Symbol is a data object */
	ELF_SYMBOL_INFO_ST_TYPE_FUNC = 2, /* Symbol is a code object */
	ELF_SYMBOL_INFO_ST_TYPE_SECTION = 3, /* Symbol associated with a section
					      */
	ELF_SYMBOL_INFO_ST_TYPE_FILE = 4, /* Symbol's name is file name */
	ELF_SYMBOL_INFO_ST_TYPE_COMMON = 5, /* Symbol is a common data object */
	ELF_SYMBOL_INFO_ST_TYPE_TLS = 6, /* Symbol is thread-local data object*/
	ELF_SYMBOL_INFO_ST_TYPE_NUM = 7, /* Number of defined types.  */
	ELF_SYMBOL_INFO_ST_TYPE_LOOS = 10, /* Start of OS-specific */
	ELF_SYMBOL_INFO_ST_TYPE_GNU_IFUNC = 10, /* Symbol is indirect code
						   object */
	ELF_SYMBOL_INFO_ST_TYPE_HIOS = 12, /* End of OS-specific */
	ELF_SYMBOL_INFO_ST_TYPE_LOPROC = 13, /* Start of processor-specific */
	ELF_SYMBOL_INFO_ST_TYPE_HIPROC = 15, /* End of processor-specific */
} ElfSymbolInfoStTypes;

static inline ElfSymbolInfoStBinds elf_st_bind_from_symbol(const ElfSymbol *sym)
{
	u32 ret = ((sym->info) >> 4);
	// ASSERT(ret >= ELF_SYMBOL_INFO_ST_BIN_LOCAL && ret <=
	// ELF_SYMBOL_INFO_ST_BIN_HIPROC , "wront ret");
	return (ElfSymbolInfoStBinds)ret;
}

static inline ElfSymbolInfoStTypes elf_st_type_from_symbol(const ElfSymbol *sym)
{
	u32 ret = ((sym->info) & 0xf);
	// ASSERT(ret >= ELF_SYMBOL_INFO_ST_TYPE_NOTYPE  && ret <=
	// ELF_SYMBOL_INFO_ST_TYPE_HIPROC , "wront ret");
	return (ElfSymbolInfoStTypes)ret;
}

static inline u8 elf_st_info_reconstruct(u8 st_bind, u8 st_type)
{
	return (u8)(((st_bind) << 4) + ((st_type) & 0xf));
}

/* dynamic -------------- */
typedef enum {
	ELF_DYNAMIC_TAG_NULL = 0, /* Marks end of dynamic section */
	ELF_DYNAMIC_TAG_NEEDED = 1, /* Name of needed library */
	ELF_DYNAMIC_TAG_PLTRELSZ = 2, /* Size in bytes of PLT relocs */
	ELF_DYNAMIC_TAG_PLTGOT = 3, /* Processor defined value */
	ELF_DYNAMIC_TAG_HASH = 4, /* Address of symbol hash table */
	ELF_DYNAMIC_TAG_STRTAB = 5, /* Address of string table */
	ELF_DYNAMIC_TAG_SYMTAB = 6, /* Address of symbol table */
	ELF_DYNAMIC_TAG_RELA = 7, /* Address of Rela relocs */
	ELF_DYNAMIC_TAG_RELASZ = 8, /* Total size of Rela relocs */
	ELF_DYNAMIC_TAG_RELAENT = 9, /* Size of one Rela reloc */
	ELF_DYNAMIC_TAG_STRSZ = 10, /* Size of string table */
	ELF_DYNAMIC_TAG_SYMENT = 11, /* Size of one symbol table entry */
	ELF_DYNAMIC_TAG_INIT = 12, /* Address of init function */
	ELF_DYNAMIC_TAG_FINI = 13, /* Address of termination function */
	ELF_DYNAMIC_TAG_SONAME = 14, /* Name of shared object */
	ELF_DYNAMIC_TAG_RPATH = 15, /* Library search path (deprecated) */
	ELF_DYNAMIC_TAG_SYMBOLIC = 16, /* Start symbol search here */
	ELF_DYNAMIC_TAG_REL = 17, /* Address of Rel relocs */
	ELF_DYNAMIC_TAG_RELSZ = 18, /* Total size of Rel relocs */
	ELF_DYNAMIC_TAG_RELENT = 19, /* Size of one Rel reloc */
	ELF_DYNAMIC_TAG_PLTREL = 20, /* Type of reloc in PLT */
	ELF_DYNAMIC_TAG_DEBUG = 21, /* For debugging; unspecified */
	ELF_DYNAMIC_TAG_TEXTREL = 22, /* Reloc might modify .text */
	ELF_DYNAMIC_TAG_JMPREL = 23, /* Address of PLT relocs */
	ELF_DYNAMIC_TAG_BIND_NOW = 24, /* Process relocations of object */
	ELF_DYNAMIC_TAG_INIT_ARRAY = 25, /* Array with addresses of init fct */
	ELF_DYNAMIC_TAG_FINI_ARRAY = 26, /* Array with addresses of fini fct */
	ELF_DYNAMIC_TAG_INIT_ARRAYSZ = 27, /* Size in bytes of DT_INIT_ARRAY */
	ELF_DYNAMIC_TAG_FINI_ARRAYSZ = 28, /* Size in bytes of DT_FINI_ARRAY */
	ELF_DYNAMIC_TAG_RUNPATH = 29, /* Library search path */
	ELF_DYNAMIC_TAG_FLAGS = 30, /* Flags for the object being loaded */
	ELF_DYNAMIC_TAG_ENCODING = 32, /* Start of encoded range */
	ELF_DYNAMIC_TAG_PREINIT_ARRAY = 32, /* Array with addresses of preinit
					       fct*/
	ELF_DYNAMIC_TAG_PREINIT_ARRAYSZ = 33, /* size in bytes of
						 DT_PREINIT_ARRAY */
	ELF_DYNAMIC_TAG_SYMTAB_SHNDX = 34, /* Address of SYMTAB_SHNDX section */
	ELF_DYNAMIC_TAG_NUM = 35, /* Number used */
	ELF_DYNAMIC_TAG_LOOS = 0x6000000d, /* Start of OS-specific */
	ELF_DYNAMIC_TAG_HIOS = 0x6ffff000, /* End of OS-specific */
	ELF_DYNAMIC_TAG_LOPROC = 0x70000000, /* Start of processor-specific */
	ELF_DYNAMIC_TAG_HIPROC = 0x7fffffff, /* End of processor-specific */
} ElfDynamicTag;

typedef struct {
	i64 tag; /* Dynamic entry type */
	union {
		u64 val; /* Integer value */
		u64 ptr; /* Address value */
	} un;
} PACKED ElfDynamic;

/* relocation -------------- */
typedef struct {
	u64 r_offset; /* Address */
	u64 r_info; /* Relocation type and symbol index */
} PACKED ElfRel;

/* rela */
typedef struct {
	u64 r_offset; /* Address */
	u64 r_info; /* Relocation type and symbol index */
	i64 r_addend; /* Addend */
} PACKED ElfRela;
