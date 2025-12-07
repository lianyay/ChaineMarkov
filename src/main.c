#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <math.h>
#include "list.h"
#include "hasse.h"
#include "tarjan.h"
#include "matrix.h"

// Fonction pour afficher tous les fichiers .txt du répertoire
void afficherFichiersDisponibles() {
    DIR *dir;
    struct dirent *entry;
    int count = 0;

    printf("\nFichiers disponibles dans le repertoire ../data/ :\n");
    printf("===================================================\n");

    dir = opendir("../data/");
    if (dir == NULL) {
        printf("Erreur : Impossible d'ouvrir le repertoire ../data/\n");
        printf("Assurez-vous que le repertoire existe avec les fichiers .txt\n");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        // Vérifier si c'est un fichier .txt (et pas un fichier _mermaid ou _hasse)
        char *ext = strrchr(entry->d_name, '.');
        if (ext != NULL && strcmp(ext, ".txt") == 0) {
            // Extraire le nom de base (sans extension et sans suffixe)
            char nom_base[100];
            strncpy(nom_base, entry->d_name, strlen(entry->d_name) - 4);
            nom_base[strlen(entry->d_name) - 4] = '\0';

            // Ignorer les fichiers avec _mermaid ou _hasse
            if (strstr(nom_base, "_mermaid") == NULL &&
                strstr(nom_base, "_hasse") == NULL) {
                printf("- %s\n", nom_base);
                count++;
            }
        }
    }

    closedir(dir);

    if (count == 0) {
        printf("Aucun fichier .txt trouve dans ../data/\n");
    } else {
        printf("===================================================\n");
        printf("Total : %d fichier(s) disponible(s)\n\n", count);
    }
}

int main() {
    // Afficher d'abord tous les fichiers disponibles
    afficherFichiersDisponibles();

    // Laisser l'utilisateur choisir le fichier
    char input[100];
    char filename[120];

    printf("Quel fichier voulez-vous utiliser ? (Entrez le nom sans .txt)\n");
    printf("Exemple: pour 'exemple1.txt', entrez 'exemple1'\n");
    printf("> ");
    scanf("%99s", input);

    // Vérifier si l'utilisateur a entré l'extension .txt
    char *has_ext = strstr(input, ".txt");
    if (has_ext != NULL) {
        // Enlever l'extension .txt si l'utilisateur l'a entrée
        *has_ext = '\0';
    }

    sprintf(filename, "../data/%s.txt", input);
    printf("Fichier choisi : %s\n", filename);

    // Vérifier si le fichier existe avant de continuer
    FILE *test = fopen(filename, "r");
    if (test == NULL) {
        printf("\nERREUR : Le fichier '%s' n'existe pas !\n", filename);
        printf("Verifiez le nom et assurez-vous qu'il se trouve dans ../data/\n");

        // Réafficher les fichiers disponibles
        printf("\n");
        afficherFichiersDisponibles();
        return 1;
    }
    fclose(test);

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

    // On commence la partie 2 - Algorithme de tarjan + diagramme de hasse
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

    // Affiche les caractéristiques complètes du graphe
    analyserCarac(partition, &g);
    printf("\n");

    // Génère le diagramme de Hasse
    char hasseFile[150];
    sprintf(hasseFile, "../data/%s_hasse.txt", input);
    genererHasseMermaid(partition, liens, hasseFile, 0);

    // On commence la partie 3 - le calcul matriciel
    printf("=========================================================\n");
    printf("=====================Calcul matriciel====================\n");
    printf("=========================================================\n");

    // Affiche la matrice d'origine
    t_matrix* M = creer_matrice_liste_adjacence(&g);
    printf("Matrice M (%dx%d) :\n", M->lignes, M->cols);
    afficher_matrice(M);
    printf("\n");

    // ====== PARTIE 1 : Création de la matrice de distribution initiale ======
    printf("=== Creation de la matrice de distribution initiale ===\n");

    int nb_etat = 0;
    printf("Combien d'etats initiaux voulez-vous saisir (1 a %d) : ", g.nb_sommets);
    scanf("%d", &nb_etat);

    // Limiter le nombre d'états initiaux
    if (nb_etat < 1 || nb_etat > g.nb_sommets) {
        printf("Nombre d'etats invalide! Utilisation d'un seul etat par defaut.\n");
        nb_etat = 1;
    }

    // Créer un tableau pour stocker les états initiaux
    int* etats_init = (int*)malloc(nb_etat * sizeof(int));
    if (etats_init == NULL) {
        printf("Erreur d'allocation memoire!\n");
        return 1;
    }

    // Demander les états initiaux
    printf("\nEntrez les numeros des etats initiaux (1 a %d) :\n", g.nb_sommets);
    for (int i = 0; i < nb_etat; i++) {
        printf("Etat %d : ", i + 1);
        scanf("%d", &etats_init[i]);

        // Validation
        if (etats_init[i] < 1 || etats_init[i] > g.nb_sommets) {
            printf("Etat invalide! Utilisation de l'etat 1 par defaut.\n");
            etats_init[i] = 1;
        }
        etats_init[i]--;  // Convertir en index 0-based
    }

    // Demander le type de distribution
    printf("\n=== TYPE DE DISTRIBUTION ===\n");
    printf("1. Distribution personnalisee (vous saisissez les probabilites)\n");
    printf("2. Distribution uniforme (meme probabilite pour tous les etats)\n");
    printf("3. Distribution aleatoire (probabilites generees aleatoirement)\n");
    printf("\nVotre choix (1-3) : ");

    int choix_distribution;
    scanf("%d", &choix_distribution);

    // Créer un tableau pour les probabilités
    double* probabilites = (double*)malloc(nb_etat * sizeof(double));
    if (probabilites == NULL) {
        printf("Erreur d'allocation memoire!\n");
        free(etats_init);
        return 1;
    }

    // Gérer le type de distribution choisi
    if (choix_distribution == 1) {
        // Distribution personnalisée
        printf("\nEntrez les probabilites pour chaque etat (la somme doit faire 1.0) :\n");
        double somme_proba = 0.0;

        for (int i = 0; i < nb_etat; i++) {
            printf("Probabilite pour l'etat %d : ", etats_init[i] + 1);
            scanf("%lf", &probabilites[i]);

            if (probabilites[i] < 0.0) {
                printf("Probabilite negative! Mise a 0.0.\n");
                probabilites[i] = 0.0;
            }

            somme_proba += probabilites[i];
        }

        // Normaliser si nécessaire
        if (fabs(somme_proba - 1.0) > 0.0001) {
            printf("\nATTENTION : La somme des probabilites est %.6f (au lieu de 1.0)\n", somme_proba);

            if (somme_proba == 0.0) {
                // Si tout est à 0, mettre une distribution uniforme
                printf("Distribution uniforme appliquee.\n");
                for (int i = 0; i < nb_etat; i++) {
                    probabilites[i] = 1.0 / nb_etat;
                }
            } else {
                // Normaliser les probabilités
                printf("Normalisation des probabilites...\n");
                for (int i = 0; i < nb_etat; i++) {
                    probabilites[i] /= somme_proba;
                }
            }
        }

    } else if (choix_distribution == 2) {
        // Distribution uniforme
        double proba_uniforme = 1.0 / nb_etat;
        for (int i = 0; i < nb_etat; i++) {
            probabilites[i] = proba_uniforme;
        }
        printf("\nDistribution uniforme appliquee : probabilite = %.4f pour chaque etat\n", proba_uniforme);

    } else if (choix_distribution == 3) {
        // Distribution aléatoire
        printf("\nGenerer une distribution aleatoire...\n");

        // Initialiser le générateur de nombres aléatoires
        srand(time(NULL));
        double somme_proba = 0.0;

        // Générer des nombres aléatoires
        for (int i = 0; i < nb_etat; i++) {
            probabilites[i] = (double)(rand() % 1000 + 1) / 1000.0;  // Valeurs entre 0.001 et 1.000
            somme_proba += probabilites[i];
        }

        // Normaliser pour que la somme fasse 1
        for (int i = 0; i < nb_etat; i++) {
            probabilites[i] /= somme_proba;
        }

        printf("Distribution aleatoire generee et normalisee.\n");

    } else {
        printf("Choix invalide! Utilisation de la distribution uniforme par defaut.\n");
        double proba_uniforme = 1.0 / nb_etat;
        for (int i = 0; i < nb_etat; i++) {
            probabilites[i] = proba_uniforme;
        }
    }

    // Créer un VECTEUR LIGNE (1 x nb_sommets)
    t_matrix* mat_init = creer_matrice_valzeros(1, g.nb_sommets);

    // Remplir avec les probabilités spécifiées
    for (int i = 0; i < nb_etat; i++) {
        mat_init->data[0][etats_init[i]] = probabilites[i];
    }

    // Afficher le récapitulatif
    printf("\n================ RECAPITULATIF ================\n");
    printf("Etats initiaux et leurs probabilites :\n");
    for (int i = 0; i < nb_etat; i++) {
        printf("  Etat %d : probabilite = %.4f\n",
               etats_init[i] + 1, probabilites[i]);
    }

    // Vérifier la somme
    double somme_finale = 0.0;
    for (int i = 0; i < nb_etat; i++) {
        somme_finale += probabilites[i];
    }
    printf("Somme totale : %.6f\n", somme_finale);

    printf("\nDistribution initiale complete (vecteur ligne 1x%d):\n[ ", g.nb_sommets);
    for (int i = 0; i < g.nb_sommets; i++) {
        printf("%.4f", mat_init->data[0][i]);
        if (i < g.nb_sommets - 1) printf(", ");
        if ((i + 1) % 8 == 0 && i < g.nb_sommets - 1) {
            printf("\n  ");
        }
    }
    printf(" ]\n\n");

    // Libérer les tableaux temporaires
    free(etats_init);
    free(probabilites);

    // ====== PARTIE 2 : Calcul de la puissance de M ======
    int n = 0;
    printf("Entrez la puissance n pour M^n : ");
    scanf("%d", &n);

    if (n < 0) {
        printf("Puissance invalide. Utilisation de n=0.\n");
        n = 0;
    }

    printf("\n=== Calcul de M^%d ===\n", n);

    // Calculer M^n
    t_matrix* M_puissance_n = NULL;

    if (n == 0) {
        // M^0 = matrice identité
        M_puissance_n = creer_matrice_valzeros(g.nb_sommets, g.nb_sommets);
        for (int i = 0; i < g.nb_sommets; i++) {
            M_puissance_n->data[i][i] = 1.0;
        }
        printf("M^0 = matrice identite\n");
    }
    else if (n == 1) {
        // M^1 = M
        M_puissance_n = creer_matrice_valzeros(M->lignes, M->cols);
        copie_matrice(M, M_puissance_n);
        printf("M^1 = M\n");
    }
    else {
        // Calculer M^n par multiplication successive
        printf("Calcul de M^%d en cours...\n", n);

        // Commencer avec M
        t_matrix* current = creer_matrice_valzeros(M->lignes, M->cols);
        copie_matrice(M, current);

        // Multiplier n-1 fois
        for (int i = 2; i <= n; i++) {
            t_matrix* temp = multiplication_matrice(current, M);
            if (temp == NULL) {
                printf("Erreur dans le calcul de M^%d\n", i);
                liberer_matrice(current);
                break;
            }

            // Pour la dernière itération, garder le résultat
            if (i == n) {
                M_puissance_n = temp;  // On garde directement temp
            } else {
                // Sinon, préparer pour l'itération suivante
                liberer_matrice(current);
                current = temp;
            }
        }

        if (n > 1 && M_puissance_n == NULL) {
            // Si on n'a pas atteint M^n, utiliser current
            M_puissance_n = current;
        } else if (n > 1) {
            // Sinon, libérer current
            liberer_matrice(current);
        }

        printf("M^%d calculee avec succes.\n", n);
    }

    // Option: afficher M^n
    if (g.nb_sommets <= 60) {  // Afficher seulement pour les petites matrices
        printf("\nMatrice M^%d :\n", n);
        afficher_matrice(M_puissance_n);
    } else {
        printf("Matrice M^%d calculee (taille %dx%d, non affichee)\n",
               n, g.nb_sommets, g.nb_sommets);
    }
    printf("\n");

    // ====== PARTIE 3 : Multiplication distribution initiale × M^n ======
    printf("=== Calcul de la distribution apres %d transitions ===\n", n);
    printf("Distribution_finale = Distribution_initiale × M^%d\n", n);
    printf("(Avec la distribution initiale configuree)\n");

    // Vérifier la compatibilité des dimensions
    printf("Dimensions: mat_init = %dx%d, M^%d = %dx%d\n",
           mat_init->lignes, mat_init->cols, n, M_puissance_n->lignes, M_puissance_n->cols);

    // Multiplication matricielle
    t_matrix* distribution_finale = multiplication_matrice(mat_init, M_puissance_n);

    if (distribution_finale == NULL) {
        printf("Erreur : Impossible de multiplier les matrices\n");
    } else {
        printf("\nDistribution de probabilite apres %d transitions :\n", n);
        printf("(A partir de la distribution initiale configuree)\n");
        printf("=====================================================\n");

        // Calculer la somme des probabilités
        double somme_finale = 0.0;

        // 1. Affichage détaillé
        printf("\n1. Probabilites par etat :\n");
        for (int i = 0; i < distribution_finale->cols; i++) {
            double proba = distribution_finale->data[0][i];
            printf("P(etat %d) = %.6f\n", i + 1, proba);
            somme_finale += proba;
        }

        printf("\nVerification : somme = %.6f\n", somme_finale);

        // 2. Trouver l'état le plus probable
        int etat_max = 0;
        double proba_max = distribution_finale->data[0][0];
        for (int i = 1; i < distribution_finale->cols; i++) {
            if (distribution_finale->data[0][i] > proba_max) {
                proba_max = distribution_finale->data[0][i];
                etat_max = i;
            }
        }

        printf("Etat le plus probable : %d (p = %.4f = %.2f%%)\n",
               etat_max + 1, proba_max, proba_max * 100);

        // 3. Les états accessibles
        printf("\n2. Etats accessibles (avec probabilite > 0.000001) :\n");
        int nb_accessibles = 0;
        for (int i = 0; i < distribution_finale->cols; i++) {
            if (distribution_finale->data[0][i] > 0.000001) {
                printf("  Etat %d : %.6f\n", i + 1, distribution_finale->data[0][i]);
                nb_accessibles++;
            }
        }
        printf("Total : %d etats accessibles\n", nb_accessibles);

        // 4. Affichage matriciel
        printf("\n3. Vecteur ligne (1x%d) :\n", distribution_finale->cols);
        printf("[ ");
        for (int i = 0; i < distribution_finale->cols; i++) {
            printf("%.6f", distribution_finale->data[0][i]);
            if (i < distribution_finale->cols - 1) printf(", ");
            if ((i + 1) % 5 == 0 && i < distribution_finale->cols - 1) {
                printf("\n  ");
            }
        }
        printf(" ]\n");

        liberer_matrice(distribution_finale);
    }

    // Libérer les matrices temporaires
    liberer_matrice(mat_init);
    liberer_matrice(M_puissance_n);

    printf("\n=========================================\n");

    // Continuer avec le reste de votre code...
    printf("============================================\n");
    printf("Convergence\n");
    printf("============================================\n");

    // Convergence M^k
    printf("Convergence (epsilon = 0.01)\n");

    t_matrix* Mk = creer_matrice_valzeros(g.nb_sommets, g.nb_sommets);
    t_matrix* Mk_prev = creer_matrice_valzeros(g.nb_sommets, g.nb_sommets);
    t_matrix* temp = NULL;

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
        temp = multiplication_matrice(Mk, M);
        if (temp == NULL) {
            printf("Erreur dans la multiplication matricielle\n");
            break;
        }
        liberer_matrice(Mk);
        Mk = temp;
        temp = NULL;

        diff = difference_matrix(Mk, Mk_prev);
        iterations++;

        printf("Iteration %d: difference = %.6f", iterations, diff);
        if (diff <= epsilon) {
            printf(" (convergence atteinte)\n");
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

        // 1. Sous-matrice carrée pour calcul de période
        t_matrix* square_sub_matrice = extractSquareSubMatrix(M, partition, compo_index);

        if (square_sub_matrice != NULL) {
            printf("Sous-matrice CARREE %dx%d (pour calcul de periode):\n",
                   square_sub_matrice->lignes, square_sub_matrice->cols);
            afficher_matrice(square_sub_matrice);

            // Calcul de la période
            printf("Calcul de la periode pour cette classe...\n");
            int period = getPeriod(square_sub_matrice);
            if (period >= 0) {
                printf("Periode de la classe C%d : %d\n", compo_index + 1, period);

                // Classification basée sur la période
                if (period == 1) {
                    printf("Classe C%d est APERIODIQUE\n", compo_index + 1);
                } else {
                    printf("Classe C%d est PERIODIQUE (periode = %d)\n", compo_index + 1, period);
                }
            } else {
                printf("Erreur dans le calcul de la periode\n");
            }

            liberer_matrice(square_sub_matrice);
        }

        // 2. Sous-matrice originale (toutes lignes, colonnes de la classe)
        t_matrix* sous_matrice = subMatrix(M, partition, compo_index);
        if (sous_matrice != NULL) {
            printf("Sous-matrice ORIGINALE %dx%d (toutes lignes, colonnes de la classe):\n",
                   sous_matrice->lignes, sous_matrice->cols);
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

    // Nettoyage mémoire
    for (int i = 0; i < g.nb_sommets; i++) {
        cell *current = g.tab_liste[i].head;
        while (current != NULL) {
            cell *tmp = current;
            current = current->suivante;
            free(tmp);
        }
    }
    free(g.tab_liste);

    // Libération mémoire additionnelle
    free(corresp);
    free(liens->links);
    free(liens);

    // Liberation des matrices
    if (temp != NULL) {
        liberer_matrice(temp);
    }

    liberer_matrice(Mk);
    liberer_matrice(Mk_prev);
    liberer_matrice(M);

    return 0;
}