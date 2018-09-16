/*
 * Simulation of M/M/1 queue with and without control of queue length
 *
 *   https://en.wikipedia.org/wiki/M/M/1_queue
 *
 *   "In queueing theory, a discipline within the mathematical theory of
 *   probability, an M/M/1 queue represents the queue length in a system
 *   having a single server, where arrivals are determined by a Poisson
 *   process and job service times have an exponential distribution.
 *   The model name is written in Kendall's notation. The model is the
 *   most elementary of queueing models and an attractive object of study
 *   as closed-form expressions can be obtained for many metrics of interest
 *   in this model."
 *
 * Approach (following Bertsekas and Tsitsiklis, 2008):
 *
 * - Arrivals: Use a Bernoulli process as a discrete approximation of the
 *   Poisson process.
 * - Interprete a Bernoulli process as a sequence of independent Bernoulli
 *   random variables with probability prob_1 of success at any given trial,
 *   a trial being, e.g., the flip of an unfair coin.
 * - Service times: Use a geometric distribution as a discrete approximation
 *   of the exponential distribution.
 * - Interprete a geometric random variable in terms of repeated independent
 *   trials with probability prob_2 of success (again, a trial being, e.g., 
 *   the flip of an unfair coin) until the first success.
 * - Discrete time: Loop with one iteration being one time step.
 * - In each iteration, use random number generator to generate arrivals
 *   with probabilty prob_1 and departures with probability prob_2. 
 *   (Either one or zero arrivals per time step, either one or zero 
 *   departures per time step).
 * - Keep track of queue length.
 * - Departures are only possible when queue length > 0 (after arrivals).
 * - Simple control mechanism: truncate every 10 steps to limit = x elements
 *   in queue.
 * 
 * Implementation of FIFO queue using array, following Cormen, Leiserson, 
 * Rivest and Stein (2009), p. 234
 * (Description and example: see fifo_example.c)
 *
 * In this example, the array size is 20, and the elements in the queue
 * are strings of size 2 (3 including \0). 
 */
#include <stdio.h>
#include <stdlib.h>

/*
 * Function enqeue
 *
 * Parameters:
 *   fifo:    array of strings (char pointers)
 *   size:    size of array
 *   p_head:  pointer to index head
 *   p_tail:  pointer to index tail
 *   arrival: string (char pointer) to be enqueued
 *
 * Return value:
 *   0: no error
 *   1: overflow
 */
int enqueue(char *fifo[], int size, int *p_head, int *p_tail, char *arrival) {
    fifo[*p_tail] = arrival; 
    *p_tail = (*p_tail + 1) % size; 
    // next slot must be empty, otherwise overflow
    if (!fifo[*p_tail]) {
        return 0;
    } else {
        return 1;
    } 
}

/*
 * Function deqeue
 *
 * Parameters:
 *   fifo:    array of strings (char pointers)
 *   size:    size of array
 *   p_head:  pointer to index head
 *   p_tail:  pointer to index tail
 *
 * Return value:
 *   dequeued string (char pointer) 
 *   NULL if queue was empty
 */
char* dequeue(char *fifo[], int size, int *p_head, int *p_tail) {
    char *departure = fifo[*p_head];
    if (departure) {
        fifo[*p_head] = NULL;
        if (*p_head != *p_tail) {
            *p_head = (*p_head + 1) % size; 
        }
    }
    return departure;
}

/*
 * Function check_and_truncate
 *
 * Parameters:
 *   fifo:    array of strings (char pointers)
 *   size:    size of array
 *   p_head:  pointer to index head
 *   limit:   if queue length exceeds this limit, truncate to this limit
 */
void check_and_truncate(char *fifo[], int size, int *p_head, int limit) {
    int queue_length = 0;
    for(int i = 0; i < size; i++) {
        if (fifo[i]) {
            queue_length++;
        }
    }
    while (fifo[*p_head] && queue_length > limit) {
        fifo[*p_head] = NULL; 
        *p_head = (*p_head + 1) % size; 
        queue_length--;
    } 
}

// Helper functions for messages

void check_departure(char *departure) {
    if (departure) {
        printf("departure %c%c\\%i\n", departure[0], departure[1], 
                                       (int)departure[2]);
    } else {
        puts("no departure");
    }
}

void show_queue(char *fifo[], int array_size) {
    int queue_length = 0;
    char visualization[array_size + 1];  
    for(int i = 0; i < array_size; i++) {
        if (fifo[i]) {
            queue_length++;
            visualization[i] = '*';
        } else {
            visualization[i] = ' ';
        }
    }
    visualization[array_size] = '\0';
    printf(" %s %i\n", visualization, queue_length);
}

// Function main with examples

int main() {
    int array_size = 20;
    char *fifo[] = {NULL, NULL, NULL, NULL, NULL, 
                    NULL, NULL, NULL, NULL, NULL, 
                    NULL, NULL, NULL, NULL, NULL, 
                    NULL, NULL, NULL, NULL, NULL};
    int head = 0;
    int tail = 0;
    int *p_head = &head;
    int *p_tail = &tail;
    int status = 0;
    int iterations = 0;
    char *departure;

    // Choose example (see below)
    int EXAMPLE = 8;
    
    switch(EXAMPLE) {
    /* 
     * Example 1
     * Enqueuing one element in each iteration
     */
    case 1:
        while ((status == 0) && (iterations < 100)) {
            iterations++;
            status = enqueue(fifo, array_size, p_head, p_tail, "ab");
            show_queue(fifo, array_size);
        }
        if (status == 1) {
            puts("OVERFLOW!"); 
        }
        break;        
    /* 
     * Example 2
     * Enqueuing one element in each iteration
     * Dequeuing one element in every second iteration
     */
    case 2:
        while ((status == 0) && (iterations < 100)) {
            iterations++;
            status = enqueue(fifo, array_size, p_head, p_tail, "ab");
            if (iterations % 2 == 0) {
                departure = dequeue(fifo, array_size, p_head, p_tail);
                //check_departure(departure);
            }
            show_queue(fifo, array_size);
        }
        if (status == 1) {
            puts("OVERFLOW!"); 
        }
        break;        
    /* 
     * Example 3
     * Enqueuing with probability 0.5
     * Dequeuing with probability 0.5
     * Without control
     */
    case 3:
        srand(1234);
        while ((status == 0) && (iterations < 1000)) {
            iterations++;
            if (rand() % 100 < 50) {
                status = enqueue(fifo, array_size, p_head, p_tail, "ab");
            }
            if (rand() % 100 < 50) {
                departure = dequeue(fifo, array_size, p_head, p_tail);
            }
            show_queue(fifo, array_size);
        }
        if (status == 1) {
            puts("OVERFLOW!"); 
        }
        break;        
    /* 
     * Example 4
     * Enqueuing with probability 0.2
     * Dequeuing with probability 0.4
     * Without control
     */
    case 4:
        srand(1234);
        while ((status == 0) && (iterations < 1000)) {
            iterations++;
            if (rand() % 100 < 20) {
                status = enqueue(fifo, array_size, p_head, p_tail, "ab");
            }
            if (rand() % 100 < 40) {
                departure = dequeue(fifo, array_size, p_head, p_tail);
            }
            show_queue(fifo, array_size);
        }
        if (status == 1) {
            puts("OVERFLOW!"); 
        }
        break;        
    /* 
     * Example 5
     * Enqueuing with probability 0.4
     * Dequeuing with probability 0.2
     * Without control
     */
    case 5:
        srand(1234);
        while ((status == 0) && (iterations < 1000)) {
            iterations++;
            if (rand() % 100 < 40) {
                status = enqueue(fifo, array_size, p_head, p_tail, "ab");
            }
            if (rand() % 100 < 20) {
                departure = dequeue(fifo, array_size, p_head, p_tail);
            }
            show_queue(fifo, array_size);
        }
        if (status == 1) {
            puts("OVERFLOW!"); 
        }
        break;        
    /* 
     * Example 6
     * Enqueuing with probability 0.49
     * Dequeuing with probability 0.52
     * Without control
     */
    case 6:
        srand(1234);
        while ((status == 0) && (iterations < 1000)) {
            iterations++;
            if (rand() % 100 < 49) {
                status = enqueue(fifo, array_size, p_head, p_tail, "ab");
            }
            if (rand() % 100 < 52) {
                departure = dequeue(fifo, array_size, p_head, p_tail);
            }
            show_queue(fifo, array_size);
        }
        if (status == 1) {
            puts("OVERFLOW!"); 
        }
        break;        
    /* 
     * Example 7
     * Enqueuing with probability 0.4
     * Dequeuing with probability 0.2
     * With control
     */
    case 7:
        srand(1234);
        while ((status == 0) && (iterations < 1000)) {
            iterations++;
            if (rand() % 100 < 40) {
                status = enqueue(fifo, array_size, p_head, p_tail, "ab");
            }
            if (rand() % 100 < 20) {
                departure = dequeue(fifo, array_size, p_head, p_tail);
            }
            // truncate every 10 steps to 2 elements in  queue
            if (iterations % 10 == 0) {
                check_and_truncate(fifo, array_size, p_head, 2);
            }
            show_queue(fifo, array_size);
        }
        if (status == 1) {
            puts("OVERFLOW!"); 
        }
        break;        
    /* 
     * Example 8
     * Enqueuing with probability 0.49
     * Dequeuing with probability 0.52
     * With control
     */
    case 8:
        srand(1234);
        while ((status == 0) && (iterations < 1000)) {
            iterations++;
            if (rand() % 100 < 49) {
                status = enqueue(fifo, array_size, p_head, p_tail, "ab");
            }
            if (rand() % 100 < 52) {
                departure = dequeue(fifo, array_size, p_head, p_tail);
            }
            // truncate every 10 steps to 2 elements in  queue
            if (iterations % 10 == 0) {
                check_and_truncate(fifo, array_size, p_head, 2);
            }
            show_queue(fifo, array_size);
        }
        if (status == 1) {
            puts("OVERFLOW!"); 
        }
        break;        
    }

}

