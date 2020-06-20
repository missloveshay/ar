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
    char ar_size[11];//�ļ���С
    char ar_name[17];//ȡ���ļ���
    char magic[8];//��ȡħ��
    int size, len, i;
    unsigned long file_offset;

    lseek(fd, 0, SEEK_SET);

    //�����Ѿ�������ħ��
    read(fd, magic, sizeof(magic));

    for(;;) {
        len = read(fd, &hdr, sizeof(hdr));//��ȡǩ��
        if (len == 0)
            break;
        if (len != sizeof(hdr)) {//��С��һ�±���
            ar_error("��Ч�Ĺ鵵�ļ�");
        }
        memcpy(ar_size, hdr.ar_size, sizeof(hdr.ar_size));
        ar_size[sizeof(hdr.ar_size)] = '\0';
        size = strtol(ar_size, NULL, 0);//���ַ���������ת������,�ļ���Ա��С
        memcpy(ar_name, hdr.ar_name, sizeof(hdr.ar_name));
        for(i = sizeof(hdr.ar_name) - 1; i >= 0; i--) {//������������Ա,��������
            if (ar_name[i] != ' ')
                break;
        }
        ar_name[i + 1] = '\0';
        file_offset = lseek(fd, 0, SEEK_CUR);//���ص�ǰ�����ļ���ƫ����
        //������ż��
        size = (size + 1) & ~1;//+1��Ϊ����Ը��������,Ҳ��Ը����ʧ���ݾ��ȡ��������Բ���-1
        if (!strcmp(ar_name, "/")) {
            //coff���ű�������������Ա
            return load_alacarte(s1, fd, size, 4);//size�洢���ļ���Ա��С
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

//��С�����ȡ4���ֽ�
int get_be32(const uint8_t *b)
{
    return b[3] | (b[2] << 8) | (b[1] << 16) | (b[0] << 24);
}

//��С�����ȡ8���ֽ�
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
        ar_error("�����ļ�ʧ��");
    write(fd1, data, size);
    close(fd1);
    ar_free(data);
}

//�����ؽ���δ������ŵĶ���
int load_alacarte(Ar_State *s1, int fd, int size, int entrysize)
{
    long i, bound, nsyms, sym_index, off, ret, len, size1;
    uint8_t *data;
    const char *ar_names, *p;
    const uint8_t *ar_index;
    Elf64_Sym *sym;
    Elf64_Ehdr ehdr;
    char ar_size[11];//��Ҫȡ���ļ���
    char ar_name[17];//�ļ���С

    ArchiveHeader hdr;

    data = ar_malloc(size);//�����ļ���С
    if (read(fd, data, size) != size)//��ȡ�˳�Ա����,��һ������Ŀ���ļ���ǩ��������
        return -1;
    for(;;){
        len = read(fd, &hdr, sizeof(hdr));//��ȡǩ��
        if(len == 0 || len != sizeof(hdr))
            break;
        if (len == 0)
            ar_error("error");
        if (len != sizeof(hdr)) //��С��һ�±���
            ar_error("invalid archive");
        memcpy(ar_size, hdr.ar_size, sizeof(hdr.ar_size));
        ar_size[sizeof(hdr.ar_size)] = '\0';
        size1 = strtol(ar_size, NULL, 0);//���ַ���������ת������,�ļ���Ա��С
        memcpy(ar_name, hdr.ar_name, sizeof(hdr.ar_name));
        for(i = sizeof(hdr.ar_name) - 1; i >= 0; i--) {//������������Ա,��������
            if (ar_name[i] != ' ')
                break;
        }
        ar_name[i] = '\0';
        void *fdata = ar_malloc(size1);
        read(fd, fdata, size1);
        ar_write(ar_name, fdata, size1);
    }
    exit(0);
    nsyms = entrysize == 4 ? get_be32(data) : get_be64(data);//��ȡ���ŵĸ���?//50
    ar_index = data + entrysize;//���ź����ݵ���ʼ��ַ
    ar_names = (char *) ar_index + nsyms * entrysize;//ָ������������Ա�ĺ�����

    for(p = ar_names, i = 0; i < nsyms; i++, p += strlen(p)+1) {//��ȡ���еĺ�����
            printf("sym is %s\n",p);
    }
    printf("����%d��sym\n",nsyms);
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
        ar_error("�ļ��޷���");
    s1->fd = i;
    return i;
}


//��ȡħ��,ʶ���ļ�����
int object_type(int fd, Elf64_Ehdr *h)
{
    int size = read(fd, h, sizeof *h);
    if (size == sizeof *h && 0 == memcmp(h, ELFMAG, 4)) {//Ŀ���ļ�
        if (h->e_type == ET_REL)
            return AFF_BINTYPE_REL;//���ض�λ�ļ�
        if (h->e_type == ET_DYN)
            return AFF_BINTYPE_DYN;//�����
    } else if (size >= 8) {
        if (0 == memcmp(h, ARMAG, 8))
            return AFF_BINTYPE_AR;//�鵵�ļ�
    }
    return 0;
}



void *load_data(int fd, unsigned long file_offset, unsigned long size)
{
    void *data;

    data = ar_malloc(size);
    lseek(fd, file_offset, SEEK_SET);//�ƶ����ڱ��ļ�ƫ��
    read(fd, data, size);//��ȡ���еĽ�
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
    if (object_type(fd, &ehdr) != AFF_BINTYPE_REL)//�����ض�λĿ���ļ��򱨴�
        ar_error("�ļ����ǿ��ض�λ�ļ�");

    //shdr�ж�ȡ�˽ڱ�40
    lseek(fd, 0L, SEEK_SET);
    shdr = load_data(fd, ehdr.e_shoff,//�ڱ��ļ�ƫ��
                     sizeof(Elf64_Shdr) * ehdr.e_shnum);//e_shnum�ڵ�����


    //���ضε�����
    sh = &shdr[ehdr.e_shstrndx];//���ַ�����Ľ�����,sh�б������ַ����ڵ�����ͷ
    lseek(fd, 0, SEEK_SET);
    strsec = load_data(fd, sh->sh_offset, sh->sh_size);//��ȡ���ַ����ڵ�����


    symtab = NULL;
    strtab = NULL;
    nb_syms = 0;
    printf("��ʼ%d\n",ehdr.e_shnum);
    for(i = 1; i < ehdr.e_shnum; i++) {//�����ڱ�
        sh = &shdr[i];
        if (sh->sh_type == SHT_SYMTAB) {
            if (symtab) {
                ar_error("object must contain only one symtab");
            }
            nb_syms = sh->sh_size / sizeof(Elf64_Sym);//���ŵ�����
            printf("sym is %d\n",nb_syms);
            lseek(fd, 0, SEEK_SET);
            symtab = load_data(fd,sh->sh_offset, sh->sh_size);//��ȡ�˷��ű����ʼ��ַ
            //sm_table[i].s = symtab_section;//i��Ӧ�˸�Ŀ���ļ��ڵ�λ��
            //�Ѹ�Ŀ���ļ���Ӧ��SectionMergeInfo���ֶ�ָ��symtab

            /* ��ȡ�ַ����� */
            sh = &shdr[sh->sh_link];//sh�����˷��ű���ص��ַ���������ͷ,Ҳ���ǽڱ��е��ַ�����
            lseek(fd, 0, SEEK_SET);
            strtab = load_data(fd, sh->sh_offset, sh->sh_size);//��ȡ���ű���ص��ַ����������
        }
    }

    int strlen1 = 0;
    unsigned char *buf1;
    //indexͷ4���ֽڴ洢�˶��ٸ���Ա
    ELF_off *ar_off = ar_malloc(sizeof(ELF_off));
    ar_off->file = filename;
    ar_off->fd = fd;
    //������ļ���ʶ��
    //�ļ��ƶ�������󣨲���һ�´�С�Ƿ�ȷ����
    //��ȡvoid *data
    //��д�뵽�ļ���
    //-d���,�������ļ���
    //ע�⿪ͷΪ04�ֽ�,��Ҫע������,ע������Ƿ��Ǻ���
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
        strlen1++;//ȷ�����ַ����ĳ���,�����˿��ַ�
        write_elf(name, p1, strlen1);//д���ַ���
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
