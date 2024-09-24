# Puissance 4 avec IA utilisant l'algorithme MCTS

Ce projet implémente un jeu de **Puissance 4** avec une intelligence artificielle basée sur l'algorithme **MCTS** (Monte Carlo Tree Search). Cet algorithme permet de sélectionner les meilleurs coups après avoir effectué de nombreuses simulations de parties, offrant à l'IA la capacité de choisir le coup qui maximise les chances de victoire.

### Groupe
- GEHIN Sandy
- BELUCHE Quentin

## Fonctionnement de l'algorithme MCTS

L'algorithme MCTS fonctionne en simulant plusieurs parties à partir de l'état actuel du jeu. À chaque tour, l'IA réalise les étapes suivantes :

1. **Sélection** : exploration de l'arbre des possibilités à partir de l'état actuel du jeu.
2. **Expansion** : ajout de nouveaux nœuds pour explorer davantage de coups.
3. **Simulation** : exécution de simulations de parties complètes de manière aléatoire pour estimer l'issue potentielle.
4. **Rétropropagation** : mise à jour des probabilités de succès en remontant l'arbre à partir des résultats des simulations.

Cela permet à l'IA de choisir le coup ayant la plus grande chance de succès dans un large éventail de parties simulées.

## Optimisation avec gcc -O3

Le programme peut être compilé avec l'option **gcc -O3** pour optimiser l'exécution. Cette option permet :

- Une exécution plus rapide en optimisant les boucles, les appels de fonctions et les registres.
- Une meilleure gestion de l'espace mémoire.
- Une augmentation du nombre de simulations possibles dans un temps donné, améliorant ainsi la qualité des décisions prises par l'IA.

## Amélioration des coups gagnants

Une amélioration importante a été ajoutée : l'IA est désormais capable de vérifier si un coup gagnant est possible à chaque tour. Si c'est le cas, l'IA choisira ce coup directement, évitant de s'appuyer uniquement sur les simulations MCTS qui, en raison de leur nature probabiliste, pourraient passer à côté d'un coup décisif. Cela garantit une meilleure performance de l'IA, en particulier dans les situations de fin de partie.

## Comparaison des critères "max" et "robuste"

Deux critères ont été utilisés pour évaluer le choix final du coup par l'IA :

- **Max** : sélectionne le coup ayant la plus grande probabilité de victoire.
- **Robuste** : sélectionne le coup minimisant les chances de défaite.

Ces deux critères conduisent parfois à des stratégies différentes. Le critère "robuste" tend à donner une meilleure performance en permettant à l'IA de jouer plus prudemment, en maximisant les chances de ne pas perdre, ce qui peut prolonger la partie et donner l'impression d'une IA plus compétente.

## Estimation du temps de calcul avec Minimax

Une estimation du temps de calcul nécessaire pour jouer le premier coup avec l'algorithme **Minimax** (sans optimisation alpha-beta) montre qu’il serait extrêmement long sans limites de profondeur. Par exemple, avec un plateau de 6 lignes et 7 colonnes, l’espace de recherche s’élève à environ **78 millions de coups**, rendant cette approche peu pratique sans optimisation.

---

Ce projet démontre comment l'algorithme MCTS, couplé à des optimisations et des améliorations spécifiques, permet à l'IA de jouer de manière compétitive en choisissant les meilleurs coups possibles dans un environnement de jeu complexe comme le **Puissance 4**.
