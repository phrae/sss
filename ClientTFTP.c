#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

//TFTP Client



void messageerror(char ep[4], short int errMessage){
        short int tmp = htons(5);
        memcpy(ep, &tmp, 2);
        tmp = htons(errMessage);
        memcpy(ep+2, &errMessage, 2);
}

void errordecodemas(int r, char code[], char errMessage[128]){
        short int tmp, en ;
        memcpy(&tmp, code+2, 2);
        en = ntohs(tmp);
        switch(en){
                case 0 : strcpy(errMessage, "Not defined");             break;
                case 1 : strcpy(errMessage, "File not found");          break;
                case 2 : strcpy(errMessage, "Access violation");        break;
                case 3 : strcpy(errMessage, "Disk full");               break;
                case 4 : strcpy(errMessage, "Illegal TFTP operation");  break;
                case 5 : strcpy(errMessage, "Unknow port");             break;
                case 6 : strcpy(errMessage, "File already exists");     break;
                case 7 : strcpy(errMessage, "No such user");            break;
        }
}


void DeCodeDP(char dp[516], int r, short int *opcode, short int *blockno, char data[512]){
	short int tmp;
	memcpy(&tmp, dp, 2);
	*opcode = ntohs(tmp);
	memcpy(&tmp, dp+2, 2);
	*blockno = ntohs(tmp);
	memcpy(data, dp+4, r-4); 
}

int checkcode(char blockIN[]){
        short int tmp;
        memcpy(&tmp, blockIN, 2);
        return ntohs(tmp);
}


void codeACK(char ack[4], short blockno){	
	short int tmp;
	tmp = htons(4);
	memcpy(ack, &tmp, 2);
	tmp = htons(blockno);
	memcpy(ack+2, &tmp, 2);
}

int EncodeRW(char bd[139], short int opcode, char filename[128], char mode[9]){

	short int tmp = htons(opcode);
	memcpy(bd, &tmp, 2);
	memcpy(bd+2, filename, strlen(filename)+1);
	memcpy(bd+2+strlen(filename)+1, mode, strlen(mode)+1);

	printf("bd = %d:%s:%s\n", opcode, filename, mode);

	return 2+strlen(filename)+1+strlen(mode)+1;
}

int main(){

	int client_socket = socket(AF_INET, SOCK_DGRAM, 0);
	
	struct sockaddr_in server_address;
	bzero((char*)&server_address, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(9002);
	server_address.sin_addr.s_addr = INADDR_ANY;

	char bd[139];
        int len = EncodeRW(bd, 1, "serverread.txt", "octet");

	sendto(client_socket, bd, len, 0, (struct sockaddr*)&server_address, sizeof(server_address));
	printf("send read req\n");

	char dp[516], data[512];
	short int opcode, blockno;

	//create timeout
	

	int r = recvfrom(client_socket, dp, 516, 0, (struct sockaddr*)0, (int*)0);
	
	//calcelTimeout and checkTimeout
        


	printf("r = %d\n",r);
	if(checkcode(dp)==5){
		//error
		char errMessage[128];
                errordecodemas(r, dp, errMessage);
                printf("!!! got err message : %s!!!\n", errMessage);
		return 1;
	}else if(checkcode(dp)==3){
		DeCodeDP(dp, r, &opcode, &blockno, data);
		printf("got dp block = %d\n", blockno);
		printf("========data==============\n%s\n===============\n", data);
		int fd = open("FileMe.txt", O_WRONLY);
		short int blockno2 = 1;
		short int blockno3;
		short int opcode2;
		if(fd<0){
			//create file
			printf("You have to create file name FileMe.txt 1st\n");
		}
		while(1){
			//printf("r = %d\n", r);
			write(fd, data, r-4);
			if(r<512){
				printf("\n\nDONE\n\n");
				break;
			}
			//memset(data, 0, sizeof(data));
			char ack[4];
			codeACK(ack, blockno2);
			sendto(client_socket, ack, 4, 0, (struct sockaddr*)&server_address, sizeof(server_address));
			printf("send ack block %d\n", blockno2);
	

			//create timeout
			
			r = recvfrom(client_socket, dp, 516, 0, (struct sockaddr*)0, (int*)0);
		
			//cancelTimeout and checkTimeout
        		


			printf("r = %d\n", r);
			if(checkcode(dp)==5){
				//error
				char errMessage[128];
                		errordecodemas(r, dp, errMessage);
                		printf("!!! got err message : %s!!!\n", errMessage);
				return 1;
			}else if(checkcode(dp)==3){
				DeCodeDP(dp, r, &opcode2, &blockno3, data);
				printf("got dp block = %d\n", blockno3);
				printf("========data==============\n%s\n===============\n", data);
				blockno2++;
			}
		}
	}

	//printf("@@@ test @@@\n\n");
	close(client_socket);

	return 0;
}
