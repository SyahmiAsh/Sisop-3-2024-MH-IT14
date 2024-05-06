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
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#define MAX_FILENAME_LENGTH 512
#define MAX_FILE_CONTENT_LENGTH 1024
#define MAX_FILES 10
#define SHARED_MEMORY_KEY 4321
#define SHARED_MEMORY_SIZE (sizeof(FileInfo) * MAX_FILES)

typedef struct {
    char filename[MAX_FILENAME_LENGTH];
    char content[MAX_FILE_CONTENT_LENGTH];
} FileInfo;

//Fungsi untuk menentukan tipe file dari judul file
void tipe_file(const char *filename, char *type) {
    if (strstr(filename, "trashcan") || strstr(filename, "TrashCan")) {
        strcpy(type, "Trash Can");
    } else if (strstr(filename, "parkinglot") || strstr(filename, "ParkingLot")) {
        strcpy(type, "Parking Lot");
    } 
}


//Fungsi untk mencatat ke dalam file db.log
void catat_log(const char *filename, const char *type) {
    time_t current_time;
    struct tm *local_time;
    char time_string[80];

    current_time = time(NULL);
    local_time = localtime(&current_time);

    strftime(time_string, sizeof(time_string), "[%d/%m/%Y %H:%M:%S]", local_time);

    //Untuk mencari dan membuka file db.log
    FILE *log_file = fopen("database/db.log", "a");
    if (log_file == NULL) {
        log_file = fopen("database/db.log", "w");

        if (log_file == NULL) {
            perror("Error creating log file");
            exit(EXIT_FAILURE);
        }

        fprintf(log_file, "Log File Created\n");
    }

    fprintf(log_file, "%s\t[%s]\t[%s]\n", time_string, type, filename);

    fclose(log_file);
}

int main() {
    //Mendapatkan akses ke shared memory yang sama
    int shmid = shmget(SHARED_MEMORY_KEY, SHARED_MEMORY_SIZE, 0666);
    if (shmid < 0) {
        perror("Error accessed shared memory");
        exit(EXIT_FAILURE);
    }

    //Menghubungkan shared memory ke ruang alamat proses
    FileInfo *shmaddr = (FileInfo *) shmat(shmid, NULL, 0);
    if (shmaddr == (FileInfo *) -1) {
        perror("Error attaching shared memory");
        exit(EXIT_FAILURE);
    }

    //Mendapatkan jalur direktori saat ini
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("Error getting current directory");
        exit(EXIT_FAILURE);
    }

    //Menambahkan "/database" ke jalur direktori saat ini
    strcat(cwd, "/database");

    //Membuat folder database jika belum ada
    struct stat st = {0};
    if (stat(cwd, &st) == -1) {
        mkdir(cwd, 0700);
    }

    //Menyalin file dari shared memory ke folder database
    for (int i = 0; i < MAX_FILES; ++i) {
        if (strlen(shmaddr[i].filename) > 0) {
            char dest_path[MAX_FILENAME_LENGTH + 1024]; 

            snprintf(dest_path, sizeof(dest_path), "%s/%s", cwd, shmaddr[i].filename);

            FILE *fp = fopen(dest_path, "w");

            if (fp == NULL) {
                perror("Error creating file");
                continue;
            }

            fwrite(shmaddr[i].content, 1, strlen(shmaddr[i].content), fp);

            fclose(fp);

            printf("Berhasil memindah file ke database : %s\n", shmaddr[i].filename);

            //Menentukan jenis berdasarkan nama file
            char type[MAX_FILENAME_LENGTH];
            tipe_file(shmaddr[i].filename, type);

            //Mencatat log
            catat_log(shmaddr[i].filename, type);

            //Menghapus file asli dari folder new-data setelah menyalinnya
            char source_path[MAX_FILENAME_LENGTH + 1000];

            snprintf(source_path, sizeof(source_path), "../new-data/%s", shmaddr[i].filename);

            if (remove(source_path) != 0) {
                perror("Error deleting original file");
            }

        }
    }

    //Melepaskan shared memory
    shmdt((void *) shmaddr);

    return 0;
}
