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

//----------------------------------------------------------------
//      FUNÇÕES DE VALIDAÇÃO
//--------------------------------------------------------------
char *validWords[] = {"-l", "--count-links", "-a", "--all", "-b", "--bytes", "-S", "--separate-dirs", "-L", "--dereference", "-B", "--block-size", "--max-depth"};

char path[50] = "/home/diogo/Documentos/SOPE/projetoSOPE1";

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
            else if(strcmp(argv[i], "group") == 0){
                if(!isValidNumber(argv[i+1], 1)){
                    printf("Invalid Format 5!\n");
                    exit(1);
                }
            }
            else {
                if(!isValidNumber(argv[i], 1) && ((&argv[i])[0][0] != '/') && ((&argv[i])[0][0] != '~') && ((&argv[i])[0][0] != '.')){
                    printf("Invalid Format!1\n");
                    exit(1);
                }
                else if(isValidNumber(argv[i], 1) == 1){
                    if(strcmp(argv[i-1], "group") != 0 && strcmp(argv[i-1], "--max-depth") != 0 && strcmp(argv[i-1], "--block-size") != 0 && strcmp(argv[i-1], "-B") != 0){
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
        if(strcmp(arg[i], "-L") == 0 || strcmp(arg[i], "--dereference") == 0)
            return 1;
    }
    return -1;
}

int verifyBlocks(int num, char *arg[]){
    for(int i = 1; i < num; i++){
        if(strcmp(arg[i], "-B") == 0 || strcmp(arg[i], "--block-size") == 0){
            if(isValidNumber(arg[i+1], 0))
                return atoi(arg[i+1]);
            else
                printf("Next to -B/--block-size has to be a number greater than zero!");
        }
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

int findGroup(int argc, char *argv[]){

    for(int i = 0; i < argc; i++){
        if(strcmp(argv[i], "group") == 0){
            return atoi(argv[i+1]);
        }
    }

    return -1;
}

int makeArg(int ind, char *argv[], int argc, char *d, char *arraPass[], int group, int pid){
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
                if(group != pid){
                    arraPass[i+1] = "group";
                    char aux[50];
                    sprintf(aux, "%d", pid);
                    arraPass[i+2] = aux;
                    arraPass[i+3] = NULL;
                }
                else
                    arraPass[i+1] = NULL;
            }
            else {
                if(group != pid){
                    arraPass[i] = "group";
                    char aux[50];
                    sprintf(aux, "%d", pid);
                    arraPass[i+1] = aux;
                    arraPass[i+2] = NULL;
                }
                else
                    arraPass[i] = NULL;
            }
        }
    }
    return ret;
}

int soma(FILE * des){
    char *buffer[50];
    int soma = 0, cont = 0;
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

    while(1){
        val = wait(NULL);
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
    int num;

    if(getenv("PIDGROUP") == NULL){
        printf("Erro\n");
        exit(7);
    }

    num = atoi(getenv("PIDGROUP"));

    if(num != -1){
        if(kill(-num, SIGSTOP) == -1){
            printf("Error on Kill\n");
            exit(7);
        }
    }

    printf("\n\n######################################################\n\t\tMenu de Saida\n######################################################\n");

    while(1){
        printf("\nTem a certeza que pretende abandonar a execução do programa(s/n)? ");
        getline(&res, &len, stdin);
        if(res[0] == 's' || res[0] == 'n')
            break;
    }

    printf("\n######################################################\n\n");

    if(res[0] == 'n' && num != -1){
        if(kill(-num, SIGCONT) == -1){
            printf("Error on Kill\n");
            exit(7);
        }
    }
    else if(num != 0){
         if(kill(-num, SIGTERM) == -1){
            printf("Error on Kill\n");
            exit(7);
        }
        //fazWait();
        exit(5);
    }

}

//-----------------------------------------------------------------------

int main(int argc, char *argv[], char *envp[]){
    DIR *dir, *aux;                       //
    struct dirent *dentry;          //   Usadas na leitura dos diretorios
    struct stat stat_entry;         //
    //-------------------------------------------------------
    char fileName[PATH_MAX];                //Nome do ficheiro onde vai ser mantida a informacao
    char d[PATH_MAX], directory[PATH_MAX],pathcpy[50];  //Usadas na impressao do nome dos diretorios
    //-------------------------------------------------------
    int a, b, S, B, L, m; //opções do comando simpleDu
    int ind, somaBlocks = 0, somaSize = 0;   //Vai guardar o tamanho dos subdiretorios
    //-------------------------------------------------------
    char buffer[50];  //Variavel auxiliar
    //-------------------------------------------------------
    FILE *f, *regProg;
    //-------------------------------------------------------

    setbuf(stdout, NULL);

    //---------------------------------------------------

    int group = findGroup(argc, argv);  //junta aos argumentos do programa um pid que vou definir para criar um grupo
                                        //ao qual todos vao pertencer menos o processo inicial

    sprintf(buffer, "PIDGROUP=%d", group); //passo o group id dos processos
    putenv(buffer);

    //-------------------------------------------------------
        
    signal(SIGINT, sigIntHandler);      //Instalacao do handler para CTRL+C

    //-----------------------------------------------------
    //      Guarda a informaçao em ficheiro
    //-----------------------------------------------------

    strcpy(fileName, path);
    strcat(fileName, "/");
    if(getenv("LOG_FILENAME") == NULL)
        strcat(fileName, "RegistoProg.txt");
    else
        strcat(fileName, getenv("LOG_FILENAME"));
    
    /*if(group == -1){
        regProg = fopen(fileName, "a");
        fclose(regProg);
    }*/
    
    //-------------------------------------------------------
    //                              PASSO 1
    //-------------------------------------------------------
    //Validacao do Formato da String
    //Verificacao se houve passagem de diretorio
    //Verificacao das opcoes utilizadas

    //1
    validFormat(argc, argv);  //verifica se está num formato válido

    //2
    //verifica se passou algum diretorio se nao vai buscar o atual as variaveis de ambiente
    if((ind = passDir(argc, argv, envp)) == -1)
        strcpy(directory, getenv("PWD"));
    else
        strcpy(directory, argv[ind]);

    if ((dir = opendir(directory)) == NULL) {
        perror(directory);
        return 2;
    }

    //3
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

        if((dentry = readdir(dir)) == NULL)
            break;
        lstat(dentry->d_name, &stat_entry);
        //------------------------------
        //      Imprimir Diretorio 
        //------------------------------
        //Forma a string com o nome do diretorio
        strcpy(d, directory);
        if(!(strcmp(dentry->d_name, ".") == 0 || strcmp(dentry->d_name, "..") == 0)){
            if(strcmp(&d[strlen(d)-1], "/") != 0)
                strcat(d, "/");
            strcat(d, dentry->d_name);
        }
         //----------------------------------------------------
        //se tiver subdiretorios invoca o fork()
        if(S_ISDIR(stat_entry.st_mode) && strcmp(dentry->d_name, ".") != 0 && strcmp(dentry->d_name, "..") != 0){
            int pid = fork();

            if(pid < 0){
                perror("Fork");
                exit(1);
            }
            else if(pid == 0){
                char *arraPass[argc+3], string[PATH_MAX];
                if(group == -1)
                    group = getpid();

                if(setpgid(getpid(), group) == -1){ //altero o groupid dos processos que vao surgir para pertencerem
                    printf("setpgid error\n");           //todos ao mesmo mas diferente do pai
                    exit(5);
                }

                int num = findGroup(argc, argv);
                int val = makeArg(ind, argv, argc, d, arraPass, num, group);

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
            else{
                if(group == -1)
                    group = pid;
                
                sprintf(buffer, "PIDGROUP=%d", group); //passo o group id dos processos
                putenv(buffer);
            }
        }
         //----------------------------------------------------
        //Ficheiros de um tipo tal que nao sao regulares nem links simbolicos
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
         //----------------------------------------------------
        //Links simbolicos
        else if(S_ISLNK(stat_entry.st_mode)){
            //Nao segue links simbolicos
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
            //Segue links simbolicos
            else{
                char aux1[PATH_MAX], aux2[PATH_MAX];
                readlink(d, aux1, sizeof(aux1));
                strcpy(aux2, directory);
                if(strcmp(&aux2[strlen(d)-1], "/") != 0)
                    strcat(aux2, "/");
                strcat(aux2, aux1);
                lstat(aux2, &stat_entry);
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
        }
        //----------------------------------------------------
        //Ficheiros Regulares
        else if(S_ISREG(stat_entry.st_mode)){
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

    //------------------------------------------------------
    
    fazWait();

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
            strcpy(buffer, "");
            strcpy(pathcpy,path);
            sprintf(buffer, strcat(pathcpy,"/sizes%d.txt"), getpid());
            FILE *fsize = fopen(buffer, "r");
            if(fsize != NULL){
                if(S!=1)
                    somaSize += ((int)stat_entry.st_size) + soma(fsize);
                else
                    somaSize += (int)stat_entry.st_size;
                fclose(fsize);
                remove(buffer);
            }
            else
                somaSize += (int)stat_entry.st_size;
    
            
            strcpy(buffer, "");
            strcpy(pathcpy,path);
            sprintf(buffer, strcat(pathcpy,"/blocks%d.txt"), getpid());
            FILE *fblock = fopen(buffer, "r");
            if(fblock != NULL){ //se tiver subdiretorios
                if(S != 1)
                    somaBlocks += ((int)stat_entry.st_blocks / 2) + soma(fblock);
                else
                    somaBlocks += ((int)stat_entry.st_blocks)/2;

                fclose(fblock);
                remove(buffer);
            }
            else  //se nao tiver sido criado ficheiro ou seja se nao tiver subdiretorios  
               somaBlocks += ((int)stat_entry.st_blocks)/2; 
            if(m != -1){
                if(B >= 1)
                    printf("%-10d%s\n",(int)ceil(somaBlocks * 1024 / B), d);
                else if(b != 1)
                    printf("%-10d%s\n",somaBlocks, d);
                else if(b == 1)
                    printf("%-10d%s\n",somaSize, d);
            }
        }

    }

    //------------------------------------------------------
    //Guarda em ficheiros com o pid do pai os valores atuais dos subdirectorios em relacao ao tamanho em bytes   
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

    //---------------------------------------------

    return 0; 
}