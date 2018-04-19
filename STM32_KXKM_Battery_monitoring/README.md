# STM32 - Monitoring batterie

## Compilation
Dans les préférences Arduino ajouter https://github.com/stm32duino/BoardManagerFiles/raw/master/STM32/package_stm_index.json à la liste "URL de gestionnaire de cartes supplémentaires" puis installer STM32 Cores depuis le gestionnaire de carte (pour installer les outils, etc).


* Type de carte : KXKM_STM32F030 (dans le 2e menu STM32)
* Serial interface : Generic interface
* Optimize : Smallest (-Os) with LTO

## Démarrage / extinction
Un appui long sur le bouton est nécessaire pour démarrer la carte. Un nouvel appui long déclenche l'extinction.

L'extinction se fait automatiquement si la tension batterie est trop faible, ou peut être déclenchée à distance (par l'ESP32). En revanche un accès physique est toujours obligatoire pour le démarrage.


## Fonctionnement de la détection du type de batterie
La position du switch 3 positions **au démarrage** détermine le type de batterie.

### Position LiPo / LiFe
Le nombre de cellules est choisi automatiquement. Pour éviter les recoupements entre les plages de tensions, la batterie ne doit pas être complètement déchargée : pour les batteries LiPo il faut au moins 3.5V par cellule, pour les LiFe au moins 2.9V par cellule.

Les batteries 5S, 6S ne sont pas supportées (recoupement avec 3S / 7S).

Si le nombre de cellules n'a pas pu être déterminé (tension en dehors des plages acceptables) alors la carte ne démarre pas et la LED rouge clignote rapidement jusqu'à ce que le bouton soit relâché.

Sinon la carte démarre (la jauge LED se remplit) et une LED s'allume  au bout de 2s pour indiquer le niveau de batterie. Un appui bref sur le bouton affiche toute la jauge.

### Position custom
En position Custom, tant qu'un profil de batterie n'a pas été précisé, **le monitoring de batterie est désactivé**. La LED rouge reste allumée en fixe et la jauge complète clignote après un appui bref sur le bouton.

Une fois qu'un profil de batterie a été transmis par l'ESP32 la carte se comporte de la même façon qu'en position LiPo / LiFe. Si la tension est inférieure à la tension de coupure du profil, la carte s'éteint immédiatement.

Les profils de batterie consistent en 7 tensions qui représentent la tension de coupure, les tensions à 1/6 ; 1/3 ; 1/2 ; 4/6 ; 2/3 ; 5/6 de charge, et la tension à pleine charge. Certaines tensions intermédiaires peuvent être omises mais les tensions de coupure et à pleine charge sont obligatoires.


## Sortie de puissance
La carte comporte une sortie de puissance (10A max) qui peut être commandée par l'ESP32.

Par défaut, la sortie est activée au démarrage après 2s sauf si une commande est transmise avant la fin de ce délai. Ceci permet de retarder ou désactiver entièrement l'activation de la sortie si nécessaire.


## Section critique
L'ESP32 peut annoncer lorsqu'il entre dans une section critique où l'alimentation ne devrait pas être coupée (par ex pour éviter les corruptions de carte SD). La section critique a une durée maximale et se termine à la fin de cet intervalle de temps ou si une commande a été reçue pour en annoncer la fin.

Lorsqu'on est dans une section critique, l'extinction de la carte en cas d'appui long sur le bouton ou de batterie faible est reportée (indicateur LED "aller retour") tant que la section critique n'est pas terminée.
