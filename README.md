# Sisop-3-2024-MH-IT14
Laporan pengerjaan soal shift modul 3 Praktikum Sistem Operasi 2024 oleh Kelompok IT14
## Praktikan Sistem Operasi Kelompok IT14
1. Tsaldia Hukma Cita          : 5027231036
2. Muhammad Faqih Husain       : 5027231023
3. Muhammad Syahmi Ash Shidqi  : 5027231085

## Soal Shift Modul 3
no 1 

no 2

Catatan dari asisten : untuk fungsi pembagian jika bilangan pertama lebih kecil dari bilangan kedua seharusnya menampilkan nol

no 3

actions.c

```
#include <stdio.h>
#include <string.h>
#include <ctype.h>

char* gap(float jarak) {
    if (jarak < 3.5) {
        return "Gogogo";
    } else if (jarak > 3.5 && jarak <= 10) {
        return "Push";
    } else {
        return "Stay out of trouble";
    }
}

char* fuel(float bensin) {
    if (bensin > 80) {
        return "Push Push Push";
    } else if (bensin >= 50 && bensin <= 80) {
        return "You can go";
    } else {
        return "Conserve Fuel";
    }
}

char* tire(int pemakaian) {
    if (pemakaian > 80) {
        return "Go Push Go Push";
    } else if (pemakaian > 50 && pemakaian <= 80) {
        return "Good Tire Wear";
    } else if (pemakaian > 30 && pemakaian <= 50) {
        return "Conserve Your Tire";
    } else {
        return "Box Box Box";
    }
}

char* tireChange(char* tipe) {
    while(isspace((unsigned char)*tipe)) tipe++;
    if (*tipe == 0)
        return "Invalid tire type";

    char* end = tipe + strlen(tipe) - 1;
    while(end > tipe && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    for(int i = 0; tipe[i]; i++){
        tipe[i] = tolower((unsigned char)tipe[i]);
    }
    if (strcmp(tipe, "soft") == 0) {
        return "Mediums Ready";
    } else if (strcmp(tipe, "medium") == 0) {
        return "Box for Softs";
    } else {
        return "Invalid tire type";
    }
}
```

driver.c

```
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
```

paddock.c

```
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "actions.c"
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


void daemonize() {
    pid_t pid;
    pid = fork();
    if (pid < 0)
        exit(EXIT_FAILURE);
    if (pid > 0)
        exit(EXIT_SUCCESS);
    if (setsid() < 0)
        exit(EXIT_FAILURE);
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    pid = fork();
    if (pid < 0)
        exit(EXIT_FAILURE);
    if (pid > 0)
        exit(EXIT_SUCCESS);
    umask(0);
    chdir("/");
}

int main(int argc, char const *argv[]) {
    daemonize();

    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    char *hello = "Hello from server";
      
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
      
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
      
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while(1) {
        printf("\n+++++++ Waiting for new connection ++++++++\n\n");
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        
        int pid;
        if ((pid = fork()) == 0) {
            close(server_fd);

            memset(buffer, 0, 1024);
            read(new_socket, buffer, 1024);
            printf("%s\n",buffer);
            char* hasil;
            if (strncmp(buffer, "Gap ", 4) == 0) {
            int value = atoi(buffer + 4);
            hasil = gap(value);
            } else if (strncmp(buffer, "Fuel ", 5) == 0) {
            int value = atoi(buffer + 5);
            hasil = fuel(value);
            } else if (strncmp(buffer, "Tire Change ", 12) == 0) {
            char* type = buffer + 12;
            hasil = tireChange(type);
            } else if (strncmp(buffer, "Tire ", 5) == 0) {
            int value = atoi(buffer + 5);
            hasil = tire(value);
            }
            char* prefix;
            char* suffix;
            split_last_word_copy(buffer, &prefix, &suffix);
            write_log("Paddock", prefix, hasil);
            send(new_socket, hasil, strlen(hasil), 0);
            printf("Result message sent\n");
            close(new_socket);
            exit(0);
        } else {
            close(new_socket);
        }
    }

    return 0;
}
```
