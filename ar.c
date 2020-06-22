#include "ar.h"
//#include "arlib.c"

const char* ar_options[] = {
	"-v",
	"--version",
	"-h",
	"--help",
    "-x",
    "-t",
    "-r",
    "-p",
	NULL,
};


static const char ar_help[] =
    "Usage: ar [options...] [filename]\n"
    "-v :AR version\n"
    "-h :AR option help\n"
    "-x :解压打包文件\n"
    "-t :查看打包文件的所有内容(可重定位文件)\n"
    "-r :打包文件,第一参数是输出文件名,后面是要打包的文件\n"
    "-p :查看所有函数\n"
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
    *str = p;//选项和参数有空格就是空字符,没有空格就不是空字符
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
        
		for(; ; n++){
            l = ar_options[n];
			if(l==NULL)
				ar_error("error:无效的选项 : \'%s\'", str);//没有找到此选项
			if(!strstart(l ,&str))//不相同就跳过
				continue;
            break;
		}

		switch(n){
			case 0x0: case 0x1:
                return OPT_VERSION;
			case 0x2: case 0x3:
            	return OPT_HELP;
            case 0x4:
                opt++;
                s->file_num++;
                s->filename = argv[opt];
                return OPT_X;
            case 0x5:
                opt++;
                s->file_num++;
                s->filename = argv[opt];
                return OPT_T;
            case 0x6:
                opt++;
                str = argv[opt];
                s->outfile = str;
                opt++;
                while(opt < argc){
                    str = argv[opt];
                    if (str) {
                        parse_add_file(s, str);
                        opt++;
                    }else
                        break;
                }
                return OPT_R;
            case 0x7:
                opt++;
                s->file_num++;
                s->filename = argv[opt];
                return OPT_P;
            default:
                ar_init_error("无法识别的选项");
		}
        opt++;
	}
	
	if(opt != 1)
		return 0;
	
	return OPT_HELP;
}

int main(int argc, char **argv){
    int i, type, fd, ret;
    Elf64_Ehdr ehdr;

    Ar_State *s = new_arState();
    i = parse_option(s, argc, argv);
    optind_init(i);
    
    if (s->file_num <= 0)
        ar_init_error("no inputfile");

    switch(i){
        case OPT_T:{
            ar_t=1;
            fd = ar_open(s);
            type = object_type(fd, &ehdr);
            switch (type) {
                case AFF_BINTYPE_AR:
                ret = load_archive(s, fd);
                break;
            default:
                ar_error("不识别的文件类型");
                break;
            }

        }
        return 0;

        case OPT_X:{
            fd = ar_open(s);
            type = object_type(fd, &ehdr);
            switch (type) {
                case AFF_BINTYPE_AR:
                ret = load_archive(s, fd);
                break;
            default:
                ar_error("invalid filetype");
                break;
            }
        }
            return 0;
        case OPT_R:{
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

            memset(hdr.ar_size, ' ', sizeof(hdr.ar_size));
            itoa(index->ind + name->ind,hdr.ar_size,10);

            write_elf(&outfile, "!<arch>\n", 8);
            // unsigned char uptr = 0xa;
            // write_elf(&outfile, &uptr, 1);
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

            while(head){
            lseek(head->fd, 0L,SEEK_END);
	        long int s=tell(head->fd);
            memset(&hdr, 0, sizeof(ArchiveHeader));
            memset(hdr.ar_name, 32, sizeof(hdr.ar_name));
            memcpy(hdr.ar_name, head->file, strlen(head->file));
            hdr.ar_name[strlen(head->file)] = '/';
            memset(hdr.ar_size, ' ', sizeof(hdr.ar_size));
            itoa(s,hdr.ar_size,10);

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

    int fd1 = open(s->outfile, O_CREAT |O_WRONLY |O_BINARY);
    write(fd1, outfile.data, outfile.ind);
    close(fd1);

    }
            return 0;
        case OPT_P:{
            ar_p=1;
            fd = ar_open(s);
            type = object_type(fd, &ehdr);
            switch (type) {
                case AFF_BINTYPE_AR:
                ret = load_archive(s, fd);
                break;
            default:
                ar_error("不识别的文件类型");
                break;
            }
        }
            return 0;
    }
    return 0;
}
