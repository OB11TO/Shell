/* SHELL */


/*


  Требуется ВВОДИТЬ команды типа ls | wc > log ; pwd & через пробел.

*/

/*ошибки  					+	  */
/* точка с запятой  		+	  */
/*коментарии   				+	  */
/*перенаправление ввода   	+ 	  */
/*сигналы   				+	  */
/*команда cd  				+	  */
/*команда pwd   			+	  */
/*задний фон                +     */
/*конвеер                   +     */
/*подстановка значений      +     */
/*текст в кавычках			+	  */
/*запуск из истории 		+	  */
/*экранирование символа     +    */
/*программы джобс фг и бг   +-    */




#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>
#include <termios.h>
#include <sys/wait.h>
#include <fcntl.h>



#define ERR_MES1 "ERR can't get memory.\n"
#define ERR_MES2 " ERR in Memory  \n"

#define ERR_COS_EMTY_STR         -1
#define EXIT                     0
#define STRGOESAFTER             2
#define COMM_MEAN_EXIT_STR       5
#define DOUBLESLASH              6
#define TWOCOV                    7
#define ONECOV                    8
#define READ                      9
#define WRITE                     10
#define DOUBLEREAD                11




void setdeafault();
void chistka_komandnoi_stroki(char **);
void chistka_massiva_strok (char ***);
void chistka_jobs();
void chistka_vsego();
void *zamena_znachenii(char *);
char *schitka_stroki();
char **zapusk_job(char *);
char *extend();
void get_function();
char **free_changes_read_write (char **);
int esli_izmenenia (char**, int);
int izmenenia (char **);
void dobavlenie_v_job ();
void realizasia_konveera ();
char *komandi();

char *directory=NULL;
int count_of_prog=0;
char*fakestr=NULL;
char **in_command;
int flexpandstr=0;
int su=0;
char *str;
int i,flag=100;
int count_of_char=0;
int a=0;
int fldollar=0;
int flexit=0;
int fl_for_continue_string=0;
int flalready=0;
int fon_proc_count=0;
uid_t uid;
pid_t pid1;
pid_t fonov_pid[1000];
int back_count;

/*структура для записи программы*/
struct program
{
    int prog_status;
    int number_of_arguments;
    char **arguments;
} ;

/*структура для конвеера*/
typedef struct
{
  pid_t pid;
    int background;
    char **program;
    char ***conv;
    int number_of_programs;
} job;

int count_of_shell_jobs = 0;
job *shell_jobs = NULL;
struct program shellprogram;
pid_t pid;
int flerror=0;
int end_of_file = 0;


/* очищение памяти для команд и др элементов */
void chistka_komandnoi_stroki(char **commands)
{
    int k=0;
    if(commands!=NULL)
    {
        while(commands[k]!=NULL)
        {
            free(commands[k++]);
            commands[k-1]=NULL;
        }
    }
}
void setdeafault()
{
    fl_for_continue_string=0;
    flag=100;
    fldollar=0;

}


void chistka_massiva_strok (char ***str)
{
    int i = 0;


    if (str != NULL)
    {
        while (str[i] != NULL)
        {
            free(str[i]);
            str[i++] = NULL;
        }

        free(str);
        str = NULL;
    }

    return;
}

void chistka_jobs ()
{
    int i = 0;
    if(shell_jobs!=NULL)
        for (i = 0; i < count_of_shell_jobs; i++)
        {
            free(shell_jobs[i].program);
            shell_jobs[i].program = NULL;
            chistka_massiva_strok(shell_jobs[i].conv);
        }

    if (shell_jobs != NULL)
        free(shell_jobs);

    shell_jobs = NULL;
    return;
}

void chistka_vsego ()
{
    free(directory);
    if (count_of_shell_jobs != 0)
        chistka_jobs();
    return;
}
void hdl(int sig)
{
    printf("THE SIGNAL\n ");
    signal(SIGTSTP, hdl);
}
/* епроисходит замена значения */
void *zamena_znachenii(char *buf)
{
    char *substr=NULL;
    char *buffer=NULL;
    int chislo_dlaya_podstavl = -1, i=0;
    chislo_dlaya_podstavl=buf[1]-'0';

	for(i=0; buf[i]!='\0';i++)
	{
    if ((strcmp(buf, "$HOME") == 0) || (strcmp(buf, "${HOME}") == 0))
    {
        buffer = malloc((strlen(getenv("HOME"))+3)*sizeof(char));
        strcpy(buffer, getenv("HOME"));
    }
    if ((strcmp(buf, "$PWD") == 0) || (strcmp(buf, "${PWD}") == 0))
    {
        buffer = malloc((strlen(getenv("PWD"))+3)*sizeof(char));
        strcpy(buffer, getenv("PWD"));
    }
    if ((strcmp(buf, "$USER") == 0) || (strcmp(buf, "${USER}") == 0))
    {
        buffer = malloc((strlen(getenv("USER"))+3)*sizeof(char));
        strcpy(buffer, getenv("USER"));
    }
    if ((chislo_dlaya_podstavl >= 0) && (chislo_dlaya_podstavl <=9) && (buf[2]=='\0'))
    {
        if (chislo_dlaya_podstavl <= shellprogram.number_of_arguments)
        {
            buffer = malloc((strlen(shellprogram.arguments[chislo_dlaya_podstavl-1])+3)*sizeof(char));
            strcpy(buffer, shellprogram.arguments[chislo_dlaya_podstavl-1]);
        }
        else printf("This argument is not specified\n");
    }
    if ((strcmp(buf, "$PID") == 0) || (strcmp(buf, "${PID}") == 0))
    {
        buffer = malloc(10*sizeof(char));
        sprintf(buffer, "%d", pid);
    }
    if ((strcmp(buf, "$UID") == 0) || (strcmp(buf, "${UID}") == 0))
    {
        buffer = malloc(10*sizeof(char));
        sprintf(buffer, "%d",uid);
    }

    if (strcmp(buf, "$#") == 0)
    {
        buffer = malloc(10*sizeof(char));
        sprintf(buffer, "%d",shellprogram.number_of_arguments);
    }

    if ((strcmp(buf, "$SHELL") == 0) || (strcmp(buf, "${SHELL}") == 0))
    {

        buffer=malloc((strlen(directory)+2)*sizeof(char));
        strcpy(buffer,directory);

    }
   /*if ((strcmp(buf, "$FILE_NAME") == 0) || (strcmp(buf, "${SHELL}") == 0))
    {
        buffer = malloc((5)*sizeof(char));
        strcpy(buffer,"log");
    }*/
    if (strcmp(buf, "$?") == 0)
    {
        buffer = malloc(10*sizeof(char));
        sprintf(buffer, "%d", shellprogram.prog_status);
    }
    if (substr!=NULL) free(substr);
    }
	return buffer;
}


/*получаем строку + если что считываем еще одну если \ в конце*/
char *schitka_stroki()
{
    char *tempstr=NULL;
    char *stringcheck=NULL;
    char ch;
    int j=0,i=0;
    int flnext=0;
    int  count_buf=0;
    signal(SIGTSTP, hdl);
    if (flag==STRGOESAFTER)   count_buf=count_of_char;
    count_of_char=0;
    tempstr=(char*)malloc(4*(sizeof(char)));
    if(tempstr==NULL)
    {
        fprintf(stderr,ERR_MES1);
        chistka_vsego();
        exit(-1);
    }
    while(flnext==0)
    {
        ch=getchar();
        if ( (ch!='\n'))
        {
            if(ch==EOF)
            {
                fprintf(stderr," Нажали CTRL+D. Выход из программы. \n");
                free(tempstr);
                chistka_vsego();
                exit(-1);
            }
            stringcheck=(char*)realloc(tempstr,(count_of_char+3)*sizeof(char));
            if(stringcheck==NULL)
            {
                fprintf(stderr,ERR_MES1);
                chistka_vsego();
                exit(-1);
            }
            tempstr=stringcheck;
            tempstr[j]=ch;
            j++;
            tempstr[j]='\0';

            count_of_char++;
        }
        else
        {
            flnext=1;
        }
    }
    if (ch=='\n')
    {
        tempstr[j]='\n';
        j++;
        tempstr[j]='\0';
    }
    if(tempstr[0]=='\n')
    {
        free(tempstr);
        return (char*)ERR_COS_EMTY_STR;
    }
    while (tempstr[j-2]=='\\')
    {
        flnext=0;
        j=j-2;
        fakestr=malloc((strlen(tempstr)+2)*sizeof(char));
        fakestr=strcpy(fakestr,tempstr);
        tempstr[j]='\0';

        printf(">");
        while(flnext==0)
        {
            ch=getchar();
            if ( (ch!='\n'))
            {
                if(ch==EOF)
                {
                    fprintf(stderr," Нажали CTRL+D. Выход из программы. \n");
                    free(tempstr);
                    chistka_vsego();
                    exit(-1);
                }
                stringcheck=(char*)realloc(tempstr,(count_of_char+3)*sizeof(char));
                if(stringcheck==NULL)
                {
                    fprintf(stderr,ERR_MES1);
                    chistka_vsego();
                    exit(-1);
                }
                tempstr=stringcheck;
                tempstr[j]=ch;
                j++;
                tempstr[j]='\0';

                count_of_char++;

            }
            else
            {
                flnext=1;
            }

        }
        if (ch=='\n')
        {
            tempstr[j]='\n';
            j++;
            tempstr[j]='\0';
        }
        free(fakestr);
    }

    count_of_char+=count_buf;

	for (i=0;tempstr[i]==' ';i++)
	{
	}
	if (i!=0)
	{
		j=0;
		count_buf-=i;
		for (;tempstr[i]!='\0';i++,j++)
			tempstr[j]=tempstr[i];
		tempstr[j]='\0';

	}
	if (tempstr[0]==';' || tempstr[0]=='#' || tempstr[0]=='\'' || tempstr[0]=='\"' || tempstr[0]=='|' || tempstr[0]=='>' || tempstr[0]=='<' || tempstr[0]=='&' || tempstr[strlen(tempstr)]==';')
	{
		puts("Syntax ERROR");
		tempstr[0]='\0';
	}
	return tempstr;

}
/*разделяем строку и перепсываем в массив ** */
char **zapusk_job(char *str)
{

    int j=0;
    int fladd=0;
    int count_buf=0;
    char **prog = NULL;
    char *tempbuf = NULL, *tempbuf1 = NULL,*tempbuf2=NULL;

    tempbuf = (char *) malloc((count_of_char*4)*sizeof(char));
    tempbuf[0]='\0';

    if (flag==STRGOESAFTER)   count_buf=count_of_prog;
    count_of_prog=0;
    flexpandstr=0;
    setdeafault();
    for (i=0; i<=(int)strlen(str); i++)
    {
        if ((str[i]=='\"')&&(flag!=TWOCOV))
        {

            flag=TWOCOV;

        }
        if ((str[i]=='\'')&&(flag!=ONECOV))
        {
            flag=ONECOV;

        }
        if ((str[i]==' ') && ((str[i-1]!='\'') || (str[i-1]!='\"')) && ((flag==ONECOV)|| (flag==TWOCOV)))
        {
            tempbuf[j]=str[i];
            j++;
            tempbuf[j]='\0';
        }

        if ((str[i]!=' ') && (str[i]!='\'') && (str[i]!='\"') && (str[i]!='\\') && (str[i]!='\n') && (str[i]!='$'))
        {
            tempbuf[j]=str[i];
            j++;
            tempbuf[j]='\0';
        }

        if (i>=1)
        {
            if ( (fldollar==1) && ((str[i]==' ') || (str[i]=='\n')))
            {
                fladd=1;
                tempbuf1=zamena_znachenii(tempbuf);
                if (tempbuf1!=NULL)
                {
                    tempbuf= realloc(tempbuf,(strlen(tempbuf1)+3)*sizeof(char));
                    strcpy(tempbuf,tempbuf1);
                }
            }
            if (((str[i]==' ') && (flag!=ONECOV) && (flag!=TWOCOV)) || (((flag==ONECOV)|| (flag==TWOCOV)) && ((str[i-1]=='\'') || (str[i-1]=='\"')) && (str[i]==' '))  || (str[i]=='\n') || (fladd==1))
            {
                prog=(char**)realloc(prog,(++count_of_prog)*sizeof(char*));
                tempbuf2 = (char *) malloc((strlen(tempbuf)+2)*sizeof(char));
                if ( tempbuf2 == NULL)
                {
                    fprintf(stderr, ERR_MES2);
                    chistka_vsego();
                    exit(-1);
                }
                memcpy(tempbuf2,tempbuf,strlen(tempbuf)+2);
                prog[count_of_prog-1] = tempbuf2;

                tempbuf2=NULL;
                if ((flexpandstr==0) && (fladd!=1))  setdeafault();
                j=0;
                tempbuf[j]='\0';

            }
        }
        if ((str[i]=='\\')  && (i<(int)strlen(str)))
        {
            if (str[i+1]=='\\')
            {

                tempbuf[j]=str[i];
                j++;
                tempbuf[j]='\0';

            }

        }

        if ((str[i]=='#') && (str[i-1]!='$'))
        {
            flag=COMM_MEAN_EXIT_STR;
        }
       /* if (i>1)
        {
            if ((str[i]=='$') && (flag==TWOCOV) && (str[i-1]==' '))
            {
                prog=(char**)realloc(prog,(++count_of_prog)*sizeof(char*));
                tempbuf2 = (char *) malloc((strlen(tempbuf)+3)*sizeof(char));
                if ( tempbuf2 == NULL)
                {
                    fprintf(stderr, ERR_MES2);
                    chistka_vsego();
                    exit(-1);
                }
                memcpy(tempbuf2,tempbuf,strlen(tempbuf)+2);
                prog[count_of_prog-1] = tempbuf2;

                tempbuf2=NULL;

                j=0;
                tempbuf[j]='\0';
                tempbuf[j]=str[i];
                j++;
                tempbuf[j]='\0';

            }
        }*/
        if (str[i]=='$')
        {
            if (flag==TWOCOV)
            {
                fldollar=1;
            }
            tempbuf[j]='$';
            j++;
            tempbuf[j]='\0';

        }

        if(tempbuf1!=NULL)
        {
            free(tempbuf1);
            tempbuf1=NULL;
        }
        if (flag==COMM_MEAN_EXIT_STR) break;
    }
    prog = (char **) realloc(prog, ++count_of_prog*sizeof(char *));
    if (prog == NULL)
    {
        fprintf(stderr, ERR_MES2);
        chistka_vsego();
        exit(-1);
    }
    prog[count_of_prog-1] = NULL;

    free(tempbuf);

    count_of_prog+=count_buf;

    return prog;
}


char **free_changes_read_write (char **prog)
{
    int  arg_count = 0, new_size = 0;
    char **tmpprog = NULL;
    i = 0;

    for (arg_count = 0; prog[arg_count] != NULL; arg_count++);

    while (i != arg_count)
    {
        if ((strcmp(prog[i], "<") == 0) || (strcmp(prog[i], ">") == 0) || (strcmp(prog[i], ">>") == 0))
        {
            i++;
        }
        else
        {
            tmpprog = (char **) realloc(tmpprog, (++new_size+1)*sizeof(char *));
            if (tmpprog == NULL)
            {
                fprintf(stderr, "Memory error\n");
                chistka_vsego();
                exit(-1);
            }
            tmpprog[new_size-1] = prog[i];
            tmpprog[new_size] = NULL;
        }

        i++;
    }

    return tmpprog;
}

int esli_izmenenia (char **prog, int status)
{
    int fd = 0;
    i = 0;


    while (prog[i] != NULL)
    {
        if ((status == READ) && (strcmp(prog[i], "<") == 0))
        {
            if (prog[i+1] == NULL)
            {
                fprintf(stderr, "syntax error near '<'\n");
                return -1;
            }
            else if ((fd = open(prog[i+1], O_RDONLY)) == -1)
            {
                perror("input");
                return -1;
            }
            else
                return fd;
        }
        else if ((status == WRITE) && (strcmp(prog[i], ">") == 0))
        {
            if (prog[i+1] == NULL)
            {
                fprintf(stderr, "syntax error near '>'\n");
                return -1;
            }
            else if ((fd = open(prog[i+1], /*O_TRUNC*/  O_WRONLY | O_CREAT, 0666)) == -1)
            {
                perror("output");
                return -1;
            }
            else
                return fd;
        }
        else if ((status == WRITE) && (strcmp(prog[i], ">>") == 0))
        {
            if (prog[i+1] == NULL)
            {
                fprintf(stderr, "syntax error '>>'\n");
                return -1;
            }
            else if ((fd = open(prog[i+1], O_APPEND | O_WRONLY | O_CREAT, 0666)) == -1)
            {
                perror("output");
                return -1;
            }
            else
                return fd;
        }

        i++;
    }

    return 0;
}

int izmenenia (char **prog)
{
    int fd = 0;

    fd = esli_izmenenia(prog, READ);

    if (fd == -1)
    {
        return -1;
    }

    if (fd != 0)
    {
        close(0);
        dup(fd);
    }

     fd = esli_izmenenia(prog, WRITE);

    if (fd == -1)
    {
        return -1;
    }

    if (fd != 0)
    {
        close(1);
        dup(fd);
    }
    return 0;
}
void dobavlenie_v_job ()
{
    int i = 0, count_of_args = 0;
    char **new_job = NULL;
    count_of_shell_jobs = 0;
    do
    {
        if (((count_of_args != 0) && (in_command[i] == NULL)) || ((in_command[i] != NULL) && (strcmp(in_command[i], ";") == 0)))
        {
            shell_jobs = (job *) realloc(shell_jobs, ++count_of_shell_jobs*sizeof(job));
            if (shell_jobs == NULL)
            {
                fprintf(stderr, ERR_MES2);
                chistka_vsego();
                exit(-1);
            }
            shell_jobs[count_of_shell_jobs-1].background = 0;
            shell_jobs[count_of_shell_jobs-1].program = new_job;
            shell_jobs[count_of_shell_jobs-1].conv = NULL;
            new_job = NULL;
            new_job = (char **) malloc(sizeof(char *));
            if (new_job == NULL)
            {
                fprintf(stderr, ERR_MES2);
                chistka_vsego();
                exit(-1);
            }
            *new_job = NULL;
            count_of_args = 0;
        }
        else if ((in_command[i] != NULL) && (strcmp(in_command[i], "&") == 0))
        {
            shell_jobs = (job *) realloc(shell_jobs, ++count_of_shell_jobs*sizeof(job));
            if (shell_jobs == NULL)
            {
                fprintf(stderr, ERR_MES2);
                chistka_vsego();
                exit(-1);
            }
            shell_jobs[count_of_shell_jobs-1].background = 1;
            shell_jobs[count_of_shell_jobs-1].program = new_job;
            shell_jobs[count_of_shell_jobs-1].conv = NULL;

            new_job = NULL;
            new_job = (char **) malloc(sizeof(char *));
            if (new_job == NULL)
            {
                fprintf(stderr, ERR_MES2);
                chistka_vsego();
                exit(-1);
            }
            *new_job = NULL;
            count_of_args = 0;
        }
        else
        {
            new_job = (char **) realloc(new_job, (++count_of_args+1)*sizeof(char *));
            if (new_job == NULL)
            {
                fprintf(stderr, ERR_MES2);
                chistka_vsego();
                exit(-1);
            }
            new_job[count_of_args-1] = in_command[i];
            new_job[count_of_args] = NULL;
        }
    }
    while (in_command[i++] != NULL);
    free(new_job);
    return;
}


void realizasia_konveera ()
{
    int i = 0, j = 0, count_of_args = 0;
    char **new_prog = NULL;


    for (i = 0; i < count_of_shell_jobs; i++)
    {
        j = 0;
        count_of_args = 0;
        shell_jobs[i].number_of_programs = 0;

        do
        {
            if (((count_of_args != 0) && (shell_jobs[i].program[j] == NULL)) || ((shell_jobs[i].program[j] != NULL) && (strcmp(shell_jobs[i].program[j], "|") == 0)))
            {
                shell_jobs[i].conv = (char ***) realloc(shell_jobs[i].conv, (++shell_jobs[i].number_of_programs+1)*sizeof(char **));
                if (shell_jobs[i].conv == NULL)
                {
                    fprintf(stderr, ERR_MES2);
                    chistka_vsego();
                    exit(-1);
                }
                shell_jobs[i].conv[shell_jobs[i].number_of_programs-1] = new_prog;

                shell_jobs[i].conv[shell_jobs[i].number_of_programs] = NULL;

                new_prog = NULL;
                count_of_args = 0;
            }
            else
            {
                new_prog = (char **) realloc(new_prog, (++count_of_args+1)*sizeof(char *));
                if (new_prog == NULL)
                {
                    fprintf(stderr, ERR_MES2);
                    chistka_vsego();
                    exit(-1);
                }
                new_prog[count_of_args-1] = shell_jobs[i].program[j];
                new_prog[count_of_args] = NULL;
            }

        }
        while (shell_jobs[i].program[j++] != NULL);


    }

    return;
}



void execute (job job,char* string)
{
	int j;
    int  rfd[job.number_of_programs][2], fd = 0, i = 0;
       signal(SIGINT, SIG_DFL);
           signal(SIGTSTP, hdl);
    for (i = 0; i < job.number_of_programs; i++)
    {
        if (job.number_of_programs != 1)
            pipe(rfd[i]);

        switch ((pid = fork()))
        {
            case -1:
            {
                perror("fork");
                return;
            }

            case 0:
            {
                char *str = NULL, **tmpprog = NULL;

                if (job.number_of_programs == 1)
                {
                    if (job.background == 1)
                       {
                            printf("Process № %d launched\n",getpid());
                        }

                    fd = izmenenia(job.conv[i]);

                    if (fd == -1)
                    {
                        in_command = NULL;
                        chistka_vsego();
                        exit(-1);
                    }
                }
                else
                {
                    if (job.background == 1)
                    {
                        printf("Process № %d launched\n",getpid());
                    }
                    if (i == 0)
                    {
                        close(1);
                        dup(rfd[i][1]);
                        close(rfd[i][0]);
                        fd = izmenenia(job.conv[i]);

                        if (fd == -1)
                        {
                            in_command = NULL;
                            chistka_vsego();
                            exit(-1);
                        }
                    }
                    else if (i != job.number_of_programs-1)
                    {
                        close(0);
                        dup(rfd[i-1][0]);
                        close(1);
                        dup(rfd[i][1]);
                        close(rfd[i][0]);
                        fd = izmenenia(job.conv[i]);
                        if (fd == -1)
                        {
                            in_command = NULL;
                            chistka_vsego();
                            exit(-1);
                        }
                    }
                    else if (i == job.number_of_programs-1)
                    {
                        close(0);
                        dup(rfd[i-1][0]);
                        close(rfd[i][1]);
                        close(rfd[i][0]);
                        fd = izmenenia(job.conv[i]);

                        if (fd == -1)
                        {
                            in_command = NULL;
                            chistka_vsego();
                            exit(-1);
                        }
                    }
                }

                tmpprog = free_changes_read_write(job.conv[i]);


                if ((tmpprog[0] != NULL) && (strcmp(tmpprog[0], "pwd") == 0))
                {
                    str=getcwd(NULL,1000);
    				printf("%s\n", str);
                    free(str);
                    str = NULL;
                    free(string);
                    chistka_komandnoi_stroki(in_command);
                    free(in_command);
                    in_command = NULL;

                    free(tmpprog);
                    tmpprog = NULL;
                    chistka_vsego();
                    exit(0);
                }

                else if ((tmpprog[0] != NULL) && (strcmp(tmpprog[0], "jobs") == 0))
                {
                    printf(" HERE fon proc count %d ",back_count);
                    if (back_count == 0)
                    {
                        printf("bash: jobs :none \n" );
                    }
                    else
                    {
                        for (j =0; j<back_count; j++)
                        {
                            printf("Работал процесс с номером %d\n",  fonov_pid[j]);
                        }
                    }
                    free(str);
                    str = NULL;
                    free(string);
                    chistka_komandnoi_stroki(in_command);
                    free(in_command);
                    in_command = NULL;

                    free(tmpprog);
                    tmpprog = NULL;
                    chistka_vsego();
                    exit(0);
                }
                else if ((tmpprog[0] != NULL) && (strcmp(tmpprog[0], "mcat") == 0))
                {
                    int file;
                    int i=0;
                    char symbol, text[1000];
                    if (tmpprog[1]!=NULL)
                    {
                        file=open (tmpprog[1], O_RDWR );
                        if (file == -1)
                            fprintf (stderr, "ERROR: Open file\n");
                        else
                        {
                            dup2(file,0);
                            close(file);
                        }
                    }

                    do
                    {
                        symbol=getchar();
                        sprintf(text+i,"%c",symbol);
                        i++;
                        if (symbol=='\n')
                        {
                            printf("%s", text);
                            text[0]='\0';
                            i=0;
                        }
                    }
                    while(symbol!=EOF);

                    free(string);
                    chistka_komandnoi_stroki(in_command);
                    free(in_command);
                    in_command = NULL;

                    free(tmpprog);
                    tmpprog = NULL;
                    chistka_vsego();
                    exit(-1);
                }
                else if ((tmpprog[0] != NULL) && (strcmp(tmpprog[0], "msed") == 0))
                {
                    int file;
                    int i=0;
                    char symbol, text[1000];
                    if (tmpprog[1]!=NULL && tmpprog[2]!=NULL)
                    {
                        do
                        {
                            symbol=getchar();
                            sprintf(text+i,"%c",symbol);
                            i++;
                            if (symbol=='\n')
                            {
                                char tel[1000];
                                if(strcmp(tmpprog[1],"^")==0)
                                {
                                    sprintf(tel,"%s%s",tmpprog[2],text);
                                    sprintf(text,"%s",tel);
                                }
                                else if (strcmp(tmpprog[1],"$")==0)
                                {

                                    sprintf(text+i-1,"%s\n", tmpprog[2]);
                                }
                                else
                                {
                                    int j =0,k=0;
                                    for (j=0; text[j]!='\n';j++ )
                                    {
                                        if (strncmp(text+j,tmpprog[1],strlen(tmpprog[1]))==0)
                                        {
                                            sprintf(tel,"%s",text+k);
                                            sprintf(tel+j,"%s",tmpprog[2]);
                                            k=j+strlen(tmpprog[2]);
                                            sprintf(tel+k,"%s\n",text+j+strlen(tmpprog[1]));
                                            sprintf(text,"%s",tel);
                                            j=k;
                                        }
                                    }
                                }
                                printf("%s", text);
                                text[0]='\0';
                                i=0;
                            }
                        }
                        while(symbol!=EOF);
                    }
                    else
                    {
                        fprintf (stderr, "ERROR: Malo argumentov\n");
                    }

                    free(string);
                    chistka_komandnoi_stroki(in_command);
                    free(in_command);
                    in_command = NULL;

                    free(tmpprog);
                    tmpprog = NULL;
                    chistka_vsego();
                    exit(-1);
                }
                else if ((tmpprog[0] != NULL) && (strcmp(tmpprog[0], "mgrep") == 0))
                {
                    int file;
                    int i=0;
                    char symbol, text[1000];
                    if (tmpprog[1]!=NULL )
                    {
                        do
                        {
                            symbol=getchar();
                            sprintf(text+i,"%c",symbol);
                            i++;
                            if (symbol=='\n')
                            {
                                char tel[1000];
                                if(strcmp(tmpprog[1],"*")==0)
                                {
                                    /* не понял что это*/
                                }
                                else if (strcmp(tmpprog[1],"+")==0)
                                {

                                    /* не понял что это*/
                                }
                                else
                                {
                                    int j =0,k=0;
                                    for (j=0; text[j]!='\n';j++ )
                                    {
                                        if (strncmp(text+j,tmpprog[1],strlen(tmpprog[1]))==0)
                                            printf("%s", text);
                                    }
                                }
                                text[0]='\0';
                                i=0;
                            }
                        }
                        while(symbol!=EOF);
                    }
                    else
                    {
                        fprintf (stderr, "ERROR: Malo argumentov\n");
                    }

                    free(string);
                    chistka_komandnoi_stroki(in_command);
                    free(in_command);
                    in_command = NULL;

                    free(tmpprog);
                    tmpprog = NULL;
                    chistka_vsego();
                    exit(-1);
                }
                else if (flalready!=1)
                {
                    execvp(tmpprog[0], tmpprog);
                    perror("exec");
                    free(string);
                    chistka_komandnoi_stroki(in_command);
                    free(in_command);
                    in_command = NULL;


                    free(tmpprog);
                    tmpprog = NULL;
                    chistka_vsego();
                    exit(-1);
                }
                return ;
            }
            default:
            {
                if ( job.number_of_programs>1)
                {
                   if ( esli_izmenenia(job.conv[i],WRITE)!=-1  && esli_izmenenia(job.conv[i],WRITE)!=0)
                        close(rfd[i][0]);
                    close(rfd[i][1]);
                }
            }
        }

    }
if (job.background == 0)
{
   for (i = 0; i < job.number_of_programs; i++)
        wait(NULL);
}
if (job.background == 1) {
         fonov_pid[back_count]=getpid();
back_count++;
}
printf(" bsck count %d\n", back_count);
printf(" fon proc count %d \n",fon_proc_count);
    return;
}
void  get_function()
{
    int a;
    flerror=0;
    flalready=0;
    if ((shell_jobs[i].program[0] != NULL) && (strcmp(shell_jobs[i].program[0], "|") == 0))
    {
        printf("bash: syntax error near '|'\n");
        flerror=1;
    }
    a=0;
    while (shell_jobs[i].program[a+1] != NULL)
    {
        if ((strcmp(shell_jobs[i].program[a], "|") == 0) && (strcmp(shell_jobs[i].program[a+1], "|") == 0))
        {
            printf("bash: syntax error near unexpected token `|' \n");
            flerror=1;
        }
        a++;
    }
    if ((shell_jobs[i].program[0] != NULL) && (strcmp(shell_jobs[i].program[0], "cd") == 0))
    {
        if (shell_jobs[i].program[1] != NULL)
        {
            if (strcmp(shell_jobs[i].program[1], "~") == 0)
            {
                if (chdir((getenv("HOME"))) != 0)
                    printf("bash: cd: %s: No such file or directory\n", shell_jobs[i].program[1]);
            }
            else if (chdir(shell_jobs[i].program[1]) != 0)
                printf("bash: cd: %s: No such file or directory\n", shell_jobs[i].program[1]);
        }
        else
        {
            printf("cd:No such file or directory\n");
        }

        flalready=1;
    }
    if ((shell_jobs[i].program[0] != NULL) && (strcmp(shell_jobs[i].program[0], "exit") == 0))
        flexit=1;




}
char *komandi()
{
    char *tempstr;
    int  status = 0;
    while ((pid = waitpid(0, NULL, WNOHANG | WUNTRACED)) > 0)
        printf("Process  ID № %d  stopped ; status = %d\n",pid, WEXITSTATUS(status));
    tempstr=schitka_stroki();

	if(tempstr==(char*)ERR_COS_EMTY_STR)
        return (char*)ERR_COS_EMTY_STR;
    in_command=zapusk_job(tempstr);

    i=0;

    dobavlenie_v_job();
    realizasia_konveera();

    for (i = 0; i < count_of_shell_jobs; i++)
    {
        get_function();
        if (flerror==1) goto here;
        if (flexit==1)
        {
            chistka_komandnoi_stroki(in_command);
            free(in_command);
            free(tempstr);
            in_command = NULL;
            chistka_vsego();
            exit(0);
        }
        if (flalready==0)
        {
            a=0;
            execute(shell_jobs[i],tempstr);
        }
here:
        if ((i == count_of_shell_jobs-1) && (end_of_file == 1))
        {
            flag=EXIT;
            chistka_komandnoi_stroki(in_command);
            free(in_command);
            free(tempstr);
            in_command = NULL;
            chistka_vsego();
            exit(0);
        }
    }

    chistka_komandnoi_stroki(in_command);
    free(in_command);
    in_command = NULL;

    count_of_char=0;
    count_of_prog=0;
    return tempstr;
}

int main(int argc, char **argv)
{
    char *buf_dir=NULL;
    char *substr;

    pid1 = getpid();
    uid = getuid();
    shellprogram.prog_status = 0;
    shellprogram.number_of_arguments = argc;
    shellprogram.arguments = argv;
	back_count=0;
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, hdl);
    directory = getcwd(directory, 2000);
    buf_dir=realloc(directory,(strlen(directory)+strlen(argv[0])+4)*sizeof(char));
    if(buf_dir==NULL)
    {
        fprintf(stderr,ERR_MES1);
        chistka_vsego();
        free(directory);
        exit(-1);
    }
    directory=buf_dir;
    directory=strcat(directory,argv[0]+1);
    while (flag!=EXIT)
    {
        a=0;
        setdeafault();
        printf("OBIITO@SHELL:~$ ");
        substr=komandi();
        if(substr==(char*)ERR_COS_EMTY_STR)/* проверяем не пустой ли*/
            continue;
        chistka_komandnoi_stroki(in_command);
        chistka_jobs();
        free(in_command);
        free(substr);
        free(str);
    }
    free(directory);
    free(fakestr);
    free(shell_jobs);
    if (flexit==1)
        exit(0);
    return 0;
}
