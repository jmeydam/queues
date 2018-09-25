#include <stdio.h>
#include <stdlib.h>
#include <Arduino.h>
#include <LedControl.h>

/*
 * set up led matrix for visualization
 */

// pin 10: CLK
const int CLK = 10;
// pin 11: LOAD(CS)
const int LOAD = 11;
// pin 12: DataIn
const int DATA_IN = 12;
// one MAX7219, 8x8 LEDs
const int MAX7219 = 1;

LedControl lc = LedControl(DATA_IN, CLK, LOAD, MAX7219);

/*
 * parameters of simulation
 */

// seed for random number generator
const int SEED = 1234;
// enqueueing with probability ARRIVAL_PROB (in %)
// try: 25.0, 30.0, 35.0
const float ARRIVAL_PROB = 35.0;
// dequeueing with probability DEPARTURE_PROB (in %)
const float DEPARTURE_PROB = 30.0;
// enable ('Y') or disable ('N') control
const char CONTROL = 'Y';
// check and truncate queue every QUEUE_CONTROL_INTERVAL iterations
const int QUEUE_CONTROL_INTERVAL = 10;
// truncate queue to QUEUE_CONTROL_LIMIT
const int QUEUE_CONTROL_LIMIT = 2;
// delay between iterations (in milliseconds)
const int DELAY = 10;

/*
 * constants and variables for implementation of FIFO queue
 * (see also README and functions and comments below)
 */

const int ARRAY_SIZE = 64;
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

// status of queueing system
// 0: no error
// 1: overflow
int system_status = 0;

// dummy input value, to be enqueued
char arrival[3] = "ab";

// pointer to dequeued value
// NULL if queue was empty
char *departure = NULL;

/*
 * function enqueue
 *   add new item to queue
 *
 * parameters:
 *   fifo:    array of strings (char pointers)
 *   size:    size of array
 *   p_tail:  pointer to index of tail
 *   arrival: string (char pointer) to be enqueued
 *
 * return value:
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
 * function dequeue
 *   remove item from queue, FIFO
 *
 * parameters:
 *   fifo:    array of strings (char pointers)
 *   size:    size of array
 *   p_head:  pointer to index of head
 *   p_tail:  pointer to index of tail
 *
 * return value:
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
 * function get_queue_length
 *
 * parameters:
 *   fifo:       array of strings (char pointers)
 *   array_size: size of array
 *
 * return value:
 *   queue length
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
 * function check_and_truncate
 *   if queue length exceeds the limit, truncate to this limit
 *   (current implementation: truncate starting from head of queue)
 *
 * parameters:
 *   fifo:    array of strings (char pointers)
 *   size:    size of array
 *   p_head:  pointer to index of head
 *   limit:   desired limit of queue length
 */
void check_and_truncate(char *fifo[], int size, int *p_head, int limit) {
	int queue_length = get_queue_length(fifo, size);
	while (fifo[*p_head] && (queue_length > limit)) {
		fifo[*p_head] = NULL;
		*p_head = (*p_head + 1) % size;
		queue_length--;
	}
}

/*
 * function write_led_matrix
 *   visualize queue length on LED matrix
 *
 * parameters:
 *   queue_length:  queue length
 */
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

void setup() {
	// the MAX72XX is in power-saving mode on startup
	lc.shutdown(0, false);
	// set the brightness to a medium value
	lc.setIntensity(0, 8);
	// clear the display
	lc.clearDisplay(0);
	// set seed for random number generator
	srand(SEED);
}

void loop() {
	if (system_status == 0) {
		iterations++;
		// enqueueing with probability ARRIVAL_PROB
		if (rand() % 100 < ARRIVAL_PROB) {
			system_status = enqueue(fifo, ARRAY_SIZE, p_tail, arrival);
		}
		// dequeueing with probability DEPARTURE_PROB
		if (rand() % 100 < DEPARTURE_PROB) {
			departure = dequeue(fifo, ARRAY_SIZE, p_head, p_tail);
		}
		// control: truncate every QUEUE_CONTROL_INTERVAL steps to
		// QUEUE_CONTROL_LIMIT elements in queue
		if ((CONTROL == 'Y') && (iterations % QUEUE_CONTROL_INTERVAL == 0)) {
			check_and_truncate(fifo, ARRAY_SIZE, p_head, QUEUE_CONTROL_LIMIT);
			iterations = 0;
		}
		queue_length = get_queue_length(fifo, ARRAY_SIZE);
		write_led_matrix(queue_length);
	} else {
		// overflow - stop simulation
		write_led_matrix(ARRAY_SIZE);
	}
	delay(DELAY);
}
