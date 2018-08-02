#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netinet/tcp.h> // Added this for the definition of SOL_TCP to set NO_DELAY
#include <string.h>
#define PORT 1998
#define BUFLEN 1024

int main(int argc, char* argv[]){
	struct sockaddr_in address;
	int sock=0, one=1,valread,valsend;
	struct sockaddr_in serv_addr;
	char *hello = "(param-disp) ";
	char buffer[BUFLEN];
	if ((sock = socket(AF_INET, SOCK_STREAM,0)) < 0){
		printf("\n Socket creation error \n");
		return -1;
	}
	
	memset(&serv_addr,'0',sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	//Convert IPv4 and IPv6 addresses from text to binary form
	if(inet_pton(AF_INET,"129.93.68.222",&serv_addr.sin_addr)<=0){
		printf("\nInvalid address/ Address not supported \n");
		return -1;
	}

	if(connect(sock,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0){
		printf("\nConnection Failed \n");
		return -1;
	}
	// Added this to attempt to satisfy the setting of the socket option,
	// it appears to have accepted setting the option, but it did not 
	// result in a intelligible response from the laser. 
	//if(setsockopt(sock,SOL_TCP,TCP_NODELAY,&one,sizeof(one)) < 0){
	//	printf("\nSetting Socket Option Failed \n");
	//	return -1;
	//}

	valread = read(sock, buffer, BUFLEN);
	printf("Valread return: %d\n",valread);
	printf("%s\n",buffer);
	valread = read(sock, buffer, BUFLEN);
	printf("Valread return: %d\n",valread);
	printf("%s\n",buffer);
	valread = read(sock, buffer, BUFLEN);
	printf("Valread return: %d\n",valread);
	printf("%s\n",buffer);
	valsend = send(sock, hello, strlen(hello),0);
	printf("Valsend return: %d\n",valsend);

	close(sock);

	return 0;
}
