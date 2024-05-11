#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define FILENAME "/home/ash23/Downloads/soalsisop/soal_4/myanimelist.csv"
#define LOG_FILE "/home/ash23/Downloads/soalsisop/soal_4/change.log"

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
