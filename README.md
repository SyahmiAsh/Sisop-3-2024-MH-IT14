# Sisop-3-2024-MH-IT14
Laporan pengerjaan soal shift modul 3 Praktikum Sistem Operasi 2024 oleh Kelompok IT14
## Praktikan Sistem Operasi Kelompok IT14
1. Tsaldia Hukma Cita          : 5027231036
2. Muhammad Faqih Husain       : 5027231023
3. Muhammad Syahmi Ash Shidqi  : 5027231085

## Soal Shift Modul 3
## Soal 1
Pada zaman dahulu pada galaksi yang jauh-jauh sekali, hiduplah seorang Stelle. Stelle adalah seseorang yang sangat tertarik dengan Tempat Sampah dan Parkiran Luar Angkasa. Stelle memulai untuk mencari Tempat Sampah dan Parkiran yang terbaik di angkasa. Dia memerlukan program untuk bisa secara otomatis mengetahui Tempat Sampah dan Parkiran dengan rating terbaik di angkasa. Programnya berbentuk microservice sebagai berikut:
1. Dalam auth.c pastikan file yang masuk ke folder new-entry adalah file csv dan berakhiran  trashcan dan parkinglot. Jika bukan, program akan secara langsung akan delete file tersebut. 
Contoh dari nama file yang akan diautentikasi:
--- belobog_trashcan.csv
--- osaka_parkinglot.csv
2. Format data (Kolom)  yang berada dalam file csv adalah seperti berikut:
![Screenshot (623)](https://github.com/SyahmiAsh/Sisop-3-2024-MH-IT14/assets/150339585/8f164a98-0c7b-470d-b9ff-88fbb568f042)
![Screenshot (623) - Copy](https://github.com/SyahmiAsh/Sisop-3-2024-MH-IT14/assets/150339585/91521ed5-dbed-443c-9807-f6a1517728cb)

3. File csv yang lolos tahap autentikasi akan dikirim ke shared memory. 

4. Dalam rate.c, proses akan mengambil data csv dari shared memory dan akan memberikan output Tempat Sampah dan Parkiran dengan Rating Terbaik dari data tersebut.
![Screenshot (624)](https://github.com/SyahmiAsh/Sisop-3-2024-MH-IT14/assets/150339585/326834d3-e6a3-4b99-9637-f5f39e3b1522)
5. Pada db.c, proses bisa memindahkan file dari new-data ke folder microservices/database, WAJIB MENGGUNAKAN SHARED MEMORY.
6. Log semua file yang masuk ke folder microservices/database ke dalam file db.log dengan contoh format sebagai berikut:
[DD/MM/YY hh:mm:ss] [type] [filename]
contoh : 
[07/04/2024 08:34:50] [Trash Can] [belobog_trashcan.csv]


### Penyelesaian
Untuk penyelesaian soal nomor 1 ini membutuhkan 3 program, yaitu `auth.c` , `auth.c` , dan `rate.c`

### auth.c
Secara garis besar program ini memiliki fitur sebagai berikut:
- Mengautentikasi file yang berada di direktori `/new-data` berdasarkan nama file yang sesuai, apabila file tidak lolos autentikasi maka file akan secara otomatis dihapus
- File yang lolos autentikasi akan dikirimkan menuju shared memory 

```
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
```
`struct` berguna untuk menyimpan data dalam bentuk array dan memudahkan ketika proses pemindahan data menuju shared memory 

Kode berikut merupakan fungsi untuk mengecek apakah nama file yang ada di dalam direktori `/new-data` sudah sesuai dengan ketentuan atau belum, yaitu file dengan akhiran `parkinglot.csv` atau `trashcan.csv`

```
int cek_file(const char *filename) {

    //Cek apakah nama file berakhiran dengan "parkinglot.csv" atau "trashcan.csv"
    if (strstr(filename, "parkinglot.csv") || strstr(filename, "trashcan.csv")) {
        return 1; 
    } else {
        return 0; // File selain csv tidak valid
    }
}
```

Kode berikut ini merupakan fungsi untuk membuat dan mengakses shared memory 
```
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
```

Kode berikut merupakan fungsi untuk memindahkan file dari direktori `/new-data` yang sesuai beserta data didalamnya menuju ke dalam shared memory dengan menggunakan looping hingga semua file telah selesai dipindah. Sedangkan untuk file yang tidak sesuai maka akan secara otomatis dihapus oleh program dengan `remove`. Pertamanya program akan membuka file, lalu membaca dan menyalin file lalu memasukkannya ke dalam shared memory
```
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
```

### db.c
Secara garis besar program ini memiliki fitur sebagai berikut:
- Menyalin file dari shared memory kedalam direktori `/database`
- Mencatat log penyalinan file kedalam file db.log di dalam direktori `/database`
- Menghapus file asli yang berada di direktori `/new-data`

```
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
```
`struct` berguna untuk menyimpan data dalam bentuk array dan memudahkan ketika proses pemindahan data dari shared memory menuju direktori `/database`

Berikut ini merupakan fungsi untuk menentukan tipe file berdasarkan nama file, jika file memiliki `trashcan` maka file akan disimpan sebagai tipe`Trash Can`, dan jika file memiliki `parkinglot` maka file akan disimpan sebagai tipe `Parking Lot`
```
void tipe_file(const char *filename, char *type) {
    if (strstr(filename, "trashcan") || strstr(filename, "TrashCan")) {
        strcpy(type, "Trash Can");
    } else if (strstr(filename, "parkinglot") || strstr(filename, "ParkingLot")) {
        strcpy(type, "Parking Lot");
    } 
}
```

Berikut ini merupakan fungsi untuk mencatat proses penyalinan file dari shared memory menuju direktori `./database` kedalam file db.log yang berada di dalam direktori `./database/db.log`
-- Proses ini akan mencari waktu real dengan lokasi saat ini
-- Mencari db.log, jika tidak ada/belum dibuat maka akan otomatis membuat file db.log, jika sudah tersedia maka akan dibuka dan memunculkan pesan catatan sesuai dengan format 
```
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
```

Berikut ini merupakan proses untuk menyalin file yang ada di dalam shared memory menuju direktori `./database`, setelah berhasil memindah maka program akan memanggil proses pencatatan ke dalam file db.log, serta menghapus file asli yang berada di direktori `./new-data`
```
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
```

### rate.c
Secara garis besar program ini memiliki fitur sebagai berikut:
- Mengurutkan data berdasarkan rating tertinggi 
- Menampilkan output berupa data dengan rating tertinggi untuk tiap file `Trashcan` dan `Parkinglot`
 

```
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
```
`struct` berguna untuk menyimpan data dalam bentuk array dan memudahkan ketika proses pemindahan data menuju shared memory 

Berikut ini merupakan fungsi untuk memisahkan antara nama dan rating yang dibedakan oleh koma `,`
```
void nama_rating(const char *content, char *name, float *rating) {
    sscanf(content, "%[^,], %f", name, rating);
}
```

Berikut merupakan fungsi untuk membandingkan rating dan mencari nilai rating tertinggi 
```
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
```

Ini merupakan fungsi utama untuk mengakses shared memory yang sama dengan program yang lain lalu memanggil fungsi-fungsi di atas 
```
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
```

### Error Dalam Pengerjaan
Terjadi eror saat memindahkan file dari shared memory menuju direktori `./database`
![Screenshot (620)](https://github.com/SyahmiAsh/Sisop-3-2024-MH-IT14/assets/150339585/c0b27700-25f3-44e1-b1b7-cad5dafe4dcb)

Terjadi eror tidak terbuat shared memory dan tidak terdeteksi saat dicek dengan `ipcs`
![Screenshot (618)](https://github.com/SyahmiAsh/Sisop-3-2024-MH-IT14/assets/150339585/ad77fb0a-69a4-4cc2-bfd4-6a91a360b8af)

### Dokumentasi Running
##### Direktori sebelum menjalankan program `auth.c` dan `db.c`
![Screenshot (625)](https://github.com/SyahmiAsh/Sisop-3-2024-MH-IT14/assets/150339585/ac4debba-aa06-4b23-9d07-121c194d65f8)
![Screenshot (626)](https://github.com/SyahmiAsh/Sisop-3-2024-MH-IT14/assets/150339585/18e27066-5385-42f7-85e3-2588a3127a87)
![Screenshot (629)](https://github.com/SyahmiAsh/Sisop-3-2024-MH-IT14/assets/150339585/1e3e3079-5bd6-40c0-bfac-50095deddb81)
![Screenshot (630)](https://github.com/SyahmiAsh/Sisop-3-2024-MH-IT14/assets/150339585/093e0193-87c6-46b3-b616-0ecb34186e4f)
![Screenshot (631)](https://github.com/SyahmiAsh/Sisop-3-2024-MH-IT14/assets/150339585/2dac437f-4232-4345-a024-3df302b1705a)
![Screenshot (632)](https://github.com/SyahmiAsh/Sisop-3-2024-MH-IT14/assets/150339585/7ecf5563-4f48-4bf7-b671-4bfa86b47d3d)
![Screenshot (628)](https://github.com/SyahmiAsh/Sisop-3-2024-MH-IT14/assets/150339585/7437862b-81da-4087-be05-32daa47c4fd6)



##### Menjalankan `auth.c`
![Screenshot (634)](https://github.com/SyahmiAsh/Sisop-3-2024-MH-IT14/assets/150339585/ee4de861-c154-4784-9857-742d1e67c277)
File yang tidak sesuai akan dihapus dari direktori `./new-data`
![Screenshot (636)](https://github.com/SyahmiAsh/Sisop-3-2024-MH-IT14/assets/150339585/15e85588-c256-4ff1-983c-e9de57cad83a)


##### Menjalankan `db.c`
![Screenshot (634) - Copy](https://github.com/SyahmiAsh/Sisop-3-2024-MH-IT14/assets/150339585/9e80274b-e60b-46a1-9798-638c36c1ad27)
File disalin dari shared memori menuju ke dalam direktori `./database`
![Screenshot (633)](https://github.com/SyahmiAsh/Sisop-3-2024-MH-IT14/assets/150339585/2250dfe6-8b28-4037-a4be-6d97ff86b657)


##### Catatan log dalam file db.log
![Screenshot (637)](https://github.com/SyahmiAsh/Sisop-3-2024-MH-IT14/assets/150339585/0a1458ed-4064-4edc-a926-0c8bf0188ada)

##### Menjalankan `rate.c`
![Screenshot (635)](https://github.com/SyahmiAsh/Sisop-3-2024-MH-IT14/assets/150339585/6200bcc0-607e-4532-ad98-a7a49ee89359)


##### Direktori sesudah menjalankan program `auth.c` dan `db.c`
![Screenshot (625)](https://github.com/SyahmiAsh/Sisop-3-2024-MH-IT14/assets/150339585/ac4debba-aa06-4b23-9d07-121c194d65f8)
File di dalam direktori `./new-data` telah dihapus oleh program `db.c`
![Screenshot (639)](https://github.com/SyahmiAsh/Sisop-3-2024-MH-IT14/assets/150339585/94559446-99f7-458b-a422-731e9fc2018b)
![Screenshot (632)](https://github.com/SyahmiAsh/Sisop-3-2024-MH-IT14/assets/150339585/7ecf5563-4f48-4bf7-b671-4bfa86b47d3d)
File berhasil disalin menuju direktori `./database`
![Screenshot (638)](https://github.com/SyahmiAsh/Sisop-3-2024-MH-IT14/assets/150339585/1ffc56bd-26c0-409d-9250-5816d12fcd26)



### Revisi
Tidak ada revisi atau catatan dari asisten lab praktikum

## Soal 2
by Muhammad Faqih Husain

Max Verstappen 🏎️ seorang pembalap F1 dan programer memiliki seorang adik bernama Min Verstappen (masih SD) sedang menghadapi tahap paling kelam dalam kehidupan yaitu perkalian matematika, Min meminta bantuan Max untuk membuat kalkulator perkalian sederhana (satu sampai sembilan). Sembari Max nguli dia menyuruh Min untuk belajar perkalian dari web (referensi) agar tidak bergantung pada kalkulator.
(Wajib menerapkan konsep pipes dan fork seperti yang dijelaskan di modul Sisop. Gunakan 2 pipes dengan diagram seperti di modul 3).

```
#include<stdio.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<string.h> 
#include<sys/wait.h> 
#include<time.h>
```
Fungsi penjumlahan
```
int add(int a, int b) {
    return a + b;
}

```
Fungsi pengurangan
```
int subtract(int a, int b) {
    return a - b;
}
```
fungsi perkalian
```

int multiply(int a, int b) {
    return a * b;
}
```
Fungsi pembagian yang dibagi kedalam beberapa kasus, pembagian akan dibulatkan kebawah sehingga digunakan bilangan integer untuk melakukan pembagian
```
int divide(int a, int b) {
    if(a < b){
        return 0;
    }
    if(b != 0)
        return a / b;
    else
        return -1; // return -1 if division by zero
}
```
Melakukan konversi angka dari string ke angka mulai dari nol sampai dengan Sembilan sesuai Batasan soal
```

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
```
Mengubah hasil perkalian dalam bentuk angka menjadi string, fungsi ini dapat menghandle mulai dari nol sampai dengan delapan puluh Sembilan karena bilangan maksimal yang dapat dihasilkan dari kalkulator ini adalah delapan puluh satu. Fungsi ini bekerja dengan memisahkan satuan, belasan, dan puluhan lalu mengkombinasikan mereka sesuai angka yang akan dikonversi.
```

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
```
Menuliskan log dengan format

Format: [date] [type] [message]

Type: KALI, TAMBAH, KURANG, BAGI

Ex:

[10/03/24 00:29:47] [KALI] tujuh kali enam sama dengan empat puluh dua.
[10/03/24 00:30:00] [TAMBAH] sembilan tambah sepuluh sama dengan sembilan belas.
[10/03/24 00:30:12] [KURANG] ERROR pada pengurangan

```

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
```
Berikut ini merupakan fungsi utama
```

int main(int argc, char *argv[]) 
{ 
```
Persiapan untuk melakukan pipe and fork
```
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
```
Memasukkan input yaitu angka yang akan dihitung menggunakan kalkulator

dengan format  angka_pertama angka_kedua

misal : tiga tujuh
```

    printf("Masukkan dua string:\n");
    scanf("%s", input_str1); 
    scanf("%s", input_str2);

    p = fork(); 

    if (p < 0) 
    { 
        fprintf(stderr, "fork Failed" ); 
        return 1; 
    } 
    else if (p > 0) 
    { 
```
Pada parent proses string diubah menjadi integer
```
        int num1 = stringToNumber(input_str1);
        int num2 = stringToNumber(input_str2);
        int result;
```
Kemudian input diproses sesuai dengan argument saat menjalankan fungsi
```

        if(strcmp(argv[1], "-tambah") == 0) {
            result = add(num1, num2);
        } else if(strcmp(argv[1], "-kurang") == 0) {
            result = subtract(num1, num2);
        } else if(strcmp(argv[1], "-kali") == 0) {
            result = multiply(num1, num2);
        } else if(strcmp(argv[1], "-bagi") == 0) {
            result = divide(num1, num2);
        }
```
Close reading end of first pipe 
```
        close(fd1[0]);

```
Write input string and close writing end of first pipe. 
```
        write(fd1[1], &result, sizeof(result)); 
        close(fd1[1]); 
```
Wait for child to send a string 
```
        wait(NULL); 
```
 Close writing end of second pipe 
```
        close(fd2[1]);
```
Read string from child, print it and close reading end. 
```
        char concat_str[100]; 
        read(fd2[0], concat_str, 100); 
        printf("%s\n", concat_str); 
        close(fd2[0]); 
    } 
```
child process 
```
    else
    { 
```
Close writing end of first pipe 
```
        close(fd1[1]);
```
 Read a string using first pipe 
```
        int result;
        read(fd1[0], &result, sizeof(result)); 
```
Convert number to string
```
        char str[100];
        numberToString(result, str);
```
Close both reading ends 
```
        close(fd1[0]); 
        close(fd2[0]); 

```
Persiapan string yang akan di outputkan di terminal
```
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
```
Menampilkan hasil operasi pada terminal
```
        char final_str[200];
        sprintf(final_str, "hasil %s %s dan %s adalah %s.", operation, input_str1, input_str2, str);
```
Menyimpan log dari operasi yang sudah dilakukan
```
        writeLog(operation, final_str);
```
Write concatenated string and close writing end 
```
        write(fd2[1], final_str, strlen(final_str)+1); 
        close(fd2[1]); 

        exit(0); 
    } 
}
```

How to play
 - `./kalkulator -kali`
 - `./kalkulator -tambah`
 - `./kalkulator -kurang`
 - `./kalkulator -bagi`

Contoh Penggunaan

![Screenshot (103)](https://github.com/SyahmiAsh/Sisop-3-2024-MH-IT14/assets/88548292/b168eed8-9cc5-480d-86e3-eea4a681dcc3)
![Screenshot (103)](https://github.com/SyahmiAsh/Sisop-3-2024-MH-IT14/assets/88548292/3a4d38ca-2056-4ea0-922d-d40c7e93e5fd)

Catatan dari asisten : untuk fungsi pembagian jika bilangan pertama lebih kecil dari bilangan kedua seharusnya menampilkan nol

Before revisi
```
void numberToString(int num, char *str) {
    if(num < 0) {
        strcpy(str, "ERROR");
        return;
    }
    char *satuan[] = {"", "satu", "dua", "tiga", "empat", "lima", "enam", "tujuh", "delapan", "sembilan"};
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
```

After revisi

Menambahkan nol pada array satuan dari yang sebelumnya merupakan string kosong
```
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
```

## Soal 3
by Muhammad Faqih Husain

Shall Leglerg🥶 dan Carloss Signs 😎 adalah seorang pembalap F1 untuk tim Ferrari 🥵. Mobil F1 memiliki banyak pengaturan, seperti penghematan ERS, Fuel, Tire Wear dan lainnya. Pada minggu ini ada race di sirkuit Silverstone. Malangnya, seluruh tim Ferrari diracun oleh Super Max Max pada hari sabtu sehingga seluruh kru tim Ferrari tidak bisa membantu Shall Leglerg🥶 dan Carloss Signs 😎 dalam race. Namun, kru Ferrari telah menyiapkan program yang bisa membantu mereka dalam menyelesaikan race secara optimal. Program yang dibuat bisa mengatur pengaturan - pengaturan dalam mobil F1 yang digunakan dalam balapan.

Pengerjaan Soal no 3 dibagi kedalam 3 program yaitu `action.c` , `paddock.c` , dan `driver.c`

### actions.c
Berisi fungsi-fungsi yang akan digunakan oleh `paddock.c` 
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
### driver.c
Menerima input dari user untuk kemudian dikirimkan menuju paddock.c dan juga menerima hasil dari paddock untuk ditampilkan ke terminal
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
```
`split_last_word_copy` digunakan untuk memisahkan command dengan info. Misalnya `Tire Change medium` akan dipisahkan menjadi `Tire Change` dan `medium`. Hal ini diperlukan untuk melakukan pencatatan log.
```
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
```
`write_log` digunakan untuk mencatat interaksi antara driver dan paddock pada file race.log. Fungsi ini akan membuat file race.log jika file tersebut belum ada.

Format log:

[Source] [DD/MM/YY hh:mm:ss]: [Command] [Additional-info]

ex :

[Driver] [07/04/2024 08:34:50]: [Fuel] [55%]
[Paddock] [07/04/2024 08:34:51]: [Fuel] [You can go]

```
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
```
Berikut ini merupakan fungsi main 
```
int main(int argc, char const *argv[]) {
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    char hello[1024];
```
Membaca input dari user dengan format [command] [info]
```
    printf("Enter your message: ");
    fgets(hello, 1024, stdin);
```
Membuat koneksi socker ke server
```
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
```
prefix untuk menyimpan command dan suffix untuk menyimpan info
```
    char* prefix;
    char* suffix;
```
Menjalankan fungsi `split_last_word_copy` untuk memisahkan command dan info. Kemudian, `write_log` untuk mencatat command dan info yang dikirimkan menuju server. `send()` melakukan pengiriman menuju server. 
```
    split_last_word_copy(hello, &prefix, &suffix);
    write_log("Driver", prefix, suffix);
    send(sock , hello , strlen(hello) , 0 );
```
`read()` menerima input dari server yang akan disimpan pada buffer. Kemudian isi dari buffer yaitu pesan dari paddock akan ditampilkan pada terminal 
```
    valread = read( sock , buffer, 1024);
    printf("%s\n",buffer );
    return 0;
}
```

### paddock.c
Menerima input dari driver kemudian diolah dan dikirimkan kembali menuju driver
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
```
`#include "actions.c"` digunakan untuk melakukan function import dari program `actions.c`
```
#include "actions.c"
#include <time.h>
#define PORT 8080

```
`split_last_word_copy` digunakan untuk memisahkan command dengan info. Misalnya `Tire Change medium` akan dipisahkan menjadi `Tire Change` dan `medium`. Hal ini diperlukan untuk melakukan pencatatan log.
```

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

```
`write_log` digunakan untuk mencatat interaksi antara driver dan paddock pada file race.log. Fungsi ini akan membuat file race.log jika file tersebut belum ada.

Format log:

[Source] [DD/MM/YY hh:mm:ss]: [Command] [Additional-info]

ex :

[Driver] [07/04/2024 08:34:50]: [Fuel] [55%]
[Paddock] [07/04/2024 08:34:51]: [Fuel] [You can go]
```

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
```
`daemonize()` untuk menjalankan program paddock secara daemon
```
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
```
Berikut merupakan fungsi utama dari paddock.c
```

int main(int argc, char const *argv[]) {
```
Menjalankan program secara daemon
```
    daemonize();
```
Membuat server socket
```
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
```
```
    while(1) {
```
Menunggu koneksi socket dari client yaitu driver.c
```
        printf("\n+++++++ Waiting for new connection ++++++++\n\n");
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        
        int pid;
        if ((pid = fork()) == 0) {
            close(server_fd);
```
`read()` menerima pesan dari driver kemudian disimpan pada buffer dan ditampilkan pada terminal
```
            memset(buffer, 0, 1024);
            read(new_socket, buffer, 1024);
            printf("%s\n",buffer);
```
Mengecek command yang sesuai dengan pesan yang dikirimkan oleh driver kemudian memanggil fungsi dari program `action.c`. Output dari fungsi tersebut kemudian disimpan pada variable hasil.
```
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
```
Memisahkan buffer menjadi prefix yang berisi command dan suffix yang berisi info
```
            char* prefix;
            char* suffix;
            split_last_word_copy(buffer, &prefix, &suffix);
```
Mencatat Komunikasi yang dilakukan oleh paddock
```
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
### Hasil Run
Struktur Direktori

![Screenshot_2024-05-11_08_51_46](https://github.com/fqhhusain/Sisop-1-2024-MH-IT14/assets/88548292/b9f17cdc-5a79-4f54-a205-238d05e3cda4)

Interaksi antara driver dan Paddock
![Screenshot (100)](https://github.com/SyahmiAsh/Sisop-3-2024-MH-IT14/assets/88548292/1ac394ce-ece7-4aee-88b0-6bc0a4477baf)

Mengecek apakah program berjalan secara daemon
![Screenshot (101)](https://github.com/SyahmiAsh/Sisop-3-2024-MH-IT14/assets/88548292/7c77a431-c0b1-4cf5-a364-21b829d4a9d2)

Isi dari file race.log

![Screenshot (102)](https://github.com/SyahmiAsh/Sisop-3-2024-MH-IT14/assets/88548292/a95292ed-0b2a-4139-bca2-ed09385f6fd1)

## Soal 4
Lewis Hamilton 🏎 seorang wibu akut dan sering melewatkan beberapa episode yang karena sibuk menjadi asisten. Maka dari itu dia membuat list anime yang sedang ongoing (biar tidak lupa) dan yang completed (anime lama tapi pengen ditonton aja). Tapi setelah Lewis pikir-pikir malah kepikiran untuk membuat list anime. Jadi dia membuat file (harap diunduh) dan ingin menggunakan socket yang baru saja dipelajarinya untuk melakukan CRUD pada list animenya.

Membuat struktur repository sebagai berikut:

soal_4/

    ├── change.log
    ├── client/
    │   └── client.c
    ├── myanimelist.csv
    └── server/
        └── server.c


untuk mendapatkan file `myanimelist.csv` dengan cara mengunduh file pada soal dan diunduh pada folder soal_4.

commandnya `wget -O myanimelist "https://drive.google.com/uc?export=download&id=10p_kzuOgaFY3WT6FVPJIXFbkej2s9f50"`

Membuat file program server.c di dalam folder server.
```
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
```
Library yang digunakan
```

#define PORT 8080
#define BUFFER_SIZE 1024
#define FILENAME "/home/ash23/Downloads/soalsisop/soal_4/myanimelist.csv"
#define LOG_FILE "/home/ash23/Downloads/soalsisop/soal_4/change.log"
```
- Mendefinisikan konstanta bernama `PORT` dengan nilai `8080`
- Mendefinisikan konstanta `BUFFER_SIZE` dengan nilai `1024`
- Mendefinisikan konstanta `FILENAME` yang menyimpan alamat file `"myanimelist.csv"`
- Mendefinisikan konstanta `LOG_FILE` yang menyimpan alamat file log `"change.log"`

Server dapat mencatat anime yang dihapus, ditambah, diedit dalam sebuah log yang diberi nama change.log.
- Format: [date] [type] [massage]
- Type: ADD, EDIT, DEL

```
void log_action(const char *type, const char *message, const char *info, const char *extra_info) {
    FILE *log_file = fopen(LOG_FILE, "a");
    if (log_file != NULL) {
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        char timestamp[20];
        strftime(timestamp, sizeof(timestamp), "[%d/%m/%y]", tm_info);

        if (strcmp(type, "[ADD]") == 0) {
            fprintf(log_file, "%s %s %s ditambahkan.\n", timestamp, type, message);
        } else if (strcmp(type, "[EDIT]") == 0) {
            fprintf(log_file, "%s %s %s diubah menjadi %s.\n", timestamp, type, info, extra_info);
        } else if (strcmp(type, "[DEL]") == 0) {
            fprintf(log_file, "%s %s %s berhasil dihapus.\n", timestamp, type, message);
        }
        fclose(log_file);
    }
}
```
File change.log akan dibuka, menyimpan waktu pada variabel timestamp, dan yang terakhir akan penulisan kan pada change.log dengan format yang diatas berdasarkan typenya lalu file akan ditutup

Server digunakan untuk membaca myanimelist.csv. Dimana terjadi pengiriman data antara client ke server dan server ke client.
- Menampilkan seluruh judul
```
void send_third_column(int client_socket) {
    FILE *file = fopen(FILENAME, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    char *token;
    int counter = 1;
    char response[BUFFER_SIZE] = "";

    while (fgets(buffer, BUFFER_SIZE, file) != NULL) {
        char *line = strdup(buffer);

        token = strtok(line, ",");
        for (int i = 0; i < 2; i++) {
            token = strtok(NULL, ",");
        }

        if (token != NULL) {
            char temp[BUFFER_SIZE];
            snprintf(temp, BUFFER_SIZE, "%d. %s\n", counter++, token);
            strcat(response, temp);
        } else {
            send(client_socket, "Column not found\n", 17, 0);
        }

        free(line);
    }

    send(client_socket, response, strlen(response), 0);

    fclose(file);
}
```
Dalam file `myanimelist.csv` judul terletak pada urutan ke-3 yang dipisahkan dengan (,) apabila sudah mendapatkan judul maka akan tertulis urutan dan judul yang ada di dalam file tersebut.

- Menampilkan judul berdasarkan genre
```
void send_titles_by_genre(int client_socket, const char *genre) {
    FILE *file = fopen(FILENAME, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    char *token;
    char response[BUFFER_SIZE] = "";
    int counter = 1;

    fgets(buffer, BUFFER_SIZE, file);

    while (fgets(buffer, BUFFER_SIZE, file) != NULL) {
        char *line = strdup(buffer);

        token = strtok(line, ",");
        token = strtok(NULL, ",");
        if (token != NULL && strcmp(token, genre) == 0) {
            token = strtok(NULL, ",");
            if (token != NULL) {
                char temp[BUFFER_SIZE];
                snprintf(temp, BUFFER_SIZE, "%d. %s\n", counter++, token);
                strcat(response, temp);
            }
        }

        free(line);
    }

    if (strlen(response) == 0) {
        strcat(response, "No titles found for the genre\n");
    }

    send(client_socket, response, strlen(response), 0);

    fclose(file);
}
```
Dalam file `myanimelist.csv` genre terletak pada urutan ke-2 yang dipisahkan dengan (,) apabila sudah mendapatkan genre maka akan tertulis urutan dan judul yang sesuai dengan genre yang diinputkan dalam file tersebut dan apabila genre yang diinputkan tidak ada maka akan tertulis `"No titles found for the genre\n"`.

- Menampilkan judul berdasarkan hari
```
void send_titles_by_day(int client_socket, const char *day) {
    FILE *file = fopen(FILENAME, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE] = "";
    int counter = 1;

    fgets(buffer, BUFFER_SIZE, file);

    while (fgets(buffer, BUFFER_SIZE, file) != NULL) {
        char *line = strdup(buffer); 

        char *token = strtok(line, ",");
        if (token != NULL && strcmp(token, day) == 0) {
            token = strtok(NULL, ",");
            token = strtok(NULL, ",");
            if (token != NULL) {
                char temp[BUFFER_SIZE];
                snprintf(temp, BUFFER_SIZE, "%d. %s\n", counter++, token);
                strcat(response, temp);
            }
        }

        free(line);
    }

    if (strlen(response) == 0) {
        strcat(response, "No titles found for the day\n");
    }

    send(client_socket, response, strlen(response), 0);

    fclose(file);
}
```
Dalam file `myanimelist.csv` hari terletak pada urutan ke-1 yang dipisahkan dengan (,) apabila sudah mendapatkan hari maka akan tertulis urutan dan judul yang sesuai dengan hari yang diinputkan dalam file tersebut dan apabila hari yang diinputkan tidak ada maka akan tertulis `"No titles found for the day\n"`.

- Menampilkan status berdasarkan berdasarkan judul
```
void send_status_by_titles(int client_socket, const char *titles) {
    FILE *file = fopen(FILENAME, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE] = "";
   
    fgets(buffer, BUFFER_SIZE, file);

    while (fgets(buffer, BUFFER_SIZE, file) != NULL) {
        char *line = strdup(buffer); 

        char *token = strtok(line, ",");
        token = strtok(NULL, ","); 
        token = strtok(NULL, ",");


        if (token != NULL && strcmp(token, titles) == 0) {
            token = strtok(NULL, ",");
            char temp[BUFFER_SIZE];
            snprintf(temp, BUFFER_SIZE, "%s", token);
            strcat(response, temp);
        }

        free(line);
    }

    if (strlen(response) == 0) {
        strcat(response, "No status found for the title\n");
    }

    send(client_socket, response, strlen(response), 0);

    fclose(file);
}
```
Dalam file `myanimelist.csv` judul terletak pada urutan ke-3 yang dipisahkan dengan (,) apabila sudah mendapatkan judul maka akan tertulis status dari judul yang diinputkan dalam file tersebut dan apabila judul yang diinputkan tidak ada maka akan tertulis `"No status found for the title\n"`.

- Menambahkan anime ke dalam file myanimelist.csv
```
void add(const char *day, const char *genre, const char *title, const char *status) {
    FILE *file = fopen(FILENAME, "a");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // Pindahkan posisi penulisan ke akhir file
    fseek(file, 0, SEEK_END);

    // Tulis data ke file
    fprintf(file, "%s,%s,%s,%s\n", day, genre, title, status);
    
    // Tutup file
    fclose(file);

    log_action("[ADD]", title, "berhasil", "ditambah");
}
```
Fungsi ini akan membuka file `myanimelist.csv` lalu menuliskan hari,genre,title,status yang diinputkan ke dalam file tersebut dan menutup file lalu memanggil fungsi log_action. 

- Melakukan delete berdasarkan judul
```
void delete(int client_socket, const char *title) {
    char response[100]; // Inisialisasi response

    // Buka file asli dan file sementara
    FILE *file = fopen(FILENAME, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    FILE *temp = fopen("temp.csv", "w");
    if (temp == NULL) {
        perror("Error creating temporary file");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    int found = 0;

    // Baca setiap baris dalam file asli
    while (fgets(buffer, sizeof(buffer), file)) {
        char *line = strdup(buffer); // Salin baris ke variabel lain untuk diproses
        if (line == NULL) {
            perror("Error duplicating line");
            fclose(file);
            fclose(temp);
            exit(EXIT_FAILURE);
        }

        // Pisahkan baris ke dalam token menggunakan koma sebagai delimiter
        char *token = strtok(line, ",");
        token = strtok(NULL, ","); // Lewati kolom kedua
        token = strtok(NULL, ","); // Ambil token di kolom ketiga (judul)
        if (token != NULL && strcmp(token, title) == 0) {
            // Jika judul cocok, abaikan baris ini
            found = 1;
        } else {
            // Jika judul tidak cocok, salin baris ke file sementara
            fputs(buffer, temp);
        }

        free(line); // Bebaskan memori yang dialokasikan untuk baris yang disalin
    }

    fclose(file);
    fclose(temp);

    // Hapus file asli dan ganti dengan file sementara
    if (found) {
        if (remove(FILENAME) != 0) {
            perror("Error deleting file");
            exit(EXIT_FAILURE);
        }
        if (rename("temp.csv", FILENAME) != 0) {
            perror("Error renaming file");
            exit(EXIT_FAILURE);
        }
        sprintf(response, "Anime berhasil dihapus.\n");
        send(client_socket, response, strlen(response), 0); // Mengirim pesan ke client
        log_action("[DEL]", title, "berhasil", "dihapus");
    } else {
        sprintf(response, "Anime dengan judul '%s' tidak ditemukan.\n", title);
        send(client_socket, response, strlen(response), 0); // Mengirim pesan ke client
        remove("temp.csv"); // Hapus file sementara jika tidak ada entri yang dihapus
    }
}
```
Fungsi ini akan membuka file `myanimelist.csv` lalu membaca file tersebut dan apabila judul yang diinputkan terdapat di dalam file maka baris itu akan di hapus, fungsi ini memakai file sementara untuk mengecek membuat salinan dari file yang asli dan apabila judul di temukan maka file yang asli akan di hapus dan di gantikan oleh file sementara yang akan diganti namanya sesuai dengan file yang asli. Apabila berhasil menghapus maka akan tertulis `"Anime berhasil dihapus.\n"` dan memanggil fungsi log_action, apabila sebaliknya maka akan tertulis `Anime dengan judul '%s' tidak ditemukan.\n` dan file sementara akan dihapus.

- Melakukan edit anime berdasarkan judul
```
void edit(const char *old_title, const char *new_day, const char *new_genre, const char *new_title, const char *new_status, int client_socket) {
    FILE *file = fopen(FILENAME, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    FILE *temp = fopen("temp.csv", "w");
    if (temp == NULL) {
        perror("Error creating temporary file");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    int found = 0;

    while (fgets(buffer, sizeof(buffer), file)) {
        char *line = strdup(buffer);
        if (line == NULL) {
            perror("Error duplicating line");
            fclose(file);
            fclose(temp);
            exit(EXIT_FAILURE);
        }

        // Pisahkan baris ke dalam token menggunakan koma sebagai delimiter
        char *token = strtok(line, ",");
        char *day = token; // Ambil hari dari kolom pertama
        char *genre = strtok(NULL, ","); // Ambil genre dari kolom kedua
        char *title = strtok(NULL, ","); // Ambil judul dari kolom ketiga
        char *status = strtok(NULL, ","); // Ambil status dari kolom keempat

        // Jika judul cocok, edit komponennya
        if (title != NULL && strcmp(title, old_title) == 0) {
            found = 1;
            // Tulis kembali token yang dilewati dan kemudian komponen baru
            fprintf(temp, "%s,%s,%s,%s\n", new_day, new_genre, new_title, new_status);
            // Mencatat tindakan pengeditan ke dalam file log
            char extra_info[BUFFER_SIZE];
            char info[BUFFER_SIZE];
            snprintf(extra_info, BUFFER_SIZE, "%s,%s,%s,%s", new_day, new_genre, new_title, new_status);
            snprintf(info, BUFFER_SIZE, "%s,%s,%s,%s", day, genre, title, status);
            log_action("[EDIT]", title, info, extra_info);
        } else {
            // Jika tidak cocok, tulis baris tanpa perubahan
            fputs(buffer, temp);
        }

        free(line);
    }

    fclose(file);
    fclose(temp);

    // Replace the original file with the temporary file
    if (found) {
        if (remove(FILENAME) != 0) {
            perror("Error deleting file");
            exit(EXIT_FAILURE);
        }
        if (rename("temp.csv", FILENAME) != 0) {
            perror("Error renaming file");
            exit(EXIT_FAILURE);
        }
        char response[100];
        sprintf(response, "Anime berhasil diedit.\n");
        send(client_socket, response, strlen(response), 0); // Mengirim pesan ke client
    } else {
        printf("Title '%s' not found.\n", old_title);
        remove("temp.csv");
        char response[100];
        sprintf(response, "Anime dengan judul '%s' tidak ditemukan.\n", old_title);
        send(client_socket, response, strlen(response), 0); // Mengirim pesan ke client
    }
}
```
Fungsi ini akan membuka file `myanimeliast` lalu mengambil setiap kata yang diinputkan dengan format <judul>,<hari.baru><genre.baru><judul.baru><status.baru>, dan apabila judul yang diinputkan terdapat di dalam file maka hari,genre,judul,status yang lama akan digantikan dengan yang baru lalu memanggil fungsi log_action. Fungsi ini memakai file salinan apabila judul cocok maka file yang asli akan di gantikan dengan file sementara, apabila judul tidak ada yang cocok file sementara akan di hapus dan tertulis `"Title '%s' not found.\n"`.

```
void process_command(int client_socket, char *command) {
    char *token;

    if (strcmp(command, "exit\n") == 0) {
        close(client_socket);
        exit(EXIT_SUCCESS);
    }

    if (strcmp(command, "tampilkan\n") == 0) {
        send_third_column(client_socket);
    } else if (strncmp(command, "genre ", 6) == 0) {
        token = strtok(command, " ");
        token = strtok(NULL, "\n");
        send_titles_by_genre(client_socket, token);
    } else if (strncmp(command, "hari ", 5) == 0) {
        token = strtok(command, " ");
        token = strtok(NULL, "\n");
        send_titles_by_day(client_socket, token);
    } else if (strncmp(command, "status ", 7) == 0) {
        token = strtok(command, " ");
        token = strtok(NULL, "\n");
        send_status_by_titles(client_socket, token);
    } else if (strncmp(command, "add ", 4) == 0) {
        char *title = strtok(command + 4, ",");
        char *genre = strtok(NULL, ",");
        char *day = strtok(NULL, ",");
        char *status = strtok(NULL, "\n");
        add(title, genre, day, status);
        send(client_socket, "Anime berhasil ditambahkan.\n", 28, 0);
    } else if (strncmp(command, "delete ", 7) == 0) {
        char *title = strtok(command + 7, "\n");

        if (title != NULL) {
            delete(client_socket, title);
        } else {
            send(client_socket, "Invalid command format\n", 23, 0);
        }
    } else if (strncmp(command, "edit ", 5) == 0) {
        char *old_title = strtok(command + 5, ",");
        char *new_day = strtok(NULL, ",");
        char *new_genre = strtok(NULL, ",");
        char *new_title = strtok(NULL, ",");
        char *new_status = strtok(NULL, "\n");

        if (old_title != NULL && new_day != NULL && new_genre != NULL && new_title != NULL && new_status != NULL) {
            edit(old_title, new_day, new_genre, new_title, new_status, client_socket);
        } else {
            send(client_socket, "Invalid command format\n", 23, 0);
        }
```
Fungsi ini berguna untuk mengecek setiap command yang diinputkan apabila sesuai fungsi" yang ada maka akan menjalankan fungsinya dan apabila tidak ada yang sesuai maka akan menampilkan `“Invalid Command”`.

- Selain command yang diberikan akan menampilkan tulisan “Invalid Command”
```
    } else {
        send(client_socket, "Invalid command\n", 17, 0);
    }
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
```
- Server berfungsi sebagai penerima pesan dari client dan hanya menampilkan pesan perintah client saja. 
```
    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        while (1) {
            int bytes_received = recv(new_socket, buffer, BUFFER_SIZE, 0);
            if (bytes_received <= 0) {
                break;
            }
            buffer[bytes_received] = '\0';
            printf("Received: %s\n", buffer);
        
            // Check if client sent "exit" command
            if (strcmp(buffer, "exit") == 0) {
                printf("exit\n");
                close(new_socket); // Close the connection
                close(server_fd); // Close the server socket
                exit(EXIT_SUCCESS); // Exit the server
            }
        
            process_command(new_socket, buffer);
            memset(buffer, 0, BUFFER_SIZE);
        }
    }

return 0;
}
```

Membuat file program client.c di dalam folder client.

Library yang dipakai
```
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
```

```
#define PORT 8080
#define IP "127.0.0.1"
#define BUFFER_SIZE 1024
```
- Mendefinisikan sebuah konstanta bernama `PORT` dengan nilai `8080`
- Mendefinisikan konstanta `IP` yang menyimpan alamat `"127.0.0.1"`
- Mendefinisikan konstanta `BUFFER_SIZE` dengan nilai `1024`
```
int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IP address from text to binary form
    if (inet_pton(AF_INET, IP, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }
```
Client berfungsi sebagai pengirim pesan dan dapat menerima pesan dari server.
```
    while (1) {
        // Get user input
        printf("You: ");
        fgets(buffer, BUFFER_SIZE, stdin);

        // Send command to server
        send(sock, buffer, strlen(buffer), 0);
```
Koneksi antara client dan server tidak akan terputus jika ada kesalahan input dari client, cuma terputus jika user mengirim pesan “exit”. Program exit dilakukan pada sisi client.
```
        // Periksa untuk perintah keluar
        if (strcmp(buffer, "exit\n") == 0 || strcmp(buffer, "exit") == 0) {
            printf("Exiting the client\n");
            break;
        }

        // Receive response from server
        int bytes_received = recv(sock, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            printf("Server disconnected.\n");
            break;
        }
        buffer[bytes_received] = '\0';

        // Print server's response
        printf("Server:\n%s\n", buffer);
    }


    // Close socket
    close(sock);

    return 0;
```
cara menjalankan program:

1. Jalankan program server.c terlebih dahulu pada folder server
```
gcc -o server server.c

./server
```
2. Jalankan program client.c pada folder client di terminal ke-2
```
gcc -o client client.c

./client
```

saat program dijalankan:
![Screenshot (98)](https://github.com/SyahmiAsh/Sisop-3-2024-MH-IT14/assets/149950475/fe42932c-51b9-4484-9fc3-21faa3ca5d3c)

Isi dari file change.log:
![Screenshot (100)](https://github.com/SyahmiAsh/Sisop-3-2024-MH-IT14/assets/149950475/4aa18539-cf10-41b4-9b4e-08bf976adc3e)

### kendala
Agak kesulitan dalam membuat fungsi-fungsinya sempat salah menamai file.csv nya lupa memanggil fungsi status.

### Revisi
Ada beberapa revisi untuk program server yaitu, fungsi status, fungsi add, fungsi delete dan fungsi edit masih belum berjalan dengan benar.
