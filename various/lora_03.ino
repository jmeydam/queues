#define STRING_LENGTH 50
#define ARRAY_SIZE 20


// STRING_LENGTH:
// length of strings (including \0) to be:
// 1. read from serial interface
// 2. enqueued,
// 3. dequeued and
// 4. transmitted via LoRa
// TODO: set to appropriate value

// ARRAY_SIZE:
// size of array that holds queue

#include <stdio.h>
#include <stdlib.h>
#include <Arduino.h>

/*
 * constants and variables
 */

// check and truncate queue every QUEUE_CONTROL_INTERVAL iterations
const int QUEUE_CONTROL_INTERVAL = 10;
// truncate queue to QUEUE_CONTROL_LIMIT
const int QUEUE_CONTROL_LIMIT = 2;

// delay between iterations (in milliseconds)
// TODO: set to appropriate value
const int DELAY = 20; //3000;

// array of strings (char arrays) that holds queue
char fifo[ARRAY_SIZE][STRING_LENGTH];

// other variables for implementation of queueing system
int head = 0;
int tail = 0;
int iterations = 0;

// status of queueing system
// 0: no error
// 1: overflow
int system_status = 0;

// array for string to be enqueued
char arrival[STRING_LENGTH];

// arrival_status (after attempt to read from serial interface)
// 0: new string received and copied to arrival
// 1: no new string - arrival still contains old string
int arrival_status = 0;

// array for dequeued string
char departure[STRING_LENGTH];

// status of previous transmission attempt
// 0: transmission successful
// 1: transmission failed
int transmission_status = 0;

/*
 * function initialize_array
 *   initialize array that holds queue
 */
void initialize_array() {
	for (int i = 0; i < ARRAY_SIZE; i++) {
		for (int j = 0; j < STRING_LENGTH; j++) {
			fifo[i][j] = '\0';
		}
	}
}

/*
 * function get_string_length
 *
 * parameters:
 *   str: string (char array)
 *
 * return value:
 *   string length up to and excluding the first occurrence of '\0'
 */
int get_string_length(char str[]) {
	int string_length = 0;
	for (int i = 0; i < STRING_LENGTH; i++) {
		if (str[i] == '\0') {
			break;
		}
		string_length++;
	}
	return string_length;
}

/*
 * function set_to_empty_string
 *   overwrite string str with sequence of '\0' of length STRING_LENGTH
 *
 * parameters:
 *   str: string (char array)
 */
void set_to_empty_string(char str[]) {
	for (int i = 0; i < STRING_LENGTH; i++) {
		str[i] = '\0';
	}
}

/*
 * function get_queue_length
 *
 * return value:
 *   queue length
 */
int get_queue_length() {
	int queue_length = 0;
	for (int i = 0; i < ARRAY_SIZE; i++) {
		if (get_string_length(fifo[i]) > 0) {
			queue_length++;
		}
	}
	return queue_length;
}

/*
 * function print_queue
 */
void print_queue() {
	int queue_length = 0;
	char visualization[ARRAY_SIZE + 1];
	for (int i = 0; i < ARRAY_SIZE; i++) {
		if (get_string_length(fifo[i]) > 0) {
			queue_length++;
			visualization[i] = '*';
		} else {
			visualization[i] = ' ';
		}
	}
	visualization[ARRAY_SIZE] = '\0';
	Serial.print(visualization);
	Serial.print(" L: ");
	Serial.print(queue_length);
	Serial.print(" H: ");
	Serial.print(head);
	Serial.print(" T: ");
	Serial.println(tail);
}

/*
 * function enqueue
 *   add new item to queue
 *
 * return value:
 *   0: no error
 *   1: overflow
 */
int enqueue() {
	strcpy(fifo[tail], arrival);
	tail = (tail + 1) % ARRAY_SIZE;
	// next slot must be empty, otherwise overflow
	if (get_string_length(fifo[tail]) == 0) {
		return 0;
	} else {
		return 1;
	}
}

/*
 * function dequeue
 *   remove item from queue, FIFO
 *   removed value is copied to departure
 */
void dequeue() {
	strcpy(departure, fifo[head]);
	set_to_empty_string(fifo[head]);
	if (head != tail) {
		head = (head + 1) % ARRAY_SIZE;
	}
}

/*
 * function check_and_truncate
 *   if queue length exceeds the limit, truncate to this limit
 *   (current implementation: truncate starting from head of queue)
 *
 * parameters:
 *   limit:   desired limit of queue length
 */
void check_and_truncate(int limit) {
	int queue_length = get_queue_length();
	while ((get_string_length(fifo[head]) > 0) && (queue_length > limit)) {
		set_to_empty_string(fifo[head]);
		head = (head + 1) % ARRAY_SIZE;
		queue_length--;
	}
}

/*
 * function get_next_arrival
 *   try to read next data package (string) from serial interface
 *
 * return value:
 *   0: new string received and copied to array arrival
 *   1: no new string - array arrival still contains old string
 */
int get_next_arrival() {
	int status = 0;
	char str[STRING_LENGTH];
	set_to_empty_string(str);

	// TODO: uncomment and check carefully once serial interface 1 is set up
	/*
	// read new string from serial interface, if available
	// (up to string_length bytes including \0)
	if (Serial1.available()) {
		status = 0;
		// delay allows all bytes sent to be received together
		delay(100);
		int i = 0;
		while (Serial1.available() && i < string_length) {
			str[i] = Serial1.read();
			i++;
		}
	} else {
		status = 1;
	}
	*/

	// TODO: delete simulation code once serial interface 1 is set up
	// SIMULATION CODE START
	// for now dummy values, changing at random
	char dummy_input_1[50] = "abcdefghijabcdefghijabcdefghijabcdefghijabcdefghi"; // 49 + 1 chars
	char dummy_input_2[50] = "0123456789012345678901234567890123456789012345678"; // 49 + 1 chars
	// simulate read from serial interface - sometimes successful, sometimes not
	if (rand() % 100 < 90) {
		// simulating reading new value from serial interface
		status = 0;
		// two different values, each about half of the time
		if (rand() % 100 < 50) {
			strcpy(str, dummy_input_1);
		} else {
			strcpy(str, dummy_input_2);
		}
	} else {
		// simulating failure to read new value from serial interface
		status = 1;
	}
	// SIMULATION CODE END

	// copy input string to arrival
	strcpy(arrival, str);
	return status;
}

/*
 * function transmission_status_and_cleanup
 *   asynchronous checking of transmission status -
 *   transmission status received via downlink message
 *
 *   cleanup: departure is set to empty string if
 *   previous transmission was successful
 *
 * return value:
 *   0: previous transmission successful
 *   1: previous transmission failed
 */
int transmission_status_and_cleanup() {
	// TODO: replace simulation code with actual check
	if (rand() % 100 < 20) {
		// previous transmission successful
		set_to_empty_string(departure);
		return 0;
	} else {
		// previous transmission failed
		return 1;
	}
}

/*
 * function transmit
 *   try to transmit dequeued string (departure) via LoRa network
 *
 * no return value
 *  (asynchronous checking of transmission status -
 *   transmission status received via downlink message;
 *   see function transmission_status_and_cleanup)
 */
void transmit() {
	// TODO: replace messages with code for transmission via LoRa uplink
	if (get_string_length(departure) > 0) {
		Serial.print("to be transmitted: ");
		Serial.println(departure);
	} else {
		Serial.print("no departure to transmit.");
	}
	Serial.println("");
}

/*
 * function terminate
 *   terminate normal operation - unrecoverable system failure (overflow)
 */
void terminate() {
	Serial.println("OVERFLOW - program terminated");
	// blinking to indicate termination due to overflow
	digitalWrite(LED_BUILTIN, HIGH);
	delay(100);
	digitalWrite(LED_BUILTIN, LOW);
	delay(100);
}

void setup() {
	// setup serial ports (baud rate)
	// use default port for logging
	Serial.begin(9600);
	// use port 1 for receiving sensor data from Arduino Uno
	// TODO: uncomment:
	// Serial1.begin(9600);

	// initialize digital pin LED_BUILTIN as an output
	// (used to indicate overflow)
	pinMode(LED_BUILTIN, OUTPUT);

	// TODO: setup LoRa shield, uplink and downlink

	// TODO: remove seed - only needed for debugging with simulation
	// set seed for random number generator
	srand(1);

	// initialize array that holds queue
	initialize_array();
	// initialize arrival and departure strings
	set_to_empty_string(arrival);
	set_to_empty_string(departure);
	Serial.println("");
	Serial.println("setup completed.");
	Serial.println("");
}

void loop() {
	if (system_status == 0) {
		iterations++;
		// try to read next data package (string) from serial interface
		arrival_status = get_next_arrival();
		Serial.print("arrival status: ");
		Serial.println(arrival_status);
		// enqueueing next data package if available
		if (arrival_status == 0) {
			system_status = enqueue();
		}
		// get transmission_status of previous transmission to LoRa
		// gateway by checking if a confirmation message has been
		// received via downlink
		transmission_status = transmission_status_and_cleanup();
		Serial.print("transmission status: ");
		Serial.println(transmission_status);
		// dequeueing only if last transmission was successful
		if (transmission_status == 0) {
			dequeue();
			// try to transmit new departure
			transmit();
		} else {
			// do not dequeue,
			// retry transmission
			transmit();
		}
		Serial.println("before check and truncate: ");
		Serial.println("");
		print_queue();
		Serial.println("");
		// control: truncate every QUEUE_CONTROL_INTERVAL steps to
		// QUEUE_CONTROL_LIMIT elements in queue
		if (iterations % QUEUE_CONTROL_INTERVAL == 0) {
			check_and_truncate(QUEUE_CONTROL_LIMIT);
			iterations = 0;
			Serial.println("after check and truncate: ");
			Serial.println("");
			print_queue();
			Serial.println("");
		}
	} else {
		// overflow
		terminate();
	}
	delay(DELAY);
}
