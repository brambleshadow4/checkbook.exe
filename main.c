// https://www.sqlite.org/c3ref/funclist.html

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "printcolor.h"
#include "sqlite3.h"
#include "main.h"
#include "consoleOps.c"

int main()
{
    sqlite3_open("finances.db", &db);

    sqlite3_exec(db,
        "CREATE TABLE accounts ( "
        "ID INTEGER PRIMARY KEY, "
        "name varchar(255) NOT NULL, "
        "value bigint, "
        "valid tinyint)" ,
        null, null, null
    );

    sqlite3_exec(db,
        "CREATE TABLE transactions ( "
        "ID INTEGER PRIMARY KEY, "
        "account INTEGER, "
        "amount bigint, "
        "date varchar(255), "
        "message varchar(255))",
        null, null, null
    );

    sqlite3_exec(db, "SELECT * FROM accounts WHERE valid = 1", &loadAccountCallback, null, null);

    //FreeConsole();
    //SetConsoleTitle("Checkbook.exe");

    printf("Checkbook.exe\r\n");
    printf("Type \"help\" to learn more about the availiable commands\r\n");

    int doLoop = 1;

    while(doLoop)
    {
        doLoop = commandLoop();
    }

    while(accounts_count > 0)
    {
        accounts_count--;

        free(accounts[accounts_count]->name);
        free(accounts[accounts_count]);
    }

    sqlite3_close(db);
    return 0;
}

int commandLoop()
{
    printf("\r\ncheckbook> ");
    fgets(commandBuffer, 1000, stdin);
    next_arg_ptr = commandBuffer;

    char* arg0 = next_arg();

    int i;
    for(i = 0; arg0[i]; i++)
    {
        arg0[i] = tolower(arg0[i]);
    }

    if(strcmp(arg0, "s") == 0 || strcmp(arg0, "sum") == 0)
    {
        commandSum();
        return 1;
    }

    if(strcmp(arg0, "new") == 0)
    {
        commandNew();
        return 1;
    }

    if(strcmp(arg0, "l") == 0 || strcmp(arg0, "last") == 0)
    {
        commandLast();
        return 1;
    }

    if(strcmp(arg0, "h") == 0 || strcmp(arg0, "help") == 0)
    {
        printcf(
            "\r\nThis is a list of all commands included in the program\r\n"
            //"For a more user friendly introduction to the software, type \"help intro\"\r\n"

            "\r\n"
            "  e    - Shorthand for \"edit\"\r\n"
            "  edit - Launches an interactive tool to edit/delete a transaction.\r\n"
            "  h    - Shorthand for \"help\"\r\n"
            "  help - Displays this help dialog\r\n"
            "  l    - Shorthand for \"last\"\r\n"
            "  last - Lists the last few transactions.\r\n"
            "         last %C<n>%N displays the last %C<n>%N transactions\r\n"
            "         last %C<n> <x1> <x2>%N ... %C<xn>%N displays the last %C<n>%N transactions from accounts %C<x1>%N, %C<x2>%N, ... , %C<xn>%N\r\n"
            "  q    - Shorthand for \"quit\"\r\n"
            "  quit - Quits the program.\r\n"
            "  s    - Shorthand for \"sum\"\r\n"
            "  sum  - Lists all accounts, their current values, and a total sum of all account values.\r\n"
            "\r\n"
            "To record a transaction, use the following command:\r\n"
            "    %C<acount> %G<amount> %M<date> %Y<message>\r\n"
            "\r\n"
            "    %C<account>%N is the name of the account to add/remove money to (try 'sum' to )\r\n"
            "    %G<amount>%N is the amount to add/remove. It must be a '+' or '-' "
                "followed by an integer or a two digit decimal e.g. %C+42 %Nor %C-6.28\r\n"
            "    %M<date>%N is the date in mm/dd format, or mm/dd/yyyy format.\r\n"
            "    %Y<message>%N is the message assoicated with the transaction. If it contains spaces, it must be enclosed in \"quotes\"\r\n"
        );
        return 1;
    }

    if(strcmp(arg0, "e") == 0 || strcmp(arg0, "edit") == 0)
    {
        commandEdit();
        return 1;
    }

    for(i=0; i < accounts_count; i++)
    {
        if(strcmp(arg0, accounts[i]->name) == 0)
        {
            commandTransaction(accounts[i]);
            return 1;
        }
    }

    if(strcmp(arg0, "q") == 0 || strcmp(arg0, "quit") == 0)
        return 0;
    if(strcmp(arg0, "recalibrate") == 0){

        commandRecalibrate();
        return 1;
    }
    else
        printcf("%RInvalid Syntax\r\n%NFor more information about the available commands, type \"help\"\r\n");
    return 1;
}


void commandLast()
{
    //figure out how many rows to fetch
    char* count = next_arg();
    int numberOfTransactions = 100;

    if(count != null)
    {
        numberOfTransactions = atoll(count);
        if(numberOfTransactions == 0)
        {
            printf_yellow("Invalid count; using 100 instead\n");
            numberOfTransactions = 100;
        }
    }

    // build the sql statement.
    char* whereClause = " WHERE account IN (SELECT account FROM accounts WHERE valid='1')";
    int defaultWhere = 1;
    char accountNum[100];
    char* accountName = next_arg();
    struct account* acc = getAccountWithName(accountName);

    long long totalSum = 0;

    while(acc)
    {
        itoa(acc->id, accountNum, 10);

        if(defaultWhere)
        {
            whereClause = new_string_from_parts(3," WHERE account='", accountNum, "'");
            defaultWhere = 0;
        }
        else
        {
            char* newWhere = new_string_from_parts(4, whereClause, " OR account='", accountNum, "'");
            free(whereClause);
            whereClause = newWhere;
        }

        totalSum += acc->value;

        accountName = next_arg();
        acc = getAccountWithName(accountName);
    }

    char numberOfTransactionsString[100];
    sprintf(numberOfTransactionsString, "%lld", numberOfTransactions);

    char* sql = new_string_from_parts(5, "SELECT * FROM transactions", whereClause, " ORDER BY date DESC LIMIT ", numberOfTransactionsString, ";");

    #ifdef PRINT_SQL
        printf_magenta("%s\n", sql);
    #endif


    int i;

    if(!defaultWhere)
        free(whereClause);
    else
    {
        // calculate sum;
        totalSum = 0;
        for(i=0; i< accounts_count; i++)
        {
            totalSum += accounts[i]->value;
        }
    }

    transactions = malloc(sizeof(struct transaction*) * numberOfTransactions);

    numberOfTransactions = 0;
    sqlite3_exec(db, sql, &loadTransactionsCallback, &numberOfTransactions, null);

    print_transaction_table(numberOfTransactions, -1, &totalSum);

    for(i=numberOfTransactions-1; i >=0; i--)
    {
        free(transactions[i]->date);
        free(transactions[i]->message);
    }

    free(sql);
    free(transactions);
}

void commandNew()
{
    char* newAccountName = next_arg();

    int i;

    if(newAccountName == null || newAccountName[0] == null)
    {
        printf_red("Invalid syntax: Please provide a new account name.\r\n");
        return;
    }

    for(i=0; newAccountName[i]; i++)
    {
        if(!isalnum(newAccountName[i]) && newAccountName[i] != '_')
        {
            printf_red("The new account name must only include alphanumeric characters\r\n");
            return;
        }
    }


    int match = 0;

    while(match < accounts_count && strcmp(newAccountName, accounts[match]->name) != 0)
        match++;

    if(match == MAX_ACCOUNTS)
    {
        printf("Cannot create any more accounts: you already have the maximum number of accounts allowed.");
    }
    else if(match == accounts_count)
    {
        //add the account to the db
        char* sql1 = "INSERT INTO accounts (name, value, valid) VALUES ('";
        char* sql2 = "', 0, 1)";
        char* sql = (char*) malloc(strlen(sql1) + strlen(sql2) + strlen(newAccountName)+1);

        sql[0] = '\0';
        strcat(sql, sql1);
        strcat(sql, newAccountName);
        strcat(sql, sql2);

        sqlite3_exec(db, sql, null, null, null);
        free(sql);

        //get the account's db id
        char* sql3 = "SELECT ID FROM accounts WHERE valid=1 AND name='";
        sql = (char*) malloc(strlen(sql3) + 2 + strlen(newAccountName));

        sql[0] = '\0';
        strcat(sql, sql3);
        strcat(sql, newAccountName);
        strcat(sql, "'");

        int accountId = -1;

        sqlite3_exec(db, sql, &getIdCallback, &accountId, null);
        free(sql);

        //create the account in memory
        accounts[accounts_count] = malloc(sizeof(struct account));
        accounts[accounts_count]->id = accountId;
        accounts[accounts_count]->value = 0;
        accounts[accounts_count]->name = (char*) malloc(strlen(newAccountName)+1);
        strcpy(accounts[accounts_count]->name, newAccountName);

        accounts_count++;
    }
    else
    {
        printf("Cannot create ");
        printf_cyan(newAccountName);
        printf(": name already exists.\r\n");
    }

    //free(sql);
    return;
}

void commandSum()
{
    int accountCol = 5;
    int moneyCol = 0;

    int i;
    int l;
    long long sum = 0;

    for(i =0; i< accounts_count; i++)
    {
        l= strlen(accounts[i]->name);
        if(l > accountCol)
            accountCol = l;

        l = print_money_length(accounts[i]->value);
        if(l > moneyCol)
            moneyCol = l;
        sum += accounts[i]->value;
    }

    l = print_money_length(sum);
    if(l > moneyCol)
        moneyCol = l;

    for(i =0; i< accounts_count; i++)
    {
        l = strlen(accounts[i]->name);
        printf_cyan(accounts[i]->name);
        while(l <= accountCol)
        {
            printf(" ");
            l++;
        }

        l = print_money_length(accounts[i]->value);
        while(l <= moneyCol)
        {
            printf(" ");
            l++;
        }
        print_money(accounts[i]->value);
        printf("\r\n");
    }

    for(l = 0; l <= accountCol + moneyCol + 1; l++)
    {
        printf("=");
    }

    printf_yellow("\r\nTOTAL");
    for(l = 5 + print_money_length(sum); l <= accountCol + moneyCol + 1; l++)
    {
        printf(" ");
    }

    print_money(sum);
    printf("\r\n");
}

void commandEdit()
{
    int maxTransactions = 5;
    sqlite3_exec(db, "SELECT COUNT(*) FROM transactions WHERE account IN "
        "(SELECT account FROM accounts WHERE valid='1')", &getIdCallback, &maxTransactions, null);

    int offset = 0;
    int loaded = -20;

    int numberOfTransactions = 0;
    int transactionId = -1;
    int i = 0;

    int refresh = 1;

    short isDeleting = 0;

    char sql[240];
    while(1)
    {
        if(offset < loaded || offset >= loaded + 20)
        {
            loaded = offset - (offset % 20);

            strcpy(sql,"SELECT * FROM transactions WHERE account IN "
                "(SELECT account FROM accounts WHERE valid='1') ORDER BY date DESC LIMIT 20 OFFSET ");
            char* offsetPtr = sql + strlen(sql);
            itoa(loaded,offsetPtr, 10);

            transactions = malloc(sizeof(struct transaction*) * 20);
            numberOfTransactions = 0;
            sqlite3_exec(db, sql, &loadTransactionsCallback, &numberOfTransactions, null);

            #ifdef PRINT_SQL
                printf_magenta("%s\n",sql);
            #endif
        }

        if(refresh)
        {
            clearScreen();
            print_transaction_table(numberOfTransactions, offset-loaded, null);
            printf("Use Up + Down arrows to select an entry to edit.\n");
            printf("Press Enter to edit\n");
            printf("Press Esc to cancel\n");
            refresh = 0;
        }

        int key = getKeypress();

        if(key == KEYCODE_UPARROW && offset+1 < maxTransactions)
        {
            offset++;
            refresh = 1;
        }
        else if(key == KEYCODE_PGUP && offset+20 < maxTransactions)
        {
            offset += 20;
            refresh = 1;
        }
        else if (key == KEYCODE_DOWNARROW && offset != 0)
        {
            offset--;
            refresh = 1;
        }
        else if(key == KEYCODE_PGDOWN && offset-20 >=0)
        {
            offset -= 20;
            refresh = 1;
        }
        else if(key == KEYCODE_ENTER)
        {
            transactionId = transactions[offset-loaded]->id;
            break;
        }
        else if(key == KEYCODE_ESCAPE || key == KEYCODE_BACKSPACE)
        {
            return;
        }
        else if(key == KEYCODE_D || key == KEYCODE_DELETE)
        {
            transactionId = transactions[offset-loaded]->id;
            isDeleting = 1;
            break;
        }
    }

    if(isDeleting)
    {
        printf_magenta("Are you sure you want to delete this transaction (y/n)? ");
    }
    else
    {
        printf_magenta("Please type a replacement entry for this row. You may include any of the following, but put them in this order.\r\n");

        printf_cyan("   <account> ");
        printf_green("<amount> ");
        printf("<date> ");
        printf_yellow("<message>\r\n");

    }

    fgets(commandBuffer, 1000, stdin);
    next_arg_ptr = commandBuffer;
    char* arg = next_arg();

    struct account* updateAccount = null;
    struct account* updateAccount2 = null;

    struct account* oldAccount = getAccount(transactions[offset-loaded]->account);
    long oldMoney = transactions[offset-loaded]->amount;
    char* oldDate = transactions[offset-loaded]->date;
    char* oldMsg = transactions[offset-loaded]->message;

    if(isDeleting)
    {
        if(strcmp(arg, "y") == 0 || strcmp(arg, "yes") == 0)
        {
            updateAccount = oldAccount;
            oldAccount->value -= transactions[offset-loaded]->amount;

            sprintf(commandBuffer, "DELETE FROM transactions WHERE ID=%i", transactions[offset-loaded]->id);
            sqlite3_exec(db, commandBuffer, null, null, null);
        }
    }
    else
    {
        while(1)
        {
            struct account* newAccount  = getAccountWithName(arg);

            if(newAccount)
                arg = next_arg();
            else
                newAccount = oldAccount;

            long newMoney = oldMoney;
            long long monTemp;
            if(parseMoney(arg, &monTemp))
            {
                newMoney = (long) monTemp;
                arg = next_arg();
            }

            char newDate[20];
            strcpy(newDate, oldDate);
            if(parseDateIntoBuff(arg, newDate))
                arg = next_arg();


            int freeMessage = 0;
            char* newMessage = makeStringDBSafe(arg, &freeMessage);


            char sql[2000] = "\0";
            char* sqlHead = sql;
            //account=%i, amount=%lld, date='%s', message='%s'
            sprintf(sql, "UPDATE transactions SET account='%i', amount='%li', date='%s', message='%s' WHERE ID=%i",
                newAccount->id,
                newMoney,
                newDate,
                newMessage == null ? oldMsg : newMessage,
                transactionId
            );


            sqlite3_exec(db, sql, null, null, null);

            oldAccount->value -= oldMoney;
            newAccount->value += newMoney;

            if(freeMessage)
                free(newMessage);

            if(oldAccount != newAccount)
            {
                updateAccount = oldAccount;
                updateAccount2 = newAccount;
            }
            else if(oldMoney != newMoney)
                updateAccount = oldAccount;

            //printf_magenta("%s",sql);
            printf_green("Success!");

            break;
        }
    }

    //update total sums if money has been changed.
    if(updateAccount)
    {
        sprintf(commandBuffer, "UPDATE accounts SET value=%lld WHERE ID=%i\0\0", updateAccount->value, updateAccount->id);
        sqlite3_exec(db, commandBuffer, null, null, null);
    }

    if(updateAccount2)
    {
        sprintf(commandBuffer, "UPDATE accounts SET value=%lld WHERE ID=%i\0\0", updateAccount2->value, updateAccount2->id);
        sqlite3_exec(db, commandBuffer, null, null, null);
    }

    //free all the transactions
    for(i=numberOfTransactions-1; i >=0; i--)
    {
        free(transactions[i]->date);
        free(transactions[i]->message);
    }

    free(transactions);
}

void commandTransaction(struct account* acc1)
{
    char* arg1 = next_arg(); //money
    char* date = next_arg();
    char* message = next_arg();

    struct account* transferToAcc = getAccountWithName(date);

    if(arg1 != null && strcmp(arg1, "->") == 0 && transferToAcc)
    {
        arg1 = message;
        date = next_arg();
        message = next_arg();

        while(1)
        {
            long long money;

            if(arg1 == null)
                break;
            else
            {
                arg1--;
                arg1[0] = '+';
            }

            if(parseMoney(arg1, &money) == 0)
                break;

            int day = 0;
            int month = 0;
            int year = 0;

            if(!parseDate(date, &year, &month, &day))
                break;

            int freeMessage;
            message = makeStringDBSafe(message, &freeMessage);

            acc1->value -= money;
            transferToAcc->value += money;

            char sql[3000];

            sprintf(sql, "UPDATE accounts SET value=%lld WHERE ID=%i;\r\n"
                "UPDATE accounts SET value=%lld WHERE ID=%i",
                 acc1->value, acc1->id, transferToAcc->value, transferToAcc->id);

            #ifdef PRINT_SQL
                printf_magenta(sql);
            #endif
            sqlite3_exec(db, sql, null, null, null);

            //format datestring properly
            char finDate[100];
            finDate[0] = '\0';

            if(year < 1000) strcat(finDate, "0");
            if(year < 100) strcat(finDate, "0");
            if(year < 10) strcat(finDate, "0");
            if(year < 1) strcat(finDate, "0");

            itoa(year, finDate, 10);

            strcat(finDate, "-");

            if(month < 10) strcat(finDate, "0");
            itoa(month, finDate + strlen(finDate), 10);
            strcat(finDate, "-");

            if(day < 10) strcat(finDate, "0");
            itoa(day, finDate + strlen(finDate), 10);

            sprintf(sql, "INSERT INTO transactions (account, amount, date, message) "
                "VALUES (%i, -%lld, '%s', '%s'), (%i, %lld, '%s', '%s');",
                acc1->id, money, finDate, message ? message : "",
                transferToAcc->id, money, finDate, message ? message : ""
            );

            #ifdef PRINT_SQL
                printf_magenta(sql);
            #endif
            sqlite3_exec(db, sql, null, null, null);

            if(freeMessage)
                free(message);

            return;
        }

        printcf("%RInvalid Syntax%N\r\n"
            "%C<account1> %N-> %C<account1> <amount> <date> <message>\r\n"
            "%Nwhere\r\n"
            "   %C<account1> %Nand %C<account2> %Nare the names of the accounts\r\n"
            "   %C<amount> %Nis the amount transfered. It must be an integer or a two digit decimal e.g. %C42 %Nor %C6.28\r\n"
            "   %C<date> %Nis the date in mm/dd or mm/dd/yy format\r\n"
            "   %C<message> %Nis a message associated with the transaction.\r\n"
        //  "      If your message includes spaces, put the message inside \"quotes\"\r\n"
        );

        printcf("      If your message includes spaces, put the message inside \"quotes\"\r\n");
    }

    else
    {
        while(1)
        {
            long long money;

            if(parseMoney(arg1, &money) == 0)
                break;

            int day = 0;
            int month = 0;
            int year = 0;

            if(!parseDate(date, &year, &month, &day))
                break;

            int freeMessage;
            message = makeStringDBSafe(message, &freeMessage);

            if(message != null && *message == '\0')
                message = null;

            acc1->value += money;


            char money_string[100];
            itoa(acc1->value, money_string, 10);

            char id_string[100];
            itoa(acc1->id, id_string, 10);

            char* sql = new_string_from_parts(4,"UPDATE accounts SET value=", money_string, " WHERE ID=", id_string);
            sqlite3_exec(db, sql, null, null, null);
            free(sql);

            lltoa(money, money_string, 10);

            char finDate[20];
            finDate[0] = '\0';

            if(year < 1000) strcat(finDate, "0");
            if(year < 100) strcat(finDate, "0");
            if(year < 10) strcat(finDate, "0");
            if(year < 1) strcat(finDate, "0");

            itoa(year, finDate, 10);

            strcat(finDate, "-");

            if(month < 10) strcat(finDate, "0");
            itoa(month, finDate + strlen(finDate), 10);
            strcat(finDate, "-");

            if(day < 10) strcat(finDate, "0");
            itoa(day, finDate + strlen(finDate), 10);

            char* sqlmessage;
            if(message == null)
                sqlmessage = new_string_from_parts(1, "NULL");
            else
                sqlmessage = new_string_from_parts(3, "'", message, "'");

            sql = new_string_from_parts( 9,
                "INSERT INTO transactions (account, amount, date, message) VALUES ('",
                id_string,
                "', '" ,
                money_string ,
                "', '",
                finDate ,
                "', ",
                sqlmessage,
                ")"
            );

            #ifdef PRINT_SQL
                printf_magenta(sql);
            #endif
            sqlite3_exec(db, sql, null, null, null);

            free(sql);
            free(sqlmessage);

            if(freeMessage)
                free(message);

            return;
        }

        printcf(
            "%RInvalid synatx\r\n"
            "%NCorrect syntax is as follows:\r\n"
            "%C<accountname> %G<amount> %M<date> %Y<message>%N\r\n"
            "where\r\n"
            "   %C<accountname>%N is the name of the account\r\n"
            "   %G<amount>%N is the amount transfered. It must begin with '+' or '-' followed by an integer.\r\n"
            "      Two-digit decimal values are also allowed.\r\n"
            "   %M<date>%N is the date in mm/dd or mm/dd/yy format\r\n"
            "   %Y<message>%N is a message associated with the transaction.\r\n"
            "      If you message includes spaces, put the message between \"quotes\"\r\n"
        );
    }
}


void commandRecalibrate()
{
    char sql[300];

    // the index of an account in accounts
    int i;
    for(i = 0; i < accounts_count; i++)
    {
        sprintf(sql, "SELECT SUM(amount) FROM transactions WHERE account=%i", accounts[i]->id);

        int sum;
        sqlite3_exec(db, sql, &getIdCallback, &sum, null);

        accounts[i]->value = sum;

        sprintf(sql, "UPDATE accounts SET value=%i WHERE ID=%i", sum, accounts[i]->id);

        sqlite3_exec(db, sql, null, null, null);
    }

    printcf("%MAccount values recalibrated");
}


int genericCallback(void* k, int cols, char** values, char** colNames)
{
    printf("Callback called!\r\n");
    int i;
    for(i=0; i< cols; i++)
    {
        printf("%s ", values[i]);
    }
    printf("\r\n");
    return 0;
}


int loadTransactionsCallback(void* row, int cols, char** values, char** colNames)
{
    //printf("callback %i\r\n", cols);
    int* row2 = row;
    struct transaction* t = malloc(sizeof(struct transaction));

    t->id =  atoi(values[0]);
    t->account =  atoi(values[1]);
    t->amount = atoll(values[2]);
    t->date = new_string_from_parts(1, values[3]);
    t->message = new_string_from_parts(1, values[4]);
    transactions[*row2] = t;
    (*row2)++;

    return 0;
}


int getIdCallback(void* ptr, int cols, char** values, char** colNames)
{
    //printf("callback! %s\n", values[0]);
    int* i = (int*) ptr;
    *i = atoi(values[0]);
    return 0;
}

//this callback takes the results of a SQL query and loads them into accounts
int loadAccountCallback(void* k, int cols, char** values, char** colNames)
{
    if(accounts_count < MAX_ACCOUNTS)
    {
        accounts[accounts_count] = malloc(sizeof(struct account));
        accounts[accounts_count]->id = atoi(values[0]);
        accounts[accounts_count]->value = atoll(values[2]);
        accounts[accounts_count]->name = (char*) malloc(strlen(values[1])+1);
        strcpy(accounts[accounts_count]->name, values[1]);

        accounts_count++;
    }

    return 0;
}

int getYear()
{
    time_t now;
    time(&now);
    struct tm *mytime = localtime(&now);
    return 1900 + mytime->tm_year;
}

struct account* getAccount(int accNum)
{
    int i;
    for(i=0; i < accounts_count; i++)
    {
        if(accounts[i]->id == accNum)
            return accounts[i];
    }
    return null;
}

struct account* getAccountWithName(char* name)
{
    int i;
    if(name == null)
        return null;

    for(i=0; i < accounts_count; i++)
    {
        if(strcmp(accounts[i]->name, name) == 0)
            return accounts[i];
    }
    return null;
}

// displayCumulativeSum: null to not display that column, otherwise points to the sum
void print_transaction_table(int tableSize, int rowHighlight, long long* displayCumulativeSum)
{
    int i;
    int tabSizeAccount = 0;
    int tabSizeTrans = 0;
    int tabSizeDate = 0;
    int tabSizeBalance = 0;

    for(i=0; i< tableSize; i++)
    {
        struct account* acc = getAccount(transactions[i]->account);

        int l = strlen(transactions[i]->date);
        if(l > tabSizeDate)
            tabSizeDate = l;

        l = print_money_length(transactions[i]->amount);
        if(l > tabSizeTrans)
            tabSizeTrans = l;

        l = strlen(acc->name);
        if(l > tabSizeAccount)
            tabSizeAccount = l;

        if(displayCumulativeSum != null)
        {
            if(i == 0)
                transactions[i]->cumulative = *displayCumulativeSum;
            else
                transactions[i]->cumulative = transactions[i-1]->cumulative - transactions[i-1]->amount;

            l = print_money_length(transactions[i]->cumulative);
            if(l > tabSizeBalance)
                tabSizeBalance = l;
        }
    }

    //print each transaction

    for(i=tableSize-1; i >=0; i--)
    {
        int (*printfn)(const char*, ...) = &printf;
        if(i == rowHighlight)
        {
            printfn = &printf_highlight;
        }


        int l = 0;
        char* name = getAccount(transactions[i]->account)->name;

        if(i == rowHighlight)
            printf_highlight("%s", name);
        else
            printf_cyan("%s", name);
        l = strlen(name);

        while(l <= tabSizeAccount)
        {
            (*printfn)(" ");
            l++;
        }

        //print date;
        (*printfn)("%s",transactions[i]->date);
        l = strlen(transactions[i]->date);
        while(l <= tabSizeDate)
        {
            (*printfn)(" ");
            l++;
        }

        //print money column
        l = print_money_length(transactions[i]->amount);
        while(l < tabSizeTrans)
        {
            (*printfn)(" ");
            l++;
        }

        if(rowHighlight == i)
            print_money_highlight(transactions[i]->amount);
        else
            print_money(transactions[i]->amount);
        (*printfn)(" ");


        //print balance column
        if(displayCumulativeSum != null)
        {
            l = print_money_length(transactions[i]->cumulative);
            while(l < tabSizeBalance)
            {
                (*printfn)(" ");
                l++;
            }

            if(rowHighlight == i)
                print_money_highlight(transactions[i]->cumulative);
            else
                print_money(transactions[i]->cumulative);
            (*printfn)(" ");
        }


        //print message
        (*printfn)("%s",transactions[i]->message);
        (*printfn)("\r\n");
    }
}


void print_money(int amount)
{
    char cents[2];
    cents[0] = '0';
    cents[1] = '0';

    int isNeg = 0;
    if(amount <0)
    {
        isNeg = 1;
        amount *= -1;
    }

    if(amount % 100 > 9)
        itoa(amount % 100, cents, 10);
    else
        itoa(amount % 100, cents+1, 10);

    if(isNeg)
        printf_red("-$%lld.%s", amount/100, cents);
    else
        printf_green("$%lld.%s", amount/100, cents);
}

void print_money_highlight(int amount)
{
    char cents[2];
    cents[0] = '0';
    cents[1] = '0';

    int isNeg = 0;
    if(amount <0)
    {
        isNeg = 1;
        amount *= -1;
    }

    if(amount % 100 > 9)
        itoa(amount % 100, cents, 10);
    else
        itoa(amount % 100, cents+1, 10);

    if(isNeg)
        printf_highlight("-$%lld.%s", amount/100, cents);
    else
        printf_highlight("$%lld.%s", amount/100, cents);
}


int print_money_length(long long amount)
{
    int length = 5;
    if(amount < 0)
    {
        amount *= -1;
        length++;
    }

    amount = amount / 1000;

    while(amount != 0)
    {
        amount /= 10;
        length++;
    }
    
    return length;
}


int parseMoney(char* str, long long* moneyOut)
{
    if(str == null)
        return 0;

    if(str[0] != '+' && str[0] != '-')
        return 0;

    int sign = (str[0] == '+' ? 1 : -1);

    int j = 1;
    if(str[j] == '$')
        j++;

    char* dollarStart = str+j;

    if(!isDigit(str[j]))
        return 0;

    while(isDigit(str[j]))
        j++;

    if(str[j] == '.')
    {
        str[j] = '\0';

        if(!isDigit(str[j+1]))
            return 0;
        if(!isDigit(str[j+2]))
            return 0;
        if(str[j+3] != '\0')
            return 0;

        *moneyOut = sign*(atoll(dollarStart)*100 + atoll(str+j+1));
        return 1;
    }
    if(str[j] == '\0')
    {
        *moneyOut = atoll(dollarStart)*100*sign;
        return 1;
    }

    return 0;
}

int parseDate(char* arg, int* yearOut, int* monthOut, int* dayOut)
{
    if(arg == null)
        return 0;

    //parse month
    if(!isDigit(arg[0]))
        return 0;

    int j = 0;
    while(isDigit(arg[j]))
        j++;

    if(arg[j] != '/')
        return 0;

    arg[j] = '\0';
    *monthOut = atoi(arg);
    arg += j+1;

    if(*monthOut < 0 || *monthOut > 12)
        return 0;

    //parse day
    if(!isDigit(arg[0]))
        return 0;
    j = 0;
    while(isDigit(arg[j]))
        j++;

    if(arg[j] == '\0')
    {
        *dayOut = atoi(arg);
        *yearOut = getYear();
    }
    else if(arg[j] == '/')
    {
        arg[j] = '\0';
        *dayOut = atoi(arg);
        arg = arg+j+1;
        j = 0;

        //parse year
        if(!isDigit(arg[0]))
            return 0;
        while(isDigit(arg[j]))
            j++;
        if(arg[j] != '\0')
            return 0;
        *yearOut = atoi(arg) + (getYear()/100)*100;
    }
    else
        return 0;

    if(*dayOut < 0 || *dayOut > 32)
        return 0;
    if(*yearOut < 0 || *yearOut > 9999)
        return 0;

    return 1;
}

// Takes in a date in the format mm/dd or mm/dd/yy and puts it into a buffer as yyyy-mm-dd
int parseDateIntoBuff(char* date, char* buff)
{
    int day ; int month; int year;
    if(!parseDate(date, &year, &month, &day))
        return 0;

    if(year < 1000)
    {
        strcpy(buff,"0");
        buff += 1;

        if(year < 100)
        {
            strcpy(buff,"0");
            buff += 1;
            if(year < 10)
            {
                strcpy(buff,"0");
                buff += 1;
            }
        }
    }

    sprintf(buff, "%i", year);
    buff += strlen(buff);

    sprintf(buff, month < 10 ? "-0%i" : "-%i", month);
    buff += 3;

    sprintf(buff, day < 10 ? "-0%i" : "-%i", day);
    return 1;
}


//precondition: in contains no '"' OR (in contains 2 '"' AND in starts with '"' AND in ends with '"')
char* makeStringDBSafe(char* message, int* newMemoryAllocated)
{
    *newMemoryAllocated = 0;

    int aposCount = 0;
    int j = 0;

    if(message != null)
    {
        int messageLen = strlen(message);

        if(*message == '"')
        {
            message[messageLen-1] = '\0';
            message[0] = '\0';
            message++;
            messageLen-= 2;
        }

        for(j =0; j<messageLen; j++)
        {
            if(message[j] == '\'')
                aposCount++;
        }

        if(aposCount)
        {
            *newMemoryAllocated = 1;

            char* message2 = malloc(messageLen+aposCount+1);

            while(messageLen >= 0)
            {
                message2[messageLen+aposCount] = message[messageLen];
                if(message[messageLen] == '\'')
                {
                    aposCount--;
                    message2[messageLen+aposCount] = '\'';
                }
                messageLen--;
            }

            return message2;
        }

        return message;
    }

    return null;
}

int isDigit(char c)
{
    int val = (int) c;
    if(48 <= c && c < 58)
        return 1;
    return 0;
}

// next_arg() proccesses the buffer, making sure the next arg conforms, and then returns a ptr to that arg.
char* next_arg()
{
    char* beginning = next_arg_ptr;
    next_arg_ptr = null;
    char* head = beginning;

    if(head == null)
        return null;

    if(*head == '"')
    {
        head++;
        while(*head != '\0' && *head != '"')
            head++;
        if(*head == '\0')
            return null;

        head++;
        if(*head == '\0')
            return beginning;

        *head = '\0';
        head++;

        while(*head != ' ' && *head != '\0' && *head != '\n' && *head != '\r')
            head++;
        while(*head == ' ' || *head == '\n' || *head == '\r')
            head++;
        if(*head != '\0')
            next_arg_ptr = head;

        return beginning;
    }
    else
    {
        while(*head != ' ' && *head != '\0' && *head != '\n' && *head != '\r')
            head++;

        while(*head == ' ' || *head == '\n' || *head == '\r' )
        {
            *head = '\0';
            head++;
        }

        if(*head != '\0')
            next_arg_ptr = head;

        return beginning;
    }
}

char* new_string_from_parts(int count, ...)
{
    int size = 64;
    char* buffer = (char*) malloc(size);
    buffer[0] = '\0';
    int used = 0;


    va_list vl;
    va_start(vl, count);

    int i;
    for(i = 0; i< count; i++)
    {
        char* part = va_arg(vl,char*);

        if(part != null)
        {
            int len = strlen(part);

            if(len+used >= size)
            {
                while(len+used >= size)
                    size *= 2;

                buffer = (char*) realloc(buffer, size);

                if(buffer == null)
                {
                    free(buffer);
                    printf_red("OUT OF MEMORY!");
                    return null;
                }
            }

            strcpy(buffer+used, part);
            used += len;
        }
    }

    va_end(vl);
    return buffer;
}
