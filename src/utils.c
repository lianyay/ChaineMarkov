#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"


//Fonction qui transforme un nombre en chaine de caractere
char* getID(int num) {
  //allocation dynamique de la mémoire (max 10 caracteres)
    char* id = malloc(10 * sizeof(char));
    int index = 0;
    int temp = num;
//Conversion nombre/lettre à partir d'une base 26
    while (temp > 0) {
        temp--;
        id[index++] = 'A' + (temp % 26);
        temp /= 26;
    }
    // Inversion de la chaîne pour la remettre à l'endroit
    for (int i = 0; i < index / 2; i++) {
        char tmp = id[i];
        id[i] = id[index - 1 - i];
        id[index - 1 - i] = tmp;
    }
//ajout du caractère de fin de chaîne
    id[index] = '\0';
    return id;
}