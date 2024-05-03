/*

__________                             .__  __     ____   _____  
\______   \__ __  ____ ___.__._____    |__|/  |_  /_   | /  |  | 
 |     ___/  |  \/    <   |  |\__  \   |  \   __\  |   |/   |  |_
 |    |   |  |  /   |  \___  | / __ \_ |  ||  |    |   /    ^   /
 |____|   |____/|___|  / ____|(____  / |__||__|    |___\____   | 
                     \/\/          \/                       |__| 
*/
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

