/**************************************************************
* Class:  CSC-415-02 Fall 2021
* Name: Ramit Singh
* Student ID: 918213925
* GitHub ID: ramitsingh447
* Project: Assignment 4 – Word Blast
*
* File: singh_ramit_HW4_main.c
*
* Description: This assignment is about creating a program that will tend to read words 6 or more charcters long from a file. 
*
**************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#define FREQ 10000
#define MINCHARS 6 
#define TOP 10     

// You may find this Useful
char *delim = "\"\'.“”‘’?:;-,—*($%)! \t\n\x0A\r";
int count = 0; // declaring count variable
int rem = 0;   // declaring remaninng variable
int flag = 0;  // declaring flag
int fileDesc, chunkSize; // declaring filedescription and chunk size

pthread_mutex_t lock;
typedef struct Pair 
{
    char *word;
    int count;
} Pair;
struct Pair words[FREQ]; 
void initWords() 
{
    for (int i = 0; i < FREQ; i++)
    {
        words[i].word = malloc(TOP);
        words[i].count = 0;
    }
    flag = 1;
}
void *addWord(void *p) //called by each thread to process the chunks
{
    int compare; // Intializing the compare
    char *buf, *eachWord; 
    buf = malloc(chunkSize + rem); //allocates the buffer
    if (buf == NULL) //malloc error check
    {
        printf("ERROR allocating buffer"); // print statemnt for allocating the buffer
        return NULL;
    }
    read(fileDesc, buf, chunkSize + rem); //reads file into buffer
    //strtok_r is thread safe and tokenizes words and loops through the buffer
    while ((eachWord = strtok_r(buf, delim, &buf)))
    {
        if (strlen(eachWord) >= MINCHARS) //only storing words > 6 letters
        {
            for (int i = 0; i < FREQ; i++) //checking if word is already in words(array)
            {
                compare = strcasecmp(words[i].word, eachWord);
                if (compare == 0) //increase count
                {
                    pthread_mutex_lock(&lock); // To see if the threads are running properly
                    words[i].count++;
                    pthread_mutex_unlock(&lock); // To see if the threads are not over running on each other
                    break;
                }
            }
            if (compare != 0 && count < FREQ) //checking if word is not in words(array)
            { //add word and increase count
                pthread_mutex_lock(&lock);
                strcpy(words[count].word, eachWord);
                words[count].count++;
                pthread_mutex_unlock(&lock);
                count++;
            }
        }
    }
    return 0;
}
int main(int argc, char *argv[])
{
    int fileSize, threadCount; // Initilzing filesize and threadcount
    Pair temp; // using as temporary variable
    char *filename; // Declared filename
    filename = argv[1];
    if (argc > 2) 
    {
        threadCount = atoi(argv[2]);
    }
    if (flag == 0) //initializes words array
    {
        initWords();
    }
    if (pthread_mutex_init(&lock, NULL) != 0) 
    {
        printf("ERROR: Mutex init failed\n");
        return 1;
    }
    fileDesc = open(filename, O_RDONLY);     
    fileSize = lseek(fileDesc, 0, SEEK_END); 
    lseek(fileDesc, 0, SEEK_SET);            
    chunkSize = fileSize / threadCount;      

    //**************************************************************
    // DO NOT CHANGE THIS BLOCK
    //Time stamp start
    struct timespec startTime;
    struct timespec endTime;
    clock_gettime(CLOCK_REALTIME, &startTime);

    //**************************************************************
    // *** TO DO ***  start your thread processing
    //  wait for the threads to finish
    pthread_t thread[threadCount]; //declaring pthread and array of threadCount
    for (int i = 0; i < threadCount; i++) //creating threads to run in parallel
    {
        if (i == threadCount - 1) //last thread adjustment
        {
            rem = fileSize % threadCount; // remaining is equal to the filesize and threadcount
        }
        pthread_create(&thread[i], NULL, addWord, (void *)&i); //creates thread
    }
    for (int i = 0; i < threadCount; i++) //wait for thread to finish
    {
        pthread_join(thread[i], NULL);
    }
    // ***TO DO *** Process TOP 10 and display
    for (int i = 0; i < FREQ; i++) //sorting the words in decreasing order of frequency
    {
        for (int m = i + 1; m < FREQ; m++)
        {
            if (words[i].count < words[m].count)
            {
                temp = words[i];
                words[i] = words[m];
                words[m] = temp;
            }
        }

    }
    printf("\nWord Frequency Count on %s with %d threads\n", filename, threadCount);
    printf("Printing top %d words %d characters or more.\n", TOP, MINCHARS);

    for (int i = 0; i < TOP; i++) //printing top 10 words
    {
        printf("Number %d is %s with a count of %d\n", i + 1, words[i].word, words[i].count);
    }
    //**************************************************************
    // DO NOT CHANGE THIS BLOCK
    clock_gettime(CLOCK_REALTIME, &endTime);
    time_t sec = endTime.tv_sec - startTime.tv_sec;
    long n_sec = endTime.tv_nsec - startTime.tv_nsec;
    if (endTime.tv_nsec < startTime.tv_nsec)
    {
        --sec;
        n_sec = n_sec + 1000000000L;
    }
    printf("Total Time was %ld.%09ld seconds\n", sec, n_sec);

    //**************************************************************
    // ***TO DO *** cleanup
    close(fileDesc);              
    pthread_mutex_destroy(&lock); 
    for (int i = 0; i < FREQ; i++)
    {
        free(words[i].word);
    }
    return 0;
}