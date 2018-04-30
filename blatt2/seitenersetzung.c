/* Seitenersetzungsstrategien
 *
 * Dieses Programm dient als Rahmen zum Experimentieren mit
 * Seitenersetzungsstrategien. 
 *
 * Bitte ergaenzen Sie die Funktion "seitenzugriff()" um die
 * noetige Funktionalitaet zur Zuweisung eines Seitenrahmens
 * an die Seite, auf die zugegriffen werden soll.
 * 
 * Der Übersicht halber werden die Seiten mit "Namen"
 * aus einem Zeichen (char) bezeichnet, nicht mit numerischen Adressen.
 *
 * Falls Sie die "Aging"-Strategie realisieren moechten, steht
 * Ihnen die Funktion "zaehler_aktualisieren()" als Basis zur
 * Verfuegung, in der Sie die Aktualisierung der Zaehlerstaende
 * aus den uebergebenen "Referenziert"-Bits ref_bits[] ergaenzen
 * koennen.
 * 
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>

#define PAGEFRAMES 5

typedef struct pf {
    char ref_bit;  /* "Referenced"-Flag */
    char seite;    /* Name ("Adresse") der in diesem Rahmen gelagerten Seite (oder Null) */
    unsigned int zaehler;   /* Zaehlervariable fuer Aging-Verfahren */
} PageFrame;

/** Hilfsfunktion: Bitweise Ausgabe von 'c' **/
void printbits(unsigned int c) {
  int i;
  for (i=7; i>=0; i--) {
    printf("%d", (c & 1<<i) != 0);
  }
}

/** Hilfsfunktion: Seitenrahmen und Zaehlerstaende ausgeben **/
void dumpframes(PageFrame rahmen[]) {
  int j;
  printf("*** Seitenrahmen- und Zaehlerstaende ***\n");
  for (j=0; j < PAGEFRAMES; j++) {
    printf("* Rahmen %d: Inhalt='%c', ref=%d, Zaehler=", j,
	    rahmen[j].seite, rahmen[j].ref_bit);
    printbits(rahmen[j].zaehler);
    printf("\n");
  }
}

/****************************************************************************/

void zaehler_aktualisieren(PageFrame rahmen[]) {
    int i;
    int zahlerMod = 1<<7;
    
    for(i = 0;i <PAGEFRAMES;i++){
        if(!rahmen[i].ref_bit){
            rahmen[i].zaehler >>= 1; 
            rahmen[i].ref_bit = 0;
        }else{
            rahmen[i].zaehler >>= 1;
            rahmen[i].zaehler = rahmen[i].zaehler|zahlerMod;
            rahmen[i].ref_bit = 0;
        }
    }
}


void seitenzugriff(char seite, PageFrame rahmen[]) {
    int i,oldesti;
    unsigned int oldest = 257;
    char page = 0;
    int pi = 0;
    int r = PAGEFRAMES+1;
    printf("\nZugriff auf Seite %c\n",seite);
    for(i = 0;i<PAGEFRAMES;i++){
        if(!rahmen[i].seite){
            r = i;
            continue;
        }else if(rahmen[i].zaehler<oldest){
            oldest = rahmen[i].zaehler;
            oldesti = i;
        }
        if(rahmen[i].seite == seite){
            page = seite;
            pi = i;
        }
    }
    
    if(!page){
        printf("SEITENFEHLER fuer Seite %c\n",seite);
    }else{
        printf("Seite ist bereits in Rahmen %d\n",pi);
        rahmen[pi].ref_bit = 1;
        return;
    }
    
    if(r <= PAGEFRAMES){
        printf("Rahmen %d ist noch frei, wird mit %c belegt\n",r,seite);
        rahmen[r].seite = seite;
        rahmen[r].ref_bit = 1;
        rahmen[r].zaehler = 0;
    }else{
        printf("Verdreange Seite %c aus Rahmen %d\n",rahmen[oldesti].seite,oldesti);
        printf("Belege Rahmen %d mit %c\n",oldesti,seite);
        rahmen[oldesti].seite = seite;
        rahmen[oldesti].zaehler = 0;
    }
}

/*
 * H a u p t f u n k t i o n
 */
int main(int argc, char **argv) {
  int i, n;

  PageFrame rahmen[PAGEFRAMES] = {{0}};
  char seite;

  /* Genau einen Parameter entgegennehmen, sonst Abbruch */
  assert(argc == 2);

  n = strlen(argv[1]);
  for (i=0; i<n; i++) {
    /* Kommandozeilenparameter zeichenweise ('seite') abarbeiten */
    seite = argv[1][i];
    assert((seite >= 'A') && (seite<= 'Z'));

    /* Seitenzugriff fuer dieses Seite simulieren */
    seitenzugriff(seite, rahmen);

    /* Aging-Verfahren: Zaehler aktualisieren */
    zaehler_aktualisieren(rahmen);

    dumpframes(rahmen);

  }
  printf("\n*** ENDE ***\n");
  return 0;
}
