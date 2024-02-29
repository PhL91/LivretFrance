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

Exemple de fichier taux (il suffit de rajouter une ligne du même format à chaque changement de taux):

    $ cat taux.txt
    # Commentaire possible avec "#" en 1er caractère
    2024-01-01 3.00

Remarque: il y a au moins une ligne au 1er janvier!

Exemple de fichier opération:

    # Le solde initial doit être mis sur la première ligne avec la date du 31 décembre de l'année précédente
    2023-12-31 5682.16
    2024-01-09 1164
    2024-01-23 -165
    2024-02-09 1502
    2024-02-15 107

