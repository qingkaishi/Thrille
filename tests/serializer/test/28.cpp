//test cond, barrer, mutex, sem reinitialization

#include <pthread.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <semaphore.h>

using namespace std;

int x;
pthread_t child, child2;
pthread_mutex_t mutex, mutex2;
pthread_cond_t cond, cond2;
sem_t sem;
pthread_barrier_t bar;

void setup() {
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&mutex2, NULL);
    pthread_cond_init(&cond, NULL);
    pthread_cond_init(&cond2, NULL);
}

void teardown() {
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mutex2);
    pthread_cond_destroy(&cond);
    pthread_cond_destroy(&cond2);
}


int main(int argc, char *argv[]) {
    setup();
    
    pthread_barrier_init(&bar, NULL, 1);
    pthread_barrier_wait(&bar);
    pthread_barrier_destroy(&bar);
    pthread_barrier_init(&bar, NULL, 1);
    pthread_barrier_wait(&bar);
    
    sem_init(&sem, 0, 1);
    sem_wait(&sem);
    sem_destroy(&sem);
    sem_init(&sem, 0, 1);
    sem_wait(&sem);
    
    pthread_mutex_lock(&mutex);
    pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy(&mutex);
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_lock(&mutex);
    pthread_mutex_unlock(&mutex);
    
    pthread_cond_signal(&cond);
    pthread_cond_destroy(&cond);
    pthread_cond_init(&cond, NULL);
    pthread_cond_signal(&cond);
    
    teardown();
}

