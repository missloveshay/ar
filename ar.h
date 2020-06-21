#ifndef AR_H
#define AR_H

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>

#endif

#define AR_VERSION "1.0.1"


#define OPT_HELP        0x1
#define OPT_VERSION     0x2
#define OPT_T           0x4
#define OPT_X           0x8
#define OPT_R           0x10
#define OPT_P           0x100




typedef signed char int8_t;
typedef short int int16_t;
typedef int int32_t;
typedef long long int int64_t;
typedef unsigned char           uint8_t;
typedef unsigned short int      uint16_t;
typedef unsigned int            uint32_t;
typedef unsigned long long int  uint64_t;

/* Type for a 16-bit quantity.  */
typedef uint16_t Elf32_Half;
typedef uint16_t Elf64_Half;

/* Types for signed and unsigned 32-bit quantities.  */
typedef uint32_t Elf32_Word;
typedef	int32_t  Elf32_Sword;
typedef uint32_t Elf64_Word;
typedef	int32_t  Elf64_Sword;

/* Types for signed and unsigned 64-bit quantities.  */
typedef uint64_t Elf32_Xword;
typedef	int64_t  Elf32_Sxword;
typedef uint64_t Elf64_Xword;
typedef	int64_t  Elf64_Sxword;

/* Type of addresses.  */
typedef uint32_t Elf32_Addr;
typedef uint64_t Elf64_Addr;

/* Type of file offsets.  */
typedef uint32_t Elf32_Off;
typedef uint64_t Elf64_Off;

/* Type for section indices, which are 16-bit quantities.  */
typedef uint16_t Elf32_Section;
typedef uint16_t Elf64_Section;

/* Type for version symbol information.  */
typedef Elf32_Half Elf32_Versym;
typedef Elf64_Half Elf64_Versym;


#define	ELFMAG		"\177ELF"       //ELF文件魔数
#define ARMAG  "!<arch>\012"        //归档文件魔数



#define AFF_BINTYPE_REL 1
#define AFF_BINTYPE_DYN 2
#define AFF_BINTYPE_AR  3

/* Legal values for e_type (object file type).  */

#define ET_NONE		0		/* No file type */
#define ET_REL		1		/* Relocatable file *///可重定位文件
#define ET_EXEC		2		/* Executable file */
#define ET_DYN		3		/* Shared object file *///共享对象文件
#define ET_CORE		4		/* Core file */

typedef struct Ar_State{
    const char *outfile;
    char *filename;
    int fd;
    int opt;

    char **file;
    int file_num;
}Ar_State;

Ar_State *ar;
int sym_num;

typedef struct ArchiveHeader {
    char ar_name[16];           /* name of this member */
    char ar_date[12];           /* file mtime */
    char ar_uid[6];             /* owner uid; printed as decimal */
    char ar_gid[6];             /* owner gid; printed as decimal */
    char ar_mode[8];            /* file mode, printed as octal   */
    char ar_size[10];           /* 文件大小，打印为十进制.头部大小不计算在内.*/
    char ar_fmag[2];            /* should contain ARFMAG */
} ArchiveHeader;


#define EI_NIDENT (16)
typedef struct
{
  unsigned char	e_ident[EI_NIDENT];	//魔树和其他信息
  Elf64_Half	e_type;			//目标文件类型
  /*
ET_NONE 0 无文件类型
ET_REL 1 可重定位文件
ET_EXEC 2 可执行文件
ET_DYN 3 共享目标文件
ET_CORE 4 核心文件
ET_LOPROC 0xff00 特定于处理器
ET_HIPROC 0xffff 特定于处理器

  */
  Elf64_Half	e_machine;		/* 指定独立文件所需的体系结构 */
  /*
  EM_NONE 0 无计算机 
  EM_SPARC 2 SPARC 
  EM_386 3 Intel80386 
  EM_SPARC32PLUS 18 SunSPARC32+ 
  EM_SPARCV9 43 SPARCV9 
  EM_AMD64 62 AMD64
  */
  Elf64_Word	e_version;		/* 标识目标文件版本，如下表中所列 */
  /*
EV_NONE 0 无效版本
EV_CURRENT >=1 当前版本

  */
  Elf64_Addr	e_entry;		/* 虚拟地址，系统首先将控制权转移到该地址，进而启动进程 */
  Elf64_Off	e_phoff;		/* 程序头表的文件偏移（以字节为单位）。如果文件没有程序头表，则此成员值为 零*/
  Elf64_Off	e_shoff;		/* 节头表的文件偏移（以字节为单位）。如果文件没有节头表，则此成员值为零。*/
  Elf64_Word	e_flags;		/* 与文件关联的特定于处理器的标志 */
  Elf64_Half	e_ehsize;		/* ELF头的大小（以字节为单位）。 */
  Elf64_Half	e_phentsize;		/* Program header table entry size */
  Elf64_Half	e_phnum;		/* 程序头表中的项数。 */
  Elf64_Half	e_shentsize;		/* 节头的大小 */
  Elf64_Half	e_shnum;		/* 节头表中的项数。e_shentsize和e_shnum的积指定了节头表的大小（ */
  Elf64_Half	e_shstrndx;		/* 与节名称字符串表关联的项的节头表索引。 */
} Elf64_Ehdr;


typedef struct
{
  Elf64_Word	st_name;		//符号名称,字符串tbl索引(字符串tbl索引+偏移量)
  unsigned char	st_info;		/* Symbol type and binding */
  unsigned char st_other;		/* Symbol visibility */
  Elf64_Section	st_shndx;		//符号所在段的索引
  Elf64_Addr	st_value;		/* Symbol value */
  Elf64_Xword	st_size;		/* Symbol size */
} Elf64_Sym;



typedef struct
{
  Elf64_Word	sh_name;		/* 节的名称。此成员值是节头字符串表节的索引 */
  Elf64_Word	sh_type;		/* 用于将节的内容和语义分类。*/
  Elf64_Xword	sh_flags;		/* 节可支持用于说明杂项属性 */
  Elf64_Addr	sh_addr;		/* 如果节显示在进程的内存映像中，则此成员会指定节的第一个字节所在的地址。 */
  Elf64_Off	sh_offset;		    /* 从文件的起始位置到节中第一个字节的字节偏移 */
  Elf64_Xword	sh_size;		/* 节的大小（以字节为单位）。 */
  Elf64_Word	sh_link;		/* 节头表索引链接，其解释依赖于节类型 */
  Elf64_Word	sh_info;		/* 额外信息，其解释依赖于节类型。 */
  Elf64_Xword	sh_addralign;		/*一些节具有地址对齐约束 */
  Elf64_Xword	sh_entsize;		/* 一些节包含固定大小的项的表，如符号表。 */
} Elf64_Shdr;

typedef struct ELF_buf{
    void *data;
    void *ptr;
    size_t ind;
    size_t data_all;//已分配
}
ELF_buf;

typedef struct ELF_off{
    char *file;
    int **off;
    int num;
    int fd;
    struct ELF_off *next;
}
ELF_off;

void *elf_realloc(void *data,size_t size, size_t *size_all);
void *w_ELF(ELF_buf *p,size_t size);
void *write_elf(ELF_buf *p,void *p1,size_t size);
void dynarray_add(void *ptab, int *nb_ptr, void *data);
ELF_off *load_object_file(Ar_State *s1, char *filename);
void *write_elf_i(ELF_buf *p,int p1);
ELF_buf *index, *name;


//sh_type（节类型）的合法值。
#define SHT_NULL	  0		/* Section header table entry unused */
#define SHT_PROGBITS	  1	//本节所含有的信息的格式和含义都由程序来决定
#define SHT_SYMTAB	  2		/* Symbol table */
#define SHT_STRTAB	  3		/* String table */
#define SHT_RELA	  4		/* 带附加项的重定位条目,目标文件可以有多个重定位节 */
#define SHT_HASH	  5		/* Symbol hash table */
#define SHT_DYNAMIC	  6		/* Dynamic linking information */
#define SHT_NOTE	  7		/* Notes */
#define SHT_NOBITS	  8		/* bss,这一节的内容是空的,节并不占用实际的空间 */
#define SHT_REL		  9		/* Relocation entries, no addends */
#define SHT_SHLIB	  10		/* Reserved */
#define SHT_DYNSYM	  11		/* Dynamic linker symbol table */




#define ELF32_ST_BIND(val)		(((unsigned char) (val)) >> 4)
#define ELF32_ST_TYPE(val)		((val) & 0xf)
#define ELF32_ST_INFO(bind, type)	(((bind) << 4) + ((type) & 0xf))

/* Both Elf32_Sym and Elf64_Sym use the same one-byte st_info field.  */
#define ELF64_ST_BIND(val)		ELF32_ST_BIND (val)
#define ELF64_ST_TYPE(val)		ELF32_ST_TYPE (val)
#define ELF64_ST_INFO(bind, type)	ELF32_ST_INFO ((bind), (type))


#define STT_NOTYPE	0		/* Symbol type is unspecified */
#define STT_OBJECT	1		/* Symbol is a data object */
#define STT_FUNC	2		/* Symbol is a code object */
#define STT_SECTION	3		/* Symbol associated with a section */
#define STT_FILE	4		/* Symbol's name is file name */
#define STT_COMMON	5		/* Symbol is a common data object */
#define STT_TLS		6		/* Symbol is thread-local data object*/
#define	STT_NUM		7		/* Number of defined types.  */
#define STT_LOOS	10		/* Start of OS-specific */
#define STT_GNU_IFUNC	10		/* Symbol is indirect code object */
#define STT_HIOS	12		/* End of OS-specific */
#define STT_LOPROC	13		/* Start of processor-specific */
#define STT_HIPROC	15		/* End of processor-specific */


void ar_print_log(int level,char *fmt, va_list ap);
void ar_error(char *fmt, ...);
void ar_warning(char *fmt, ...);
void ar_init_error(char *fmt, ...);


int load_archive(Ar_State *s1, int fd);
int load_alacarte(Ar_State *s1, int fd, int size, int entrysize);


Ar_State *new_arState();
void *ar_malloc(size_t size);
__inline void ar_free(void *ptr);

#endif
