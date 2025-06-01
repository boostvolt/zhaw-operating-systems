# 6 Memory

## Introduction & Prerequisites

This laboratory is to learn how to:

- Understand how Linux reports memory statistics and usage
- Understand the relationship between program memory requirements and Linux
- Understand practical ramifications of pages
- How to limit memory on a user basis

The following resources and tools are required for this laboratory session:

- A ZHAW VPN session
- Any modern web browser
- Any modern SSH client application
- OpenStack Horizon dashboard: https://ned.cloudlab.zhaw.ch
- OpenStack account details
  - See Moodle
- Username to login with SSH into VMs in ned.cloudlab.zhaw.ch OpenStack cloud from your laptops
- Ubuntu VM with at least 2 cores
  - Installed C compiler, and tools (gcc/make)
  - `sudo apt update`
  - `sudo apt install build-essential`

### Time

The entire session will require 90 minutes.

### Assessment

No assessment foreseen

### Task 1 – Memory basic Tasks

Check the number of CPUs and the number of online-cpus (using which command?)

**[Lab Output & Explanation]**

```sh
lscpu
```

Output (abridged):

```
CPU(s):                          2
On-line CPU(s) list:             0
Off-line CPU(s) list:            1
```

- `lscpu` reports 2 CPUs, but only CPU 0 is online. This is confirmed by:

```sh
nproc
```

Output:

```
1
```

- Only 1 processing unit is available to the OS.

```sh
cat /proc/cpuinfo | grep processor
```

Output:

```
processor       : 0
```

- Only processor 0 is present/online.

| Command       | Output/Meaning                |
| ------------- | ----------------------------- |
| lscpu         | 2 CPUs, only CPU 0 online     |
| nproc         | 1 (processing unit available) |
| /proc/cpuinfo | Only processor 0 listed       |

---

Check the compiler installation (using which command?)

**[Lab Output & Explanation]**

```sh
gcc --version
```

Output:

```
gcc (Ubuntu 9.4.0-1ubuntu1~20.04.2) 9.4.0
```

- GCC is installed and available.

#### Subtask 1.1 – Basic memory under Linux

Download and unzip the file package `MEM_students_code.zip`.

Open the `MEM_lab.c` file and inspect the code. In the first section (**ToDo 1**,) call up a function to print out the PID of the running process - this will come in useful later (hint - use the getpid system call).

---

```c
(void) printf("----- the PID of this process is %i\n", getpid() );
while (1) { sleep(1); }
```

Add an endless loop below this, exit, compile and run.

**[Lab Output & Explanation]**

- The code was modified as above, compiled, and run on the SSH server.
- Output when running:

```sh
$ ./MEM_lab
Hello MEM Lab
------------ Part 1:  Simple program check memory of process
----- the PID of this process is 37565
```

- The process enters an endless loop, allowing inspection from other terminals.

---

In a second terminal run top or htop - what memory parameters are useful to know? (Hint - use the documentation for htop to look for VIRT/RES/SHR).

**[Lab Output & Explanation]**

```sh
top -b -n 1 | head -20
```

Output (abridged):

```
    PID USER      PR  NI    VIRT    RES    SHR S  %CPU  %MEM     TIME+ COMMAND
  ...
 37565 ubuntu     20   0   3984K   900K      0 S   ...   ...      ...   MEM_lab
```

- **-b**: Run top in batch mode (suitable for scripting or capturing output; no interactive UI)
- **-n 1**: Run for 1 iteration (one snapshot of the process list, then exit)
- **VIRT**: Virtual memory size
- **RES**: Resident memory (physical RAM used)
- **SHR**: Shared memory with other processes

---

In a third terminal using the command free (hint - man free) display the system memory parameters in kilobytes.

**[Lab Output & Explanation]**

```sh
free -k
```

Output:

```
              total        used        free      shared  buff/cache   available
Mem:        8153156      185936     6547728        1020     1419492     7692548
Swap:             0           0           0
```

- **-k**: Show memory values in kilobytes (default in many modern systems, but explicit for clarity)
- **total**: Total installed RAM
- **used**: Memory in use
- **free**: Unused memory
- **shared**: Memory used (mainly) by tmpfs
- **buff/cache**: Memory used by kernel buffers and cache
- **available**: Estimated available memory for new processes

---

Read the file `/proc/${pid}/status` specifically the memory related portions. What do the fields mean? (hint-man proc)

**[Lab Output & Explanation]**

```sh
cat /proc/37565/status | grep -i vm
```

Output:

```
VmPeak:     3984 kB
VmSize:     3984 kB
VmLck:         0 kB
VmPin:         0 kB
VmHWM:       900 kB
VmRSS:       900 kB
VmData:      192 kB
VmStk:       132 kB
VmExe:       136 kB
VmLib:      2312 kB
VmPTE:        48 kB
VmSwap:        0 kB
```

- **-i**: Ignore case when matching (so 'vm', 'Vm', 'VM', etc. will all match)
- **vm**: The search pattern; matches any line containing 'vm' (case-insensitive)
- **VmPeak**: Peak virtual memory size
- **VmSize**: Current virtual memory size
- **VmLck**: Locked memory size (see mlock(2))
- **VmPin**: Pinned memory size (pages that can't be moved)
- **VmHWM**: Peak resident set size
- **VmRSS**: Resident set size (physical RAM)
- **VmData**: Data segment size
- **VmStk**: Stack size
- **VmExe**: Text (code) size
- **VmLib**: Shared library code size
- **VmPTE**: Page table entries size
- **VmSwap**: Swapped-out virtual memory size

---

What is the difference between VmPin and VmLck?

**[Lab Output & Explanation]**

- **VmLck**: Amount of memory (in kB) locked into RAM by the process using `mlock`/`mlockall`. Locked memory cannot be swapped out, but may still be moved in physical RAM by the kernel.
- **VmPin**: Amount of memory (in kB) pinned by the kernel, which cannot be swapped out _or_ moved. This is typically used for kernel operations like direct I/O or DMA, where the memory must remain at a fixed physical address.

| Field | Meaning                                                                                  |
| ----- | ---------------------------------------------------------------------------------------- |
| VmLck | Memory locked into RAM by user request (mlock/munlock); cannot be swapped, but can move. |
| VmPin | Memory pinned by the kernel; cannot be swapped or moved (used for DMA, etc.).            |

---

Research the command `smem`, install if necessary. What information does smem give you about your system?

Note: `smem` is a useful tool for helping set up resource management in server systems.

**[Lab Output & Explanation]**

```sh
smem -r | head -20
```

Output (abridged):

```
  PID User     Command                         Swap      USS      PSS      RSS
  690 ubuntu   /usr/bin/python3 -m http.se        0     8596    10096    16264
 1524 ubuntu   /usr/bin/python3 /usr/bin/s        0     6368     7795    12800
  677 ubuntu   /lib/systemd/systemd --user        0     1540     2805     9532
  951 ubuntu   -bash                              0     1840     2508     5112
 1523 ubuntu   bash -c smem -r | head -20         0      292      929     3228
 1525 ubuntu   head -20                           0      148      272     1708
```

- `smem` provides detailed memory usage statistics for each process, including:
  - **Swap**: Amount of swap used by the process
  - **USS (Unique Set Size)**: Memory unique to the process (not shared)
  - **PSS (Proportional Set Size)**: Process's share of shared memory
  - **RSS (Resident Set Size)**: Total physical memory used by the process
- `smem` is useful for understanding real memory usage, especially in systems with significant shared memory between processes.

- The `-r` flag for `smem` tells it to run as root (with elevated privileges) if possible, which allows it to collect more complete and accurate memory usage information for all processes, including those owned by other users. Without `-r`, smem may not be able to access memory information for some processes due to permission restrictions.

---

Go back to the code program. Remove the endless loop and insert code to read the page size (store it in a variable) and print it out. (**ToDo 2**) Build and run (hint - man getpagesize).

**[Lab Output & Explanation]**

Modified the code to implement ToDo 2:

```c
// ToDo 2: insert function to get and read the page size
int pagesize = getpagesize();
(void) printf("Page size is %d bytes\n", pagesize);
```

Building and running the program:

```sh
cd MEM_students_code && make out
```

Output (abridged):

```
gcc -std=gnu99 -Wall  -c sobel_rgb2g.c -o sobel_rgb2g.o
gcc -std=gnu99 -Wall  -c MEM_lab.c -o MEM_lab.o
...
Linking
gcc -std=gnu99 -Wall  sobel_rgb2g.o MEM_lab.o -o out.t -lm
```

Running the program:

```sh
./out.t
```

Output:

```
Hello MEM Lab
------------ Part 1:  Simple program check memory of process
----- the PID of this process is 4876
------------ Part 2:  whats the page size
Page size is 4096 bytes
Press return to continue
```

- The `getpagesize()` function successfully returns the system's memory page size
- The endless loop from ToDo 1 has been removed (commented out)
- Page size is reported as 4096 bytes, which is typical for x86_64 systems

---

Verify this with the `getconf` command (hint - man getconf)

**[Lab Output & Explanation]**

```sh
getconf PAGESIZE
```

Output:

```
4096
```

- The `getconf PAGESIZE` command confirms the same page size as reported by `getpagesize()`
- Both commands return 4096 bytes, validating our implementation
- `getconf` provides system configuration values, including memory page size

| Command            | Output     | Purpose                                      |
| ------------------ | ---------- | -------------------------------------------- |
| `getpagesize()`    | 4096 bytes | C function to get page size programmatically |
| `getconf PAGESIZE` | 4096       | Shell command to get system page size        |

---

Now include code to reserve memory (hint - man malloc) the size of a number of pages (**ToDo 3**).

**[Lab Output & Explanation]**

Modified the code to implement ToDo 3:

```c
// ToDo 3: insert code to reserve memory
// Reserve memory for 4 pages
int num_pages = 4;
void *memory_ptr = malloc(num_pages * pagesize);

if (memory_ptr == NULL) {
    perror("malloc failed");
    exit(EXIT_FAILURE);
}

(void) printf("Reserved %d bytes (%d pages) of memory at address %p\n",
              num_pages * pagesize, num_pages, memory_ptr);
```

Building and running the program:

```sh
cd MEM_students_code && make out
```

Output:

```
Hello MEM Lab
------------ Part 1:  Simple program check memory of process
----- the PID of this process is 7469
------------ Part 2:  whats the page size
Page size is 4096 bytes
Press return to continue
------------ Part 3:  generate some memory area
Reserved 16384 bytes (4 pages) of memory at address 0x558d96fcf2c0
```

- Successfully allocated 16,384 bytes (4 × 4096-byte pages) using `malloc()`
- Memory address is in the heap region (0x558d96fcf2c0)
- The program continues to next sections without errors

---

After each execution step of this code run the command (another terminal).

`ps -o min_flt,maj_flt {pid}`

Use the man page to understand the parameters. What do you notice?

**[Lab Output & Explanation]**

Checking man page for page fault parameters:

```sh
man ps | grep -A 5 -B 5 'min_flt'
```

Output:

```
maj_flt     MAJFLT    The number of major page faults that have
                      occurred with this process.

min_flt     MINFLT    The number of minor page faults that have
                      occurred with this process.
```

Monitoring page faults during program execution:

```sh
ps -o pid,min_flt,maj_flt -p {pid}
```

Sample output (before and after memory allocation):

```
    PID  MINFL  MAJFL
   7469   4856      0
```

After memory operations:

```
    PID  MINFL  MAJFL
   7469   5200      0
```

**What do you notice:**

- **Minor page faults (MINFL)** increase during program execution
- **Major page faults (MAJFL)** remain at 0 in this case
- **Minor faults**: Occur when a page is in memory but not in the process's page table (e.g., first access to allocated memory)
- **Major faults**: Occur when a page must be loaded from disk (swap or file system)
- The `malloc()` call itself may not immediately cause page faults due to lazy allocation

| Parameter | Meaning           | Typical Behavior                 |
| --------- | ----------------- | -------------------------------- |
| min_flt   | Minor page faults | Increases with memory access     |
| maj_flt   | Major page faults | Usually 0 unless swapping occurs |

---

Return to the code - for **ToDo 4** - use the `aligned_alloc` function to reserve a buffer of a number of pages size, aligned on a page boundary. Then use the function `mincore` to check whether the pages are in memory.

```c
// ToDo 4: reserve memory with aligned_alloc; this is necessary for the mincore function to function properly
char *pointer1 = aligned_alloc(pagesize, 4 * pagesize);

// initialize array the size of the number of pages previously reserved through malloc
char page_array[3] = {0, 0, 0};

// check the pages/malloc memory reserved
errno = 0;
result = mincore( pointer1, pagesize * 3, page_array );

if ( result != 0 )
    perror("Error returned");
else
    (void) printf("Area 1 is in memory %i, Area 2 is in memory %i, Area 3 is in memory %i\n", ( page_array[0] & 0x01), ( page_array[1] & 0x01), ( page_array[2] & 0x01) );
```

What do you see?

**[Lab Output & Explanation]**

Implemented ToDo 4 with `aligned_alloc` and `mincore`:

```c
// ToDo 4: reserve memory with aligned_alloc
char *pointer1 = aligned_alloc(pagesize, 4 * pagesize);

if (pointer1 == NULL) {
    perror("aligned_alloc failed");
    exit(EXIT_FAILURE);
}

(void) printf("Allocated %d bytes aligned on page boundary at address %p\n",
              4 * pagesize, pointer1);

// initialize array the size of the number of pages previously reserved through malloc
char page_array[3] = {0, 0, 0};

// check the pages/malloc memory reserved
errno = 0;
result = mincore( pointer1, pagesize * 3, page_array );

if ( result != 0 )
    perror("Error returned");
else
    (void) printf("Area 1 is in memory %i, Area 2 is in memory %i, Area 3 is in memory %i\n", ( page_array[0] & 0x01), ( page_array[1] & 0x01), ( page_array[2] & 0x01) );
```

Running the program:

```sh
./out.t
```

Output:

```
------------ Part 4: Are these pages in memory?
Allocated 16384 bytes aligned on page boundary at address 0x562b050d0000
Area 1 is in memory 0, Area 2 is in memory 0, Area 3 is in memory 0
```

**What do you see:**

- All three pages show as **0** (not in memory)
- This demonstrates **Linux lazy allocation** - memory is allocated virtually but not physically
- `aligned_alloc()` provides page-aligned memory required by `mincore()`
- `mincore()` checks if pages are resident in physical memory
- The `& 0x01` operation extracts the least significant bit indicating residency

---

Linux uses lazy allocation - include an access to the buffer - **ToDo 5** - and run the code again. What do you see when you run the code and check the page faults reported by the ps command?

**[Lab Output & Explanation]**

Implemented ToDo 5 to access the allocated memory:

```c
// ToDo 5: write something into the reserved buffer
// Access the memory to trigger lazy allocation
(void) printf("Writing to the allocated memory...\n");
pointer1[0] = 'A';                    // Write to page 1
pointer1[pagesize] = 'B';             // Write to page 2
pointer1[2 * pagesize] = 'C';         // Write to page 3

// Check again after accessing the memory
errno = 0;
result = mincore( pointer1, pagesize * 3, page_array );

if ( result != 0 )
    perror("Error returned after memory access");
else
    (void) printf("After access - Area 1 is in memory %i, Area 2 is in memory %i, Area 3 is in memory %i\n", ( page_array[0] & 0x01), ( page_array[1] & 0x01), ( page_array[2] & 0x01) );
```

Running the program:

```sh
./out.t
```

Output:

```
Area 1 is in memory 0, Area 2 is in memory 0, Area 3 is in memory 0
Writing to the allocated memory...
After access - Area 1 is in memory 1, Area 2 is in memory 1, Area 3 is in memory 1
```

Monitoring page faults during execution:

```sh
ps -o pid,min_flt,maj_flt -p {pid}
```

Sample output (before and after memory access):

```
Before:  PID  MINFL  MAJFL
        9090   6070      0

After:   PID  MINFL  MAJFL
        9090   7782      0
```

**What do you see:**

- **Before access**: All pages show as 0 (not in memory)
- **After access**: All pages show as 1 (in memory)
- **Page faults increased**: Minor faults increased by ~1700 (from 6070 to 7782)
- **Lazy allocation demonstrated**: Physical memory is only allocated when first accessed
- **Minor faults occur**: When pages are allocated on first write access

| Condition               | Page Status | Page Faults  | Explanation                        |
| ----------------------- | ----------- | ------------ | ---------------------------------- |
| After `aligned_alloc()` | 0,0,0       | Lower count  | Virtual memory allocated only      |
| After memory writes     | 1,1,1       | Higher count | Physical pages allocated on access |

**Key insight**: Linux's lazy allocation means `malloc()` and `aligned_alloc()` only reserve virtual address space. Physical memory allocation happens on first access, triggering minor page faults.

#### Subtask 1.2 - Limiting memory

From reading the process status file we know the maximum amount of memory the process uses during startup. We can now attempt to limit this on a high level.

Research the `ulimit` command and use it to display the resource limitations. What precisely is limited?

Using the data from reading `/proc/${pid}/status` let us limit the available memory for the start phase of the test program to under the peak requirement. What happens?

**[Lab Output & Explanation]**

**Research on `ulimit` command:**

Checking current resource limitations:

```sh
ulimit -a
```

Output:

```
core file size          (blocks, -c) 0
data seg size           (kbytes, -d) unlimited
scheduling priority             (-e) 0
file size               (blocks, -f) unlimited
pending signals                 (-i) 31712
max locked memory       (kbytes, -l) 65536
max memory size         (kbytes, -m) unlimited
open files                      (-n) 1024
pipe size            (512 bytes, -p) 8
POSIX message queues     (bytes, -q) 819200
real-time priority              (-r) 0
stack size              (kbytes, -s) 8192
cpu time               (seconds, -t) unlimited
max user processes              (-u) 31712
virtual memory          (kbytes, -v) unlimited
file locks                      (-x) unlimited
```

**What precisely is limited by each option:**

| Option | Limit             | Description                               |
| ------ | ----------------- | ----------------------------------------- |
| `-d`   | Data segment size | Limits heap memory (malloc, data segment) |
| `-m`   | Max memory size   | Limits resident set size (physical RAM)   |
| `-v`   | Virtual memory    | Limits total virtual address space        |
| `-s`   | Stack size        | Limits stack memory (default: 8192 KB)    |
| `-l`   | Max locked memory | Limits memory that can be locked in RAM   |

**Memory limitation experiments:**

From `/proc/pid/status`, our program's memory requirements:

```
VmPeak:    77404 kB    (Peak virtual memory)
VmSize:    77404 kB    (Current virtual memory)
VmHWM:     29628 kB    (Peak resident set size)
VmRSS:     29628 kB    (Current resident set size)
```

**Test 1: Virtual memory limitation (`ulimit -v`)**

Limiting virtual memory to 50MB (below 77MB requirement):

```sh
ulimit -v 50000
./out.t
```

Output:

```
Testing memory limitation with ulimit -v (virtual memory)
Virtual memory limited to 50MB
File read failed
: Cannot allocate memory
Hello MEM Lab
------------ Part 1:  Simple program check memory of process
----- the PID of this process is 11344
[... program continues but fails later ...]
Program failed with exit code: 1
```

**Test 2: Very restrictive virtual memory (20MB)**

```sh
ulimit -v 20000
./out.t
```

Output:

```
Testing with very restrictive virtual memory limit (20MB)
Hello MEM Lab
[... program runs partially then fails ...]
Program failed with exit code: 1
File read failed
: Cannot allocate memory
```

**Test 3: Resident set size limitation (`ulimit -m`)**

```sh
ulimit -v unlimited  # Reset virtual memory
ulimit -m 20000      # Limit RSS to 20MB
./out.t
```

Output:

```
Testing with resident set size limit (RSS) - 20MB
Hello MEM Lab
[... program runs completely successfully ...]
Bye MEM Lab
```

**Test 4: Data segment limitation (`ulimit -d`)**

```sh
ulimit -m unlimited  # Reset RSS limit
ulimit -d 10000      # Limit data segment to 10MB
./out.t
```

Output:

```
Testing with data segment size limit (-d) - 10MB
File read failed
: Cannot allocate memory
[... program runs partially then fails ...]
Program failed with exit code: 1
```

**Test 5: Extremely restrictive data segment (10KB)**

```sh
ulimit -d 10          # Limit to 10KB
./out.t
```

Output:

```
Testing with extremely small data segment limit (-d) - 10KB
bash: line 3: 11824 Segmentation fault      ./out.t
Program failed with exit code: 139
```

**Test 6: Page fault monitoring with unlimited resources**

```sh
ulimit -d unlimited && ulimit -v unlimited && ulimit -m unlimited
./out.t &
PID=$!
ps -o pid,min_flt,maj_flt -p $PID  # Before completion
sleep 3
ps -o pid,min_flt,maj_flt -p $PID  # After completion
```

Output:

```
Process PID: 11920
    PID  MINFL  MAJFL
  11920   7271      0
    PID  MINFL  MAJFL
  11920  11946      0
```

**Analysis - What happens when memory is limited:**

| Limit Type                   | Effect                                        | Behavior                                            |
| ---------------------------- | --------------------------------------------- | --------------------------------------------------- |
| **Virtual memory (`-v`)**    | Process cannot allocate virtual address space | Program fails during malloc/aligned_alloc calls     |
| **Resident set size (`-m`)** | Limits physical RAM usage                     | Often ignored by modern Linux (swap available)      |
| **Data segment (`-d`)**      | Limits heap memory (malloc area)              | Program fails when heap allocation exceeds limit    |
| **Stack size (`-s`)**        | Limits stack memory                           | Would affect recursive functions/large local arrays |

**Key findings:**

1. **Virtual memory limits (`-v`)** are strictly enforced - program fails when trying to allocate beyond limit
2. **Data segment limits (`-d`)** prevent heap allocation (malloc/aligned_alloc) - most effective for controlling our program
3. **RSS limits (`-m`)** are often not enforced on modern Linux systems with swap available
4. **Very restrictive limits** cause segmentation faults (exit code 139)
5. **Page faults increase** significantly during memory operations (~4675 minor faults during execution)

**Conclusion:**

- `ulimit -v` controls total virtual address space (most comprehensive)
- `ulimit -d` controls heap allocation (affects malloc/aligned_alloc directly)
- `ulimit -m` controls physical memory but may be ignored if swap is available
- Memory limitations can cause programs to fail at different stages depending on when memory allocation occurs
