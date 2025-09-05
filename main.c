#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <sys/types.h>

#define BUFFER 2024
#define MAX_INGREDIENTS 30

time_t t;
struct tm tm;
int currentDay;
int currentMonth;
int userRange;
char userName[30];

struct Recipe
{
    char name[50];
    char ingredients[MAX_INGREDIENTS][40];
    char method[1024];
};

struct Bday
{
    char name[50];
    int day;
    int month;
} Bday;

bool isInWeek(int day, int month, int range);
void trim(char *str);

void setConfig()
{
    printf("\n\nFirst time start up Config setting, welcome :)");
    printf("During the start up you would be greeted with the welcome screen. This will show you your upcoming event.\n\nUsername: ");

    bool correctInput = false;

    while (!correctInput)
    {
        char *buffer = NULL;
        size_t len = 0;
        ssize_t read;
        read = getline(&buffer, &len, stdin);
        if (read != -1)
        {
            if (read <= 30)
            {
                strcpy(userName, buffer);
                correctInput = true;
            }
            else
                printf("Username to long\nUsername: ");
        }
        else
            printf("Error entering - Try again\nUsername: ");
        free(buffer);
    }

    printf("\n\nOn the welcome page you'll see a list of Events coming up.\nHow many days in the future would you like to be shown? ");

    correctInput = false;
    while (!correctInput)
    {
        scanf("%d", &userRange);
        getchar();
        printf("Range: %d\n\nCorrect? (y/n)\n", userRange);
        int c;
        c = getchar();
        getchar();
        if (c == 'y' || c == 'Y')
            correctInput = true;
        if (!correctInput)
            printf("\nTry input again: ");
    }

    FILE *file = fopen("config.config", "w");
    if (file == NULL)
    {
        printf("[ERROR] Could not open file!\n");
        return;
    }
    fprintf(file, "// Username\n%s", userName); // no \n at the end as the getline will save it.
    fprintf(file, "// User Range\n%d\n", userRange);

    fclose(file);

    printf("\nConfiguration all done, you're ready to rock :)\nViel Spaß\n\n");
}

bool isConfig()
{
#define CONFIG_COUNT 2

    FILE *file = fopen("config.config", "r");
    if (file == NULL)
        return false;

    bool userConfig[CONFIG_COUNT] = {false};
    int configLine = 0;

    char line[BUFFER];
    while (fgets(line, sizeof(line), file))
    {
        trim(line); // Remove leading/trailing whitespace
        if (line[0] == '/' && line[1] == '/')
            continue; // Skip comment lines

        switch (configLine) // only counting uncommented lines
        {
        case 0:
            strcpy(userName, line);
            userConfig[1] = true;
            break;

        case 1:
            userRange = atoi(line);
            userConfig[0] = true;
            break;
        default:
            break;
        }
        configLine++;
    }
    fclose(file);

    for (int i = 0; i < CONFIG_COUNT; i++)
    {
        if (!userConfig[i])
            return false;
    }

    return true;
}

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

    for (int i = 0; i < 1024; i++)
    {
        if (new.method[i] == ',') // changing any ',' that made it into the method.
            new.method[i] = '.';
    }

    FILE *file = fopen("recipes.csv", "a");
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
    FILE *file = fopen("recipes.csv", "r");
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
        // printf("[INFO] Finished loading recipe %d\n\n", index);
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

void deleteRecipe(char *recipeP)
{
    char recipe[50];
    strcpy(recipe, recipeP);
    trim(recipe);
    printf("\n%s ??\n\n", recipe);
    const char *filename = "recipes.csv";
    const char *tempname = "recipes.csv.temp";

    FILE *file = fopen("recipes.csv", "r");
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

    struct Recipe recipes[recipeCount];
    char line[BUFFER];
    int index = 0;
    int ingredientCountBuffer[recipeCount];

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
        ingredientCountBuffer[index] = n;

        index++;
        // printf("[INFO] Finished loading recipe %d\n\n", index);
    }

    fclose(file);

    FILE *newFile = fopen(tempname, "w");
    if (!newFile)
    {
        perror("fopen temp");
        return;
    }
    int check = 0;
    for (int i = 0; i < recipeCount; i++)
    {

        if (strcasecmp(recipe, recipes[i].name) == 0)
        {
            check++;
        }
        else
        {
            fprintf(newFile, "%s,%s,", recipes[i].name, recipes[i].method);
            for (int j = 0; j < ingredientCountBuffer[i]; j++)
            {
                fprintf(newFile, "%s", recipes[i].ingredients[j]);
                if (j < ingredientCountBuffer[i] - 1)
                    fprintf(newFile, ",");
            }
            fprintf(newFile, "\n");
        }
    }
    fclose(newFile);

    if (check == 0)
    {
        printf("ERROR - recipes found with name: %s\n", recipe);
        remove(tempname);
        return;
    }
    else if (check > 1)
    {
        printf("WARNING - more than one recipe found with name: %s\n", recipe);
        printf("would you like to continue regardless? (y/n)\n");
        char option;
        scanf("%c", &option);
        getchar();
        if (option != 'y' || option != 'Y')
        {
            remove(tempname);
            return;
        }
    }

    remove(filename);
    rename(tempname, filename);

    printf("Recipe Removed\n");
}

void newBday()
{
    bool correct = false;
    char name[50] = {0};
    int date = 0, month = 0;

    while (!correct)
    {
        printf("\n<name> - <day.monthDigit>\n");
        char *bday = NULL;
        size_t len = 0;

        getline(&bday, &len, stdin);

        char *token = strtok(bday, "-");
        if (token)
        {
            strcpy(name, token);
            trim(name);
            token = strtok(NULL, "-");
            if (token)
            {
                trim(token);
                char *date_st = strtok(token, ".");
                char *month_st = strtok(NULL, ".");

                if (date_st && month_st)
                {
                    date = atoi(date_st);
                    month = atoi(month_st);
                    printf("%s - %d.%d\nRichtig? (y/n)\n", name, date, month);
                    char option;
                    scanf("%c", &option);
                    getchar();
                    if (option == 'y' || option == 'Y')
                        correct = true;
                }
                else
                {
                    printf("Error...\nInput again\n");
                }
            }
            else
            {
                printf("Error...\nInput again\n");
            }
        }
        else
        {
            printf("Error...\nInput again\n");
        }
    }

    FILE *file = fopen("bdays.csv", "a");
    if (!file)
    {
        printf("Could not open file for appending!\n");
        return;
    }
    fprintf(file, "%d,%d,%s\n", date, month, name);
    fclose(file);
}

void printBday(int option, int range)
{
    FILE *file = fopen("bdays.csv", "r");
    if (!file)
    {
        printf("Could not open file for reading!\n");
        return;
    }

    int bdayCount = 0;
    int ch;
    while ((ch = fgetc(file)) != EOF)
    {
        if (ch == '\n')
            bdayCount++;
    }
    rewind(file);

    struct Bday bdays[bdayCount];
    memset(bdays, 0, sizeof(bdays));
    char line[20];
    int index = 0;
    while (fgets(line, sizeof(line), file) && index < bdayCount)
    {
        char *token = strtok(line, ",");

        bdays[index].day = atoi(token);

        token = strtok(NULL, ",");

        bdays[index].month = atoi(token);

        token = strtok(NULL, ",");
        trim(token);
        strcpy(bdays[index].name, token);

        if (option == 1)
            printf("\n%d.%d - %s", bdays[index].day, bdays[index].month, bdays[index].name);
        else if (option == 2)
        {
            if (isInWeek(bdays[index].day, bdays[index].month, range))
                printf("\n%d.%d - %s", bdays[index].day, bdays[index].month, bdays[index].name);
        }
        index++;
    }
    fclose(file);
    printf("\n");
}

void deleteBday(char *nameP)
{
    char name[50];
    strcpy(name, nameP);
    trim(name);
    // printf("\n%s ??\n\n", name);
    const char *filename = "bdays.csv";
    const char *tempname = "bdays.csv.temp";

    FILE *file = fopen(filename, "r");
    if (!file)
    {
        printf("Could not open file for reading!\n");
        return;
    }

    int bdayCount = 0;
    int ch;
    while ((ch = fgetc(file)) != EOF)
    {
        if (ch == '\n')
            bdayCount++;
    }
    rewind(file);

    struct Bday bdays[bdayCount];
    memset(bdays, 0, sizeof(bdays));
    char line[20];
    int index = 0;
    while (fgets(line, sizeof(line), file) && index < bdayCount)
    {
        char *token = strtok(line, ",");

        bdays[index].day = atoi(token);

        token = strtok(NULL, ",");

        bdays[index].month = atoi(token);

        token = strtok(NULL, ",");
        trim(token);
        strcpy(bdays[index].name, token);

        index++;
    }
    fclose(file);

    FILE *newFile = fopen(tempname, "w");
    if (!newFile)
    {
        perror("fopen temp");
        return;
    }

    int check = 0;

    for (int i = 0; i < index; i++)
    {

        if (strcasecmp(name, bdays[i].name) == 0)
        {
            check++;
        }
        else
            fprintf(newFile, "%d,%d,%s\n", bdays[i].day, bdays[i].month, bdays[i].name);
    }
    fclose(newFile);

    if (check == 0)
    {
        printf("ERROR - noboday found named: %s\n", name);
        remove(tempname);
        return;
    }
    else if (check > 1)
    {
        printf("WARNING - more than one person found with name: %s\n", name);
        printf("would you like to continue regardless? (y/n)\n");
        char option;
        scanf("%c", &option);
        getchar();
        if (option != 'y' || option != 'Y')
        {
            remove(tempname);
            return;
        }
    }

    remove(filename);
    rename(tempname, filename);

    printf("Event Removed\n");
}

bool isLeapYear(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int daysInMonth(int month, int year)
{
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2)
        return isLeapYear(tm.tm_year + 1900) ? 29 : 28;
    return days[month - 1];
}

bool isInWeek(int day, int month, int range)
{
    int year = tm.tm_year + 1900;
    int d = currentDay;
    int m = currentMonth;
    int r = range;

    while (r >= 0)
    {
        if (d == day && m == month)
            return true;
        d++;
        if (d > daysInMonth(m, year))
        {
            d = 1;
            m++;
            if (m > 12)
            {
                m = 1;
                year++;
            }
        }
        r--;
    }
    return false;
}

int main()
{
    t = time(NULL);
    tm = *localtime(&t);
    currentDay = tm.tm_mday;
    currentMonth = tm.tm_mon + 1;
    int dayOfWeek = tm.tm_wday; // 0=Sunday, 1=Monday, ..., 6=Saturday

    printf("\n\n**** Welcome to bShell ****");

    if (!isConfig())
        setConfig();

    printf("\n\n%s, Happy ", userName);
    switch (dayOfWeek)
    {
    case 0:
        printf("Sunday - why are you booting me up... chill the fuck out");
        break;
    case 1:
        printf("Monday - Have a cracking start to the week");
        break;
    case 2:
        printf("Tuesday - Now that reall works begins");
        break;
    case 3:
        printf("Wednesday - #Humpday");
        break;
    case 4:
        printf("Thursday - Final push you, got this");
        break;
    case 5:
        printf("T.G.I.Friday");
        break;
    case 6:
        printf("Saturday - Get out and have a cracking weekend");
        break;
    default:
        printf("... Ops I don't know what day it is");
    }
    printf("\n\nEvents coming up soon in:");
    printBday(2, userRange);

    while (1)
    {
        printf("\nOptions:\n1 - Search Recipe\n2 - Add Recipe\n3 - Remove Recipe\n4 - Events\n5 - Add Event\n6 - Remove Event\n9 - exit\n");
        int option;

        scanf("%d", &option);
        getchar();

        if (option == 9)
        {
            printf("By..byeee :)\n");
            return 0;
        }

        if (option == 1)
        {
            printf("\nWrite 'all' if you want all\nRecipe name: ");
            char *seek = NULL;
            size_t len = 0;
            getline(&seek, &len, stdin);
            loadRecipe(seek);
        }
        else if (option == 2)
        {
            inputRecipe();
        }
        else if (option == 3)
        {
            printf("\nRecipe to delete: ");
            char *seek = NULL;
            size_t len = 0;
            getline(&seek, &len, stdin);
            deleteRecipe(seek);
        }
        else if (option == 4)
            printBday(1, 0);
        else if (option == 5)
            newBday();
        else if (option == 6)
        {
            printf("\nEvent to delete: ");
            char *seek = NULL;
            size_t len = 0;
            getline(&seek, &len, stdin);
            deleteBday(seek);
        }
    }

    return 0;
}
