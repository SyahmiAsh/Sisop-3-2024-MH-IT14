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

#define MAX_FILENAME_LENGTH 512
#define MAX_FILE_CONTENT_LENGTH 1024
#define MAX_FILES 10
#define SHARED_MEMORY_KEY 4321
#define SHARED_MEMORY_SIZE (sizeof(FileInfo) * MAX_FILES)

typedef struct {
    char filename[MAX_FILENAME_LENGTH];
    char content[MAX_FILE_CONTENT_LENGTH];
} FileInfo;

//Fungsi untuk memisahkan nama dan rating dari konten file
void nama_rating(const char *content, char *name, float *rating) {
    sscanf(content, "%[^,], %f", name, rating);
}

//Fungsi untuk membandingkan dua entri berdasarkan rating untuk pengurutan
int rating_max(const void *a, const void *b) {
    float rating_a = *(float *)a;
    float rating_b = *(float *)b;
    return (rating_a < rating_b) - (rating_a > rating_b);
}

//Fungsi untuk mencetak rating tertinggi setelah diurutkan
void print_ratingmx(const char *filename, const char *content) {

    // Memisahkan dan menyimpan rating dari konten file
    char temp_content[MAX_FILE_CONTENT_LENGTH];
    strcpy(temp_content, content); 

    float highest_rating = 0.0;
    char highest_rated_name[MAX_FILENAME_LENGTH] = "";

    char *token = strtok(temp_content, "\n"); 
    token = strtok(NULL, "\n"); 
   
    //Looping untuk mencari rating tertinggi
    while (token != NULL) {
        char name[MAX_FILENAME_LENGTH];
        float rating;
        nama_rating(token, name, &rating);

        if (rating > highest_rating) {
            highest_rating = rating;
            strcpy(highest_rated_name, name);
        }

        token = strtok(NULL, "\n");
    }


    printf("Type: %s\n", strstr(filename, "parkinglot") ? "Parking Lot" : "Trash Can");
    printf("Filename: %s\n", filename);
    printf("----------------------\n");
    printf("Name: %s\n", highest_rated_name);
    printf("Rating: %.1f\n", highest_rating);
    printf("--------------------------------\n\n");
}


int main() {
    //Mendapatkan akses ke shared memory yang sama
    int shmid = shmget(SHARED_MEMORY_KEY, SHARED_MEMORY_SIZE, 0666);
    if (shmid < 0) {
        perror("Error accessing shared memory");
        exit(EXIT_FAILURE);
    }

    //Menghubungkan shared memory ke ruang alamat proses
    FileInfo *shmaddr = (FileInfo *) shmat(shmid, NULL, 0);
    if (shmaddr == (FileInfo *) -1) {
        perror("Error attaching shared memory");
        exit(EXIT_FAILURE);
    }

    //Membaca dan mencetak rating tertinggi dari isi file di shared memory
    for (int i = 0; i < MAX_FILES; ++i) {
        if (strlen(shmaddr[i].filename) > 0) {
            print_ratingmx(shmaddr[i].filename, shmaddr[i].content);
        }
    }

    //Melepaskan shared memory
    shmdt((void *) shmaddr);

    return 0;
}
