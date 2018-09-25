rm(list = ls())

generate_time_series <- function(steps,
                                 arrival_prob,
                                 departure_prob,
                                 control = FALSE,
                                 limit = Inf) {
  queue_length <- numeric(steps)
  for (i in 1:steps) {
    # either zero or one arrivals
    arrival <- rbinom(1, size = 1, prob = arrival_prob)
    # queue length after previous time step
    # queue length is assumed to be zero if this is the first time step
    if (i == 1) {
      previous <- 0
    } else {
      previous <- queue_length[i - 1]
    }
    # either zero or one departures
    # always zero departures if queue is empty
    if (previous + arrival == 0) {
      departure <- 0
    } else {
      departure <- rbinom(1, size = 1, prob = departure_prob)
    }
    # queue length after current time step
    queue_length[i] <- previous + arrival - departure
    # if control:
    # truncate every 10 steps to 2 elements in queue
    if (control &
        i %% 10 == 0 & queue_length[i] > limit) {
      queue_length[i] <- limit
    }
  }
  queue_length
}


# global parameters ---------------------------------------------------------

p_1 <- 0.25 # arrival probability
p_2 <- 0.30 # departure probability
N <- 10000


# 1) example without and with control ---------------------------------------

print(paste("steps:",
            N))
print(paste("arrival probabilty:",
            p_1,
            "departure probabilty:",
            p_2))
print(paste(
  "probability of unused departure events when queue is empty:",
  round((1 - p_1) * p_2, 2)
))

# helper function
run_example <- function(control, limit = Inf) {
  set.seed(1234)
  
  queue_length <- generate_time_series(
    steps = N,
    arrival_prob = p_1,
    departure_prob = p_2,
    control = control,
    limit = limit
  )
  
  barplot(queue_length)
  hist(
    queue_length,
    main = paste("control = ", control,
                 " distribution queue length"),
    xlab = "",
    ylab = ""
  )
  
  print(
    paste(
      "control:",
      control,
      "( truncate to",
      limit,
      ")",
      "queue length zero (%):",
      round(100 * mean(queue_length == 0), 1),
      "median:",
      median(queue_length),
      "mean:",
      round(mean(queue_length), 1),
      "max:",
      max(queue_length)
    )
  )
}

run_example(control = FALSE)
run_example(control = TRUE, limit = 2)


# 2) distribution of various statistics without and with control ------------

# helper function
run_stats <- function(control, limit = Inf) {
  set.seed(1234)
  
  stats <- replicate(100, {
    ts <- generate_time_series(
      steps = N,
      arrival_prob = p_1,
      departure_prob = p_2,
      control = control,
      limit = limit
    )
    c(
      queue_zero = round(100 * mean(ts == 0), 1),
      median = median(ts),
      mean = mean(ts),
      max = max(ts)
    )
  })
  
  stats_df <- as.data.frame(t(stats))
  
  hist(
    stats_df$queue_zero,
    main = paste("control = ", control,
                 " distribution % queue zero"),
    xlab = "",
    ylab = ""
  )
  hist(
    stats_df$median,
    main = paste("control = ", control,
                 " distribution median"),
    xlab = "",
    ylab = ""
  )
  hist(
    stats_df$mean,
    main = paste("control = ", control,
                 " distribution mean"),
    xlab = "",
    ylab = ""
  )
  hist(
    stats_df$max,
    main = paste("control = ", control,
                 " distribution max"),
    xlab = "",
    ylab = ""
  )
}

run_stats(control = FALSE)
run_stats(control = TRUE, limit = 2)
