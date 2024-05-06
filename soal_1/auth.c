/*

__________                             .__  __     ____   _____  
\______   \__ __  ____ ___.__._____    |__|/  |_  /_   | /  |  | 
 |     ___/  |  \/    <   |  |\__  \   |  \   __\  |   |/   |  |_
 |    |   |  |  /   |  \___  | / __ \_ |  ||  |    |   /    ^   /
 |____|   |____/|___|  / ____|(____  / |__||__|    |___\____   | 
                     \/\/          \/                       |__| 
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_FILENAME_LENGTH 512
#define MAX_FILE_CONTENT_LENGTH 1024
#define MAX_FILES 10
#define SHARED_MEMORY_KEY 4321
#define SHARED_MEMORY_SIZE (sizeof(FileInfo) * MAX_FILES)

typedef struct {
    char filename[MAX_FILENAME_LENGTH];
    char content[MAX_FILE_CONTENT_LENGTH];
} FileInfo;

//Fungsi untuk mengecek apakah nama file sesuai dengan ketentuan atau tidak 
int cek_file(const char *filename) {

    //Cek apakah nama file berakhiran dengan "parkinglot.csv" atau "trashcan.csv"

    if (strstr(filename, "parkinglot.csv") || strstr(filename, "trashcan.csv")) {
        return 1; 
    } else {
        return 0; // File selain csv tidak valid
    }
}

//Fungsi untuk memindahkan file yang lolos autentikasi menuju shared memory 
void file_lolos() {
    DIR *dir;
    struct dirent *entry;

    //Membuka direktori folder yang diinginkan, new-data
    dir = opendir("new-data");
    if (dir == NULL) {
        perror("Error opening directory");
        exit(EXIT_FAILURE);
    }
    
    //Membuat shared memory
    int shmid = shmget(SHARED_MEMORY_KEY, SHARED_MEMORY_SIZE, IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("Error creating shared memory");
        exit(EXIT_FAILURE);
    }
    
    //Menghubungkan shared memory ke ruang alamat proses
    FileInfo *shmaddr = (FileInfo *) shmat(shmid, NULL, 0);
    if (shmaddr == (FileInfo *) -1) {
        perror("Error attaching shared memory");
        exit(EXIT_FAILURE);
    }

    int file_count = 0;
    

    //Looping untuk memindah file menuju shared memory 
    while ((entry = readdir(dir)) != NULL && file_count < MAX_FILES) {
        if (entry->d_type == DT_REG) { 
            char filename[MAX_FILENAME_LENGTH];
            snprintf(filename, MAX_FILENAME_LENGTH, "new-data/%s", entry->d_name);

            //Menyalin nama file dan isi file
            if (cek_file(entry->d_name)) {
                strcpy(shmaddr[file_count].filename, entry->d_name);

                FILE *fp = fopen(filename, "r");
                if (fp == NULL) {
                    perror("Error opening file");
                    exit(EXIT_FAILURE);
                }

                fread(shmaddr[file_count].content, 1, MAX_FILE_CONTENT_LENGTH, fp);

                fclose(fp);

                printf("File berhasil disimpan di shared memory:\t[%s]\n", shmaddr[file_count].filename);

                file_count++;
            } else {

                //File yang tidak sesuai akan dihapus

                printf("File tidak valid, dihapus:\t[%s]\n", entry->d_name);
                
                remove(filename);
            }
        }
    }

    //Melepaskan shared memory
    shmdt((void *) shmaddr);

    closedir(dir);
}

int main() {
    file_lolos();
    return 0;
}
