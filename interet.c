/*
"define" ci-dessous nécessaire: sans lui, la compilation donne l'erreur:
warning: implicit declaration of function ‘strptime’
*/
#define _GNU_SOURCE
/*
Nombre maximum d'opérations
*/
#define MAXOP 100
/*
Nombre maximum de caractères dans les noms de fichiers taux et operation
*/
#define MAXCHAR 200

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <getopt.h>
#include <libgen.h>

/*
getfortnight: Renvoie un numéro de quinzaine [1-24] 
en fonction d'une date donnée
*/
int getfortnight(struct tm *ptm) {
/* Attention: tm_mon va de [0,11]: nombre de mois depuis janvier */
	return(ptm->tm_mday > 15 ? ptm->tm_mon*2+2 : ptm->tm_mon*2+1);
}

void usage(char *argv0) {
	printf("Utilisation: %s [OPTION]\n",basename(argv0));
	printf("  Calcul des intérêts des livrets type France avec intérêts par quinzaine\n");
	printf("  -h,--help\t\t\t\tAffichage de l'aide\n");
	printf("  -o,--operation <fichier opération>\tDéfaut: operation.txt\n");
	printf("  -t,--taux <fichier taux>\t\tDéfaut: taux.txt\n");
	printf("  -d,--date <date de simulation>\tDéfaut: aujourd'hui\n");
}

int main(int argc, char *argv[]) {

int i,k,c,qcourantetaux,qcouranteop,qhui,nop,cline=200,flagdatesimu=0,nbquinzhui[MAXOP],nbquinzan[MAXOP];
char fichiertaux[MAXCHAR]="taux.txt";
char fichieroperation[MAXCHAR]="operation.txt";
FILE *pfichiertaux,*pfichieroperation;
char d[20],t[20],m[20],datesimu[20],dateop[MAXOP][20],line[200];
float curtaux,montanttotalhui,montanttotalan,tauxcourant,cumulinteretan[MAXOP],cumulinterethui[MAXOP],montant[MAXOP];
struct datetaux {
	char ascdate[20];
	float taux;
	int fortnightno;
} pdt[24];
struct tm tm,tmdatesimu;
time_t u;
int option_index = 0;
static struct option long_options[] = {
	{"taux",     required_argument, NULL,  't' },
	{"operation",required_argument, NULL,  'o' },
	{"date",     required_argument, NULL,  'd' },
	{"help",     no_argument,       NULL,  'h' },
	{0,          0,                 0,  0 }
};

/* Elements de la ligne de commande */
while (1) {
	c = getopt_long(argc, argv, "ht:o:d:",long_options, &option_index);
	if (c == -1)
		break;
	switch (c) {
		case 0:
			/* Ne devrait jamais passer dans cette option car
			flag est toujours à NULL */
			printf("option %s", long_options[option_index].name);
			if (optarg) printf(" with arg %s\n", optarg);
			printf("\n");
			break;
		case 'h':
			usage(argv[0]);
			exit(EXIT_SUCCESS);
		case 't':
			if (strlen(optarg) > MAXCHAR - 1) {
				printf("Le nombre de caractères du fichier taux excède la limite (%d)\n", MAXCHAR);
				exit(EXIT_FAILURE);
			}
			strcpy(fichiertaux,optarg);
			break;
		case 'o':
			if (strlen(optarg) > MAXCHAR - 1) {
				printf("Le nombre de caractères du fichier operation excède la limite (%d)\n", MAXCHAR);
				exit(EXIT_FAILURE);
			}
			strcpy(fichieroperation,optarg);
			break;
		case 'd':
			if ( strptime(optarg,"%F",&tmdatesimu) == 0 ) {
				printf("\nErreur sur strptime avec la date: %s\n",optarg);
				exit(EXIT_FAILURE);
			}
			flagdatesimu=1;
			strcpy(datesimu,optarg);
			break;
		case '?':
			printf("Erreur dans les options!\n");
			exit(EXIT_FAILURE);
		default:
			printf("Option non reconnue: 0%c\n", c);
			exit(EXIT_FAILURE);
	}
}
if (optind < argc) {
	printf("Arguments inattendus sur la ligne de commande: ");
	while (optind < argc)
		printf("%s ", argv[optind++]);
	printf("\n");
	exit(EXIT_FAILURE);
}

/*
Initialisation du tableau datetaux
*/
for (i=0;i<24;i++) pdt[i].fortnightno=0;

pfichiertaux=fopen(fichiertaux,"r");
if (!pfichiertaux) {
	perror("fopen taux");
	exit(EXIT_FAILURE);
}
while (fgets(line,cline,pfichiertaux) != NULL) {
	if (strncmp("#",line,1) == 0) continue;
	if (sscanf(line,"%s %s",d,t) == EOF) {
		if (ferror(pfichiertaux) != 0) {
			perror("sscanf");
			exit(EXIT_FAILURE);
		}
	}
	if ( strptime(d,"%F",&tm) == 0 ) {
		printf("\nErreur sur strptime avec la date: %s\n",d);
		exit(EXIT_FAILURE);
	}
	i=getfortnight(&tm);
	strcpy(pdt[i-1].ascdate,d);
	pdt[i-1].taux=strtof(t,NULL);
	pdt[i-1].fortnightno=i;
}
fclose(pfichiertaux);
printf("\nTableau d'évolution du taux d'intérêt:\n");
printf("Date      Quinzaine       Taux\n");
for (i=0;i<24;i++) {
	if (pdt[i].fortnightno != 0) {
		printf("%s %8d %#10.2f\n",pdt[i].ascdate,pdt[i].fortnightno,pdt[i].taux);
	}
}
printf("\n");
/*
La valeur de taux dans pdt est rempli avec la valeur courante
pour chaque quinzaine.
A la sortie de la boucle, le taux dans pdt est renseigné pour chaque quinzaine
A noter que la valeur du taux au 1er janvier est obligatoire!
pdt[0].taux correspond au taux du 1er janvier
*/
curtaux=pdt[0].taux;
for (i=1;i<24;i++) {
	if (pdt[i].fortnightno == 0) {
		pdt[i].fortnightno=i+1;
		pdt[i].taux=curtaux;
		strcpy(pdt[i].ascdate,"          ");
	}
	else curtaux=pdt[i].taux;
}

/*
Boucle sur toutes les lignes d'opération:
- Calcul du numéro de semaine sur laquelle s'applique l'opération
- Calcul des intérêts générés jusqu'à aujourd'hui et jusqu'à la fin de l'année
- Impression des données
*/
/*
On recherche le numéro de quinzaine d'aujourd'hui,
sachant qu'il faut lui retirer "1" pour avoir les
intérêts déjà acquis (quinzaine précédente)
*/
/*
Structure tm d'aujourd'hui ou à la date de simulation
*/
u=time(NULL);
tm = *localtime(&u);
/* Numéro de quinzaine d'aujourd'hui */
if (flagdatesimu == 1 ) {
	qhui=getfortnight(&tmdatesimu);
	if (strftime(datesimu,20,"%F",&tmdatesimu) ==0 ) {
		printf("Erreur de conversion avec strftime de la date: %s\n",datesimu);
		exit(EXIT_FAILURE);
	}
	printf("Date prise en compte pour le calcul des intérêts courus: %s\n\n",datesimu);
}
else {
	qhui=getfortnight(&tm);
	printf("Date prise en compte pour le calcul des intérêts courus: aujourd'hui\n\n");
}
/*
Les intérêts ne compteront que pour la quinzaine précédente
*/
qhui=qhui-1;

pfichieroperation=fopen(fichieroperation,"r");
if (!pfichieroperation) {
	perror("fopen operation");
	exit(EXIT_FAILURE);
}
k=0;
while (fgets(line,cline,pfichieroperation) != NULL) {
	if (strncmp("#",line,1) == 0) continue;
	if (sscanf(line,"%s %s",d,m) == EOF) {
		if (ferror(pfichiertaux) != 0) {
			perror("sscanf");
			exit(EXIT_FAILURE);
		}
	}
	if ( strptime(d,"%F",&tm) == 0 ) {
		printf("\nErreur sur strptime avec la date: %s\n",d);
		exit(EXIT_FAILURE);
	}
	if (tm.tm_mday==31 && tm.tm_mon==11) {
		qcouranteop=1;
		}
	else {
		qcouranteop=getfortnight(&tm);
		}
	montant[k]=strtof(m,NULL);
	strcpy(dateop[k],d);
/*
Le 31/12 est particulier: le montant sur cette ligne est le solde du livret 
au 31/12 de l'année précédente
En dehors du 31/12, si le montant est positif, il n'entrera dans le calcul
que la quinzaine suivante.
Pour les montants négatifs, c'est la quinzaine de l'opération.
*/
	if ( montant[k] > 0 && !(tm.tm_mday==31 && tm.tm_mon==11)) qcouranteop=qcouranteop+1;
	cumulinterethui[k]=0;
	cumulinteretan[k]=0;
	nbquinzhui[k]=0;
	nbquinzan[k]=0;
	for (i=0;i<24;i++) {
		tauxcourant=pdt[i].taux;
		qcourantetaux=pdt[i].fortnightno;
		if ( qcourantetaux >= qcouranteop ) {
			cumulinteretan[k]=cumulinteretan[k]+montant[k]*tauxcourant/(24*100);
			nbquinzan[k]=nbquinzan[k]+1;
			if (qcourantetaux <= qhui ) {
				cumulinterethui[k]=cumulinterethui[k]+montant[k]*tauxcourant/(24*100);
				nbquinzhui[k]=nbquinzhui[k]+1;
			}
		}
	}
	k=k+1;
	if (k > MAXOP) {
		printf("Nombre maximal d'opérations dépassé!\n");
		exit(EXIT_FAILURE);
	}
}
fclose(pfichieroperation);
/*
Impression des résultats
*/
nop=k;
printf("Nombre d'opérations: %d\n",nop);
montanttotalhui=0;
montanttotalan=0;
printf("Date op      Montant op   Nb quinz couru Int courus Nb quinz an     Int an\n");
for (i=0;i<nop;i++) {
	montanttotalhui=montanttotalhui+cumulinterethui[i];
	montanttotalan=montanttotalan+cumulinteretan[i];
	printf("%s   %#10.2f                %d %#10.2f          %d %#10.2f\n",dateop[i],montant[i],nbquinzhui[i],cumulinterethui[i],nbquinzan[i],cumulinteretan[i]);
}

printf("Montant total couru: %#10.2f\n",montanttotalhui);
printf("Montant total année: %#10.2f\n",montanttotalan);

exit(EXIT_SUCCESS);
}

