#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#define MAX_NAME 256
#define TABLE_SIZE 2000
#define MAX_LINE_LENGHT 512

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

unsigned int hash(const char *inputString) {//hash function
    if (inputString == NULL) {
        fprintf(stderr, "Error: inputString is NULL\n");
        exit(EXIT_FAILURE);
    }
    unsigned long hash = 5381;
    int c;
    while ((c = *inputString++)) {
        hash = (hash << 5) + hash + c;
    }
    const unsigned int hash_value = hash % 2000;
    return hash_value;
}

void init_hash_tables() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        cookbook[i] = NULL;
        warehouse[i] = NULL;
    }
}

Recipe *hash_table_lookup(const char *name) {//function to search elements inside the hash table
    const int index = hash(name);
    Recipe *tmp = cookbook[index];
    while (tmp != NULL && strncmp(tmp->recipeName, name, MAX_NAME) != 0) {
        tmp = tmp->next;
    }
    return tmp;
}

warehouseIngredient *hash_table_warehouse_lookup(const char *name) {//function to search elements inside the hash table
    const int index = hash(name);
    warehouseIngredient *tmp = warehouse[index];
    while (tmp != NULL && strncmp(tmp->name, name, MAX_NAME) != 0) {
        tmp = tmp->next;
    }
    return tmp;
}

Ingredient *create_new_ingredient(const char *name, int quantity) {
    Ingredient *newIngredient = malloc(sizeof(Ingredient));
    if (newIngredient == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    strcpy(newIngredient->ingredientName, name);
    newIngredient->ingredientQuantity = quantity;
    newIngredient->next = NULL;
    newIngredient->prev = NULL;
    return newIngredient;
}

Batch *create_new_batch(int quantity, int expirationDate) {
    Batch *newBatch = (Batch *)malloc(sizeof(Batch));
    if (newBatch == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    newBatch->quantity = quantity;
    newBatch->expirationDate = expirationDate;
    newBatch->isUsed = false;
    newBatch->next = NULL;
    newBatch->prev = NULL;
    return newBatch;
}

int calculate_recipe_weight(const Recipe *recipe) {
    int weight = 0;
    const Ingredient *current = recipe->head;
    while (current != NULL) {
        weight += current->ingredientQuantity;
        current = current->next;
    }
    return weight;
}

Order *create_new_order(const char *recipeName, const int quantity, const int arrivingTime) {
    Order *newOrder = (Order *)malloc(sizeof(Order));
    if (newOrder == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    strcpy(newOrder->recipeName, recipeName);

    newOrder->numberOfPieces = quantity;
    newOrder->weight = calculate_recipe_weight(hash_table_lookup(recipeName)) * quantity;
    newOrder->arrivingTime = arrivingTime;
    newOrder->next = NULL;
    newOrder->prev = NULL;
    return newOrder;
}

waitingOrderQueue *create_new_waiting_orders_queue() {
    waitingOrderQueue *newWaitingOrdersQueue = (waitingOrderQueue *)malloc(sizeof(waitingOrderQueue));
    if (newWaitingOrdersQueue == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    newWaitingOrdersQueue->head = NULL;
    newWaitingOrdersQueue->tail = NULL;
    return newWaitingOrdersQueue;
}

camionQueue *create_new_camion_queue() {
    camionQueue *newCamionQueue = (camionQueue *)malloc(sizeof(camionQueue));
    if (newCamionQueue == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    newCamionQueue->head = NULL;
    newCamionQueue->tail = NULL;
    return newCamionQueue;
}

void waiting_orders_queue_insert(waitingOrderQueue *queue, const char *recipeName, const int quantity, const int arrivingTime) {
    Order *newOrder = create_new_order(recipeName, quantity, arrivingTime);
    if (queue->tail == NULL) { // empty list
        queue->head = newOrder;
        queue->tail = newOrder;
    }
    else {
        queue->tail->next = newOrder;
        newOrder->prev = queue->tail;
        queue->tail = newOrder;
    }
}

void camion_queue_sorted_insert(camionQueue *queue, const char *recipeName, const int quantity, const int arrivingTime) {
    Order *newOrder = create_new_order(recipeName, quantity, arrivingTime);
    if (queue->tail == NULL) {// empty list
        queue->head = newOrder;
        queue->tail = newOrder;
    }
    else {
        Order *current = queue->head;
        while (current != NULL && current->arrivingTime < arrivingTime) {// check if the arriving time is less than the current order
            current = current->next;
        }
        if (current == NULL) {// insert at tail
            queue->tail->next = newOrder;
            newOrder->prev = queue->tail;
            queue->tail = newOrder;
        }
        else if (current->prev == NULL) {// insert at head
            newOrder->next = queue->head;
            queue->head->prev = newOrder;
            queue->head = newOrder;
        }
        else {// insert in the middle
            newOrder->next = current;
            newOrder->prev = current->prev;
            current->prev->next = newOrder;
            current->prev = newOrder;
        }
    }
}


void camion_queue_sorted_loading_insert(camionQueue *queue, const char *recipeName, const int quantity, const int arrivingTime) {
    Order *newOrder = create_new_order(recipeName, quantity, arrivingTime);
    if (queue->head == NULL) {// empty list
        queue->head = newOrder;
        queue->tail = newOrder;
    }
    else {
        Order *current = queue->head;
        while (current != NULL && current->weight >= newOrder->weight) {// check if the weight is less than the current order
            current = current->next;
        }
        if (current == NULL) {// insert at tail
            queue->tail->next = newOrder;
            newOrder->prev = queue->tail;
            queue->tail = newOrder;
        }
        else if (current->prev == NULL) {//insert at head
            newOrder->next = queue->head;
            queue->head->prev = newOrder;
            queue->head = newOrder;
        }
        else {// insert at middle
            newOrder->next = current;
            newOrder->prev = current->prev;
            current->prev->next = newOrder;
            current->prev = newOrder;
        }
    }
}

void recipe_insert_at_tail(Recipe *recipe, const char *name, const int quantity) {
    //printf("Debug: Adding ingredient %s with quantity %d to recipe %s\n", name, quantity, recipe->recipeName); // Debug print
    Ingredient *newIngredient = create_new_ingredient(name, quantity);
    if (recipe->tail == NULL) { // empty list
        //printf("Debug: Recipe is empty, adding as first ingredient\n"); // Debug print
        recipe->head = newIngredient;
        recipe->tail = newIngredient;
    }
    else { //not empty list, adjustment needed
        //printf("Debug: Recipe is not empty, adding to the tail\n"); // Debug print
        recipe->tail->next = newIngredient; //the new ingredient is actually the next of the previous tail
        newIngredient->prev = recipe->tail; //the prev of the new ingredient is the previous tail
        recipe->tail = newIngredient; //the recipe's tail is the new ingredient
    }
    //printf("Debug: Ingredient %s added successfully\n", name); // Debug print
}

void print_list_of_ingredients_from_head(const Recipe *recipe) {//despite tail insert print starting from the head
    Ingredient* current = recipe->head;
    while (current != NULL) {
        printf("%s %d - ", current->ingredientName, current->ingredientQuantity);
        current = current->next;
    }
}

Order *waiting_orders_queue_lookup(const waitingOrderQueue *queue, const char *name) {
    Order *current = queue->head;
    while (current != NULL && strcmp(current->recipeName, name) != 0) {
        current = current->next;
    }
    return current;
}

Order *camion_queue_lookup(const camionQueue *queue, const char *name) {
    Order *current = queue->head;
    while (current != NULL && strcmp(current->recipeName, name) != 0) {
        current = current->next;
    }
    return current;
}

char *hash_table_insert(char name[]) {
    Recipe *newRecipe = malloc(sizeof(Recipe));
    if (newRecipe == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    newRecipe->head = NULL;
    newRecipe->tail = NULL;
    strcpy(newRecipe->recipeName, name);

    int index = hash(newRecipe->recipeName);
    newRecipe->next = cookbook[index];
    cookbook[index] = newRecipe;

    return "aggiunta\n";
}

void print_cookbook() { //support function to see the entire cookbook
    printf("Start\n");
    for (int i = 0; i < TABLE_SIZE; i++) {
        if (cookbook[i] == NULL) {
            printf("\t%i\t---\n", i);
        } else {
            printf("\t%i\t", i);
            Recipe *tmp = cookbook[i];
            while (tmp != NULL) {
                printf("%s ", tmp->recipeName);
                print_list_of_ingredients_from_head(tmp);
                tmp = tmp->next;
            }
            printf("\n");
        }
    }
    printf("End\n");
}

char *remove_recipe(char name[], const waitingOrderQueue *waitingOrderQueue, const camionQueue *camionQueue) {
    int index = hash(name);
    Recipe *current = cookbook[index];
    Recipe *prev = NULL;
    while (current != NULL && strcmp(current->recipeName, name) != 0) {
        prev = current;
        current = current->next;
    }
    // Element not found
    if (current == NULL) {
        return "non presente\n";
    }
    // Element found but in use
    if (waiting_orders_queue_lookup(waitingOrderQueue, name) != NULL || camion_queue_lookup(camionQueue, name) != NULL) {
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

void print_list_of_batches_from_head(warehouseIngredient *ingredient) {
    if (ingredient == NULL) {
        printf("Ingredient is NULL\n");
        return;
    }
    const Batch *current = ingredient->head;
    if (current == NULL) {
        return;
    }
    while (current != NULL) {
        printf("%s %d %d - ", ingredient->name, current->quantity, current->expirationDate);
        current = current->next;
    }
    printf("\n");
}

void print_warehouse() {
    printf("Start\n");
    for (int i = 0; i < TABLE_SIZE; i++) {
        if (warehouse[i] == NULL) {
            printf("\t%i\t---\n", i);
        } else {
            printf("\t%i\t", i);
            warehouseIngredient *tmp = warehouse[i];
            while (tmp != NULL) {
                printf("%s - ", tmp->name);
                print_list_of_batches_from_head(tmp);
                tmp = tmp->next;
            }
            printf("\n");
        }
    }
    printf("End\n");
}

void warehouse_sorted_insert(warehouseIngredient *ingredient, const int quantity, const int expirationDate) {
    Batch *newBatch = create_new_batch(quantity, expirationDate);
    if (ingredient->tail == NULL) { // empty list
        ingredient->head = newBatch;
        ingredient->tail = newBatch;
    }
    else {
        Batch *current = ingredient->head;
        while (current != NULL && current->expirationDate < expirationDate) { // check if the expiration date is less than the current batch
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

void hash_table_warehouse_insert(char name[], const int quantity, const int expirationDate) {
    int index = hash(name);

    if (hash_table_warehouse_lookup(name) == NULL) {// Ingredient not found, create a new one
        warehouseIngredient *newWarehouseIngredient = (warehouseIngredient *)malloc(sizeof(warehouseIngredient));
        if (newWarehouseIngredient == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }
        strcpy(newWarehouseIngredient->name, name);
        newWarehouseIngredient->head = NULL;
        newWarehouseIngredient->tail = NULL;
        newWarehouseIngredient->next = warehouse[index];
        warehouse[index] = newWarehouseIngredient;
        warehouse_sorted_insert(newWarehouseIngredient, quantity, expirationDate);
    }
    else {// Ingredient found, insert the batch
        warehouseIngredient *current = hash_table_warehouse_lookup(name);
        warehouse_sorted_insert(current, quantity, expirationDate);
    }
}

void check_expiration_date(const int time) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        if (warehouse[i] != NULL) {
            warehouseIngredient *current = warehouse[i];
            warehouseIngredient *prev = NULL;
            while (current != NULL) {
                Batch *tmp = current->head;
                while (tmp != NULL) {
                    Batch *next = tmp->next; // Save the next batch before deleting the current one
                    if (tmp->expirationDate <= time) {
                        if (tmp->prev == NULL) { // delete head
                            current->head = tmp->next;
                            if (tmp->next != NULL) {
                                tmp->next->prev = NULL;
                            } else {
                                current->tail = NULL; // List becomes empty
                            }
                        } else if (tmp->next == NULL) { // delete tail
                            current->tail = tmp->prev;
                            tmp->prev->next = NULL;
                        } else { // delete in the middle
                            tmp->prev->next = tmp->next;
                            tmp->next->prev = tmp->prev;
                        }
                        free(tmp);
                    }
                    tmp = next; // Move to the next batch
                }
                // Check if the ingredient has no batches left
                if (current->head == NULL) {
                    if (prev == NULL) { // Ingredient is the first in the list
                        warehouse[i] = current->next;
                    }
                    else {
                        prev->next = current->next;
                    }
                    free(current);
                    if (prev == NULL) {
                        current = warehouse[i]; // Set current to the head of the list
                    }
                    else {
                        current = prev->next; // Set current to the next ingredient after prev
                    }
                }
                else {
                    prev = current;
                    current = current->next;
                }
            }
        }
    }
}

int check_orders_executability(const Order *order) {
    const Recipe *recipe = hash_table_lookup(order->recipeName);
    if (recipe == NULL) {
        printf("%s", "rifiutato\n");
        return -1;
    }

    Ingredient *current = recipe->head;
    while (current != NULL) {
        warehouseIngredient *ingredient = warehouse[hash(current->ingredientName)];
        if (ingredient == NULL) {
            return 0; // Missing ingredient
        }

        int neededQuantity = current->ingredientQuantity * order->numberOfPieces;
        Batch *batch = ingredient->head;

        // Iterate through batches and check availability
        while (batch != NULL && neededQuantity > 0) {
            if (batch->quantity > neededQuantity) {
                batch->isUsed = true;
                neededQuantity -= batch->quantity;
                break;
            }
            neededQuantity -= batch->quantity;
            batch->isUsed = true;

            batch = batch->next;
        }

        if (neededQuantity > 0) {
            return 0; // Not enough ingredients
        }

        current = current->next;
    }
    return 1;
}

void reset_isUsed_flag() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        if (warehouse[i] != NULL) {
            warehouseIngredient *current = warehouse[i];
            while (current != NULL) {
                Batch *batch = current->head;
                while (batch != NULL) {
                    batch->isUsed = false;
                    batch = batch->next;
                }
                current = current->next;
            }
        }
    }
}

void execute_order(const Order *order) {
    Recipe *recipe = hash_table_lookup(order->recipeName);
    if (recipe == NULL) {
        fprintf(stderr, "Recipe not found\n");
        return;
    }
    Ingredient *current = recipe->head;
    if (current == NULL) {
        fprintf(stderr, "No ingredients in recipe\n");
        return;
    }
    warehouseIngredient *ingredient = warehouse[hash(current->ingredientName)];
    if (ingredient == NULL) {
        fprintf(stderr, "Ingredient not found in warehouse\n");
        return;
    }
    Batch *batch = ingredient->head;

    while (current != NULL) {
        int neededQuantity = current->ingredientQuantity * order->numberOfPieces;
        //printf("Debug: Processing ingredient %s, needed quantity: %d\n", current->ingredientName, neededQuantity); // Debug print
        while (batch != NULL) {
            //printf("Debug: Processing batch with quantity %d\n", batch->quantity); // Debug print
            if (batch->isUsed == true) {
                int ingredientReduction = batch->quantity;
                batch->quantity -= neededQuantity;
                neededQuantity -= ingredientReduction;
                //printf("Debug: Updated batch quantity: %d\n", batch->quantity); // Debug print
                if (batch->quantity <= 0) {
                    if (batch->prev == NULL) { // delete head
                        ingredient->head = batch->next;
                        if (batch->next != NULL) {
                            batch->next->prev = NULL;
                        }
                        else {
                            ingredient->tail = NULL; // List becomes empty
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
                    Batch *temp = batch;
                    batch = batch->next;
                    free(temp);
                } else {
                    batch = batch->next;
                }
            } else {
                batch = batch->next;
            }
        }
        current = current->next;
        if (current != NULL) {
            ingredient = warehouse[hash(current->ingredientName)];
            if (ingredient == NULL) {
                fprintf(stderr, "Ingredient not found in warehouse\n");
                return;
            }
            batch = ingredient->head;
        }
    }
}

void process_order(const Order *order, waitingOrderQueue *waitingOrderQueue, camionQueue *camionQueue) {
    if (check_orders_executability(order) == 1) {
        camion_queue_sorted_insert(camionQueue, order->recipeName, order->numberOfPieces, order->arrivingTime);
        execute_order(order);
        printf("%s", "accettato\n");

    }
    else {
        waiting_orders_queue_insert(waitingOrderQueue, order->recipeName, order->numberOfPieces, order->arrivingTime);
        reset_isUsed_flag();
        printf("%s", "accettato\n");
    }
}

void process_waiting_orders(waitingOrderQueue *waitingOrderQueue, camionQueue *camionQueue) {
    Order *current = waitingOrderQueue->head;
    while (current != NULL) {
        Order *next = current->next; // Save the next order before possibly freeing current
        if (check_orders_executability(current) == 1) {
            camion_queue_sorted_insert(camionQueue, current->recipeName, current->numberOfPieces, current->arrivingTime);
            execute_order(current);

            // Remove the processed order from the waiting queue
            if (current->prev == NULL) {// delete head
                waitingOrderQueue->head = current->next;
                if (current->next != NULL) {
                    current->next->prev = NULL;
                }
                else {
                    waitingOrderQueue->tail = NULL;// List becomes empty
                }
            }
            else if (current->next == NULL) { // delete tail
                waitingOrderQueue->tail = current->prev;
                current->prev->next = NULL;
            }
            else {// delete in the middle
                current->prev->next = current->next;
                current->next->prev = current->prev;
            }

            free(current);// Free the memory for the processed order
        }
        current = next;// Move to the next order
    }
}

void print_camion_queue(const camionQueue *camionQueue) {
    if (camionQueue == NULL) {
        printf("CamionQueue is NULL\n");
        return;
    }
    printf("%s", "CamionQueue: \n");
    Order *current = camionQueue->head;
    if (current == NULL) {
        return;
    }
    while (current != NULL) {
        printf("%d %s  %d - ", current->arrivingTime, current->recipeName, current->numberOfPieces);
        current = current->next;
    }
    printf("\n");
}

void print_waiting_orders_queue(const waitingOrderQueue *waitingOrdersQueue) {
    if (waitingOrdersQueue == NULL) {
        printf("WaitingOrdersQueue is NULL\n");
        return;
    }
    printf("%s", "WaitingOrdersQueue: \n");
    Order *current = waitingOrdersQueue->head;
    if (current == NULL) {
        return;
    }
    while (current != NULL) {
        printf("%s %d %d - ", current->recipeName, current->numberOfPieces, current->arrivingTime);
        current = current->next;
    }
    printf("\n");
}

camionQueue *select_orders_to_load(camionQueue *queue, int camionCapacity) {
    camionQueue *selectedOrders = create_new_camion_queue();

    if (queue == NULL) {
        return NULL;
    }
    Order *current = queue->head;
    while (current != NULL && current->weight <= camionCapacity) {
        camionCapacity -= current->weight;
        Order *next = current->next;
        camion_queue_sorted_loading_insert(selectedOrders, current->recipeName, current->numberOfPieces, current->arrivingTime);

        // Remove current order from camionQueue
        if (current->prev == NULL) {// delete head
            queue->head = current->next;
            if (current->next != NULL) {
                current->next->prev = NULL;
            }
            else {
                queue->tail = NULL;// List becomes empty
            }
        }
        else if (current->next == NULL) { // delete tail
            queue->tail = current->prev;
            current->prev->next = NULL;
        }
        else {// delete in the middle
            current->prev->next = current->next;
            current->next->prev = current->prev;
        }
        free(current);// Free the memory for the processed order
        current = next;
    }
    return selectedOrders;
}

void load_camion(camionQueue *queue, const int camionCapacity) {
    camionQueue *selectedOrders = select_orders_to_load(queue, camionCapacity);

    if (selectedOrders == NULL) {
        printf("selectedOrders queue is NULL\n");
        return;
    }

    if (selectedOrders->head == NULL) {
        printf("%s", "camioncino vuoto\n");
    }
    else {
        Order *current = selectedOrders->head;
        while (current != NULL) {
            printf("%d %s %d\n", current->arrivingTime, current->recipeName, current->numberOfPieces);
            current = current->next;
        }
    }
    // Free the selectedOrders queue
        Order *current = selectedOrders->head;
        while (current != NULL) {
            Order *next = current->next;
            free(current);
            current = next;
        }
        free(selectedOrders);
}


int main() {
    init_hash_tables();
    waitingOrderQueue *waitingOrderQueue = create_new_waiting_orders_queue();
    camionQueue *camionQueue = create_new_camion_queue();
    int time=0;
    int camionCapacity=0;
    int refillFrequency=0;
    char line[MAX_LINE_LENGHT];
    int quantity = 0;
    int expirationDate;
    int numOfPieces;


    scanf("%d %d", &refillFrequency, &camionCapacity);
    while (fgets(line, sizeof(line), stdin) != NULL) {
        char command[19];
        char *cursor = line;
        if (time != 0 && time % refillFrequency == 0) {
            //print_camion_queue(camionQueue);
            //print_waiting_orders_queue(waitingOrderQueue);
            load_camion(camionQueue, camionCapacity);
        }
        if (sscanf(line, "%s", command) == 1) {
            char recipeName[50];
            //printf("Current time: %d\n", time); //debug print
            check_expiration_date(time);
            if (strcmp(command, "aggiungi_ricetta") == 0) {
                cursor += strlen(command) + 1;
                if (sscanf(cursor, "%s", recipeName) != 1) {
                    printf("Warning!\n");
                    return 1;
                }
                cursor += strlen(recipeName) + 1;

                Recipe *newRecipe = hash_table_lookup(recipeName);

                if (newRecipe == NULL) {
                    printf("%s", hash_table_insert(recipeName));
                    newRecipe = hash_table_lookup(recipeName);
                }
                else {
                    printf("ignorato\n");
                    time++;
                    continue;;
                }

                char ingredientName[255];
                while (sscanf(cursor, "%s %d", ingredientName, &quantity) == 2) {
                    recipe_insert_at_tail(newRecipe, ingredientName, quantity);
                    cursor += strlen(ingredientName) + 1;
                    while (*cursor == ' ') {
                        cursor++;
                    }
                    cursor += snprintf(NULL, 0, "%d", quantity);
                    while (*cursor == ' ') {
                        cursor++;
                    }
                }
            }
            else if (strcmp(command, "rimuovi_ricetta") == 0) {
                cursor += strlen(command) + 1;
                if (sscanf(cursor, "%s", recipeName) != 1) {
                    printf("Warning!\n");
                    return 1;
                }
                printf("%s", remove_recipe(recipeName, waitingOrderQueue, camionQueue));
            }
            else if (strcmp(command, "rifornimento") == 0) {
                char name[MAX_NAME];
                cursor += strlen(command) + 1;
                while (sscanf(cursor, "%s %d %d", name, &quantity, &expirationDate) == 3) {
                    hash_table_warehouse_insert(name, quantity, expirationDate);
                    cursor += strlen(name) + 1 + snprintf(NULL, 0, "%d", quantity) + 1 + snprintf(NULL, 0, "%d", expirationDate) + 1;
                }
                process_waiting_orders(waitingOrderQueue, camionQueue);
                printf("%s", "rifornito\n");
            }
            else if (strcmp(command, "ordine") == 0) {
                char name[MAX_NAME];
                cursor += strlen(command) + 1;
                //printf("Debug: Command = %s\n", command); // Debug print)
                if (sscanf(cursor, "%s %d", name, &numOfPieces) == 2) {
                    //printf("Debug: Processing order for %s with %d pieces at time %d\n", name, numOfPieces, time); // Debug print

                    if (hash_table_lookup(name) == NULL) {
                        printf("%s", "rifiutato\n");
                    }
                    else {
                        Order *newOrder = create_new_order(name, numOfPieces, time);
                        process_order(newOrder, waitingOrderQueue, camionQueue);
                    }
                }
            }
            time++;
        }
        // Re-initialize the line at the end of each step
        memset(line, 0, sizeof(line));
    }
    if (time != 0 && time % refillFrequency == 0) {
        load_camion(camionQueue, camionCapacity);
    }
    return 0;
}