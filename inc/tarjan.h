#ifndef TARJAN_H
#define TARJAN_H

#include "list.h"

/** Structure pour représenter chaque sommet:
- Numéro de sommet dans le graphe
- Indique si le sommet a été visité
-
- Indique si le sommet est dans la pile
**/
typedef struct tarjan_vertex {
    int id;
    int num;
    int num_access;
    int in_pile;
} t_tarjan_vertex;

/** Structure pour représenter une classe:
- Nom pour l'affichage
- Tableau dynamique avec les indices des sommets
- Nombre de sommets dans la classe
- Capacité du tableau qui augmente au besoin
**/
typedef struct classe {
    char nom[15];
    int *sommets;
    int taille;
    int capacite;
} t_classe;

// Structure d'une partition (= ensemble de classes):
typedef struct partition {
    t_classe *classes;
    int taille;
    int capacite;
} t_partition;

/** Structure contenant toutes les informations nécessaires pour l'algorithme de Tarjan:
- Informations sur chaque sommet
- La pile utilisé
- La taille actuelle de la pile
- Numérotation
- Le graphe
- Le résultat
**/
typedef struct tarjan_data {
    t_tarjan_vertex *sommets;
    int *pile;
    int size_pile;
    int count;
    listeAdj *graphe;
    t_partition *partition;
} t_tarjan_data;

//*******PROTOTYPES*******/

t_tarjan_data* initialiser_tarjan(listeAdj *g);
void tarjan_parcours(t_tarjan_data *data, int sommet_index);
t_partition* algorithme_tarjan(listeAdj g);
void liberer_tarjan(t_tarjan_data *data);
void afficher_partition(t_partition *partition);
t_partition* tarjan_calculer_partition(listeAdj g);

#endif
