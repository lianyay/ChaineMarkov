#ifndef __LIST_H__
#define __LIST_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
Structure d'une cellule (une cellule = une arrête d'un sommet vers un autre):
- Numéro du sommet d'arrivée
- Probabilité associée
- pointeur vers la cellule suivante de la liste
**/
typedef struct cell {
    int sommet_arrivee;
    float proba;
    struct cell* suivante;
} cell;

/**
Structure d'une liste (représente les arrêtes sortantes d'un sommet):
- head -> première cellule
- tail -> dernière cellule (utile pour ajouter en fin de liste)
**/
typedef struct liste {
    struct cell* head;
    struct cell* tail;
} liste;

/**
Structure d'une liste d'adjacence (représentation d'un graphe):
- Tableau de listes
- Nombre de sommets du graphe
**/
typedef struct liste_adj{
    liste* tab_liste;
    int nb_sommets;
} listeAdj;


//*******PROTOTYPES*******/

cell *createCell(int arrivee, float probab);
liste *createListe();
void addCell(liste *l, int arrivee, float probab);
void displayListe(liste l);
listeAdj createListeAdj(int taille);
void displayListeAdj(listeAdj g);
listeAdj readGraph(const char *filename);
int verifierGrapheMarkov(listeAdj g);
void genererFichierMermaid(listeAdj g, const char* nomFichier);
void genererFichierMermaid(listeAdj g, const char* nomFichier);
int verifierGrapheMarkov(listeAdj g);

#endif
