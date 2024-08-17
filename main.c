#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#define MAX_NAME 256
#define TABLE_SIZE 3000

typedef struct Order {
    char recipeName[MAX_NAME];
    int numberOfPieces;
    int arrivingTime;
    int weight;
    struct Order *next;
    struct Order *prev;
} Order;

typedef struct waitingOrdersQueue {
    Order *head;
    Order *tail;
} waitingOrderQueue;

typedef struct camionQueue {
    Order *head;
    Order *tail;
} camionQueue;

typedef struct Batch {
    int expirationDate;
    int quantity;
    bool isUsed;
    struct Batch *next;
    struct Batch *prev;
} Batch;

typedef struct warehouseIngredient {
    char name[MAX_NAME];
    Batch *head;
    Batch *tail;
    struct warehouseIngredient *next;
} warehouseIngredient;

warehouseIngredient *warehouse[TABLE_SIZE];

typedef struct Ingredient {
    char ingredientName [MAX_NAME];
    int ingredientQuantity;
    struct Ingredient *next;
    struct Ingredient *prev;
} Ingredient;

typedef struct Recipe {
    char recipeName[MAX_NAME];
    Ingredient *head;
    Ingredient *tail;
    struct Recipe *next;
} Recipe;

Recipe *cookbook[TABLE_SIZE];

unsigned int hash(char *inputString) { //hash function
    if (inputString == NULL) {
        printf("Hash warning!\n");
        return 1;
    }
    unsigned long hash = 5381;
    int c;
    while ((c = *inputString++)) {
        hash = (hash << 5) + hash + c;
    }
    unsigned int hash_value = hash % 2000;
    return hash_value;
}

void init_hash_tables() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        cookbook[i] = NULL;
        warehouse[i] = NULL;
    }
}

Recipe *hash_table_cookbook_lookup(char *name) { //function to search elements inside the cookbook hash table
    int index = hash(name);
    Recipe *tmp = cookbook[index];
    while (tmp != NULL && strncmp(tmp->recipeName, name, MAX_NAME) != 0) {
        tmp = tmp->next;
    }
    return tmp;
}

warehouseIngredient *hash_table_warehouse_lookup(char *name) { //function to search elements inside the warehouse hash table (they could be merged)
    int index = hash(name);
    warehouseIngredient *tmp = warehouse[index];
    while (tmp != NULL && strncmp(tmp->name, name, MAX_NAME) != 0) {
        tmp = tmp->next;
    }
    return tmp;
}

Ingredient *create_new_ingredient(char *name, int quantity) {
    Ingredient *newIngredient = malloc(sizeof(Ingredient));
    assert (newIngredient != NULL);
    strcpy(newIngredient->ingredientName, name);
    newIngredient->ingredientQuantity = quantity;
    newIngredient->next = NULL;
    newIngredient->prev = NULL;
    return newIngredient;
}

Batch *create_new_batch(int quantity, int expirationDate) {
    Batch *newBatch = malloc(sizeof(Batch));
    assert (newBatch != NULL);
    newBatch->quantity = quantity;
    newBatch->expirationDate = expirationDate;
    newBatch->isUsed = false;
    newBatch->next = NULL;
    newBatch->prev = NULL;
    return newBatch;
}

int calculate_recipe_weight(Recipe *recipe) {
    int weight = 0;
    Ingredient *current = recipe->head;
    while (current != NULL) {
        weight += current->ingredientQuantity;
        current = current->next;
    }
    return weight;
}

Order *create_new_order(char *recipeName, int quantity, int arrivingTime) {
    Order *newOrder = malloc(sizeof(Order));
    assert (newOrder != NULL);
    strcpy(newOrder->recipeName, recipeName);

    newOrder->numberOfPieces = quantity;
    newOrder->weight = calculate_recipe_weight(hash_table_cookbook_lookup(recipeName)) * quantity;
    newOrder->arrivingTime = arrivingTime;
    newOrder->next = NULL;
    newOrder->prev = NULL;
    return newOrder;
}

waitingOrderQueue *create_new_waiting_orders_queue() {
    waitingOrderQueue *newWaitingOrdersQueue = malloc(sizeof(waitingOrderQueue));
    assert (newWaitingOrdersQueue != NULL);
    newWaitingOrdersQueue->head = NULL;
    newWaitingOrdersQueue->tail = NULL;
    return newWaitingOrdersQueue;
}

camionQueue *create_new_camion_queue() { //i need to find the reason why this function creates a memory leak
    camionQueue *newCamionQueue = malloc(sizeof(camionQueue));
    assert (newCamionQueue != NULL);
    newCamionQueue->head = NULL;
    newCamionQueue->tail = NULL;
    return newCamionQueue;
}

void print_waiting_orders_queue(waitingOrderQueue *waitingQueue) {
    assert (waitingQueue != NULL);
    Order *current = waitingQueue->head;
    if (current == NULL) {
        return;
    }
    printf("%s", "ORDINI IN ATTESA: \n");
    while (current != NULL) {
        printf(" %d %s %d con peso %d\n ", current->arrivingTime, current->recipeName, current->numberOfPieces, current->weight);
        current = current->next;
    }
    printf("\n");
}

void waiting_orders_queue_sorted_insert(waitingOrderQueue *waitingQueue, char *recipeName, int quantity, int arrivingTime) {
    Order *newOrder = create_new_order(recipeName, quantity, arrivingTime);
    if (waitingQueue->tail == NULL) { // empty list
        waitingQueue->head = newOrder;
        waitingQueue->tail = newOrder;
    }
    else {
        Order *current = waitingQueue->head;
        while (current != NULL && current->arrivingTime < arrivingTime) { // check if the arriving time is less than the current order
            current = current->next;
        }
        if (current == NULL) { // insert at tail
            waitingQueue->tail->next = newOrder;
            newOrder->prev = waitingQueue->tail;
            waitingQueue->tail = newOrder;
        }
        else if (current->prev == NULL) { // insert at head
            newOrder->next = waitingQueue->head;
            waitingQueue->head->prev = newOrder;
            waitingQueue->head = newOrder;
        }
        else { // insert in the middle
            newOrder->next = current;
            newOrder->prev = current->prev;
            current->prev->next = newOrder;
            current->prev = newOrder;
        }
    }
}

void print_camion_queue(camionQueue *camion_queue) {
    assert (camion_queue != NULL);
    Order *current = camion_queue->head;
    assert (current != NULL);
    printf("%s", "ORDINI PRONTI: \n");
    while (current != NULL) {
        printf(" %d %s %d con peso %d\n ", current->arrivingTime, current->recipeName, current->numberOfPieces, current->weight);
        current = current->next;
    }
    printf("\n");
}

void camion_queue_sorted_insert(camionQueue *camion_queue, char *recipeName, int quantity, int arrivingTime) {
    Order *newOrder = create_new_order(recipeName, quantity, arrivingTime);
    if (camion_queue->tail == NULL) { // empty list
        camion_queue->head = newOrder;
        camion_queue->tail = newOrder;
    }
    else {
        Order *current = camion_queue->head;
        while (current != NULL && current->arrivingTime < arrivingTime) { // check if the arriving time is less than the current order
            current = current->next;
        }
        if (current == NULL) { // insert at tail
            camion_queue->tail->next = newOrder;
            newOrder->prev = camion_queue->tail;
            camion_queue->tail = newOrder;
        }
        else if (current->prev == NULL) { // insert at head
            newOrder->next = camion_queue->head;
            camion_queue->head->prev = newOrder;
            camion_queue->head = newOrder;
        }
        else { // insert in the middle
            newOrder->next = current;
            newOrder->prev = current->prev;
            current->prev->next = newOrder;
            current->prev = newOrder;
        }
    }
}

void camion_queue_sorted_loading_insert(camionQueue *camion_queue, char *recipeName, int quantity, int arrivingTime) {
    Order *newOrder = create_new_order(recipeName, quantity, arrivingTime);
    if (camion_queue->head == NULL) { // empty list
        camion_queue->head = newOrder;
        camion_queue->tail = newOrder;
    }
    else {
        Order *current = camion_queue->head;
        while (current != NULL && current->weight >= newOrder->weight) { // check if the weight is less than the current order
            current = current->next;
        }
        if (current == NULL) { // insert at tail
            camion_queue->tail->next = newOrder;
            newOrder->prev = camion_queue->tail;
            camion_queue->tail = newOrder;
        }
        else if (current->prev == NULL) { //insert at head
            newOrder->next = camion_queue->head;
            camion_queue->head->prev = newOrder;
            camion_queue->head = newOrder;
        }
        else { // insert in the middle
            newOrder->next = current;
            newOrder->prev = current->prev;
            current->prev->next = newOrder;
            current->prev = newOrder;
        }
    }
}

void recipe_insert_at_tail(Recipe *recipe, char *name, int quantity) {
    Ingredient *newIngredient = create_new_ingredient(name, quantity);
    if (recipe->tail == NULL) { // empty list
        recipe->head = newIngredient;
        recipe->tail = newIngredient;
    }
    else { //not empty list
        recipe->tail->next = newIngredient;
        newIngredient->prev = recipe->tail;
        recipe->tail = newIngredient;
    }
}

void print_list_of_ingredients_from_head(Recipe *recipe) {
    Ingredient* current = recipe->head;
    while (current != NULL) {
        printf("%s %d - ", current->ingredientName, current->ingredientQuantity);
        current = current->next;
    }
}

Order *waiting_orders_queue_lookup(waitingOrderQueue *waitingQueue, char *name) {
    Order *current = waitingQueue->head;
    while (current != NULL && strcmp(current->recipeName, name) != 0) {
        current = current->next;
    }
    return current;
}

Order *camion_queue_lookup(camionQueue *camion_queue, char *name) {
    Order *current = camion_queue->head;
    while (current != NULL && strcmp(current->recipeName, name) != 0) {
        current = current->next;
    }
    return current;
}

void hash_table_insert(char name[]) { //i need to find the reason why this function creates a memory leak
    Recipe *newRecipe = malloc(sizeof(Recipe));
    assert (newRecipe != NULL);
    newRecipe->head = NULL;
    newRecipe->tail = NULL;
    strcpy(newRecipe->recipeName, name);

    int index = hash(newRecipe->recipeName);
    newRecipe->next = cookbook[index];
    cookbook[index] = newRecipe;
}

char *remove_recipe(char name[], waitingOrderQueue *waitingQueue, camionQueue *camion_queue) {
    int index = hash(name);
    Recipe *current = cookbook[index];
    Recipe *prev = NULL;
    while (current != NULL && strcmp(current->recipeName, name) != 0) {
        prev = current;
        current = current->next;
    }
    if (current == NULL) { // element not found
        return "non presente\n";
    }
    else if (waiting_orders_queue_lookup(waitingQueue, name) != NULL || camion_queue_lookup(camion_queue, name) != NULL) { // element found but in use
        return "ordini in sospeso\n";
    }
    // Element found, not in use
    if (prev == NULL) { // deleating the head
        cookbook[index] = current->next;
    }
    else {
        prev->next = current->next;
    }

    Ingredient *currentIngredient = current->head;
    while (currentIngredient != NULL) {
        Ingredient *tmp = currentIngredient;
        currentIngredient = currentIngredient->next;
        free(tmp); //free the memory of all the ingredients
    }

    free(current); // free the memory reserved for the element
    return "rimossa\n";
}

void warehouse_sorted_insert(warehouseIngredient *ingredient, int quantity, int expirationDate) {
    Batch *newBatch = create_new_batch(quantity, expirationDate);
    if (ingredient->tail == NULL) { // empty list
        ingredient->head = newBatch;
        ingredient->tail = newBatch;
    }
    else {
        Batch *current = ingredient->head;
        while (current != NULL && current->expirationDate <= expirationDate) { // check if the expiration date is less than the current batch
            current = current->next;
        }
        if (current == NULL) { // insert at tail
            ingredient->tail->next = newBatch;
            newBatch->prev = ingredient->tail;
            ingredient->tail = newBatch;
        }
        else if (current->expirationDate == expirationDate) { // insert in the same day updating the current quantity
            current->quantity += quantity;
            free(newBatch);
        }
        else if (current->prev == NULL) { // insert at head
            newBatch->next = ingredient->head;
            ingredient->head->prev = newBatch;
            ingredient->head = newBatch;
        }
        else { // insert in the middle
            newBatch->next = current;
            newBatch->prev = current->prev;
            current->prev->next = newBatch;
            current->prev = newBatch;
        }
    }
}

void hash_table_warehouse_insert(char name[], int quantity, int expirationDate) {
    int index = hash(name);

    if (hash_table_warehouse_lookup(name) == NULL) { // ingredient not found, create a new one
        warehouseIngredient *newWarehouseIngredient = malloc(sizeof(warehouseIngredient));
        assert (newWarehouseIngredient != NULL);
        strcpy(newWarehouseIngredient->name, name);
        newWarehouseIngredient->head = NULL;
        newWarehouseIngredient->tail = NULL;
        newWarehouseIngredient->next = warehouse[index];
        warehouse[index] = newWarehouseIngredient;
        warehouse_sorted_insert(newWarehouseIngredient, quantity, expirationDate);
    }
    else { // ingredient found, insert the batch
        warehouseIngredient *current = hash_table_warehouse_lookup(name);
        warehouse_sorted_insert(current, quantity, expirationDate);
    }
}

void check_expiration_date(int time) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        if (warehouse[i] != NULL) {
            warehouseIngredient *current = warehouse[i];
            warehouseIngredient *prev = NULL;
            while (current != NULL) {
                Batch *tmp = current->head;
                while (tmp != NULL) {
                    Batch *next = tmp->next; // save the next batch before deleting the current one
                    if (tmp->expirationDate <= time) {
                        if (tmp->prev == NULL) { // delete head
                            current->head = tmp->next;
                            if (tmp->next != NULL) {
                                tmp->next->prev = NULL;
                            }
                            else {
                                current->tail = NULL; // list becomes empty
                            }
                        }
                        else if (tmp->next == NULL) { // delete tail
                            current->tail = tmp->prev;
                            tmp->prev->next = NULL;
                        }
                        else { // delete in the middle
                            tmp->prev->next = tmp->next;
                            tmp->next->prev = tmp->prev;
                        }
                        free(tmp);
                    }
                    tmp = next; // Move to the next batch
                }
                if (current->head == NULL) {
                    warehouseIngredient *toFree = current; // store current ingredient to free
                    if (prev == NULL) { // ingredient is the first in the list
                        warehouse[i] = current->next;
                    } else {
                        prev->next = current->next;
                    }
                    current = current->next; // move to the next ingredient
                    free(toFree); // Free memory of the ingredient
                }
                else {
                    prev = current;
                    current = current->next;
                }
            }
        }
    }
}

int check_orders_executability(Order *order) {
    Recipe *recipe = hash_table_cookbook_lookup(order->recipeName);
    if (recipe == NULL) {
        printf("%s", "rifiutato\n");
        return -1;
    }
    Ingredient *current = recipe->head;
    while (current != NULL) {
        warehouseIngredient *ingredient = warehouse[hash(current->ingredientName)];
        if (ingredient == NULL) {
            return 0; // missing ingredient
        }
        int neededQuantity = current->ingredientQuantity * order->numberOfPieces;
        Batch *batch = ingredient->head;
        while (batch != NULL && neededQuantity > 0) { // iterate through batches and check availability
            if (batch->quantity >= neededQuantity) {
                neededQuantity -= batch->quantity;
                break;
            }
            neededQuantity -= batch->quantity;
            batch = batch->next;
        }
        if (neededQuantity > 0) {
            return 0; // not enough ingredients
        }
        current = current->next;
    }
    return 1;
}

void execute_order(Order *order) {
    Recipe *recipe = hash_table_cookbook_lookup(order->recipeName);
    if (recipe == NULL) {
        printf("Recipe not found\n");
        return;
    }
    Ingredient *current = recipe->head;
    if (current == NULL) {
        printf("No ingredients in recipe\n");
        return;
    }
    warehouseIngredient *ingredient = warehouse[hash(current->ingredientName)];
    if (ingredient == NULL) {
        printf("Ingredient not found in warehouse\n");
        return;
    }
    Batch *batch = ingredient->head;
    while (current != NULL) {
        int neededQuantity = current->ingredientQuantity * order->numberOfPieces;
        while (batch != NULL && neededQuantity > 0) {
                int ingredientReduction = batch->quantity;
                batch->quantity -= neededQuantity;
                neededQuantity -= ingredientReduction;
                Batch *temp = batch;
                if (batch->quantity <= 0) {
                    if (batch->prev == NULL) { // delete head
                        ingredient->head = batch->next;
                        if (batch->next != NULL) {
                            batch->next->prev = NULL;
                        }
                        else {
                            ingredient->tail = NULL; // list becomes empty
                        }
                    }
                    else if (batch->next == NULL) { // delete tail
                        ingredient->tail = batch->prev;
                        batch->prev->next = NULL;
                    }
                    else { // delete in the middle
                        batch->prev->next = batch->next;
                        batch->next->prev = batch->prev;
                    }
                    batch = batch->next;
                    free(temp);
                }
        }
        current = current->next;
        if (current != NULL) {
            ingredient = warehouse[hash(current->ingredientName)];
            if (ingredient == NULL) {
                printf("Ingredient not found in warehouse\n");
                return;
            }
            batch = ingredient->head;
        }
    }
}

void process_order(Order *order, waitingOrderQueue *waitingQueue, camionQueue *camion_queue) {
    if (check_orders_executability(order) == 1) {
        execute_order(order);
        camion_queue_sorted_insert(camion_queue, order->recipeName, order->numberOfPieces, order->arrivingTime);
    }
    else {
        waiting_orders_queue_sorted_insert(waitingQueue, order->recipeName, order->numberOfPieces, order->arrivingTime);
    }
}

void process_waiting_orders(waitingOrderQueue *waitingQueue, camionQueue *camion_queue) {
    Order *current = waitingQueue->head;
    while (current != NULL) {
        Order *next = current->next; // save the next order before possibly freeing current
        if (check_orders_executability(current) == 1) {
            execute_order(current);
            camion_queue_sorted_insert(camion_queue, current->recipeName, current->numberOfPieces, current->arrivingTime);

            // remove the processed order from the waiting queue
            if (current->prev == NULL) {// delete head
                waitingQueue->head = current->next;
                if (current->next != NULL) {
                    current->next->prev = NULL;
                }
                else {
                    waitingQueue->tail = NULL;// list becomes empty
                }
            }
            else if (current->next == NULL) { // delete tail
                waitingQueue->tail = current->prev;
                current->prev->next = NULL;
            }
            else { // delete in the middle
                current->prev->next = current->next;
                current->next->prev = current->prev;
            }

            free(current); // free the memory for the processed order
        }
        current = next; // move to the next order
    }
}

camionQueue *select_orders_to_load(camionQueue *camion_queue, int camionCapacity) {
    camionQueue *selectedOrders = create_new_camion_queue();
    assert(camion_queue != NULL);
    Order *current = camion_queue->head;
    while (current != NULL && current->weight <= camionCapacity) {
        camionCapacity -= current->weight;
        Order *next = current->next;
        camion_queue_sorted_loading_insert(selectedOrders, current->recipeName, current->numberOfPieces, current->arrivingTime);
        // remove current order from camionQueue
        if (current->prev == NULL) { // delete head
            camion_queue->head = current->next;
            if (current->next != NULL) {
                current->next->prev = NULL;
            }
            else {
                camion_queue->tail = NULL; // list becomes empty
            }
        }
        else if (current->next == NULL) { // delete tail
            camion_queue->tail = current->prev;
            current->prev->next = NULL;
        }
        else { // delete in the middle
            current->prev->next = current->next;
            current->next->prev = current->prev;
        }
        free(current);// free the memory for the processed order
        current = next;
    }
    return selectedOrders;
}

void load_camion(camionQueue *queue, int camionCapacity) {
    camionQueue *loadingOrders = select_orders_to_load(queue, camionCapacity);

    if (loadingOrders->head == NULL) {
        printf("%s", "camioncino vuoto\n");
    }
    else {
        Order *current = loadingOrders->head;
        while (current != NULL) {
            printf("%d %s %d\n", current->arrivingTime, current->recipeName, current->numberOfPieces);
            current = current->next;
        }
    }
    // free the selectedOrders queue
    if (loadingOrders->head != NULL) {
        Order *current = loadingOrders->head;
        while (current != NULL) {
            Order *next = current->next;
            free(current);
            current = next;
        }
        free(loadingOrders);
    }
}

void free_queues(camionQueue *camionQueue, waitingOrderQueue *waitingOrderQueue) {
    Order *current = camionQueue->head;
    while (current != NULL) {
        Order *tmp = current;
        current = current->next;
        free(tmp);
    }
    free(camionQueue);

    current = waitingOrderQueue->head;
    while (current != NULL) {
        Order *tmp = current;
        current = current->next;
        free(tmp);
    }
    free(waitingOrderQueue);
}

void free_hash_tables() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        if (cookbook[i] != NULL) {
            Recipe *current = cookbook[i];
            while (current != NULL) {
                Recipe *tmp = current;
                current = current->next;
                Ingredient *currentIngredient = tmp->head;
                while (currentIngredient != NULL) {
                    Ingredient *tmpIngredient = currentIngredient;
                    currentIngredient = currentIngredient->next;
                    free(tmpIngredient);
                }
                free(tmp);
            }
        }
        if (warehouse[i] != NULL) {
            warehouseIngredient *current = warehouse[i];
            while (current != NULL) {
                warehouseIngredient *tmp = current;
                current = current->next;
                Batch *currentBatch = tmp->head;
                while (currentBatch != NULL) {
                    Batch *tmpBatch = currentBatch;
                    currentBatch = currentBatch->next;
                    free(tmpBatch);
                }
                free(tmp);
            }
        }
    }
}

int main() {
    init_hash_tables();
    waitingOrderQueue *waitingOrderQueue = create_new_waiting_orders_queue();
    camionQueue *camionQueue = create_new_camion_queue();
    int time=0;
    int camionCapacity=0;
    int refillFrequency=0;
    int quantity = 0;
    int expirationDate = 0;
    int numOfPieces = 0;
    char command[MAX_NAME];

    if (!scanf("%d %d", &refillFrequency, &camionCapacity)) {
        printf ("Failed getting refillFrequency and camionCapacity\n");
        return 1;
    }
    while (scanf("%s", command) != EOF) {
        if (time != 0 && time % refillFrequency == 0) {
            load_camion(camionQueue, camionCapacity);
        }
        if (strcmp(command, "aggiungi_ricetta") == 0) {
            char recipeName[MAX_NAME];
            if (!scanf("%s", recipeName)) {
                printf ("Failed getting recipeName\n");
                return 1;
            }
            Recipe *newRecipe = hash_table_cookbook_lookup(recipeName);
            if (newRecipe == NULL) {
                hash_table_insert(recipeName);
                newRecipe = hash_table_cookbook_lookup(recipeName);
            }
            else {
                printf("ignorato\n");
                while (getchar_unlocked() != '\n');
                time++;
                check_expiration_date(time);
                continue;
            }
            while (getchar() != '\n'){
                char ingredient[MAX_NAME];
                if (!scanf("%s %d", ingredient, &quantity)) {
                    printf ("Failed getting ingredient and quantity\n");
                    return 1;
                }
                recipe_insert_at_tail(newRecipe, ingredient, quantity);
            }
            printf("aggiunta\n");
        }
        else if (strcmp(command, "rimuovi_ricetta") == 0) {
            char recipeName[MAX_NAME];
            if (!scanf("%s", recipeName)) {
                printf ("Failed getting recipeName\n");
                return 1;
            }
            printf("%s", remove_recipe(recipeName, waitingOrderQueue, camionQueue)); // change this
        }
        else if (strcmp(command, "rifornimento") == 0) {
            char name[MAX_NAME];
            while (getchar_unlocked() != '\n'){
            if (!scanf("%s %d %d", name, &quantity, &expirationDate)) {
                printf ("Failed getting ingredientName, quantity and expirationDate\n");
                return 1;
            }
                hash_table_warehouse_insert(name, quantity, expirationDate);
            }
            process_waiting_orders(waitingOrderQueue, camionQueue);
            printf("%s", "rifornito\n");
        }
        else if (strcmp(command, "ordine") == 0) {
            char name[MAX_NAME];
            if (!scanf("%s %d", name, &numOfPieces)) {
                printf("Failed getting orderName and numOfPieces\n");
                return 1;
            }
            if (hash_table_cookbook_lookup(name) == NULL) {
                printf("%s", "rifiutato\n");
                time++;
                check_expiration_date(time);
                continue;
            }
            Order *newOrder = create_new_order(name, numOfPieces, time);
            process_order(newOrder, waitingOrderQueue, camionQueue);
            printf("accettato\n");
        }
        time++;
        check_expiration_date(time);
    }
    if (time != 0 && time % refillFrequency == 0) {
    load_camion(camionQueue, camionCapacity);
}

    free_hash_tables();
    free_queues(camionQueue, waitingOrderQueue);
    return 0;
}