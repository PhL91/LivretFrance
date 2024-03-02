# LivretFrance

Petit programme C pour calculer les intérêts des livrets français type Livret A, LDDS,...

L'aide:

    $ ./interet -h
    Utilisation: interet [OPTION]
      Calcul des intérêts des livrets type France avec intérêts par quinzaine
      -h,--help				Affichage de l'aide
      -o,--operation <fichier opération>	Défaut: operation.txt
      -t,--taux <fichier taux>		Défaut: taux.txt
      -d,--date <date de simulation>	Défaut: aujourd'hui

Exemple de lancement:
```
$ ./interet

Tableau d'évolution du taux d'intérêt:
Date      Quinzaine       Taux
2024-01-01        1       3.00

Date prise en compte pour le calcul des intérêts courus: aujourd'hui

Nombre d'opérations: 5
Date op      Montant op   Nb quinz couru Int courus Nb quinz an     Int an
2023-12-31      5682.16                3      21.31          24     170.46
2024-01-09      1164.00                2       2.91          23      33.47
2024-01-23      -165.00                2      -0.41          23      -4.74
2024-02-09      1502.00                0       0.00          21      39.43
2024-02-15       107.00                0       0.00          21       2.81
Montant total couru:      23.81
Montant total année:     241.42
```
Exemple de fichier taux (il suffit de rajouter une ligne du même format à chaque changement de taux):

    $ cat taux.txt
    # Commentaire possible avec "#" en 1er caractère
    2024-01-01 3.00

Remarque: il y a au moins une ligne au 1er janvier!

Exemple de fichier opération:

```
# Le solde initial doit être mis sur la première ligne avec la date du 31 décembre de l'année précédente
2023-12-31 5682.16
2024-01-09 1164
2024-01-23 -165
2024-02-09 1502
2024-02-15 107
```

## Remarque sur la prise en compte des versements/retraits et le calcul des intérêts

Pour les livrets type livret A, la règle des quinzaines s'appliquent lors des versements/retraits. Un versement ne sera pris en compte qu'à partir de la quinzaine suivante - un retrait sera pris en compte au début de la quinzaine du retrait.

Qu'est qu'une quinzaine? Une quinzaine commence le 1er ou le 16 du mois et se termine le 15 ou le dernier jour du mois.

Deux exemples:

* Je verse 1000€ le 17 janvier: le calcul des intérêts ne prendra en compte ce versement qu'à partir du 01 février (date de valeur)
* Je retire 1000€ le 17 janvier: le calcul des intérêts pour ce montant s'arrêtera le 15 janvier (date de valeur)

Le calcul des intérêts:

Ça se complique! Autant la règle des quinzaines est respectée par toutes les banques en ce qui concerne les dates de valeurs (ça doit être réglementaire), autant pour le calcul, il y a des divergences:

### Calcul en nombre de quinzaines
C'est la méthode que j'ai utilisée dans le programme ci-dessus. Le montant des intérêts est calculé en fonction du nombre de quinzaine entière. Un exemple valant mieux qu'un long discours:

* J'ai versé 1000€ le 17 janvier et le taux du livret est à 3%: les intérêts reçus au 31 décembre seront de:
```
1000*(3/100)*(22/24) = 27.50
```
* J'ai retiré 1000€ le 17 janvier et le taux du livret est à 3%: les intérêts **non perçus** (retirés de la somme des intérêts) au 31 décembre:
```
-1000*(3/100)*(23/24) = -28.75
```
### Calcul en nombre de jours
Le montant des intérêts n'est plus calculé en fonction d'un nombre de quinzaine mais en fonction du nombre de jours dans les quinzaines. En reprenant les 2 exemples ci-dessus et en supposant être en 2024 qui est une année bissextile (!):

* J'ai versé 1000€ le 17 janvier et le taux du livret A est à 3%: les intérêts reçu au 31 décembre seront de:
```
1000*(3/100)*((1*29+4*30+6*31)/366) = 27.46
```
* J'ai retiré 1000€ le 17 janvier et le taux du livret est à 3%: les intérêts **non perçus** (retirés de la somme des intérêts) au 31 décembre:
```
-1000*(3/100)*((16+1*29+4*30+6*31)/366) = -28.77
```

Comme vous pouvez le constater, ceci introduit de légères différences dans les montants. Cependant, pour une somme présente toute l'année, il n'y aura aucune différence entre les 2 méthodes.
