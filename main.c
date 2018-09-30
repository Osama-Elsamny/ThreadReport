#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>

#define MAX_THREADS 30

bool all_threads_are_created = false;
int personCounter = 0;
int numPeopleGlobal = MAX_THREADS;
bool arrayLocks[MAX_THREADS];  
pthread_mutex_t personCounter_mutex;
pthread_mutex_t fetch_inc_mutex;

typedef struct{
    int next_ticket;
    int now_servig; 
} TicketLock;

TicketLock *ticketLock;

typedef struct{
    int personNumber;
    long double arrivalTime;
    long double waitingTime;
    long double leavingTime;
} Person;

/** 
 * Funcition name: fetch_and_inc
 * Purpose:  Should be an atomic instruction that we will use in acquring a lock 
 * Developer: Osama Elsamny
 * Input: N/A
 * Output: return an int that represents the number of the ticket holder
*/
int fetch_and_inc(){
    int x = ticketLock->next_ticket;
    (ticketLock->next_ticket)++;
    return x;
}

/** 
 * Funcition name: acquire_lock
 * Purpose: acquireing the lock for method 3 (Ticket Lock)
 * Developer: Osama Elsamny
 * Input: N/A
 * Output: N/A
*/
void acquire_lock(){
    pthread_mutex_lock(&fetch_inc_mutex);
    int myTicket = fetch_and_inc();
    pthread_mutex_unlock(&fetch_inc_mutex);
    while(ticketLock->now_servig != myTicket){}
}

/** 
 * Funcition name: release_lock
 * Purpose: releaseing the lock for method 3 (Ticket Lock)
 * Developer: Osama Elsamny
 * Input: N/A
 * Output: N/A
*/
void release_lock(){
    (ticketLock->now_servig)++;
}

/** 
 * Funcition name:current_time_in_ms
 * Purpose: To know the current time in ms
 * Developer: Osama Elsamny
 * Input: N/A
 * Output: return type is double varible
*/
long double current_time_in_ms(void){
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    return now.tv_sec * 1000000000.0 + now.tv_nsec;
}

/** 
 * Funcition name:initialzeArrayLocks
 * Purpose: To initial all the array to false for th lock required for method 2
 * Developer: Osama Elsamny
 * Input: N/A
 * Output: N/A
*/
void initialzeArrayLocks(){
    for(int i = 0; i < numPeopleGlobal; i++){
		arrayLocks[i] = false;
	}
}

/** 
 * Funcition name: createThreadMethodOne()
 * Purpose: Using the pthread_mutex in order to increment a variable 
 * Developer: Osama Elsamny
 * Input: the spcific number of each thread
 * Output: N/A
*/
void* createThreadMethodOne(void* arg){
    Person *person = (Person*) malloc(sizeof(Person));
    person->personNumber = *((int *) arg);
    person->arrivalTime = current_time_in_ms();
    while(true){
        if(!all_threads_are_created)
            continue;
        pthread_mutex_lock(&personCounter_mutex);
        person->waitingTime = current_time_in_ms() - person->arrivalTime;
        personCounter++;
        printf("personCounter: %d\n", personCounter);
        pthread_mutex_unlock(&personCounter_mutex);
        person->leavingTime = current_time_in_ms();
        printf("Person with number (%d) is done increming the counter with the following statistics: Arrival_Time:%Lf, Delay_Time:%Lf, Leaving_Time:%Lf\n" ,person->personNumber, person->arrivalTime, person->waitingTime, person->leavingTime);
        break;
    }
    free(person);
    printf("Person with number (%d) is home.\n", *((int *) arg));
    return NULL;
}

/** 
 * Funcition name: createThreadMethodTwo()
 * Purpose: * Have all threads spin on a array cells of an array. 
 *          * Once the cell becomes true, then have the thread that was spinning 
 *          * on that cell set the next cell to true as well.
 * Developer: Osama Elsamny
 * Input: the spcific number of each thread
 * Output: N/A
*/
void* createThreadMethodTwo(void* arg){
    Person *person = (Person*) malloc(sizeof(Person));
    person->personNumber = *((int *) arg);
    person->arrivalTime = current_time_in_ms();
    while(true){
        if(!all_threads_are_created)
            continue;
        if(arrayLocks[person->personNumber] == false)
            continue;
        person->waitingTime = current_time_in_ms() - (person->arrivalTime);
        personCounter++;
        printf("personCounter: %d\n", personCounter);
        arrayLocks[(person->personNumber) + 1] = true;
        person->leavingTime = current_time_in_ms();
        printf("Person with number (%d) is done increming the counter with the following statistics: Arrival_Time:%Lf, Delay_Time:%Lf, Leaving_Time:%Lf\n" ,person->personNumber, person->arrivalTime, person->waitingTime, person->leavingTime);
        break;
    }
    free(person);
    printf("Person with number (%d) is home.\n", *((int *) arg));
    return NULL;
}

/** 
 * Funcition name: createThreadMethodThree()
 * Purpose: * Have all threads spin on a array cells of an array. 
 *          * Once the cell becomes true, then have the thread that was spinning 
 *          * on that cell set the next cell to true as well.
 * Developer: Osama Elsamny
 * Input: the spcific number of each thread
 * Output: N/A
*/
void* createThreadMethodThree(void* arg){
    Person *person = (Person*) malloc(sizeof(Person));
    person->personNumber = *((int *) arg);
    person->arrivalTime = current_time_in_ms();
    while(true){
        if(!all_threads_are_created)
            continue;
        /*lock*/
        acquire_lock();
        person->waitingTime = current_time_in_ms() - person->arrivalTime;
        personCounter++;
        printf("personCounter: %d\n", personCounter);
        /*unlock*/
        release_lock();
        person->leavingTime = current_time_in_ms();
        printf("Person with number (%d) is done increming the counter with the following statistics: Arrival_Time:%Lf, Delay_Time:%Lf, Leaving_Time:%Lf\n" ,person->personNumber, person->arrivalTime, person->waitingTime, person->leavingTime);
        break;
    }
    free(person);
    printf("Person with number (%d) is home.\n", *((int *) arg));
    return NULL;
}

/** 
 * Funcition name: arrayEnumeration
 * Purpose: To enumerate the number of threads created inorder to keep track of the
 *          number of thread while debuging. 
 * Developer: Osama Elsamny
 * Input: max number of entries in the arry and a pointer to array created 
 * Output: N/A
*/
void arrayEnumeration(int max, int *array){
    for(int i = 0; i < max; i++){
		array[i] = i;
	}
}

/**
 * Funcition name: main()
 * Purpose: * Testing 3 different methods of locks to know which one method is better 
 *              using 1000 tread in all cases.   
 *          * Also to the run the main thread of the program. 
 * Developer: Osama Elsamny
 * Input: N/A
 * Output: N/A
*/
int main(){
    /*-------------------------------------Method One------------------------------------------------*/
    printf("-------------------------------------Method One------------------------------------------------\n");
    int numPeople1 = numPeopleGlobal;
    pthread_t people1[numPeople1];
    int peopleEnumeration1[numPeople1];
    arrayEnumeration(numPeople1, peopleEnumeration1);
    for(int i = 0; i < numPeople1; i++){
        pthread_create(&people1[i], NULL, createThreadMethodOne, &peopleEnumeration1[i]);
    }
    all_threads_are_created = true;
    sleep(30);
    for(int i = 0; i < numPeople1; i++){
        pthread_join(people1[i], NULL);
    }
    printf("-------------------------------------Method Two------------------------------------------------\n");
    /*-------------------------------------Method Two------------------------------------------------*/
    personCounter = 0;
    all_threads_are_created = false;
    int numPeople2 = numPeopleGlobal;
    pthread_t people2[numPeople2];
    int peopleEnumeration2[numPeople2];
    arrayEnumeration(numPeople2, peopleEnumeration2);
    initialzeArrayLocks();
    for(int i = 0; i < numPeople2; i++){
        pthread_create(&people2[i], NULL, createThreadMethodTwo, &peopleEnumeration2[i]);
    }
    all_threads_are_created = true;
    arrayLocks[0] = true;
    sleep(30);
    for(int i = 0; i < numPeople2; i++){
        pthread_join(people2[i], NULL);
    }
    printf("-------------------------------------Method Three------------------------------------------------\n");
    /*-------------------------------------Method Three------------------------------------------------*/
    personCounter = 0;
    ticketLock = (TicketLock*) malloc(sizeof(TicketLock));
    ticketLock->next_ticket = 0;
    ticketLock->now_servig = 0;
    all_threads_are_created = false;
    int numPeople3 = numPeopleGlobal;
    pthread_t people3[numPeople3];
    int peopleEnumeration3[numPeople3];
    arrayEnumeration(numPeople3, peopleEnumeration3);
    for(int i = 0; i < numPeople3; i++){
        pthread_create(&people3[i], NULL, createThreadMethodThree, &peopleEnumeration3[i]);
    }
    all_threads_are_created = true;
    sleep(30);
    for(int i = 0; i < numPeople3; i++){
        pthread_join(people3[i], NULL);
    }
    return 0;
}
//Templet for the functions
/** 
 * Funcition name:
 * Purpose: 
 * Developer:
 * Input: 
 * Output: 
*/