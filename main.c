#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Tutti i dati sono espressi secondo le unità di misura del sistema internazionale

#define EARTH_MASS (5.97219*(powf(10,24))) //kg
#define EARTH_RADIUS (6.371005076123*(powf(10,6))) //m
#define GRAVITATIONAL_CONSTANT (6.6743*(powf(10,-11))) //m^3 kg^-1 s^2
#define M_PI 3.14159265358979323846

typedef struct orbits orbit;

orbit *newOrbit(orbit *orbitData);

double computeOrbitSpeed(double radius, double majorSemiaxis);

void printOrbitData(orbit *orbitData, int orbitIndex);

double orbitChanger(orbit *orbitData, int oldOrbitIndex, int newOrbitIndex);

int flag; //Variabile sentinella globale utilizzato nella stampa delle informazioni relative al cambio di orbita

int main() {
    orbit *orbitData = NULL;
    int orbitIndex = 0;
    double speedDifference;
    printf("\norbitCalculator\nVersion 1.0\nSimone Giaccaria - MAAT04\n");

    printf("\nInserisci le condizioni iniziali dell'orbita:");
    orbitData = newOrbit(orbitData);
    printOrbitData(orbitData, orbitIndex);

    printf("\n\nSi vuole cambiare orbita? 1 - Si\n0 - No\n");
    scanf("%d", &flag);

    while (flag == 1){
        printf("\n\nInserire i dati della nuova orbita:");
        orbitData = newOrbit(orbitData);
        printOrbitData(orbitData, orbitIndex + 1);
        speedDifference = orbitChanger(orbitData, orbitIndex, orbitIndex + 1);
        orbitIndex++;
        if (flag == 0){
            if(speedDifference < 0)
                printf("\nIl satellite deve ridurre la velocita' di %lf m/s all'apogeo", sqrt(pow(speedDifference,2)));
            else
                printf("\nIl satellite deve aumentare la velocita' di %lf m/s all'apogeo", speedDifference);
        }
        else{
            if(speedDifference < 0)
                printf("\nIl satellite deve ridurre la velocita' di %lf m/s al perigeo", sqrt(pow(speedDifference,2)));
            else
                printf("\nIl satellite deve aumentare la velocita' di %lf m/s al perigeo", speedDifference);
        }
        printf("\n\nSi vuole cambiare orbita? 1 - Si\n0 - No\n");
        scanf("%d", &flag);
    }
    free(orbitData);
    return 0;
}

//Inizializzazione della struttura dati per la memorizzazione dell'orbita

typedef struct orbits{
    char orbitType[10];
    double apogeeRadius;
    double perigeeRadius;
    double apogeeSpeed;
    double perigeeSpeed;
    double majorSemiaxis;
    double period;
} orbit;

//Funzione che calcola la velocità dell'orbita dato il raggio e il semiasse maggiore dell'orbita

double computeOrbitSpeed(double radius, double majorSemiaxis){
    double speed;
    speed = sqrt(GRAVITATIONAL_CONSTANT * EARTH_MASS * (2 / radius - 1 / majorSemiaxis)); //m s^-1
    return speed;
}

//Funzione per la memorizzazione di una nuova orbita

orbit *newOrbit(orbit *orbitData) {
    int dim = 0, ans;
    double apogeeAltitude, perigeeAltitude, altitude;
    printf("\nTipo di orbita:\n1 - Circolare\n2 - Ellittica\n");
    scanf("%d", &ans);
    if(ans == 1){
        printf("\nQuota di orbita [km]: ");
        scanf("%lf", &altitude);
        altitude *= 1000;
    }
    else if(ans == 2){
        printf("\nQuota di orbita all'apogeo [km]: ");
        scanf("%lf", &apogeeAltitude);
        apogeeAltitude *= 1000;
        printf("\nQuota di orbita al perigeo [km]: ");
        scanf("%lf", &perigeeAltitude);
        perigeeAltitude *= 1000;
    }
    //Ciclo per il conteggio di orbite presenti nella struttura dati
    if (orbitData != NULL) {
        while (orbitData[dim].orbitType[0] != '\0') {
            dim++;
        }
    }
    //Riallocamento dinamico della memoria per il set di orbite aggiornato
    orbitData = realloc(orbitData, (dim + 21) * sizeof(orbit));
    if (orbitData == NULL) {
        printf("Errore di allocazione memoria\n");
        exit(1);
    }
    if(ans == 1){
        strcpy(orbitData[dim].orbitType, "Circolare");
        orbitData[dim].apogeeRadius = orbitData[dim].perigeeRadius = EARTH_RADIUS + altitude;
        orbitData[dim].majorSemiaxis = orbitData[dim].apogeeRadius;
        orbitData[dim].apogeeSpeed = orbitData[dim].perigeeSpeed = computeOrbitSpeed(orbitData[dim].apogeeRadius, orbitData[dim].majorSemiaxis);
        orbitData[dim].period = 2 * M_PI * orbitData[dim].apogeeRadius / orbitData[dim].apogeeSpeed / 60;
    }
    else{
        strcpy(orbitData[dim].orbitType, "Ellittica");
        orbitData[dim].apogeeRadius = EARTH_RADIUS + apogeeAltitude;
        orbitData[dim].perigeeRadius = EARTH_RADIUS + perigeeAltitude;
        orbitData[dim].majorSemiaxis = (orbitData[dim].apogeeRadius + orbitData[dim].perigeeRadius) / 2;
        orbitData[dim].apogeeSpeed = computeOrbitSpeed(orbitData[dim].apogeeRadius, orbitData[dim].majorSemiaxis);
        orbitData[dim].perigeeSpeed = computeOrbitSpeed(orbitData[dim].perigeeRadius, orbitData[dim].majorSemiaxis);
    }
    //Creazione del valore sentinella per il conteggio di orbite presenti
    orbitData[dim + 1].orbitType[0] = '\0';
    return orbitData;
}

//Funzione per stampare a schermo un'orbita memorizzata dato il set di orbite e l'indice dell'orbita interessata

void printOrbitData(orbit *orbitData, int orbitIndex){
    printf("\nOrbita %s", orbitData[orbitIndex].orbitType);
    if(strncmp(orbitData[orbitIndex].orbitType, "Circolare", strlen("Circolare")) == 0)
        printf("\nVelocita' dell'orbita = %lf m/s\nPeriodo = %lf min", orbitData[orbitIndex].apogeeSpeed, orbitData[orbitIndex].period);
    else
        printf("\nVelocita' all'apogeo = %lf m/s\nVelocita' al perigeo = %lf m/s", orbitData[orbitIndex].apogeeSpeed, orbitData[orbitIndex].perigeeSpeed);
}

//Funzione per il calcolo della variazione di velocità per il cambio di orbita dato il set di orbite, l'indice dell'orbita iniziale e dell'orbita finale

double orbitChanger(orbit *orbitData, int oldOrbitIndex, int newOrbitIndex){
    double speedDifference;
    if (strncmp(orbitData[oldOrbitIndex].orbitType, "Circolare", strlen("Circolare")) == 0 && strncmp(orbitData[newOrbitIndex].orbitType, "Ellittica", strlen("Ellittica")) == 0){
        if(orbitData[oldOrbitIndex].majorSemiaxis < orbitData[newOrbitIndex].majorSemiaxis){
            speedDifference = orbitData[newOrbitIndex].perigeeSpeed - orbitData[oldOrbitIndex].perigeeSpeed;
            flag = 1;
        }
        else{
            speedDifference = orbitData[newOrbitIndex].apogeeSpeed - orbitData[oldOrbitIndex].apogeeSpeed;
            flag = 0;
        }
    }
    else if (strncmp(orbitData[oldOrbitIndex].orbitType, "Ellittica", strlen("Ellittica")) == 0 && strncmp(orbitData[newOrbitIndex].orbitType, "Circolare", strlen("Circolare")) == 0) {
        if (orbitData[oldOrbitIndex].majorSemiaxis < orbitData[newOrbitIndex].majorSemiaxis){
            speedDifference = orbitData[newOrbitIndex].apogeeSpeed - orbitData[oldOrbitIndex].apogeeSpeed;
            flag = 0;
        }
        else{
            speedDifference = orbitData[newOrbitIndex].perigeeSpeed - orbitData[oldOrbitIndex].perigeeSpeed;
            flag = 1;
        }
    }
    else{
        speedDifference = orbitData[newOrbitIndex].apogeeSpeed - orbitData[oldOrbitIndex].apogeeSpeed;
    }
    return speedDifference;
}