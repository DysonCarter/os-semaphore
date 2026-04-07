#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

// Semaphores
sem_t agentSem;
sem_t tobacco, paper, match;
sem_t tobaccoSem, paperSem, matchSem;
sem_t mutex;

bool isTobacco = false, isPaper = false, isMatch = false;

// Agent Threads
void* agentA(void* arg) { //Tobacco Paper
    for (int i = 0; i < 6; ++i) {
        usleep((rand() % 200) * 1000); // Sleep up to 200ms
        sem_wait(&agentSem);
        printf("Agent A Puts tobacco and paper on the table\n");
        sem_post(&tobacco);
        sem_post(&paper);
    }
    printf("Agent A finished\n");
    return NULL;
}

void* agentB(void* arg) { //Paper Match
    for (int i = 0; i < 6; ++i) {
        usleep((rand() % 200) * 1000);
        sem_wait(&agentSem);
        printf("Agent B Puts paper and a match on the table\n");
        sem_post(&paper);
        sem_post(&match);
    }
    printf("Agent B finished\n");
    return NULL;
}

void* agentC(void* arg) { // Match Tobacco
    for (int i = 0; i < 6; ++i) {
        usleep((rand() % 200) * 1000);
        sem_wait(&agentSem);
        printf("Agent C Puts a match and tobacco on the table\n");
        sem_post(&match);
        sem_post(&tobacco);
    }
    printf("Agent C finished\n");
    return NULL;
}

//Pusher Threads

void* pusherTobacco(void* arg) {
    for (int i = 0; i < 12; ++i) {
        sem_wait(&tobacco);
        sem_wait(&mutex);

        if (isPaper) {
            isPaper = false;
            sem_post(&matchSem);
        } else if (isMatch) {
            isMatch = false;
            sem_post(&paperSem);
        } else {
            isTobacco = true;
        }

        sem_post(&mutex);
    }
    return NULL;
}

void* pusherPaper(void* arg) {
    for (int i = 0; i < 12; ++i) {
        sem_wait(&paper);
        sem_wait(&mutex);

        if (isTobacco) {
            isTobacco = false;
            sem_post(&matchSem);
        } else if (isMatch) {
            isMatch = false;
            sem_post(&tobaccoSem);
        } else {
            isPaper = true;
        }

        sem_post(&mutex);
    }
    return NULL;
}

void* pusherMatch(void* arg) {
    for (int i = 0; i < 12; ++i) {
        sem_wait(&match);
        sem_wait(&mutex);

        if (isTobacco) {
            isTobacco = false;
            sem_post(&paperSem);
        } else if (isPaper) {
            isPaper = false;
            sem_post(&tobaccoSem);
        } else {
            isMatch = true;
        }

        sem_post(&mutex);
    }
    return NULL;
}

// Smoker Threads

void* smokerTobacco(void* arg) {
    int id = *((int*)arg);
    for (int i = 0; i < 3; ++i) {
        sem_wait(&tobaccoSem);

        printf("Smoker %d has Tobacco Making cigarette #%d...\n", id, i + 1);
        usleep((rand() % 50) * 1000); // Make cigarette (up to 50ms)

        sem_post(&agentSem); // Ingredients picked up, signal agent

        printf("Smoker %d has Tobacco Smoking cigarette #%d...\n", id, i + 1);
        usleep((rand() % 50) * 1000); // Smoke cigarette (up to 50ms)
    }
    printf("Smoker %d has Tobacco Done smoking\n", id);
    return NULL;
}

void* smokerPaper(void* arg) {
    int id = *((int*)arg);
    for (int i = 0; i < 3; ++i) {
        sem_wait(&paperSem);

        printf("Smoker %d has Paper Making cigarette #%d...\n", id, i + 1);
        usleep((rand() % 50) * 1000);

        sem_post(&agentSem);

        printf("Smoker %d has Paper Smoking cigarette #%d...\n", id, i + 1);
        usleep((rand() % 50) * 1000);
    }
    printf("Smoker %d has Paper Done smoking\n", id);
    return NULL;
}

void* smokerMatch(void* arg) {
    int id = *((int*)arg);
    for (int i = 0; i < 3; ++i) {
        sem_wait(&matchSem);

        printf("Smoker %d has Match Making cigarette #%d...\n", id, i + 1);
        usleep((rand() % 50) * 1000);

        sem_post(&agentSem);

        printf("Smoker %d has Match Smoking cigarette #%d...\n", id, i + 1);
        usleep((rand() % 50) * 1000);
    }
    printf("Smoker %d has Match Done smoking, exiting\n", id);
    return NULL;
}

// --- Main Execution ---

int main() {
    srand(time(NULL)); // Seed random number generator

    // Initialize Semaphores
    sem_init(&agentSem, 0, 1);
    sem_init(&tobacco, 0, 0);
    sem_init(&paper, 0, 0);
    sem_init(&match, 0, 0);

    sem_init(&tobaccoSem, 0, 0);
    sem_init(&paperSem, 0, 0);
    sem_init(&matchSem, 0, 0);
    sem_init(&mutex, 0, 1);

    // Create Thread Variables
    pthread_t agents[3];
    pthread_t pushers[3];
    pthread_t smokers[6]; // Two of each type
    int smokerIds[6] = {1, 2, 3, 4, 5, 6};

    // Spawn Agents
    pthread_create(&agents[0], NULL, agentA, NULL);
    pthread_create(&agents[1], NULL, agentB, NULL);
    pthread_create(&agents[2], NULL, agentC, NULL);

    // Spawn Pushers
    pthread_create(&pushers[0], NULL, pusherTobacco, NULL);
    pthread_create(&pushers[1], NULL, pusherPaper, NULL);
    pthread_create(&pushers[2], NULL, pusherMatch, NULL);

    // Spawn Smokers
    pthread_create(&smokers[0], NULL, smokerTobacco, &smokerIds[0]);
    pthread_create(&smokers[1], NULL, smokerTobacco, &smokerIds[1]);

    pthread_create(&smokers[2], NULL, smokerPaper, &smokerIds[2]);
    pthread_create(&smokers[3], NULL, smokerPaper, &smokerIds[3]);

    pthread_create(&smokers[4], NULL, smokerMatch, &smokerIds[4]);
    pthread_create(&smokers[5], NULL, smokerMatch, &smokerIds[5]);

    // Join All Threads
    for (int i = 0; i < 3; ++i) pthread_join(agents[i], NULL);
    for (int i = 0; i < 3; ++i) pthread_join(pushers[i], NULL);
    for (int i = 0; i < 6; ++i) pthread_join(smokers[i], NULL);

    // Clean up semaphores
    sem_destroy(&agentSem);
    sem_destroy(&tobacco);
    sem_destroy(&paper);
    sem_destroy(&match);
    sem_destroy(&tobaccoSem);
    sem_destroy(&paperSem);
    sem_destroy(&matchSem);
    sem_destroy(&mutex);

    return 0;
}
