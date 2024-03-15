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
#include <stdbool.h>

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
	printf("  -h,--help                             Affichage de l'aide\n");
	printf("  -o,--operation <fichier opération>    Défaut: operation.txt\n");
	printf("  -t,--taux <fichier taux>              Défaut: taux.txt\n");
	printf("  -d,--date <date de simulation>        Défaut: aujourd'hui\n");
	printf("  -m,--mode {jour|quinzaine|jourbourso} Défaut: jour - Mode de calcul des intérêts\n");
	printf("                                        jour: calcul en nombre de jours véritables (365/366)\n");
	printf("                                        quinzaine: calcul en nombre de quinzaines (24)\n");
	printf("                                        jourbourso: idem jour sauf nombre de jours/an fixé à 365\n");
}

int main(int argc, char *argv[]) {

int i,k,c,qcourantetaux,qcouranteop,qhui,nop,cline=200,an,nbjourantot=0,flagdatesimu=0,nbquinzhui[MAXOP],nbquinzan[MAXOP],nbjouran[MAXOP],nbjourhui[MAXOP];
bool jour=true,anbi,bourso=false;
char fichiertaux[MAXCHAR]="taux.txt";
char fichieroperation[MAXCHAR]="operation.txt";
FILE *pfichiertaux,*pfichieroperation;
char d[20],t[20],m[20],datesimu[20],dateop[MAXOP][20],line[200];
float curtaux,montanttotalhui,montanttotalan,tauxcourant,cumulinteretan[MAXOP],cumulinterethui[MAXOP],montant[MAXOP];
struct datetaux {
	char ascdate[20];
	float taux;
	int fortnightno;
	int nbjour;
} pdt[24];
struct tm tm,tmdatesimu;
time_t u;
int option_index = 0;
static struct option long_options[] = {
	{"taux",     required_argument, NULL,  't' },
	{"operation",required_argument, NULL,  'o' },
	{"date",     required_argument, NULL,  'd' },
	{"mode",     required_argument, NULL,  'm' },
	{"help",     no_argument,       NULL,  'h' },
	{0,          0,                 0,  0 }
};

/* Elements de la ligne de commande */
while (1) {
	c = getopt_long(argc, argv, "ht:o:d:m:",long_options, &option_index);
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
		case 'm':
			if (strcmp(optarg,"quinzaine") == 0) {
				jour=false;
				bourso=false;
				}
			else
				if (strcmp(optarg,"jour") == 0) {
					jour=true;
					bourso=false;
					}
				else
					if (strcmp(optarg,"jourbourso") == 0) {
						jour=true;
						bourso=true;
						}
					else
						{
						printf("mode doit être 'jour','quinzaine' ou 'jourbourso'\n");
						exit(EXIT_FAILURE);
						}
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
Structure tm d'aujourd'hui ou à la date de simulation
*/
u=time(NULL);
tm = *localtime(&u);
/* Numéro de quinzaine d'aujourd'hui */
if (flagdatesimu == 1 ) {
	qhui=getfortnight(&tmdatesimu);
	if (strftime(datesimu,20,"%F",&tmdatesimu) == 0 ) {
		printf("Erreur de conversion avec strftimes\n");
		exit(EXIT_FAILURE);
	}
	printf("\nDate prise en compte pour le calcul des intérêts courus: %s\n",datesimu);
}
else {
	qhui=getfortnight(&tm);
	if (strftime(datesimu,20,"%F",&tm) == 0 ) {
		printf("Erreur de conversion avec strftime\n");
		exit(EXIT_FAILURE);
	}
	printf("\nDate prise en compte pour le calcul des intérêts courus: %s (aujourd'hui)\n",datesimu);
}
printf("Mode de calcul des intérêts                            : %s", (jour? "jour" : "quinzaine" ));
if (bourso) printf(" avec option boursobank");
printf("\n");
/*
Est-on en année bissextile?
*/
if (flagdatesimu == 1 ) {
	an=tmdatesimu.tm_year+1900;
	}
else {
	an=tm.tm_year+1900;
}
if (an % 400 == 0)
	anbi=true;
else
	{
	if (an % 100 == 0)
		anbi=false;
	else
		{
		if(an % 4 == 0)
			anbi=true;
		else
			anbi=false;
		}
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
On en profite aussi pour calculer le nombre de jours dans chaque quinzaine (cas du mode "jour")
*/
curtaux=pdt[0].taux;
pdt[0].nbjour=15;
nbjourantot=pdt[0].nbjour;
for (i=1;i<24;i++) {
	switch (i){
		case 1:
		case 5:
		case 9:
		case 13:
		case 15:
		case 19:
		case 23:
			pdt[i].nbjour=16;
			nbjourantot=nbjourantot+pdt[i].nbjour;
			break;
		case 2:
		case 4:
		case 6:
		case 7:
		case 8:
		case 10:
		case 11:
		case 12:
		case 14:
		case 16:
		case 17:
		case 18:
		case 20:
		case 21:
		case 22:
			pdt[i].nbjour=15;
			nbjourantot=nbjourantot+pdt[i].nbjour;
			break;
		case 3:
			if (anbi)
				pdt[i].nbjour=14;
			else
				pdt[i].nbjour=13;
			nbjourantot=nbjourantot+pdt[i].nbjour;
			break;
		default:
			printf("Cas imprévu sur le nombre de jours par quinzaine: BUG\n");
			exit(EXIT_FAILURE);
	}
	if (pdt[i].fortnightno == 0) {
		pdt[i].fortnightno=i+1;
		pdt[i].taux=curtaux;
		strcpy(pdt[i].ascdate,"          ");
	}
	else curtaux=pdt[i].taux;
}
if (bourso) nbjourantot=365;
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
	nbjourhui[k]=0;
	nbjouran[k]=0;
	for (i=0;i<24;i++) {
		tauxcourant=pdt[i].taux;
		qcourantetaux=pdt[i].fortnightno;
		if ( qcourantetaux >= qcouranteop ) {
			if (jour)
				cumulinteretan[k]=cumulinteretan[k]+montant[k]*tauxcourant*pdt[i].nbjour/(nbjourantot*100);
			else
				cumulinteretan[k]=cumulinteretan[k]+montant[k]*tauxcourant/(24*100);
			nbquinzan[k]=nbquinzan[k]+1;
			nbjouran[k]=nbjouran[k]+pdt[i].nbjour;
			if (qcourantetaux <= qhui ) {
				if (jour) {
					cumulinterethui[k]=cumulinterethui[k]+montant[k]*tauxcourant*pdt[i].nbjour/(nbjourantot*100);
					}
				else {
					cumulinterethui[k]=cumulinterethui[k]+montant[k]*tauxcourant/(24*100);
					}
				nbquinzhui[k]=nbquinzhui[k]+1;
				nbjourhui[k]=nbjourhui[k]+pdt[i].nbjour;
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
printf("                        Nombre de  Nombre de               Nombre de  Nombre de   Interêts\n");
printf("      Date    Montant  quinzaines      jours    Intérêts  quinzaines   jours de         de\n");
printf(" opération  opération     courues     courus      courus  de l'année    l'année    l'année\n");
for (i=0;i<nop;i++) {
	montanttotalhui=montanttotalhui+cumulinterethui[i];
	montanttotalan=montanttotalan+cumulinteretan[i];
	printf("%s %#10.2f        %4d       %4d  %#10.2f        %4d       %4d %#10.2f\n",dateop[i],montant[i],nbquinzhui[i],nbjourhui[i],cumulinterethui[i],nbquinzan[i],nbjouran[i],cumulinteretan[i]);
}

printf("Montant total intérêts courus: %#10.2f\n",montanttotalhui);
printf("Montant total intérêts année : %#10.2f\n",montanttotalan);

exit(EXIT_SUCCESS);
}

