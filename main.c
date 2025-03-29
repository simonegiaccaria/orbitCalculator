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

typedef struct orbits{
    char orbitType[10];
    double apogeeRadius;
    double perigeeRadius;
    double apogeeSpeed;
    double perigeeSpeed;
    double majorSemiaxis;
    double period;
} orbit;

typedef struct thrusters{
    char thrusterName[100];
    double size;
    char mount[100];
    char propellant[100];
    char thrusterType[100];
    char thrustersNumber[100];
    double thrust;
    double impulse;
    double mass;
} thruster;

orbit *newOrbit(orbit *orbitData);

double computeOrbitSpeed(double radius, double majorSemiaxis);

void printOrbitData(orbit *orbitData, int orbitIndex);

double orbitChanger(orbit *orbitData, int oldOrbitIndex, int newOrbitIndex);

double **satellite(int satellitePieces);

double **payloads(int payloadsNumber);

double *computeSatelliteCog(double **satelliteCoordsMass, int satellitePieces, double **payloadsCoordsMass, int payloadsNumber);

double *normalizeVector(double *vector);

void printThrustersData(thruster *thrusterData, int nThrusters);

thruster *allocateThrusters(thruster *thrusterData, int nThrusters, FILE *fp);

orbit *orbitPop(orbit *orbitData, int removeIndex);

orbit *hohmannManouver(orbit *orbitData, int orbitIndex);

int flag; //Variabile sentinella globale utilizzato nella stampa delle informazioni relative al cambio di orbita

int main() {
    FILE *fp;
    orbit *orbitData = NULL;
    thruster *thrusterData = NULL;
    int orbitIndex = 0, satellitePieces, payloadsNumber, ans;
    double speedDifference, totalSpeedDifference = 0;
    double **satelliteCoordsMass, **payloadsCoordsMass, thrusters;
    double *cog;

    printf("\norbitCalculator\nVersion 1.0\nSimone Giaccaria - MAAT04\n");

    printf("\nInserisci le condizioni iniziali dell'orbita:");
    orbitData = newOrbit(orbitData);
    printOrbitData(orbitData, orbitIndex);

    printf("\n\nSi vuole cambiare orbita? 1 - Si\n0 - No\n");
    scanf("%d", &ans);
    if(ans == 1){
        printf("\n\nInserire i dati della nuova orbita:");
        orbitData = newOrbit(orbitData);
        
        printOrbitData(orbitData, orbitIndex + 1);
        
        if (strncmp(orbitData[orbitIndex].orbitType, "Circolare", strlen("Circolare")) == 0 && strncmp(orbitData[orbitIndex + 1].orbitType, "Circolare", strlen("Circolare")) == 0) {
            if(orbitData[orbitIndex+1].apogeeRadius / orbitData[orbitIndex].apogeeRadius <= 11.94) {
                hohmannManouver(orbitData, orbitIndex);
                speedDifference = orbitChanger(orbitData, orbitIndex, orbitIndex + 2);
                totalSpeedDifference += speedDifference;
                speedDifference = orbitChanger(orbitData, orbitIndex + 2, orbitIndex + 1);
                totalSpeedDifference += speedDifference;
                printf("\n\nPer il cambio di orbita, conviene la manovra di Hohmann, con un incremento di velocità al perigeo di %lf m/s e all'apogeo di %lf m/s", totalSpeedDifference - speedDifference, speedDifference);
            }
        }
    }

    printf("\n\nInserire i dati del satellite:");
    printf("\nInserire il numero di pezzi del satellite:");
    scanf("%d", &satellitePieces);
    satelliteCoordsMass = satellite(satellitePieces);
    printf("\nInserire i dati dei payload:");
    printf("\nInserire il numero di payload:");
    scanf("%d", &payloadsNumber);
    payloadsCoordsMass = payloads(payloadsNumber);

    fp = fopen("thrusters.csv", "r");
    if (fp == NULL) {
        printf("Errore nell'apertura del file\n");
        exit(1);
    }
    int nThrusters = 0;
    for(int c = getc(fp); c != EOF; c = getc(fp)){
        if(c == '\n')
            nThrusters++;
    }
    rewind(fp);
    thrusterData = allocateThrusters(thrusterData, nThrusters, fp);
    fclose(fp);
    cog = computeSatelliteCog(satelliteCoordsMass, satellitePieces, payloadsCoordsMass, payloadsNumber);
    int j;
    if (speedDifference != 0) {
        printThrustersData(thrusterData, nThrusters);
        for(int i = 0; i < nThrusters; i++){
            if (i == 0)
                j = i;
            else{
                if(abs(thrusterData[i].impulse - (((cog[3]+thrusterData[i].mass)/1000) * totalSpeedDifference)) < abs(thrusterData[j].impulse - (((cog[3]+thrusterData[j].mass)/1000) * totalSpeedDifference))){
                    j = i;
                }
            }   
        }
        printf("\nIl satellite necessita di un sistema di propulsione. E' consiglibile l'utilizzo del %s", thrusterData[j].thrusterName);
    }
    cog[3] += thrusterData[j].mass;
    printf("\nIl centro di gravita' del satellite si trova in (%lf cm, %lf cm, %lf cm) e ha una massa di %lf kg", cog[0], cog[1], cog[2], cog[3]/1000);
}

//Funzione che rimuove un'orbita indesiderata

orbit *orbitPop(orbit *orbitData, int removeIndex){
	int dim = 0, i;
	if (orbitData != NULL) {
        while (orbitData[dim].orbitType[0] != '\0') {
            dim++;
        }
    }
	for (i = removeIndex; i < dim - 1; i++) {
        orbitData[i] = orbitData[i + 1];
    }
    memset(&orbitData[dim - 1], 0, sizeof(orbit));
    return orbitData;
}

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
    orbitData = realloc(orbitData, (dim + 2) * sizeof(orbit));
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

//Funzione per la manovra di Hohmann

orbit *hohmannManouver(orbit *orbitData, int orbitIndex){
    int dim = 0;
    if (orbitData != NULL) {
        while (orbitData[dim].orbitType[0] != '\0') {
            dim++;
        }
    }
    //Riallocamento dinamico della memoria per il set di orbite aggiornato
    orbitData = realloc(orbitData, (dim + 2) * sizeof(orbit));
    if (orbitData == NULL) {
        printf("Errore di allocazione memoria\n");
        exit(1);
    }
    //Ciclo per il conteggio di orbite presenti nella struttura dati
    strcpy(orbitData[dim].orbitType, "Ellittica");
    orbitData[dim].apogeeRadius = orbitData[orbitIndex + 1].apogeeRadius;
    orbitData[dim].perigeeRadius = orbitData[orbitIndex].perigeeRadius;
    orbitData[dim].majorSemiaxis = (orbitData[dim].apogeeRadius + orbitData[dim].perigeeRadius) / 2;
    orbitData[dim].apogeeSpeed = computeOrbitSpeed(orbitData[dim].apogeeRadius, orbitData[dim].majorSemiaxis);
    orbitData[dim].perigeeSpeed = computeOrbitSpeed(orbitData[dim].perigeeRadius, orbitData[dim].majorSemiaxis);
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

//Funzione per il calcolo del cog del satellite

double *computeSatelliteCog(double **satelliteCoordsMass, int satellitePieces, double **payloadsCoordsMass, int payloadsNumber){
    double x = 0, y = 0, z = 0, satelliteTotalMass = 0;
    double *cog;
    cog = malloc(4 * sizeof(double));

    for(int i = 0; i < payloadsNumber; i++){
        x += payloadsCoordsMass[i][0] * payloadsCoordsMass[i][3];
        y += payloadsCoordsMass[i][1] * payloadsCoordsMass[i][3];
        z += payloadsCoordsMass[i][2] * payloadsCoordsMass[i][3];
        satelliteTotalMass += payloadsCoordsMass[i][3];
    }
    for(int i = 0; i < satellitePieces; i++){
        x += satelliteCoordsMass[i][0] * satelliteCoordsMass[i][3];
        y += satelliteCoordsMass[i][1] * satelliteCoordsMass[i][3];
        z += satelliteCoordsMass[i][2] * satelliteCoordsMass[i][3];
        satelliteTotalMass += satelliteCoordsMass[i][3];
    }
    x /= satelliteTotalMass;
    y /= satelliteTotalMass;
    z /= satelliteTotalMass;
    cog[0] = x;
    cog[1] = y;
    cog[2] = z;
    cog[3] = satelliteTotalMass;
    return cog;
}

//Funzione per l'inserimento delle coordinate e della massa dei pezzi del satellite

double **satellite(int satellitePieces){
    double **satelliteCoordsMass;
    satelliteCoordsMass = malloc(satellitePieces * sizeof(double *));
    if(satelliteCoordsMass == NULL){
        printf("Errore di allocazione memoria\n");
        exit(1);
    }
    for(int i = 0; i < satellitePieces; i++){
        satelliteCoordsMass[i] = malloc(4 * sizeof(double));
        if(satelliteCoordsMass[i] == NULL){
            printf("Errore di allocazione memoria\n");
            exit(1);
        }
    }
    printf("\nInserire le coordinate e la massa dei pezzi del satellite(x[cm] y[cm] z[cm] m[g]):\n");
    for(int i = 0; i < satellitePieces; i++){
        scanf("%lf %lf %lf %lf", &satelliteCoordsMass[i][0], &satelliteCoordsMass[i][1], &satelliteCoordsMass[i][2], &satelliteCoordsMass[i][3]);
    }
    return satelliteCoordsMass;
}

//Funzione per l'inserimento delle coordinate e della massa dei payload

double **payloads(int payloadsNumber){
    double **payloadsCoordsMass;
    payloadsCoordsMass = malloc(payloadsNumber * sizeof(double *));
    if(payloadsCoordsMass == NULL){
        printf("Errore di allocazione memoria\n");
        exit(1);
    }
    for(int i = 0; i < payloadsNumber; i++){
        payloadsCoordsMass[i] = malloc(4 * sizeof(double));
        if(payloadsCoordsMass[i] == NULL){
            printf("Errore di allocazione memoria\n");
            exit(1);
        }
    }
    printf("\nInserire le coordinate e la massa dei payload(x[cm] y[cm] z[cm] m[g]):\n");
    for(int i = 0; i < payloadsNumber; i++){
        scanf("%lf %lf %lf %lf", &payloadsCoordsMass[i][0], &payloadsCoordsMass[i][1], &payloadsCoordsMass[i][2], &payloadsCoordsMass[i][3]);
    }
    return payloadsCoordsMass;
}

//Funzione per la normalizzazione di un vettore

double *normalizeVector(double *vector){
    double norm = sqrt(pow(vector[0], 2) + pow(vector[1], 2) + pow(vector[2], 2));
    vector[0] /= norm;
    vector[1] /= norm;
    vector[2] /= norm;
    return vector;
}

thruster *allocateThrusters(thruster *thrusterData, int nThrusters, FILE *fp){
    thrusterData = malloc((nThrusters + 1) * sizeof(thruster));
    char row[100];
    char *token;
    int i = 0;
    while (!feof(fp) && 0 < nThrusters)
    {
        fgets(row, 100, fp);

        token = strtok(row, ",");
        strcpy(thrusterData[i].thrusterName, token);

        token = strtok(NULL, ",");
        thrusterData[i].size = atof(token);

        token = strtok(NULL, ",");
        strcpy(thrusterData[i].mount, token);

        token = strtok(NULL, ",");
        strcpy(thrusterData[i].propellant, token);

        token = strtok(NULL, ",");
        strcpy(thrusterData[i].thrusterType, token);

        token = strtok(NULL, ",");
        strcpy(thrusterData[i].thrustersNumber, token);

        token = strtok(NULL, ",");
        thrusterData[i].thrust = atof(token) *1000;

        token = strtok(NULL, ",");
        thrusterData[i].impulse = atof(token);

        token = strtok(NULL, ",");
        thrusterData[i].mass = atof(token);

        i++;
    }
    return thrusterData;
}

void printThrustersData(thruster *thrusterData, int nThrusters){
    printf("\nNome del propulsore | Dimensione | Posizione di montaggio | Propellente | Tipo di propulsore | Numero di propulsori | Spinta | Impulso specifico | Massa\n");
    for(int i = 0; i < nThrusters; i++){
        printf("%s | %lf | %s | %s | %s | %s | %lf | %lf | %lf\n", thrusterData[i].thrusterName, thrusterData[i].size, thrusterData[i].mount, thrusterData[i].propellant, thrusterData[i].thrusterType, thrusterData[i].thrustersNumber, thrusterData[i].thrust, thrusterData[i].impulse, thrusterData[i].mass);
    }
}