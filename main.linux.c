#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

struct fgate
{
    int number;     //done in open_file
    int fin_same;   //done in count_fout
    int fout_same;  //done in count_fout
    int fanout;     //done in count_fout
    char *name;      //done in open_file
    char *output;    //done in open_file
    char *input;    //done in open_file
    struct fgate *next; //done in open_file
};


struct gate_list
{
    char *name;       //done in open_file
    int num_total;   //done in open_file
    int num_in_same; //done in search_fio
    int num_out_same;   //done in search_fio
    struct gate_list *next; //done in open_file
};


int strloc(const char *array, char *s, int n);
void open_file(struct fgate **p, struct gate_list **g, char *file_name);
void search_fio(struct fgate *p, struct gate_list *q,int *f);
int count_fout(struct fgate *p, int *m, int *n);
void out_and_free(struct fgate *p, struct gate_list *q, int *f, int max, char *file_name, int m, int n);
int ainb(const char *a, const char *b);


int main(int argc, char *args[])
{
    if (argc < 2){
        printf("wrong operation!\n");
        return 0;
    }
    int i = 0;
    struct fgate *p = NULL;
    struct gate_list *g = NULL;
    int *fo_list = NULL;
    int pri_num = 0;
    int pro_num = 0;

    for(i = 1; i < argc; i++){
        open_file(&p, &g,args[i]);
        i++;
        int max_fo = 0;
        pri_num = 0;
        pro_num = 0;
        max_fo = count_fout(p,&pri_num,&pro_num);
        fo_list = (int *)malloc(sizeof(int) * (max_fo + 1));
        memset(fo_list, 0, sizeof(int) * (max_fo + 1));
        search_fio(p,g,fo_list);
        out_and_free(p, g, fo_list, max_fo, args[i], pri_num, pro_num);
        free(fo_list);
   }
    printf("Well Done!\n");
    return 0;
}

int strloc(const char *array, char *s, int n)
{
    int i = 0;
    while(i < n){
        if (s[0] == array[i]){
            return i;
        }
        i++;
    }
    return -1;
}

void open_file(struct fgate **p, struct gate_list **g, char *file_name)
{
    FILE *ofile = fopen(file_name,"r");
    if (ofile == NULL)
        return;
    *p = (struct fgate *)malloc(sizeof(struct fgate)); //set the address in the heap
    memset(*p, 0, sizeof(struct fgate));
    *g = (struct gate_list *)malloc(sizeof(struct gate_list));
    memset(*g, 0, sizeof(struct gate_list));
    struct fgate *pg = *p; //for the writing convenient
    struct gate_list *gg = *g;
    char buf[1024] = {0};
    int i = 0;
    size_t len = 0;
    while(!feof(ofile)){
        memset(buf, 0, sizeof(buf));
        fgets(buf, sizeof(buf),ofile);
        len = strlen(buf);
    if (len > 2){
        int n_not = strloc(buf, "#", len); //"#" match
        int n_out = strloc(buf, "=", len); //"=" match
        int n_gate = strloc(buf, "(", len); //"(" match
        int n_in = strloc(buf, ")", len); //")" match

        if (n_not == -1){
            pg->number = i;
            i += 1;
            if (n_out == -1){
                pg->name = (char *)malloc(n_gate + 1);
                memset(pg->name, 0, n_gate + 1);
                strncpy(pg->name, buf, n_gate);
                pg->output = (char *)malloc(n_in - n_gate );
                memset(pg->output, 0, n_in - n_gate );
                if((strncasecmp("input", buf, 5)) == 0){
                    strncpy(pg->output, &buf[n_gate + 1], n_in - n_gate - 1);
                }
                pg->input = (char *)malloc(n_in - n_gate);
                memset(pg->input, 0, n_in - n_gate);
                if((strncasecmp("output", buf, 6)) == 0){
                    strncpy(pg->input, &buf[n_gate + 1], n_in - n_gate - 1);
                }
            }else if (n_out >0){
                pg->output = (char *)malloc(n_out);
                memset(pg->output, 0, n_out);
                strncpy(pg->output, buf, n_out - 1);
                pg->name = (char *)malloc(n_gate - n_out -1);
                memset(pg->name, 0, n_gate - n_out -1);
                strncpy(pg->name, &buf[n_out + 2], n_gate - n_out -2);
                pg->input = (char *)malloc(n_in - n_gate);
                memset(pg->input, 0, n_in - n_gate);
                strncpy(pg->input, &buf[n_gate + 1], n_in - n_gate - 1);

                gg = *g;
                while(gg->name != NULL){
                    if((strncmp(gg->name, &buf[n_out + 2], strlen(gg->name))) == 0){
                        gg->num_total += 1;
                        break;
                    }
                    gg = gg->next;
                }
                if(gg->name == NULL){
                        gg->name = (char *)malloc(n_gate - n_out -1);
                        memset(gg->name, 0, n_gate - n_out -1);
                        strncpy(gg->name, &buf[n_out + 2], n_gate - n_out -2);
                        gg->num_total = 1;
                        gg->next = (struct gate_list *)malloc(sizeof(struct gate_list));
                        memset(gg->next, 0, sizeof(struct gate_list));
                    }
            }
            pg->next = (struct fgate *)malloc(sizeof(struct fgate));
            memset(pg->next, 0, sizeof(struct fgate));
            pg = pg->next;
        }
    }
    }
    fclose(ofile);

}

int count_fout(struct fgate *p, int *m, int *n)
{
    int max = 0;
    int i = 0;
    struct fgate *pc = p;
    while(pc->next){
        if(strlen(pc->output) != 0){
            struct fgate *pl = p;
            while(pl->next){
                i = 0;
                
                if(strlen(pl->input) > 0){                //some changes must be made to run on linux
                i = ainb(pc->output, pl->input);
                }
                pc->fanout += i;
                if (i > 0){
                    if(strncasecmp(pc->name, pl->name, strlen(pc->name)) == 0){
                        pc->fout_same = 1;
                        pl->fin_same = 1;
                    }
                }
            pl = pl->next;
            }
        }else if (strlen(pc->output) == 0){
            *n += 1;
        }
        if(strlen(pc->input) == 0){
            *m += 1;
        }
        if(max < pc->fanout){
             max = pc->fanout;
        }
        pc = pc->next;
    }
    return max;
}

void search_fio(struct fgate *p, struct gate_list *q,int *f)
{
    struct gate_list *ql = q;
    struct fgate *pl = p;
    int *fl = f;
    while(ql->name){
            pl = p;
        while(pl->name){
            if(strncasecmp(ql->name, pl->name, strlen(ql->name)) == 0){
                if(pl->fin_same)
                    ql->num_in_same += 1;
                if(pl->fout_same)
                    ql->num_out_same += 1;
            }
            pl = pl->next;
        }
        ql = ql->next;
    }
    pl = p;
    while(pl->name){
        fl[pl->fanout] += 1;
        pl = pl->next;
    }
}

void out_and_free(struct fgate *p, struct gate_list *q, int *f, int max, char *file_name, int m, int n)
{
    struct fgate *pl = p;
    struct gate_list *ql = q;

    FILE *pfile = fopen(file_name,"a");
    fprintf(pfile,"%d\n",max);
    int i;
    for(i = 0; i < (max + 1); i++){
        fprintf(pfile, "%d\n", f[i]);
    }
    fprintf(pfile, "%d ", m);

    pl = p;
    while(pl->name ){
        if(strlen(pl->input) == 0){
            fprintf(pfile,"%d ", pl->number);
        }
        pl = pl->next;
    }

    fprintf(pfile, "\n");
    fprintf(pfile, "%d ", n);

    pl = p;
    while(pl->name){
        if(strlen(pl->output)== 0){
            fprintf(pfile,"%d ", pl->number);
        }
        pl = pl->next;
    }

    fprintf(pfile, "\n");
    while(ql->name){
        fprintf(pfile,"%s %d %d %d\n", ql->name, ql->num_total, ql->num_out_same, ql->num_in_same);
        ql = ql->next;
    }
    fclose(pfile);
    pl = p;
    while(pl->next){
        if(pl->name)
            free(pl->name);
        if(pl->output)
            free(pl->output);
        if(pl->input)
            free(pl->input);
        struct fgate *tmp;
        tmp = pl->next;
        free(pl);
        pl = tmp;
    }
    ql = q;
    while(ql->next){
        if(pl->name)
            free(pl->name);
        struct gate_list *tmp;
        tmp = ql->next;
        free(ql);
        ql = tmp;
    }
}

int ainb(const char *a, const char *b)
{
    int i = 0;
    char *tmp;
    char *pp;
    pp = (char *)malloc(strlen(b) + 1);
    memset(pp, 0 , strlen(b) + 1);
    strcpy(pp, b);
    tmp = strtok(pp,",");
    if(strcasecmp(a,tmp) == 0){
        i += 1;
    }
    while((tmp = strtok(NULL, ","))){
        if(strcasecmp(a,&tmp[1]) == 0){
            i++;
        }
    }
    free(pp);
    return i;
}












