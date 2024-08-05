#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define MAX_NAME 256
#define TABLE_SIZE 2000
#define MAX_LINE_LENGHT 256

typedef struct Batch {
    int expirationDate;
    int quantity;
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
    char recipeName[20];
    Ingredient *head;
    Ingredient *tail;
    struct Recipe *next;
} Recipe;

Recipe *cookbook[TABLE_SIZE];

Ingredient *create_new_ingredient(char *name, int quantity) {
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

void recipe_insert_at_tail(Recipe *recipe, char *name, int quantity) {
    //printf("Debug: Adding ingredient %s with quantity %d to recipe %s\n", name, quantity, recipe->recipeName); // Debug print
    Ingredient *newIngredient = create_new_ingredient(name, quantity);
    if (recipe->tail == NULL) { // empty list
        //printf("Debug: Recipe is empty, adding as first ingredient\n"); // Debug print
        recipe->head = newIngredient;
        recipe->tail = newIngredient;
    } else { //not empty list, adjustment needed
        //printf("Debug: Recipe is not empty, adding to the tail\n"); // Debug print
        recipe->tail->next = newIngredient; //the new ingredient is actually the next of the previous tail
        newIngredient->prev = recipe->tail; //the prev of the new ingredient is the previous tail
        recipe->tail = newIngredient; //the recipe's tail is the new ingredient
    }
    //printf("Debug: Ingredient %s added successfully\n", name); // Debug print
}

void print_list_of_ingredients_from_head(Recipe *recipe) { //despite tail insert print starting from the head
    Ingredient* current = recipe->head;
    while (current != NULL) {
        printf("%s %d - ", current->ingredientName, current->ingredientQuantity);
        current = current->next;
    }
}

unsigned int hash(const char *inputString) { //hash function
    unsigned long hash = 5381;
    int c;
    while ((c = *inputString++)) {
        hash = (hash << 5) + hash + c;
    }
    const unsigned int hash_value = hash % 2000;
    return hash_value;
}

void init_cookbook_hash_table() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        cookbook[i] = NULL;
    } // empty hash table
}

Recipe *hash_table_lookup(char *name) { //function to search elements inside the hash table
    const int index = hash(name);
    Recipe *tmp = cookbook[index];
    while (tmp != NULL && strncmp(tmp->recipeName, name, MAX_NAME) != 0) {
        tmp = tmp->next;
    }
    return tmp;
}

char *hash_table_insert(char name[]) {
    Recipe *newRecipe = malloc(sizeof(Recipe));
    if (newRecipe == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    strcpy(newRecipe->recipeName, name);
    newRecipe->head = NULL;
    newRecipe->tail = NULL;

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

char *remove_recipe(char name[]) {
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

    // Element found
    if (prev == NULL) { // deleating the head
        cookbook[index] = current->next;
    } else {
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

void init_warehouse_hash_table() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        warehouse[i] = NULL;
    } // hash table vuota
}

Batch *create_new_batch(int quantity, int expirationDate) {
    Batch *newBatch = (Batch *)malloc(sizeof(Batch));
    if (newBatch == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    newBatch->quantity = quantity;
    newBatch->expirationDate = expirationDate;
    newBatch->next = NULL;
    newBatch->prev = NULL;
    return newBatch;
}

void warehouse_sorted_insert(warehouseIngredient *ingredient, int quantity, int expirationDate) {
    Batch *newBatch = create_new_batch(quantity, expirationDate);
    if (ingredient->tail == NULL) { // empty list
        ingredient->head = newBatch;
        ingredient->tail = newBatch;
    } else {
        Batch *current = ingredient->head;
        while (current != NULL && current->expirationDate < expirationDate) { // check if the expiration date is less than the current batch
            current = current->next;
        }
        if (current == NULL) { // insert at tail
            ingredient->tail->next = newBatch;
            newBatch->prev = ingredient->tail;
            ingredient->tail = newBatch;
        } else if (current->expirationDate == expirationDate) { // insert in the same day updating the current quantity
            current->quantity += quantity;
            free(newBatch);
        } else if (current->prev == NULL) { // insert at head
            newBatch->next = ingredient->head;
            ingredient->head->prev = newBatch;
            ingredient->head = newBatch;
        } else { // insert in the middle
            newBatch->next = current;
            newBatch->prev = current->prev;
            current->prev->next = newBatch;
            current->prev = newBatch;
        }
    }
}

void hash_table_warehouse_insert(char name[], int quantity, int expirationDate) {
    int index = hash(name);
    warehouseIngredient *current = warehouse[index];

    // Check if the ingredient already exists
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            // Ingredient found, add the batch
            warehouse_sorted_insert(current, quantity, expirationDate);
            return; // Return after adding the batch
        }
        current = current->next;
    }

    // Ingredient not found, create a new one
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

    // Add the batch to the new ingredient
    warehouse_sorted_insert(newWarehouseIngredient, quantity, expirationDate);
}

void print_list_of_batches_from_head(warehouseIngredient *ingredient) {
    if (ingredient == NULL) {
        printf("Ingredient is NULL\n");
        return;
    }
    Batch *current = ingredient->head;
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

void check_expiration_date(int time) {
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
                    } else {
                        prev->next = current->next;
                    }
                    free(current);
                    if (prev == NULL) {
                        current = warehouse[i]; // Set current to the head of the list
                    } else {
                        current = prev->next; // Set current to the next ingredient after prev
                    }
                } else {
                    prev = current;
                    current = current->next;
                }
            }
        }
    }
}

int main() {
    init_cookbook_hash_table();
    init_warehouse_hash_table();
    int time=0;
    int camionCapacity=0;
    int refillFrequency=0;
    char line[MAX_LINE_LENGHT];
    char command[19];
    char recipeName[50];
    int quantity = 0;
    int expirationDate;

    scanf("%d %d", &camionCapacity, &refillFrequency);
    while (fgets(line, sizeof(line), stdin)) {
        char *cursor = line;
        if (sscanf(line, "%s", command) == 1) {
            time ++;
            check_expiration_date(time);
            //printf("Debug: Command = %s\n", command); // Debug print)
            if (strcmp(command, "aggiungi_ricetta") == 0) {
                cursor += strlen(command);
                //printf("Debug: Cursor after command = %s\n", cursor); // Debug print
                if (sscanf(cursor, "%s", recipeName) != 1) {
                    printf("Warning!\n");
                    return 1;
                }
                //printf("Debug: Recipe Name = %s\n", recipeName); // Debug print
                cursor += strlen(recipeName) + 1;


                Recipe *newRecipe = hash_table_lookup(recipeName);
                //printf("Debug: Recipe Lookup = %p\n", newRicetta); // Debug print

                if (newRecipe == NULL) {
                    printf("%s", hash_table_insert(recipeName));
                    newRecipe = hash_table_lookup(recipeName);
                    //printf("Debug: New Recipe Inserted = %p\n", newRicetta); // Debug print
                } else {
                    printf("ignorato\n");
                    continue;
                }

                char ingredientName[255];
                while (sscanf(cursor, "%s %d", ingredientName, &quantity) == 2) {
                    //printf("Debug: Ingredient = %s, Quantity = %d\n", nomeIngrediente, quantity); // Debug print
                    recipe_insert_at_tail(newRecipe, ingredientName, quantity);
                    cursor += strlen(ingredientName) +1;
                    //printf("Debug: Cursor after ingredient name = %s\n", cursor); // Debug print
                    while (*cursor == ' ') cursor++; // ignore spaces
                    cursor += snprintf(NULL, 0, "%d", quantity); // consider the quantity length as a string
                    //printf("Debug: Cursor after quantity = %s\n", cursor); // Debug print
                    while (*cursor == ' ') cursor++; // ignore spaces
                }
            }


            else if (strcmp(command, "rimuovi_ricetta") == 0) {
                cursor += strlen(command);
                if (sscanf(cursor, "%s", recipeName) != 1) {
                    printf("Warning!\n");
                    return 1;
                }
                printf("%s", remove_recipe(recipeName));
            }
            else if (strcmp(command, "rifornimento") == 0) {
                char name[MAX_NAME];
                cursor += strlen(command);
                while (sscanf(cursor, "%s %d %d", name, &quantity, &expirationDate) == 3) {
                    hash_table_warehouse_insert(name, quantity, expirationDate);
                    cursor += strlen(name) + 1 + snprintf(NULL, 0, "%d", quantity) + 1 + snprintf(NULL, 0, "%d", expirationDate) + 1;
                }
                printf("%s", "rifornito\n");
            }
        }
    }





    printf("%d\n", time);
    printf("%d\n%d\n", camionCapacity, refillFrequency);
    //print_cookbook();
    //print_warehouse();

    return 0;
}