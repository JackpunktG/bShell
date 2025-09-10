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
#define MAX_ARGS 5

time_t t;
struct tm tm;
int currentDay;
int currentMonth;
int currentHour;
int currentMinute;
char currentMonthSt[10];
int currentYear;
int userRange;
char userName[30];
bool running;

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
char *future_month(int month);
int daysInMonth(int month, int year);
void random_gen(char *out);

void setConfig()
{
    char mkdirectory[256];
    snprintf(mkdirectory, sizeof(mkdirectory),
             "mkdir -p config data/recipes data/events data/task_manager/%d",
             currentYear);

    int made = system(mkdirectory);

    if (made != 0)
        printf("ERROR - directory system not made\n");

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

    FILE *file = fopen("config/user.config", "w");
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

    FILE *file = fopen("config/user.config", "r");
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

void random_TaskID(char *out)
{
    char filename[50];
    snprintf(filename, sizeof(filename), "data/task_manager/%d/.%dactiveIDs", currentYear, currentYear);

    char id[5];
    bool unique = false;

    while (!unique)
    {
        random_gen(id);
        unique = true;

        FILE *file = fopen(filename, "r");
        if (file)
        {
            char line[6];
            while (fgets(line, sizeof(line), file))
            {
                trim(line);
                if (strcmp(line, id) == 0)
                {
                    unique = false;
                    break;
                }
            }
            fclose(file);
        }
    }

    FILE *file = fopen(filename, "a");
    if (file)
    {
        fprintf(file, "%s\n", id);
        fclose(file);
    }

    strcpy(out, id);
}

void random_gen(char *out)
{
    out[0] = 'A' + rand() % 26;
    out[2] = 'A' + rand() % 26;
    out[1] = '0' + rand() % 10;
    out[3] = '0' + rand() % 10;
    out[4] = '\0';
}

int *task_Date(int range)
{
    int *taskDate = malloc(3 * sizeof(int));
    int year = currentYear;
    int d = currentDay;
    int m = currentMonth;
    int r = range;

    while (r > 0)
    {
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
    taskDate[0] = d;
    taskDate[1] = m;
    taskDate[2] = year;
    return taskDate;
}

void check_TaskDirectory(int *date)
{
    char *monthStr = future_month(date[1]);
    char filename[50];
    snprintf(filename, sizeof(filename), "data/task_manager/%d/%s.txt", date[2], monthStr);

    bool new = false;
    FILE *file = fopen(filename, "a");
    if (file == NULL)
    {
        char mkdirectory[128];
        snprintf(mkdirectory, sizeof(mkdirectory),
                 "mkdir -p data/task_manager/%d && touch data/task_manager/%d/%s.txt",
                 date[2], date[2], monthStr);
        system(mkdirectory);

        file = fopen(filename, "a"); // re-use the same variable
        if (file == NULL)
        {
            printf("ERROR creating task directory && file\n");
            free(monthStr);
            return;
        }

        new = true;
    }
    if (new)
        fprintf(file, "// Here is the log for %s %d\n", monthStr, date[2]);
    free(monthStr);
    fclose(file);
}

void print_task(int r)
{
    int range = r;

    while (range >= 0)
    {
        int *searchDate = task_Date(range);

        char *monthStr = future_month(searchDate[1]);
        char filename[50];
        snprintf(filename, sizeof(filename), "data/task_manager/%d/%s.txt", searchDate[2], monthStr);

        FILE *file = fopen(filename, "r");
        if (file == NULL)
        {
            range--;
            continue;
        }

        bool check = false;
        bool printing = false;
        char line[BUFFER];

        while (fgets(line, sizeof(line), file))
        {

            trim(line);
            if (check)
            {
                char *comma = strchr(line, ',');
                if (comma)
                {
                    char day_str[10];
                    size_t len = comma - line;
                    strncpy(day_str, line, len);
                    day_str[len] = '\0';
                    int day = atoi(day_str);
                    if (day <= searchDate[0])
                    {
                        if (!printing)
                            printf("\n****************************\n");
                        printing = true;
                    }
                }
                check = false;
            }

            if (line[0] == '*' && line[1] == '*')
            {
                if (printing)
                {
                    printf("****************************\n");
                    printing = false;
                }
                else
                    check = true;
            }

            if (printing)
            {
                printf("%s\n", line);
            }
        }
        fclose(file);

        if (searchDate[1] == currentMonth)
        {
            free(searchDate);
            free(monthStr);
            break;
        }
        else
        {
            range = -searchDate[0];
            free(searchDate);
            free(monthStr);
        }
    }
}

void input_task(char *input)
{

    char *token = strtok(input, "|");
    trim(token);
    char *task = malloc(strlen(token) + 1);
    strcpy(task, token);

    token = strtok(NULL, "|");
    trim(token);
    int how_many_days = atoi(token);

    int *taskDate = task_Date(how_many_days);
    check_TaskDirectory(taskDate);

    char *monthStr = future_month(taskDate[1]);
    char filename[50];
    snprintf(filename, sizeof(filename), "data/task_manager/%d/%s.txt", taskDate[2], monthStr);
    FILE *file = fopen(filename, "a");

    fprintf(file, "\n****************************\n");
    fprintf(file, "%d,", taskDate[0]);

    char id[5];
    random_TaskID(id);

    token = strtok(NULL, "|");
    if (token != NULL)
    {
        trim(token);
        char *time = malloc(strlen(token) + 1);
        strcpy(time, token);
        fprintf(file, " %s", time);
        fprintf(file, "\n%s\n#APPOINTMENT\t\t%s\n", task, id);
        free(time);
    }
    else
    {
        fprintf(file, "\n%s\n#WORKING\t\t%s\n", task, id);
    }
    fprintf(file, "****************************\n");
    fclose(file);
    free(task);
    free(taskDate);
}

void RecipePrint(struct Recipe r)
{
    printf("\n%s\n", r.name);
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

    printf("Input Recipe Mode!\n You know the drill\n\n");

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

    FILE *file = fopen("data/rerecipes/recipes.csv", "a");
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

    printf("Recipe input successful!\n\n");
}

void loadRecipe(char *search)
{
    FILE *file = fopen("data/recipes/recipes.csv", "r");
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
    // printf("%d", recipeCount);
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

    // printf("\n%s\n", searchName);

    if (strcasecmp(searchName, "all") == 0)
    {

        for (int i = 0; i < index; i++)
        {
            RecipePrint(recipes[i]);
        }
    }
    else
    {
        bool printed = false;
        for (int i = 0; i < index; i++)
        {
            if (strcasecmp(recipes[i].name, searchName) == 0)
            {
                RecipePrint(recipes[i]);
                printed = true;
            }
        }
        if (!printed)
            printf("Recipe not found with name %s\n", search);
    }
}

void deleteRecipe(char *recipeP)
{
    char recipe[50];
    strcpy(recipe, recipeP);
    trim(recipe);
    // printf("\n%s ??\n\n", recipe);
    const char *filename = "data/recipes/recipes.csv";
    const char *tempname = "data/recipes/recipes.csv.temp";

    FILE *file = fopen("data/recipes/recipes.csv", "r");
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

    FILE *file = fopen("data/events/events_yearly.csv", "a");
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
    FILE *file = fopen("data/events/events_yearly.csv", "r");
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
    printf("\n\n");
}

void deleteBday(char *nameP)
{
    char name[50];
    strcpy(name, nameP);
    trim(name);
    // printf("\n%s ??\n\n", name);
    const char *filename = "data/events/events_yearly.csv";
    const char *tempname = "data/events/events_yearly.csv.temp";

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

void welcome()
{
    int dayOfWeek = tm.tm_wday; // 0=Sunday, 1=Monday, ..., 6=Saturday

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
}

char *read_input()
{
    char *buffer;
    size_t size = 0;
    ssize_t len;
    len = getline(&buffer, &size, stdin);
    if (len > 0)
    {
        if (buffer[len - 1] == '\n')
            buffer[len - 1] = '\0';
        return buffer;
    }
    else
    {
        printf("ERROR whilst reading input");
        free(buffer);
        return NULL;
    }
}

char **process_input(char *input)
{

    char **args = malloc(sizeof(char *) * MAX_ARGS);
    int i = 0;
    char *ptr = input;

    while (*ptr && i < MAX_ARGS)
    {
        while (isspace((unsigned char)*ptr))
            ptr++; // skip space in between
        if (*ptr == '"')
        {
            ptr++; // Skipping starting quotes
            char *start = ptr;
            while (*ptr && *ptr != '"')
                ptr++;
            size_t len = ptr - start;
            args[i] = malloc(len + 1);
            strncpy(args[i], start, len);
            args[i][len] = '\0';
            if (*ptr == '"')
                ptr++; // Skip closing quote
        }
        else if (*ptr)
        {
            char *start = ptr;
            while (*ptr && *ptr != ' ')
                ptr++;
            size_t len = ptr - start;
            args[i] = malloc(len + 1);
            strncpy(args[i], start, len);
            args[i][len] = '\0';
        }
        else
        {
            printf("ERROR parsing args\n");
            break;
        }
        i++;
    }

    // Set unused args to NULL
    for (; i < MAX_ARGS; i++)
        args[i] = NULL;

    return args;
}

void run_args(char **args)
{
    if (strcmp(args[0], "recipe") == 0)
    {
        if (strcmp(args[1], "-n") == 0)
            inputRecipe();
        else if (strcmp(args[1], "-d") == 0)
            if (args[2] != NULL)
                deleteRecipe(args[2]);
            else
                printf("ERROR: Args Required\n");
        else if (strcmp(args[1], "-p") == 0)
        {
            if (args[2] == NULL)
                loadRecipe("all");
            else
                loadRecipe(args[2]);
        }

        else
            printf("recipe command not vaild\n");
    }
    else if (strcmp(args[0], "task") == 0)
    {
        if (strcmp(args[1], "-n") == 0)
            input_task(args[2]);
        else if (strcmp(args[1], "-p") == 0)
        {
            int range = atoi(args[2]);
            print_task(range);
        }
        else
            printf("ERROR processing task args\n");
    }
    else if (strcmp(args[0], "event") == 0)
    {
        if (strcmp(args[1], "-n") == 0)
            newBday();
        else if (strcmp(args[1], "-d") == 0)
            if (args[2] != NULL)
                deleteBday(args[2]);
            else
                printf("ERROR: Args Required\n");
        else if (strcmp(args[1], "-p") == 0)
        {
            if (args[2] == NULL)
                printBday(1, 0);
            else
            {
                int inputRange = atoi(args[2]);
                if (inputRange > 0)
                    printBday(2, inputRange);
                else
                    printf("Error in getting Event Range\n");
            }
        }
        else if (strcmp(args[1], "-pr") == 0)
        {
            if (args[2] == NULL)
                printBday(1, 0);
        }
        else
            printf("event command not valid\n");
    }
    else if (strcmp(args[0], "welcome") == 0)
        welcome();
    else if (strcmp(args[0], "q") == 0 || strcmp(args[0], "exit") == 0)
        running = false;
    else
        printf("Invalid Command\n");
}

char *future_month(int month)
{
    char *result = malloc(10); // 9 chars for "September" + 1 for '\0'
    if (!result)
        return NULL;

    switch (month)
    {
    case 1:
        strcpy(result, "January");
        break;
    case 2:
        strcpy(result, "February");
        break;
    case 3:
        strcpy(result, "March");
        break;
    case 4:
        strcpy(result, "April");
        break;
    case 5:
        strcpy(result, "May");
        break;
    case 6:
        strcpy(result, "June");
        break;
    case 7:
        strcpy(result, "July");
        break;
    case 8:
        strcpy(result, "August");
        break;
    case 9:
        strcpy(result, "September");
        break;
    case 10:
        strcpy(result, "October");
        break;
    case 11:
        strcpy(result, "November");
        break;
    case 12:
        strcpy(result, "December");
        break;
    default:
        strcpy(result, "Unknown");
        break;
    }
    return result;
}

void setMonthSt()
{
    switch (currentMonth)
    {
    case 1:
        strcpy(currentMonthSt, "January");
        break;
    case 2:
        strcpy(currentMonthSt, "February");
        break;
    case 3:
        strcpy(currentMonthSt, "March");
        break;
    case 4:
        strcpy(currentMonthSt, "April");
        break;
    case 5:
        strcpy(currentMonthSt, "May");
        break;
    case 6:
        strcpy(currentMonthSt, "June");
        break;
    case 7:
        strcpy(currentMonthSt, "July");
        break;
    case 8:
        strcpy(currentMonthSt, "August");
        break;
    case 9:
        strcpy(currentMonthSt, "September");
        break;
    case 10:
        strcpy(currentMonthSt, "October");
        break;
    case 11:
        strcpy(currentMonthSt, "November");
        break;
    case 12:
        strcpy(currentMonthSt, "December");
        break;
    default:
        strcpy(currentMonthSt, "Unknown");
        printf("ERROR - Month unknown!!\n");
        break;
    }
}

void set_time_date()
{
    t = time(NULL);
    tm = *localtime(&t);
    currentDay = tm.tm_mday;
    currentMonth = tm.tm_mon + 1;
    setMonthSt();
    currentYear = tm.tm_year + 1900;
    currentHour = tm.tm_hour;
    currentMinute = tm.tm_min;
}

int main()
{
    srand(time(NULL));
    set_time_date();

    running = true;

    printf("\n\n**** Welcome to bShell ****");

    if (!isConfig())
        setConfig();

    welcome();
    printf("Todays tasks:\n");
    print_task(0);
    printf("You can do it :)\n\n");
    while (running)
    {
        printf("bShell> ");
        fflush(stdout);
        char *line = read_input();
        set_time_date();
        char **args = process_input(line);

        /*
                printf("%s\n", line);

                for (int i = 0; i < 5 && args[i] != NULL; i++)
                {
                    printf("%s\n", args[i]);
                }
        */
        if (args[0] != NULL)
            run_args(args);

        free(line);
        free(args);
    }
    return 0;
}
/*
     bool printing = false;
        char line[BUFFER];
        while (fgets(line, sizeof(line), file))
        {
            trim(line);
            if (line[0] == '*' && line[1] == '*')
            {
                if (!printing) printf("\n");
                printf("%s\n", line);
                if (printing) printing = false;
                else printing = true;
            }
        }
            */