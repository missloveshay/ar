#include <string.h>
#include <stdlib.h>
#include "ar.h"
void ar_print_log(int level,char *fmt, va_list ap){
    char buf[1024];
    vsprintf(buf,fmt,ap);
    if(level & 0x1){
            printf("%s:error:%s",ar->filename,buf);
            exit(-1);
    }else if(level & 0x2){
        printf("%s:warning:%s",ar->filename,buf);
    }else if(level & 0x4){
        printf("error:%s",buf);
    }
}

void ar_error(char *fmt, ...){
    va_list ap;
    va_start(ap,fmt);
	ar_print_log(0x1, fmt, ap);
    va_end(ap);
}

void ar_init_error(char *fmt, ...){
    va_list ap;
    va_start(ap,fmt);
	ar_print_log(0x4, fmt, ap);
    va_end(ap);
}

void ar_warning(char *fmt, ...){
    va_list ap;
    va_start(ap,fmt);
	ar_print_log(0x4, fmt, ap);
    va_end(ap);
}


void *ar_malloc(size_t size){
	void *ptr = malloc(size);
	if(!ptr)
		ar_error("memory full (malloc)");
	memset(ptr, 0, size);
    return ptr;
}





int load_archive(Ar_State *s1, int fd)
{
    ArchiveHeader hdr;
    char ar_size[11];//文件大小
    char ar_name[17];//取出文件名
    char magic[8];//读取魔数
    int size, len, i;
    unsigned long file_offset;

    lseek(fd, 0, SEEK_SET);

    //跳过已经检查过的魔术
    read(fd, magic, sizeof(magic));

    for(;;) {
        len = read(fd, &hdr, sizeof(hdr));//读取签名
        if (len == 0)
            break;
        if (len != sizeof(hdr)) {//大小不一致报错
            ar_error("无效的归档文件");
        }
        memcpy(ar_size, hdr.ar_size, sizeof(hdr.ar_size));
        ar_size[sizeof(hdr.ar_size)] = '\0';
        size = strtol(ar_size, NULL, 0);//将字符串的数字转化数字,文件成员大小
        memcpy(ar_name, hdr.ar_name, sizeof(hdr.ar_name));
        for(i = sizeof(hdr.ar_name) - 1; i >= 0; i--) {//解析链接器成员,解析名字
            if (ar_name[i] != ' ')
                break;
        }
        ar_name[i + 1] = '\0';
        file_offset = lseek(fd, 0, SEEK_CUR);//返回当前对于文件的偏移量
        //对齐至偶数
        size = (size + 1) & ~1;//+1是为了宁愿增大数据,也不愿意损失数据精度。反正绝对不能-1
        if (!strcmp(ar_name, "/")) {
            //coff符号表：处理链接器成员
            return load_alacarte(s1, fd, size, 4);//size存储了文件成员大小
	    } else if (!strcmp(ar_name, "/SYM64/")) {
            return load_alacarte(s1, fd, size, 8);
        } else {
//            Elf64_Ehdr ehdr;
//            if (object_type(fd, &ehdr) == AFF_BINTYPE_REL) {
//                if (load_object_file(s1, fd, file_offset) < 0)
//                    return -1;
//            }
        }
    lseek(fd, file_offset + size, SEEK_SET);
    }
    return 0;
}

//以小段序读取4个字节
int get_be32(const uint8_t *b)
{
    return b[3] | (b[2] << 8) | (b[1] << 16) | (b[0] << 24);
}

//以小段序读取8个字节
long get_be64(const uint8_t *b)
{
  long long ret = get_be32(b);
  ret = (ret << 32) | (unsigned)get_be32(b+4);
  return (long)ret;
}


__inline void ar_free(void *ptr)
{
    free(ptr);
}


int ar_write(char *filename,void *data, size_t size){
    int fd1 = open(filename,O_CREAT|O_RDWR);
    if(fd1 < 0)
        ar_error("创建文件失败");
    write(fd1, data, size);
    close(fd1);
    ar_free(data);
}

//仅加载解析未定义符号的对象
int load_alacarte(Ar_State *s1, int fd, int size, int entrysize)
{
    long i, bound, nsyms, sym_index, off, ret, len, size1;
    uint8_t *data;
    const char *ar_names, *p;
    const uint8_t *ar_index;
    Elf64_Sym *sym;
    Elf64_Ehdr ehdr;
    char ar_size[11];//主要取出文件名
    char ar_name[17];//文件大小

    ArchiveHeader hdr;

    data = ar_malloc(size);//分配文件大小
    if (read(fd, data, size) != size)//读取了成员内容,下一个则是目标文件的签名和数据
        return -1;
    for(;;){
        len = read(fd, &hdr, sizeof(hdr));//读取签名
        if(len == 0 || len != sizeof(hdr))
            break;
        if (len == 0)
            ar_error("error");
        if (len != sizeof(hdr)) //大小不一致报错
            ar_error("invalid archive");
        memcpy(ar_size, hdr.ar_size, sizeof(hdr.ar_size));
        ar_size[sizeof(hdr.ar_size)] = '\0';
        size1 = strtol(ar_size, NULL, 0);//将字符串的数字转化数字,文件成员大小
        memcpy(ar_name, hdr.ar_name, sizeof(hdr.ar_name));
        for(i = sizeof(hdr.ar_name) - 1; i >= 0; i--) {//解析链接器成员,解析名字
            if (ar_name[i] != ' ')
                break;
        }
        ar_name[i] = '\0';
        void *fdata = ar_malloc(size1);
        read(fd, fdata, size1);
        ar_write(ar_name, fdata, size1);
    }
    exit(0);
    nsyms = entrysize == 4 ? get_be32(data) : get_be64(data);//读取符号的个数?//50
    ar_index = data + entrysize;//符号和数据的起始地址
    ar_names = (char *) ar_index + nsyms * entrysize;//指向了链接器成员的函数名

    for(p = ar_names, i = 0; i < nsyms; i++, p += strlen(p)+1) {//读取所有的函数名
            printf("sym is %s\n",p);
    }
    printf("共计%d个sym\n",nsyms);
    ret = 0;
 the_end:
 
    ar_free(data);
    return ret;
}


Ar_State *new_arState(){
    Ar_State *s = ar_malloc(sizeof(Ar_State));
    index = (ELF_buf *)ar_malloc(sizeof(ELF_buf));
    name = (ELF_buf *)ar_malloc(sizeof(ELF_buf));
    write_elf(index, "\0\0\0\0", 4);
    return s;
}

int ar_open(Ar_State *s1){
    int i = open(s1->filename, O_RDONLY | O_BINARY);
    if(i < 0)
        ar_error("文件无法打开");
    s1->fd = i;
    return i;
}


//读取魔数,识别文件类型
int object_type(int fd, Elf64_Ehdr *h)
{
    int size = read(fd, h, sizeof *h);
    if (size == sizeof *h && 0 == memcmp(h, ELFMAG, 4)) {//目标文件
        if (h->e_type == ET_REL)
            return AFF_BINTYPE_REL;//可重定位文件
        if (h->e_type == ET_DYN)
            return AFF_BINTYPE_DYN;//共享库
    } else if (size >= 8) {
        if (0 == memcmp(h, ARMAG, 8))
            return AFF_BINTYPE_AR;//归档文件
    }
    return 0;
}



void *load_data(int fd, unsigned long file_offset, unsigned long size)
{
    void *data;

    data = ar_malloc(size);
    lseek(fd, file_offset, SEEK_SET);//移动到节表文件偏移
    read(fd, data, size);//读取所有的节
    return data;
}

ELF_off *load_object_file(Ar_State *s1, char *filename)
{
    Elf64_Ehdr ehdr;
    Elf64_Shdr *shdr, *sh;
    int size, i, j, offset, offseti, nb_syms, sym_index, ret;
    unsigned char *strsec, *strtab;
    char *sh_name;
    Elf64_Sym *sym, *symtab;

    int stab_index;
    int stabstr_index;

    stab_index = stabstr_index = 0;
    int fd = ar_open(s1);
    memset(&ehdr, 0 ,sizeof(Elf64_Ehdr));
    lseek(fd, 0L, SEEK_SET);
    if (object_type(fd, &ehdr) != AFF_BINTYPE_REL)//不是重定位目标文件则报错
        ar_error("文件不是可重定位文件");

    //shdr中读取了节表，40
    lseek(fd, 0L, SEEK_SET);
    shdr = load_data(fd, ehdr.e_shoff,//节表文件偏移
                     sizeof(Elf64_Shdr) * ehdr.e_shnum);//e_shnum节的数量


    //加载段的名字
    sh = &shdr[ehdr.e_shstrndx];//节字符串表的节索引,sh中保存了字符串节的数据头
    lseek(fd, 0, SEEK_SET);
    strsec = load_data(fd, sh->sh_offset, sh->sh_size);//读取了字符串节的数据


    symtab = NULL;
    strtab = NULL;
    nb_syms = 0;
    printf("开始%d\n",ehdr.e_shnum);
    for(i = 1; i < ehdr.e_shnum; i++) {//遍历节表
        sh = &shdr[i];
        if (sh->sh_type == SHT_SYMTAB) {
            if (symtab) {
                ar_error("object must contain only one symtab");
            }
            nb_syms = sh->sh_size / sizeof(Elf64_Sym);//符号的数量
            printf("sym is %d\n",nb_syms);
            lseek(fd, 0, SEEK_SET);
            symtab = load_data(fd,sh->sh_offset, sh->sh_size);//读取了符号表的起始地址
            //sm_table[i].s = symtab_section;//i对应了该目标文件节的位置
            //把该目标文件对应的SectionMergeInfo的字段指向symtab

            /* 读取字符串节 */
            sh = &shdr[sh->sh_link];//sh保存了符号表相关的字符串表数据头,也就是节表中的字符串节
            lseek(fd, 0, SEEK_SET);
            strtab = load_data(fd, sh->sh_offset, sh->sh_size);//读取符号表相关的字符串表的数据
        }
    }

    int strlen1 = 0;
    unsigned char *buf1;
    //index头4个字节存储了多少个成员
    ELF_off *ar_off = ar_malloc(sizeof(ELF_off));
    ar_off->file = filename;
    ar_off->fd = fd;
    //管理好文件标识符
    //文件移动到了最后（测试一下大小是否确定）
    //读取void *data
    //再写入到文件内
    //-d打包,随后紧跟文件名
    //注意开头为04字节,还要注意段序号,注意代码是否是函数
    char *p, *p1;
    sym = symtab + 1;
    printf("name:\n");
    for(i = 1; i < nb_syms; i++, sym++){
        if(ELF64_ST_TYPE(sym->st_info) != STT_FUNC)
            continue;
        sym_num++;
        p1 = p = (char *) strtab + sym->st_name;
        int *off1;
        while(*p++!='\0')
            strlen1++;
        strlen1++;//确定了字符串的长度,算上了空字符
        write_elf(name, p1, strlen1);//写入字符串
        off1 = write_elf_i(index, 0);
        dynarray_add(&ar_off->off,&ar_off->num,off1);
        strlen1 = 0;
    }
    return ar_off;

}


void dynarray_add(void *ptab, int *nb_ptr, void *data)
{
    int nb, nb_alloc;
    void **pp;

    nb = *nb_ptr;
    pp = *(void ***)ptab;
    /* every power of two we double array size */
    if ((nb & (nb - 1)) == 0) {
        if (!nb)
            nb_alloc = 1;
        else
            nb_alloc = nb * 2;
        pp = realloc(pp, nb_alloc * sizeof(void *));
        *(void***)ptab = pp;
    }
    pp[nb++] = data;
    *nb_ptr = nb;
}

void *elf_realloc(void *data,size_t size, size_t *size_all){
    int len = *size_all;
    if(len == 0)
        len = 1024;
    while(size >= len)
        len *=2;
    *size_all = len;
    return realloc(data,len);
}

void *w_ELF(ELF_buf *p,size_t size){
    unsigned char *ptr;
    if((p->ind + size) >= (p->data_all)){
        ptr = elf_realloc(p->data, p->ind + size, &p->data_all);
        p->data = ptr;
        return ptr + p->ind;
    }
    return ((unsigned char *)p->data + p->ind);
}

void *write_elf(ELF_buf *p,void *p1,size_t size){
	unsigned char *ptr= w_ELF(p, size);
	memcpy(ptr, p1, size);
	p->ind += size;
    return ptr;
}

void *write_elf_i(ELF_buf *p,int p1){
	int *ptr= w_ELF(p, 4);
    *ptr = p1;
	p->ind += 4;
    return ptr;
}
