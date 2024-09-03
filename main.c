#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MAX_NAME 20
#define TABLE_SIZE 15000

typedef struct Batch {
    int expirationDate;
    short int quantity;
    struct Batch *next;
} Batch;

typedef struct warehouseIngredient {
    char ingredientName[MAX_NAME];
    short int totalQuantity;
    Batch *head;
    struct warehouseIngredient *next;
} warehouseIngredient;

warehouseIngredient *warehouse[TABLE_SIZE] = {NULL};

typedef struct Ingredient {
    char ingredientName[MAX_NAME];
    short int quantity;
    unsigned int hashvalue;
    struct Ingredient *next;
} Ingredient;

typedef struct Recipe {
    char recipeName[MAX_NAME];
    short int weight;
    Ingredient *head;
    struct Recipe *next;
} Recipe;

Recipe *cookbook[TABLE_SIZE] = {NULL};

typedef struct Order {
    Recipe *recipe;
    unsigned int hashvalue;
    short int numberOfPieces;
    int arrivingTime;
    short int weight;
    struct Order *next;
} Order;

typedef struct orderVector {
    struct Order *order;
    int size;
    int capacity;
} orderVector;

typedef struct orderQueue {
    Order *checkpoint;
    Order *head;
    Order *tail;
} orderQueue;

void print_order_vector(orderVector *orderVector);
void free_ingredients(Ingredient *head);
void free_hash_tables();
void free_vector(orderVector *vector);

unsigned int hash(const char *inputString) {
    unsigned long hash = 5381;
    short int c;

    while ((c = *inputString++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % TABLE_SIZE;
}

Order *create_new_order(Recipe *recipe, int numOfPieces, int arrivingTime, int recipeWeight, unsigned int recipeHash) {
    Order *newOrder = malloc(sizeof(Order));
    assert(newOrder != NULL);
    newOrder->recipe = recipe;
    newOrder->numberOfPieces = numOfPieces;
    newOrder->arrivingTime = arrivingTime;
    newOrder->weight = recipeWeight * numOfPieces;
    newOrder->hashvalue = recipeHash;
    newOrder->next = NULL;
    return newOrder;
}

orderVector *init_vector(int capacity) {
    orderVector *newVector = malloc(sizeof(orderVector));
    assert(newVector != NULL);
    newVector->order = malloc(capacity * sizeof(Order));
    assert(newVector->order != NULL);
    newVector->size = 0;
    newVector->capacity = capacity;
    return newVector;
}

void expand_vector(orderVector *vector) {
    vector->capacity += vector->capacity/2;
    vector->order = realloc(vector->order, vector->capacity * sizeof(Order));
    assert(vector->order != NULL);
}

void vector_insert(orderVector *vector, Order *tmp) {
    if (vector->size == vector->capacity) {
        expand_vector(vector);
    }
    vector->order[vector->size] = *tmp;
    vector->size++;
}

int partition(orderVector *vector, int p, int r) {
    Order x = vector->order[r];
    int i = p - 1;
    for (int j = p; j < r; j++) {
        if (vector->order[j].weight > x.weight || (vector->order[j].weight == x.weight && vector->order[j].arrivingTime < x.arrivingTime)) {
            i++;
            Order tmp = vector->order[i];
            vector->order[i] = vector->order[j];
            vector->order[j] = tmp;
        }
    }
    Order tmp = vector->order[i + 1];
    vector->order[i + 1] = vector->order[r];
    vector->order[r] = tmp;
    return i + 1;
}

void quicksort(orderVector *vector, int p, int r) {
    if (p < r) {
        int q = partition(vector, p, r);
        quicksort(vector, p, q - 1);
        quicksort(vector, q + 1, r);
    }
}

orderQueue *init_queue() {
    orderQueue *newQueue = malloc(sizeof(orderQueue));
    assert(newQueue != NULL);
    newQueue->checkpoint = NULL;
    newQueue->head = NULL;
    newQueue->tail = NULL;
    return newQueue;
}

Batch *create_new_batch(int expirationDate, int quantity) {
    Batch *newBatch = malloc(sizeof(Batch));
    assert(newBatch != NULL);
    newBatch->next = NULL;
    newBatch->quantity = quantity;
    newBatch->expirationDate = expirationDate;
    return newBatch;
}

Ingredient *create_new_ingredient(char ingredientName[], int quantity) {
    Ingredient *newIngredient = malloc(sizeof(Ingredient));
    assert(newIngredient != NULL);
    strcpy(newIngredient->ingredientName, ingredientName);
    newIngredient->quantity = quantity;
    newIngredient->hashvalue = hash(ingredientName);
    newIngredient->next = NULL;
    return newIngredient;
}

void queue_tail_insert(Order *order, orderQueue *queue) {
    if (queue->head == NULL) {
        queue->head = order;
        queue->tail = order;
        queue->tail->next = NULL;
    }
    else if (queue->tail == queue->head) {
        queue->tail = order;
        queue->head->next = order;
        queue->tail->next = NULL;
    }
    else {
        queue->tail->next = order;
        queue->tail = order;
        queue->tail->next = NULL;
    }
}

int queue_lookup(Recipe *recipe, orderQueue *queue) {
    Order *current = queue->head;
    while (current != NULL && strcmp(current->recipe->recipeName, recipe->recipeName) != 0) {
        current = current->next;
    }
    if (current == NULL) {
        return 0;
    }
    return 1;
}

void init_hash_tables() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        warehouse[i] = NULL;
        cookbook[i] = NULL;
    }
}

Recipe *hash_table_cookbook_insert(char name[], unsigned int index) {
    assert(name != NULL);
    Recipe *newRecipe = malloc(sizeof(Recipe));
    assert (newRecipe != NULL);
    strcpy(newRecipe->recipeName, name);
    newRecipe->weight = 0;
    newRecipe->head = NULL;
    newRecipe->next = cookbook[index];
    cookbook[index] = newRecipe;
    return newRecipe;
}

void hash_table_ingredient_insert(char name[], int quantity, Recipe *recipe) {
    assert(name != NULL);
    Ingredient *newIngredient = create_new_ingredient(name, quantity);
    newIngredient->next = recipe->head;
    recipe->head = newIngredient;
}

Recipe *hash_table_cookbook_lookup(char name[], unsigned int index) { //function to search elements inside the cookbook hash table
    Recipe *tmp = cookbook[index];
    while (tmp != NULL && strcmp(tmp->recipeName, name) != 0) {
        tmp = tmp->next;
    }
    return tmp;
}

char *hash_table_cookbook_delete(char name[], orderQueue *waitingQueue, orderQueue *camionQueue, unsigned int index) {
    Recipe *tmp = cookbook[index];
    Recipe *prev = NULL;
    while (tmp != NULL && strcmp(tmp->recipeName, name) != 0) {
        prev = tmp;
        tmp = tmp->next;
    }
    if (tmp == NULL) {
        return "non presente\n";
    }
    else if (queue_lookup(tmp,waitingQueue) == 1 || queue_lookup(tmp,camionQueue) == 1) { // element found but in use
        return "ordini in sospeso\n";
    }
    if (prev == NULL) { //deleting the head
        cookbook[index] = tmp->next;
    }
    else {
        prev->next = tmp->next;
    }
    Ingredient *current = tmp->head;
    while (current != NULL) {
        Ingredient *toFree = current;
        current = current->next;
        free(toFree);
    }
    free(tmp);
    return "rimossa\n";
}

void hash_table_warehouse_insert(char name[], unsigned int index) {
    assert (name != NULL);
    warehouseIngredient *newIngredient = malloc(sizeof(warehouseIngredient));
    assert(newIngredient != NULL);
    strcpy(newIngredient->ingredientName, name);
    newIngredient->totalQuantity = 0;
    newIngredient->head = NULL;
    newIngredient->next = warehouse[index];
    warehouse[index] = newIngredient;
}

warehouseIngredient *hash_table_warehouse_lookup(char name[], unsigned int index) { //function to search elements inside the warehouse hash table (they could be merged)
    warehouseIngredient *tmp = warehouse[index];
    while (tmp != NULL && strcmp(tmp->ingredientName , name) != 0) {
        tmp = tmp->next;
    }
    return tmp;
}

int check_order_exacutability(Order *order, Recipe *recipe, int time) {
    Ingredient *current = recipe->head;
    while (current != NULL) {
        int neededQuantity = (current->quantity * order->numberOfPieces);
        warehouseIngredient *ingredient = hash_table_warehouse_lookup(current->ingredientName, current->hashvalue);

        if (ingredient == NULL) {
            return 0;
        }
        Batch *tmp = ingredient->head;
        Batch *oldTmp = NULL;

        while (tmp != NULL && tmp->expirationDate <= time) {
            oldTmp = tmp;
            tmp = tmp->next;
            ingredient->totalQuantity -= oldTmp->quantity;
            free(oldTmp);
        }
        ingredient->head = tmp;

        if (ingredient->totalQuantity < neededQuantity) {
            return 0;
        }
        current = current->next;
    }
    return 1;
}

void waiting_orders_insert(Order *toInsert, orderQueue *camionQueue) {
    Order *tmp = camionQueue->checkpoint;
    Order *oldTmp = NULL;
    while (tmp != NULL && tmp->arrivingTime < toInsert->arrivingTime) {
        oldTmp = tmp;
        tmp = tmp->next;
    }
    tmp = oldTmp;
    camionQueue->checkpoint = toInsert;
    if (tmp == NULL) { //insert at the head
        toInsert->next = camionQueue->head;
        camionQueue->head = toInsert;
        if (camionQueue->tail == NULL) {
            camionQueue->tail = toInsert;
        }
    }
    else if (tmp == camionQueue->tail && camionQueue->head == camionQueue->tail) { //insert at tail with initialization of the new tail
        toInsert->next = NULL;
        camionQueue->tail = toInsert;
        camionQueue->head->next = toInsert;
    }
    else if (tmp == camionQueue->tail) { //insert at tail
        camionQueue->tail->next = toInsert;
        camionQueue->tail = toInsert;
        toInsert->next = NULL;
    }
    else {
        toInsert->next = tmp->next;
        tmp->next = toInsert;
    }
}

void execute_order(Order *order, Recipe *recipe) {
    Ingredient *current = recipe->head;
    while (current != NULL) {
        int neededQuantity = (current->quantity * order->numberOfPieces);
        warehouseIngredient *ingredient = hash_table_warehouse_lookup(current->ingredientName, current->hashvalue);
        Batch *batch = ingredient->head;
        while (batch != NULL && neededQuantity > 0) {
            if (batch->quantity > neededQuantity) {
                batch->quantity -= neededQuantity;
                ingredient->totalQuantity -= neededQuantity;
                break;
            }
            neededQuantity -= batch->quantity;
            Batch *tmp = batch;
            ingredient->totalQuantity -= batch->quantity;
            batch = batch->next;
            ingredient->head = batch;
            free(tmp);
        }
        current = current->next;
    }
}

void check_waiting_orders(orderQueue *waitingOrdersQueue, orderQueue *camionQueue, int time) {
    Order *tmp = waitingOrdersQueue->head;
    Order *prev = NULL;
    camionQueue->checkpoint = camionQueue->head;

    while (tmp != NULL) {
        Order *nextOrder = tmp->next;

        if (check_order_exacutability(tmp, tmp->recipe, time) == 1) {
            execute_order(tmp, tmp->recipe);

            if (prev == NULL) {
                waitingOrdersQueue->head = tmp->next;
            }
            else {
                prev->next = tmp->next;
            }

            if(waitingOrdersQueue->tail == tmp){
                waitingOrdersQueue->tail = prev;
                if(prev) prev->next = NULL;
            }
            waiting_orders_insert(tmp, camionQueue);
        }
        else {
            prev = tmp;
        }
        tmp = nextOrder;
    }
}

void free_order_queue(orderQueue *queue) {
    Order *tmp = queue->head;
    while (tmp != NULL) {
        Order *toFree = tmp;
        tmp = tmp->next;
        free(toFree);
    }
}

Order *dequeue_camion(orderQueue *queue) {
    if (queue->head == NULL)
        return NULL;
    Order *temp = queue->head;
    queue->head = queue->head->next;
    if (queue->head == NULL)
        queue->tail = NULL;
    return temp;
}

void load_camion(orderQueue *camionQueue, int camionCapacity) {
    Order *tmp = NULL;
    orderVector *loadingVector = init_vector(512);

    while ( (tmp = camionQueue->head) != NULL && camionCapacity >= tmp->weight) {
        camionCapacity -= tmp->weight;

        tmp = dequeue_camion(camionQueue);
        vector_insert(loadingVector, tmp);
    }
    if (loadingVector->size == 0) {
        printf("camioncino vuoto\n");
    }
    else {
        quicksort(loadingVector, 0, loadingVector->size - 1);
        print_order_vector(loadingVector);
    }
    free_vector(loadingVector);
}

void batches_sorted_insert(warehouseIngredient *ingredient, Batch *newBatch) {
    Batch *tmp = ingredient->head;
    Batch *oldTmp = NULL;
    while (tmp != NULL && tmp->expirationDate < newBatch->expirationDate) {
        oldTmp = tmp;
        tmp = tmp->next;
    }
    if (tmp != NULL && tmp->expirationDate == newBatch->expirationDate) {
        tmp->quantity += newBatch->quantity;
        free(newBatch);
        return;
    }
    if (oldTmp == NULL) {
        newBatch->next = ingredient->head;
        ingredient->head = newBatch;
    }
    else {
        newBatch->next = oldTmp->next;
        oldTmp->next = newBatch;
    }
}

void free_batches(Batch *head) {
    Batch *tmp = head;
    while (tmp != NULL) {
        Batch *next = tmp->next;
        free(tmp);
        tmp = next;
    }
}

int main() {
    int time=0;
    init_hash_tables();
    orderQueue *camionQueue = init_queue();
    orderQueue *waitingQueue = init_queue();
    char command[MAX_NAME] = "";
    int camionCapacity=0;
    int refillFrequency=0;
    int quantity=0;
    int expirationDate=0;

    if (!scanf("%d %d", &refillFrequency, &camionCapacity)) {
        printf ("Failed getting refillFrequency and camionCapacity\n");
        return 1;
    }
    while (scanf("%s", command) != EOF) {
        if (strcmp(command, "aggiungi_ricetta") == 0) {
            char recipeName[MAX_NAME];
            if (!scanf("%s", recipeName)) {
                printf ("Failed getting recipeName\n");
                return 1;
            }
            unsigned int hashvalue = hash(recipeName);
            if (hash_table_cookbook_lookup(recipeName, hashvalue) != NULL) {
                while (getchar_unlocked() != '\n');
                printf("ignorato\n");
                time++;
                continue;
            }
            Recipe *recipe = hash_table_cookbook_insert(recipeName, hashvalue);
            int recipeWeight=0;
            while (getchar_unlocked() != '\n'){
                char ingredient[MAX_NAME];
                if (!scanf("%s %d", ingredient, &quantity)) {
                    printf ("Failed getting ingredient and quantity\n");
                    return 1;
                }
                recipeWeight = recipeWeight + quantity;
                hash_table_ingredient_insert(ingredient, quantity, recipe);
            }
            recipe->weight = recipeWeight;
            printf("aggiunta\n");
        }
        else if (strcmp(command, "rifornimento") == 0) {
            while (getchar_unlocked() != '\n') {
                char ingredientName[MAX_NAME];
                if (!scanf("%s %d %d", ingredientName, &quantity, &expirationDate)) {
                    printf ("Failed getting ingredientName, quantity and expirationDate\n");
                    return 1;
                }
                if (expirationDate <= time) {
                    continue;
                }
                unsigned int hashvalue = hash(ingredientName);
                if (hash_table_warehouse_lookup(ingredientName, hashvalue) == NULL) {
                    hash_table_warehouse_insert(ingredientName, hashvalue);
                }
                warehouseIngredient *ingredient = hash_table_warehouse_lookup(ingredientName, hashvalue);
                Batch *newBatch = create_new_batch(expirationDate, quantity);
                ingredient->totalQuantity += quantity;
                batches_sorted_insert(ingredient, newBatch);
            }
            check_waiting_orders(waitingQueue, camionQueue, time);
            printf("rifornito\n");
        }
        else if (strcmp(command, "ordine") == 0) {
            char recipeName[MAX_NAME];
            int numOfPieces=0;
            if (!scanf("%s %d", recipeName, &numOfPieces)) {
                printf("Failed getting orderName and numOfPieces\n");
                return 1;
            }
            unsigned int hashvalue = hash(recipeName);
            Recipe *recipe = hash_table_cookbook_lookup(recipeName, hashvalue);
            if (recipe == NULL) {
                while (getchar_unlocked() != '\n');
                printf("rifiutato\n");
            }
            else {
                Order *newOrder = create_new_order(recipe, numOfPieces, time, recipe->weight, hashvalue);
                if (check_order_exacutability(newOrder, recipe, time) == 0) {
                    queue_tail_insert(newOrder, waitingQueue);
                }
                else {
                    execute_order(newOrder, recipe);
                    queue_tail_insert(newOrder, camionQueue);
                }
                printf("accettato\n");
            }
        }
        else if (strcmp(command, "rimuovi_ricetta") == 0){
            char recipeName[MAX_NAME];
            if (!scanf("%s", recipeName)) {
                printf ("Failed getting recipeName\n");
                return 1;
            }
            unsigned int hashvalue = hash(recipeName);
            printf("%s", hash_table_cookbook_delete(recipeName, waitingQueue, camionQueue, hashvalue));
        }
        time++;
        if (time % refillFrequency == 0) {
            load_camion(camionQueue, camionCapacity);
        }
    }
    free_hash_tables();
    free_order_queue(waitingQueue);
    free(waitingQueue);
    free_order_queue(camionQueue);
    free(camionQueue);
    return 0;
}

void print_order_vector(orderVector *vector) {
    for (int i = 0; i < vector->size; i++) {
        printf("%d %s %d\n", vector->order[i].arrivingTime, vector->order[i].recipe->recipeName, vector->order[i].numberOfPieces);
    }
}

//FREE FUNCTIONS
void free_ingredients(Ingredient *head) {
    if (head == NULL) {
        return;
    }
    free_ingredients(head->next);
    free(head);
}
void free_hash_tables() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        Recipe *tmp = cookbook[i];
        while (tmp != NULL) {
            free_ingredients(tmp->head);
            Recipe *next = tmp->next;
            free(tmp);
            tmp = next;
        }
    }

    for (int i = 0; i < TABLE_SIZE; i++) {
        warehouseIngredient *tmp = warehouse[i];
        while (tmp != NULL) {
            free_batches(tmp->head);
            warehouseIngredient *next = tmp->next;
            free(tmp);
            tmp = next;
        }
    }
}

void free_vector(orderVector *vector) {
    free(vector->order);
    free(vector);
}