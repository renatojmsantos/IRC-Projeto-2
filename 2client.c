#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <termios.h>
#include <signal.h>

#define BUF_SIZE 1024
#define BACKSPACE 127

char user[21];
char pazz[21];

void erro(char *msg);
void menu(int fd);
void change_password();
void change_permission(int fd);

int main(int argc, char *argv[]) {
	char endServer[100];
	int fd;
	char buffer[BUF_SIZE];
	int nread=0;
	struct sockaddr_in addr;
	struct hostent *hostPtr;
	if (argc != 3) {
		printf("cliente <host> <port>\n");
		exit(-1);
	}
	strcpy(endServer, argv[1]);
	if ((hostPtr = gethostbyname(endServer)) == 0)
		erro("Nao consegui obter endereço");
	bzero((void *) &addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = ((struct in_addr *)(hostPtr->h_addr))->s_addr;
	addr.sin_port = htons((short) atoi(argv[2]));
	if((fd = socket(AF_INET,SOCK_STREAM,0)) == -1)
		erro("socket");
	if( connect(fd,(struct sockaddr *)&addr,sizeof (addr)) < 0)
		erro("Connect");
//username

	printf("Username: ");

	fgets(user,sizeof(user),stdin);
	user[strlen(user) -1] = 0;
	write(fd,user,strlen(user));
	fflush(stdout);
//password
	printf("Password: ");

	my_getpass(pazz, sizeof(pazz), stdin);
	//pazz[strlen(pazz) -1] = 0;
	//fgets(buffer,100,stdin);
	write(fd,pazz,strlen(pazz));
	fflush(stdout);	
//verifica se user/pass estao bem
	
	
	nread = read(fd,buffer, BUF_SIZE-1);
	fflush(stdout);
	buffer[nread] = '\0';
	if(atoi(buffer)==1)
		menu(fd);
	else
		printf("Wrong User/Pass\n");

	close(fd);
	exit(0);
}

void menu(int fd){
	int num,i;
	int nread;
	char buffer[BUF_SIZE],fim[BUF_SIZE];
	char n_receivers[10];
	char receiver[61];
	char message[4096];

	do{
//print menu
		printf(" 1)LIST_MESS\n 2)LIST_USERS\n 3)SEND_MESS\n 4)LIST_READ\n 5)REMOVE_MES\n 6)CHANGE_PASSW\n 7)OPER \n 8)QUIT\n\nOPCAO: ");
		
		scanf("%d",&num);
		getchar();
//switch

		switch(num) {

			case 1 :
				//escreve opcao
				write(fd,"1", strlen("1") + 1);
                		fflush(stdin);
				//
				printf("-----MENSAGENS NAO LIDAS-----\n");
				//le se tem mensagens
				nread = read(fd,fim, BUF_SIZE-1);
				fflush(stdout);
				fim[nread] = '\0';
				if(strcmp(fim,"sem_mensagem")==0)
					printf("\n");
				else{
					do{
						//le fim
						nread = read(fd,fim, BUF_SIZE-1);
						fflush(stdout);
						fim[nread] = '\0';
						//le tipo de mensagem
						nread = read(fd,buffer, BUF_SIZE-1);
						fflush(stdout);
						buffer[nread] = '\0';
						if(strcmp(buffer,"mensagem_nao_lida")==0){
							//escreve sender
							nread = read(fd,buffer, BUF_SIZE-1);
							fflush(stdout);
							buffer[nread] = '\0';
							printf("From [%s]: ",buffer);
							//escreve mensagem
							nread = read(fd,buffer, BUF_SIZE-1);
							fflush(stdout);
							buffer[nread] = '\0';
							printf("%s\n",buffer);
						}
					}while(strcmp(fim,"fim")!=0);
				}
				printf("\n");
				break; 
	
			case 2  :
				i=1;
				//escreve opcao
				write(fd,"2", strlen("2")+1);
                		fflush(stdin);
				//
				printf("-----USERS-----\n");
				do{
					//le fim
					nread = read(fd,fim, BUF_SIZE-1);
					fflush(stdout);
					fim[nread] = '\0';
					//le nome
					nread = read(fd,buffer, BUF_SIZE-1);
					fflush(stdout);
					buffer[nread] = '\0';
					printf("%d. ",i);
					printf("%s\n",buffer);
					i++;
				}while(strcmp(fim,"fim")!=0);
				printf("\n");
				break; 

			case 3  :
				//escreve opcao
				write(fd,"3", strlen("3") + 1);
                		fflush(stdin);
				//pede user
				

				printf("Numero de Destinatarios: ");
				fgets(n_receivers,10,stdin);
				n_receivers[strlen(n_receivers)-1]=0;
				write(fd, n_receivers, strlen(n_receivers)+1);
				fflush(stdin);

				printf("Mensagem: ");
				fgets(message, 4096, stdin);
				message[strlen(message)-1]=0;
				write(fd, message, strlen(message)+1);
				fflush(stdin);

				for(i=0;i<atoi(n_receivers);i++){
					printf("Destinarios [%d]: ", i+1);
					fgets(receiver, 60,stdin);
					receiver[strlen(receiver)-1]=0;
					write(fd, receiver, strlen(receiver)+1);
					fflush(stdin);
				}
				break; 
	
			case 4  :
				//escreve opcao
				write(fd,"4", strlen("4") + 1);
                		fflush(stdin);
				//
				printf("-----MENSAGENS LIDAS-----\n");
				//le se tem mensagens
				nread = read(fd,fim, BUF_SIZE-1);
				fflush(stdout);
				fim[nread] = '\0';
				if(strcmp(fim,"sem_mensagem")==0)
					printf("\n");
				else{
					do{
						//le fim
						nread = read(fd,fim, BUF_SIZE-1);
						fflush(stdout);
						fim[nread] = '\0';
						//le tipo de mensagem
						nread = read(fd,buffer, BUF_SIZE-1);
						fflush(stdout);
						buffer[nread] = '\0';
						if(strcmp(buffer,"mensagem_lida")==0){
							//escreve sender
							nread = read(fd,buffer, BUF_SIZE-1);
							fflush(stdout);
							buffer[nread] = '\0';
							printf("From [%s]: ",buffer);
							//escreve mensagem
							nread = read(fd,buffer, BUF_SIZE-1);
							fflush(stdout);
							buffer[nread] = '\0';
							printf("%s\n",buffer);
						}
					}while(strcmp(fim,"fim")!=0);
				}
				printf("\n");
				break; 
		
			case 5  :
				i=1;
				//escreve opcao
				write(fd,"5", strlen("5") + 1);
                		fflush(stdin);
				//
				printf("-----MENSAGENS-----\n");
				//le se tem mensagens
				nread = read(fd,fim, BUF_SIZE-1);
				fflush(stdout);
				fim[nread] = '\0';
				if(strcmp(fim,"sem_mensagem")==0)
					printf("\n");
				else{
					printf("0. sair\n");
					do{
						//le fim
						nread = read(fd,fim, BUF_SIZE-1);
						fflush(stdout);
						fim[nread] = '\0';
						//print do indice da mensagem
						printf("%d. ",i);
						i++;
						//escreve sender
						nread = read(fd,buffer, BUF_SIZE-1);
						fflush(stdout);
						buffer[nread] = '\0';
						printf("From [%s]: ",buffer);
						//escreve mensagem
						nread = read(fd,buffer, BUF_SIZE-1);
						fflush(stdout);
						buffer[nread] = '\0';
						printf("%s\n",buffer);
					}while(strcmp(fim,"fim")!=0);
					printf("\n");
					//pede o indice da mensagem a eliminar
					//printf("A eliminar: ");

					printf("A eliminar: ");
					fgets(n_receivers,10,stdin);
					n_receivers[strlen(n_receivers)-1]=0;
					write(fd, n_receivers, strlen(n_receivers)+1);
					fflush(stdin);
				}
				break; 
	
			case 6  :
				printf("option 6\n");
				//escreve opcao
				write(fd,"6", strlen("6") + 1);
                		fflush(stdin);	
				//func
				change_password();
				//manda pass nova
				write(fd,pazz, strlen(pazz) + 1);
                		fflush(stdin);
				break; 
		
			case 7  :
				//escreve opcao
				write(fd,"7", strlen("7") + 1);
                		fflush(stdin);
				//func
				change_permission(fd);
				//
				break; 
	
			case 8  :
				//escreve opcao
				write(fd,"8", strlen("8") + 1);
                		fflush(stdin);
				printf("option quit\n");
				break; 

			default : 
				printf("non-existing option\n");
				break;

		}
	}while(num!=8);
}

void change_password(){
	char pass1[20],pass2[20];
//inserir pass nova
	printf("Nova pass: ");
	fgets(pass1,sizeof(pass1),stdin);
	pass1[strlen(pass1)] = 0;
//confirmar pass nova
	printf("Confirmar nova pass: ");
	fgets(pass2,sizeof(pass2),stdin);
	pass2[strlen(pass2)] = 0;
//verificar se sao iguais
	if(strcmp(pass1,pass2)==0){
		strcpy(pazz,pass1);
		pazz[strlen(pazz) -1] = 0;
	}
	else
		printf("Passwords dont match\n");
}

void change_permission(int fd){
	int nread;
	char buffer[BUF_SIZE];
	//recebe se o user é OPER ou nao
	nread = read(fd,buffer, BUF_SIZE-1);
	fflush(stdout);
	buffer[nread] = '\0';
	//
	if(atoi(buffer)==1)
		printf("O user já é OPER\n\n");	
}


void erro(char *msg)
{
	printf("Erro: %s\n", msg);
	exit(-1);
}


ssize_t my_getpass (char *lineptr, size_t len, FILE *stream)
{
    struct termios old, new;
    int nread = 0;
    char c;

    /* Turn echoing off and fail if we can't. */
    if (tcgetattr (fileno (stream), &old) != 0)
        return -1;
        new = old;
        new.c_lflag &= ~ECHO || ECHOCTL;
    if (tcsetattr (fileno (stream), TCSAFLUSH, &new) != 0)
        return -1;
    /* Read the password. */
    while ((c = getchar()) != '\n' && nread + 1 < len) {
        if (c == BACKSPACE) {
            if (nread > 0) {
                nread--;
                printf("\b \b");
            }
        } else {
                lineptr[nread++] = c;
                printf("*");
                }
    }
    printf("\n");
    lineptr[nread] = '\0';

    /* Restore terminal. */
    (void) tcsetattr (fileno (stream), TCSAFLUSH, &old);
    return nread;
}
