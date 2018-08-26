#define null 0
#define MAX_ACCOUNTS 256

struct account {
    int id;
    char* name;
    long long value;
};

struct transaction {
    int id;
    int account;
    long amount;
    char* date;
    char* message;
    long long cumulative;
};


int genericCallback(void* k, int cols, char** values, char** colNames);
int commandLoop();
void commandTransaction(struct account* acc1);
void commandLast();
void commandEdit();
void commandSum();
void commandNew();
void commandRecalibrate();

int str_matches(char* source, int offset, char* match);

int sumHandler(void* k, int cols, char** values, char** colNames);
int getIdCallback(void* ptr, int cols, char** values, char** colNames);
int loadTransactionsCallback(void* row, int cols, char** values, char** colNames);
int loadAccountCallback(void* k, int cols, char** values, char** colNames);
int isDigit(char c);
void exitProgram();


char* new_string_from_parts(int count, ...);
int parseMoney(char* str, long long* moneyOut);
int parseDate(char* arg, int* yearOut, int* monthOut, int* dayOut);
int parseDateIntoBuff(char* date, char* buff);
char* makeStringDBSafe(char* message, int* newMemoryAllocated);

int print_money_length(long long amount);
void print_money(int amount);
void print_money_highlight(int amount);
void print_transaction_table(int tableSize, int rowHighlight, long long* displayCumulativeSum);

struct account* getAccount(int accNum);
struct account* getAccountWithName(char* name);
int getYear();
char* getAccountName(int acc);
char* next_arg();

char commandBuffer[1000];
char* next_arg_ptr;

struct account* accounts[MAX_ACCOUNTS];
int accounts_count = 0;

struct transaction** transactions;
int transactions_count = 0;

sqlite3* db;
