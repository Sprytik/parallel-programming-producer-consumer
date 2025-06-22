#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define USLEEP_TIME 800
#define N 16
#define FULL_SYNC_NUMBER 2

pthread_t P1, P2, P3, P4, P5, P6;

//засоби взаємного виключення
pthread_mutex_t mcr1 = PTHREAD_MUTEX_INITIALIZER;
sem_t scr1;
int usleep_flag = 0;  //Flag to toggle usleep in Consumers threads (p4, p2, p5)

//засоби синхронізації паралельних потоків
pthread_barrier_t bcr2;
pthread_cond_t Sig21 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex_Sig21 = PTHREAD_MUTEX_INITIALIZER;
int Sig21_p2_flag = 0;
int Sig21_p3_flag = 0;
int Sig21_p4_flag = 0;
int Sig21_p6_flag = 0;

//атомарні змінні
int ivar1 = 1, ivar2 = 2;
unsigned uvar1 = 3, uvar2 = 4;
long lvar1 = 5, lvar2 = 6;
long unsigned luvar1 = 7, luvar2 = 8;

void use_Atomic(int thread_number);
void modify_Atomic(int thread_number);

//Структура спільного ресурсі та необхідні допоміжні засоби
int max_list_size = N;
int elementCounter = 0;
int total_element_created = 0;

struct node {
    int data;
    struct node *next;
};

struct node *tail = NULL;

void addToEmpty(int value) {
    struct node *temp = malloc(sizeof(struct node));
    temp->data = value;
    temp->next = temp;
    tail = temp;
}

void addAtEnd(int value) {
    if (tail == NULL) {
        addToEmpty(value);
        return;
    }

    struct node *temp = malloc(sizeof(struct node));
    temp->data = value;
    temp->next = tail->next;

    tail->next = temp;
    tail = tail->next;
}

int delFirst(void) {
    int ret_data;

    if(tail == NULL) 
        return -1;

    if(tail->next == tail) {
        ret_data = tail->data;
        free(tail);
        tail = NULL;
        return ret_data;
    }

    struct node *temp;
    temp = tail->next;
    ret_data = temp->data;
    tail->next = temp->next;
    free(temp);
    temp = NULL;
    return ret_data;
} 


void freeBuffer(void) {
    if(tail == NULL)
        return;

    if(tail->next == tail) {
        free(tail);
        tail = NULL;
        return;
    }
    
    struct node *temp = NULL;
    while(tail->next != tail) {
        temp = tail->next;
        tail->next = temp->next;
        free(temp);
    }

    free(tail);
    tail = NULL;
    return;
}

void print_CR1(void) {
    if(tail == NULL) {
        printf("List is empty\n");
    }
    else {
        printf("Content of the Circular Linked List: ");
        struct node *temp = tail->next;
        do {
            printf("%d ", temp->data);
            temp = temp->next;
        } while(temp != tail->next);
        printf("\n");
    }
}


void modify_Atomic(int thread_number)
{
    printf("\nP%d modifies atomic variables using numbered atomic ops\n", thread_number);

    // 2. __atomic_add_fetch
    printf("(2) int add fetch: %d\n", __atomic_add_fetch(&ivar1, ivar2, __ATOMIC_RELAXED));

    // 8. __atomic_fetch_sub
    printf("(8) int fetch sub: %d\n", __atomic_fetch_sub(&ivar2, ivar1, __ATOMIC_RELAXED));

    // 4. __atomic_xor_fetch
    printf("(4) long xor fetch: %ld\n", __atomic_xor_fetch(&lvar1, lvar2, __ATOMIC_RELAXED));

    // 5. __atomic_or_fetch
    printf("(5) long or fetch: %ld\n", __atomic_or_fetch(&lvar2, lvar1, __ATOMIC_RELAXED));

    // 10. __atomic_fetch_xor
    printf("(10) unsigned fetch xor: %u\n", __atomic_fetch_xor(&uvar1, uvar2, __ATOMIC_RELAXED));

    // 11. __atomic_fetch_or
    printf("(11) unsigned fetch or: %u\n", __atomic_fetch_or(&uvar2, uvar1, __ATOMIC_RELAXED));

    // 13. __atomic_compare_exchange_n
    printf("(13) long unsigned compare exchange success: %d\n",
           __atomic_compare_exchange_n(&luvar1, &luvar2, thread_number, 0,
                                       __ATOMIC_RELAXED, __ATOMIC_RELAXED));

    // 14. __atomic_exchange
    printf("(14) long unsigned before exchange: %lu %lu\n", luvar1, luvar2);
    __atomic_exchange(&luvar2, &luvar1, &luvar1, __ATOMIC_RELAXED);
    printf("(14) long unsigned after exchange: %lu %lu\n", luvar1, luvar2);
}

void use_Atomic(int thread_number)
{
    printf("\nP%d uses atomic variables\n", thread_number);
    printf("ivar1 + ivar2 = %d + %d = %d\n", ivar1, ivar2, (ivar1 + ivar2));
    printf("uvar1 + uvar2 = %u + %u = %u\n", uvar1, uvar2, (uvar1 + uvar2));
    printf("lvar1 + lvar2 = %ld + %ld = %ld\n", lvar1, lvar2, (lvar1 + lvar2));
    printf("luvar1 + luvar2 = %lu + %lu = %lu\n\n", luvar1, luvar2, (luvar1 + luvar2));
}


void *P1_thread(void *arg) {
    printf("Thread P1 has started\n");
    pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
    
    int sem_value = 0;
    while(1) {
        
        sem_getvalue(&scr1, &sem_value);
        if(sem_value < max_list_size) {
            printf("P1 Producer tries to lock MCR1\n");        
            while(pthread_mutex_trylock(&mcr1) != 0) {
                //P1 Producer doing something useful while mutex mcr1 is locked by other thread
            }
            printf("P1 Producer successfully locked MCR1\n");

            addAtEnd(total_element_created);
            total_element_created++;
            printf("P1 Producer successfully added a new element number %d to CR1 list\n", elementCounter);
            elementCounter++;
            //print_CR1();
            sem_post(&scr1);
            printf("P1 Producer sent SCR1 post signal\n");
            pthread_mutex_unlock(&mcr1);
            printf("P1 Producer unlocked MCR1\n");
            
       }

    }
    return NULL;
}


void *P2_thread(void *arg) {
    printf("Thread P2 has started\n");
    pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
    int data;

    while(1) {

        printf("P2 tries to lock Sig21\n");
        pthread_mutex_lock(&mutex_Sig21);
        printf("P2 Consumer waits for Sig21\n");
        while(!Sig21_p2_flag) {
            pthread_cond_wait(&Sig21, &mutex_Sig21);
        }
        printf("P2 Consumer got Sig21\n");
        
        Sig21_p2_flag = 0;
        pthread_mutex_unlock(&mutex_Sig21);
        printf("P2 unlocked Sig21\n");
        
        use_Atomic(2);
        modify_Atomic(2);

        printf("P2 Consumer waits for SCR1 post signal\n");
        while(sem_trywait(&scr1) != 0) {
            //P2 Consumer doing something useful while there is no elements 
        }
        printf("P2 Consumer got SCR1 post signal\n");
        
        printf("P2 Consumer tries to lock MCR1\n");        
        while(pthread_mutex_trylock(&mcr1) != 0) {
            //P2 Consumer doing something useful while mutex mcr1 is locked by other thread
        }
        printf("P2 Consumer successfully locked MCR1\n");

        data = delFirst();
        elementCounter--;
        printf("P2 Consumer successfully took %d from CR1 list\n", data);
        printf("Now there is %d elements left in CR1 list\n", elementCounter);
        pthread_mutex_unlock(&mcr1);
        printf("P2 Consumer unlocked MCR1\n");

        if(usleep_flag)
            usleep(USLEEP_TIME);
    }
    return NULL;
}


void *P3_thread(void *arg) {
    printf("Thread P3 has started\n");
    pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS,NULL);

    while(1) {
        printf("P3 tries to lock Sig21\n");
        pthread_mutex_lock(&mutex_Sig21);
        printf("P3 waits for Sig21\n");
        while(!Sig21_p3_flag) {
            pthread_cond_wait(&Sig21, &mutex_Sig21);
        }
        printf("P3 got Sig21\n");
        
        Sig21_p3_flag = 0;
        pthread_mutex_unlock(&mutex_Sig21);
        printf("P3 unlocked Sig21\n");

        use_Atomic(3);
        
        printf("P3 waits on Barrier bcr2\n");
        pthread_barrier_wait(&bcr2);
        printf("P3 passed through Barrier bcr2\n");

        use_Atomic(3);
    }
    return NULL;
}


void *P4_thread(void *arg) {
    printf("Thread P4 has started\n");
    pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
    int data;

    while(1) {

        printf("P4 tries to lock Sig21\n");
        pthread_mutex_lock(&mutex_Sig21);
        printf("P4 Consumer waits for Sig21 \n");
        while(!Sig21_p4_flag) {
            pthread_cond_wait(&Sig21, &mutex_Sig21);
        }
        printf("P4 Consumer got Sig21\n");
        
        Sig21_p4_flag = 0;
        pthread_mutex_unlock(&mutex_Sig21);
        printf("P4 Consumer unlocked Sig21\n");

        printf("P4 Consumer waits for SCR1 post signal\n");
        while(sem_trywait(&scr1) != 0) {
            //P4 Consumer doing something useful while there is no elements 
        }
        printf("P4 Consumer got SCR1 post signal\n");
        
        printf("P4 Consumer tries to lock MCR1\n");        
        while(pthread_mutex_trylock(&mcr1) != 0) {
            //P4 Consumer doing something useful while mutex mcr1 is locked by other thread
        }
        printf("P4 Consumer successfully locked MCR1\n");

        data = delFirst();
        elementCounter--;
        printf("P4 Consumer successfully took %d from CR1 list\n", data);
        printf("Now there is %d elements left in CR1 list\n", elementCounter);
        pthread_mutex_unlock(&mcr1);
        printf("P4 Consumer unlocked MCR1\n");

        if(usleep_flag)
            usleep(USLEEP_TIME);
    }
    return NULL;
}


void *P5_thread(void *arg) {
    printf("Thread P5 has started\n"); 
    pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
    int data;

    while(1) {
        modify_Atomic(5);
       
        printf("P5 Consumer tries to lock Sig21\n");
        pthread_mutex_lock(&mutex_Sig21);
        Sig21_p2_flag = 1;      
        Sig21_p3_flag = 1;      
        Sig21_p4_flag = 1;      
        Sig21_p6_flag = 1;      

        pthread_cond_broadcast(&Sig21);
        printf("P5 Consumer broadcasted Sig21\n");

        pthread_mutex_unlock(&mutex_Sig21);
        printf("P5 Consumer unlocked Sig21\n");

        printf("P5 Consumer waits for SCR1 post signal\n");
        while(sem_trywait(&scr1) != 0) {
            //"P5 Consumer doing something useful while there is no elements 
        }
        printf("P5 Consumer got SCR1 post signal\n");
        
        printf("P5 Consumer tries to lock MCR1\n");
        while(pthread_mutex_trylock(&mcr1) != 0) {
            //P5 Consumer doing something useful while mutex mcr1 is locked by other thread
        }
        printf("P5 Consumer successfully locked MCR1\n");

        data = delFirst();
        elementCounter--;
        printf("P5 Consumer successfully took %d from CR1 list\n", data);
        printf("Now there is %d elements left in CR1 list\n", elementCounter);
        pthread_mutex_unlock(&mcr1);
        printf("P5 Consumer unlocked MCR1\n");

        if(usleep_flag)
            usleep(USLEEP_TIME);
    }
    return NULL;
}


void *P6_thread(void *arg) {
    printf("Thread P6 has started\n");
    pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS,NULL);

    while(1) {

        printf("P6 tries to lock Sig21\n");
        pthread_mutex_lock(&mutex_Sig21);

        printf("P6 waits for Sig21\n");
        while(!Sig21_p6_flag) {
            pthread_cond_wait(&Sig21, &mutex_Sig21);
        }
        printf("P6 got signal Sig21\n");

        Sig21_p6_flag = 0;
        pthread_mutex_unlock(&mutex_Sig21);
        printf("P6 unlocked Sig21\n");
        
        use_Atomic(6);
        
        printf("P6 waits on Barrier bcr2\n");
        pthread_barrier_wait(&bcr2);
        printf("P6 passed through Barrier bcr2\n");

        modify_Atomic(6);
    }
    return NULL;
}


int main(void) {

    pthread_barrier_init(&bcr2, NULL, FULL_SYNC_NUMBER);
    sem_init(&scr1, 0, 0);

    pthread_create(&P1, NULL, P1_thread, NULL);
    pthread_create(&P2, NULL, P2_thread, NULL);
    pthread_create(&P3, NULL, P3_thread, NULL);
    pthread_create(&P4, NULL, P4_thread, NULL);
    pthread_create(&P5, NULL, P5_thread, NULL);
    pthread_create(&P6, NULL, P6_thread, NULL);

    pthread_join(P1, NULL);
    pthread_join(P2, NULL);
    pthread_join(P3, NULL);
    pthread_join(P4, NULL);
    pthread_join(P5, NULL);
    pthread_join(P6, NULL);

    printf("Finished!!!\n");

    pthread_barrier_destroy(&bcr2);
    sem_destroy(&scr1);
    freeBuffer();

    return 0;
}
