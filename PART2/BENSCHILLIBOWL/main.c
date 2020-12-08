#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "BENSCHILLIBOWL.h"

// Feel free to play with these numbers! This is a great way to
// test your implementation.
#define BENSCHILLIBOWL_SIZE 4
#define NUM_CUSTOMERS 6
#define NUM_COOKS 3
#define ORDERS_PER_CUSTOMER 3
#define EXPECTED_NUM_ORDERS NUM_CUSTOMERS * ORDERS_PER_CUSTOMER

// Global variable for the restaurant.
BENSCHILLIBOWL *bcb;

/**
 * Thread funtion that represents a customer. A customer should:
 *  - allocate space (memory) for an order.
 *  - select a menu item.
 *  - populate the order with their menu item and their customer ID.
 *  - add their order to the restaurant.
 */
void* BENSCHILLIBOWLCustomer(void* tid) {
  int customerId = (int)(long) tid;
  
  for (int i = 0; i < ORDERS_PER_CUSTOMER; i++) {
    Order* order = (Order*) malloc(sizeof(Order));
    MenuItem menu_item = PickRandomMenuItem();
    
    order->customer_id = customerId;
    order->menu_item = menu_item;
    order->next = NULL;
    int orderNumber = AddOrder(bcb, order);
    printf("Order #%d added by Customer #%d.\n", orderNumber, customerId);
  }
  return NULL;
}

/**
 * Thread function that represents a cook in the restaurant. A cook should:
 *  - get an order from the restaurant.
 *  - if the order is valid, it should fulfill the order, and then
 *    free the space taken by the order.
 * The cook should take orders from the restaurants until it does not
 * receive an order.
 */
void* BENSCHILLIBOWLCook(void* tid) {
  int cookId = (int)(long) tid;
	int ordersFulfilled = 0;
  
  Order* order = GetOrder(bcb);
  // Keep getting orders until no more is left.
  while (order != NULL) {
    free(order);
    ordersFulfilled += 1;
    order = GetOrder(bcb);
  }
  
	printf("%d orders made by Cook #%d\n", ordersFulfilled, cookId);
	return NULL;
}

/**
 * Runs when the program begins executing. This program should:
 *  - open the restaurant
 *  - create customers and cooks
 *  - wait for all customers and cooks to be done
 *  - close the restaurant.
 */
int main() {
  // restaurant needs to be open
  bcb = OpenRestaurant(BENSCHILLIBOWL_SIZE, EXPECTED_NUM_ORDERS);
  
  // initialize cooks and customers
  pthread_t cooks[NUM_COOKS];
  pthread_t customers[NUM_CUSTOMERS];
  
  int cookIds[NUM_COOKS];
  int customer_ids[NUM_CUSTOMERS];
  
  for (int j = 0; j < NUM_COOKS; j++) {
    cookIds[j] = j+1;
    pthread_create(&(cooks[j]), NULL, BENSCHILLIBOWLCook, &(cookIds[j]));
  }

  for (int i = 0; i < NUM_CUSTOMERS; i++) {
    customer_ids[i] = i+1;
    pthread_create(&(customers[i]), NULL, BENSCHILLIBOWLCustomer, &(customer_ids[i]));
  }
  
  // Need to let everyone finish. Wait until everyone is done.
  for (int i = 0; i < NUM_CUSTOMERS; i++) {
    printf("Waiting for customer %d\n", i+1);
    pthread_join(customers[i], NULL);
  }
  
  for (int j = 0; j < NUM_COOKS; j++) {
    printf("Waiting for cook %d\n", j+1);
    pthread_join(cooks[j], NULL);
  }
  
  // Close restaurant. 
  CloseRestaurant(bcb);
  
  return 0;
}