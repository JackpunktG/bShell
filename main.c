#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER 2024
#define MAX_INGREDIENTS 30

struct Recipe
{
    char name[50];
    char ingredients[MAX_INGREDIENTS][40];
    char method[1024];
};

void RecipePrint(struct Recipe r)
{
    printf("%s\n", r.name);
    printf("Method: %s\n", r.method);
    printf("Ingredients:\n");
    for (int i = 0; i < MAX_INGREDIENTS && r.ingredients[i][0] != '\0'; i++)
    {
        printf(" - %s\n", r.ingredients[i]);
    }
    printf("\n");
}

void trim_newline(char *str)
{
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n')
        str[len - 1] = '\0';
}

void trim(char *str)
{
    char *start = str;
    char *end;

    // Trim leading spaces
    while (*start && isspace((unsigned char)*start))
    {
        start++;
    }

    // All spaces?
    if (*start == 0)
    {
        str[0] = '\0';
        return;
    }

    // Trim trailing spaces
    end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end))
    {
        end--;
    }

    // Write new null terminator
    *(end + 1) = '\0';

    // Shift trimmed string to the beginning
    memmove(str, start,
            end - start + 2); // +1 for '\0', +1 for end-start indexing
}

void inputRecipe()
{
    char input[100][256];
    int count = 0;

    while (count < 100 && fgets(input[count], 256, stdin))
    {
        if (input[count][0] == '\n')
            break;

        input[count][255] = '\0';
        count++;
    }

    struct Recipe new;
    char *name = input[0];
    trim(name);
    strcpy(new.name, name);
    int ingredientCount = 0;

    for (int i = 1; i < count; i++)
    {
        if (input[i][0] == '-')
        {
            char *data = input[i] + 1;
            trim(data);
            strcat(new.ingredients[ingredientCount++], data);
            strcat(new.method, " ");
            strcat(new.method, (data));
        }
        else
        {
            char *data = input[i];
            trim(data);
            if (i != 1)
                strcat(new.method, " ");
            strcat(new.method, data);
        }
    }

    FILE *file = fopen("data.csv", "a"); // "a" = append mode
    if (!file)
    {
        printf("Could not open file for appending!\n");
        return;
    }

    fprintf(file, "%s,%s,", new.name, new.method);
    for (int i = 0; i < ingredientCount; i++)
    {
        fprintf(file, "%s", new.ingredients[i]);
        if (i < ingredientCount - 1)
            fprintf(file, ",");
    }
    fprintf(file, "\n");
    fclose(file);
}

void loadRecipe(char *search)
{
    FILE *file = fopen("data.csv", "r");
    if (file == NULL)
    {
        printf("[ERROR] Could not open file!\n");
        return;
    }

    // printf("[INFO] Successfully opened file.\n");

    int recipeCount = 0;
    int ch;
    while ((ch = fgetc(file)) != EOF)
    {
        if (ch == '\n')
            recipeCount++;
    }
    rewind(file);
    printf("%d", recipeCount);
    struct Recipe recipes[recipeCount];
    char line[BUFFER];
    int index = 0;

    while (fgets(line, sizeof(line), file) && index < recipeCount)
    {
        trim_newline(line);

        char *token = strtok(line, ",");

        // Uppercase recipe name
        for (int i = 0; token[i]; i++)
        {
            token[i] = toupper(token[i]);
        }
        strcpy(recipes[index].name, token);

        token = strtok(NULL, ",");
        if (token)
        {
            strcpy(recipes[index].method, token);
        }
        else
        {
            strcpy(recipes[index].method, "EMPTY");
            printf("[WARN] No method found for recipe: %s\n",
                   recipes[index].name);
        }

        int n = 0;
        while ((token = strtok(NULL, ",")) != NULL && n < MAX_INGREDIENTS)
        {
            strcpy(recipes[index].ingredients[n], token);
            n++;
        }

        index++;
        printf("[INFO] Finished loading recipe %d\n\n", index);
    }

    fclose(file);
    // printf("[INFO] Finished reading file. Total recipes loaded: %d\n\n",

    char searchName[50];
    strcpy(searchName, search);
    trim(searchName);
    printf("\n%s\n", searchName);

    if (strcasecmp(searchName, "all") == 0)
    {

        for (int i = 0; i < index; i++)
        {
            RecipePrint(recipes[i]);
        }
    }
    else
    {
        for (int i = 0; i < index; i++)
        {
            if (strcasecmp(recipes[i].name, searchName) == 0)
                RecipePrint(recipes[i]);
        }
    }
}

int main()
{
    printf("**** Welcome to your cookbook ****");

    while (1)
    {
        printf("\nOptions:\n1 - Input\n2 - Search\n5 - exit\n");
        int option;

        scanf("%d", &option);
        getchar();

        if (option == 5)
            return 0;
        if (option == 1)
            inputRecipe();
        else if (option == 2)
        {
            printf("\nWrite 'all' if you want all\nRecipe name: ");
            char *nameSeek = NULL;
            size_t len = 0;
            getline(&nameSeek, &len, stdin);
            loadRecipe(nameSeek);
        }
    }

    return 0;
}
