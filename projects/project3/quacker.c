#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include "string_parser.h"

#define NUMPROXIES 4
#define MAXNAME 15
#define URLSIZE 1024
#define CAPSIZE 1024
#define MAXQUEUES 3
#define SEPARATORS " \t"

int work;
pthread_t cleanup_thread;
float delta;
pthread_cond_t ptc = PTHREAD_COND_INITIALIZER;
pthread_mutex_t ptm = PTHREAD_MUTEX_INITIALIZER;

typedef struct topicEntry{
	int entryNum;
	struct timeval timeStamp;
  int pubID;
  char photoURL[URLSIZE];
  char photoCaption[CAPSIZE];
} topicEntry;

typedef struct TEQ {
	char topic[MAXNAME];
	topicEntry **buffer;
	int head;
	int tail;
	int length;
	int max;
	int entry_counter;
	pthread_mutex_t mutex;
} TEQ;

typedef struct thread_pool {
	pthread_t tid;
	int flag;
} thread_pool;

TEQ *registry[MAXQUEUES];
thread_pool pubpool[NUMPROXIES];
thread_pool subpool[NUMPROXIES];

//format
void dq_remove(char *str);

void pool_init();
void q_init (TEQ *queue, char *TEQ_ID, int size);
void te_init (topicEntry *TE, char *url, char *cap);
void freeTEQ(TEQ *queue);

//threads routines particles
int enqueue(char *TEQ_ID, topicEntry *TE);
int getEntry(char *TEQ_ID, int lastEntry, topicEntry *TE);
int dequeue(char *TEQ_ID);

//threads routines
void run_pub(int num_token, char **command_list);
void run_sub(int num_token, char **command_list, int lastEntry);
void run_put(int num_token, char **command_list);
void run_get(int num_token, char **command_list, int lastEntry);
void run_sleep(int num_token, char **command_list);
void run_pub_stop();
void run_sub_stop();

//threads
void *publisher(void *args);
void *subscriber(void *args);
void *cleanup();

//main thread file reading routines
void run_insta(int num_token, char **command_list);
void run_create_topic(int num_token, char **command_list);
void run_add(int num_token, char **command_list);
void run_set_delta(int num_token, char **command_list);
void run_start(int num_token, char **command_list);

//main
int main(int argc, char const *argv[]) {
	//open file
  if (argc == 2) {
    int count = 0, size;
    FILE *ipf;
		char file_loc[] = "input/";
		strcat(file_loc, argv[1]);
		ipf = fopen(file_loc, "r");
		if (ipf == NULL) {
			exit(EXIT_FAILURE);
		} else {
      fseek(ipf, 0, SEEK_END);
      size = ftell(ipf);
      fseek(ipf, 0, SEEK_SET);
    }
		size_t len = 0;
		size_t lineNum = 0;
		char *line = NULL;
		//working
		work = 1;
		//get each command
		pool_init();
		while(getline(&line, &len, ipf) != EOF) {
			command_line cl = str_filler(line, SEPARATORS);
			run_insta(cl.num_token, cl.command_list);
		}
		fclose(ipf);
		free(line);
	} else {
		printf("Error: Run the program incorrectly!\n");
	}
	//Join the thread-pools
	for (int i = 0; i < MAXQUEUES; i++) {
		pthread_join(pubpool[i].tid, NULL);
		pthread_join(subpool[i].tid, NULL);
	}
	work = 0;
	pthread_join(cleanup_thread, NULL);
	//Free the registry
	for (int i = 0; i < MAXQUEUES; i++) {
		freeTEQ(registry[i]);
	}
	return EXIT_SUCCESS;
}




//double quote
void dq_remove(char *str) {
	int size = strlen(str)-1;
	char *noq = (char*)malloc(size);
	strncpy(noq, str+1, size-1);
	noq[size-1] = '\0';
	strcpy(str, noq);
	free(noq);
}

void pool_init () {
	  for (int i = 0; i < (NUMPROXIES); i++) {
	  	pubpool[i].flag = 1;
			subpool[i].flag = 1;
	  }
}
void q_init (TEQ *queue, char *TEQ_ID, int size) {
	  strcpy(queue->topic, TEQ_ID);
	  queue->buffer = malloc(sizeof(topicEntry *)*MAXQUEUES);
	  queue->head = 0;
	  queue->tail = 0;
	  queue->length = 0;
		queue->max = size;
		queue->entry_counter = 0;
		pthread_mutex_init(&queue->mutex, NULL);
	}
void te_init (topicEntry *TE, char *url, char *cap) {
	  strcpy(TE->photoURL, url);
	  strcpy(TE->photoCaption, cap);
	  TE->entryNum = -1;
	}
void freeTEQ(TEQ *queue) {
		free(queue->buffer);
		free(queue);
	}

int enqueue(char *TEQ_ID, topicEntry *TE) {
	int i = 1;
	for (; i < MAXQUEUES; i++) {
		if (!strcmp(registry[i]->topic, TEQ_ID)) break;
	}
	TEQ *queue = registry[i];
	while (queue->length == (MAXQUEUES-1)) {
		printf("pushing: Queue %s -Error Queue is full -waiting\n", TEQ_ID);
		pthread_mutex_unlock(&queue->mutex);
		return 0;
	}
	TE->entryNum = ++queue->entry_counter;
	gettimeofday(&TE->timeStamp, NULL);
	TE->pubID = pthread_self();
	queue->length++;
	queue->buffer[queue->head++] = TE;
	printf("pushing: Queue %s -URL: %s -Caption: %s\n", queue->topic, queue->buffer[((queue->head)-1)]->photoURL, queue->buffer[((queue->head)-1)]->photoCaption);
	if (queue->head == MAXQUEUES) queue->head = 0;
	pthread_mutex_unlock(&queue->mutex);
	return 1;
}
int getEntry(char *TEQ_ID, int lastEntry, topicEntry *TE) {
	int i = 1;
	for (; i < MAXQUEUES; i++) {
		if (!strcmp(registry[i]->topic, TEQ_ID)) break;
	}
	TEQ *queue = registry[i];
	pthread_mutex_lock(&queue->mutex);
	//Case Empty
	if (queue->length == 0) {
		printf("getting entry: Queue %s -Queue is empty, nothing to get\n", TEQ_ID);
		pthread_mutex_unlock(&queue->mutex);
		return 0;
	}
	int update = queue->entry_counter;
	//Case No enqueue yet
	if (lastEntry == update) {
		printf("getting entry: Queue %s -Queue is the latest, nothing to get\n", TEQ_ID);
		pthread_mutex_unlock(&queue->mutex);
		return 0;
	}
	printf("getting entry: Queue %s\tthread ID: %ld\n", queue->topic, pthread_self());
	int pos = queue->tail;
	//Case Get dequeued entry
	printf("WE GOOD TIL HERE\n");
	//Cant move data to empty struct
	//
	//no part 5...
	if (queue->buffer[pos]->entryNum > lastEntry) {
		lastEntry = queue->buffer[pos]->entryNum;
		TE = queue->buffer[pos];
		printf("getting entry: Queue %s\tthread ID: %ld\tItem looking for is gone, oldest entry number: %d\n", queue->topic, pthread_self(), lastEntry);
		pthread_mutex_unlock(&queue->mutex);
		return lastEntry;
	}
	//Case Get entry
	if (lastEntry < update) {
		lastEntry = queue->buffer[++pos]->entryNum;
		TE = queue->buffer[pos];
		printf("getting entry: Queue %s\tthread ID: %ld\tURL: %s\tCAP: %s\n", queue->topic, pthread_self(), TE->photoURL, TE->photoCaption);
		pthread_mutex_unlock(&queue->mutex);
		return 1;
	}
}
int dequeue(char *TEQ_ID) {
	int i = 1;
	for (; i < MAXQUEUES; i++) {
		if (!strcmp(registry[i]->topic, TEQ_ID)) break;
	}
	TEQ *queue = registry[i];
	pthread_mutex_lock(&queue->mutex);
	if (queue->length == 0) {
		printf("popping: Queue %s -Queue is empty, nothing to pop\n", TEQ_ID);
		pthread_mutex_unlock(&queue->mutex);
		return 0;
	}
	int pos = queue->tail;
	struct timeval timeNow;
	gettimeofday(&timeNow, NULL);
	//Checking age
	float timer = (float)(timeNow.tv_sec - queue->buffer[pos]->timeStamp.tv_sec);
	if (timer >= delta) {
		printf("popping: Queue %s\tthread ID: %ld\n", queue->topic, pthread_self());
		queue->tail++;
		queue->length--;
		if (queue->tail == MAXQUEUES) queue->tail = 0;
		pthread_mutex_unlock(&queue->mutex);
		return 1;
	} else {
		printf("popping: Queue %s -Queue is too young, cannot pop\n", TEQ_ID);
		pthread_mutex_unlock(&queue->mutex);
		return 0;
	}
}

void run_pub(int num_token, char **command_list) {
  char *commands[] = {"put", "sleep", "stop"};
  //put
  if (!strcmp(command_list[0], commands[0])) {
  	run_put(num_token, command_list);
		printf("PUT DONE\n");
  }
  //sleep
  if (!strcmp(command_list[0], commands[1])) {
    run_sleep(num_token, command_list);
		printf("SLEEP DONE\n");
  }
  //stop
  if (!strcmp(command_list[0], commands[2])) {
    run_pub_stop();
		printf("STOP DONE\n");
  }
}
void run_sub(int num_token, char **command_list, int lastEntry) {
  char *commands[] = {"get", "sleep", "stop"};
  //get
  if (!strcmp(command_list[0], commands[0])) {
    run_get(num_token, command_list, lastEntry);
  }
  //sleep
  if (!strcmp(command_list[0], commands[1])) {
    run_sleep(num_token, command_list);
		printf("SLEEP DONE\n");
  }
  //stop
  if (!strcmp(command_list[0], commands[2])) {
    run_sub_stop();
		printf("STOP DONE\n");
  }
}
void run_put(int num_token, char **command_list) {
  int pos = atoi(command_list[1]), i = 0;
  topicEntry *te = malloc(sizeof(topicEntry));
	//get rid of ""
	dq_remove(command_list[2]);
	strcpy(te->photoURL, command_list[2]);
	char cap[CAPSIZE];
	for (int j = 3; j < num_token-1; j++) {
		if (j == 3) {
			strcpy(cap, command_list[j]);
		} else {
			strcat(cap, " ");
			strcat(cap, command_list[j]);
		}
	}
	dq_remove(cap);
	strcpy(te->photoCaption, cap);
	while(!enqueue(registry[pos]->topic, te)) {
		sched_yield();
		sleep(delta);
	}
	printf("type: publisher\tthread ID: %ld\n", pthread_self());
	sched_yield();
	sleep(1);
}
void run_get(int num_token, char **command_list, int lastEntry) {
	int pos = atoi(command_list[1]);
  topicEntry *ret_struct = malloc(sizeof(topicEntry));
	int ret = getEntry(registry[pos]->topic, lastEntry, ret_struct);
	if (ret)	{
		printf("type: subscriber\tthread ID: %ld\tgetting entry\n", pthread_self());
		if (ret != 1) {
			printf("type: subscriber\tthread ID: %ld\tEntry has changed: dequeued\tNew: %d\n", pthread_self(), ret);
		}
		sched_yield();
		sleep(1);
	}
}
void run_sleep(int num_token, char **command_list) {
  if (num_token >= 1) {
    unsigned long int mlsec = strtoul(command_list[1], NULL, 10);
		if (mlsec >= 0) {
			long int sec = mlsec / 1000;
			long int mcsec = (mlsec-(sec*1000))*1000;
			sleep(sec);
			usleep(mcsec);
		} else {
			printf("Invalid amount of arguments: sleep cannot have negative number\n");
		}
  } else {
    printf("Invalid amount of arguments: sleep requires 2\n");
  }
}
void run_pub_stop() {
  long int id = pthread_self();
  for (int idx = 0; idx < NUMPROXIES; idx++) {
		if (pubpool[idx].tid == id) {
			pubpool[idx].flag = 1;
			break;
		}
	}
	pthread_exit(0);
}
void run_sub_stop() {
	long int id = pthread_self();
	for (int idx = 0; idx < NUMPROXIES; idx++) {
		if (subpool[idx].tid == id) {
			subpool[idx].flag = 1;
			break;
		}
	}
	pthread_exit(0);
}

void *publisher(void *args) {
	char *copy = (char*)args;
	int count = 0, size;
	FILE *ipf;
	ipf = fopen(copy, "r");
	if (ipf == NULL) {
		exit(EXIT_FAILURE);
	} else {
		fseek(ipf, 0, SEEK_END);
		size = ftell(ipf);
		fseek(ipf, 0, SEEK_SET);
	}
	size_t len = 0;
	size_t lineNum = 0;
	char *line = NULL;
	//get each line
	printf("type: publisher\tthread ID: %ld\twaiting\n", pthread_self());
	pthread_mutex_lock(&ptm);
	pthread_cond_wait(&ptc, &ptm);
	pthread_mutex_unlock(&ptm);
	printf("type: publisher\tthread ID: %ld\tbegin\n", pthread_self());
	while(getline(&line, &len, ipf) != EOF) {
		command_line cl = str_filler(line, SEPARATORS);
		run_pub(cl.num_token, cl.command_list);
		free_command_line(&cl);
	}
	fclose(ipf);
	free(line);
}
void *subscriber(void *args) {
	int lastEntry = 0;
	char *copy = (char*)args;
	int count = 0, size;
	FILE *ipf;
	ipf = fopen(copy, "r");
	if (ipf == NULL) {
		exit(EXIT_FAILURE);
	} else {
		fseek(ipf, 0, SEEK_END);
		size = ftell(ipf);
		fseek(ipf, 0, SEEK_SET);
	}
	size_t len = 0;
	size_t lineNum = 0;
	char *line = NULL;
	//get each line
	printf("type: sublisher\tthread ID: %ld\twaiting\n", pthread_self());
	pthread_mutex_lock(&ptm);
	pthread_cond_wait(&ptc, &ptm);
	pthread_mutex_unlock(&ptm);
	printf("type: subscriber\tthread ID: %ld\tbegin\n", pthread_self());
	while(getline(&line, &len, ipf) != EOF) {
		command_line cl = str_filler(line, SEPARATORS);
		run_sub(cl.num_token, cl.command_list, lastEntry);
		free_command_line(&cl);
	}
	fclose(ipf);
	free(line);
}
void *cleanup() {
	sleep(delta);
	int ret, idx = 0;
	while (work) {
		printf("\n\n\nEXTERMINATOR\n");
		if (idx == MAXQUEUES) idx = 0;
		ret = dequeue(registry[++idx]->topic);
		while (ret == 0) {
			printf("type: cleanup\tthread ID: %ld\twaiting for 15\n", pthread_self());
			ret = dequeue(registry[idx]->topic);
			sched_yield();
			sleep(delta);
		}
	}
	pthread_exit(0);
}

void run_insta(int num_token, char **command_list) {
	//command list
	char *commands[] = {"create", "query", "add", "delta", "start"};
  //create topic
  if (!strcmp(command_list[0], commands[0])) {
  	run_create_topic(num_token, command_list);
		printf("CREATION SUCCESS\n");
  }
  //add pub and sub
  if (!strcmp(command_list[0], commands[2])) {
    run_add(num_token, command_list);
		printf("ADD SUCCESS\n");
  }
  //delta
  if (!strcmp(command_list[0], commands[3])) {
  	run_set_delta(num_token, command_list);
		printf("%f\n", delta);
		printf("DELTA SUCCESS\n");
  }
  //start
  if (!strcmp(command_list[0], commands[4])) {
		pthread_create(&cleanup_thread, NULL, cleanup, NULL);
    run_start(num_token, command_list);
		printf("START SUCCESS\n");
  }
}
void run_create_topic(int num_token, char **command_list) {
  if (num_token >= 4) {
    if (!strcmp(command_list[1], "topic")) {
      int pos = atoi(command_list[2]);
      registry[pos] = malloc(sizeof(TEQ));
			dq_remove(command_list[3]);
      q_init(registry[pos], command_list[3], atoi(command_list[4]));
    }
  } else {
    printf("Invalid amount of arguments: create topic requires 5\n");
  }
}
void run_add(int num_token, char **command_list) {
  if (num_token >= 2) {
    int count = 0;
		dq_remove(command_list[2]);
		char *inpf = (char*)malloc(strlen(command_list[2])+7);
		strcpy(inpf, "input/");
		strcat(inpf, command_list[2]);
		printf("%s\n", inpf);
		command_list[2] = realloc(command_list[2], strlen(inpf)+2);
		strcpy(command_list[2], inpf);
		free(inpf);

    if (!strcmp(command_list[1], "publisher")) {
			for (; count < NUMPROXIES; count++) {
				if (pubpool[count].flag) {
					pubpool[count].flag = 0;
					pthread_create(&pubpool[count].tid, NULL, publisher, command_list[2]);
					break;
				}
			}
    }
    if (!strcmp(command_list[1], "subscriber")) {
			for (; count < NUMPROXIES; count++) {
				if (subpool[count].flag) {
        	subpool[count].flag = 0;
        	pthread_create(&subpool[count].tid, NULL, subscriber, command_list[2]);
					break;
				}
			}
    }
  } else {
    printf("Invalid amount of arguments: add requires 3\n");
  }
}
void run_set_delta(int num_token, char **command_list) {
  if (num_token >= 1) {
    if (strtof(command_list[1], NULL) > 0) {
      delta = strtof(command_list[1], NULL);
    } else {
      printf("Delta must be greater than 0.00 sec\n");
    }
  } else {
    printf("Invalid amount of arguments: delta requires 2\n");
  }
}
void run_start(int num_token, char **command_list) {
  if (num_token >= 0) {
    pthread_mutex_lock(&ptm);
    printf("BROADCASTING\n");
    pthread_cond_broadcast(&ptc);
    pthread_mutex_unlock(&ptm);
  } else {
    printf("Invalid amount of arguments: start requires 1\n");
  }
}
