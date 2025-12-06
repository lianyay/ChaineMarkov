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
    if (M == NULL || N == NULL) {
        printf("Erreur: Une des matrices est NULL\n");
        return NULL;
    }

    if (M->cols != N->lignes) {
        printf("Erreur de dimensions: impossible de multiplier (%dx%d) par (%dx%d)\n",
               M->lignes, M->cols, N->lignes, N->cols);
        printf("Le nombre de colonnes de la premiere (%d) doit etre egal au nombre de lignes de la seconde (%d)\n",
               M->cols, N->lignes);
        return NULL;
    }

    // Création de notre matrice finale
    t_matrix* result = creer_matrice_valzeros(M->lignes, N->cols);

    // Multiplication standard
    for (int i = 0; i < M->lignes; i++) {
        for (int j = 0; j < N->cols; j++) {
            double sum = 0.0;
            for (int k = 0; k < M->cols; k++) {
                sum += M->data[i][k] * N->data[k][j];
            }
            result->data[i][j] = sum;
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

int gcd(int *vals, int nbvals) {
    if (nbvals == 0) return 0;
    int result = vals[0];
    for (int i = 1; i < nbvals; i++) {
        int a = result;
        int b = vals[i];
        while (b != 0) {
            int temp = b;
            b = a % b;
            a = temp;  // THIS LINE WAS MISSING!
        }
        result = a;
    }
    return result;
}

int getPeriod(t_matrix* sub_matrix)  // Note: pointer, not struct by value
{
    int n = sub_matrix->lignes;  // Use your field name
    int *periods = (int *)malloc(n * sizeof(int));
    int period_count = 0;

    // Create matrices using YOUR functions
    t_matrix* power_matrix = creer_matrice_valzeros(n, n);
    t_matrix* temp_matrix = NULL;  // For multiplication result

    // Initialize power_matrix = sub_matrix
    copie_matrice(sub_matrix, power_matrix);

    for (int cpt = 1; cpt <= n; cpt++)
    {
        // Check diagonal
        int diag_nonzero = 0;
        for (int i = 0; i < n; i++)
        {
            if (power_matrix->data[i][i] > 0.0)  // Use double, not float
            {
                diag_nonzero = 1;
                break;  // Can break early
            }
        }

        if (diag_nonzero) {
            periods[period_count] = cpt;
            period_count++;
        }

        // Compute next power: M^(cpt+1) = M^cpt × M^1
        temp_matrix = multiplication_matrice(power_matrix, sub_matrix);
        if (temp_matrix == NULL) {
            // Handle error
            liberer_matrice(power_matrix);
            free(periods);
            return -1;
        }

        // Update power_matrix for next iteration
        copie_matrice(temp_matrix, power_matrix);
        liberer_matrice(temp_matrix);
        temp_matrix = NULL;
    }

    int period = gcd(periods, period_count);

    // Cleanup
    liberer_matrice(power_matrix);
    free(periods);

    return period;
}


// Extrait une sous-matrice carrée (lignes ET colonnes d'une classe)
t_matrix* extractSquareSubMatrix(t_matrix* matrix, t_partition* part, int compo_index) {
    if (matrix == NULL || part == NULL || compo_index < 0 || compo_index >= part->taille) {
        printf("Erreur: Paramètres invalides\n");
        return NULL;
    }

    t_classe* classe = &part->classes[compo_index];
    int size = classe->taille;

    t_matrix* result = creer_matrice_valzeros(size, size);

    // Remplir la sous-matrice carrée
    for (int i = 0; i < size; i++) {
        int row_idx = classe->sommets[i] - 1;  // -1 pour indice C
        for (int j = 0; j < size; j++) {
            int col_idx = classe->sommets[j] - 1;

            if (row_idx >= 0 && row_idx < matrix->lignes &&
                col_idx >= 0 && col_idx < matrix->cols) {
                result->data[i][j] = matrix->data[row_idx][col_idx];
            }
        }
    }

    return result;
}


