/*******************************************************************************
 Chris Nosowsky
 Computer Project #6
 ******************************************************************************/
#include <unistd.h>
#include <string.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <pthread.h>
#include <semaphore.h>
#define MAX 128
#define NTHREADS 10
#define NTHREADSCONSUMER 1                  //only 1 consumer thread
void* process_transaction(void* arg);
void* consume_transaction(void* arg);
struct account                              //account struct for record keeping
{
    int acc_num;                            //account number
    char trans_type[100];                   //transaction type(withdraw or deposit))
    double acc_balance;                     //either account balance or deposit/withdraw fund
    int count = 0;                          //keeping track of threads
};
struct account buffer[20];                  //global vars
struct account accs[100];
int k = 5;
int in = 0;
int out = 0;
int numProd = 1;
int N = 0;
sem_t S; //producer-1 value                //our 3 semaphores for producer/consumer
sem_t NSem; //consumer-0 value
sem_t E;

int main(int argc, char* argv[]) {
    /*--------------------------------------------------------------------------   
     * Name:     main   
     * Purpose:  Loops through each file left to right and processes each line in
     *           input files as directive for first token, if valid.
     * Receive:  Char array command line arguments and # of arguments given
     * Return:   0 or 1(error)
     * -----------------------------------------------------------------------*/
    FILE *fp, *fp2, *fpTrans;
    int prodset = 0;
    int buffset = 0;
    int a = 0;
    int tokenNum = 0;
    char numChar[10];
    char result[100];
    char line[MAX];
    char *token;
    char *end;
    const char del[] = " \t\r\n\f"; 
    pthread_t consumer_thread[NTHREADSCONSUMER];
    
    for (int i=1; i<argc; i++) {                        //loops through the commands in the command line given.
        if (strcmp(argv[i], "-p") == 0) {
            prodset = 1;
            buffset = 0;
        }
        else if(strcmp(argv[i], "-b") == 0) {
            buffset = 1;
            prodset = 0;
        }
        else if(argv[i][0] == '-') {                // another flag other then s and v. error.
            printf("Error on: '%s' -- invalid option. Please use '-p' or '-b' for options.\n", argv[i]);
            return 1;
        }
        else {
            if (prodset == 1) {                         //if producer flag set
                if(atoi(argv[i]) != 0){
                    if (atoi(argv[i]) < 11) {
                        numProd = atoi(argv[i]); //might need atoi
                    }
                    else {
                        printf("Error: exceeded producer size.\n");
                        return 1;
                    }
                }
                else {
                    printf("Error: argument expected to be int but got different type or zero/negative provided.\n");
                    return 1;
                }
                prodset = 0;
            }
            else if(buffset == 1) {                     // if buffer flag set
                if(atoi(argv[i]) != 0) {
                    if(atoi(argv[i]) < 21) {
                        k = atoi(argv[i]);
                    }
                    else {
                        printf("Error: exceeded buffer size of 20.\n");
                        return 1;
                    }
                }
                else {
                    printf("Error: argument expected to be int but got different type or zero/negative provided.\n");
                    return 1;
                }
                buffset = 0;
            }
        }
    }
    pthread_t producer_thread[numProd];                 // new producer thread(s) based on number given or default 1
    if(sem_init(&S, 0, 1) != 0) {
        printf("Error: sem_init returned with errors.\n");
        return 1;
    }
    
    if(sem_init(&NSem, 0, 0) != 0) {
        printf("Error: sem_init returned with errors.\n");
        return 1;
    }
    if(sem_init(&E, 0, k) != 0) {
        printf("Error: sem_init returned with errors.\n");
        return 1;
    }
    fp = fopen("accounts.old", "r");
    if(fp < 0 || fp == NULL) {
        printf("Error: file not found\n");
        return 1;
    }

    while(fgets(line, MAX, fp) != NULL) {                   //looping through accounts.old to get an idea of what accounts we have to write to accounts.new
        token = strtok(line, del);
        while(token != NULL) {
            if (tokenNum == 0) {
                accs[a].acc_num = atoi(token);
            }
            else if (tokenNum == 1) {
                accs[a].acc_balance = strtod(token, &end);
            }
            tokenNum += 1;
            token = strtok(NULL, del);
        }
        tokenNum = 0;
        a += 1;
    }
    
    for (int i=0; i <numProd; i++) {                        //went the route of converting each producer number into a transaction number for the producer function
        sprintf(numChar, "%d", i);
        strcpy(result,"trans");
        strcat(result, numChar);
        fpTrans = fopen(result, "r");                       //transN for reading
        if (fpTrans < 0 || fpTrans == NULL) {
            printf("Error: failed to open transaction file.\n");
            return 1;
        }
        if(pthread_create(&producer_thread[i], NULL, process_transaction, (void *) fpTrans)) {
            printf("Error: couldn't create thread.\n");
            return 1;
        }
    }


    if(pthread_create(&consumer_thread[0], NULL, consume_transaction, (void *) a)) { 
        printf("Error: couldn't create thread.\n");
        return 1;
    }
    
    for (int i=0; i <numProd; i++) {
        if(pthread_join(producer_thread[i], NULL)) {                            //waiting/blocked state
            printf("Error: couldn't join thread.\n");
            return 1;
        }
    }
    
    if(pthread_join(consumer_thread[0], NULL)) {                                //waiting /blocked state
        printf("Error: couldn't join thread.\n");
        return 1;
    }
    fp2 = fopen("accounts.new", "w");
    if(fp2 == NULL) {
        printf("Error: Unable to create file\n");
        return 1;
    }

    char update[80];
    for(int i = 0; i<a; i++) {                                                  // after threads above have exited, signal given and this will write all new balanced to accounts.new
        snprintf(update, sizeof(update), "%d %.2f\n", accs[i].acc_num, accs[i].acc_balance);
        fputs(update, fp2);
        strcpy(update, "");
    }
    
    fclose(fp);
    fclose(fp2);
      
    pthread_exit(NULL); 
}


void* process_transaction(void* arg) {
    char line[100];
    struct account transaction;
    int x = 0;
    char *end;
    const char *hey = "DONE";
    char *token;
    const char del[] = " \t\r\n\f";
    FILE *fp3 = (FILE *) arg;
    transaction.count += N;                     //everytime we enter this producer function, new thread created, meaning increase count..thread #
    N+=1;
    while(fgets(line, MAX, fp3) != NULL) {      //until EOF, get each line and process it
        token = strtok(line, del);
        while(token != NULL) {
            if(x==0)
                transaction.acc_num = atoi(token);
            else if(x==1)
                strcpy(transaction.trans_type, token);
            else if(x==2)
                transaction.acc_balance = strtod(token, &end);
            x+=1;
            token = strtok(NULL, del);
        }
        if(x == 3) {                        //critical section
            sem_wait(&E);
            sem_wait(&S);
            buffer[in] = transaction;
            in = (in + 1) % k;
            sem_post(&S);
            sem_post(&NSem);
        }
        x = 0;
    }
    transaction.acc_num = 123;              //the special transaction
    strcpy(transaction.trans_type, (char*)hey);
    transaction.acc_balance = 123.00;
    
    sem_wait(&E);
    sem_wait(&S);
    buffer[in] = transaction;
    in = (in + 1) % k;
    sem_post(&S);
    sem_post(&NSem);
    
    fclose(fp3);
    pthread_exit(NULL); 
}

void* consume_transaction(void* arg) {
    struct account transaction;
    FILE *fp4;
    fp4 = fopen("accounts.log", "w");
    double old_account_balance;
    double new_account_balance;
    int breakit = 0; 
    char line[80];
    int P = 0;
    if(fp4 == NULL) {
        printf("Error: Unable to create file\n");
        pthread_exit(NULL);
    }
    while(1) {                                                  //repeat forever like on the 2/6 slide. process is the same
        sem_wait(&NSem);
        sem_wait(&S);
        transaction = buffer[out];                              
        out = (out + 1) % k;
        sem_post(&S);
        sem_post(&E);
        for(int t=0; t< (int) arg; t++) {
            if (transaction.acc_num == 123 && strcmp(transaction.trans_type, "DONE") == 0) {        //if special transaction found, then break out of for and then increase P to keep track of thread count and when to halt
                breakit = 1;
                break;
            }
            if (accs[t].acc_num == transaction.acc_num) {
                if(strcmp(transaction.trans_type,"deposit")==0) {                       //all logic to update the transaction balance based on transaction type
                    
                    old_account_balance = accs[t].acc_balance;
                    accs[t].acc_balance += transaction.acc_balance;
                    new_account_balance = accs[t].acc_balance;
                    break;
                }
                else if(strcmp(transaction.trans_type, "withdraw")==0) {
                    old_account_balance = accs[t].acc_balance;
                    accs[t].acc_balance -= transaction.acc_balance;
                    new_account_balance = accs[t].acc_balance;
                    break;
                }
                else {
                    printf("Error: didn't recognize transaction type.");
                    pthread_exit(NULL); 
                }
            }
        }
        if(breakit == 0) {      //formatting below
            
            snprintf(line, sizeof(line), "%d %d %12s $%8.2f $%8.2f $%8.2f\n", transaction.count, transaction.acc_num, transaction.trans_type, old_account_balance, transaction.acc_balance, new_account_balance);
            if (line[strlen(line) - 1] != '\n') {   // Line exceeds buffer size according to specs
                printf("Buffer size exceeded for line. Each line will be no more than 80 characters\n");
                pthread_exit(NULL);
            }
            fputs(line, fp4);
            strcpy(line, "");
        }
        else {
            breakit = 0;
            P+=1;
        }
        if(P == numProd) {
            pthread_exit(NULL);                         //if special transaction called for example 4 times and we have 4 threads, exit.
        }
    }
    pthread_exit(NULL); 
}