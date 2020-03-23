#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <errno.h>
#include <string.h> 
#include <stdlib.h>
#include <fcntl.h> 
#include <math.h>
#include <errno.h>

#define READ 0
#define WRITE 1

//----------------------------------------------------------------
//      FUNÇÕES DE VALIDAÇÃO
//--------------------------------------------------------------
char *validWords[] = {"-l", "--count-links", "-a", "--all", "-b", "--bytes", "-S", "--separate-dirs", "-L", "--deference", "-B", "--block-size", "--max-depth"};

char path[50] = "/home/marcelo/SOPE/PROJ/projetoSOPE1";

//zero- indica se inclui o zero ou nao
int isValidNumber(char *string, int zero){

    if(string == NULL || strcmp("", string) == 0){
        return 0;
    }

    if(strcmp("-1", string) == 0){
        return 1;
    }

    for(int i = 0; i < strlen( string ); i ++)
   {
      if (string[i] < (49-zero) || string[i] > 57){
         return 0;
      }
   }
 
   return 1;
}


int inValidWords(char *string){
    for(int i = 0; i < 13; i++){
        if(strcmp(string, validWords[i]) == 0)
            return 1;
    }
    return 0;
}

void validFormat(int argc, char *argv[]){
   for(int i = 1; i < argc; i++){
        if(inValidWords(argv[i]) == 0){
            if (strncmp(argv[i], "--max-depth=", 12) == 0) {
                if (!isValidNumber(&(argv[i])[12], 1)) {
                    printf("Invalid Format 0!\n");
                    exit(1);
                }
            }
            else {
                if(!isValidNumber(argv[i], 1) && ((&argv[i])[0][0] != '/') && ((&argv[i])[0][0] != '~') && ((&argv[i])[0][0] != '.')){
                printf("Invalid Format!1\n");
                exit(1);
                }
                else if(isValidNumber(argv[i], 1) == 1){
                    if(strcmp(argv[i-1], "--max-depth") != 0 && strcmp(argv[i-1], "--block-size") != 0 && strcmp(argv[i-1], "-B") != 0){
                        printf("Invalid Format!2\n");
                        exit(1);
                    }
                }
            }

        }
        else if(strcmp(argv[i], "--max-depth") == 0 || strcmp(argv[i], "--block-size") == 0 || strcmp(argv[i], "-B") == 0){
            if(argv[i+1] == NULL){
                printf("Invalid Format 3!\n");
                exit(1);
            }
            else if(!isValidNumber(argv[i + 1],1)){
                printf("Invalid Format 4!\n");
                exit(1);
            }
        }
    }
}

int verifyA(int num, char *arg[]){

    for(int i = 1; i < num; i++){
        if(strcmp(arg[i], "-a") == 0 || strcmp(arg[i], "--all") == 0)
            return 1;
    }
    return -1;
}

int verifyB(int num, char *arg[]){

    for(int i = 1; i < num; i++){
        if(strcmp(arg[i], "-b") == 0 || strcmp(arg[i], "--bytes") == 0)
            return 1;
    }
    return 0;
}

int verifyS(int num, char *arg[]){

    for(int i = 1; i < num; i++){
        if(strcmp(arg[i], "-S") == 0 || strcmp(arg[i], "--separate-dirs") == 0)
            return 1;
    }
    return -1;
}

int verifyL(int num, char *arg[]){

    for(int i = 1; i < num; i++){
        if(strcmp(arg[i], "-L") == 0 || strcmp(arg[i], "--deference") == 0)
            return 1;
    }
    return -1;
}

int verifyBlocks(int num, char *arg[]){
    for(int i = 1; i < num; i++){
        if(strcmp(arg[i], "-B") == 0 || strcmp(arg[i], "--block-size") == 0)
            if(isValidNumber(arg[i+1], 0))
                return atoi(arg[i+1]);
            else
                printf("Next to -B/--block-size has to be a number greater than zero!");
    }
    return -1;
}

int verifyMax(int num, char *arg[]){
     for(int i = 1; i < num; i++){
        if(strcmp(arg[i], "--max-depth") == 0) {
            if (isValidNumber(arg[i + 1], 1)) {
                return atoi(arg[i + 1]);
            }
            else
                printf("Next to --max-depth has to be a number!");
        }else  if (strncmp(arg[i], "--max-depth=", 12) == 0) {
            char size[10];
            strcpy(size, &(arg[i])[12]);
            if (isValidNumber(size, 1)) {
                return atoi(size);
            }
            else
                printf("Should have a number(SIZE) in --max-depth=SIZE!");

        }
    }
    return -2;
}

int passDir(int num, char *arg[], char *envp[]){
    for(int i = 1; i < num; i++){
        if((&arg[i])[0][0] == '/' || (&arg[i])[0][0] == '~')
            return i;
        else if(strcmp(arg[i], ".") == 0){
            arg[i] = getenv("PWD");
            return i;
        }
        else if((&arg[i])[0][0] == '.')
            return i;
    }
    return -1;
}

//------------------------------------------------------------------------

int makeArg(int ind, char *argv[], int argc, char *d, char *arraPass[]){
    int ret = -1;
    arraPass[0] = argv[0];
    for(int i = 1; i <= argc; i++){
        if(i == ind)
            arraPass[i] = d;
        else if(i != argc){
            if(strcmp(argv[i], "--max-depth") == 0)
                ret = i + 1;
            else if (strncmp(argv[i], "--max-depth=", 12) == 0)
            {
                ret = i;
            }
            
            arraPass[i] = argv[i];
        }
        else{
            if(ind == -1){
                arraPass[i] = d;
                arraPass[i+1] = NULL;
            }
            else    
                arraPass[i] = NULL;
        }
    }
    return ret;
}

int soma(FILE * des){
    char *buffer[50];
    int soma = 0, num, cont = 0;
    size_t len;
    while(getline(buffer, &len, des)!=-1){
        soma += atoi(buffer[0]);
        cont++;
    }
    return soma;

}

//---------------------------------------------
//      INVOCADA PARA FAZER WAITS DOS FILHOS
//---------------------------------------------

void fazWait(){

    int val;

    while(val = wait(NULL)){
        if(val == -1 && errno == ECHILD)
            break;
    }

}

//--------------------------------------------------------
//      FUNÇÃO DE PROTEÇÃO CONTRA CTRL + C
//--------------------------------------------------------

void sigIntHandler(int signal){
    char *res;
    size_t len;
    int num = atoi(getenv("PIDGROUPSON"));

    if(num != 0)
        kill(0 - num, SIGSTOP);

    printf("\n######################################################\n\t\tMenu de Saida\n######################################################\n");

    while(1){
        printf("\nTem a certeza que pretende abandonar a execução do programa(s/n)? ");
        getline(&res, &len, stdin);
        if(res[0] == 's' || res[0] == 'n')
            break;
    }

    printf("\n######################################################\n\n");

    if(res[0] == 'n' && num != 0){
        printf("Entrei\n");
        kill(0 - num, SIGCONT);
    }
    else if(num != 0){
        kill(0 - num, SIGTERM);
        fazWait();
        exit(5);
    }

}

//-----------------------------------------------------------------------

int main(int argc, char *argv[], char *envp[]){
    DIR *dir;
    struct dirent *dentry;
    struct stat stat_entry;
    char d[PATH_MAX], fileName[PATH_MAX];
    char directory[PATH_MAX],buffer[PATH_MAX];
    int a, b, S, B, L, m; //opções do comando simpleDu
    int ind, somaBlocks = 0, contFil = 0, somaSize = 0;
    char pathcpy[50];
    FILE *f, *regProg;
    int countChilds = 0; // conta o numero de processos filho
    int fd[2],pid;


    // criar um pipe para comunicar com os filhos
    if (pipe(fd)<0){
        perror("Pipe");
        exit(1);
    }


    //---------------------------------------------------

    //signal(SIGINT, sigIntHandler);


    //-----------------------------------------------------
    //      Guarda a informaçao em ficheiro
    //-----------------------------------------------------
    /*
    if(getenv("LOG_FILENAME") == NULL)
        strcpy(fileName, "RegistoProg.txt");
    else
        strcpy(fileName, getenv("LOG_FILENAME"));
    
    regProg = fopen(fileName, "a");
    fclose(regProg);
    */
    //-------------------------------------------------------
    //                              PASSO 1
    //-------------------------------------------------------

    validFormat(argc, argv);  //verifica se está num formato válido

    //verifica se passou algum diretorio se nao vai buscar o atual as variaveis de ambiente
    if((ind = passDir(argc, argv, envp)) == -1)
        strcpy(directory, getenv("PWD"));
    else
        strcpy(directory, argv[ind]);

    if ((dir = opendir(directory)) == NULL) {
        perror(directory);
        return 2;
    }

    //Verifica as opcoes do utilizador
    a = verifyA(argc, argv);  //verifica se colocou -a ou --all
    b = verifyB(argc, argv);  //verifica se colocou -b ou --bytes
    S = verifyS(argc, argv);  //verifica se colocou -S ou --separate-dirs
    L = verifyL(argc, argv);  //verifica se colocou -L ou --deference
    B = verifyBlocks(argc, argv);  //verifica se colocou -B ou --block-size
    m = verifyMax(argc, argv);  //verifica se colocou --max-depth
    //----------------------------------------------------

    chdir(directory);

    //Imprime primeiro os ficheiros
    while (1) {

        int inodes[PATH_MAX];
        int i = 0;

        if((dentry = readdir(dir)) == NULL)
            break;
        lstat(dentry->d_name, &stat_entry);
        //------------------------------
        //      Imprimir Diretorio 
        //------------------------------
        strcpy(d, directory);
        if(!(strcmp(dentry->d_name, ".") == 0 || strcmp(dentry->d_name, "..") == 0)){
            if(strcmp(&d[strlen(d)-1], "/") != 0)
                strcat(d, "/");
            strcat(d, dentry->d_name);
        }
        //se tiver subdiretorios invoca o fork()
        if(S_ISDIR(stat_entry.st_mode) && strcmp(dentry->d_name, ".") != 0 && strcmp(dentry->d_name, "..") != 0){
            countChilds++;

            //char aux[50];
            //printf(aux, "PIDGROUPSON=%d", pid);
            //putenv(aux);

            pid=fork();

            if(pid < 0){
                perror("Fork");
                exit(1);
            }
            
            if(pid == 0){

                close(fd[READ]);

                dup2(fd[WRITE],STDOUT_FILENO);

                char *arraPass[argc+1], string[PATH_MAX];
                int val = makeArg(ind, argv, argc, d, arraPass);

                if(val != -1 && (m > -1) && (strcmp(argv[val-1], "--max-depth") == 0)){
                    sprintf(string, "%d", (m - 1));
                    arraPass[val] = string;
                }else if (val!=-1 && m >-1){
                    sprintf(string, "--max-depth=%d", (m - 1));
                    arraPass[val] = string;
                }

                strcpy(pathcpy,path);
                execve(strcat(pathcpy,"/simpledu"), arraPass, envp);
                perror("execvp");
                exit(2);
            }
            
        }
        //------------------------------
        //Nao imprime ficheiros regulares nem diretorios
        if (!S_ISLNK(stat_entry.st_mode) && !S_ISREG(stat_entry.st_mode) && !S_ISDIR(stat_entry.st_mode)){
            somaBlocks += ((int)stat_entry.st_blocks)/2;
            somaSize += (int)stat_entry.st_size;
            if(m == -2 || m > 0){
                if(B >= 1)
                    printf("%-10d%s\n",(int)ceil((((int)stat_entry.st_blocks)/2)*1024/B), d);
                else if(b != 1)
                    printf("%-10d%s\n",((int)stat_entry.st_blocks)/2, d);
                else if(b == 1)
                    printf("%-10d%s\n",(int)stat_entry.st_size, d);
            }
        }
        //imprime links simbolicos se for preciso
        if(S_ISLNK(stat_entry.st_mode)){
            if(L != 1){
                somaBlocks += ((int)stat_entry.st_blocks)/2;
                somaSize += (int)stat_entry.st_size;
                if((m == -2 || m > 0) &&  a==1){
                    if(B >= 1)
                        printf("%-10d%s\n",(int)ceil((((int)stat_entry.st_blocks)/2)*1024/B), d);
                    else if(b!= 1)
                        printf("%-10d%s\n",((int)stat_entry.st_blocks)/2, d);
                    else if(b == 1)
                        printf("%-10d%s\n",(int)stat_entry.st_size, d);
                }
            }
            //arranjar funcao que siga symbolic link para ver o seu tamanho real
            /*else{
                int size, blocks;
                readlink()
            }*/
        }
        //imprime ficheiros regulares se for preciso
        if(S_ISREG(stat_entry.st_mode)){
            somaBlocks += ((int)stat_entry.st_blocks)/2;
            somaSize += (int)stat_entry.st_size;
            if((m == -2 || m > 0) && a == 1){
                if(B >= 1)
                    printf("%-10d%s\n",(int)ceil((((int)stat_entry.st_blocks)/2)*1024/B), d);
                else if(b!= 1)
                    printf("%-10d%s\n",((int)stat_entry.st_blocks)/2, d);
                else if(b == 1)
                    printf("%-10d%s\n",(int)stat_entry.st_size, d);
            }
        }
    }

    close(fd[WRITE]);
    //------------------------------------------------------
    if (countChilds != 0){
        fazWait();
    }
        //----------------------------------------------------------------------------

    rewinddir(dir);
    
    while (1) {
        if((dentry = readdir(dir)) == NULL)
            break;
        lstat(dentry->d_name, &stat_entry);
        //------------------------------
        //      Imprimir Diretorio 
        //------------------------------
        strcpy(d, directory);
        if(!(strcmp(dentry->d_name, ".") == 0 || strcmp(dentry->d_name, "..") == 0)){
            if(strcmp(&d[strlen(d)-1], "/") != 0)
                strcat(d, "/");
            strcat(d, dentry->d_name);
        }

        //------------------------------
        //So nos interesssa imprimir o diretorio .
        //----------------------------
        if(S_ISDIR(stat_entry.st_mode) && strcmp(dentry->d_name, ".") == 0){
            
            if (countChilds!=0 ){
                FILE *receiver;
                char *buffer[20];
                size_t len;
                receiver =fdopen(fd[READ],"r");                
                for (int i = 0; i < countChilds; i++)
                {
                    getline(buffer,&len,receiver);
                    somaSize += atoi(buffer[0]);
                    getline(buffer,&len,receiver);
                    somaBlocks += atoi(buffer[0]);
                }
            }
            somaSize += (int)stat_entry.st_size;
            somaBlocks += ((int)stat_entry.st_blocks)/2;
            
            /*
            strcpy(buffer, "");
            strcpy(pathcpy,path);
            sprintf(buffer, strcat(pathcpy,"/sizes%d.txt"), getpid());
            FILE *fsize = fopen(buffer, "r");
            if(fsize != NULL && S!=1 ){
                somaSize +=  soma(fsize);
                fclose(fsize);
                remove(buffer);
            }
            somaSize += (int)stat_entry.st_size;
    
            
            strcpy(buffer, "");
            strcpy(pathcpy,path);
            sprintf(buffer, strcat(pathcpy,"/blocks%d.txt"), getpid());
            FILE *fblock = fopen(buffer, "r");
            if(fblock != NULL && S!=1){ //se tiver subdiretorios
                somaBlocks +=  soma(fblock);
                fclose(fblock);
                remove(buffer);
            }
         //se nao tiver sido criado ficheiro ou seja se nao tiver subdiretorios  
            somaBlocks += ((int)stat_entry.st_blocks)/2; 
            
            
            if(m != -1){
                if(B >= 1)
                    printf("%-10d%s\n",(int)ceil(somaBlocks * 1024 / B), d);
                else if(b != 1)
                    printf("%-10d%s\n",somaBlocks, d);
                else if(b == 1)
                    printf("%-10d%s\n",somaSize, d);
            }
            */
        }

    }
    
    char msg[50];
    size_t len;
    sprintf(msg,"%d\n%d\n",somaSize,somaBlocks);
    len = strlen(msg);
    write(STDOUT_FILENO,msg,len); 
    
    //------------------------------------------------------
    //Guarda em ficheiros com o pid do pai os valores atuais dos subdirectorios em relacao ao tamanho em bytes   
    /*
    strcpy(buffer, "");
    strcpy(pathcpy,path);
    sprintf(buffer, strcat(pathcpy,"/sizes%d.txt"), getppid());
    f = fopen(buffer, "a");
    strcpy(buffer, "");
    sprintf(buffer, "%d\n", somaSize);
    fwrite(buffer, sizeof(char), strlen(buffer), f);
    fclose(f);

    //------------------------------------------------------
    //Guarda em ficheiros com o pid do pai os valores atuais dos subdirectorios em relacao ao tamanho em blocks de 1024 
    strcpy(buffer, "");
    strcpy(pathcpy,path);
    sprintf(buffer, strcat(pathcpy,"/blocks%d.txt"), getppid());
    f = fopen(buffer, "a");
    strcpy(buffer, "");
    sprintf(buffer, "%d\n", somaBlocks);
    fwrite(buffer, sizeof(char), strlen(buffer), f);
    fclose(f);
    */
    //---------------------------------------------
    
    return 0; 
}