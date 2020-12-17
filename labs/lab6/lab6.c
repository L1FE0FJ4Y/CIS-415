#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_SIZE 4
#define MAXNAME 256

typedef struct mealTicket {
  int ticketNum;
  char *dish_name;
}mealTicket;

typedef struct MTQ {
  char name[MAXNAME];
  mealTicket **buffer;
  int head;
  int tail;
  int length;
}MTQ;

MTQ *registry[MAX_SIZE];

void q_init (MTQ *queue, char *MTQ_ID) {
  strcpy(queue->name, MTQ_ID);
  queue->buffer = malloc(sizeof(mealTicket *)*MAX_SIZE);
  queue->head = 0;
  queue->tail = 0;
  queue->length = 0;
}

void tck_init(mealTicket *MT, char *MT_ID, int num) {
  MT->dish_name = MT_ID;
  MT->ticketNum = num;
}

int enqueue(char *MTQ_ID, mealTicket *MT) {
  int i = 0;
  for (; i < MAX_SIZE; i++) {
    if (strcmp(registry[i]->name, MTQ_ID) == 0) break;
  }
  MTQ *queue = registry[i];
  if (queue->length == (MAX_SIZE-1)) {
    printf("pushing: Queue %s -Error Queue is full\n", MTQ_ID);
    return 0;
  }
  printf("pushing: Queue %s -Ticket Number: %d -Dish: %s\n", queue->name, MT->ticketNum, MT->dish_name);
  queue->length++;
  queue->buffer[queue->head++] = MT;
  if (queue->head == MAX_SIZE) queue->head = 0;
  return 1;
}

int dequeue (char *MTQ_ID, mealTicket *MT) {
  int i = 0;
  for (; i < MAX_SIZE; i++) {
    if (strcmp(registry[i]->name, MTQ_ID) == 0) break;
  }
  MTQ *queue = registry[i];
  if (queue->length == 0) {
    printf("popping: Queue %s -Queue is empty, nothing to pop\n", MTQ_ID);
    return 0;
  }
  MT = queue->buffer[queue->tail];
  printf("popping: Queue %s -Ticket Number: %d -Dish: %s\n", queue->name, MT->ticketNum, MT->dish_name);
  registry[i]->tail++;
  registry[i]->length--;
  if (registry[i]->tail == MAX_SIZE) registry[i]->tail = 0;
  return 1;
}

int main() {
  MTQ *bf = malloc(sizeof(MTQ));
  MTQ *lc = malloc(sizeof(MTQ));
  MTQ *dn = malloc(sizeof(MTQ));
  MTQ *br = malloc(sizeof(MTQ));
  mealTicket *egg = malloc(sizeof(mealTicket));
  mealTicket *pancake = malloc(sizeof(mealTicket));
  mealTicket *coffee = malloc(sizeof(mealTicket));
  mealTicket *burger = malloc(sizeof(mealTicket));
  mealTicket *fries = malloc(sizeof(mealTicket));
  mealTicket *pepsi = malloc(sizeof(mealTicket));
  mealTicket *steak = malloc(sizeof(mealTicket));
  mealTicket *caviar = malloc(sizeof(mealTicket));
  mealTicket *wine = malloc(sizeof(mealTicket));
  mealTicket *cheese = malloc(sizeof(mealTicket));
  mealTicket *salami = malloc(sizeof(mealTicket));
  mealTicket *beer = malloc(sizeof(mealTicket));
  mealTicket *cupnoodle = malloc(sizeof(mealTicket));

  q_init(bf, "Breakfast");
  q_init(lc, "Lunch");
  q_init(dn, "Dinner");
  q_init(br, "Bar");

  registry[0] = bf;
  registry[1] = lc;
  registry[2] = dn;
  registry[3] = br;

  tck_init(egg, "Egg", 0);
  tck_init(pancake, "Pancake", 1);
  tck_init(coffee, "Coffee", 2);
  tck_init(burger, "Burger", 0);
  tck_init(fries, "Fries", 1);
  tck_init(pepsi, "Pepsi", 2);
  tck_init(steak, "Steak", 0);
  tck_init(caviar, "Caviar", 1);
  tck_init(wine, "Wine", 2);
  tck_init(cheese, "Cheese", 0);
  tck_init(salami, "Salami", 1);
  tck_init(beer, "Beer", 2);
  tck_init(cupnoodle, "Cupnoodle", 3);

  enqueue("Breakfast", egg); enqueue("Breakfast", pancake); enqueue("Breakfast", coffee); enqueue("Breakfast", cupnoodle);
  enqueue("Lunch", burger); enqueue("Lunch", fries); enqueue("Lunch", pepsi); enqueue("Lunch", cupnoodle);
  enqueue("Dinner", steak); enqueue("Dinner", caviar); enqueue("Dinner", wine); enqueue("Dinner", cupnoodle);
  enqueue("Bar", cheese); enqueue("Bar", salami); enqueue("Bar", beer); enqueue("Bar", cupnoodle);
  printf("===============================================================================\n");
  struct mealTicket *test = malloc(sizeof(mealTicket));
  for (int i = 0; i < MAX_SIZE; i++) {
    dequeue("Breakfast", test);
    dequeue("Lunch", test);
    dequeue("Dinner", test);
    dequeue("Bar", test);
  }
  printf("===============================================================================\n");
  dequeue("Breakfast", test);
  dequeue("Lunch", test);
  dequeue("Dinner", test);
  dequeue("Bar", test);

  free(bf->buffer);
  free(lc->buffer);
  free(dn->buffer);
  free(br->buffer);
  free(bf);
  free(lc);
  free(dn);
  free(br);
  free(egg);
  free(pancake);
  free(coffee);
  free(burger);
  free(fries);
  free(pepsi);
  free(steak);
  free(caviar);
  free(wine);
  free(cheese);
  free(salami);
  free(beer);
  free(cupnoodle);
  free(test);
  return 0;
}
