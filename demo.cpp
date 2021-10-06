#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h>
#include <unistd.h>
#include <threads.h>
#include <thread>
#include <vector>

#define BUFFER_LEN 4 //缓冲区队列长度

SDL_mutex *mutex;                    //互斥锁变量
SDL_cond *cond;                      //条件变量
std::vector<int> buffer(BUFFER_LEN); //缓冲区
int head = 0, tail = 0;              //队列头指针和尾指针
int productID = 1;                   //产品ID
int productCount = 0;                //队列中产品计数

static int producerThread(void *ptr)
{
    while (true)
    {
        if (SDL_LockMutex(mutex) == 0)
        {                                      //获取锁，进入临界区
            while (BUFFER_LEN == productCount) //如果缓冲区已满，释放锁，并等待缓冲区出现空闲空间
            {
                SDL_CondWait(cond, mutex);
                printf("Buffer is full, producer is waiting...... \n");
            }
            buffer[head] = productID;
            head = (head + 1) % BUFFER_LEN;
            printf("Producer has produced a product %d\n", productID);
            productID++;
            productCount++;
            SDL_CondSignal(cond);
            SDL_UnlockMutex(mutex);
            sleep(rand() % 5);
        }
        else
        {
            fprintf(stderr, "Couldn't lock mutex\n");
        }
    }
    return 0;
}

static int consumerThread(void *ptr)
{
    while (1)
    {
        if (SDL_LockMutex(mutex) == 0)
        {
            while (0 == productCount)
            { //如果缓冲区为空，释放锁，并等待缓冲区非空
                SDL_CondWait(cond, mutex);
                printf("\t\t Buffer is empty, consumer is waiting......\n");
            }

            printf("\t\t Consumer consumed product %d\n", buffer[tail]);
            tail = (tail + 1) % BUFFER_LEN;
            productCount--;
            SDL_CondSignal(cond);
            SDL_UnlockMutex(mutex);
            sleep(rand() % 5);
        }
        else
        {
            fprintf(stderr, "Couldn't lock mutex\n");
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    mutex = SDL_CreateMutex(); //申请锁变量
    if (!mutex)
    {
        fprintf(stderr, "Couldn't create mutex\n");
        return -1;
    }
    cond = SDL_CreateCond(); //申请条件变量

    if (!cond)
    {
        fprintf(stderr, "Couldn't create cond\n");
        return -1;
    }
    SDL_Thread *threadA;
    SDL_Thread *threadB;
    int threadReturnValueA;
    int threadReturnValueB;
    threadA = SDL_CreateThread(producerThread, "A", (void *)NULL);

    if (NULL == threadA)
    {
        printf("\nSDL_CreateThread A failed: %s\n", SDL_GetError());
    }

    threadB = SDL_CreateThread(consumerThread, "B", (void *)NULL);
    if (NULL == threadB)
    {
        printf("\nSDL_CreateThread B failed: %s\n", SDL_GetError());
    }
    SDL_WaitThread(threadA, &threadReturnValueA);
    SDL_WaitThread(threadB, &threadReturnValueB);

    return 0;
}
