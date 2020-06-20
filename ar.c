#include "ar.h"
#include "arlib.c"



const char* ar_options[] = {
	"-v",
	"--version",
	"-h",
	"--help",
	NULL,
};


static const char ar_help[] =
    //"VCC Compiler "VCC_VERSION" \n"
    "Usage: ar [options...] [filename]\n"
    "-v :AR version\n"
    "-h :AR option help\n"
    "-x :��ѹȫ���ض�λ�ļ�\n"
    "-t :�鿴���ض�λ�ļ�\n"
	;

static __inline void optind_init(int opt){
	if(opt & OPT_HELP){
		printf(ar_help);
		exit(0);
	}
	if(opt & OPT_VERSION){
		printf("AR"AR_VERSION"\tx86-64");
		exit(0);
	}
}

static void parse_add_file(Ar_State *s, const char *filename){
    char *p = ar_malloc(strlen(filename));
    strcpy(p, filename);
    dynarray_add(&s->file,&s->file_num,p);
}

static int strstart(const char *val, const char **str)
{
    const char *p, *q;
    p = *(char **)str;
    q = val;
    while (*q) {
        if (*p != *q)
            return 0;
        p++;
        q++;
    }
    *str = p;//ѡ��Ͳ����пո���ǿ��ַ�,û�пո�Ͳ��ǿ��ַ�
    return 1;
}

static int parse_option(Ar_State *s, int argc, char **argv){
	int i, opt = 1;
	const char *str, *nextopt, *l;
    int n, only = 0;
	while(opt < argc){
        n = 0;
		str = argv[opt];
        if (*str != '-') {
            parse_add_file(s, str);
            opt++;
            only++;
        	continue;
        }
        
		for(l = ar_options[n]; ; n++){
			if(l==NULL)
				ar_error("error:��Ч��ѡ�� : \'%s\'", str);//û���ҵ���ѡ��

			if(!strstart(l ,&str))//����ͬ������
				continue;
            break;
		}

		switch(n){
			case 0x1: case 0x2:
                return OPT_VERSION;
			case 0x3: case 0x4:
            	return OPT_HELP;
		}
        opt++;
	}
	
	if(opt != 1)//
		return 0;
	
	return OPT_HELP;
}

int main(int argc, char **argv){
    int i, type, fd, ret;
    Elf64_Ehdr ehdr;

    Ar_State *s = new_arState();
    i = parse_option(s, argc, argv);
    optind_init(i);
    s->outfile="a.a";
    
    // if (!s->filename)
    //     ar_init_error("no inputfile");
#ifdef AR_DEBUG
		printf("��ǰ�����ļ�:%s\n",s->filename);
#endif
    
    //  fd = ar_open(s);
    
    // type = object_type(fd, &ehdr);
    // switch (type) {
    //     case AFF_BINTYPE_REL:
    //         // ret = load_object_file(s, fd, 0);
    //         break;
    //     case AFF_BINTYPE_AR:
    //         ret = load_archive(s, fd);
    //         break;
    //     default:
    //         ar_error("��ʶ����ļ�����");
    //         break;
    //     }
    ELF_off *ptr = ar_malloc(sizeof(ELF_off));
    ELF_off *head = ptr;
    for(i=0; i<s->file_num; i++){
        s->filename = s->file[i];
        ptr->next = load_object_file(s, s->filename);
        ptr = ptr->next;
    }
    ptr->next = NULL;
    head = head->next;
    ptr = head;

    ArchiveHeader hdr;ELF_buf outfile;
    memset(&hdr, 0, sizeof(ArchiveHeader));
    memset(&outfile, 0, sizeof(ELF_buf));
    memset(hdr.ar_name, 32, sizeof(hdr.ar_name));
    memcpy(hdr.ar_name, "/",1);
    *(int *)index->data = sym_num;
    //char *str = atoi(index->ind + name->ind);
    itoa(index->ind + name->ind,hdr.ar_size,10);
    //memcpy(hdr.ar_size,str,sizeof(str) );//&hdr.ar_size���ֶο���ҲҪ��0

    write_elf(&outfile, &hdr, sizeof(ArchiveHeader));

    int cv;
    while(ptr){
        for(cv=0;cv<ptr->num;cv++){
            *ptr->off[cv] = outfile.ind + name->ind + index->ind;
        }
        ptr = ptr->next;
    }
    ptr = head;

    write_elf(&outfile, index->data, index->ind);
    write_elf(&outfile, name->data, name->ind);

    printf("ind size is %d\n",outfile.ind);
    printf("size is %d\n",sizeof(ArchiveHeader) + name->ind + index->ind);

    while(head){
        printf("aaa");
        lseek(head->fd, 0L,SEEK_END);
	    long int s=tell(head->fd);
        memset(&hdr, 0, sizeof(ArchiveHeader));
        memset(hdr.ar_name, 32, sizeof(hdr.ar_name));
        memcpy(hdr.ar_name, head->file, strlen(head->file));
        itoa(s,hdr.ar_size,10);
//        str = stoi(s);//&hdr.ar_size���ֶο���ҲҪ��0,�����ǿ��ַ����������ַ�
//        memcpy(hdr.ar_size,str,sizeof(str));

        write_elf(&outfile, &hdr, sizeof(ArchiveHeader));

        void *data = ar_malloc(s);
        lseek(head->fd, 0L,SEEK_SET);
        read(head->fd,data,s);
        write_elf(&outfile, data, s);
        ar_free(data);
        close(head->fd);
        
        ar_free(head);
        head = head->next;
    }

    int fd1 = open(s->outfile,O_CREAT|O_RDWR);
    printf("\noutfile.ind is %d",outfile.ind);
    write(fd1, outfile.data, outfile.ind);
    close(fd1);
}
