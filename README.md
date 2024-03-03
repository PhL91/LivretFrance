# LivretFrance

Petit programme C pour calculer les intérêts des livrets français type Livret A, LDDS,...

L'aide:
```
 $ ./interet -h
Utilisation: interet [OPTION]
  Calcul des intérêts des livrets type France avec intérêts par quinzaine
  -h,--help                             Affichage de l'aide
  -o,--operation <fichier opération>    Défaut: operation.txt
  -t,--taux <fichier taux>              Défaut: taux.txt
  -d,--date <date de simulation>        Défaut: aujourd'hui
  -m,--mode {jour|quinzaine|jourbourso} Défaut: jour - Mode de calcul des intérêts
                                        jour: calcul en nombre de jours véritables (365/366)
                                        quinzaine: calcul en nombre de quinzaines (24)
                                        jourbourso: idem jour sauf nombre de jours/an fixé à 365
```
Exemple de lancement:
```
$ ./interet 

Date prise en compte pour le calcul des intérêts courus: aujourd'hui
Mode de calcul des intérêts:                             jour

Tableau d'évolution du taux d'intérêt:
Date      Quinzaine       Taux
2024-01-01        1       3.00

Nombre d'opérations: 5
Date op      Montant op Nb quinz couru Nb jour couru Int courus Nb quinz an Nb Jour an     Int an
2023-12-31      5682.16              4            60      27.95          24        366     170.46
2024-01-09      1164.00              3            45       4.29          23        351      33.49
2024-01-23      -165.00              3            45      -0.61          23        351      -4.75
2024-02-09      1502.00              1            14       1.72          21        320      39.40
2024-02-15       107.00              1            14       0.12          21        320       2.81
Montant total couru:      33.48
Montant total année:     241.41

```
Exemple de fichier taux (il suffit de rajouter une ligne du même format à chaque changement de taux):
```
$ cat taux.txt
# Commentaire possible avec "#" en 1er caractère
2024-01-01 3.00
```
Remarque: il doit y avoir au moins une ligne pour le 1er janvier!

Exemple de fichier opération:

```
# Le solde initial doit être mis sur la première ligne avec la date du 31 décembre de l'année précédente
2023-12-31 5682.16
2024-01-09 1164
2024-01-23 -165
2024-02-09 1502
2024-02-15 107
```

## Remarque sur les vérifications effectuées par le programme
En abrégé: très réduite

Plus exactement, il n'y a pas de vérifications exhaustives que les valeurs dans les 2 fichiers d'entrée (taux et opération) ne contiennent pas des valeurs aberrantes. Autrement dit, si vous rentrez des valeurs sans réfléchir, le fonctionnement peut s'avérer aléatoire!

## Remarque sur la prise en compte des versements/retraits et le calcul des intérêts

Pour les livrets type livret A, la règle des quinzaines s'appliquent lors des versements/retraits. Un versement ne sera pris en compte qu'à partir de la quinzaine suivante - un retrait sera pris en compte au début de la quinzaine du retrait.

Qu'est qu'une quinzaine? Une quinzaine commence le 1er ou le 16 du mois et se termine le 15 ou le dernier jour du mois.

Deux exemples:

* Je verse 1000€ le 17 janvier: le calcul des intérêts ne prendra en compte ce versement qu'à partir du 01 février (date de valeur)
* Je retire 1000€ le 17 janvier: le calcul des intérêts pour ce montant s'arrêtera le 15 janvier (date de valeur)

Le calcul des intérêts:

Ça se complique! Autant la règle des quinzaines est respectée par toutes les banques en ce qui concerne les dates de valeurs (ça doit être réglementaire), autant pour le calcul, il y a des divergences:

### Calcul en nombre de quinzaines
Le montant des intérêts est calculé en fonction du nombre de quinzaine entière. Un exemple valant mieux qu'un long discours:

* J'ai versé 1000€ le 17 janvier et le taux du livret est à 3%: les intérêts reçus au 31 décembre seront de:
```
1000*(3/100)*(22/24) = 27.50
```
* J'ai retiré 1000€ le 17 janvier et le taux du livret est à 3%: les intérêts **non perçus** (retirés de la somme des intérêts) au 31 décembre:
```
-1000*(3/100)*(23/24) = -28.75
```
### Calcul en nombre de jours
Le montant des intérêts n'est plus calculé en fonction d'un nombre de quinzaine mais en fonction du nombre de jours dans les quinzaines: c'est le mode par défaut du programme. En reprenant les 2 exemples ci-dessus et en supposant être en 2024 qui est une année bissextile (!):

* J'ai versé 1000€ le 17 janvier et le taux du livret A est à 3%: les intérêts reçu au 31 décembre seront de:
```
1000*(3/100)*((1*29+4*30+6*31)/366) = 27.46
```
* J'ai retiré 1000€ le 17 janvier et le taux du livret est à 3%: les intérêts **non perçus** (retirés de la somme des intérêts) au 31 décembre:
```
-1000*(3/100)*((16+1*29+4*30+6*31)/366) = -28.77
```

Comme vous pouvez le constater, ceci introduit de légères différences dans les montants. Cependant, pour une somme présente toute l'année, il n'y aura aucune différence entre les 2 méthodes.

## Remarque sur le mode 'jourbourso'
Boursobank utilise le mode en nombre de jours mais avec un bémol: Si elle prend bien en compte le 29 février dans les quinzaines, il n'est pas pris en compte dans la division: ils semblent diviser toujours par 365 au lieu de 366. Comme c'est plutôt à notre avantage, je vais me garder de le signaler!

Cependant, j'ai ajouté ce mode qui permet de forcer le nombre de jours total de l'année à 365 au lieu de 366 pour les années bissextiles. Cette option est inutile pour une année non bissextile.

Je suppute qu'il s'agit d'un bug chez Bourso.

## Vérification effectuée
J'ai comparé les résultats avec le fichier JxLivretV2f.ods disponible sur Internet et je conclus que c'est correct!
