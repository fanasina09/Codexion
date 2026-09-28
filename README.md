*This project has been created as part of the 42 curriculum by faharila.*

# Codexion

> Master the race for resources before the deadline masters you.

## Description

**Codexion** is a concurrency project written in C. It simulates a group of coders sitting around a circular co-working hub, sharing a **Quantum Compiler**. To compile, a coder must hold **two USB dongles at the same time** (one on the left, one on the right), but there are only as many dongles as coders, so neighbours compete for them.

Each coder loops through three phases: **compile → debug → refactor**. If a coder does not start compiling within `time_to_burnout` milliseconds (counted from the start of their last compile, or from the start of the simulation), they **burn out** and the simulation stops. The simulation also stops once every coder has compiled at least `number_of_compiles_required` times.

The goal is to design a fair and efficient protocol for sharing the dongles, using only POSIX threads, mutexes and condition variables, while avoiding deadlocks, starvation and race conditions.

### Overview of the rules

- One thread per coder (`pthread_create`), plus one separate **monitor** thread.
- One dongle between each pair of adjacent coders (a single dongle if there is only one coder).
- Each dongle is protected by a mutex (`pthread_mutex_t`); condition variables (`pthread_cond_t`) manage the waiting queues.
- After being released, a dongle is **unavailable** for `dongle_cooldown` ms.
- Arbitration between coders requesting the same dongle follows the chosen `scheduler`:
  - `fifo`: First In, First Out (arrival order).
  - `edf`: Earliest Deadline First, with `deadline = last_compile_start + time_to_burnout`.
- The scheduling queue is implemented with a **custom priority queue (binary heap)**. No standard library priority queue is used.
- Logging is serialized with a mutex so messages never interleave.
- A burnout must be reported **within 10 ms** of the actual burnout.
- No global variables.

## Instructions

### Requirements

- A Unix-like system (Linux / macOS)
- `cc` and `make`
- POSIX threads (`-pthread`)

### Compilation

```bash
make        # builds the ./codexion executable
make clean  # removes object files
make fclean # removes object files and the executable
make re     # fclean + all
```

The project is compiled with `cc -Wall -Wextra -Werror -pthread`.

### Execution

```bash
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
           time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

| Argument | Unit | Description |
|---|---|---|
| `number_of_coders` | - | Number of coders (and of dongles) |
| `time_to_burnout` | ms | Max delay between the start of two compiles before a coder burns out |
| `time_to_compile` | ms | Duration of a compile (two dongles held) |
| `time_to_debug` | ms | Duration of the debug phase |
| `time_to_refactor` | ms | Duration of the refactor phase |
| `number_of_compiles_required` | - | The simulation stops once every coder has compiled at least this many times |
| `dongle_cooldown` | ms | Time a dongle stays unavailable after being released |
| `scheduler` | - | `fifo` or `edf` (exactly) |

All arguments are mandatory. Invalid inputs (negative numbers, non-integers, unknown scheduler, wrong argument count) are rejected with an error message.

### Usage examples

```bash
./codexion 5 800 200 200 200 7 50 fifo
./codexion 5 800 200 200 200 7 50 edf
./codexion 1 800 200 200 200 3 50 fifo   # single coder: only one dongle, burns out
```

### Log format

Every state change is printed on its own line, with the timestamp in milliseconds and the coder number:

```
timestamp_in_ms X has taken a dongle
timestamp_in_ms X is compiling
timestamp_in_ms X is debugging
timestamp_in_ms X is refactoring
timestamp_in_ms X burned out
```

## Blocking cases handled

### Deadlock prevention (Coffman's conditions)

A deadlock requires the four Coffman conditions to hold at the same time. The solution breaks them as follows:

| Condition | How it is handled |
|---|---|
| Mutual exclusion | Inherent to the problem (a dongle is used by one coder at a time). Kept, and enforced with one mutex per dongle. |
| Hold and wait | A coder only starts compiling once it holds both dongles; dongle acquisition is arbitrated by the scheduler queue. |
| No preemption | Kept: a dongle is only released voluntarily after compiling. |
| Circular wait | Broken by imposing a global acquisition order on the dongles (e.g. lowest dongle id first), so a cycle of waiting coders cannot form. |

> Adapt this table to the exact strategy implemented in the code.

### Starvation prevention

- With `edf`, the coder whose burnout deadline is the closest is served first, so a coder close to burning out cannot be overtaken indefinitely.
- With `fifo`, requests are served strictly in arrival order.
- Both policies rely on the custom heap; a **deterministic tie-breaker** (e.g. lowest coder id) is applied when two deadlines are equal, so the EDF policy is fully deterministic.

### Cooldown handling

When a dongle is released, its release timestamp is recorded. A dongle is considered available only when `now >= release_time + dongle_cooldown`. Coders waiting on a dongle in cooldown sleep using `pthread_cond_timedwait` with the exact remaining time instead of busy-waiting.

### Precise burnout detection

A dedicated **monitor thread** loops with a very short period (a few milliseconds at most) and checks, for each coder, whether `now - last_compile_start >= time_to_burnout`. Since the check period is far below 10 ms, the `burned out` message is printed within the 10 ms tolerance required by the subject. Once a burnout is detected, the monitor sets the shared stop flag and prints the message.

### Log serialization

All output goes through a single logging function protected by a dedicated mutex. A message is fully written before the mutex is released, so two messages can never be mixed on the same line. The function also checks the stop flag, so nothing is printed after the burnout message.

### Edge cases

- **Single coder**: only one dongle exists, so the coder can never hold two dongles, cannot compile, and burns out after `time_to_burnout`.
- **Invalid arguments**: rejected before any thread or allocation is created.
- **Clean shutdown**: all threads are joined and all mutexes/condition variables are destroyed and all memory freed (no leaks).

## Thread synchronization mechanisms

### Primitives used

| Primitive | Purpose |
|---|---|
| `pthread_t` (one per coder + one monitor) | Concurrent execution of coders and burnout detection |
| `pthread_mutex_t` per dongle | Protects the dongle state (taken/free, release timestamp, waiting queue) |
| `pthread_cond_t` per dongle | Lets coders sleep until the dongle is granted or its cooldown expires (`pthread_cond_wait` / `pthread_cond_timedwait`), woken by `pthread_cond_signal` / `pthread_cond_broadcast` |
| `pthread_mutex_t` for logging | Serializes output lines |
| `pthread_mutex_t` for simulation state | Protects the stop flag shared by coders and monitor |
| `pthread_mutex_t` per coder (or shared state mutex) | Protects `last_compile_start` and the compile counter read by the monitor |
| Custom binary heap | Priority queue for fifo/edf arbitration of each dongle's waiting coders |

### How resources are coordinated

- **Dongles**: a coder pushes a request into the dongle's heap, then waits on the dongle's condition variable until it is at the top of the heap **and** the dongle is free and out of cooldown. The check and the state change happen under the dongle mutex.
- **Logging**: any thread that wants to print calls the same function, which locks the log mutex, checks the stop flag, prints, then unlocks.
- **Monitor state**: the stop flag and per-coder timing data are only read or written while holding their mutex, so the monitor always sees consistent values.

### Race conditions prevented (examples)

- *Two coders taking the same dongle*: the "is the dongle free?" test and the "mark it as taken" write are done in the same critical section under the dongle mutex.
- *Monitor reading a timestamp while a coder updates it*: `last_compile_start` is updated and read under the same mutex.
- *Interleaved output*: only one thread at a time can hold the log mutex.
- *Coder continuing after the simulation ended*: the stop flag is checked (under its mutex) at each phase transition and inside waiting loops, so all threads exit cleanly.

### Thread-safe communication between coders and the monitor

Coders never talk to each other. The only communication channels are:

1. Shared per-coder data (`last_compile_start`, compile count) written by the coder and read by the monitor, protected by a mutex.
2. The shared stop flag written by the monitor (burnout) or by the completion check, and read by every coder, protected by a mutex.

## Resources

### References

- POSIX Threads Programming (LLNL): https://hpc-tutorials.llnl.gov/posix/
- `man pthread_create`, `man pthread_mutex_lock`, `man pthread_cond_wait`, `man pthread_cond_timedwait`
- `man gettimeofday`, `man clock_gettime`, `man usleep`
- Coffman conditions (deadlock): https://en.wikipedia.org/wiki/Deadlock_(computer_science)
- Dining philosophers problem: https://en.wikipedia.org/wiki/Dining_philosophers_problem
- Earliest deadline first scheduling: https://en.wikipedia.org/wiki/Earliest_deadline_first_scheduling
- Binary heap: https://en.wikipedia.org/wiki/Binary_heap

### Use of AI

> Fill this section honestly with what you actually did. Example structure:

- **Tasks where AI was used**: e.g. understanding the subject, brainstorming the architecture, explaining Coffman's conditions, reviewing the README structure.
- **Parts of the project concerned**: e.g. planning / documentation.
- **What was verified and understood**: all AI-generated content was reviewed, tested and discussed with peers; no code is used that cannot be explained during the defense.
