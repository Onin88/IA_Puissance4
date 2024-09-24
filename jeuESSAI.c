/*
	Canvas pour algorithmes de jeux à 2 joueurs
	
	joueur 0 : humain
	joueur 1 : ordinateur
			
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// Paramètres du jeu
#define LARGEUR_MAX 7 		// nb max de fils pour un noeud (= nb max de coups possibles)

#define TEMPS 3		// temps de calcul pour un coup avec MCTS (en secondes)

#define C sqrt(2)   // constante pour UCT

// macros
#define AUTRE_JOUEUR(i) (1-(i))
#define min(a, b)       ((a) < (b) ? (a) : (b))
#define max(a, b)       ((a) < (b) ? (b) : (a))

// Critères de fin de partie
typedef enum {NON, MATCHNUL, ORDI_GAGNE, HUMAIN_GAGNE } FinDePartie;

// Definition du type Etat (état/position du jeu)
typedef struct EtatSt {

	int joueur; // à qui de jouer ? 

	char plateau[6][7];	

} Etat;

// Definition du type Coup
typedef struct {
	int ligne;
	int colonne;

} Coup;


// Definition du type Noeud 
typedef struct NoeudSt {
		
	int joueur; // joueur qui a joué pour arriver ici
	Coup * coup;   // coup joué par ce joueur pour arriver ici
	
	Etat * etat; // etat du jeu
			
	struct NoeudSt * parent; 
	struct NoeudSt * enfants[LARGEUR_MAX]; // liste d'enfants : chaque enfant correspond à un coup possible
	int nb_enfants;	// nb d'enfants présents dans la liste
	
	// POUR MCTS:
	int nb_victoires;
	int nb_simus;
	
} Noeud;

// Copier un état 
Etat * copieEtat( Etat * src ) {
	Etat * etat = (Etat *)malloc(sizeof(Etat));

	etat->joueur = src->joueur;
	
	int i,j;	
	for (i=0; i< 6; i++)
		for ( j=0; j<7; j++)
			etat->plateau[i][j] = src->plateau[i][j];
	

	
	return etat;
}

// Etat initial 
Etat * etat_initial( void ) {
	Etat * etat = (Etat *)malloc(sizeof(Etat));
	
	int i,j;	
	for (i=0; i< 6; i++)
		for ( j=0; j<7; j++)
			etat->plateau[i][j] = ' ';
	
	return etat;
}


void afficheJeu(Etat * etat) {
	int i,j;
	printf("   |");
	for ( j = 0; j < 7; j++) 
		printf(" %d |", j);
	printf("\n");
	printf("--------------------------------");
	printf("\n");
	
	for(i=0; i < 6; i++) {
		printf(" %d |", i);
		for ( j = 0; j < 7; j++) 
			printf(" %c |", etat->plateau[i][j]);
		printf("\n");
		printf("--------------------------------");
		printf("\n");
	}
}



// Nouveau coup 
// TODO: adapter la liste de paramètres au jeu
Coup * nouveauCoup( int i, int j ) {
	Coup * coup = (Coup *)malloc(sizeof(Coup));
	
	coup->ligne = i;
	coup->colonne = j;
	
	return coup;
}

Coup * nouveauCoupHumain(int j ) {
	Coup * coup = (Coup *)malloc(sizeof(Coup));
	
	coup->colonne = j;
	
	return coup;
}



// Demander à l'humain quel coup jouer 
Coup * demanderCoup () {
	int j;
	printf(" quelle colonne ? ") ;
	scanf("%d",&j); 
	
	return nouveauCoupHumain(j);
}

int jouerCoup( Etat * etat, Coup * coup ) {

	int ligne = 5;
	for(int i = 0 ; i < 5; i++){
		if(etat->plateau[i+1][coup->colonne] != ' '){
			ligne = i;
			break;
		}

	}
	etat->plateau[ligne][coup->colonne] = etat->joueur ? 'R' : 'J';
	etat->joueur = AUTRE_JOUEUR(etat->joueur); 
	return 1;
}

// Retourne une liste de coups possibles à partir d'un etat 
// (tableau de pointeurs de coups se terminant par NULL)
Coup ** coups_possibles(Etat *etat) {
    Coup **coups = (Coup **)malloc((1 + LARGEUR_MAX) * sizeof(Coup *));
    int k = 0;
    for (int j = 0; j < 7; j++) {
        for (int i = 5; i >= 0; i--) { // Parcourir les lignes de bas en haut
            if (etat->plateau[i][j] == ' ') {
                coups[k] = nouveauCoup(i, j);
                k++;
                break; // Sortir de la boucle pour passer à la colonne suivante
            }
        }
    }
    coups[k] = NULL;
    return coups;
}




// Créer un nouveau noeud en jouant un coup à partir d'un parent 
// utiliser nouveauNoeud(NULL, NULL) pour créer la racine
Noeud * nouveauNoeud (Noeud * parent, Coup * coup ) {
	Noeud * noeud = (Noeud *)malloc(sizeof(Noeud));
	
	if ( parent != NULL && coup != NULL ) {
		noeud->etat = copieEtat ( parent->etat );
		jouerCoup ( noeud->etat, coup );
		noeud->coup = coup;			
		noeud->joueur = AUTRE_JOUEUR(parent->joueur);		
	}
	else {
		noeud->etat = NULL;
		noeud->coup = NULL;
		noeud->joueur = 0; 
	}
	noeud->parent = parent; 
	noeud->nb_enfants = 0; 
	
	// POUR MCTS:
	noeud->nb_victoires = 0;
	noeud->nb_simus = 0;	
	

	return noeud; 	
}

// Ajouter un enfant à un parent en jouant un coup
// retourne le pointeur sur l'enfant ajouté
Noeud * ajouterEnfant(Noeud * parent, Coup * coup) {
	Noeud * enfant = nouveauNoeud (parent, coup ) ;
	parent->enfants[parent->nb_enfants] = enfant;
	parent->nb_enfants++;
	return enfant;
}

void freeNoeud ( Noeud * noeud) {
	if ( noeud->etat != NULL)
		free (noeud->etat);
		
	while ( noeud->nb_enfants > 0 ) {
		freeNoeud(noeud->enfants[noeud->nb_enfants-1]);
		noeud->nb_enfants --;
	}
	if ( noeud->coup != NULL)
		free(noeud->coup); 

	free(noeud);
}
	

// Test si l'état est un état terminal 
// et retourne NON, MATCHNUL, ORDI_GAGNE ou HUMAIN_GAGNE
FinDePartie testFin(Etat *etat) {
    // Vérification des lignes
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j <= 3; j++) {
            if (etat->plateau[i][j] != ' ' &&
                etat->plateau[i][j] == etat->plateau[i][j + 1] &&
                etat->plateau[i][j] == etat->plateau[i][j + 2] &&
                etat->plateau[i][j] == etat->plateau[i][j + 3]) {
                return (etat->plateau[i][j] == 'J') ? HUMAIN_GAGNE : ORDI_GAGNE;
            }
        }
    }

    // Vérification des colonnes
    for (int j = 0; j < 7; j++) {
        for (int i = 0; i <= 2; i++) {
            if (etat->plateau[i][j] != ' ' &&
                etat->plateau[i][j] == etat->plateau[i + 1][j] &&
                etat->plateau[i][j] == etat->plateau[i + 2][j] &&
                etat->plateau[i][j] == etat->plateau[i + 3][j]) {
                return (etat->plateau[i][j] == 'J') ? HUMAIN_GAGNE : ORDI_GAGNE;
            }
        }
    }

    // Vérification des diagonales
    for (int i = 0; i <= 2; i++) {
        for (int j = 0; j <= 3; j++) {
            if (etat->plateau[i][j] != ' ' &&
                etat->plateau[i][j] == etat->plateau[i + 1][j + 1] &&
                etat->plateau[i][j] == etat->plateau[i + 2][j + 2] &&
                etat->plateau[i][j] == etat->plateau[i + 3][j + 3]) {
                return (etat->plateau[i][j] == 'J') ? HUMAIN_GAGNE : ORDI_GAGNE;
            }
        }
    }

    for (int i = 0; i <= 2; i++) {
        for (int j = 3; j < 7; j++) {
            if (etat->plateau[i][j] != ' ' &&
                etat->plateau[i][j] == etat->plateau[i + 1][j - 1] &&
                etat->plateau[i][j] == etat->plateau[i + 2][j - 2] &&
                etat->plateau[i][j] == etat->plateau[i + 3][j - 3]) {
                return (etat->plateau[i][j] == 'J') ? HUMAIN_GAGNE : ORDI_GAGNE;
            }
        }
    }

    // Vérification s'il y a encore des cases vides
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 7; j++) {
            if (etat->plateau[i][j] == ' ') {
                return NON; // La partie n'est pas terminée
            }
        }
    }

    // Si aucune condition de victoire n'est remplie et qu'il n'y a plus de cases vides, alors c'est un match nul
    return MATCHNUL;
}

double calculer_B(Noeud * noeud) {
    
    if (noeud->nb_simus == 0)
        return INFINITY; 
    
    return (double)noeud->nb_victoires / (double)noeud->nb_simus + C * sqrt(log(noeud->parent->nb_simus) / (double)noeud->nb_simus);
}


// Calcule et joue un coup de l'ordinateur avec MCTS-UCT
// en tempsmax secondes
void ordijoue_mcts(Etat * etat, int tempsmax) {
    clock_t tic, toc;
    tic = clock();
    int temps;

    Coup ** coups;
    Coup * meilleur_coup = NULL; 

    Noeud * racine = nouveauNoeud(NULL, NULL);
    racine->etat = copieEtat(etat);

    do {
        // Sélection
        Noeud * n = racine;
        while (n->nb_enfants > 0) {
            double meilleur_B = -INFINITY;
            Noeud * meilleur_enfant = NULL; 

            // Parcourir les enfants pour sélectionner celui avec la plus grande valeur B
            for (int i = 0; i < n->nb_enfants; i++) {
                double B = calculer_B(n->enfants[i]);
                if (B > meilleur_B) {
                    meilleur_B = B;
                    meilleur_enfant = n->enfants[i];
                }
            }

            n = meilleur_enfant;
        }

        // Expansion
        if (n->nb_simus > 0 && testFin(n->etat) == NON) {
            coups = coups_possibles(n->etat);
            int k = 0; 
            while (coups[k] != NULL) {
                ajouterEnfant(n, coups[k]);
                k++;
            }
            if (k > 0) { // Vérifiez que des coups ont été générés
                n = n->enfants[rand() % k];
            }
            free(coups);
        }

        // Simulation
        Etat * copie_etat = copieEtat(n->etat);
        FinDePartie fin_simulation;
        while ((fin_simulation = testFin(copie_etat)) == NON) {
            coups = coups_possibles(copie_etat); 
            int k = 0; 
            while (coups[k] != NULL) {
                jouerCoup(copie_etat, coups[k]);
                k++;
            }
            free(coups);
        }
        double resultat_simulation = 0;
        if (fin_simulation == ORDI_GAGNE) {
            resultat_simulation = 1;
        } else if (fin_simulation == HUMAIN_GAGNE) {
            resultat_simulation = 0;
        } else {
            resultat_simulation = 0.5; // Match nul
        }

        // Rétropropagation
        while (n != NULL) {
            n->nb_simus++;
            n->nb_victoires += resultat_simulation;
            n = n->parent;
        }

        free(copie_etat);

        toc = clock();
        temps = (int)(((double)(toc - tic)) / CLOCKS_PER_SEC);
    } while (temps < tempsmax);

    // Sélectionner le meilleur coup
    double meilleur_score = -INFINITY;
	Noeud * meilleur_noeud = NULL;
    for (int i = 0; i < racine->nb_enfants; i++) {
        double score = (double)racine->enfants[i]->nb_victoires / (double)racine->enfants[i]->nb_simus;
        if (score > meilleur_score) {
            meilleur_score = score;
            meilleur_coup = racine->enfants[i]->coup;
			meilleur_noeud = racine->enfants[i];
        }
    }

	printf("Nombre de simulations pour ce coup: %d\n", meilleur_noeud->nb_simus);
    printf("Estimation de la probabilité de victoire pour l'ordinateur: %.2f%%\n", meilleur_score * 100);

    // Jouer le meilleur coup s'il est valide
    if (meilleur_coup != NULL) {
        jouerCoup(etat, meilleur_coup);
    }

    // Libérer la mémoire
    freeNoeud(racine);
}




int main(void) {

	Coup * coup;
	FinDePartie fin;
	
	// initialisation
	Etat * etat = etat_initial(); 
	
	// Choisir qui commence : 
	printf("Qui commence (0 : humain, 1 : ordinateur) ? ");
	scanf("%d", &(etat->joueur) );
	
	// boucle de jeu
	do {
		printf("\n");
		afficheJeu(etat);
		
		if ( etat->joueur == 0 ) {
			// tour de l'humain
			
			do {
				coup = demanderCoup();
			} while ( !jouerCoup(etat, coup) );
									
		}
		else {
			// tour de l'Ordinateur
			
			ordijoue_mcts( etat, TEMPS );
			
		}
		
		fin = testFin( etat );
	}	while ( fin == NON ) ;

	printf("\n");
	afficheJeu(etat);
		
	if ( fin == ORDI_GAGNE )
		printf( "** L'ordinateur a gagné **\n");
	else if ( fin == MATCHNUL )
		printf(" Match nul !  \n");
	else
		printf( "** BRAVO, l'ordinateur a perdu  **\n");
	return 0;
}