#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// Paramètres du jeu
#define LARGEUR_MAX 9	// nb max de coups possibles pour un état donné
#define NB_LIGNE 6
#define NB_COLONNE 7
#define TEMPS 5	// temps de calcul pour un coup avec MCTS (en secondes)
#define C sqrt(2) 	// constante pour UCT

// macros
#define AUTRE_JOUEUR(i) (1 - (i))
#define min(a, b)       ((a) < (b) ? (a) : (b))
#define max(a, b)       ((a) < (b) ? (b) : (a))

// Critères de fin de partie
typedef enum { NON, MATCHNUL, ORDI_GAGNE, HUMAIN_GAGNE } FinDePartie;

// Definition du type Etat (état/position du jeu)
typedef struct EtatSt {
    int joueur; // à qui de jouer ?
    char plateau[NB_LIGNE][NB_COLONNE];
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
Etat *copieEtat(Etat *src) {
    Etat *etat = (Etat *)malloc(sizeof(Etat));

    etat->joueur = src->joueur;

    for (int i = 0; i < NB_LIGNE; i++)
        for (int j = 0; j < NB_COLONNE; j++)
            etat->plateau[i][j] = src->plateau[i][j];

    return etat;
}

// Etat initial
Etat *etat_initial(void) {
    Etat *etat = (Etat *)malloc(sizeof(Etat));

    etat->joueur = 0; // Le joueur humain commence

    for (int i = 0; i < NB_LIGNE; i++)
        for (int j = 0; j < NB_COLONNE; j++)
            etat->plateau[i][j] = ' ';

    return etat;
}

void afficheJeu(Etat *etat) {
    // Affichage des numéros de colonnes
    printf("   ");
    for (int j = 0; j < NB_COLONNE; j++)
        printf(" %d  ", j);
    printf("\n");

    // Affichage du plateau de jeu
    for (int i = 0; i < NB_LIGNE; i++) {
        printf(" |");
        for (int j = 0; j < NB_COLONNE; j++)
            printf(" %c |", etat->plateau[i][j]);
        printf("\n");

        // Ligne de séparation
        printf(" |");
        for (int j = 0; j < NB_COLONNE; j++)
            printf("---|");
        printf("\n");
    }
}

// Nouveau coup
Coup *nouveauCoup(int ligne, int colonne) {
    Coup *coup = (Coup *)malloc(sizeof(Coup));

    coup->ligne = ligne;
    coup->colonne = colonne;

    return coup;
}

// Demander à l'humain quel coup jouer
Coup *demanderCoup(Etat *etat) {
    int colonne;

    printf("\nQuelle colonne ? ");
    scanf("%d", &colonne);

    // S'assurer que la colonne est dans les limites du plateau
    while (colonne < 0 || colonne >= NB_COLONNE) {
        printf("Veuillez choisir une colonne valide (entre 0 et %d) : ", NB_COLONNE - 1);
        scanf("%d", &colonne);
    }

    // Recherche de la première ligne vide dans la colonne
    int ligne = NB_LIGNE - 1;
    while (etat->plateau[ligne][colonne] != ' ' && ligne >= 0)
        ligne--;

    return nouveauCoup(ligne, colonne);
}

// Modifier l'état en jouant un coup
int jouerCoup(Etat *etat, Coup *coup) {
    if (etat->plateau[coup->ligne][coup->colonne] != ' ')
        return 0;
    else {
        etat->plateau[coup->ligne][coup->colonne] = etat->joueur ? 'O' : 'X';
        etat->joueur = AUTRE_JOUEUR(etat->joueur);
        return 1;
    }
}

// Retourne une liste de coups possibles à partir d'un etat
Coup **coups_possibles(Etat *etat) {
    Coup **coups = (Coup **)malloc((1 + NB_COLONNE) * sizeof(Coup *));
    int k = 0;

    for (int j = 0; j < NB_COLONNE; j++) {
        // On parcourt les lignes de bas en haut pour trouver la première case vide
        for (int i = NB_LIGNE - 1; i >= 0; i--) {
            if (etat->plateau[i][j] == ' ') {
                coups[k] = nouveauCoup(i, j);
                k++;
                break;
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
FinDePartie testFin( Etat * etat ) {

    // Vérifier les lignes et colonnes
    for (int i = 0; i < NB_LIGNE; i++) {
        for (int j = 0; j < NB_COLONNE; j++) {
            char joueur = etat->plateau[i][j];
            if (joueur == ' ') continue;  // Case vide, passer à la suivante

            // Vérification des lignes
            if (j + 3 < NB_COLONNE &&
                joueur == etat->plateau[i][j + 1] &&
                joueur == etat->plateau[i][j + 2] &&
                joueur == etat->plateau[i][j + 3]) {
                return (joueur == 'O') ? ORDI_GAGNE : HUMAIN_GAGNE;
            }

            // Vérification des colonnes
            if (i + 3 < NB_LIGNE &&
                joueur == etat->plateau[i + 1][j] &&
                joueur == etat->plateau[i + 2][j] &&
                joueur == etat->plateau[i + 3][j]) {
                return (joueur == 'O') ? ORDI_GAGNE : HUMAIN_GAGNE;
            }

            // Vérification des diagonales ascendantes '/'
            if (i - 3 >= 0 && j + 3 < NB_COLONNE &&
                joueur == etat->plateau[i - 1][j + 1] &&
                joueur == etat->plateau[i - 2][j + 2] &&
                joueur == etat->plateau[i - 3][j + 3]) {
                return (joueur == 'O') ? ORDI_GAGNE : HUMAIN_GAGNE;
            }

            // Vérification des diagonales descendantes '\'
            if (i + 3 < NB_LIGNE && j + 3 < NB_COLONNE &&
                joueur == etat->plateau[i + 1][j + 1] &&
                joueur == etat->plateau[i + 2][j + 2] &&
                joueur == etat->plateau[i + 3][j + 3]) {
                return (joueur == 'O') ? ORDI_GAGNE : HUMAIN_GAGNE;
            }
        }
    }

    // Si le plateau est rempli, c'est un match nul
    for (int i = 0; i < NB_LIGNE; i++) {
        for (int j = 0; j < NB_COLONNE; j++) {
            if (etat->plateau[i][j] == ' ') {
                return NON; // Il reste au moins une case vide, le jeu continue
            }
        }
    }

    return MATCHNUL; // Le plateau est rempli et aucun joueur n'a gagné, c'est un match null
}

// Retourne la B-valeur d'un noeud
double Bvaleur(Noeud * noeud) {
    
    if (noeud->nb_simus == 0)
        return 1000000; 
    
    return (double)noeud->nb_victoires/(double)noeud->nb_simus+C*(sqrt(log(noeud->parent->nb_simus)/(double)noeud->nb_simus));
}

// Vérifie si un coup est gagnant dans l'état actuel du plateau
int estCoupGagnant(Etat *etat, Coup *coup) {
    // On copie l'état actuel pour ne pas le modifier directement
    Etat *copie = copieEtat(etat);

    // On joue le coup
    jouerCoup(copie, coup);

    // On vérifie si le coup est gagnant
    FinDePartie resultat = testFin(copie);

    // On libère la mémoire de la copie
    free(copie);

    // On retourne vrai si le coup est gagnant et faux sinon
    return (resultat == ORDI_GAGNE);
}

// Calcule et joue un coup de l'ordinateur avec MCTS-UCT
// en tempsmax secondes
void ordijoue_mcts(Etat * etat, int tempsmax) {

	clock_t tic, toc;
	tic = clock();
	int temps;

	Coup ** coups;
	Coup * meilleur_coup ;
	
	// Créer l'arbre de recherche
	Noeud * racine = nouveauNoeud(NULL, NULL);	
	racine->etat = copieEtat(etat); 
	
	/* Suppression du coup aléatoire
    // créer les premiers noeuds:
	coups = coups_possibles(racine->etat); 
	int k = 0;
	Noeud * enfant;
	while ( coups[k] != NULL) {
		enfant = ajouterEnfant(racine, coups[k]);
		k++;
	}

	meilleur_coup = coups[ rand()%k ]; // choix aléatoire
    */


	/*  TODO :
		- supprimer la sélection aléatoire du meilleur coup ci-dessus
		- implémenter l'algorithme MCTS-UCT pour déterminer le meilleur coup ci-dessous
	*/
	
	do {
		// Compléter avec l'algorithme MCTS-UCT

		/* --- Sélectionner r récursivement à partir de la racine le nœud avec la plus grande
        B-valeur jusqu’à un nœud terminal ou un avec un fils non développé ---*/
        Noeud * noeud = racine;
        // Tant que le noeud n'est pas terminal
        while (noeud->nb_enfants > 0) {
            double meilleur_bvaleur = -1000000;
            Noeud * meilleur_noeud = NULL;

            // Sélectionner le nœud avec la plus grande B-valeur
            for (int i = 0; i < noeud->nb_enfants; i++) {
                double bvaleur = Bvaleur(noeud->enfants[i]);
                if (bvaleur > meilleur_bvaleur) {
                    meilleur_noeud= noeud->enfants[i];
                    meilleur_bvaleur = bvaleur;
                }
            }

            noeud = meilleur_noeud;
        }
        /* ------------------------------------------------------------------- */

        /* Développer le nœud sélectionné (si nécessaire) et sélectionner aléatoirement
        un fils parmi les fils non explorés (ou le nœud lui-même s’il est terminal) */
        
        // Si le nœud n'a pas été visité et qu'il n'est pas terminal
        if (noeud->nb_simus > 0 && testFin(noeud->etat) == NON) {
            coups = coups_possibles(noeud->etat);
            int k = 0;

            // Tant que des coups sont possibles, on ajoute des enfants au noeud
            while (coups[k] != NULL) {
                ajouterEnfant(noeud, coups[k]);
                k++;
            }
            // On sélectionne un enfant aléatoirement si des coups sont possibles
            if (k > 0) {
                noeud = noeud->enfants[rand() % k];
            }
            free(coups);
        }
        /* ------------------------------------------------------------------- */


        /* Simuler la fin de la partie avec une marche aléatoire */
        Etat * copie = copieEtat(noeud->etat);
        // On simule la fin de la partie avec une marche aléatoire
        while (testFin(copie) == NON) {
            coups = coups_possibles(copie); 
            int k = 0; 
            while (coups[k] != NULL) {
                jouerCoup(copie, coups[k]);
                k++;
            }
            free(coups);
        }
        // On retourne le résultat de la partie
        FinDePartie fin = testFin(copie);
        double r = 0;
        if(fin == HUMAIN_GAGNE){ // Si l'humain gagne, c'est une victoire
            r = 0;
        }
        else {
            r = 1;
        }
        /* ------------------------------------------------------------------- */
        

        /*Mettre à jour les B-valeurs de tous les nœuds sur le chemin de la racine au
        nœud sélectionné en remontant la récompense r de la position finale*/
        while (noeud != NULL) {
            noeud->nb_simus++;
            noeud->nb_victoires += r;
            noeud = noeud->parent;
        }

        /* ------------------------------------------------------------------- */
        
        free(copie);

		toc = clock(); 
		temps = (int)( ((double) (toc - tic)) / CLOCKS_PER_SEC );
	} while ( temps < tempsmax );

   // Vérifier d'abord s'il y a un coup gagnant parmi les coups possibles
    int coupGagnantIndex = -1;
    // On parcourt les enfants du noeud racine pour trouver un coup gagnant
    for (int i = 0; i < racine->nb_enfants; i++) {
        if (estCoupGagnant(racine->etat, racine->enfants[i]->coup)) {
            coupGagnantIndex = i;
            break;
        }
    }

    Noeud *meilleur_noeud = NULL;

    // Si un coup gagnant est trouvé, jouer ce coup directement
    if (coupGagnantIndex != -1) {
        printf("Coup gagnant trouvé\n");
        meilleur_coup = racine->enfants[coupGagnantIndex]->coup;
    } else {
        // Sinon, choisir le coup avec MCTS-UCT
        double meilleur_score = -100000;
        
        for (int i = 0; i < racine->nb_enfants; i++) {
            double score = Bvaleur(racine->enfants[i]);
            if (score > meilleur_score) {
                meilleur_score = score;
                meilleur_coup = racine->enfants[i]->coup;
                meilleur_noeud = racine->enfants[i];
            }
        }
    }

    /* Question 1*/
    // On affiche le nombre de simulations pour le meilleur coup lorsque c'est l'algo MCTS-UCT qui joue
    printf("Nombre de simulations pour le meilleur coup : %d\n", meilleur_noeud->nb_simus);
	printf("Probabilité de Victoire : %f\n", 100*((double)meilleur_noeud->nb_victoires/(double)meilleur_noeud->nb_simus));
	/* ----------*/
	
	// Jouer le meilleur premier coup
	jouerCoup(etat, meilleur_coup );
	
	// Penser à libérer la mémoire :
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
				coup = demanderCoup(etat);
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
