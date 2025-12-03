#include <stdio.h>
#include <stdlib.h>
#include "list.h"
#include "hasse.h"
#include "tarjan.h"
#include "matrix.h"

int main() {

    // laisser l'utilisateur choisir le fichier
    char input[100];
    char filename[120];

    printf("Quel fichier voulez-vous utiliser ?\n");
    printf("Options : exemple1, exemple1_chatGPT_fixed, exemple1_from_chatGPT, exemple2, exemple3, exemple4_2check, exemple_hasse1, exemple_scc1, exemple_valid_step3, exemple_meteo\n");
    scanf("%99s", input);
    sprintf(filename, "../data/%s.txt", input);
    printf("Fichier choisi : %s\n", filename);

    listeAdj g = readGraph(filename);

    // Affiche la liste adjacente
    printf("Liste Adjacente du graphe :\n");
    displayListeAdj(g);
    printf("\n");

    // Affiche la vérification du graphe de Markov
    printf("Verification graphe de Markov :\n");
    verifierGrapheMarkov(g);
    printf("\n");

    // Affiche le fichier Mermaid original
    char mermaidFile[150];
    sprintf(mermaidFile, "../data/%s_mermaid.txt", input);
    genererFichierMermaid(g, mermaidFile);

    //On commence la partie 2 - Algorithme de tarjan + diagramme de hasse
    // Affiche l'algo de tarjan
    t_partition *partition = tarjan_calculer_partition(g);
    printf("\n");

    printf("Diagramme de Hasse :\n");
    int *corresp = creer_tab_corresp(partition, g.nb_sommets);
    t_link_array *liens = rencenser(&g, corresp);

    printf("Liens entre les classes : %d\n", liens->log_size);
    for (int i = 0; i < liens->log_size; i++) {
        printf("C%d -> C%d\n", liens->links[i].from + 1, liens->links[i].to + 1);
    }
    printf("\n");

    //Affiche les caractétistiques complètes du graphe
    analyserCarac(partition, &g);
    printf("\n");

    // Génère le diagramme de Hasse
    char hasseFile[150];
    sprintf(hasseFile, "../data/%s_hasse.txt", input);
    genererHasseMermaid(partition, liens, hasseFile, 0);

    //On commence la partie 3 - le calcul matriciel
    printf("Calcul matriciel\n");

    //Affiche la matrice d'origine
    t_matrix* M = creer_matrice_liste_adjacence(&g);
    printf("Matrice M :\n");
    afficher_matrice(M);
    printf("\n");

    // Calculer M²
    printf("Matrice M^2 :\n");
    t_matrix* M2 = multiplication_matrice(M, M);
    if (M2 != NULL) {
        afficher_matrice(M2);
    }
    printf("\n");

    // Calculer M³
    printf("Matrice M^3 :\n");
    t_matrix* M3 = NULL;
    if (M2 != NULL) {
        M3 = multiplication_matrice(M2, M);
        if (M3 != NULL) {
            afficher_matrice(M3);
        }
    }
    printf("\n");

    // Convergence M^k
    printf("Convergence\n");//Quand episolon = 0,01

    t_matrix* Mk = creer_matrice_valzeros(g.nb_sommets, g.nb_sommets);
    t_matrix* Mk_prev = creer_matrice_valzeros(g.nb_sommets, g.nb_sommets);

    // Initialiser Mk avec M
    copie_matrice(M, Mk);

    int iterations = 0;
    double diff;
    double epsilon = 0.01;
    int convergence_atteinte = 0;

    printf("Iterations de convergence :\n");
    do {
        copie_matrice(Mk, Mk_prev);  // Sauvegarder précédent

        // Calculer Mk * M
        t_matrix* temp = multiplication_matrice(Mk, M);
        copie_matrice(temp, Mk);


        diff = difference_matrix(Mk, Mk_prev);
        iterations++;

        printf("Iteration %d: difference = %.6f", iterations, diff);
        if (diff <= epsilon) {
            printf(" \n");
            convergence_atteinte = 1;
        } else {
            printf("\n");
        }

        // Limite de sécurité
        if (iterations >= 1000) {
            printf("Limite d'iterations atteinte\n");
            break;
        }

    } while (diff > epsilon);
    printf("\n");

    // Usage de subMatrix pour toutes les classes
    printf("Sous matrice par classe\n");
    for (int compo_index = 0; compo_index < partition->taille; compo_index++) {
        printf("\n--- Classe C%d ---\n", compo_index + 1);

        t_classe* classe = &partition->classes[compo_index];
        printf("Sommets: {");
        for (int j = 0; j < classe->taille; j++) {
            printf("%d", classe->sommets[j]);
            if (j < classe->taille - 1) printf(", ");
        }
        printf("}\n");

        t_matrix* sous_matrice = subMatrix(M, partition, compo_index);

        if (sous_matrice != NULL) {
            printf("Sous-matrice %dx%d:\n", sous_matrice->lignes, sous_matrice->cols);
            afficher_matrice(sous_matrice);
            liberer_matrice(sous_matrice);
        }
    }
    printf("\n");


    // Résultats convergence
    if (convergence_atteinte) {
        printf("Convergence atteinte apres %d iterations\n", iterations);
        printf("Difference finale : %.6f <= epsilon = %.3f\n", diff, epsilon);
        printf("Matrice stationnaire M^%d:\n", iterations);
        afficher_matrice(Mk);

        // Sous-matrices de la matrice stationnaire
        printf("Sous matrice stationnaire\n");
        for (int compo_index = 0; compo_index < partition->taille; compo_index++) {
            printf("Classe C%d (stationnaire) \n", compo_index + 1);
            t_matrix* sous_matrice_stationnaire = subMatrix(Mk, partition, compo_index);

            if (sous_matrice_stationnaire != NULL) {
                afficher_matrice(sous_matrice_stationnaire);
                liberer_matrice(sous_matrice_stationnaire);
            }
        }
    } else {

        printf("Convergence non atteinte apres %d iterations\n", iterations);
        printf("Difference finale : %.6f > epsilon = %.3f\n", diff, epsilon);
    }


    // Memoire libere
    for (int i = 0; i < g.nb_sommets; i++) {
        cell *current = g.tab_liste[i].head;
        while (current != NULL) {
            cell *tmp = current;
            current = current->suivante;
            free(tmp);
        }
    }
    free(g.tab_liste);

    return 0;
}