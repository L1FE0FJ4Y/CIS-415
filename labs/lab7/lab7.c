/*=============================================================================
 * Program Name: lab7
 * Author: Jared Hall
 * Edited by Grayson Guan
 * Date: 11/17/2020
 * Description:
 *     A simple program that implements a thread-safe queue of meal tickets
 *
 *===========================================================================*/

//========================== Preprocessor Directives ==========================
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
//=============================================================================

//================================= Constants =================================
#define MAXNAME 15
#define MAXQUEUES 4
#define MAXTICKETS 3
#define MAXDISH 20
#define MAXPUBS 4
#define MAXSUBS 4
//=============================================================================

//============================ Structs and Macros =============================
pthread_cond_t ptc = PTHREAD_COND_INITIALIZER;
pthread_mutex_t ptm = PTHREAD_MUTEX_INITIALIZER;

typedef struct mealTicket{
	int ticketNum;
	char *dish;
} mealTicket;

//TODO: Declare a mutex in the struct. (e.g. add pthread_mutex_t ...)
typedef struct MTQ {
	char name[MAXNAME];
	mealTicket **buffer;
	int head;
	int tail;
	int max_length;
	int length;
	int ticket;
	pthread_mutex_t mutex;
} MTQ;

typedef struct data {
	char **meal;
	mealTicket **order;
} data;

MTQ *registry[MAXQUEUES]; //INFO: Changed to be an array of structs
//=============================================================================

//TODO: Add a mutex init to this function (e.g registry[pos].mutex = PTHREAD_MUTEX_INITIALIZER)
//TODO: Add a conditional mutex, use pthread_cond_wait and pthread_cond_broadcast to make all threads hold until they are all ready to start together.

//A good example link is posted here
//https://www.geeksforgeeks.org/condition-wait-signal-multi-threading/

//It will be really helpeful for you to add a print entry function for debug and demonstration purpose.
void q_init (MTQ *queue, char *MTQ_ID) {
  strcpy(queue->name, MTQ_ID);
  queue->buffer = malloc(sizeof(mealTicket *)*MAXTICKETS);
  queue->head = 0;
  queue->tail = 0;
  queue->length = 0;
}

void freeMTQ(MTQ *queue) {
	free(queue->buffer);
	free(queue);
}

void freeDATA(data *queue) {
	free(queue->order);
	free(queue);
}

//================================= Functions =================================
int enqueue(char *MTQ_ID, mealTicket *MT) {
	//Step-1: Find registry

	//STEP-2:
		//TODO: Aquire the lock if it's available. Otherwise, wait until it is.
		//HINT: Use conditional variables and sched_yield.
	//SETP-3: enqueue
        //....................
        //....................
	//TODO: Release the lock.
	int i = 0;
	for (; i < MAXQUEUES; i++) {
		if (strcmp(registry[i]->name, MTQ_ID) == 0) break;
	}
	MTQ *queue = registry[i];
	pthread_mutex_lock(&queue->mutex);
	if (queue->length == (MAXQUEUES-1)) {
		printf("pushing: Queue %s -Error Queue is full\n", MTQ_ID);
		pthread_mutex_unlock(&queue->mutex);
		return 0;
	}
	MT->ticketNum = registry[i]->head;
	printf("pushing: Queue %s -Ticket Number: %d -Dish: %s\n", queue->name, MT->ticketNum, MT->dish);
	queue->length++;
	queue->buffer[queue->head++] = MT;
	if (queue->head == MAXQUEUES) queue->head = 0;
	pthread_mutex_unlock(&queue->mutex);
	return 1;
}

int dequeue(char *MTQ_ID, mealTicket *MT) {
	//Step-1: Find registry

	//Step-2:
		//TODO: Aquire the lock if it's available. Otherwise, wait until it is.
		//HINT: Use conditional variables and sched_yield.
	//Setp-3: dequeue
	    //....................
	    //....................
	//Step-4:
		//TODO: Release the lock.
	int i = 0;
	for (; i < MAXQUEUES; i++) {
		if (strcmp(registry[i]->name, MTQ_ID) == 0) break;
	}
	MTQ *queue = registry[i];
	pthread_mutex_lock(&queue->mutex);
	if (queue->length == 0) {
		printf("popping: Queue %s -Queue is empty, nothing to pop\n", MTQ_ID);
		pthread_mutex_unlock(&queue->mutex);
		return 0;
	}
	MT = queue->buffer[queue->tail];
	MT->ticketNum = registry[i]->tail;
	printf("popping: Queue %s -Ticket Number: %d -Dish: %s\n", queue->name, MT->ticketNum, MT->dish);
	registry[i]->tail++;
	registry[i]->length--;
	if (registry[i]->tail == MAXQUEUES) registry[i]->tail = 0;
	pthread_mutex_unlock(&queue->mutex);
	return 1;
}


void *publisher(void *args) {
	/* TODO: The publisher will receive the following in the struct args:
	*        1. An array of mealTickets to push to the queue.
	*        2. For each meal ticket, an MTQ_ID. (Note: a publisher can push to multiple queues)
	*        3. The thread ID
	* The publisher will do the pthread_cond_wait procedure, and wait for a cond signal to begin its work
	* The publisher will then print its type and thread ID on startup. Then it will push one meal ticket at a time to
	* its appropriate queue before sleeping for 1 second. It will do this until there are no more meal tickets to push.
	*/
	data *copy = (data*)args;
	printf("type: publisher\tthread ID: %ld\twaiting\n", pthread_self());
	pthread_mutex_lock(&ptm);
	pthread_cond_wait(&ptc, &ptm);
	pthread_mutex_unlock(&ptm);
	for (int i = 0; i < MAXTICKETS; i++) {
		printf("type: publisher\tthread ID: %ld\n", pthread_self());
		enqueue(*copy->meal, copy->order[i]);
		sleep(1);
	}
}

void *subscriber(void *args) {
	/* TODO:The subscriber will take the following:
	*       1. The MTQ_ID it will pull from.
	*       2. The thread ID
	*       3. An empty meal ticket.
	* The subscriber will do the pthread_cond_wait procedure, and wait for a cond signal to begin its work
	* The subscriber will print its type and thread ID on startup. Then it will pull a ticket from its queue
	* and print it. If the queue is empty then it will print an empty message along with its
	* thread ID and wait for 1 second. If the thread is not empty then it will pop a ticket and
	* print it along with the thread id.
	*/
	data *copy = (data*)args;
	pthread_mutex_lock(&ptm);
	pthread_cond_wait(&ptc, &ptm);
	printf("type: subscriber\tthread ID: %ld\twaiting\n", pthread_self());
	pthread_mutex_unlock(&ptm);
	int check = dequeue(*copy->meal, copy->order[0]);
	while (check != 0) {
		printf("type: subscriber\tthread ID: %ld\tdequeue Meal Ticket\n", pthread_self());
		check = dequeue(*copy->meal, copy->order[0]);
		sleep(1);
	}
}
//=============================================================================

//=============================== Program Main ================================
int main(int argc, char argv[]) {
	//Variables Declarations
	char *qNames[] = {"Breakfast", "Lunch", "Dinner", "Bar"};
	char *bFood[] = {"Eggs", "Bacon", "Steak"};
	char *lFood[] = {"Burger", "Fries", "Coke"};
	char *dFood[] = {"Steak", "Greens", "Pie"};
	char *brFood[] = {"Whiskey", "Sake", "Wine"};

	int i, j;
	char dsh[] = "Empty";
	mealTicket bfast[3] = {[0].dish = bFood[0], [1].dish = bFood[1], [2].dish = bFood[2]};
	mealTicket lnch[3] = {[0].dish = lFood[0], [1].dish = lFood[1], [2].dish = lFood[2]};
	mealTicket dnr[3] = {[0].dish = dFood[0], [1].dish = dFood[1], [2].dish = dFood[2]};
	mealTicket br[3] = {[0].dish = brFood[0], [1].dish = brFood[1], [2].dish = brFood[2]};
	mealTicket ticket = {.ticketNum=0, .dish=dsh};

	//STEP-1: Initialize the registry
	for (i = 0; i < MAXQUEUES; i++) {
		registry[i] = malloc(sizeof(MTQ));
		q_init(registry[i], qNames[i]);
		pthread_mutex_init(&registry[i]->mutex, NULL);
	}

	//STEP-2: Create the publisher thread-pool
	pthread_t pubtds[MAXPUBS];

	data *bfargs = malloc(sizeof(data));
	bfargs->order = malloc(sizeof(mealTicket*)*MAXDISH);
	bfargs->meal = &qNames[0];
	for (j = 0; j < MAXDISH; j++) {
		bfargs->order[j] = &bfast[j];
	}
	pthread_create(&pubtds[0], NULL, publisher, bfargs);

	data *lnargs = malloc(sizeof(data));
	lnargs->order = malloc(sizeof(mealTicket*)*MAXDISH);
	lnargs->meal = &qNames[1];
	for (j = 0; j < MAXDISH; j++) {
		lnargs->order[j] = &lnch[j];
	}
	pthread_create(&pubtds[1], NULL, publisher, lnargs);

	data *dnargs = malloc(sizeof(data));
	dnargs->order = malloc(sizeof(mealTicket*)*MAXDISH);
	dnargs->meal = &qNames[2];
	for (j = 0; j < MAXDISH; j++) {
		dnargs->order[j] = &dnr[j];
	}
	pthread_create(&pubtds[2], NULL, publisher, dnargs);

	data *bargs = malloc(sizeof(data));
	bargs->order = malloc(sizeof(mealTicket*)*MAXDISH);
	bargs->meal = &qNames[3];
	for (j = 0; j < MAXDISH; j++) {
		bargs->order[j] = &br[j];
	}
	pthread_create(&pubtds[3], NULL, publisher, bargs);


	sleep(1);
	pthread_mutex_lock(&ptm);
	printf("BROADCASTING\n");
	pthread_cond_broadcast(&ptc);
	pthread_mutex_unlock(&ptm);

	//STEP-3: Create the subscriber thread-pool
	pthread_t subtds[MAXSUBS];

	data *bfret = malloc(sizeof(data));
	bfret->order = malloc(sizeof(mealTicket*));
	bfret->meal = &qNames[0];
	bfret->order[0] = &ticket;
	pthread_create(&subtds[0], NULL, subscriber, bfret);

	data *lnret = malloc(sizeof(data));
	lnret->order = malloc(sizeof(mealTicket*));
	lnret->meal = &qNames[1];
	lnret->order[0] = &ticket;
	pthread_create(&subtds[1], NULL, subscriber, lnret);


	data *dnret = malloc(sizeof(data));
	dnret->order = malloc(sizeof(mealTicket*));
	dnret->meal = &qNames[2];
	dnret->order[0] = &ticket;
	pthread_create(&subtds[2], NULL, subscriber, dnret);

	data *bret = malloc(sizeof(data));
	bret->order = malloc(sizeof(mealTicket*));
	bret->meal = &qNames[3];
	bret->order[0] = &ticket;
	pthread_create(&subtds[3], NULL, subscriber, bret);

	sleep(1);
	pthread_mutex_lock(&ptm);
	pthread_cond_broadcast(&ptc);
	pthread_mutex_unlock(&ptm);

	//STEP-4: Join the thread-pools
	for (i = 0; i < MAXQUEUES; i++) {
		pthread_join(pubtds[i], NULL);
		pthread_join(subtds[i], NULL);
	}

	//STEP-5: Free the registry
	for (i = 0; i < MAXQUEUES; i++) {
		freeMTQ(registry[i]);
	}

	freeDATA(bfargs);
	freeDATA(lnargs);
	freeDATA(dnargs);
	freeDATA(bargs);
	freeDATA(bfret);
	freeDATA(lnret);
	freeDATA(dnret);
	freeDATA(bret);
	return EXIT_SUCCESS;
}
//=============================================================================
