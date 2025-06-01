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

---

Verify this with the `getconf` command (hint - man getconf)

---

Now include code to reserve memory (hint - man malloc) the size of a number of pages (**ToDo 3**).

---

After each execution step of this code run the command (another terminal).

`ps -o min_flt,maj_flt {pid}`

Use the man page to understand the parameters. What do you notice?

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

---

Linux uses lazy allocation - include an access to the buffer - **ToDo 5** - and run the code again. What do you see when you run the code and check the page faults reported by the ps command?

#### Subtask 1.2 – Limiting memory

From reading the process status file we know the maximum amount of memory the process uses during startup. We can now attempt to limit this on a high level.

---

Research the `ulimit` command and use it to display the resource limitations. What precisely is limited?

---

Using the data from reading `/proc/${pid}/status` let us limit the available memory for the start phase of the test program to under the peak requirement. What happens?
