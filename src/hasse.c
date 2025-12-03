#include <malloc.h>
#include "hasse.h"
#include "list.h"
#include "tarjan.h"
#include "utils.h"

t_link_array* rencenser(listeAdj *g, int *t_link_corresp) {
    t_link_array *link = creer_list_link(g->nb_sommets);

    //Création d'une simple matrice pour stockage de ci -> cj
    int max_classes = g->nb_sommets; // worst case
    int **seen = malloc(max_classes * sizeof(int*));
    for (int i = 0; i < max_classes; i++) {
        seen[i] = calloc(max_classes, sizeof(int));
    }

    for (int i=0; i < g->nb_sommets; i++) {
        int Ci = t_link_corresp[i];
        cell *voisin = g->tab_liste[i].head;

        while (voisin != NULL) {
            int j = voisin->sommet_arrivee -1;
            int Cj = t_link_corresp[j];

            if (Ci != Cj && !seen[Ci][Cj]) {
                ajouter_link(link, Ci, Cj);
                seen[Ci][Cj] = 1; // Mark as seen
            }
            voisin = voisin->suivante;
        }
    }

    //libere la matrice nouvellement crée
    for (int i = 0; i < max_classes; i++) {
        free(seen[i]);
    }
    free(seen);

    return link;
}

//Création d'un tableau de liens entre classes
t_link_array* creer_list_link(int nb_sommets) {
    t_link_array *link = malloc(sizeof(t_link_array));
    link->links=malloc(nb_sommets * nb_sommets * sizeof(t_link));
    link->log_size = 0;
    link->alloc_size = nb_sommets * nb_sommets;
    return link;
}

int* creer_tab_corresp(t_partition *partition, int nb_sommets) {
    int *tab = malloc(nb_sommets * sizeof(int));
    for (int i = 0; i < partition->taille; i++) {
        for (int j = 0; j < partition->classes[i].taille; j++) {
            int sommet = partition->classes[i].sommets[j];
            tab[sommet - 1] = i;
        }
    }

    return tab;
}

//Rajout de chaque lien Ci -> Cj
void ajouter_link(t_link_array *link, int from, int to) {
    if (link->log_size < link->alloc_size) {
        link->links[link->log_size].from =from;
        link->links[link->log_size].to =to;
        link->log_size++;
    }
}

//generer le fichier Mermaid du diagramme de Hasse
void genererHasseMermaid(t_partition *partition, t_link_array *liens, const char* nomFichier, int avecRedondances) {
    FILE *file = fopen(nomFichier, "w");

    // En-tête exact comme demandé
    fprintf(file, "---\n");
    fprintf(file, "config:\n");
    fprintf(file, "   layout: elk\n");
    fprintf(file, "   theme: mc\n");
    fprintf(file, "   look: classic\n");
    fprintf(file, "---\n");
    fprintf(file, "\nflowchart LR\n");

    // Tableau stocke ID des classes
    char** id_classes = malloc(partition->taille * sizeof(char*));

    // Générer ID pour chaque classe
    for (int i = 0; i < partition->taille; i++) {
        id_classes[i] = getID(i + 1);
    }

    // Sommets du diagramme de Hasse
    for (int i = 0; i < partition->taille; i++) {
        fprintf(file, "%s[\"", id_classes[i]);
        fprintf(file, "{");

        // Sommets de la classe
        for (int j = 0; j < partition->classes[i].taille; j++) {
            fprintf(file, "%d", partition->classes[i].sommets[j]);
            if (j < partition->classes[i].taille - 1) {
                fprintf(file, ",");
            }
        }
        fprintf(file, "}\"]\n");
    }
    fprintf(file, "\n");

    // Suppression des redondances
    if (!avecRedondances) {
        removeTransitiveLinks(liens);
    }

    // Liens entre classes
    for (int i = 0; i < liens->log_size; i++) {
        int from = liens->links[i].from;
        int to = liens->links[i].to;

        if (from >= 0 && from < partition->taille && to >= 0 && to < partition->taille) {
            fprintf(file, "%s --> %s\n", getID(from + 1), getID(to + 1));
        }
    }

    fclose(file);
}

//Supprimer les redondances
void removeTransitiveLinks(t_link_array *p_link_array)
{
    int i = 0;
    while (i < p_link_array->log_size)
    {
        t_link link1 = p_link_array->links[i];
        int to_remove = 0;
        for (int j = 0; j < p_link_array->log_size && !to_remove; j++)
        {
            if (j != i)
            {
                t_link link2 = p_link_array->links[j];
                if (link2.from == link1.from)
                {
                    for (int k = 0; k < p_link_array->log_size && !to_remove; k++)
                    {
                        if (k != i && k != j)
                        {
                            t_link link3 = p_link_array->links[k];
                            if (link3.from == link2.to && link3.to == link1.to)
                            {
                                to_remove = 1;
                            }
                        }
                    }
                }
            }
        }
        if (to_remove)
        {
            p_link_array->links[i] = p_link_array->links[p_link_array->log_size - 1];
            p_link_array->log_size--;
        }
        else
        {
            i++;
        }
    }
}

//Fonction pour analyser les caractéristiques de notre graphe
void analyserCarac(t_partition *partition, listeAdj *g) {
    printf("\nCaracteristique du graphe\n");

    // Tableau pour savoir à quelle classe appartient chaque sommet
    int *IDclasse = malloc(g->nb_sommets * sizeof(int));
    for (int i = 0; i < partition->taille; i++) {
        for (int j = 0; j < partition->classes[i].taille; j++) {
            IDclasse[partition->classes[i].sommets[j] - 1] = i;
        }
    }

    // Vérifier si chaque classe est persistante
    int *estPersistante = malloc(partition->taille * sizeof(int));
    for (int i = 0; i < partition->taille; i++) {
        estPersistante[i] = 1;

        for (int j = 0; j < partition->classes[i].taille && estPersistante[i]; j++) {
            int sommet = partition->classes[i].sommets[j] - 1;
            cell *voisin = g->tab_liste[sommet].head;

            while (voisin != NULL && estPersistante[i]) {
                int destClasse = IDclasse[voisin->sommet_arrivee - 1];
                if (destClasse != i) estPersistante[i] = 0;
                voisin = voisin->suivante;
            }
        }
    }

    // Affichage
    printf("\n");
    for (int i = 0; i < partition->taille; i++) {
        printf("La classe {");
        for (int j = 0; j < partition->classes[i].taille; j++) {
            printf("%d%s", partition->classes[i].sommets[j],
                   (j < partition->classes[i].taille - 1) ? "," : "");
        }
        printf("} est %s", estPersistante[i] ? "persistante" : "transitoire");

        // Affichage des états
        if (partition->classes[i].taille == 1) {
            printf(" : l'etat %d est %s", partition->classes[i].sommets[0],
                   estPersistante[i] ? "persistant" : "transitoire");
            if (estPersistante[i]) printf(" : l'etat %d est absorbant", partition->classes[i].sommets[0]);
        } else {
            printf(" : les etats ");
            for (int j = 0; j < partition->classes[i].taille; j++) {
                printf("%d", partition->classes[i].sommets[j]);
                if (j < partition->classes[i].taille - 2) printf(", ");
                else if (j == partition->classes[i].taille - 2) printf(" et ");
            }
            printf(" sont %s", estPersistante[i] ? "persistants" : "transitoires");
        }
        printf(";\n");
    }

    // Irréductibilité
    printf("Le graphe de Markov ");
    if (partition->taille == 1) {
        printf("est irreductible.\n");
    } else {
        printf("n'est pas irreductible.\n");
    }

    free(IDclasse);
    free(estPersistante);
}