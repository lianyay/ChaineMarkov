#include "matrix.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "list.h"
#include "tarjan.h"

struct listeAdj* g; //Indispensable si l'on veut travailler avec les listes adjacentes après

//fonction permettant de mettre en place les matrices avec des listes d'adjacences
t_matrix* creer_matrice_liste_adjacence(listeAdj* g) {
    int n = g->nb_sommets;
    t_matrix* matrice = malloc(sizeof(t_matrix));
    matrice->lignes = n;
    matrice->cols = n;
    matrice->data = (double**)malloc(sizeof(double*) * n);

    // Initialiser à 0
    for (int i = 0; i < n; i++) {
        matrice->data[i] = (double*)calloc(n, sizeof(double));
    }

    // Remplir avec les probabilités réelles depuis la liste d'adjacence
    for (int i = 0; i < n; i++) {
        cell* current = g->tab_liste[i].head;
        while (current != NULL) {
            int etat_arrivee = current->sommet_arrivee - 1;  // -1 pour l'indice C
            float proba_reelle = current->proba; // Récupération de la probabilité de transition depuis le sommet courrant vers le voisin
            matrice->data[i][etat_arrivee] = (double)proba_reelle; //on la stocke dans la matrice à la ligne du sommet courant et à la colonne du voisin
            current = current->suivante;
        }
    }
    return matrice;
}

//Fonction qui nous renvoie une matrice de valeur 0
t_matrix* creer_matrice_valzeros(int lignes, int cols){
  t_matrix* matrice = malloc(sizeof (t_matrix));
  matrice->lignes = lignes;
  matrice->cols = cols;
  //Allocation d'un tableau de pointeurs pour les lignes
  matrice->data =(double**)malloc(sizeof(double*)*lignes);
  for (int i = 0; i < lignes; i++) {
      //Allocation de chaque ligne rempli de 0
    matrice->data[i] = (double*)calloc(cols, sizeof(double));
  }
  return matrice;
}

//Fonction permettant de copier la matrice
void copie_matrice(t_matrix* src, t_matrix* dest){
  if (src->lignes != dest->lignes || src->cols != dest->cols) {
    printf("Attention : Matrice de taille différentes\n");
    return;
    }
    for (int i = 0; i < src->lignes; i++) {
      for (int j = 0; j < src->cols; j++) {
        dest->data[i][j] = src->data[i][j]; // Copie de la matrice source à la matrice destination
      }
  }
}

//Fonction permettant de multiplier deux matrices
t_matrix* multiplication_matrice(t_matrix* M, t_matrix* N){
  if (M->cols != N->lignes) {
    printf("Attention : Matrice de dimension differentes\n");
    return NULL;
  }
    // Création de notre matrice final
  t_matrix* result = creer_matrice_valzeros(M->lignes, N->cols);
  for (int i = 0; i < M->lignes; i++) { //i --> Ligne de M
    for (int j = 0; j < N->cols; j++) { //j --> Ligne de N
      for (int k = 0; k < M->cols; k++) { // k --> produit : ligne * colonne
        result->data[i][j] += M->data[i][k] * N->data[k][j];
      }
    }
  }
  return result;
}

//Fonction qui calcul la somme des différences absolues
double difference_matrix(t_matrix* M, t_matrix* N){
  if (M->lignes != N->lignes || M->cols != N->cols) {
        printf("Attention: Matrices de tailles différentes\n");
        return -1.0;
    }

    double difference = 0.0;
    for (int i = 0; i < M->lignes; i++) {
        for (int j = 0; j < M->cols; j++) {
            difference += fabs(M->data[i][j] - N->data[i][j]); //fabs --> pour calculer la valeur absolue
        }
    }

    return difference;
}

//Fonction pour afficher la matrice
void afficher_matrice(t_matrix* matrice) {
    for (int i = 0; i < matrice->lignes; i++) {
        for (int j = 0; j < matrice->cols; j++) {
            printf("%.2f\t", matrice->data[i][j]);
        }
        printf("\n");
    }
}

// Fonction qui permet de libérer de la memoire a notre matrice subMatrix
void liberer_matrice(t_matrix* matrice) {
    if (matrice == NULL) {
        printf("Tentative de liberation d'une matrice NULL\n");
        return;
    }

    printf("Liberation d'une matrice %dx%d\n", matrice->lignes, matrice->cols);

    for (int i = 0; i < matrice->lignes; i++) {
        free(matrice->data[i]);
    }

    free(matrice->data);
    free(matrice);
}

//Fonction extrait de la matrice originale uniquement les colonnes correspondant aux sommets d’une classe.
t_matrix* subMatrix(t_matrix* matrix, t_partition* part, int compo_index) {
    // Vérifications de sécurité
    if (matrix == NULL || part == NULL || compo_index < 0 || compo_index >= part->taille) {
        printf("Erreur: Paramètres invalides\n");
        return NULL;
    }

    t_classe* classe = &part->classes[compo_index];

    // Créer une sous-matrice avec les lignes originales, mais seulement les colonnes de la classe
    t_matrix* result = creer_matrice_valzeros(matrix->lignes, classe->taille);

    // Remplir la sous-matrice
    for (int i = 0; i < matrix->lignes; i++) {           // Toutes les lignes originales
        for (int j_classe = 0; j_classe < classe->taille; j_classe++) {  // Seulement les colonnes de la classe
            int j_matrix = classe->sommets[j_classe] - 1;    // -1 pour convertir en indices C (1-based → 0-based)

            // Vérifier l'indice de colonne
            if (j_matrix < 0 || j_matrix >= matrix->cols) {
                printf("Erreur: Indice de colonne %d invalide (après conversion: %d)\n",
                       classe->sommets[j_classe], j_matrix);
                liberer_matrice(result);
                return NULL;
            }
            // Copier l'élément
            result->data[i][j_classe] = matrix->data[i][j_matrix];
        }
    }

    return result;
}
