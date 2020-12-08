#include "BENSCHILLIBOWL.h"

#include <assert.h>
#include <stdlib.h>
#include <time.h>

bool IsEmpty(BENSCHILLIBOWL* bcb);
bool IsFull(BENSCHILLIBOWL* bcb);
void AddOrderToBack(Order **orders, Order *order);

MenuItem BENSCHILLIBOWLMenu[] = { 
    "BensChilli", 
    "BensHalfSmoke", 
    "BensHotDog", 
    "BensChilliCheeseFries", 
    "BensShake",
    "BensHotCakes",
    "BensCake",
    "BensHamburger",
    "BensVeggieBurger",
    "BensOnionRings",
};
int BENSCHILLIBOWLMenuLength = 10;

/* Select a random item from the Menu and return it */
MenuItem PickRandomMenuItem() {
    int random_index = rand() % BENSCHILLIBOWLMenuLength;
    return BENSCHILLIBOWLMenu[random_index];
}

/* Allocate memory for the Restaurant, then create the mutex and condition variables needed to instantiate the Restaurant */

BENSCHILLIBOWL* OpenRestaurant(int max_size, int expected_num_orders) {
  // Start by allocating memory for the restaurant.
  BENSCHILLIBOWL *restaurant = (BENSCHILLIBOWL*) malloc(sizeof(BENSCHILLIBOWL));
  restaurant->current_size = 0;
  restaurant->max_size = max_size;
  restaurant->orders = NULL;
  restaurant->next_order_number = 1;
  restaurant->orders_handled = 0;
  restaurant->expected_num_orders = expected_num_orders;
  // Then we create the mutex and the conditional variables.
  pthread_mutex_init(&(restaurant->mutex), NULL);
  pthread_cond_init(&(restaurant->can_add_orders), NULL);
  pthread_cond_init(&(restaurant->can_get_orders), NULL);
  printf("Restaurant is open!\n");
  return restaurant;
}


/* check that the number of orders received is equal to the number handled (ie.fullfilled). Remember to deallocate your resources */

void CloseRestaurant(BENSCHILLIBOWL* restaurant) {
  // Ensure that Num_of_orders recv is equal to the Num_handled
  if (restaurant->expected_num_orders != restaurant->orders_handled) {
    // resources should be deallocated
    pthread_mutex_destroy(&(restaurant->mutex));
    free(restaurant);
    // if all orders weren't handeled, throw error    
    fprintf(stderr, "Number of orders handled is not as expected.\n");
    exit(0);
  }
  // resources should be deallocated
  pthread_mutex_destroy(&(restaurant->mutex));
  free(restaurant);
  printf("Restaurant is closed!\n");
}

/* add an order to the back of queue */
int AddOrder(BENSCHILLIBOWL* restaurant, Order* order) {
  // before changes lock mutex
  pthread_mutex_lock(&(restaurant->mutex));
  // if queue is full, wait until order can be added 
  while (IsFull(restaurant)) {
    pthread_cond_wait(&(restaurant->can_add_orders), &(restaurant->mutex));
  }
  // to the back of the queue add the order 
  order->order_number = restaurant->next_order_number;
  AddOrderToBack(&(restaurant->orders), order);
  // update the size and next order number of the restaurant. 
  restaurant->next_order_number += 1;
  restaurant->current_size += 1;
  // Broadcast to the cond that orders are available.
  pthread_cond_broadcast(&(restaurant->can_get_orders));
  // Mutex unlocked
  pthread_mutex_unlock(&(restaurant->mutex));
  return order->order_number;
}

/* remove an order from the queue */
Order *GetOrder(BENSCHILLIBOWL* restaurant) {
  // Before changes lock the mutex
  pthread_mutex_lock(&(restaurant->mutex));
  // if queue is empty wait until orders are available.
  while (IsEmpty(restaurant)) {
    // Ensure orders_handeled is greater than expected orders
    // to prevent cooks getting stuck at empty queue.
    if (restaurant->orders_handled >= restaurant->expected_num_orders) {
      pthread_mutex_unlock(&(restaurant->mutex));
      return NULL;
    }
    pthread_cond_wait(&(restaurant->can_get_orders), &(restaurant->mutex));
  }
  // Fetch order from queue and update necessary info. 
  Order* order = restaurant->orders;
  restaurant->orders = restaurant->orders->next;
  restaurant->current_size -= 1;
  restaurant->orders_handled += 1;
  // Broadcast to the cond that orders can be added. 
  pthread_cond_broadcast(&(restaurant->can_add_orders));
  // Mutex unlocked
  pthread_mutex_unlock(&(restaurant->mutex));
  return order;
}


// Optional helper functions (you can implement if you think they would be useful)
bool IsEmpty(BENSCHILLIBOWL* restaurant) {
  if (restaurant->current_size > 0) {
    return false;
  } else {
    return true;
  }
}

bool IsFull(BENSCHILLIBOWL* restaurant) {
  if (restaurant->current_size == restaurant->max_size) {
    return true;
  } else {
    return false;
  }
}

/* this methods adds order to rear of queue */
void AddOrderToBack(Order **orders, Order *order) {
  if (*orders != NULL){
    Order* curr_order = *orders;
    while (curr_order->next){
      curr_order = curr_order->next;
    }
    curr_order->next = order;
  } else {
    *orders = order;
  }
}

