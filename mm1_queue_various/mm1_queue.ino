/*
 * Simulation of M/M/1 queue with and without control of queue length.
 * The queue is visualized using a LED dot matrix (www.elegoo.com).
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
 * Approach, following Bertsekas and Tsitsiklis (2008):
 *
 * - Arrivals: Use a Bernoulli process as a discrete approximation of the
 *   Poisson process.
 * - Interpret a Bernoulli process as a sequence of independent Bernoulli
 *   random variables with probability prob_1 of success at any given trial,
 *   a trial being, e.g., the flip of an unfair coin.
 * - Service times: Use a geometric distribution as a discrete approximation
 *   of the exponential distribution.
 * - Interpret a geometric random variable in terms of repeated independent
 *   trials with probability prob_2 of success (again, a trial being, e.g.,
 *   the flip of an unfair coin) until the first success.
 * - Discrete time: Loop with one iteration being one time step.
 * - In each iteration, use random number generator to generate arrivals
 *   with probability prob_1 and departures with probability prob_2.
 *   (Either one or zero arrivals per time step, either one or zero
 *   departures per time step).
 * - Keep track of queue length.
 * - Departures are only possible when queue length > 0 (after arrivals).
 * - Simple control mechanism: truncate every 10 steps to limit = x elements
 *   in queue.
 *
 * Implementation of a FIFO queue using an array,
 * following Cormen, Leiserson, Rivest and Stein (2009), p. 234:
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
 * In this example, the array size is 64, and the elements in the queue
 * are strings of size 2 (3 including \0).
 */
#include <stdio.h>
#include <stdlib.h>
#include <Arduino.h>
#include <LedControl.h>

/*
 * pin 12: DataIn
 * pin 11: LOAD(CS)
 * pin 10: CLK
 * One MAX72XX, 8x8 LEDs
 */
LedControl lc = LedControl(12, 10, 11, 1);

int array_size = 64;
char *fifo[] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
int head = 0;
int tail = 0;
int *p_head = &head;
int *p_tail = &tail;
int iterations = 0;
int queue_length = 0;
int status = 0;
char *departure;
char input_string[3] = "ab";

/*
 * Function enqueue
 *   Add new item to queue
 *
 * Parameters:
 *   fifo:    array of strings (char pointers)
 *   size:    size of array
 *   p_tail:  pointer to index tail
 *   arrival: string (char pointer) to be enqueued
 *
 * Return value:
 *   0: no error
 *   1: overflow
 */
int enqueue(char *fifo[], int size, int *p_tail, char *arrival) {
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
 * Function dequeue
 *   Remove item from queue, FIFO
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
 * Function get_queue_length
 *
 * Parameters:
 *   fifo:       array of strings (char pointers)
 *   array_size: size of array
 */
int get_queue_length(char *fifo[], int array_size) {
	int queue_length = 0;
	for (int i = 0; i < array_size; i++) {
		if (fifo[i]) {
			queue_length++;
		}
	}
	return queue_length;
}

/*
 * Function check_and_truncate
 *   If queue length exceeds the limit, truncate to this limit
 *   (current implementation: truncate starting from head of queue)
 *
 * Parameters:
 *   fifo:    array of strings (char pointers)
 *   size:    size of array
 *   p_head:  pointer to index head
 *   limit:   desired limit of queue length
 */
void check_and_truncate(char *fifo[], int size, int *p_head, int limit) {
	int queue_length = get_queue_length(fifo, size);
	while (fifo[*p_head] && queue_length > limit) {
		fifo[*p_head] = NULL;
		*p_head = (*p_head + 1) % size;
		queue_length--;
	}
}

void setup() {
	// The MAX72XX is in power-saving mode on startup
	lc.shutdown(0, false);
	// Set the brightness to a medium value
	lc.setIntensity(0, 8);
	// clear the display
	lc.clearDisplay(0);
	// set seed for random number generator
	srand(1234);
}

void write_led_matrix(int queue_length) {
	lc.clearDisplay(0);
	int counter = 0;
	for (int row = 0; row < 8; row++) {
		for (int col = 0; col < 8; col++) {
			if (counter < queue_length) {
				lc.setLed(0, row, col, true);
				counter++;
			}
		}
	}
}

void loop() {
	if (status == 0) {
		iterations++;
		// Enqueueing with probability 0.25, 0.30, 0.35
		if (rand() % 100 < 35) {
			status = enqueue(fifo, array_size, p_tail, input_string);
		}
		// Dequeueing with probability 0.30
		if (rand() % 100 < 30) {
			departure = dequeue(fifo, array_size, p_head, p_tail);
		}
		// Control: truncate every 10 steps to 2 elements in queue
		///*
		if (iterations % 10 == 0) {
			check_and_truncate(fifo, array_size, p_head, 2);
			iterations = 0;
		}
		//*/
		queue_length = get_queue_length(fifo, array_size);
		write_led_matrix(queue_length);
	} else {
		// overflow
		write_led_matrix(array_size);
	}
	delay(10);
}
