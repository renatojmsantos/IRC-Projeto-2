#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <signal.h>

#define SERVER_PORT 9000
#define BUF_SIZE 1024


typedef struct mail *Mail;
typedef struct mail{
    char *sender;
    char *body;
    int lida;
    Mail next;
}Lemail;


typedef struct list *List;
typedef struct list{
    char *user;
    char *pass;
    int admin;
    Mail *inbox;
    List next;
}Llist;

void escreverFicheiro(List header);
void sigint(int signum);
void process_client(int fd,List header);
void erro(char *msg);
List lerFicheiro();
void insert_list(List list, char nome[],char pass[],int admin,Mail inbox);
List create_list();
void insert_message(Mail mess, char sender[],char body[],int lida);
Mail create_message();



int main() {
	int fd, client;
	struct sockaddr_in addr, client_addr;
	int client_addr_size;
	bzero((void *) &addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(SERVER_PORT);

	//signal(SIGINT, sigint);
	List header=lerFicheiro();

	if ( (fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		erro("na funcao socket");
	if ( bind(fd,(struct sockaddr*)&addr,sizeof(addr)) < 0)
		erro("na funcao bind");
	if( listen(fd, 5) < 0)
		erro("na funcao listen");
	while (1) {
		client_addr_size = sizeof(client_addr);
		client = accept(fd,(struct sockaddr *)&client_addr, (socklen_t *)&client_addr_size);
		if (client > 0) {
			if (fork() == 0) {
				close(fd);
				process_client(client,header);
				exit(0);
			}
			close(client);
		}
	}
	return 0;
}

void process_client(int client_fd,List header)
{
	int nread = 0;
	char username[BUF_SIZE],password[BUF_SIZE];
	char buffer[BUF_SIZE];
	do {

//recebe username
		nread = read(client_fd, username, BUF_SIZE-1);
		fflush(stdout);
		username[nread] = '\0';
		printf("%s\n",username);

//recebe password
		nread = read(client_fd, password, BUF_SIZE-1);
		fflush(stdout);
		password[nread] = '\0';
		printf("%s\n",password);
//verifica se a user/password tá no ficheiro 
		List temp=header->next;
		int matcher=0;
		while(temp!=NULL){
		     	if(strcmp(temp->user,username)==0 && strcmp(temp->pass,password)==0){
				matcher=1;
				break;
			}
     		temp= temp->next;
		}
		
		if(matcher==1){
			sprintf(buffer,"1");
			write(client_fd,buffer, BUF_SIZE-1);
		}
		else
		{
			sprintf(buffer,"0");
			write(client_fd,buffer, BUF_SIZE-1);
		}		
//recebe opçao
		
		int opcao,i;
		List temp_Lista;
		Mail temp_Mail,temp_Mail2,antes,depois;
		//
		do{
			//recebe funcao
			read(client_fd, buffer, BUF_SIZE-1);
			opcao=atoi(buffer);
			fflush(stdout);
			if(opcao==1){
				temp_Lista=temp;
				temp_Mail=(Mail)temp_Lista->inbox;
				temp_Mail=temp_Mail->next;
				if(temp_Mail==NULL){
					write(client_fd,"sem_mensagem", BUF_SIZE-1);
				}
				else{
					write(client_fd,"com_mensagem", BUF_SIZE-1);
					while(temp_Mail!=NULL){
						if(temp_Mail->next==NULL)
							write(client_fd,"fim", BUF_SIZE-1);
						else
							write(client_fd,"mensagem", BUF_SIZE-1);
						if(temp_Mail->lida==0){
							//escreve se é mensagem lida/mensagem nao lida
							write(client_fd,"mensagem_nao_lida", BUF_SIZE-1);
							//esrceve sender
						     	sprintf(buffer,"%s",temp_Mail->sender);
							write(client_fd,buffer, BUF_SIZE-1);
							//escreve mensagem
							sprintf(buffer,"%s",temp_Mail->body);
							write(client_fd,buffer, BUF_SIZE-1);
						
						}
						else
							//escreve se é mensagem lida/mensagem nao lida
							write(client_fd,"mensagem_lida", BUF_SIZE-1);
			     			temp_Mail=temp_Mail->next;
					}
				}
			}
			else if(opcao==2){
				temp_Lista=header->next;
				while(temp_Lista!=NULL){
					if(temp_Lista->next==NULL)
						write(client_fd,"fim", BUF_SIZE-1);
					else
						write(client_fd,"user", BUF_SIZE-1);
				     	sprintf(buffer,"%s",temp_Lista->user);
					write(client_fd,buffer, BUF_SIZE-1);
		     			temp_Lista=temp_Lista->next;
				}
			}
				
			else if(opcao==3){
				char times[BUF_SIZE],body[BUF_SIZE],receiver[BUF_SIZE];
				int n_receivers;

				nread=read(client_fd, times,BUF_SIZE-1);
				fflush(stdout);
				times[nread-1]='\0';
				n_receivers = atoi(times);
				printf("n_receivers->%d\n", n_receivers);

				nread=read(client_fd,body, BUF_SIZE-1);
				fflush(stdout);
				body[nread]=0;
				printf("BODY-> %s\n", body);

				for(i = 0; i < n_receivers; i++){
				    nread = read(client_fd, receiver, BUF_SIZE-1);
				    fflush(stdout);
				    receiver[nread] = '\0';
				    //
				    temp_Lista=header->next;
				    while(temp_Lista!=NULL){
					    if(strcmp(temp_Lista->user,receiver)==0)
						    break;
		     			    temp_Lista=temp_Lista->next;
				    }
				    printf("user->%s body->%s receiver-> %s\n",username,body,temp_Lista->user);
				    insert_message((Mail)temp_Lista->inbox,username, body,0);
				}

			}
			else if(opcao==4){
				temp_Lista=temp;
				temp_Mail=(Mail)temp_Lista->inbox;
				temp_Mail=temp_Mail->next;
				if(temp_Mail==NULL){
					write(client_fd,"sem_mensagem", BUF_SIZE-1);
				}
				else{
					write(client_fd,"com_mensagem", BUF_SIZE-1);
					while(temp_Mail!=NULL){
						if(temp_Mail->next==NULL)
							write(client_fd,"fim", BUF_SIZE-1);
						else
							write(client_fd,"mensagem", BUF_SIZE-1);
						if(temp_Mail->lida==1){
							//escreve se é mensagem lida/mensagem nao lida
							write(client_fd,"mensagem_lida", BUF_SIZE-1);
							//esrceve sender
						     	sprintf(buffer,"%s",temp_Mail->sender);
							write(client_fd,buffer, BUF_SIZE-1);
							//escreve mensagem
							sprintf(buffer,"%s",temp_Mail->body);
							write(client_fd,buffer, BUF_SIZE-1);
						
						}
						else
							//escreve se é mensagem lida/mensagem nao lida
							write(client_fd,"mensagem_nao_lida", BUF_SIZE-1);
			     			temp_Mail=temp_Mail->next;
					}
				}
			}
			else if(opcao==5){
				int eliminar;
				temp_Lista=temp;
				temp_Mail=(Mail)temp_Lista->inbox;
				temp_Mail=temp_Mail->next;
				if(temp_Mail==NULL){
					write(client_fd,"sem_mensagem", BUF_SIZE-1);
				}
				else{
					write(client_fd,"com_mensagem", BUF_SIZE-1);
					while(temp_Mail!=NULL){
						fflush(stdout);
						if(temp_Mail->next==NULL)
							write(client_fd,"fim", BUF_SIZE-1);
						else
							write(client_fd,"mensagem", BUF_SIZE-1);
						//esrceve sender
						sprintf(buffer,"%s",temp_Mail->sender);
						write(client_fd,buffer, BUF_SIZE-1);
						//escreve mensagem
						sprintf(buffer,"%s",temp_Mail->body);
						write(client_fd,buffer, BUF_SIZE-1);

			     			temp_Mail=temp_Mail->next;
					}
					//recebe indice a eliminar
					nread=read(client_fd, buffer,BUF_SIZE-1);
					fflush(stdout);
					buffer[nread-1]='\0';
					eliminar = atoi(buffer);
					printf("eliminar->%d\n", eliminar);
					temp_Lista=temp;
					temp_Mail=(Mail)temp_Lista->inbox;
					temp_Mail=temp_Mail->next;
					if(eliminar==0)
						continue;
					else{
						for(int z=0;z<eliminar-1;z++){
						
							temp_Mail=temp_Mail->next;
						}
						temp_Mail2=(Mail)temp->inbox;
						antes=temp_Mail2;
						temp_Mail2=temp_Mail2->next;
						depois=temp_Mail2->next;
						while(temp_Mail2!=temp_Mail && temp_Mail2->next!=NULL){
							antes=temp_Mail2;
							temp_Mail2=temp_Mail2->next;
							depois=temp_Mail2->next;
						}
						antes->next=depois;
						temp_Mail2->next=NULL;
						temp_Mail2->next=NULL;
					}
				}
				
			}
			else if(opcao==6){
				nread = read(client_fd,buffer, BUF_SIZE-1);
				fflush(stdout);
				buffer[nread] = '\0';
				temp->pass=buffer;
				printf("cliente mudou pass: %s\n",temp->pass);
			}
			else if(opcao==7){
				//manda se é OPER
				sprintf(buffer,"%d",temp->admin);
				write(client_fd,buffer, BUF_SIZE-1);
				//muda para OPER
				temp->admin=1;
			}			
	
			
		}while(opcao!=8);
		printf("cliente saiu\n");
		
		fflush(stdout);
		nread=0;
	} while (nread > 0);
	escreverFicheiro(header);
	close(client_fd);
}





List create_list(){
    List no=(List)malloc(sizeof(Llist));
    no->next=NULL;
    return no;
}

void insert_list(List list, char nome[],char pass[],int admin,Mail inbox){
    List no=(List)malloc(sizeof(Llist));
    no->user=(void*)strdup(nome);
    no->pass=(void*)strdup(pass);
    no->admin=admin;
    no->inbox=(void*)inbox;
    no->next=NULL;
    
    while(list->next!=NULL){
        list=list->next;
    }
    list->next=no;
}

Mail create_message(){
    Mail no=(Mail)malloc(sizeof(Lemail));
    no->next=NULL;
    return no;
}


void insert_message(Mail mess, char sender[],char body[],int lida){
    Mail no=(Mail)malloc(sizeof(Lemail));
    no->sender=(void*)strdup(sender);
    no->body=(void*)strdup(body);
    no->lida=lida;
    no->next=NULL;
    
    while(mess->next!=NULL){
        mess=mess->next;
    }
    mess->next=no;
}


List lerFicheiro()
{
    char buf[120],lixo[120],tempo[120];
    char user[60],message[120],sender[20];
    char password[60];
    int admin,lida;

    FILE * fp;
    List lista=create_list();

    if((fp=fopen("users.txt","r")))
    {
        while(fgets(buf, 120, fp))
        {
	    Mail box=create_message();
            sscanf(buf, "%s %s %d", user,password,&admin);
            insert_list(lista,user,password,admin,box);
            printf("user->%s pass->%s admin->%d\n", user,password,admin);
	    //defenir tempo(proxima flag, para ver se é um user ou mensagem)
	    strcpy(tempo,fgets(lixo, 120, fp));
	    tempo[strlen(tempo) -1] = 0;
	    //ciclo
	    while(strcmp(tempo,"---mensagem---")==0){
		fgets(buf, 120, fp);
		sscanf(buf, "%s %d", sender,&lida);
		fgets(buf, 120, fp);
		sscanf(buf, "%s", message);
		insert_message(box,sender,message,lida);
		printf("\tsender->%s message->%s lida->%d\n",sender,message,lida);
		strcpy(tempo,fgets(lixo, 120, fp));
	    	tempo[strlen(tempo) -1] = 0;
	    }

        }
    }
    fclose(fp);
    return lista;
}

void escreverFicheiro(List header)
{
    List temp_List;
    Mail temp_Mail;
    FILE * fp;

    temp_List=header->next;
    if((fp=fopen("users.txt","w")))
    {
	while(temp_List!=NULL){
		temp_Mail=(Mail)temp_List->inbox;
		temp_Mail=temp_Mail->next;
		fprintf(fp,"%s %s %d\n",temp_List->user,temp_List->pass,temp_List->admin);
			while(temp_Mail!=NULL){
				fprintf(fp,"---mensagem---\n");
				fprintf(fp,"%s %d\n",temp_Mail->sender,temp_Mail->lida);
				fprintf(fp,"%s\n",temp_Mail->body);
				temp_Mail=temp_Mail->next;
			}
		fprintf(fp,"------------------------fim user------------------------\n");
		temp_List=temp_List->next;
	}
    }
    fclose(fp);
}

/*void sigint(int signum) {
	char option[2];
	printf("\n ^C pressed. Do you want to shut the server down? ");
	scanf("%1s", option);
	if (option[0] == 'y') {
		printf("Shutting down!\n");
		exit(0);
	}
}*/

void erro(char *msg)
{
	printf("Erro: %s\n", msg);
	exit(-1);
}
