/*
 * Implementation of a FIFO queue using an array, 
 * following Cormen, Leiserson, Rivest and Stein (2009), p. 234
 *
 * - Queue has a head and a tail
 * - Enqueueing: inserting a new element at the tail of the queue
 * - Dequeueing: removing the element at the head of the queue
 * - Queue implemented with an array of size n (here: initialized with NULLs)
 * - Queued elements placed in a sequence of consecutive array slots
 * - Enqueueing adds an element at the next slot to the right
 * - Dequeueing removes the leftmost element (here: set to NULL, return value)
 * - Sequence of consecutive array slots "wraps around": slot 0 treated
 *   as the element following the last slot of the array
 * - For simplicity, this description mostly refers to the simple case 
 *   without wrapping around - the queue consists of a sequence of 
 *   consecutive array slots somewhere in the middle of the array (head 
 *   leftmost element, tail empty slot to the right of the rightmost element)
 * - Overflow: tail reaches head, all array slots occupied
 *   - To be avoided by sizing array appropriately
 *   - In case of overflow program should be terminated
 * - Here: returning NULL and no change of queue state in case of empty
 *   queue, thus avoiding underflow
 * - Variable head used for index of element that will be dequeued next
 * - Implementation of special cases:
 *   - Initial state: queue empty, head points to slot 0
 *   - The last remaining element has been removed: head points to 
 *     slot of element removed last
 * - Variable tail used for index of empty slot to the right of the element 
 *   that was enqueued last 
 * - Implementation of special cases:
 *   - Initial state: queue empty, tail points to slot 0
 *
 * In this example, the array size is 3, and the elements in the queue
 * are strings of size 2 (3 including \0). 
 */
#include <stdio.h>

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
    // messages
    puts("\nStepping into enqueue:");
    printf("  head: %i tail: %i ", *p_head, *p_tail);
    printf("arrival: %c%c\\%i \n", *arrival, *(arrival + 1), 
                                   (int) *(arrival + 2));
    // processing
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
    // messages
    puts("\nStepping into dequeue:");
    printf("  head: %i tail: %i \n", *p_head, *p_tail);
    // processing
    char *departure = fifo[*p_head];
    if (departure) {
        fifo[*p_head] = NULL;
        if (*p_head != *p_tail) {
            *p_head = (*p_head + 1) % size; 
        }
    }
    return departure;
}

// Helper functions for messages

void check_status(int status) {
    if (status == 0) {
        puts("ok");
    } else {
        puts("overflow");
    }
}

void check_departure(char *departure) {
    if (departure) {
        printf("departure %c%c\\%i\n", departure[0], departure[1], 
                                       (int)departure[2]);
    } else {
        puts("no departure");
    }
}

void print_queue(char *fifo[], int size) {
    puts("\nQueue:");
    for(int i = 0; i < size; i++) {
        if (fifo[i]) {
            printf("  %s", fifo[i]);
            printf(" (%c%c\\%i)\n", fifo[i][0], fifo[i][1], 
                                    (int)fifo[i][2]);
        } else {
            printf("  element %i is empty\n", i);
        }
    }
}

// Function main with example

int main() {
    int array_size = 3;
    char *fifo[] = {NULL, NULL, NULL};
    int head = 0;
    int tail = 0;
    int *p_head = &head;
    int *p_tail = &tail;
    int status;
    char *departure;

    print_queue(fifo, array_size);

    departure = dequeue(fifo, array_size, p_head, p_tail);
    check_departure(departure);

    print_queue(fifo, array_size);

    status = enqueue(fifo, array_size, p_head, p_tail, "ab");
    check_status(status);

    print_queue(fifo, array_size);

    status = enqueue(fifo, array_size, p_head, p_tail, "cd");
    check_status(status);

    departure = dequeue(fifo, array_size, p_head, p_tail);
    check_departure(departure);

    print_queue(fifo, array_size);

    status = enqueue(fifo, array_size, p_head, p_tail, "ef");
    check_status(status);

    departure = dequeue(fifo, array_size, p_head, p_tail);
    check_departure(departure);

    print_queue(fifo, array_size);

    departure = dequeue(fifo, array_size, p_head, p_tail);
    check_departure(departure);

    departure = dequeue(fifo, array_size, p_head, p_tail);
    check_departure(departure);

    print_queue(fifo, array_size);

    status = enqueue(fifo, array_size, p_head, p_tail, "gh");
    check_status(status);

    status = enqueue(fifo, array_size, p_head, p_tail, "ij");
    check_status(status);

    status = enqueue(fifo, array_size, p_head, p_tail, "kl");
    check_status(status);

    print_queue(fifo, array_size);

    puts("\nEnd of program. \n");
}

