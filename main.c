#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MAX_NAME 30
#define TABLE_SIZE 10000

//FILE *debugFile = NULL;

typedef struct Order {
    char recipeName[MAX_NAME];
    int numberOfPieces;
    int arrivingTime;
    int weight;
    struct Order *next;
} Order;

typedef struct orderQueue {
    Order *checkpoint;
    Order *head;
    Order *tail;
} orderQueue;

typedef struct Batch {
    int expirationDate;
    int quantity;
    struct Batch *parent;
    struct Batch *left;
    struct Batch *right;
} Batch;

typedef struct warehouseIngredient {
    char ingredientName[MAX_NAME];
    int totalQuantity;
    Batch *root;
    Batch *minimum;
    struct warehouseIngredient *next;
} warehouseIngredient;

warehouseIngredient *warehouse[TABLE_SIZE];

typedef struct Ingredient {
    char ingredientName[MAX_NAME];
    int quantity;
    struct Ingredient *next;
} Ingredient;

typedef struct Recipe {
    char recipeName[MAX_NAME];
    int weight;
    Ingredient *head;
    struct Recipe *next;
} Recipe;

Recipe *cookbook[TABLE_SIZE];

Order *create_new_order(char recipeName[], int numOfPieces, int arrivingTime, int recipeWeight) {
    Order *newOrder = malloc(sizeof(Order));
    assert(newOrder != NULL);
    strcpy(newOrder->recipeName, recipeName);
    newOrder->numberOfPieces = numOfPieces;
    newOrder->arrivingTime = arrivingTime;
    newOrder->weight = recipeWeight * numOfPieces;
    newOrder->next = NULL;
    return newOrder;
}

orderQueue *init_queue() {
    orderQueue *newQueue = malloc(sizeof(orderQueue));
    assert(newQueue != NULL);
    newQueue->checkpoint = NULL;
    newQueue->head = NULL;
    newQueue->tail = NULL;
    return newQueue;
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
    while (current != NULL && strcmp(current->recipeName, recipe->recipeName) != 0) {
        current = current->next;
    }
    if (current == NULL) {
        return 0;
    }
    return 1;
}

Batch *create_new_batch(int expirationDate, int quantity) {
    Batch *newBatch = malloc(sizeof(Batch));
    assert(newBatch != NULL);
    newBatch->left = NULL;
    newBatch->right = NULL;
    newBatch->parent = NULL;
    newBatch->quantity = quantity;
    newBatch->expirationDate = expirationDate;
    return newBatch;
}

Ingredient *create_new_ingredient(char ingredientName[], int quantity) {
    Ingredient *newIngredient = malloc(sizeof(Ingredient));
    assert(newIngredient != NULL);
    strcpy(newIngredient->ingredientName, ingredientName);
    newIngredient->quantity = quantity;
    newIngredient->next = NULL;
    return newIngredient;
}

Batch *find_minimum(Batch *root) {
    while (root->left != NULL) {
        root = root->left;
    }
    return root;
}

Batch *get_successor(Batch *node) {
    if (node->right != NULL) {
        return find_minimum(node->right);
    }
    Batch *parent = node->parent;
    while (parent != NULL && node == parent->right) {
        node = parent;
        parent = parent->parent;
    }
    return parent;
}

void insert_batch(Batch **root_ptr, Batch *newBatch) {
    Batch *root = *root_ptr;

    if (root == NULL) { //tree empty || leaf node reached
        *root_ptr = newBatch;
    }
    else if (root->expirationDate == newBatch->expirationDate) {
        root->quantity += newBatch->quantity;
        free(newBatch);
    }
    else if (newBatch->expirationDate < root->expirationDate) {
        if (root->left == NULL) {
            root->left = newBatch;
            newBatch->parent = root;
        }
        else {
            insert_batch(&root->left, newBatch);
        }

    }
    else {
        if (root->right == NULL) {
            root->right = newBatch;
            newBatch->parent = root;
        }
        else {
            insert_batch(&root->right, newBatch);
        }
    }
}

Batch *find_batch(Batch *root, int expirationDate) {
    if (root == NULL) {
        return NULL;
    }
    if (root->expirationDate == expirationDate) {
        return root;
    }
    if (expirationDate < root->expirationDate ) {
        return find_batch(root->left, expirationDate);
    }
    return find_batch(root->right, expirationDate);
}

Batch *tree_successor(Batch *root) {
    if (root->right != NULL) {
        return find_minimum(root->right);
    }
    Batch *parent = root->parent;
    while (parent != NULL && root == parent->right) {
        root = parent;
        parent = parent->parent;
    }
    return parent;
}

void print_warehouse_tree(Batch *root) {
    if (root == NULL) {
        return;
    }
    print_warehouse_tree(root->left);
    printf("%d %d --- ", root->quantity, root->expirationDate);
    print_warehouse_tree(root->right);
}

void print_warehouse_table() {
    printf("Start\n");
    for(int i=0; i<TABLE_SIZE; i++) {
        if (warehouse[i] == NULL) {
            printf("\t%i\t---\n",i);
        }
        else {
            printf("\t%i\t",i);
            warehouseIngredient *tmp = warehouse[i];
            while ( tmp != NULL) {
                printf("%s - ", tmp->ingredientName);
                print_warehouse_tree(tmp->root);
                tmp = tmp->next;
            }
            printf("\n");
        }
    }
    printf("End\n");
}

void delete_batch(Batch **T, Batch *z) { //function to delete a node from the tree from Api slides
    Batch *y = NULL;
    Batch *x = NULL;
    if (z->left == NULL || z->right == NULL) {
        y = z;
    }
    else {
        y = tree_successor(z);
    }
    if (y->left != NULL) {
        x = y->left;
    }
    else {
        x = y->right;
    }
    if (x != NULL) {
        x->parent = y->parent;
    }
    if (y->parent == NULL) {
        *T = x;
    }
    else if (y == y->parent->left) {
        y->parent->left = x;
    }
    else {
        y->parent->right = x;
    }
    if (y != z) {
        z->expirationDate = y->expirationDate;
        z->quantity = y->quantity;
    }
    free(y);
}

void delete_minimum(warehouseIngredient *ingredient) {
    Batch *minimum = ingredient->minimum;
    if (minimum->right != NULL) {
        ingredient->minimum = find_minimum(minimum->right);
    }
    else {
        ingredient->minimum = minimum->parent;
    }
    ingredient->totalQuantity -= minimum->quantity;
    delete_batch(&ingredient->root, minimum);
}

void init_hash_tables() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        warehouse[i] = NULL;
        cookbook[i] = NULL;
    }
}

unsigned int hash(const char *inputString) {
    unsigned long hash = 5381;
    int c;

    while ((c = *inputString++)) {
        hash = ((hash << 5) + hash) + c;
    }

    return hash % TABLE_SIZE;
}

Recipe *hash_table_cookbook_insert(char name[]) {
    assert(name != NULL);
    int index = hash(name);
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

Recipe *hash_table_cookbook_lookup(char name[]) { //function to search elements inside the cookbook hash table
    int index = hash(name);
    Recipe *tmp = cookbook[index];
    while (tmp != NULL && strcmp(tmp->recipeName, name) != 0) {
        tmp = tmp->next;
    }
    return tmp;
}

char *hash_table_cookbook_delete(char name[], orderQueue *waitingQueue, orderQueue *camionQueue) {
    int index = hash(name);
    Recipe *tmp = cookbook[index];
    Recipe *prev = NULL;
    while (tmp != NULL && strcmp(tmp->recipeName, name) != 0) {
        prev = tmp;
        tmp = tmp->next;
    }
    //Recipe *tmp = hash_table_cookbook_lookup(name);

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
    // bisogna togliere tutti gli ingredienti dalla ricetta
    Ingredient *current = tmp->head;
    while (current != NULL) {
        Ingredient *toFree = current;
        current = current->next;
        free(toFree);
    }
    free(tmp);
    return "rimossa\n";
}

void hash_table_warehouse_insert(char name[]) {
    assert (name != NULL);
    int index = hash(name);
    warehouseIngredient *newIngredient = malloc(sizeof(warehouseIngredient));
    assert(newIngredient != NULL);
    strcpy(newIngredient->ingredientName, name);
    newIngredient->totalQuantity = 0;
    newIngredient->root = NULL;
    newIngredient->minimum = NULL;
    newIngredient->next = warehouse[index];
    warehouse[index] = newIngredient;
}

void hash_table_warehouse_delete(char name[]) {
    int index = hash(name);
    warehouseIngredient *tmp = warehouse[index];
    warehouseIngredient *prev = NULL;
    while (tmp != NULL && strcmp(tmp->ingredientName, name) != 0) {
        prev = tmp;
        tmp = tmp->next;
    }
    if (tmp == NULL) {
        return;
    }
    if (prev == NULL) { //deleting the head
        warehouse[index] = tmp->next;
    }
    else {
        prev->next = tmp->next;
    }
    free(tmp);
}

warehouseIngredient *hash_table_warehouse_lookup(char name[]) { //function to search elements inside the warehouse hash table (they could be merged)
    int index = hash(name);
    warehouseIngredient *tmp = warehouse[index];
    while (tmp != NULL && strcmp(tmp->ingredientName , name) != 0) {
        tmp = tmp->next;
    }
    return tmp;
}

int check_order_exacutability(Order *order, Recipe *recipe) {
    Ingredient *current = recipe->head;
    while (current != NULL) {
        int neededQuantity = (current->quantity * order->numberOfPieces);
        warehouseIngredient *ingredient = hash_table_warehouse_lookup(current->ingredientName);
        if (ingredient == NULL) {
            return 0;
        }
        else if (ingredient->totalQuantity < neededQuantity) {
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
    //assert(tmp != NULL);
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
        //tmp->next = toInsert;
        //camionQueue->tail = toInsert;
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
        warehouseIngredient *ingredient = hash_table_warehouse_lookup(current->ingredientName);
        Batch *batch = ingredient->minimum;
        while (neededQuantity > 0) {
            if (batch->quantity > neededQuantity) { // if the batch has more than enough ingredients to satisfy the order quantity
                batch->quantity -= neededQuantity; // subtract the needed quantity from the batch
                ingredient->totalQuantity -= neededQuantity; // subtract the needed quantity from the total quantity of the ingredient
                break; // exit the inner loop
            }
            neededQuantity -= batch->quantity; // otherwise subtract the batch quantity from the needed quantity
            Batch *successor = get_successor(batch); // get the successor of the batch
            ingredient->totalQuantity -= batch->quantity; // subtract the batch quantity from the total quantity of the ingredient
            delete_batch(&ingredient->root, batch); // delete the batch
            batch = successor; // set the batch to the successor
        }
        ingredient->minimum = batch; // set the minimum to the batch because the previous batch was deleted and the actual batch is set to the successor of the deleted batch so it must be the new minimum
        current = current->next; // go to the next ingredient
    }
}

/*
void execute_order(Order *order, Recipe *recipe) {
    Ingredient *current = recipe->head;
    while (current != NULL) {
        int neededQuantity = (current->quantity * order->numberOfPieces);
        warehouseIngredient *ingredient = hash_table_warehouse_lookup(current->ingredientName);
        Batch *batch = ingredient->minimum;

        printf("Processing ingredient: %s\n", current->ingredientName);
        printf("Needed quantity: %d\n", neededQuantity);
        printf("Total quantity: %d\n", ingredient->totalQuantity);
        while (neededQuantity > 0) {
            printf("Current batch quantity: %d\n", batch->quantity);
            if (batch->quantity > neededQuantity) { // if the batch has more than enough ingredients to satisfy the order quantity
                batch->quantity -= neededQuantity; // subtract the needed quantity from the batch
                ingredient->totalQuantity -= neededQuantity; // subtract the needed quantity from the total quantity of the ingredient
                printf("Batch quantity after deduction: %d\n", batch->quantity);
                printf("Total quantity after deduction: %d\n", ingredient->totalQuantity);
                break; // exit the inner loop
            }
            neededQuantity -= batch->quantity; // otherwise subtract the batch quantity from the needed quantity
            printf("Needed quantity after deduction: %d\n", neededQuantity);
            Batch *successor = get_successor(batch); // get the successor of the batch
            ingredient->totalQuantity -= batch->quantity; // subtract the batch quantity from the total quantity of the ingredient
            delete_batch(&ingredient->root, batch); // delete the batch
            batch = successor; // set the batch to the successor
        }
        ingredient->minimum = batch; // set the minimum to the batch because the previous batch was deleted and the actual batch is set to the successor of the deleted batch so it must be the new minimum
        current = current->next; // go to the next ingredient
    }
}
*/

void check_waiting_orders(orderQueue *waitingOrdersQueue, orderQueue *camionQueue) {
    Order *tmp = waitingOrdersQueue->head;
    Order *prev = NULL;
    camionQueue->checkpoint = camionQueue->head;

    while (tmp != NULL) {
        Recipe *recipe = hash_table_cookbook_lookup(tmp->recipeName);
        Order *nextOrder = tmp->next;

        if (check_order_exacutability(tmp, recipe) == 1) {
            execute_order(tmp, recipe);

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

void queue_sorted_loading_insert(Order **toInsert, orderQueue *queue) {
    Order *newOrder = *toInsert;

    Order *tmp = queue->head;
    Order *oldTmp = NULL;

    while (tmp != NULL && (tmp->weight > newOrder->weight || (tmp->weight == newOrder->weight && tmp->arrivingTime < newOrder->arrivingTime))) {
        oldTmp = tmp;
        tmp = tmp->next;
    }

    if (oldTmp == NULL) {
        newOrder->next = queue->head;
        queue->head = newOrder;

        if (queue->tail == NULL) {
            queue->tail = newOrder;
        }
    }
    else if (tmp == NULL) {
        oldTmp->next = newOrder;
        newOrder->next = NULL;
        queue->tail = newOrder;
    }
    else {
        newOrder->next = oldTmp->next;
        oldTmp->next = newOrder;
    }
}



void print_order_queue(orderQueue *queue) {
    Order *tmp = queue->head;
    while (tmp != NULL) {
        ///if(tmp == camionQueue->tail) printf("Trovata tail di camion queue\n");
        printf("%d %s %d\n", tmp->arrivingTime, tmp->recipeName, tmp->numberOfPieces);
        tmp = tmp->next;
    }
    //if(camionQueue->tail) printf("Camion tail: %d %s\n", camionQueue->tail->arrivingTime, camionQueue->tail->recipeName);
}

void print_order_queue_debug(orderQueue *queue) {
    Order *tmp = queue->head;
    while (tmp != NULL) {
        printf("%d %s %d con peso %d\n", tmp->arrivingTime, tmp->recipeName, tmp->numberOfPieces, tmp->weight);
        tmp = tmp->next;
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


void load_camion(orderQueue *camionQueue, int camionCapacity) {
    Order *tmp = camionQueue->head;
    orderQueue *loadingQueue = init_queue();

    while (tmp != NULL && camionCapacity >= tmp->weight) {
        camionCapacity -= tmp->weight;

        Order *orderToInsert = malloc(sizeof(Order));
        if (orderToInsert == NULL) {
            free_order_queue(loadingQueue);
            return;
        }
        *orderToInsert = *tmp;
        orderToInsert->next = NULL;

        queue_sorted_loading_insert(&orderToInsert, loadingQueue);

        Order *toFree = tmp;
        if (tmp == camionQueue->tail) {
            camionQueue->tail = NULL;
        }
        tmp = tmp->next;
        free(toFree);
    }
    camionQueue->head = tmp;

    if (loadingQueue->head == NULL) {
        printf("camioncino vuoto\n");
    }
    else {
        print_order_queue(loadingQueue);
    }

    free_order_queue(loadingQueue);
}


void free_ingredients(Ingredient *head) {
    if (head == NULL) {
        return;
    }
    free_ingredients(head->next);
    free(head);
}

void free_batches(Batch *root) {
    if (root == NULL) {
        return;
    }
    free_batches(root->left);
    free_batches(root->right);
    delete_batch(&root, root);
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
            free_batches(tmp->root);
            warehouseIngredient *next = tmp->next;
            free(tmp);
            tmp = next;
        }
    }
}

void print_ingredient_queue(Recipe *recipe) {
    Ingredient *tmp = recipe->head;
    while (tmp != NULL) {
        printf("- %s %d --- ", tmp->ingredientName, tmp->quantity);
        tmp = tmp->next;
    }
}

void print_cookbook_table() {
    printf("Start\n");
    for(int i=0; i<TABLE_SIZE; i++) {
        if (cookbook[i] == NULL) {
            printf("\t%i\t---\n",i);
        }
        else {
            printf("\t%i\t",i);
            Recipe *tmp = cookbook[i];
            while ( tmp != NULL) {
                printf("%s - ", tmp->recipeName);
                print_ingredient_queue(tmp);
                tmp = tmp->next;
            }
            printf("\n");
        }
    }
    printf("End\n");
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
    //debugFile = fopen("debug.txt", "w");

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
            if (hash_table_cookbook_lookup(recipeName) != NULL) {
                while (getchar_unlocked() != '\n');
                printf("ignorato\n");
                time++;
                continue;
            }
            Recipe *recipe = hash_table_cookbook_insert(recipeName);
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
            //fprintf(debugFile, "\nRIFORNIMENTO TIME = %d\n", time);
            while (getchar_unlocked() != '\n') {
                char ingredientName[MAX_NAME];
                if (!scanf("%s %d %d", ingredientName, &quantity, &expirationDate)) {
                    printf ("Failed getting ingredientName, quantity and expirationDate\n");
                    return 1;
                }
                if (expirationDate <= time) {
                    continue;
                }
                if (hash_table_warehouse_lookup(ingredientName) == NULL) {
                    hash_table_warehouse_insert(ingredientName);
                }
                warehouseIngredient *ingredient = hash_table_warehouse_lookup(ingredientName);
                Batch *newBatch = create_new_batch(expirationDate, quantity);
                ingredient->totalQuantity += quantity;
                insert_batch(&ingredient->root, newBatch);
                if (ingredient->minimum == NULL || ingredient->minimum->expirationDate > expirationDate) {
                    ingredient->minimum = newBatch;
                }
            }
            check_waiting_orders(waitingQueue, camionQueue);
            printf("rifornito\n");
        }
        else if (strcmp(command, "ordine") == 0) {
            /*
            fprintf(debugFile, "\nTIME = %d\n", time);
            fprintf(debugFile, "ORDINI IN ATTESA:\n");
            print_order_queue(waitingQueue, debugFile);
            fprintf(debugFile, "ORDINI PRONTI:\n");
            print_order_queue(camionQueue, debugFile);
            if(waitingQueue->tail) fprintf(debugFile, "WAITING QUEUE TAIL = %d %s %d\n", waitingQueue->tail->arrivingTime, waitingQueue->tail->recipeName, waitingQueue->tail->numberOfPieces);
            if(camionQueue->tail) fprintf(debugFile,"CAMION QUEUE TAIL = %d %s %d\n", camionQueue->tail->arrivingTime, camionQueue->tail->recipeName, camionQueue->tail->numberOfPieces);
            */

            /*
            if(time == 97){
                printf("DEBUG\n");
            }
            */

            char recipeName[MAX_NAME];
            int numOfPieces=0;
            if (!scanf("%s %d", recipeName, &numOfPieces)) {
                printf("Failed getting orderName and numOfPieces\n");
                return 1;
            }
            Recipe *recipe = hash_table_cookbook_lookup(recipeName);
            if (recipe == NULL) {
                while (getchar_unlocked() != '\n');
                printf("rifiutato\n");
            }
            else {
                Order *newOrder = create_new_order(recipe->recipeName, numOfPieces, time, recipe->weight);
                if (check_order_exacutability(newOrder, recipe) == 0) {
                    queue_tail_insert(newOrder, waitingQueue);

                    /*
                    if(time == 114)
                        printf("ORDINE %d %s SOSPESO\n", newOrder->arrivingTime, newOrder->recipeName);


                    printf("Ordini sospesi:\n");
                    print_order_queue(waitingQueue);
                    printf("Ordini completati:\n");
                    print_order_queue(camionQueue);
                    */
                }
                else {
                    //if(time == 97) printf("Ordine %d %s completatocl\n", newOrder->arrivingTime, newOrder->recipeName);
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
            printf("%s", hash_table_cookbook_delete(recipeName, waitingQueue, camionQueue));
        }
        time++;
        for (int i = 0; i < TABLE_SIZE; i++) {
            warehouseIngredient *tmp = warehouse[i];
            while (tmp != NULL) {
                if (tmp->minimum != NULL && tmp->minimum->expirationDate <= time) {
                    delete_minimum(tmp);
                }
                tmp = tmp->next;
            }
        }

        if (time % refillFrequency == 0) {

            /*
            printf("ORDINI IN ATTESA:\n");
            print_order_queue_debug(waitingQueue);
            printf("ORDINI PRONTI:\n");
            print_order_queue_debug(camionQueue);
            */

            load_camion(camionQueue, camionCapacity);
        }
    }

    free_hash_tables();
    free_order_queue(waitingQueue);
    free_order_queue(camionQueue);
    return 0;
}








































































/*
void delete_batch(Batch **root_ptr) {
    Batch *root = *root_ptr;
    if (root == NULL) { //empty tree
        return;
    }
    if (root->left == NULL && root->right == NULL) { // leaf case
        if (root->parent == NULL) { //root node
            free(root);
            *root_ptr = NULL;
        }
        else if (root->parent->left == root) {
            root->parent->left = NULL;
            free(root);
        }
        else {
            root->parent->right = NULL;
            free(root);
        }
    }
    else if (root->left == NULL) { //only right child
        if (root->parent == NULL) { //root node
            *root_ptr = root->right;
            free(root);
        }
        else if (root->parent->left == root) {
            root->parent->left = root->right;
            free(root);
        }
        else {
            root->parent->right = root->right;
            free(root);
        }
    }
    else if (root->right == NULL) { //only left child
        if (root->parent == NULL) { //root node
            *root_ptr = root->left;
            free(root);
        }
        else if (root->parent->left == root) {
            root->parent->left = root->left;
            free(root);
        }
        else {
            root->parent->right = root->left;
            free(root);
        }
    }
    else { //both children
        Batch *tmp = root->right;
        while (tmp->left != NULL) {
            tmp = tmp->left;
        }
        root->expirationDate = tmp->expirationDate;
        root->quantity = tmp->quantity;
        delete_batch(&tmp);
    }
}

void cascade_delete(Batch *root) {
    if (root == NULL) {
        return;
    }
    cascade_delete(root->left);
    cascade_delete(root->right);
    delete_batch(&root, root);
}

Batch *expired_batch(Batch *root, int expirationDate) {
    Batch *result = NULL;

    while (root != NULL) {
        if (root->expirationDate > expirationDate) {
            result = root;
            root = root->left;
        }
        else {
            root = root->right;
        }
    }
    if (result != NULL) {
        cascade_delete(result->left);
    }
    return result;
}

void load_camion(orderQueue *camionQueue, int camionCapacity) {
    Order *tmp = camionQueue->head;
    orderQueue *loadingQueue = init_queue();
    while (tmp != NULL && camionCapacity >= tmp->weight) {
        camionCapacity -= tmp->weight;
        Order *orderToInsert = tmp;
        queue_sorted_loading_insert(&orderToInsert, loadingQueue);
        Order *toFree = tmp;
        tmp = tmp->next;
        camionQueue->head = tmp;
        free(toFree);
    }
    print_order_queue(camionQueue);
    print_order_queue(loadingQueue);

    if (loadingQueue->head == NULL) {
        printf("camioncino vuoto\n");
    }
    else {
        print_order_queue(loadingQueue);
    }

    free_order_queue(loadingQueue);
}
*/