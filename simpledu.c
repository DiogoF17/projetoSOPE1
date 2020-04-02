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
#include <time.h>

//----------------------------------------------------------------
//      FUNÇÕES DE VALIDAÇÃO
//--------------------------------------------------------------
/*
Palavras que poderao ser introduzidas pelo utilizador mas falta aqui o --max-depth=.
*/
char *validWords[] = {"-l", "--count-links", "-a", "--all", "-b", "--bytes", "-S", "--separate-dirs", "-L", "--dereference", "-B", "--block-size"};

//|||||||||||||||||||||||||||||||||||
//arraPass = {..., ..., ..., ..., ..., ..., ..., ..., ..., ...}
//            func, dir, -a, -b , -B , -L , -S, -max, groupS, Orig
//|||||||||||||||||||||||||||||||||||

/*
Macros usadas na passagem do array.
*/
#define FUNC 0   //nome da funcao.
#define DIRE 1   //nome do diretorio.   
#define a 2      //-a
#define b 3      //-b   
#define B 4      //-B x 
#define L 5      //-L  
#define S 6      //-S  
#define m 7      //--max-depth=x
#define g 8      //contem o groupid necessario para fazer o set do group.   
#define ORIG 9   //string que nos indica se e o processo original.

/*
Macros usadas nos pipes.
*/
#define READ 0
#define WRITE 1

/*
Verifica se e um numero valido.
zero - indica se inclui o zero ou nao na verificacao se e um numero.
*/
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

/*
Verifica se a palavra introduzida pelo utilizador esta nas palavras validas.
*/
int inValidWords(char *string){
    for(int i = 0; i < 13; i++){
        if(strcmp(string, validWords[i]) == 0)
            return 1;
    }
    return 0;
}

/*
Verifica se o comando introduzido esta num formato valido.
*/
void validFormat(int argc, char *argv[], int ind){
   for(int i = 1; i < argc; i++){
       if(i != ind){
            if(inValidWords(argv[i]) == 0){
                if (strncmp(argv[i], "--max-depth=", 12) == 0) {
                    if (!isValidNumber(&(argv[i])[12], 1)) {
                        printf("Invalid Format 0!\n");
                        exit(1);
                    }
                }
                else {
                    if(!isValidNumber(argv[i], 1)){
                        printf("Invalid Format!1\n");
                        exit(1);
                    }
                    else if(isValidNumber(argv[i], 1) == 1){
                        if(strcmp(argv[i-1], "--block-size") != 0 && strcmp(argv[i-1], "-B") != 0){
                            printf("Invalid Format!2\n");
                            exit(1);
                        }
                    }
                }

            }
            else if(strcmp(argv[i], "--block-size") == 0 || strcmp(argv[i], "-B") == 0){
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
}

/*
Verifica se foi introduzido -a.
*/
char* verifyA(int num, char *arg[]){

    for(int i = 1; i < num; i++){
        if(strcmp(arg[i], "-a") == 0 || strcmp(arg[i], "--all") == 0)
            return "1";
    }
    return "-1";
}

/*
Verifica se foi introduzido -b.
*/
char* verifyB(int num, char *arg[]){

    for(int i = 1; i < num; i++){
        if(strcmp(arg[i], "-b") == 0 || strcmp(arg[i], "--bytes") == 0)
            return "1";
    }
    return "-1";
}

/*
Verifica se foi introduzido -S.
*/
char* verifyS(int num, char *arg[]){

    for(int i = 1; i < num; i++){
        if(strcmp(arg[i], "-S") == 0 || strcmp(arg[i], "--separate-dirs") == 0)
            return "1";
    }
    return "-1";
}

/*
Verifica se foi introduzido -L.
*/
char* verifyL(int num, char *arg[]){

    for(int i = 1; i < num; i++){
        if(strcmp(arg[i], "-L") == 0 || strcmp(arg[i], "--dereference") == 0)
            return "1";
    }
    return "-1";
}

/*
Verifica qual o valor de -B se introduzido.
*/
char* verifyBlocks(int num, char *arg[]){
    for(int i = 1; i < num; i++){
        if(strcmp(arg[i], "-B") == 0 || strcmp(arg[i], "--block-size") == 0){
            if(isValidNumber(arg[i+1], 0))
                return arg[i+1];
            else
                printf("Next to -B/--block-size has to be a number greater than zero!");
        }
    }
    return "-1";
}

/*
Verifica qual o valor de --max-depth se introduzido.
*/
char* verifyMax(int num, char *arg[]){
     for(int i = 1; i < num; i++){
        if (strncmp(arg[i], "--max-depth=", 12) == 0) {
            char size[10];
            strcpy(size, &(arg[i])[12]);
            if (isValidNumber(&(arg[i])[12], 1)) {
                return &(arg[i])[12];
            }
            else
                printf("Should have a number(SIZE) in --max-depth=SIZE!");

        }
    }
    return "-2";
}

/*
Verifica se foi passado algum diretorio pelo utilizador.
*/
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

FILE *regProg;
struct timeval start, stop;
//todo
double get_time(){
    gettimeofday(&stop,NULL);
    return (double)(stop.tv_usec-start.tv_usec)/1000;
}

int file_open_new_empty(){
    char fileName[PATH_MAX];
    if (getenv("LOG_FILENAME") == NULL){
        sprintf(fileName, "%s/Registos.txt", getenv("PWD"));
    }
    else
        strcpy(fileName, getenv("LOG_FILENAME"));
    if((regProg = fopen(fileName, "w"))==NULL){
        perror("fopen");
        exit(4);
    }
    fclose(regProg);

}

int file_open_append(){
    char fileName[PATH_MAX];
    if (getenv("LOG_FILENAME") == NULL){
        sprintf(fileName, "%s/Registos.txt", getenv("PWD"));
    }
    else
        strcpy(fileName, getenv("LOG_FILENAME"));
    if((regProg = fopen(fileName, "a"))==NULL){
        perror("fopen");
        exit(4);
    }

}

void create_file(char *arraPass[11], int pid){
    //TODO:CREATE
    file_open_append();
    //os argumentos da linha de comandos
    char str_pid[PATH_MAX];
    sprintf(str_pid, "%.2f - %d - CREATE - %s %s %s %s %s %s %s %s %s %s \n",
            get_time(), pid, arraPass[0], arraPass[1], arraPass[2], arraPass[3], arraPass[4],
            arraPass[5], arraPass[6], arraPass[7], arraPass[8], arraPass[9]);

    if(fwrite(str_pid, sizeof(char), strlen(str_pid), regProg) != strlen(str_pid)){
            perror("fwrite");
            exit(6);
    }
    fclose(regProg);
}

void exit_file(int exit_number, int pid){
    //TODO:EXIT
    file_open_append();
    char str_pid[PATH_MAX];
    //o código de saída (exit status)
    sprintf(str_pid, "%.2f - %d - EXIT - %d\n", get_time(),pid, WEXITSTATUS(exit_number));
    if(fwrite(str_pid, sizeof(char), strlen(str_pid), regProg) != strlen(str_pid)){
            perror("fwrite");
            exit(6);
    }
    fclose(regProg);
}

void recv_signal_file(int number){
    //TODO:RECV_SIGNAL
    file_open_append();
    char str_pid[PATH_MAX];
    //sinal recebido(por exemplo, SIGINT)
    sprintf(str_pid, "%.2f - %d - RECV_SIGNAL - %d\n", get_time(),getpid(), number);
    if(fwrite(str_pid, sizeof(char), strlen(str_pid), regProg) != strlen(str_pid)){
            perror("fwrite");
            exit(6);
    }
    fclose(regProg);
}

void send_signal_file(int number, int pid){
    //TODO:SEND_SIGNAL
    file_open_append();
    char str_pid[PATH_MAX];
    //sinal recebido(por exemplo, SIGINT)
    sprintf(str_pid, "%.2f - %d - SEND_SIGNAL - %d %d\n", get_time(),getpid(), number, pid);
    if(fwrite(str_pid, sizeof(char), strlen(str_pid), regProg) != strlen(str_pid)){
            perror("fwrite");
            exit(6);
    }
    fclose(regProg);
}

void recv_pipe_file(int ms1, int ms2){
    //TODO:RECV_PIPE
    file_open_append();
    char str_pid[PATH_MAX];
    //a mensagem enviada
    sprintf(str_pid, "%.2f - %d - RECV_PIPE - %d %d\n", get_time(),getpid(),ms1, ms2);
     if(fwrite(str_pid, sizeof(char), strlen(str_pid), regProg) != strlen(str_pid)){
            perror("fwrite");
            exit(6);
    }
    fclose(regProg);
}

void send_pipe_file(int ms1, int ms2){
    //TODO:SEND_PIPE
    file_open_append();
    char str_pid[PATH_MAX];
   //a mensagem recebida
    sprintf(str_pid, "%.2f - %d - SEND_PIPE - %d %d\n", get_time(),getpid(),ms1, ms2);
    if(fwrite(str_pid, sizeof(char), strlen(str_pid), regProg) != strlen(str_pid)){
            perror("fwrite");
            exit(6);
    }
    fclose(regProg);
}

void entry_file(char *d, int val){
    //TODO:ENTRY
    file_open_append();
    char str_pid[PATH_MAX];
   //número de bytes(ou blocos)seguido do caminho.
    sprintf(str_pid, "%.2f - %d - ENTRY - %d %s\n", get_time(),getpid(), val, d);
     if(fwrite(str_pid, sizeof(char), strlen(str_pid), regProg) != strlen(str_pid)){
            perror("fwrite");
            exit(6);
    }
    fclose(regProg);
}

//------------------------------------------------------------------------

/*
Atraves de uma string que eu adiciono verifica se e o processo original.
*/
int findNotOrig(int argc, char *argv[]){

    for(int i = 0; i < argc; i++){
        if(strcmp(argv[i], "notOrig") == 0){
            return 1;
        }
    }

    return 0;
}

//---------------------------------------------
//      INVOCADA PARA FAZER WAITS DOS FILHOS
//---------------------------------------------

/*
Faz wait pelos filhos do processo.
*/
void fazWait(){

    int val, exit_status;

    while(1){
        val = wait(&exit_status);
        if(val == -1 && errno == ECHILD)
            break;
        else
            exit_file(exit_status, val);
    }

}

//--------------------------------------------------------
//      FUNÇÃO DE PROTEÇÃO CONTRA CTRL + C
//--------------------------------------------------------

/*
Limpa o InputBuffer
*/
void cleanInputBuffer(){
    while(1){
        if(getchar() == '\n')
            break;
    }
}

/*
Handler para o sinal CTRL+C.
*/
void sigIntHandler(int signal){
    char res;
    size_t len;
    int num;

    recv_signal_file(SIGINT);

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
        send_signal_file(SIGSTOP, -num);
    }

    printf("\n\n######################################################\n\t\tMenu de Saida\n######################################################\n");

   while(1){
        printf("\nTem a certeza que pretende abandonar a execução do programa(s/n)? ");
        res = getchar();
        cleanInputBuffer();
        if(res == 's' || res == 'n'){
            break;
        }
    }

    printf("\n######################################################\n\n");

    if(res == 'n' && num != -1){
        if(kill(-num, SIGCONT) == -1){
            printf("Error on Kill\n");
            exit(7);
        }
        send_signal_file(SIGCONT, -num);
    }
    else if(num != 0){
         if(kill(-num, SIGTERM) == -1){
            printf("Error on Kill\n");
            exit(7);
        }
        send_signal_file(SIGTERM, -num);
        exit(5);
    }

}

/*
void printfArraPass(char *arraPass[]){
    printf("func: %s\n", arraPass[FUNC]);
    printf("dire: %s\n", arraPass[DIRE]);
    printf("a: %s\n", arraPass[a]);
    printf("b: %s\n", arraPass[b]);
    printf("B: %s\n", arraPass[B]);
    printf("L: %s\n", arraPass[L]);
    printf("S: %s\n", arraPass[S]);
    printf("m: %s\n", arraPass[m]);
    printf("g: %s\n", arraPass[g]);
    printf("Ori: %s\n", arraPass[ORIG]);
    printf("ultimo: %s\n", arraPass[10]);

}*/

//-----------------------------------------------------------------------

int main(int argc, char *argv[], char *envp[]){
    gettimeofday(&start,NULL);


    DIR *dir;                  //
    struct dirent *dentry;          //   Usadas na leitura dos diretorios
    struct stat stat_entry;        //
    //-------------------------------------------------------
    char fileName[PATH_MAX];                //Nome do ficheiro onde vai ser mantida a informacao
    char d[PATH_MAX], directory[PATH_MAX];  //Usadas na impressao do nome dos diretorios
    char path[PATH_MAX];
    char *arraPass[11];
    //-------------------------------------------------------
    char *a1, *b1, *S1, *B1, *L1, *m1; //opções do comando simpleDu
    int ind, somaBlocks = 0, somaSize = 0;   //Vai guardar o tamanho dos subdiretorios
    int countChilds = 0; // conta o numero de processos filho
    pid_t pid;  // guarda o pid quando for executado o fork()
    int fd[2]; // array de inteiros utilizado para o pipe()
    int original = 0;
    int pipeFather;
    //-------------------------------------------------------
    char buffer[50];  //Variavel auxiliar
    //-------------------------------------------------------

    strcpy(path,getenv("PWD"));
    //printf("Path: %s\n", path);

    //printf("%s\n", path);

    setbuf(stdout, NULL);

    //----------------------------------------------------
    // criar um pipe para comunicar com os filhos
    if (pipe(fd)<0){
        perror("Pipe");
        exit(1);
    }

    //----------------------------------------------------
    //Verifica se e o ficheiro original
    int notOrig = findNotOrig(argc, argv);  //junta aos argumentos do programa um pid que vou definir para criar um grupo
                                        //ao qual todos vao pertencer menos o processo inicial

    //-------------------------------------------------------
    //Instalacao do handler para CTRL+C
    struct sigaction action;

    action.sa_handler = sigIntHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;


    sigaction(SIGINT, &action, NULL);

    //----------------------------------------------------
    //verifica se passou algum diretorio se nao vai buscar o atual as variaveis de ambiente
    if((ind = passDir(argc, argv, envp)) == -1)
        strcpy(directory, getenv("PWD"));
    else
        strcpy(directory, argv[ind]);

    if ((dir = opendir(directory)) == NULL) {
        perror(directory);
        return 2;
    }

    //----------------------------------------------------
    // Ações a ser realizadas apenas pelo processo original
    if(notOrig == 0){
        original = 1;

        file_open_new_empty();

        //Verifica se esta num formato valido
        validFormat(argc, argv, ind);

        //Verifica as opcoes do utilizador
        a1 = verifyA(argc, argv);  //verifica se colocou -a ou --all
        b1 = verifyB(argc, argv);  //verifica se colocou -b ou --bytes
        S1 = verifyS(argc, argv);  //verifica se colocou -S ou --separate-dirs
        L1 = verifyL(argc, argv);  //verifica se colocou -L ou --deference
        B1 = verifyBlocks(argc, argv);  //verifica se colocou -B ou --block-size
        m1 = verifyMax(argc, argv);  //verifica se colocou --max-depth
        
        //inicializa um array que facilita a analise
        arraPass[FUNC] = argv[0];
        arraPass[DIRE] = directory;
        arraPass[a] = a1;
        arraPass[b] = b1;
        arraPass[B] = B1;
        arraPass[L] = L1;
        arraPass[S] = S1;
        arraPass[m] = m1;
        arraPass[g] = "-1";
        arraPass[ORIG] = "notOrig";
        //arraPass[10] = NULL;
    }else{
        //inicializa um array que facilita a analise
        arraPass[FUNC] = argv[0];
        arraPass[DIRE] = argv[1];
        arraPass[a] = argv[2];
        arraPass[b] = argv[3];
        arraPass[B] = argv[4];
        arraPass[L] = argv[5];
        arraPass[S] = argv[6];
        arraPass[m] = argv[7];
        arraPass[g] = argv[8];
        arraPass[ORIG] = argv[9];
        //arraPass[10] = NULL;
        pipeFather = dup(STDOUT_FILENO);
    }

    //printfArraPass(arraPass);

    //cria uma variavel de ambiente com o pid do group definido
    sprintf(buffer, "PIDGROUP=%d", atoi(arraPass[g])); //passo o group id dos processos
    //printf("Buffer: %s\n", buffer);
    putenv(buffer);
    //----------------------------------------------------
    
    chdir(directory);
    //----------------------------------------------------
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
            countChilds++;

            pid=fork();

            if(pid < 0){
                perror("Fork");
                exit(1);
            }
            if(pid == 0){

                close(fd[READ]);
                dup2(fd[WRITE],STDOUT_FILENO);

                if(atoi(arraPass[g]) == -1){
                    char string[PATH_MAX];
                    sprintf(string, "%d", getpid());
                    arraPass[g] = string;
                }
                
                if(setpgid(getpid(), atoi(arraPass[g])) == -1){ //altero o groupid dos processos que vao surgir para pertencerem
                    printf("setpgid error\n");           //todos ao mesmo mas diferente do pai
                    exit(5);
                }

                //coloca o novo diretorio na array
                arraPass[DIRE] = d;

                //printfArraPass(arraPass);

                //se tiver sido passado --max-depthdecrementa
                if(atoi(arraPass[m])>-1)
                    sprintf(arraPass[m], "%d", atoi(arraPass[m])-1);

                //printfArraPass(arraPass);

                create_file(arraPass, getpid());

                execve(strcat(path,"/simpledu"), arraPass, envp);
                perror("execvp");
                exit(2);
            }
            else{
                if(atoi(arraPass[g]) == -1){
                    char string[PATH_MAX];
                    sprintf(string, "%d", pid);
                    arraPass[g] = string;
                }
                
                sprintf(buffer, "PIDGROUP=%d", atoi(arraPass[g])); //passo o group id dos processos
                putenv(buffer);

                //create_file(arraPass);
            }
        }
        if (!original){
            dup2(STDERR_FILENO,STDOUT_FILENO);
        }
         //----------------------------------------------------
        //Ficheiros de um tipo tal que nao sao regulares nem links simbolicos
        if (!S_ISLNK(stat_entry.st_mode) && !S_ISREG(stat_entry.st_mode) && !S_ISDIR(stat_entry.st_mode)){
            somaBlocks += ((int)stat_entry.st_blocks)/2;
            somaSize += (int)stat_entry.st_size;
            if(atoi(arraPass[m]) == -2 || atoi(arraPass[m]) > 0){
                if(atoi(arraPass[B]) >= 1)
                    printf("%-10d%s\n",(int)ceil((((int)stat_entry.st_blocks)/2)*1024/atoi(arraPass[B])), d);
                else if(atoi(arraPass[b]) != 1)
                    printf("%-10d%s\n",((int)stat_entry.st_blocks)/2, d);
                else if(atoi(arraPass[b]) == 1)
                    printf("%-10d%s\n",(int)stat_entry.st_size, d);
            }
        }
         //----------------------------------------------------
        //Links simbolicos
        else if(S_ISLNK(stat_entry.st_mode)){
            //Nao segue links simbolicos
            if(atoi(arraPass[L]) != 1){
                somaBlocks += ((int)stat_entry.st_blocks)/2;
                somaSize += (int)stat_entry.st_size;
                if((atoi(arraPass[m]) == -2 || atoi(arraPass[m]) > 0) &&  atoi(arraPass[a])==1){
                    if(atoi(arraPass[B]) >= 1)
                        printf("%-10d%s\n",(int)ceil((((int)stat_entry.st_blocks)/2)*1024/atoi(arraPass[B])), d);
                    else if(atoi(arraPass[b])!= 1)
                        printf("%-10d%s\n",((int)stat_entry.st_blocks)/2, d);
                    else if(atoi(arraPass[b]) == 1)
                        printf("%-10d%s\n",(int)stat_entry.st_size, d);
                }
            }
            //Segue links simbolicos
            else{
                char aux1[PATH_MAX];
                realpath(d, aux1);
                lstat(aux1, &stat_entry);
                somaBlocks += ((int)stat_entry.st_blocks)/2;
                somaSize += (int)stat_entry.st_size;
                if((atoi(arraPass[m]) == -2 || atoi(arraPass[m]) > 0) &&  atoi(arraPass[a])==1){
                    if(atoi(arraPass[B]) >= 1)
                        printf("%-10d%s\n",(int)ceil((((int)stat_entry.st_blocks)/2)*1024/atoi(arraPass[B])), d);
                    else if(atoi(arraPass[b])!= 1)
                        printf("%-10d%s\n",((int)stat_entry.st_blocks)/2, d);
                    else if(atoi(arraPass[b]) == 1)
                        printf("%-10d%s\n",(int)stat_entry.st_size, d);
                }
            }
        }
        //----------------------------------------------------
        //Ficheiros Regulares
        else if(S_ISREG(stat_entry.st_mode)){
            somaBlocks += ((int)stat_entry.st_blocks)/2;
            somaSize += (int)stat_entry.st_size;
            if((atoi(arraPass[m]) == -2 || atoi(arraPass[m]) > 0) && atoi(arraPass[a]) == 1){
                if(atoi(arraPass[B]) >= 1)
                    printf("%-10d%s\n",(int)ceil((((int)stat_entry.st_blocks)/2)*1024/atoi(arraPass[B])), d);
                else if(atoi(arraPass[b])!= 1)
                    printf("%-10d%s\n",((int)stat_entry.st_blocks)/2, d);
                else if(atoi(arraPass[b]) == 1)
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
            
            if (countChilds!=0 && atoi(arraPass[S]) != 1){
                FILE *receiver;
                char *buffer[20];
                size_t len;
                receiver =fdopen(fd[READ],"r");
                //send_pipe_file(receiver);
                for (int i = 0; i < countChilds; i++)
                {
                    int num1, num2;
                    getline(buffer,&len,receiver);
                    num1 = atoi(buffer[0]); 
                    somaSize += num1;
                    getline(buffer,&len,receiver);
                    num2 = atoi(buffer[0]);
                    somaBlocks += num2;
                    send_pipe_file(num1, num2);
                }
            }
            somaSize += (int)stat_entry.st_size;
            somaBlocks += ((int)stat_entry.st_blocks)/2;
            
            if(atoi(arraPass[m]) != -1){
                if(atoi(arraPass[B]) >= 1){
                    printf("%-10d%s\n",(int)ceil(somaBlocks * 1024 / atoi(arraPass[B])), d);
                    entry_file(directory, (int)ceil(somaBlocks * 1024 / atoi(arraPass[B])));
                }
                else if(atoi(arraPass[b]) != 1){
                    printf("%-10d%s\n",somaBlocks, d);
                    entry_file(directory, somaBlocks);
                }
                else if(atoi(arraPass[b]) == 1){
                    printf("%-10d%s\n",somaSize, d);
                    entry_file(directory, somaSize);
                }
            }
            
        }

    }

    if (!original){
        dup2(pipeFather,STDOUT_FILENO);
        char msg[50];
        size_t len;
        sprintf(msg,"%d\n%d\n",somaSize,somaBlocks);
        len = strlen(msg);
        recv_pipe_file(somaSize, somaBlocks);
        write(STDOUT_FILENO,msg,len);
    }

    return 0; 
}