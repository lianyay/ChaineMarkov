#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include <string.h>

// Crée une cellule
cell *createCell(int arrivee, float probab) {
    cell *nouv = malloc(sizeof(cell));
    nouv->sommet_arrivee = arrivee;
    nouv->proba = probab;
    nouv->suivante = NULL;
    return nouv;
}

// Crée une liste vide
liste *createListe() {
    liste *list = malloc(sizeof(liste));
    list->head = NULL;
    list->tail = NULL;
    return list;
}

// Ajoute une cellule au début d'une liste d'adjacence
void addCell(liste *l, int arrivee, float probab) {
    cell* new_cell = createCell(arrivee, probab);

    //Le nouvel élément devient le premier
    new_cell->suivante = l->head;
    l->head = new_cell;

	// Cas où la liste est vide
    if (l->tail == NULL)
        l->tail = new_cell;
}

// Affiche toute les arrêtes d'une liste
void displayListe(liste l) {
    cell* current = l.head;
    if (current == NULL) {
        printf("Empty list\n");
        return;
    }
    while (current != NULL) {
        printf("-> %d (%.2f) ", current->sommet_arrivee, current->proba);
        current = current->suivante;
    }
    printf("\n");
}

// Crée une liste d'adjacence vide (tableau de listes)
listeAdj createListeAdj(int taille) {
    listeAdj nouv;
    nouv.nb_sommets = taille;

    //Allocation dynamique d'un tableau de listes
    nouv.tab_liste = malloc(taille * sizeof(liste));

    //Les listes sont initialisées comme vides
    for (int i = 0; i < taille; i++) {
        nouv.tab_liste[i].head = NULL;
        nouv.tab_liste[i].tail = NULL;
    }
    return nouv;
}

// Affiche une liste d'adjacence (toutes les listes)
void displayListeAdj(listeAdj g) {
    for (int i = 0; i < g.nb_sommets; i++) {
        printf("Sommet %d: ", i + 1);
        displayListe(g.tab_liste[i]);
    }
}

// lit un graphe depuis un fichier texte
listeAdj readGraph(const char *filename) {
    FILE *file = fopen(filename, "rt");
    int nbvert, depart, arrivee;
    float proba;

    if (file == NULL) {
        perror("Could not open file for reading");
        exit(EXIT_FAILURE);
    }

    //Lecture du nombre de sommets
    if (fscanf(file, "%d", &nbvert) != 1) {
        perror("Could not read number of vertices");
        exit(EXIT_FAILURE);
    }

    //Création d'une liste d'adjacence vide
    listeAdj g = createListeAdj(nbvert);

    while (fscanf(file, "%d %d %f", &depart, &arrivee, &proba) == 3) {
        // REMETTRE l'ordre ORIGINAL
        addCell(&g.tab_liste[depart - 1], arrivee, proba);
    }

    fclose(file);
    return g;
}

// Fonction pour vérifier si le graphe est un graphe de Markov
int verifierGrapheMarkov(listeAdj g) {
    int estMarkov = 1;

    //On vérifie chaque sommet
    for (int i = 0; i < g.nb_sommets; i++) {
        float somme = 0.0;
        cell *current = g.tab_liste[i].head;

        // On calcule la somme des probabilités sortantes pour le sommet i
        while (current != NULL) {
            somme += current->proba;
            current = current->suivante;
        }

        // Vérifier si la somme est environ égale à 1
        if (somme < 0.99 || somme > 1.00) {
            printf("la somme des probabilites du sommet %d est %.6f\n", i + 1, somme);
            estMarkov = 0;
        }
    }

    // Affiche le résultat final UNE SEULE FOIS
    if (estMarkov) {
        printf("Le graphe est un graphe de Markov\n");
    } else {
        printf("Le graphe n'est pas un graphe de Markov\n");
    }

    return estMarkov;
}


// Fonction pour generer le fichier mermaid.txt
void genererFichierMermaid(listeAdj g, const char* nomFichier) {
    FILE *file = fopen(nomFichier, "w");
    if (!file) {
        perror("Impossible de creer le fichier");
        return;
    }

    // En-tête demandé par le sujet
    fprintf(file, "---\n");
    fprintf(file, "config:\n");
    fprintf(file, "  layout: elk\n");
    fprintf(file, "  theme: neo\n");
    fprintf(file, "  look: neo\n");
    fprintf(file, "---\n");
    fprintf(file, "flowchart LR\n");

    // Déclaration des sommets (avec leur numéro)
    for (int i = 0; i < g.nb_sommets; i++) {
        fprintf(file, "%s((%d))\n", getID(i + 1), i + 1);
    }

    // Déclaration des arêtes (avec leur probabilité)
    for (int i = 0; i < g.nb_sommets; i++) {
        cell* current = g.tab_liste[i].head;
        while (current != NULL) {
            // Pas besoin de free car getID utilise un buffer statique
            fprintf(file, "%s -->|%.2f|%s\n",
                    getID(i + 1),
                    current->proba,
                    getID(current->sommet_arrivee));
            current = current->suivante;
        }
    }

    fclose(file);
}