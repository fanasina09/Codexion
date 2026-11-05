*This project has been created as part of the 42 curriculum by faharila.*

# Codexion

> Master the race for resources before the deadline masters you.

## Description

**Codexion** is a concurrency simulation written in C with POSIX threads. It is a
variation of Dijkstra's *dining philosophers* problem.

`N` coders sit around a circular co-working hub. Between every pair of
neighbours lies one USB **dongle**, so there are exactly as many dongles as
coders. A coder repeats the same cycle:

```
   ┌────────── compile ──────────┐   needs TWO dongles (left + right)
   │                             ▼
refactor ◄────────────────── debug          needs no dongle
```

* To **compile**, a coder must hold **both** of their dongles for
  `time_to_compile` ms.
* After compiling, they put both dongles back, then **debug**
  (`time_to_debug`), then **refactor** (`time_to_refactor`), then immediately
  try to compile again.
* A released dongle is unusable for `dongle_cooldown` ms (**cooldown**).
* A coder who does not *start* a compile within `time_to_burnout` ms of the
  start of their previous compile (or of the simulation) **burns out**.
* When several coders want the same dongle, the `scheduler` decides who gets it:
  `fifo` (first request first) or `edf` (earliest burnout deadline first).
* The simulation stops when a coder burns out, or when **every** coder has
  compiled at least `number_of_compiles_required` times.

The ring for `N = 4`:

```
 coder 1 ───── d1 ───── coder 2
    │                      │
    d0                     d2
    │                      │
 coder 4 ───── d3 ───── coder 3
```

Coder `k` owns `left = d(k-1)` and `right = d(k mod N)`. Every dongle is
therefore shared by exactly two coders (by one coder with itself when `N = 1`).

### What the program does

* one **thread per coder** (`pthread_create`) plus one **monitor thread** that
  detects burnout and the end of the simulation;
* one **table mutex + condition variable** protecting the state of all the
  dongles, and one **priority queue per dongle implemented as a binary heap**
  (no standard-library queue);
* **all-or-nothing acquisition**: a coder takes **both** dongles in one atomic
  step or **none** (the *hold and wait* Coffman condition is broken, so there is
  no deadlock and no starvation chain);
* strict **FIFO / EDF arbitration**, **dongle cooldown**, **serialized logs**
  (the two `has taken a dongle` lines of a coder are always consecutive),
  **burnout reported within a few milliseconds**, **no global variable**,
  **no memory leak**.

### Source files

| File | Role |
|---|---|
| `main.c` | Parses, initializes the simulation, starts the monitor and the coder threads, joins them, cleans up. |
| `parsing.c` | Argument validation (positive integers, `fifo`/`edf`) and filling of `t_sim`. |
| `routine_coder.c` | Life cycle of a coder thread: stagger, take dongles, compile, debug, refactor. |
| `take_dongles.c` | **All-or-nothing** acquisition: enqueue in the two heaps, wait until both dongles can be taken together, take both. |
| `lone_coder.c` | Special case `N = 1` (a single dongle exists): take it, then wait for the burnout. |
| `release_dongles.c` | Releases both dongles at once, stamps the cooldown start, wakes the waiters. |
| `waiting_queue.c` | Glue between the dongles and their heaps: FIFO/EDF key, enqueue in both heaps, dequeue from both. |
| `heap.h`, `heap.c`, `heap_ops.c` | Generic binary min-heap (the priority queue). |
| `monitor.c` | Monitor thread: burnout detection and "everybody is done" detection. |
| `simulation_state.c` | Shared `running` flag, `stop_simulation`, interruptible `smart_sleep`. |
| `coder_state.c` | Mutex-protected accessors of a coder's compile counter and last compile start. |
| `print.c` | Serialized logging (`print_dongles` prints the two dongle lines in one critical section). |
| `get_timestamp_ms.c` | Time helpers (`gettimeofday`). |
| `init_dongles.c`, `init_coders.c`, `cleanup.c` | Allocation, initialization and destruction. |
| `codexion.h` | Structures and prototypes. |
---

## Instructions

### Build

```bash
make          # builds ./codexion with: cc -Wall -Wextra -Werror -pthread
make clean    # removes the object files
make fclean   # removes the object files and the binary
make re       # fclean + all
```

The Makefile does not relink needlessly and rebuilds objects when `codexion.h`
or `heap.h` change.

### Run

```bash
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
           time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

| Argument | Meaning | Constraint |
|---|---|---|
| `number_of_coders` | Number of coders = number of dongles | integer ≥ 1 |
| `time_to_burnout` | Max delay (ms) between the start of two compiles | integer ≥ 1 |
| `time_to_compile` | Compile duration (ms), two dongles held | integer ≥ 1 |
| `time_to_debug` | Debug duration (ms) | integer ≥ 1 |
| `time_to_refactor` | Refactor duration (ms) | integer ≥ 1 |
| `number_of_compiles_required` | Compiles every coder must reach to end the simulation | integer ≥ 1 |
| `dongle_cooldown` | Delay (ms) before a released dongle can be taken again | integer ≥ 0 |
| `scheduler` | Arbitration policy | exactly `fifo` or `edf` |

All eight arguments are mandatory. Negative numbers, non-integers, overflowing
values, a wrong argument count or another scheduler name print an `Error: ...`
message on `stderr` and exit with status `1`. Running `./codexion` with **no
argument at all** is not an error: it prints nothing and exits with status `0`.

### Output format

Every state change is one line: `timestamp_in_ms coder_id message`. The
timestamp is the time elapsed since the start of the simulation.

```
timestamp_in_ms X has taken a dongle
timestamp_in_ms X is compiling
timestamp_in_ms X is debugging
timestamp_in_ms X is refactoring
timestamp_in_ms X burned out
```

A coder always logs its **two** `has taken a dongle` lines **together**: same
timestamp, printed under one lock, so no other message can fall between them.

### Examples

Everybody finishes (3 coders, one compile at a time is possible, plenty of slack):

```console
$ ./codexion 3 1000 200 100 100 2 10 fifo
```

A burnout, here because the parameters are impossible: with 3 coders only one
can compile at a time (a compile uses 2 of the 3 dongles), so the last coder
would have to wait for two compiles in a row (400 ms) with only 300 ms allowed:

```console
$ ./codexion 3 300 200 100 100 5 0 edf
0 1 has taken a dongle
0 1 has taken a dongle
0 1 is compiling
200 1 is debugging
200 3 has taken a dongle
200 3 has taken a dongle
200 3 is compiling
300 1 is refactoring
301 1 burned out
```

One coder, one dongle: the coder can never get a second dongle, so the only
possible outcome is a burnout at `time_to_burnout`:

```console
$ ./codexion 1 300 100 100 100 3 0 fifo
0 1 has taken a dongle
301 1 burned out
```

### Tests

```bash
valgrind --leak-check=full ./codexion 4 800 200 200 200 3 10 edf
cc -g -pthread -fsanitize=thread -Isrc src/*.c -o codexion_tsan && ./codexion_tsan 5 800 200 200 200 4 10 edf
```

---

## Architecture

### Main structures

```c
typedef struct s_dongle {           // (simplified)
    int              is_taken;       // 0 free, 1 held   (guarded by sim.table_lock)
    t_heap           waiting;        // priority queue of the requesters (same lock)
    long             released_at;    // ms timestamp of the last release (same lock)
} t_dongle;

typedef struct s_coder {            // (simplified)
    int              id;                       // 1..N
    t_dongle         *left_dongle, *right_dongle;
    pthread_t        thread;
    long             last_compile_and_start;   // ┐ protected by data_lock
    int              nb_compiles;             // ┘
    pthread_mutex_t  data_lock;
    struct s_sim     *sim;
} t_coder;

typedef struct s_sim {
    /* parsed arguments ... */
    int              running;                  // 1 while the simulation lives
    pthread_mutex_t  running_lock;             // protects running
    pthread_mutex_t  print_lock;               // serializes stdout
    pthread_mutex_t  table_lock;               // guards ALL the dongles + their heaps
    pthread_cond_t   table_cond;               // waiters sleep here (with table_lock)
    long             next_seq;                 // global arrival ticket (table_lock)
    t_dongle         *dongles;
    t_coder          *coders;
} t_sim;
```

There is **no global variable**: every thread receives a pointer to its own
`t_coder`, which points to the shared `t_sim`.

### Threads

```
main thread ── init ── launch_coders ──┬── monitor thread   (1)
                                       ├── coder thread 1
                                       ├── coder thread 2   (N)
                                       └── ...
                       pthread_join of all the threads ── cleanup
```

### Life of a coder

```c
stagger_start();                      // even ids wait time_to_compile / 2
while (is_running && compiles < required)
{
    take_dongles();                   // BOTH dongles at once, or wait holding none
    set_last_compile();               // start of the compile = new deadline origin
    print "is compiling";   smart_sleep(time_to_compile);
    release_dongles();                // both at once
    add_compile();
    print "is debugging";   smart_sleep(time_to_debug);
    print "is refactoring"; smart_sleep(time_to_refactor);
}
```

`smart_sleep` sleeps by slices of 500 µs and re-checks the `running` flag, so
no thread ever sleeps through the end of the simulation.

### Time

All times come from `gettimeofday` (milliseconds), as recommended by the
subject.

---

## Thread synchronization mechanisms

### POSIX threads in a nutshell

A POSIX thread (*pthread*) is an independent flow of execution **inside the same
process**. All threads share the same address space: the same heap, the same
`t_sim`, the same dongles. That is what makes cooperation cheap, and also what
makes it dangerous: two threads that touch the same memory at the same time,
with at least one writing, produce a **data race** (undefined behaviour in C).

The functions used here:

| Function | Use in Codexion |
|---|---|
| `pthread_create` | Starts one thread per coder and the monitor. |
| `pthread_join` | The main thread waits for every thread, so nothing is destroyed while still in use and the process exits cleanly. |
| `pthread_mutex_init / destroy` | Creates and destroys every mutex (table, coders, running flag, printing). |
| `pthread_mutex_lock / unlock` | Mutual exclusion around every shared datum. |
| `pthread_cond_init / destroy` | One condition variable for the whole table (`table_cond`). |
| `pthread_cond_wait` | A coder sleeps (no timeout) while its two dongles are not both free or another coder has priority; the mutex is released while it sleeps. |
| `pthread_cond_broadcast` | Wakes all the waiters after a release or at the end of the simulation. |
| `gettimeofday`, `usleep` | Timestamps, interruptible sleeping, and the short polling sleep used while a cooldown runs out. |

### Mutex: mutual exclusion

A mutex (`pthread_mutex_t`) lets one thread at a time run a **critical
section**. The rule applied everywhere in the code: *a shared variable is only
read or written while holding the mutex that owns it, and the section is kept as
short as possible.*

**Why one mutex for all the dongles?** All-or-nothing means *testing and changing
two dongles in one indivisible step*. With one mutex per dongle, a coder would
have to lock two mutexes (and the lock order itself becomes a deadlock topic);
with a single `table_lock`, "are both dongles free? then take both" is one
critical section, so nobody can ever see, or create, a half-taken pair.

| Mutex | Count | Protects | Taken by |
|---|---|---|---|
| `sim.table_lock` | 1 (shared by all dongles) | `is_taken`, `released_at` and the **waiting heap** of **every** dongle, and `next_seq` | `take_dongles`, `release_dongles`, `lone_coder`, `stop_simulation` (to wake waiters) |
| `coder.data_lock` | 1 per coder | `last_compile_and_start`, `nb_compiles` | the coder (writes), the monitor (reads), neighbours computing an EDF key (reads) |
| `sim.running_lock` | 1 | the `running` flag | every thread, through `is_running` / `stop_simulation` |
| `sim.print_lock` | 1 | `stdout` and the "burned out" decision | `print_status`, `print_dongles`, `print_burnout` |

Each coder's state is read through small accessors (`get_last_compile`,
`get_compiles`, ...) so that **no code touches a protected field without
locking**.

### Condition variable: waiting without burning CPU

A condition variable (`pthread_cond_t`) lets a thread **sleep until a condition
may have become true**, instead of spinning in a loop. It is always used
*together with a mutex*:

```c
pthread_mutex_lock(&sim->table_lock);
enter_queues(coder);                                  // queue on BOTH dongles
while (is_running(sim) && !(is_available(coder) && cooldown_over(coder)))
    wait_turn(coder);                                 // pthread_cond_wait / short sleep
leave_queues(coder);
left->is_taken = right->is_taken = 1;                 // both, in the same critical section
pthread_mutex_unlock(&sim->table_lock);
...
```

Why this exact shape:

* **`while`, not `if`.** `pthread_cond_wait` may return spuriously, and several
  waiters may be woken at once but only some can proceed. The condition
  (`is_available`: both dongles free? am I the root of **both** heaps?
  `cooldown_over`: is the cooldown of both dongles over?) is re-evaluated after
  every wake-up.
* **The test and the sleep are atomic.** `pthread_cond_wait` releases the
  mutex and puts the thread to sleep in one atomic step, and re-acquires the mutex
  before returning. The condition is tested while holding the mutex, and the
  state it depends on (`is_taken`, the heap) is only changed while holding the
  same mutex. A release between "I tested" and "I sleep" is therefore
  impossible: **no lost wake-up**.
* **`broadcast`, not `signal`.** A release can make *some* waiter eligible, but
  which one depends on the heaps of two dongles. `signal` could wake a waiter
  that goes straight back to sleep while the eligible one keeps sleeping.
  `broadcast` wakes everybody and each re-tests its own condition. The price is
  that up to `N` threads wake up at each release (see *Known limits*).
* **Two kinds of waiting** (`wait_turn` in `take_dongles.c`):
  1. **A dongle is taken, or another coder is ahead in one of the two queues** →
     `pthread_cond_wait`, with **no timeout**. This is safe because every way
     these situations can end is followed by a broadcast:
     * a taken dongle becomes free in `release_dongles`, which broadcasts;
     * a coder leaves the queues only by *taking* its two dongles (the waiters'
       next event is then their release, which broadcasts) or when the
       simulation stops (`stop_simulation` broadcasts).
  2. **Both dongles are free, the coder is at the head of both queues, but the
     cooldown is not over** → nobody can signal the end of a cooldown (it is just
     time passing), so the coder unlocks the mutex, sleeps 500 µs and re-tests.
     The cooldown therefore ends within about half a millisecond.
* **Why not `pthread_cond_timedwait`?** An earlier version used a 1 ms timed wait
  for both cases. It worked, but Helgrind reports `dubious: associated lock is
  not held` from *inside* glibc's implementation of timed waits. Splitting the
  two cases removes the timed wait, keeps the condition variable for what it is
  good at (waiting for an event that will be signalled), and makes Helgrind
  report 0 errors.

### The `running` flag: a stop "event"

No custom event object is needed. The end of the simulation is an **event
implemented by a flag plus a broadcast**:

1. `stop_simulation` sets `sim->running = 0` under `running_lock`;
2. it then locks `table_lock` and calls `pthread_cond_broadcast` on `table_cond`;
3. coders blocked in `take_dongles` wake up, see `is_running() == 0`, leave their
   waiting loop and exit; coders in `smart_sleep` notice the flag within 500 µs.

### Lock ordering

Mutexes are never taken in conflicting orders, which would be a deadlock of its
own:

```
print_lock ──► running_lock          (print_status / print_burnout test the flag)
print_lock ──► table_lock            (print_burnout → stop_simulation → broadcast)
table_lock ──► running_lock          (take_dongles tests the flag in its loop)
table_lock ──► coder.data_lock       (EDF key / get_deadline)
```

The graph is acyclic and **no code holds the table mutex while asking for the
print mutex**: `take_dongles` unlocks the table *before* calling `print_dongles`.
There is only **one** mutex for the dongles, so two dongle mutexes can never be
taken in opposite orders by two coders. Critical sections stay short:
`data_lock` and `running_lock` cover one read or write, `table_lock` covers the
queue test (and is *released* while the thread sleeps in `pthread_cond_wait` or
in the cooldown polling sleep), `print_lock` covers one or two `printf`.

### Race conditions prevented, with examples

| Race | How it is prevented |
|---|---|
| Two coders take the same dongle | `is_taken` of **both** dongles is tested and set inside the same critical section on `table_lock`; the heap root rule lets only one waiter through. |
| A coder ends up holding a single dongle | Impossible: both flags are set in the same critical section, and both are cleared in the same critical section (`release_dongles`). |
| Another message falls between the two `has taken a dongle` lines | `print_dongles` prints both lines under one `print_lock` section, with a single timestamp. |
| The monitor reads `last_compile_and_start` while the coder writes it | Both go through `data_lock` (`get_last_compile` / `set_last_compile`). |
| An EDF key is computed on a half-updated value | The key is read through `get_deadline` (locks `data_lock`). A waiting coder cannot change it anyway: it only changes when that coder starts compiling, which requires the dongles it is waiting for. |
| Two log lines interleave | The whole line is a single `printf` under `print_lock`. |
| A log line appears **after** the burnout | `print_status` re-checks `running` *inside* `print_lock`, and `print_burnout` flips it inside the same lock. Exactly one burnout line, always the last. |
| Two "burned out" lines | Same mechanism: the second caller sees `running == 0` and prints nothing. |
| The heap is corrupted by two threads | Every heap is **only** touched with `table_lock` held. |
| A sleeping coder never learns the simulation ended | `stop_simulation` broadcasts `table_cond` under `table_lock`. |
| Lost wake-up | Condition tested and sleep entered atomically under the same mutex, and every event that ends a wait is followed by a broadcast (see above). |

### How coders and the monitor communicate (thread-safe)

Coders and the monitor never talk to each other directly; they only share
*protected state*:

* **coder → monitor**: a coder updates `last_compile_and_start` and `nb_compiles`
  (`data_lock`); the monitor reads them every millisecond.
* **monitor → coders**: the monitor calls `stop_simulation` (flag +
  broadcast); coders poll the flag through `is_running`.

This also respects the subject's rule that coders do not communicate and do not
know whether another coder is about to burn out.

---

## Scheduling: the priority queue (heap), FIFO and EDF

### One queue per dongle, one ticket per request

Every dongle owns a priority queue of the coders currently **waiting for it**.
When a coder wants to compile it joins the queues of **both** its dongles
(`enter_queues`), with **the same ticket** in both. It is allowed to take its
dongles only if all these conditions hold (`is_available` and `cooldown_over`
in `take_dongles.c`):

1. **both** dongles are **free**;
2. the coder is **at the top (root) of both queues**;
3. the **cooldown** since the last release of **both** dongles is over.

If a coder is not at the root of both queues it waits, *even if its dongles are
free*. Nobody can overtake a coder that has priority. While it waits, the coder
holds **nothing**.

Since a dongle is shared by only two coders, its queue never contains more than
two entries; the heap is created with capacity 2. This is an invariant: if
`heap_push` ever failed (it cannot), `take_dongles` stops the simulation cleanly
instead of corrupting memory.

### The binary heap

A **binary min-heap** is a complete binary tree stored in an array, where every
parent is "smaller" (higher priority) than its children:

```
index:    0  1  2  3  4          tree:       [0]
          A  B  C  D  E                    /   \
                                         [1]   [2]
parent(i)   = (i - 1) / 2                / \
left(i)     = 2i + 1                   [3] [4]
right(i)    = 2i + 2
```

The smallest element is always at index 0, so *who goes next* is read in O(1).

| Operation | How | Cost |
|---|---|---|
| `heap_push` | store at the end, `heap_sift_up` (swap with parent while smaller) | O(log n) |
| `heap_peek` | return `nodes[0].coder` | O(1) |
| `heap_remove` | find the coder, move the last element into its slot, `heap_sift_up` then `heap_sift_down` | O(n) to find + O(log n) |
| `heap_sift_down` | swap with the smaller child while a child is smaller | O(log n) |

`heap_remove` removes *any* coder, not only the root. It is needed for the
shutdown path: when the simulation ends, a waiting coder leaves the queue
without having been served.

Each node holds three values:

```c
typedef struct s_heap_node {
    long            key;    // main priority: smaller = served first
    long            seq;    // global arrival ticket (same in both heaps): tie-breaker
    struct s_coder  *coder;
} t_heap_node;
```

and the order is the lexicographic comparison on `(key, seq)`:

```c
static int heap_less(t_heap_node *a, t_heap_node *b)
{
    if (a->key != b->key)
        return (a->key < b->key);
    return (a->seq < b->seq);
}
```

The **same heap serves both policies**; only the `key` differs
(`get_key`, called by `enter_queues`, in `waiting_queue.c`). The scheduler
is a `t_scheduler` enum (`SCHEDULER_FIFO`, `SCHEDULER_EDF`) filled by
`get_scheduler`, so adding a policy only means a new enum value, one line in
`get_scheduler` and one line in `get_key`:

```c
seq      = sim->next_seq++;                    // global ticket, taken under table_lock
node.key = get_key(coder, seq);                // same node pushed in both heaps
// get_key: EDF  -> last_compile_start + time_to_burnout
//          FIFO -> seq
```

### FIFO: First In, First Out

`key = seq`: the key is the global arrival ticket, so the root is always the
**oldest request**. Fair by construction, but blind to urgency.

### EDF: Earliest Deadline First

`key = deadline = last_compile_start + time_to_burnout`: the root is the coder
who will **burn out first**. If two coders have exactly the same deadline,
`seq` breaks the tie, so the policy is **fully deterministic**, as the subject
requires.

The deadline is computed **once, when the coder enters the queue**. That is
valid because a waiting coder's `last_compile_start` cannot change while it
waits (it changes only when that coder starts a compile, which needs the
dongles it is waiting for).

### FIFO versus EDF on an example

`time_to_burnout = 800`, compile / debug / refactor = 100 ms. Coder **A** last
started compiling at `t = 0`, coder **B** at `t = 100`. Both want the same
dongle D, B asks first:

| | request time | `last_compile_start` | deadline | FIFO key | EDF key |
|---|---|---|---|---|---|
| B | 410 | 100 | 900 | 0 | 900 |
| A | 420 | 0 | **800** | 1 | **800** |

* **FIFO** serves **B** then A (B arrived first).
* **EDF** serves **A** then B (A will burn out 100 ms earlier).

### Why neither policy starves anybody

Both policies order the requests by the pair `(key, seq)`, and **the same pair is
stored in both heaps**. Because `seq` is a *global* ticket (unique), this is a
**strict total order on all the pending requests**, identical for every dongle.
Consequences:

* the **smallest pending request** is at the root of **both** of its heaps, so it
  is never blocked by the queues: it only waits for its two dongles to be free,
  which happens when the (non-waiting) coders that hold them finish their
  compile;
* a request can only be overtaken by requests that are **smaller** than it. A
  coder that has just been served re-queues with a **larger** ticket (FIFO) and a
  **later** deadline (EDF), so it can never overtake the same neighbour twice
  in a row (**bounded overtaking**). Nobody waits forever.

---

## Blocking cases handled

### 1. Deadlock and Coffman's conditions

A deadlock can only happen if **all four Coffman conditions** hold at the same
time. Breaking any single one makes deadlock impossible.

| # | Condition | Present in Codexion? | Where |
|---|---|---|---|
| 1 | **Mutual exclusion**: a resource is held by at most one process | **Yes**, inherent: a dongle is held by one coder at a time | `is_taken` under `table_lock` |
| 2 | **Hold and wait**: a process holds a resource while waiting for another | **No, broken on purpose** | **all-or-nothing** acquisition in `take_dongles` |
| 3 | **No preemption**: a resource cannot be taken away | **Yes**, a dongle is only released voluntarily after the compile | `release_dongles` |
| 4 | **Circular wait**: a closed chain of processes, each waiting for a resource held by the next | **No**, a consequence of breaking condition 2 | nobody who waits holds a dongle |

#### The naive solution deadlocks

If every coder took `left` then `right` (one dongle at a time), then with 3
coders at the same instant:

```
coder 1 holds d0, wants d1 (held by coder 2)
coder 2 holds d1, wants d2 (held by coder 3)
coder 3 holds d2, wants d0 (held by coder 1)      ← circular wait, nobody ever moves
```

#### Why *hold and wait* is the condition to break here

An earlier version of this project broke the **circular wait** instead, with a
global order on the dongles (every coder took the lower-numbered dongle first).
That removes deadlock but keeps *hold and wait*: a coder holds a first dongle
while it waits for the second. This has two costs:

* a dongle held by a coder that is still waiting for its second dongle is
  **idle**, and its neighbour cannot use it, which creates *waiting chains*
  (coder 1 waits for coder 2, who waits for coder 3, ...);
* the chains make a coder wait for several compiles in a row, which ends in a
  **starvation** and a burnout even when the parameters leave room for a valid
  schedule.

The fix is to break **hold and wait** itself.

#### The solution: all-or-nothing acquisition

A coder either obtains **both** dongles **in one atomic step**, or it obtains
**none and holds nothing while it waits**:

```c
pthread_mutex_lock(&sim->table_lock);
enter_queues(coder);                      // register on both dongles (same ticket)
while (!(both_free && head_of_both_queues && cooldown_over))
    wait;                                 // holding NO dongle, table_lock released
leave_queues(coder);
left->is_taken = 1;  right->is_taken = 1; // both, same critical section
pthread_mutex_unlock(&sim->table_lock);
```

Two details make this correct:

1. **Atomicity.** The state of both dongles is read and written under the single
   `table_lock`, so no other coder can slip between "I check the dongles" and "I
   take them".
2. **A consistent priority.** If each dongle had its own independent queue order,
   the *head of both queues* rule could deadlock on the queues themselves:
   coder 1 is the head of `d1` but not of `d2`, coder 2 the head of `d2` but not
   of `d3`, ..., a cycle of coders each blocked by the next one's priority. This
   is why the ticket (`seq`) is **global**: the same total order is used by all
   the dongles, and a cycle of "strictly smaller than" is impossible.

**Proof of no deadlock.** A coder that waits holds no dongle, so the *hold and
wait* link of any waiting chain is missing: a waiting coder is never the reason
why another coder cannot proceed through a dongle it holds. Let `R` be the
smallest pending request (in the total order above). `R` is at the root of both
its heaps. The only things that can block it are its two dongles being
taken, or the cooldown. Dongles are only held by coders that **already own both
of theirs and are compiling**, and they release after `time_to_compile`. So `R` is
served in bounded time.

**Proof of no starvation.** Each served request leaves the queues and is replaced
by a larger one, so the number of requests smaller than any waiting request only
decreases, and each of them is served in bounded time (the same argument applies
to each in turn).

#### The start-up stagger is still needed

The strict priority has a price: a coder cannot take two free dongles if a
**higher-priority neighbour is ahead of it in a queue**, even if that neighbour
is itself blocked. At `t = 0` every coder enqueues at the same time in an
arbitrary order. With 4 coders arriving in the order 1, 2, 3, 4, coder 2 sits in
front of coder 3 on `d2`, and is blocked by coder 1, so coder 3 waits behind
it, and so on: the compiles are serialized (`4 × 210 ms = 840 ms`) although two
could run in parallel. The `stagger_start` delay (even ids wait
`time_to_compile / 2`) makes the odd coders enqueue first, so that they compile
together, then the even ones. Measurements are in
[Testing](#testing-and-validation).

Letting a lower-priority coder bypass a blocked neighbour would remove the need
for the stagger, but it reopens the door to starvation (two neighbours can keep
alternating on the dongles of a coder that never gets both). The strict rule was
kept on purpose.

#### One coder

With `N = 1` there is a single dongle, so *both* of the coder's dongles are the
same one and a pair can never exist. `take_dongles` detects it (`left == right`)
and calls `lone_coder`: it logs one `has taken a dongle` (the only dongle there
is), then sleeps on the condition variable. It burns out after
`time_to_burnout`, the monitor reports it, and the sleeping coder is woken by
`stop_simulation`. No hang.

### 2. Starvation prevention

* **No hold and wait.** A waiting coder holds no dongle, so there is no *waiting
  chain*: a coder is never delayed by a neighbour that is itself waiting while
  sitting on a dongle. This was the source of the starvation of the previous
  version.
* The requests are served in strict policy order (FIFO or EDF) with a **global,
  unique ticket** as the last tie-breaker, and **a lower-priority coder never
  overtakes a higher-priority one**: only a coder that is at the root of both its
  heaps can take its dongles.
* Overtaking is **bounded** (see the previous section): a request can only be
  overtaken by smaller ones, and a coder that has just been served comes back
  with a larger ticket / a later deadline.
* **Start-up convoy and the stagger.** At `t = 0` all the coders enqueue at once
  and, as explained above, the strict queue order can serialize the first
  compiles. To prevent this, **coders with an even `id` wait
  `time_to_compile / 2` ms before their first attempt** (`stagger_start`): the
  odd coders take their two dongles without contention and compile together,
  then the even coders take over. The delay costs nothing because an even coder's
  neighbours are odd coders who hold its dongles for at least `time_to_compile`
  anyway. See [Testing](#testing-and-validation) for measurements.

### 3. Dongle cooldown

`release_dongles` stores `released_at = now` on **both** dongles under
`table_lock`. `cooldown_over` keeps a coder waiting while
`now - released_at < dongle_cooldown` for **either** of its dongles. Nobody
signals the end of a cooldown, so the coder that is next in line polls: it
unlocks the table, sleeps 500 µs and re-tests. The cooldown is respected and ends
within about half a millisecond. Other waiters do not poll: they sleep on the
condition variable.

### 4. Precise burnout detection

* A **dedicated monitor thread** loops every `1 ms` (`usleep(1000)`).
* For each coder that has not yet reached the required number of compiles, it
  computes `now - last_compile_start` (read under `data_lock`) and declares a
  burnout when it is **strictly greater** than `time_to_burnout`.
* The monitor takes **no table mutex**, so it keeps working even when every
  coder is blocked waiting for its dongles.
* `print_burnout` prints the line and stops the simulation in the same
  `print_lock` section. The reported delay is at most about 1 ms plus scheduling
  noise, well within the **10 ms** required.
* Coders that already finished their compiles are ignored, so a coder that is
  done cannot cause a false burnout while others are still running.

### 5. Serialized logging

Every message goes through `print_status`, `print_dongles` or `print_burnout`,
which take `print_lock` for the whole `printf` of a line. Lines never mix.
`print_dongles` prints the **two** `has taken a dongle` lines of a coder inside a
single `print_lock` section with one timestamp, so **no other message can be
inserted between them**. Inside the lock the `running` flag is tested again, so
**nothing is ever printed after a burnout or after the end of the simulation**.

### 6. Clean termination

* The end is decided in one place (`stop_simulation`) and broadcast on
  `table_cond`, so no thread stays blocked.
* `launch_coders` joins all coder threads and the monitor.
* `cleanup` destroys every mutex and condition variable, frees every heap
  (`heap_destroy`), the dongles and the coders. Valgrind reports no leak, and
  the initialization functions free what they allocated if a later allocation
  fails.

### 7. Others

* **Spurious wake-ups** → condition re-tested in a `while` loop.
* **Time overflow** → `get_deadline` saturates at `LONG_MAX` instead of
  overflowing.
* **Invalid input** → rejected before any thread or allocation exists.
* **Heap full** (impossible by construction) → the simulation stops cleanly.
* **One coder** → handled separately (`lone_coder`), see above.

---

## Known limits

Being explicit about what the design does **not** promise:

* **No deadlock and no indefinite waiting are guaranteed** (all-or-nothing +
  global total order, see the Coffman section), but *hard real-time optimality* is
  not. A coder cannot take two free dongles if a higher-priority neighbour is
  ahead of it in a queue, even if that neighbour is itself blocked. That rule is
  what prevents starvation, and its cost is some lost parallelism.
* **The start-up stagger is required.** Without `stagger_start`, the first
  arrivals are served in an arbitrary order and the first compiles can be
  serialized (for 4 to 10 coders, 10 to 12 runs out of 12 burned out on feasible
  parameters, see [Testing](#testing-and-validation)). The stagger is a heuristic
  that fits the ring parity; it is not a proof for every possible parameter set.
* **Infeasible parameters still burn out**, of course. `./codexion 4 410 200 200 200 10 50 fifo`
  needs two rounds of `200 + 50` ms, i.e. 500 ms between two compiles of the same
  coder, and only 410 ms are allowed: the burnout is the correct answer, not a
  failure of the algorithm.
* **A large cooldown still leaves little room.** `./codexion 5 3000 200 200 200 10 800 edf`
  passed 4 runs out of 4 and the same parameters with `fifo` passed 3 runs out of 4
  (the previous version burned out in 6 of 6 runs). These runs were launched in
  parallel on a 2-core container, so a part of the failure can be scheduling noise.
* **`pthread_cond_broadcast` wakes every waiting coder** at each release, so up to
  `N` threads wake up and re-test. It is correct and cheap for the sizes
  tested (up to 200 coders) but it is not the most scalable choice. A per-coder
  condition variable, woken only for its two neighbours, would avoid it at the
  cost of more code.
* Timings depend on the OS scheduler: with a heavily loaded machine, a coder can
  be late by a few milliseconds.

---

## Testing and validation

Everything below was run on the final code (the heap files are unchanged: the
last row was measured on the previous revision of the same files).

| Check | Result |
|---|---|
| `norminette` on all files | OK |
| Build with `-Wall -Wextra -Werror -pthread` | no warning |
| Valgrind (`--leak-check=full`) on 4 scenarios, including 1 coder and burnouts | 0 errors, 0 leaks |
| ThreadSanitizer on 6 scenarios | no warning |
| AddressSanitizer + UBSan on 4 scenarios | no report |
| Helgrind on 7 configurations, including 1 coder, 30 coders and a cooldown | 0 errors |
| Automatic log checker on 13 parameter sets (2 to 200 coders, `fifo` and `edf`) | **0 violation**: every pair of `has taken a dongle` lines is consecutive, same coder, same timestamp; two neighbours never compile at the same time |
| Same checker on the previous version (one dongle at a time) | interleaved dongle lines in 9 of the 13 sets |
| Heap alone: 200 000 random `push` / `peek` / `remove` operations compared with a naive reference | no difference, ties broken by `seq`, overflow refused |

### All-or-nothing versus one dongle at a time

Number of runs **without burnout** on feasible parameters (same machine, same
stagger, same heap). Parameters: `number_of_coders time_to_burnout time_to_compile time_to_debug time_to_refactor number_of_compiles_required dongle_cooldown`.

| Scenario | previous version | all-or-nothing |
|---|---|---|
| `3 800 200 100 100 2 10` `fifo` | 4 / 15 | **15 / 15** |
| `3 800 200 100 100 2 10` `edf` | 6 / 15 | **15 / 15** |
| `3 650 200 100 100 4 10` `fifo` | 0 / 6 | **6 / 6** |
| `3 650 200 100 100 4 10` `edf` | 0 / 6 | **6 / 6** |
| `7 620 200 100 100 8 0` `fifo` | 2 / 8 | **8 / 8** |
| `9 700 200 100 100 8 0` `edf` | 3 / 8 | **8 / 8** |
| `5 620 200 200 200 8 0` `edf` | 7 / 8 | **8 / 8** |

On 10 other feasible sets (6 to 40 coders, 6 runs each) both versions had no
burnout; no case was found where the new version does worse.
(Absolute figures depend on the machine.)

### Effect of the start-up stagger

Number of runs with a burnout out of 12, on feasible parameters, **without** and
**with** the stagger, with the all-or-nothing code:

| Scenario | without | with |
|---|---|---|
| 2 coders, edf | 0/12 | 0/12 |
| 3 coders, edf | 0/12 | 0/12 |
| 4 coders, fifo | 11/12 | **0/12** |
| 4 coders, edf | 11/12 | **0/12** |
| 5 coders, fifo | 10/12 | **0/12** |
| 5 coders, edf | 11/12 | **0/12** |
| 8 coders, edf | 12/12 | **0/12** |
| 10 coders, edf | 12/12 | **0/12** |

(Parameters: `800 200 200 200 3 10`, except 10 coders: `1000 200 100 100 3 10`.)
The stagger matters *more* than before, because the strict queue order cannot
bypass a blocked neighbour (see the Coffman section).

---

## Resources

### Documentation and references

* POSIX threads overview: [`man 7 pthreads`](https://man7.org/linux/man-pages/man7/pthreads.7.html), and the man pages
  `pthread_create(3)`, `pthread_join(3)`, `pthread_mutex_lock(3)`,
  `pthread_cond_wait(3)`, `gettimeofday(2)`, `usleep(3)`.
* The Open Group Base Specifications (POSIX.1): <https://pubs.opengroup.org/onlinepubs/9699919799/>
* R. Arpaci-Dusseau, A. Arpaci-Dusseau, *Operating Systems: Three Easy Pieces*,
  the "Concurrency" chapters (locks, condition variables, deadlock):
  <https://pages.cs.wisc.edu/~remzi/OSTEP/>
* E. G. Coffman, M. Elphick, A. Shoshani, *System Deadlocks*, ACM Computing
  Surveys, 1971 (the four conditions).
* E. W. Dijkstra, *Hierarchical ordering of sequential processes*, 1971 (resource
  ordering and the dining philosophers).
* C. L. Liu, J. W. Layland, *Scheduling Algorithms for Multiprogramming in a
  Hard-Real-Time Environment*, 1973 (Earliest Deadline First).
* T. Cormen et al., *Introduction to Algorithms*, chapter on heaps and priority
  queues.
* Articles: [Deadlock](https://en.wikipedia.org/wiki/Deadlock),
  [Dining philosophers problem](https://en.wikipedia.org/wiki/Dining_philosophers_problem),
  [Earliest deadline first scheduling](https://en.wikipedia.org/wiki/Earliest_deadline_first_scheduling),
  [Binary heap](https://en.wikipedia.org/wiki/Binary_heap).
* Tools: Valgrind and ThreadSanitizer documentation.

### Use of AI

An AI assistant (Claude) was used as a help, never as a replacement for
understanding:

| Task | What the AI did | What was done with it |
|---|---|---|
| Design | Discussed the structure (a monitor thread, FIFO/EDF arbitration with one heap per dongle, then the switch from ordered acquisition to all-or-nothing acquisition under a single table mutex). | Choices are explained in this README so they can be defended. |
| Code review | Reviewed and corrected an earlier version of the code. | Changes were read, compiled and tested. |
| README | Drafted this document from the actual code. | Every statement was checked against the source. |

All AI-generated content was reviewed, tested and is understood well enough to be
explained and modified during the evaluation.
