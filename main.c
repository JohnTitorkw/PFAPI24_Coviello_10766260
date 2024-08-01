#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define MAX_NAME 256
#define TABLE_SIZE 2000
#define MAX_LINE_LENGHT 256

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

    strcpy(newIngredient->ingredientName, name);
    newIngredient->ingredientQuantity = quantity;
    newIngredient->next = NULL;
    newIngredient->prev = NULL;
    return newIngredient;
}

void recipe_insert_at_tail(Recipe *recipe, char *name, int quantity) {
    Ingredient *newIngredient = create_new_ingredient(name, quantity);
    if (recipe->tail == NULL) { // empty list
        recipe->head = newIngredient;
        recipe->tail = newIngredient;
    } else {//not empty list, adjustment needed
        recipe->tail->next = newIngredient; //the new ingredient is actually the next of the previous tail
        newIngredient->prev = recipe->tail; //the prev of the new ingredient is the previous tail
        recipe->tail = newIngredient; //the recipe's tail is the new ingredient
    }
}

void print_list_of_ingredients_from_head(const Recipe *recipe) { //despite tail insert print starting from the head
    Ingredient* current = recipe->head;
    while (current != NULL) {
        printf("%s %d - ", current->ingredientName, current->ingredientQuantity);
        current = current->next;
    }
}

unsigned int hash(const char *recipe) { //hash function
    unsigned long hash = 5381;
    int c;
    while ((c = *recipe++)) {
        hash = ((hash << 5) + hash) + c;
    }
    const unsigned int hash_value = hash % 2000;
    return hash_value;
}

void init_hash_table() {
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

//this is what I will do today


Recipe *hash_table_delete(const char *name) {
    const int index = hash(name);
    Recipe *tmp = cookbook[index];
    Recipe *prev = NULL;
    while (tmp != NULL && strncmp(tmp->recipeName, name, MAX_NAME) != 0) {
        prev = tmp;
        tmp = tmp->next;
    }
    if (tmp == NULL) return NULL;
    if (prev == NULL) {
        // deleting the head
        cookbook[index] = tmp->next;
    } else {
        prev->next = tmp->next;
    }
    return tmp;
}


int main() {
    init_hash_table();
    char line[MAX_LINE_LENGHT];
    char command[19];
    char recipeName[50];
    int quantity = 0;

    while (fgets(line, sizeof(line), stdin)) {
        if (sscanf(line, "%18s", command) == 1) {
            if (strcmp(command, "aggiungi_ricetta") == 0) {
                char* cursor = line + strlen("aggiungi_ricetta ");
                if (sscanf(cursor, "%49s", recipeName) != 1) {
                    printf("Warning!\n");
                    return 1;
                }
                cursor += strlen(recipeName) + 1;

                Recipe *newRicetta = hash_table_lookup(recipeName);

                if (newRicetta == NULL) {
                    printf("%s", hash_table_insert(recipeName));
                    newRicetta = hash_table_lookup(recipeName);
                } else {
                    printf("ignorato\n");
                    continue;
                }

                char nomeIngrediente[255];
                while (sscanf(cursor, "%49s %d", nomeIngrediente, &quantity) == 2) {
                    recipe_insert_at_tail(newRicetta, nomeIngrediente, quantity);
                    cursor += strlen(nomeIngrediente);
                    while (*cursor == ' ') cursor++; // Salta gli spazi
                    cursor += snprintf(NULL, 0, "%d", quantity); // Calcola la lunghezza della quantità come stringa
                    while (*cursor == ' ') cursor++; // Salta gli spazi
                }
            }
        }
    }

    print_cookbook();
    return 0;
}