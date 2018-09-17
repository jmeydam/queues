# Queues

Simulation of M/M/1 queue with and without control of queue length.

*mm1_queue.R* focuses on generating data for further analysis.

*various/mm1_queue.ino* is the "Arduino edition". The behavior of the 
queue is visualized using a LED dot matrix.

*various/fifo_example.c* is a stand-alone program testing the data structure.

*various/mm1_example.c* is a stand-alone program testing the behaviour of 
the queue.

[Wikipedia:](https://en.wikipedia.org/wiki/M/M/1_queue)

> In queueing theory, a discipline within the mathematical theory of
 probability, an M/M/1 queue represents the queue length in a system
 having a single server, where arrivals are determined by a Poisson
 process and job service times have an exponential distribution.
 The model name is written in Kendall's notation. The model is the
 most elementary of queueing models and an attractive object of study
 as closed-form expressions can be obtained for many metrics of interest
 in this model.

## Notes on Implementation

General approach, following Bertsekas and Tsitsiklis (2008):

* Arrivals: Use a Bernoulli process as a discrete approximation of the 
Poisson process.
* Interpret a Bernoulli process as a sequence of independent Bernoulli 
random variables with probability p_1 of success at any given trial,
a trial being, e.g., the flip of an unfair coin.
* Service times: Use a geometric distribution as a discrete approximation 
of the exponential distribution.
* Interpret a geometric random variable in terms of repeated independent 
trials with probability p_2 of success (again, a trial being, e.g., the 
flip of an unfair coin) until the first success.
* Discrete time: Loop with one iteration being one time step.
* In each iteration, use random number generator to generate arrivals with 
probability p_1 and departures with probability p_2. (Either one or 
zero arrivals per time step, either one or zero departures per time step).
* Keep track of queue length.
* Departures are only possible when queue length > 0 (after arrivals).
* Simple control mechanism: truncate every 10 steps to limit = x elements 
in queue.

Implementation queue data structure (only in C programs) using an 
array, following Cormen, Leiserson, Rivest and Stein (2009), p. 234:
 
* Queue has a head and a tail.
* Enqueueing: inserting a new element at the tail of the queue.
* Dequeueing: removing the element at the head of the queue.
* Queue implemented with an array of size n (here: initialized with NULLs).
* Queued elements placed in a sequence of consecutive array slots.
* Enqueueing adds an element at the next slot to the right.
* Dequeueing removes the leftmost element (here: set to NULL, return value).
* Sequence of consecutive array slots "wraps around": slot 0 treated as 
the element following the last slot of the array.
* For simplicity, this description mostly refers to the simple case without 
wrapping around - the queue consists of a sequence of consecutive array 
slots somewhere in the middle of the array (head leftmost element, tail 
empty slot to the right of the rightmost element)
* Overflow: tail reaches head, all array slots occupied. (To be avoided by 
sizing array appropriately. In case of overflow program should be 
terminated.)
* Here: returning NULL and no change of queue state in case of empty queue, 
thus avoiding underflow.
* Variable head used for index of element that will be dequeued next.
* Implementation of special cases: 
 * Initial state: queue empty, head points to slot 0.
 * The last remaining element has been removed: head points to slot of 
 element removed last.
* Variable tail used for index of empty slot to the right of the element 
that was enqueued last.
* Implementation of special cases:
 * Initial state: queue empty, tail points to slot 0.

In this example, the array size is 64, and the elements in the queue are 
strings of size 2 (3 bytes including \0).
