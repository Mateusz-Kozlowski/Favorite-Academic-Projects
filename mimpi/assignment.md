# MIMPI

[MPI](https://en.wikipedia.org/wiki/Message_Passing_Interface) 
is a standard communication protocol used for exchanging data
between processes in parallel programs, primarily used in supercomputer computations.
The goal of this task, as suggested by the name _MIMPI_ - which stands for _My Implementation of MPI_ - is
to implement a small, slightly modified fragment of MPI.
According to the following specification, you need to implement both:

- The `mimpirun` program code (in `mimpirun.c`) that launches parallel computations,
- The implementation in `mimpi.c` of procedures declared in `mimpi.h`.

## The `mimpirun` Program

The `mimpirun` program accepts the following command line arguments:

1) $n$ - number of copies to launch (you can assume this will be a natural number between 1 and 16 inclusive)
2) $prog$ - path to an executable file (may be in PATH).
   If the corresponding `exec` call fails (e.g., due to an incorrect path),
   `mimpirun` should terminate with a non-zero exit code.
3) $args$ - optionally, any number of arguments to pass to all launched $prog$ programs

The `mimpirun` program performs the following steps sequentially (each next step begins only after the previous one fully completes):

1) Prepares the environment (it's up to the implementer to decide what this entails).
2) Launches $n$ copies of the $prog$ program, each in a separate process.
3) Waits for all created processes to terminate.
4) Exits.

## Assumptions About $prog$ Programs

- $prog$ programs may **once** during their execution enter an _MPI block_.
To do this, they call the library function `MIMPI_Init` at the beginning,
and call `MIMPI_Finalize` at the end. The _MPI block_ refers to
the entire code segment between these calls.
- While in the _MPI block_, programs may execute various procedures from the `mimpi` library
to communicate with other $prog$ processes.
- They may perform any operations (write, read, open, close, etc.) on files
  whose descriptor numbers are in the ranges $[0,19]$ and $[1024, \infty)$
  (including `STDIN`, `STDOUT`, and `STDERR`).
- They won't modify environment variables starting with the `MIMPI` prefix.
- They expect properly set arguments,
  i.e., the zeroth argument should by Unix convention be the program name $prog$,
  while subsequent arguments should correspond to the $args$ arguments.
  _Hint:_ to pass additional data from `mimpirun` to $prog$, you can use the `*env` family of functions: `getenv`, `setenv`, `putenv`.

## The `mimpi` Library

You need to implement the following procedures with signatures from the header file `mimpi.h`:

### Helper Procedures

- `void MIMPI_Init(bool enable_deadlock_detection)`

  Opens the _MPI block_, initializing resources needed for the `mimpi` library to function.
  The `enable_deadlock_detection` flag enables deadlock detection until the end of the _MPI block_ (described below in **Enhancement4**).

- `void MIMPI_Finalize()`

  Ends the _MPI block_.
  All resources related to the `mimpi` library's operation:
  - open files
  - open communication channels
  - allocated memory
  - synchronization primitives
  - etc.
  
  should be released before this procedure ends.

- `int MIMPI_World_size()`

  Returns the number of $prog$ processes launched using the `mimpirun` program (should equal the parameter $n$ passed to the `mimpirun` call).

- `void MIMPI_World_rank()`

  Returns a unique identifier within the group of processes launched by `mimpirun`.
  Identifiers should be consecutive natural numbers from $0$ to $n-1$.

### Point-to-Point Communication Procedures

- `MIMPI_Retcode MIMPI_Send(void const *data, int count, int destination, int tag)`

  Sends data from the address `data`, interpreting it as a byte array of size `count`,
  to the process with rank `destination`, tagging the message with `tag`.

  Executing `MIMPI_Send` for a process that has already left the _MPI block_ should
  immediately fail by returning the error code `MIMPI_ERROR_REMOTE_FINISHED`.
  You don't need to handle situations where the target process terminates
  after `MIMPI_Send` successfully completes in the sending process.

- `MIMPI_Retcode MIMPI_Recv(void *data, int count, int source, int tag)`

  Waits for a message of size (exactly) `count` with tag `tag` from the process
  with rank `rank` and writes its content to the address `data`
  (it's the caller's responsibility to ensure sufficient allocated memory).
  The call is blocking, i.e., it completes only after receiving the entire message.

  Executing `MIMPI_Recv` for a process that
  hasn't sent a matching message and has already left the _MPI block_ should
  fail by returning the error code `MIMPI_ERROR_REMOTE_FINISHED`.
  The same behavior is expected even if the other process leaves the _MPI block_
  while waiting for `MIMPI_Recv`.

  - Basic version: you can assume that each process sends messages in exactly the order
    the recipient wants to receive them. **You cannot** assume that multiple processes **won't** send messages simultaneously to one recipient. You can assume that data associated with one message is no larger than 512 bytes.
  - **Enhancement1**: Sent messages can be arbitrarily (reasonably) large, especially larger than the link buffer (`pipe`).
  - **Enhancement2**: You can't assume anything about the order of sent packets.
    The recipient should be able to buffer incoming packets, and when `MIMPI_Recv` is called, return the first
    (by arrival time) message matching the `count`, `source`, and `tag` parameters
    (you don't need to implement a very sophisticated mechanism for selecting the next matching packet;
    linear complexity relative to the number of all packets not yet processed by the target process
    is entirely sufficient).
  - **Enhancement3**: The recipient should process incoming messages
    concurrently with performing other operations, so that message channels don't overflow.
    In other words, sending a large number of messages isn't blocking even if the target recipient isn't processing them (since they go to an ever-growing buffer).
  - **Enhancement4**: A deadlock is a situation
    where part of the system is in a state that can no longer change
    (there's no sequence of possible future process behaviors that would resolve the deadlock).
    A deadlock between two processes is
    a situation where the deadlock is caused by the state of two processes
    (when considering whether it can be broken, we allow any actions by processes outside the pair - even those not permitted in their current state).

    Examples of certain situations that are deadlocks between process pairs in our system:
    1) A pair of processes executed `MIMPI_Recv` on each other without previously sending a message via `MIMPI_Send` that could end either's wait
    2) One process waits for a message from another process,
    which is already waiting for synchronization related to a group communication procedure call

    For this enhancement, you need to implement at least detection of type 1) deadlocks between pairs.
    Detecting other types of deadlocks won't be tested (you may implement them).
    However, you shouldn't report deadlocks in situations that aren't actually deadlocks.

    When a deadlock is detected, the active call to the `MIMPI_Recv` library function in **both processes** of the detected deadlock pair should immediately terminate by returning the error code `MIMPI_ERROR_DEADLOCK_DETECTED`.

    If multiple deadlocked pairs occur simultaneously, the `MIMPI_Recv` function call
    should be interrupted in every process of every deadlocked pair.

    Deadlock detection may require sending many auxiliary messages, which can significantly slow down the system.
    Therefore, this functionality can be enabled and disabled for the entire _MPI block_ by setting the appropriate value of the `enable_deadlock detection` flag in the `MIMPI_Init` call that starts the block.

    The behavior when deadlock detection is enabled only in some processes of the current `mimpirun` execution is undefined.

    **Note**: _Enhancement4_ (deadlock detection) requires Enhancement2 (message filtering). Partial deadlock detection - without implementing _Enhancement2_ - will be awarded 0 points.

### Group Communication Procedures

#### General Requirements

Each group communication procedure $p$ constitutes a **synchronization point** for all processes,
i.e., instructions following the $i$-th call to $p$ in any process execute **after** every instruction preceding the $i$-th call to $p$ in any other process.

If synchronization of all processes cannot complete
because some process has already left the _MPI block_, the `MIMPI_Barrier` call in at least one process
should terminate with the error code `MIMPI_ERROR_REMOTE_FINISHED`.
If the process where this happens terminates in response to the error,
the `MIMPI_Barrier` call should complete in at least one more process.
Repeating this behavior, we should reach a situation where every process
has left the barrier with an error.

#### Efficiency

Each group communication procedure $p$ should be implemented efficiently.
More precisely, assuming deadlock detection is inactive, we require that from the time $p$ is called by the last process
until $p$ completes in the last process, no more than $\lceil w / 256 \rceil(3\left \lceil\log_2(n+1)-1 \right \rceil t+\epsilon)$ time passes where:

- $n$ is the number of processes
- $t$ is the longest `chsend` execution time associated with sending one message during the given group communication function call. Additional information can be found in the sample `channel.c` implementation and provided tests in the `tests/effectiveness` directory
- $\epsilon$ is a small constant (on the order of milliseconds at most), independent of $t$
- $w$ is the size in bytes of the message processed in the given group communication function call (for `MIMPI_Barrier` calls, assume $w=1$)

Additionally, for the implementation to be considered efficient, transmitted data shouldn't be
accompanied by too many metadata.
In particular, we expect that group functions called for data smaller than 256 bytes will call `chsend` and `chrecv` for packets
no larger than 512 bytes.

The tests in the `tests/effectiveness` directory included in the package check the above-defined efficiency concept.
Passing them is a necessary (though not necessarily sufficient)
condition for earning points for an efficient implementation of group functions.

#### Available Procedures

- `MIMPI_Retcode MIMPI_Barrier()`

  Synchronizes all processes.

- `MIMPI_Retcode MIMPI_Bcast(void *data, int count, int root)`

  Sends data provided by the process with rank `root` to all other processes.

- `MIMPI_Retcode MIMPI_Reduce(const void *send_data, void *recv_data, int count, MPI_Op op, int root)`

  Collects data provided by all processes in `send_data`
  (treating it as an array of `uint8_t` numbers of size `count`)
  and performs a reduction of type `op` on elements with the same indices
  from the `send_data` arrays of all processes (including `root`).
  The reduction result, i.e., a `uint8_t` array of size `count`,
  is written to the `recv_data` address **only** in the process with rank `root` (writing to `recv_data` in other processes is **not allowed**).

  Available reduction types (`enum` `MIMPI_Op` values):
  - `MIMPI_MAX`: maximum
  - `MIMPI_MIN`: minimum
  - `MIMPI_SUM`: sum
  - `MIMPI_PROD`: product

  Note that all the above operations on available data types
  are commutative and associative, and you should optimize `MIMPI_Reduce` accordingly.

### `MIMPI_Retcode` Semantics

See documentation in the `mimpi.h` code:

- `MIMPI_Retcode` documentation,
- documentation of individual procedures returning `MIMPI_Retcode`.

### Tag Semantics

We adopt the convention:

- `tag > 0` is reserved for library users for their own purposes,
- `tag = 0` means `ANY_TAG`. Using it in `MIMPI_Recv` matches any tag.
Don't use it in `MIMPI_Send` (the effect is undefined).
- `tag < 0` is reserved for library implementers and may be used for internal communication.

  This means user programs (e.g., our test programs) will never directly call `MIMPI_Send` or `MIMPI_Recv` with a tag `< 0`.

## Interprocess Communication

The MPI standard is designed with supercomputing in mind.
Therefore, communication between processes typically occurs over a network
and is slower than data exchange within a single computer.

To better simulate the environment of a real library
and thus face its implementation challenges,
interprocess communication should be conducted **exclusively**
using the channels provided in the `channel.h` library.
The `channel.h` library provides the following channel management functions:

- `void channels_init()` - initializes the channel library
- `void channels_finalize()` - finalizes the channel library
- `int channel(int pipefd[2])` - creates a channel
- `int chsend(int __fd, const void *__buf, size_t __n)` - sends a message
- `int chrecv(int __fd, void *__buf, size_t __nbytes)` - receives a message

`channel`, `chsend`, `chrecv` work similarly to `pipe`, `write`, and `read` respectively.
The idea is that the only significant (from the solution's perspective)
difference in behavior between the functions provided by `channel.h` and their originals is
that they may take significantly longer to execute.
In particular, the provided functions:

- have the same signatures as the original functions
- similarly create entries in the open files table
- guarantee atomicity of reads and writes up to and including 512 bytes
- guarantee having a buffer of at least 4 KB
- ... _(if unclear, please ask)_

**NOTE:**
You must call the following helper functions: `channels_init` from `MIMPI_Init`,
and `channels_finalize` from `MIMPI_Finalize`.

**All** reads and writes to file descriptors returned by the `channel` function
must be performed using `chsend` and `chrecv`.
Additionally, you must not call any system functions that modify file properties like `fcntl`
on file descriptors returned by the `channel` function.

Failure to follow these guidelines may result in **complete loss of points.**

Remember that the above guarantees about `chsend` and `chrecv`
don't imply they won't process fewer bytes than requested.
This may happen if the size exceeds the guaranteed channel buffer size,
or if there's insufficient data in the input buffer.
Implementations must correctly handle such situations.

## Notes

### General

- The `mimpirun` program and any functions from the `mimpi` library **must not** create named files in the filesystem.
- The `mimpirun` program and functions from the `mimpi` library may use descriptors
  with numbers in the range $[ 20, 1023 ]$ in any way.
  Additionally, you can assume descriptors in this range aren't occupied when
  the `mimpirun` program is launched.
- The `mimpirun` program and any functions from the `mimpi` library **must not** modify existing
  entries in the open files table outside positions $[ 20, 1023 ]$.
- The `mimpirun` program and any functions from the `mimpi` library **must not** perform any
  operations on files they didn't open themselves (particularly on `STDIN`, `STDOUT`, and `STDERR`).
- No active or semi-active waiting is allowed anywhere.
  - Thus, you shouldn't use any functions that pause program execution
  for a specified time (`sleep`, `usleep`, `nanosleep`) or variants of functions with timeouts (like `select`).
  - You should wait **only** for time-independent events like message arrivals.
- Solutions will be tested for memory leaks and/or other resource leaks (unclosed files, etc.).
  Carefully trace and test all paths that could lead to leaks.
- You can assume that corresponding i-th calls to group communication functions
  in different processes are of the same types (they're the same functions) and have the same values for parameters `count`, `root`, and `op`
  (if the current function type has the given parameter).
- If a system function fails, the calling program should terminate with a non-zero exit code, e.g.,
  by using the provided `ASSERT_SYS_OK` macro.
- If $prog$ programs use the library in ways violating the guarantees listed in this description,
  you may handle it arbitrarily (we won't test such situations).

### The `MIMPI` Library

- Implemented functions don't need to be _threadsafe_, i.e., you can assume they aren't called from multiple threads simultaneously.
- Function implementations should be reasonably efficient,
  i.e., when not under extreme load (e.g., handling hundreds of thousands of messages)
  they shouldn't add beyond the expected execution time (e.g., time spent waiting for a message)
  overhead on the order of tens of milliseconds (or more).
- Calling any procedure other than `MIMPI_Init` outside an MPI block has undefined behavior.
- Multiple calls to `MIMPI_Init` have undefined behavior.
- We guarantee that `channels_init` will set the `SIGPIPE` signal handler to ignore. This will help handle the requirement
  for `MIMPI_Send` to return `MIMPI_ERROR_REMOTE_FINISHED` in appropriate situations.

## Allowed Languages and Libraries

We require using C language in the `gnu11` version (plain `c11` doesn't provide access to many useful standard library functions).
We don't leave a choice because this task is partly meant to deepen C language skills.

You may use the C standard library (`libc`), the `pthread` library, and functionality provided by the system (declared in `unistd.h`, etc.).

**Using other external libraries is prohibited.**

You may borrow any code from labs. Any other borrowed code must be properly commented with its source.

## Package Description

The package contains the following files that aren't part of the solution:

- `examples/*`: simple example programs using the `mimpi` library
- `tests/*`: tests checking various configurations of running example programs with `mimpirun`
- `assignment.md`: this description
- `channel.h`: header file declaring interprocess communication functions
- `channel.c`: sample implementation of `channel.h`
- `mimpi.h`: header file declaring `MIMPI` library functions
- `Makefile`: sample file automating compilation of `mimpirun`, example programs, and running tests
- `self`: helper script for running tests provided with the task
- `test`: script running all tests from the `tests/` directory locally
- `test_on_public_repo`: script running tests according to the [grading scheme](#grading-scheme) below
- `files_allowed_for_change`: script listing files that can be modified
- `template_hash`: file specifying the template version used to prepare the solution

Templates to complete:

- `mimpi.c`: file containing skeletons of `MIMPI` library function implementations
- `mimpirun.c`: file containing the `mimpirun` program implementation skeleton
- `mimpi_common.h`: header file for declaring functionality common to the `MIMPI` library and `mimpirun` program
- `mimpi_common.c`: file for implementing functionality common to the `MIMPI` library and `mimpirun` program

### Helpful Commands

- Build `mimpirun` and all examples from `examples/`: `make`
- Run local tests: `./test`
- Run local tests with valgrind: `VALGRIND=1 ./test`
- Run tests according to the official scheme: `./test_on_public_repo`
  
  The above command lets you verify that your solution meets the technical requirements outlined in the [grading scheme](#grading-scheme).
- List all files opened by processes launched by `mimpirun`: e.g., `./mimpirun 2 ls -l /proc/self/fd`
- Track memory errors, memory leaks, and resource leaks:
  
  The `valgrind` tool may be useful, particularly these flags:
  - `--track-origins=yes`
  - `--track-fds=yes`
  
  _ASAN_ (Address Sanitizer), which has a narrower scope but can also help with debugging, may be useful.
    Enable it by passing the `-fsanitize=address` flag to `gcc`.

## Expected Solution

### Standard Variant (Recommended, Simple and Safe)

To complete the task you must:

1) Complete the solution template according to the specification, modifying only files listed in `files_allowed_for_change` (in particular, you must complete at least `mimpi.c` and `mimpirun.c`).
2) Verify that the solution fits the required [grading scheme](#grading-scheme) by running `./test_on_public_repo`
3) Export the solution using `make assignment.zip && mv assignment.zip ab123456.zip`
   (replacing `ab123456` with your students login)
   and upload the resulting archive to moodle before the deadline.

### Non-Standard Variant

If someone strongly dislikes the provided template and is highly motivated, they may change it.
First, contact the task authors to verify the need.
After positive verification, you must:

1) Fork the public repository with said template.
2) Update your fork with your improvements **ensuring they don't contain solution fragments**
3) Open a _pull request_ to the main repository, describing the changes.
4) During discussion, the changes or new template may be accepted or rejected.
5) Then work using the newly created template (switching to the appropriate branch)

It's important that the updated template fits the grading scheme below,
particularly having an appropriately updated `template_hash` file.

#### About Student Tests

The `make` command automatically tries to build
all programs from `examples/` into `examples_build`.
Then running `./test` runs all recognized tests from the `tests/` directory.
The following convention is adopted:

- `*.self` files are a special type of test run using the provided `self` helper script.
  Their first line specifies the command to run.
  From the 3rd line to the end of the file is the expected `STDOUT` value the above command should generate.
- `*.sh` files are arbitrary shell scripts. They may implement any logic.
  They should just exit with code $0$ on success (test passed) and non-zero code on error detection.

If you prepare your own tests following the above scheme, we encourage sharing them.
To do this, just somehow pass them to the task authors (preferably via _pull request_).
To encourage this effort, we'll try to run such published tests on the reference solution
and provide feedback if they're incorrect.

## Grading Scheme

Your uploaded solutions will be built and tested according to the following scheme:

1) The solution `zip` package with the correct name `ab123456.zip` (use your students login)
   will be downloaded from moodle and unpacked.
2) A clean clone $K$ of the public repository containing the provided template will be created.
3) In $K$, the branch will be selected based on the `template_hash` value from your solution.
4) To $K$ will be copied those files from your solution that are listed in `files_allowed_for_change` from the **version** in $K$
5) To $K$ will be copied our prepared set of tests (appropriate files in `examples/` and `tests/`).
6) In $K$, `make` will be called to build your solution and all examples.
7) In $K$, `./test` will be called
8) The solution will be graded based on which set of tests your solution passed.

Successful completion without errors of the above building process
and passing the following automatic tests:

- hello
- send_recv

is a necessary condition to receive non-zero points.
To check your solution's compliance with the above grading scheme, use the `./test_on_public_repo` script.
We encourage using it.

**Note:** In case of discrepancies in the above process's results, the behavior on the `students` server will be considered.

## Graded Solution Components

**Note**: The point values below are **only** indicative and may change arbitrarily.
Their purpose is to help estimate the difficulty/complexity of given functionality.

### Base: (3p)

- `mimpirun` program implementation + `MIMPI_Init` and `MIMPI_Finalize` procedures + passing the `hello` example/test (1p)
- Point-to-point function implementation (assuming messages up to 500B) (1p)
- Group function implementation (message size as above) (1p)

### Enhancements (7p)

- Efficient (logarithmic) group functions (2p)
  - `MIMPI_Barrier` (1p)
  - `MIMPI_Bcast` and `MIMPI_Reduce` (1p)
- (`MIMPI_Recv`::Enhancement1) arbitrarily large messages (arbitrarily over 500B) (1p)
- (`MIMPI_Recv`::Enhancement2) handling any message sending order (filtering by tags, size), including `MIMPI_ANY_TAG` handling (1p)
- (`MIMPI_Recv`::Enhancement3) non-clogging channels (1p)
  - No upper limit on data size until the sending process blocks
  - No waiting when calling `MIMPI_Send`
- (`MIMPI_Recv`::Enhancement4) deadlock detection between process pairs (2p)

We listed enhancements in the order we suggest prioritizing them.