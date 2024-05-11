# Sisop-3-2024-MH-IT14
Laporan pengerjaan soal shift modul 3 Praktikum Sistem Operasi 2024 oleh Kelompok IT14
## Praktikan Sistem Operasi Kelompok IT14
1. Tsaldia Hukma Cita          : 5027231036
2. Muhammad Faqih Husain       : 5027231023
3. Muhammad Syahmi Ash Shidqi  : 5027231085

## Soal Shift Modul 3
## Soal 1 
Pada zaman dahulu pada galaksi yang jauh-jauh sekali, hiduplah seorang Stelle. Stelle adalah seseorang yang sangat tertarik dengan Tempat Sampah dan Parkiran Luar Angkasa. Stelle memulai untuk mencari Tempat Sampah dan Parkiran yang terbaik di angkasa. Dia memerlukan program untuk bisa secara otomatis mengetahui Tempat Sampah dan Parkiran dengan rating terbaik di angkasa.

## Soal 2
by Muhammad Faqih Husain

Max Verstappen 🏎️ seorang pembalap F1 dan programer memiliki seorang adik bernama Min Verstappen (masih SD) sedang menghadapi tahap paling kelam dalam kehidupan yaitu perkalian matematika, Min meminta bantuan Max untuk membuat kalkulator perkalian sederhana (satu sampai sembilan). Sembari Max nguli dia menyuruh Min untuk belajar perkalian dari web (referensi) agar tidak bergantung pada kalkulator.
(Wajib menerapkan konsep pipes dan fork seperti yang dijelaskan di modul Sisop. Gunakan 2 pipes dengan diagram seperti di modul 3).


How to play
 - `./kalkulator -kali`
 - `./kalkulator -tambah`
 - `./kalkulator -kurang`
 - `./kalkulator -bagi`

Catatan dari asisten : untuk fungsi pembagian jika bilangan pertama lebih kecil dari bilangan kedua seharusnya menampilkan nol

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
command mengunduh filenya `wget -O myanimelist "https://drive.google.com/uc?export=download&id=10p_kzuOgaFY3WT6FVPJIXFbkej2s9f50"`

Membuat file program server.c di dalam folder server.
Membuat file program client.c di dalam folder client.

Ketentuan program sebagai berikut:
Client dan server terhubung melalui socket.

Client berfungsi sebagai pengirim pesan dan dapat menerima pesan dari server.

Server berfungsi sebagai penerima pesan dari client dan hanya menampilkan pesan perintah client saja.  

Server digunakan untuk membaca myanimelist.csv. Dimana terjadi pengiriman data antara client ke server dan server ke client.
- Menampilkan seluruh judul
- Menampilkan judul berdasarkan genre
- Menampilkan judul berdasarkan hari
- Menampilkan status berdasarkan berdasarkan judul
- Menambahkan anime ke dalam file myanimelist.csv
- Melakukan edit anime berdasarkan judul
- Melakukan delete berdasarkan judul
- Selain command yang diberikan akan menampilkan tulisan “Invalid Command”

Karena Lewis juga ingin track anime yang ditambah, diubah, dan dihapus. Maka dia membuat server dapat mencatat anime yang dihapus dalam sebuah log yang diberi nama change.log.
Format: [date] [type] [massage]
Type: ADD, EDIT, DEL
Ex:
[29/03/24] [ADD] Kanokari ditambahkan.
[29/03/24] [EDIT] Kamis,Comedy,Kanokari,completed diubah menjadi Jumat,Action,Naruto,completed.
[29/03/24] [DEL] Naruto berhasil dihapus.

Koneksi antara client dan server tidak akan terputus jika ada kesalahan input dari client, cuma terputus jika user mengirim pesan “exit”. Program exit dilakukan pada sisi client.
