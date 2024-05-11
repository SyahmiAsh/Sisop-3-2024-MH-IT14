/*

__________                             .__  __     ____   _____  
\______   \__ __  ____ ___.__._____    |__|/  |_  /_   | /  |  | 
 |     ___/  |  \/    <   |  |\__  \   |  \   __\  |   |/   |  |_
 |    |   |  |  /   |  \___  | / __ \_ |  ||  |    |   /    ^   /
 |____|   |____/|___|  / ____|(____  / |__||__|    |___\____   | 
                     \/\/          \/                       |__| 
*/

#include<stdio.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<string.h> 
#include<sys/wait.h> 
#include<time.h>

int add(int a, int b) {
    return a + b;
}

int subtract(int a, int b) {
    return a - b;
}

int multiply(int a, int b) {
    return a * b;
}

int divide(int a, int b) {
    if(b != 0)
        return a / b;
    else
        return -1; // return -1 if division by zero
}

int stringToNumber(char *str) {
    if(strcmp(str, "nol") == 0) return 0;
    if(strcmp(str, "satu") == 0) return 1;
    if(strcmp(str, "dua") == 0) return 2;
    if(strcmp(str, "tiga") == 0) return 3;
    if(strcmp(str, "empat") == 0) return 4;
    if(strcmp(str, "lima") == 0) return 5;
    if(strcmp(str, "enam") == 0) return 6;
    if(strcmp(str, "tujuh") == 0) return 7;
    if(strcmp(str, "delapan") == 0) return 8;
    if(strcmp(str, "sembilan") == 0) return 9;
    return -1; // return -1 jika input tidak valid
}

void numberToString(int num, char *str) {
    if(num < 0) {
        strcpy(str, "ERROR");
        return;
    }
    char *satuan[] = {"nol", "satu", "dua", "tiga", "empat", "lima", "enam", "tujuh", "delapan", "sembilan"};
    char *belasan[] = {"sepuluh", "sebelas", "dua belas", "tiga belas", "empat belas", "lima belas", "enam belas", "tujuh belas", "delapan belas", "sembilan belas"};
    char *puluhan[] = {"", "", "dua puluh", "tiga puluh", "empat puluh", "lima puluh", "enam puluh", "tujuh puluh", "delapan puluh"};

    if(num <= 9) {
        strcpy(str, satuan[num]);
    } else if(num >= 10 && num <= 19) {
        strcpy(str, belasan[num % 10]);
    } else if(num % 10 == 0) {
        strcpy(str, puluhan[num / 10]);
    } else {
        sprintf(str, "%s %s", puluhan[num / 10], satuan[num % 10]);
    }
}

void writeLog(char *operation, char *message) {
    FILE *file = fopen("histori.log", "a+");
    if(file == NULL) {
        printf("Error opening file!\n");
        return;
    }

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    fprintf(file, "[%02d/%02d/%02d %02d:%02d:%02d] [%s] %s\n", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec, operation, message);
    fclose(file);
}

int main(int argc, char *argv[]) 
{ 
    int fd1[2]; // Used to store two ends of first pipe 
    int fd2[2]; // Used to store two ends of second pipe 

    char input_str1[100]; 
    char input_str2[100];
    pid_t p; 

    if (pipe(fd1)==-1) 
    { 
        fprintf(stderr, "Pipe Failed" ); 
        return 1; 
    } 
    if (pipe(fd2)==-1) 
    { 
        fprintf(stderr, "Pipe Failed" ); 
        return 1; 
    } 

    printf("Masukkan dua string:\n");
    scanf("%s", input_str1); 
    scanf("%s", input_str2);

    p = fork(); 

    if (p < 0) 
    { 
        fprintf(stderr, "fork Failed" ); 
        return 1; 
    } 

    // Parent process 
    else if (p > 0) 
    { 
        int num1 = stringToNumber(input_str1);
        int num2 = stringToNumber(input_str2);
        int result;

        if(strcmp(argv[1], "-tambah") == 0) {
            result = add(num1, num2);
        } else if(strcmp(argv[1], "-kurang") == 0) {
            result = subtract(num1, num2);
        } else if(strcmp(argv[1], "-kali") == 0) {
            result = multiply(num1, num2);
        } else if(strcmp(argv[1], "-bagi") == 0) {
            result = divide(num1, num2);
        }

        close(fd1[0]); // Close reading end of first pipe 

        // Write input string and close writing end of first 
        // pipe. 
        write(fd1[1], &result, sizeof(result)); 
        close(fd1[1]); 

        // Wait for child to send a string 
        wait(NULL); 

        close(fd2[1]); // Close writing end of second pipe 

        // Read string from child, print it and close 
        // reading end. 
        char concat_str[100]; 
        read(fd2[0], concat_str, 100); 
        printf("%s\n", concat_str); 
        close(fd2[0]); 
    } 

    // child process 
    else
    { 
        close(fd1[1]); // Close writing end of first pipe 

        // Read a string using first pipe 
        int result;
        read(fd1[0], &result, sizeof(result)); 

        // Convert number to string
        char str[100];
        numberToString(result, str);

        // Close both reading ends 
        close(fd1[0]); 
        close(fd2[0]); 

        // Prepare the operation string
        char operation[20];
        if(strcmp(argv[1], "-tambah") == 0) {
            strcpy(operation, "penjumlahan");
        } else if(strcmp(argv[1], "-kurang") == 0) {
            strcpy(operation, "pengurangan");
        } else if(strcmp(argv[1], "-kali") == 0) {
            strcpy(operation, "perkalian");
        } else if(strcmp(argv[1], "-bagi") == 0) {
            strcpy(operation, "pembagian");
        }

        // Prepare the final string
        char final_str[200];
        sprintf(final_str, "hasil %s %s dan %s adalah %s.", operation, input_str1, input_str2, str);

        writeLog(operation, final_str);

        // Write concatenated string and close writing end 
        write(fd2[1], final_str, strlen(final_str)+1); 
        close(fd2[1]); 

        exit(0); 
    } 
}
