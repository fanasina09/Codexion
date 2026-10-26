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
* each dongle has its own **mutex**, its own **condition variable** and its own
  **priority queue implemented as a binary heap** (no standard-library queue);
* strict **FIFO / EDF arbitration**, **dongle cooldown**, **deadlock-free**
  acquisition order, **serialized logs**, **burnout reported within a few
  milliseconds**, **no global variable**, **no memory leak**.

### Source files

| File | Role |
|---|---|
| `main.c` | Parses, initializes the simulation, starts the monitor and the coder threads, joins them, cleans up. |
| `parsing.c` | Argument validation (positive integers, `fifo`/`edf`) and filling of `t_sim`. |
| `routine_coder.c` | Life cycle of a coder thread: stagger, take dongles, compile, debug, refactor. |
| `take_dongle.c` | Acquisition of one dongle: enqueue, wait on the condition variable, take. |
| `release_dongle.c` | Releases a dongle, stamps the cooldown start, wakes the waiters. |
| `waiting_queue.c` | Glue between a dongle and its heap: computes the FIFO/EDF key, enqueue, dequeue, peek. |
| `heap.h`, `heap.c`, `heap_ops.c` | Generic binary min-heap (the priority queue). |
| `monitor.c` | Monitor thread: burnout detection and "everybody is done" detection. |
| `simulation_state.c` | Shared `running` flag, `stop_simulation`, interruptible `smart_sleep`. |
| `coder_state.c` | Mutex-protected accessors of a coder's compile counter and last compile start. |
| `print.c` | Serialized logging. |
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
message on `stderr` and exit with status `1`.

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

### Examples

Everybody finishes (3 coders, one compile at a time is possible, plenty of slack):

```console
$ ./codexion 3 1000 200 100 100 2 10 fifo
```

A burnout, here because the parameters are impossible: with 3 coders only one
can compile at a time (a compile uses 2 of the 3 dongles), so the third coder
would have to wait for two compiles in a row (400 ms) with only 300 ms allowed:

```console
$ ./codexion 3 300 200 100 100 5 0 edf
1 1 has taken a dongle
1 1 has taken a dongle
1 1 is compiling
201 1 is debugging
201 3 has taken a dongle
201 3 has taken a dongle
201 3 is compiling
201 2 has taken a dongle
301 1 is refactoring
301 2 burned out
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
    int              is_taken;       // 0 free, 1 held
    pthread_mutex_t  is_lock;        // guards every field of this struct
    pthread_cond_t   is_cond;        // waiters sleep here (with is_lock)
    t_heap           waiting;        // priority queue of the requesters
    long             next_seq;       // arrival counter for this dongle
    long             released_at;    // ms timestamp of the last release
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
    take_both();                      // ascending dongle order (see Coffman)
    set_last_compile();               // start of the compile = new deadline origin
    print "is compiling";   smart_sleep(time_to_compile);
    release_dongle(left); release_dongle(right);
    add_compile();
    print "is debugging";   smart_sleep(time_to_debug);
    print "is refactoring"; smart_sleep(time_to_refactor);
}
```

`smart_sleep` sleeps by slices of 500 µs and re-checks the `running` flag, so
no thread ever sleeps through the end of the simulation.

### Time

All times come from `gettimeofday` (milliseconds), as recommended by the
subject. `pthread_cond_timedwait` uses the same clock (`CLOCK_REALTIME`), so an
absolute deadline built from `gettimeofday` is consistent.

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
| `pthread_mutex_init / destroy` | Creates and destroys every mutex (dongles, coders, running flag, printing). |
| `pthread_mutex_lock / unlock` | Mutual exclusion around every shared datum. |
| `pthread_cond_init / destroy` | One condition variable per dongle. |
| `pthread_cond_timedwait` | A coder sleeps until its dongle may be available, with a 1 ms timeout. |
| `pthread_cond_broadcast` | Wakes the waiters of a dongle after a release or at the end of the simulation. |
| `gettimeofday`, `usleep` | Timestamps and interruptible sleeping. |

### Mutex: mutual exclusion

A mutex (`pthread_mutex_t`) lets one thread at a time run a **critical
section**. The rule applied everywhere in the code: *a shared variable is only
read or written while holding the mutex that owns it, and the section is kept as
short as possible.*

| Mutex | Count | Protects | Taken by |
|---|---|---|---|
| `dongle.is_lock` | 1 per dongle | `is_taken`, `released_at`, `next_seq` and the **waiting heap** | `take_dongle`, `release_dongle`, `stop_simulation` (to wake waiters) |
| `coder.data_lock` | 1 per coder | `last_compile_and_start`, `nb_compiles` | the coder (writes), the monitor (reads), neighbours computing an EDF key (reads) |
| `sim.running_lock` | 1 | the `running` flag | every thread, through `is_running` / `stop_simulation` |
| `sim.print_lock` | 1 | `stdout` and the "burned out" decision | `print_status`, `print_burnout` |

Each coder's state is read through small accessors (`get_last_compile`,
`get_compiles`, ...) so that **no code touches a protected field without
locking**.

### Condition variable: waiting without burning CPU

A condition variable (`pthread_cond_t`) lets a thread **sleep until a condition
may have become true**, instead of spinning in a loop. It is always used
*together with a mutex*:

```c
pthread_mutex_lock(&dongle->is_lock);
add_to_waiting(dongle, coder);
while (is_running(sim) && must_wait(dongle, coder))   // the CONDITION, re-tested
    wait_dongle(dongle);                              // pthread_cond_timedwait
remove_from_waiting(dongle, coder);
...
```

Why this exact shape:

* **`while`, not `if`.** `pthread_cond_wait` may return spuriously, and several
  waiters may be woken at once but only one can take the dongle. The condition
  (`must_wait`: dongle taken? am I the heap root? is the cooldown over?) is
  re-evaluated after every wake-up.
* **The test and the sleep are atomic.** `pthread_cond_timedwait` releases the
  mutex and puts the thread to sleep in one atomic step, and re-acquires the mutex
  before returning. The condition is tested while holding the mutex, and the
  state it depends on (`is_taken`, the heap) is only changed while holding the
  same mutex. A release between "I tested" and "I sleep" is therefore
  impossible: **no lost wake-up**.
* **`broadcast`, not `signal`.** Waiters of one dongle have different
  priorities and only the *heap root* is allowed to proceed. `signal` could
  wake a non-root waiter that goes straight back to sleep while the root keeps
  sleeping. With at most two waiters per dongle, `broadcast` costs nothing.
* **A 1 ms timeout.** Nobody signals when a **cooldown expires** (it is just
  time passing), and a timeout is also a safety net. `wait_dongle` therefore
  waits at most 1 ms before re-testing, which also keeps the cooldown accurate
  to the millisecond.

### The `running` flag: a stop "event"

No custom event object is needed. The end of the simulation is an **event
implemented by a flag plus a broadcast**:

1. `stop_simulation` sets `sim->running = 0` under `running_lock`;
2. it then locks each dongle and calls `pthread_cond_broadcast` on its condition
   variable;
3. coders blocked in `take_dongle` wake up, see `is_running() == 0`, leave their
   waiting loop and exit; coders in `smart_sleep` notice the flag within 500 µs.

### Lock ordering

Mutexes are never taken in conflicting orders, which would be a deadlock of its
own:

```
print_lock ──► running_lock          (print_status / print_burnout test the flag)
print_lock ──► dongle.is_lock        (print_burnout → stop_simulation → broadcast)
dongle.is_lock ──► running_lock      (take_dongle tests the flag in its loop)
dongle.is_lock ──► coder.data_lock   (EDF key / get_deadline)
```

The graph is acyclic and **no code holds a dongle mutex while asking for the
print mutex**: `take_dongle` unlocks the dongle *before* calling `print_status`.
Critical sections stay short: `data_lock` and `running_lock` cover one read or
write, `is_lock` covers the queue test (and is *released* while the thread sleeps in
`pthread_cond_timedwait`), `print_lock` covers one `printf`.

### Race conditions prevented, with examples

| Race | How it is prevented |
|---|---|
| Two coders take the same dongle | `is_taken` is tested and set inside the same critical section on `is_lock`; the heap root rule lets only one waiter through. |
| The monitor reads `last_compile_and_start` while the coder writes it | Both go through `data_lock` (`get_last_compile` / `set_last_compile`). |
| An EDF key is computed on a half-updated value | The key is read through `get_deadline` (locks `data_lock`). A waiting coder cannot change it anyway: it only changes when that coder starts compiling, which requires the dongles it is waiting for. |
| Two log lines interleave | The whole line is a single `printf` under `print_lock`. |
| A log line appears **after** the burnout | `print_status` re-checks `running` *inside* `print_lock`, and `print_burnout` flips it inside the same lock. Exactly one burnout line, always the last. |
| Two "burned out" lines | Same mechanism: the second caller sees `running == 0` and prints nothing. |
| The heap is corrupted by two threads | The heap lives inside the dongle and is **only** touched with `is_lock` held. |
| A sleeping coder never learns the simulation ended | `stop_simulation` broadcasts every dongle condition variable. |
| Lost wake-up | Condition tested and sleep entered atomically under the same mutex (see above), plus the 1 ms timeout. |

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

### One queue per dongle

Every dongle owns a priority queue of the coders currently **waiting for it**.
A coder is allowed to take the dongle only if all three conditions hold
(`must_wait` in `take_dongle.c`):

1. the dongle is **free**;
2. the coder is **at the top (root) of the queue**;
3. the **cooldown** since the last release is over.

If a coder is not at the root it waits, *even if the dongle is free*. Nobody can
overtake a coder that has priority.

Since a dongle is shared by only two coders, its queue never contains more than
two entries; the heap is created with capacity 2. This is an invariant: if
`heap_push` ever failed (it cannot), `take_dongle` stops the simulation cleanly
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
    long            seq;    // arrival order at THIS dongle: tie-breaker
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
(`add_to_waiting` in `waiting_queue.c`):

```c
node.seq = dongle->next_seq++;                 // arrival number at this dongle
if (scheduler == EDF)  node.key = last_compile_start + time_to_burnout;
else                   node.key = node.seq;    // FIFO
```

### FIFO: First In, First Out

`key = seq`: the key is the arrival number at this dongle, so the root is
always the **oldest request**. Fair by construction, but blind to urgency.

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

Only two coders ever compete for a given dongle. After a coder is served, its
new request carries a larger `seq` (FIFO) and a larger deadline (EDF: it has
just started a compile *after* the other coder's last one). So between the
moment a coder queues and the moment it is served, the neighbour can be served
**at most once**. Nobody waits forever (**bounded overtaking**).

---

## Blocking cases handled

### 1. Deadlock and Coffman's conditions

A deadlock can only happen if **all four Coffman conditions** hold at the same
time. Breaking any single one makes deadlock impossible.

| # | Condition | Present in Codexion? | Where |
|---|---|---|---|
| 1 | **Mutual exclusion**: a resource is held by at most one process | **Yes**, inherent: a dongle is held by one coder at a time | `is_taken` under `is_lock` |
| 2 | **Hold and wait**: a process holds a resource while waiting for another | **Yes**, accepted: a coder keeps its first dongle while waiting for the second | `take_both` |
| 3 | **No preemption**: a resource cannot be taken away | **Yes**, a dongle is only released voluntarily after the compile | `release_dongle` |
| 4 | **Circular wait**: a closed chain of processes, each waiting for a resource held by the next | **No, broken on purpose** | **ordered acquisition** in `take_both` |

#### The naive solution deadlocks

If every coder took `left` then `right`, then with 3 coders at the same instant:

```
coder 1 holds d0, wants d1 (held by coder 2)
coder 2 holds d1, wants d2 (held by coder 3)
coder 3 holds d2, wants d0 (held by coder 1)      ← circular wait, nobody ever moves
```

#### The solution: a global order on the dongles (resource hierarchy)

The dongles are numbered `d0 … d(N-1)` and **every coder always takes the
lower-numbered dongle first**:

| Coder (N = 4) | Dongles | Takes first | Then |
|---|---|---|---|
| 1 | d0, d1 | d0 | d1 |
| 2 | d1, d2 | d1 | d2 |
| 3 | d2, d3 | d2 | d3 |
| **4 (= N)** | d3, d0 | **d0** (its *right*) | d3 (its *left*) |

Only the last coder is "reversed" (`take_both` in `routine_coder.c`). For the
others, left = lower index; for coder N, right = d0 is the lowest.

**Proof that no cycle can exist.** Suppose coder X holds dongle `a` and waits
for dongle `b`. Because coders take dongles in ascending order, `a < b`. The
holder of `b`, if it is itself waiting, waits for some `c > b`, and so on:
along any chain of waiting coders **the dongle indices strictly increase**. A
strictly increasing sequence can never come back to its starting point, so a
cycle of waiting coders is impossible, and **so is deadlock**.

**Progress.** Look at the highest-numbered dongle `h` currently held by somebody.
Either its holder owns **both** dongles of its pair, so it compiles and then
releases; or `h` is the *lower* dongle of its pair and the higher one is, by the
choice of `h`, **free**, so the head of that dongle's queue takes it as soon as
the cooldown is over. In both cases some coder advances in bounded time.

**The queues do not break this.** A coder blocked because it is *not the heap
root* is waiting behind the root, and the root is only blocked by the holder of
the dongle or by the cooldown, both of which end.

#### One coder

With `N = 1` there is a single dongle: the coder takes it, then waits for a
second dongle that does not exist. This is expected: it burns out after
`time_to_burnout`, the monitor reports it, and the waiting coder is woken by
`stop_simulation`. No hang.

### 2. Starvation prevention

* Each dongle serves its waiters in strict policy order (FIFO or EDF) and **never
  lets a lower-priority coder overtake**: only the heap root may take the dongle.
* Overtaking is **bounded**: a neighbour can be served at most once between a
  coder's request and its service (see the previous section).
* **Start-up convoy and the stagger.** At `t = 0` all coders compete. Without
  precaution, with 4 coders, coder 1 may win `d0` and keep it while waiting for
  `d1`, held by coder 2, who waits for `d2`, held by coder 3. Coder 4 is
  stuck behind the whole chain: four compiles in a row (`4 × (200 + 10) = 840 ms`)
  instead of two at a time, and it burns out at `800 ms` although the parameters
  allow two coders to compile in parallel. To prevent this, **coders with an even
  `id` wait `time_to_compile / 2` ms before their first attempt**
  (`stagger_start`): the odd coders take their two dongles without contention
  and compile together, then the even coders take over. The delay costs nothing
  because an even coder's neighbours are odd coders who hold its dongles for at
  least `time_to_compile` anyway. See [Testing](#testing-and-validation) for
  measurements.

### 3. Dongle cooldown

`release_dongle` stores `released_at = now` under `is_lock`. `must_wait` keeps a
coder waiting while `now - released_at < dongle_cooldown`. Nobody signals the end
of a cooldown, so waiters use `pthread_cond_timedwait` with a 1 ms timeout and
re-test: the cooldown is respected and ends within about a millisecond.

### 4. Precise burnout detection

* A **dedicated monitor thread** loops every `1 ms` (`usleep(1000)`).
* For each coder that has not yet reached the required number of compiles, it
  computes `now - last_compile_start` (read under `data_lock`) and declares a
  burnout when it is **strictly greater** than `time_to_burnout`.
* The monitor takes **no dongle mutex**, so it keeps working even when every
  coder is blocked waiting for a dongle.
* `print_burnout` prints the line and stops the simulation in the same
  `print_lock` section. The reported delay is at most about 1 ms plus scheduling
  noise, well within the **10 ms** required.
* Coders that already finished their compiles are ignored, so a coder that is
  done cannot cause a false burnout while others are still running.

### 5. Serialized logging

Every message goes through `print_status` / `print_burnout`, which take
`print_lock` for the whole `printf` of a single line. Lines never mix. Inside the
lock the `running` flag is tested again, so **nothing is ever printed after a
burnout or after the end of the simulation**.

### 6. Clean termination

* The end is decided in one place (`stop_simulation`) and broadcast to every
  dongle, so no thread stays blocked.
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

---

## Known limits

Being explicit about what the design does **not** promise:

* **No deadlock and no indefinite waiting are guaranteed**, but *hard real-time
  optimality* is not. `hold and wait` remains (Coffman condition 2): a coder keeps
  its first dongle while waiting for the second, which can delay a neighbour even
  if that neighbour could have used the free dongle. When the parameters leave
  **very little slack**, a coder can still burn out although a perfect
  centralized schedule would have worked. Example, 3 coders:
  `./codexion 3 800 200 100 100 2 10 fifo`. A perfect order needs
  `3 × (200 + 10) = 630 ms` per round, and 800 ms leaves only 170 ms of slack, but
  in a typical failing run coder 3 grabs `d0` early and holds it while it waits
  for `d2`, which delays coder 1 past its deadline. In our runs it burned out in 10 of 15 runs with `fifo` and
  12 of 15 with `edf` (the original version of the code did the same). With a little more margin
  (`./codexion 3 1000 200 100 100 4 10 edf`) it passed 10 runs out of 10.
  Removing the limit would require acquiring both dongles atomically, a
  different and larger design.
* Timings depend on the OS scheduler: with a heavily loaded machine, a coder can
  be late by a few milliseconds.

---

## Testing and validation

Everything below was run on the final code.

| Check | Result |
|---|---|
| `norminette` on all files | OK |
| Build with `-Wall -Wextra -Werror -pthread` | no warning |
| Valgrind (`--leak-check=full`) on 4 scenarios, including 1 coder and burnouts | 0 errors, 0 leaks |
| ThreadSanitizer on 5 scenarios | no warning |
| AddressSanitizer + UBSan | no report |
| Heap alone: 200 000 random `push` / `peek` / `remove` operations compared with a naive reference | no difference, ties broken by `seq`, overflow refused |

Helgrind prints a "dubious: associated lock is not held" warning raised from
inside glibc's `pthread_cond_timedwait` (where the caller does hold the mutex).
It appears on the original version too; ThreadSanitizer, which is more reliable
with modern glibc, reports nothing.

### Effect of the start-up stagger

Number of runs with a burnout out of 12, on feasible parameters, **without** and
**with** the stagger (same heap code):

| Scenario | without | with |
|---|---|---|
| 2 coders, edf | 0/12 | 0/12 |
| 3 coders, edf | 0/12 | 0/12 |
| 4 coders, fifo | 3/12 | **0/12** |
| 4 coders, edf | 2/12 | **0/12** |
| 5 coders, fifo | 2/12 | **0/12** |
| 5 coders, edf | 1/12 | **0/12** |
| 8 coders, edf | 2/12 | **0/12** |
| 10 coders, edf | 1/12 | **0/12** |

(Parameters: `800 200 200 200 3 10`, except 10 coders: `1000 200 100 100 3 10`.
Absolute figures depend on the machine.)

---

## Resources

### Documentation and references

* POSIX threads overview: [`man 7 pthreads`](https://man7.org/linux/man-pages/man7/pthreads.7.html), and the man pages
  `pthread_create(3)`, `pthread_join(3)`, `pthread_mutex_lock(3)`,
  `pthread_cond_wait(3)`, `pthread_cond_timedwait(3)`, `gettimeofday(2)`.
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
| Design | Discussed the structure (one mutex + condition variable per dongle, a monitor thread, ordered acquisition, FIFO/EDF arbitration). | Choices are explained in this README so they can be defended. |
| Code review | Reviewed and corrected an earlier version of the code. | Changes were read, compiled and tested. |
| README | Drafted this document from the actual code. | Every statement was checked against the source. |

All AI-generated content was reviewed, tested and is understood well enough to be
explained and modified during the evaluation.
