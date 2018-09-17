#include <stdio.h>
#include <stdlib.h>
#include <Arduino.h>
#include <LedControl.h>

/*
 * pin 12: DataIn
 * pin 11: LOAD(CS)
 * pin 10: CLK
 * One MAX7219, 8x8 LEDs
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
 *   p_tail:  pointer to index of tail
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
 *   p_head:  pointer to index of head
 *   p_tail:  pointer to index of tail
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
 *   p_head:  pointer to index of head
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
