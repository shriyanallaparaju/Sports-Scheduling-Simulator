/* CS 3013 - C25
Project 2
Author: Shriya Nallaparaju */

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>
#include <stdbool.h>

#define TOTAL_BASEBALL 36
#define TOTAL_FOOTBALL 44
#define TOTAL_RUGBY 60

#define MIN_RUGBY 2
#define MAX_RUGBY 30

#define TOTAL_INCREMENTS 2

#define BUFFER 10

int global_sleep;

// int game_num = 0;
// pthread_mutex_t game_lock;

typedef enum { NONE, BASEBALL, FOOTBALL, RUGBY } Sport;

typedef struct {
    Sport sport;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    int count;
    int needed;
    int ready;
} TeamFormation;

TeamFormation baseball_team = {BASEBALL, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, 0, 18};
TeamFormation football_team = {FOOTBALL, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, 0, 22};
TeamFormation rugby_team = {RUGBY, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, 1, 2, 0};

typedef struct {
    int sports_played_count;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    Sport current_sport;
    Sport next_sport;
    Sport last_sport;
    int players_on_field;
    int consec_games;
    //sem_t consec_game;
} FieldState;

FieldState field = {0, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, NONE, BASEBALL, NONE, 0, 0};

bool isEven(int num) {
    if (num % 2 == 0) {
        return true;
    }

    return false;
}

void field_entry_attempt(Sport sport) {
    pthread_mutex_lock(&field.lock);
    
    // field.current_sport != NONE && field.current_sport != sport
    int max;

    if (sport == BASEBALL) {
        max = TOTAL_BASEBALL;
    } else if (sport == FOOTBALL) {
        max = TOTAL_FOOTBALL;
    } else if (sport == RUGBY) {
        max = TOTAL_RUGBY;
    }
    
    while ((field.current_sport != NONE && field.current_sport != sport) || (field.current_sport == sport && field.players_on_field == max)) {
        pthread_cond_wait(&field.cond, &field.lock);
    }
    
    if (field.current_sport == NONE) {
    
        field.current_sport = sport;
        pthread_cond_broadcast(&field.cond);

        printf("New %s game starting!\n", 
            sport == BASEBALL ? "baseball" : 
            sport == FOOTBALL ? "football" : "rugby");

        global_sleep = rand() % 3 + 2;
        printf("The game is %d seconds long\n", global_sleep);

        //field.current_sport = NONE;
    }

    field.players_on_field++;
    pthread_mutex_unlock(&field.lock);
}

void leave_field(Sport sport) {
    pthread_mutex_lock(&field.lock);
    field.players_on_field--;
    Sport curr_sport = field.current_sport;

    // printf("Sport: %s, How many left: %d\n", sport == BASEBALL ? "baseball" : 
    //         sport == FOOTBALL ? "football" : sport == RUGBY ? "rugby" : "none", field.players_on_field);
    
    if (field.players_on_field == 0) {
        printf("The %s game is now over. Everyone has gone inside!\n", 
            sport == BASEBALL ? "baseball" : 
            sport == FOOTBALL ? "football" : "rugby");

        if (field.current_sport == RUGBY) {
            pthread_mutex_lock(&rugby_team.lock);
            rugby_team.ready = 0;
            pthread_mutex_unlock(&rugby_team.lock);
        }
        
        field.current_sport = NONE;

        usleep(rand() % 50000);
        pthread_cond_broadcast(&field.cond);
    }

    pthread_mutex_unlock(&field.lock);

}

void* baseball_player(void* arg) {
    int id = (long)arg;
    //free(arg);

    for (int i = 0; i < TOTAL_INCREMENTS; i++) {
        usleep(rand() % 500000);
        pthread_mutex_lock(&baseball_team.lock);
        // printf("Baseball player %d has arrived! They are in the locker room. Count: %d, Owned by: %s\n", id, baseball_team.count, field.current_sport == BASEBALL ? "baseball" : 
        //     field.current_sport == FOOTBALL ? "football" : field.current_sport == RUGBY ? "rugby" : "none");
        Sport sport = field.current_sport;

        while (baseball_team.count >= (TOTAL_BASEBALL/2)) {
            pthread_cond_wait(&field.cond, &baseball_team.lock);
        }
        
        baseball_team.count++;
        
        if (baseball_team.count == (TOTAL_BASEBALL/2)) {
            pthread_cond_broadcast(&baseball_team.cond);
        }

        while (baseball_team.count < (TOTAL_BASEBALL/2)) {
            pthread_cond_wait(&baseball_team.cond, &baseball_team.lock);

        }

        field_entry_attempt(BASEBALL);
        pthread_mutex_unlock(&baseball_team.lock);
    
        printf("Baseball player %d is playing in the field!\n", id);

        sleep(global_sleep);
        
        printf("Baseball player %d has left the field.\n", id);
        leave_field(BASEBALL);

        pthread_mutex_lock(&baseball_team.lock);
        baseball_team.count--;

        pthread_mutex_unlock(&baseball_team.lock);

        //sem_post(&field.football_sem);
    }
    //printf("baseball finished its double header\n");
    
    pthread_mutex_lock(&field.lock);
    if (field.players_on_field == 0) {
        pthread_cond_broadcast(&field.cond);
    }
    pthread_mutex_unlock(&field.lock);
    
    return NULL;
}

void* football_player(void* arg) {
    int id = (long)arg;
    //free(arg);

    for (int i = 0; i < TOTAL_INCREMENTS; i++) {
        usleep(rand() % 500000);
        pthread_mutex_lock(&football_team.lock);
        // printf("Football player %d has arrived! They are in the locker room. Count: %d, Owned by: %s\n", id, football_team.count, field.current_sport == BASEBALL ? "baseball" : 
        //     field.current_sport == FOOTBALL ? "football" : field.current_sport == RUGBY ? "rugby" : "none");
        Sport sport = field.current_sport;

        while (football_team.count >= (TOTAL_FOOTBALL/2)) {
            pthread_cond_wait(&field.cond, &football_team.lock);
        }
        
        football_team.count++;
        
        if (football_team.count == (TOTAL_FOOTBALL/2)) {
            pthread_cond_broadcast(&football_team.cond);
        }

        while (football_team.count < (TOTAL_FOOTBALL/2)) {
            pthread_cond_wait(&football_team.cond, &football_team.lock);

        }

        field_entry_attempt(FOOTBALL);
        pthread_mutex_unlock(&football_team.lock);
    
        printf("Football player %d is playing in the field!\n", id);

        sleep(global_sleep);
        
        printf("Football player %d has left the field.\n", id);
        leave_field(FOOTBALL);

        pthread_mutex_lock(&football_team.lock);
        football_team.count--;

        pthread_mutex_unlock(&football_team.lock);

        //sem_post(&field.football_sem);
    }
    //printf("football finished its double header\n");
    pthread_mutex_lock(&field.lock);
    if (field.players_on_field == 0) {
        pthread_cond_broadcast(&field.cond);
    }
    pthread_mutex_unlock(&field.lock);
    
    return NULL;
}

void* rugby_player(void* arg) {
    int id = (long)arg;
    //free(arg);

    for (int i = 0; i < TOTAL_INCREMENTS; i++) {
        usleep(rand() % 500000);
        pthread_mutex_lock(&rugby_team.lock);
        // printf("Rugby player %d has arrived! They are in the locker room. Count: %d, Owned by: %s\n", id, rugby_team.count, field.current_sport == BASEBALL ? "baseball" : 
        //     field.current_sport == FOOTBALL ? "football" : field.current_sport == RUGBY ? "rugby" : "none");
        rugby_team.ready++;
        while (rugby_team.count >= 30 && !isEven(rugby_team.ready)) {
            pthread_cond_wait(&field.cond, &rugby_team.lock);
        }
        //rugby_team.ready++;
        rugby_team.count++;
            
        if ((rugby_team.count >= 2) && isEven(rugby_team.ready)) {
            pthread_cond_broadcast(&rugby_team.cond);
        }
             
        while (rugby_team.count > 30) {
            pthread_cond_wait(&rugby_team.cond, &rugby_team.lock);

        }

        field_entry_attempt(RUGBY);
        pthread_mutex_unlock(&rugby_team.lock);
    
        printf("Rugby player %d is playing in the field!\n", id);

        sleep(global_sleep);
        
        printf("Rugby player %d has left the field.\n", id);
        //printf("Rugby count left: %d\n", rugby_team.count);
        leave_field(RUGBY);

        pthread_mutex_lock(&rugby_team.lock);
        //rugby_team.ready --;
        rugby_team.count = 0;

        //pthread_cond_signal(&rugby_team.cond);
        pthread_mutex_unlock(&rugby_team.lock);

        //sem_post(&field.football_sem);
    }
    //printf("rugby finished its double header\n");
    pthread_mutex_lock(&field.lock);
    if (field.players_on_field == 0) {
        pthread_cond_broadcast(&field.cond);
    }
    pthread_mutex_unlock(&field.lock);
    
    return NULL;
}

int main() {
    FILE* seed_file = fopen("seed.txt", "r");
	char seed_string[5];

	fscanf(seed_file, "%s", seed_string);
	fclose(seed_file);

	int seed = atoi(seed_string);
	srand(seed);

    //sem_init(&field.consec_game, 0, 2);

    pthread_t baseball_threads[TOTAL_BASEBALL];
    pthread_t football_threads[TOTAL_FOOTBALL];
    pthread_t rugby_threads[TOTAL_RUGBY];

    for (int i = 0; i < TOTAL_BASEBALL; i++) {
        pthread_create(&baseball_threads[i], NULL, baseball_player, (void*)(long)i);
    }

    for (int i = 0; i < TOTAL_FOOTBALL; i++) {
        pthread_create(&football_threads[i], NULL, football_player, (void*)(long)i);
    }

    for (int i = 0; i < TOTAL_RUGBY; i++) {
        pthread_create(&rugby_threads[i], NULL, rugby_player, (void*)(long)i);
    }

    for (int i = 0; i < TOTAL_BASEBALL; i++) {
        pthread_join(baseball_threads[i], NULL);
    }

    for (int i = 0; i < TOTAL_FOOTBALL; i++) {
        pthread_join(football_threads[i], NULL);
    }

    for (int i = 0; i < TOTAL_RUGBY; i++) {
        pthread_join(rugby_threads[i], NULL);
    }

    // for (int i = 0; i < tid; i++) {
    //     pthread_join(all_players[i], NULL);
    // }

    pthread_exit(baseball_threads);
    pthread_exit(football_threads);
    pthread_exit(rugby_threads);

    return 0;
}