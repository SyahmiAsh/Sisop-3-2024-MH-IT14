/*

__________                             .__  __     ____   _____  
\______   \__ __  ____ ___.__._____    |__|/  |_  /_   | /  |  | 
 |     ___/  |  \/    <   |  |\__  \   |  \   __\  |   |/   |  |_
 |    |   |  |  /   |  \___  | / __ \_ |  ||  |    |   /    ^   /
 |____|   |____/|___|  / ____|(____  / |__||__|    |___\____   | 
                     \/\/          \/                       |__| 
*/

#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#define PORT 8080

void split_last_word_copy(const char* buffer, char** prefix, char** suffix) {
    char* buffer_copy = strdup(buffer);
    char* last_space = strrchr(buffer_copy, ' ');
    if (last_space != NULL) {
        *last_space = '\0';
        *prefix = strdup(buffer_copy);
        *suffix = strdup(last_space + 1);
    } else {
        *prefix = NULL;
        *suffix = strdup(buffer_copy);
    }
    free(buffer_copy);
    size_t len = strlen(*suffix);
    if (len > 0 && (*suffix)[len - 1] == '\n') {
        (*suffix)[len - 1] = '\0';
    }
}

void write_log(const char* source, const char* command, const char* additional_info) {
    FILE* log_file = fopen("/home/kali/Sisop/modul3/soal_3/server/race.log", "a+");
    if (log_file == NULL) {
        perror("Failed to open log file");
        return;
    }

    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];

    time (&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", timeinfo);
    fprintf(log_file, "[%s] [%s]: [%s] [%s]\n", source, buffer, command, additional_info);

    fclose(log_file);
}

  
int main(int argc, char const *argv[]) {
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    char hello[1024];

    printf("Enter your message: ");
    fgets(hello, 1024, stdin); // read the message from user

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
  
    memset(&serv_addr, '0', sizeof(serv_addr));
  
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
      
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
  
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }
    char* prefix;
    char* suffix;
    split_last_word_copy(hello, &prefix, &suffix);
    write_log("Driver", prefix, suffix);
    send(sock , hello , strlen(hello) , 0 );
    valread = read( sock , buffer, 1024);
    printf("%s\n",buffer );
    return 0;
}

