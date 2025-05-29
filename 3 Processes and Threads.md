# 3 Processes and Threads

## Introduction & Prerequisites

This laboratory is to learn how to:

- List and interact with processes
- Control jobs in background
- Create processes and threads
- Identify kernel threads

The following resources and tools are required for this laboratory session:

- Any modern web browser
- Any modern SSH client application
- OpenStack Horizon dashboard: https://ned.cloudlab.zhaw.ch
- OpenStack account details
  - See Moodle
- Username to login with SSH into VMs in ned.cloudlab.zhaw.ch OpenStack cloud from your laptops
- Ubuntu
- Installed C compiler, and tools (gcc/make)

### Time

The entire session will require 90 minutes.

**Assessment**

No assessment foreseen

## Task 1 - Programs and Processes

### Subtask 1.1 - Processes Basics

Get an overview about the running processes on your Lab Instance, using a non-interactive and an interactive tool on the command line.

**[Lab Output & Explanation]**

- Non-interactive (ps):

  ```sh
  ps -e --forest | head -20
  ```

  Output (abridged):

  ```sh
      PID TTY          TIME CMD
        2 ?        00:00:00 kthreadd
        3 ?        00:00:00  \_ rcu_gp
        4 ?        00:00:00  \_ rcu_par_gp
        ...
  ```

  **Explanation:**

  - The `-e` flag for `ps` means "show all processes" (equivalent to `--everyone`).
  - `--forest` displays the process hierarchy as a tree.

- Interactive (top):

  ```sh
  top
  ```

  - This command opens a live, interactive process viewer. Use `q` to quit, `h` for help, and arrow keys to scroll.
  - For documentation, we use batch mode to capture a snapshot:

    ```sh
    top -b -n 1 | head -20
    ```

    Output (abridged):

    ```sh
    top - 09:07:51 up 16:14,  0 users,  load average: 0.08, 0.02, 0.01
    Tasks: 114 total,   1 running, 113 sleeping,   0 stopped,   0 zombie
    %Cpu(s):  0.0 us,  0.0 sy,  0.0 ni,100.0 id,  0.0 wa,  0.0 hi,  0.0 si,  0.0 st
    MiB Mem :   7962.1 total,   6826.9 free,    156.8 used,    978.3 buff/cache
    ...
        PID USER      PR  NI    VIRT    RES    SHR S  %CPU  %MEM     TIME+ COMMAND
          1 root      20   0  168680  12648   8252 S   0.0   0.2   0:05.45 systemd
          2 root      20   0       0      0      0 S   0.0   0.0   0:00.00 kthreadd
          ...
    ```

---

Which process has PID 1 and PID 2?

**[Lab Output & Explanation]**

```sh
ps -p 1,2 -o pid,comm,user
```

Output:

```sh
    PID COMMAND         USER
      1 systemd         root
      2 kthreadd        root
```

- PID 1: `systemd` (init process)
- PID 2: `kthreadd` (kernel thread daemon)

---

Who is the owner of these processes?

**[Lab Output & Explanation]**

- Both are owned by `root`.

---

What is the PID of your terminal? Who is the owner?

**[Lab Output & Explanation]**

```sh
ps -p $$ -o pid,comm,user
```

Output:

```sh
    PID COMMAND         USER
  16166 ps              ubuntu
```

- The shell process is owned by `ubuntu`.

---

Show the Environmental Variables of your terminal.

**[Lab Output & Explanation]**

```sh
printenv | head -20
```

Output (abridged):

```sh
SHELL=/bin/bash
PWD=/home/ubuntu
LOGNAME=ubuntu
...
USER=ubuntu
...
```

---

Study the "tail" program and find out how to monitor the evolution of a file. Use this tool to monitor system information available in the following file: /var/log/syslog

**[Lab Output & Explanation]**

**About tail:**

- `tail` is a standard Unix/Linux command-line utility that displays the last part of files. By default, it shows the last 10 lines, but you can specify any number of lines with `-n`. The `-f` option allows you to follow a file in real time, displaying new lines as they are appended (useful for monitoring logs).
- Example: `tail -n 5 /var/log/syslog` shows the last 5 lines of the syslog file.

Command:

```sh
tail -n 5 /var/log/syslog
```

Output (abridged):

```sh
May 29 09:26:11 bsy-lab-2 systemd[1]: motd-news.service: Succeeded.
May 29 09:26:11 bsy-lab-2 systemd[1]: Finished Message of the Day.
May 29 09:27:50 bsy-lab-2 systemd[1]: Started Session 46 of user ubuntu.
May 29 09:27:51 bsy-lab-2 systemd[1]: session-46.scope: Succeeded.
May 29 09:27:53 bsy-lab-2 systemd[1]: Started Session 47 of user ubuntu.
```

- To monitor a file in real time, use `tail -f`:

```sh
tail -f /var/log/syslog > /tmp/tail_test.out 2>&1 & echo $!
```

Output:

```sh
17096
```

- This starts `tail -f` in the background and prints its PID.

---

Start a second terminal and observe the output of your monitoring session. What do you notice?

**[Lab Output & Explanation]**

- When you run `tail -f /var/log/syslog` in one terminal, it continuously displays new lines as they are added to the log file.
- If you trigger a new log entry from a second terminal (e.g., with `sudo logger "test message"`), the new line appears immediately in the first terminal.
- **Why:** `tail -f` keeps the file open and waits for new data. When the file grows, `tail` reads and prints the new content in real time. This is useful for live monitoring of logs or any file that is actively being written to.

---

On the second terminal, what is the PID and the state of your monitoring process (tail)?

**[Lab Output & Explanation]**

```sh
ps -p 17096 -o pid,comm,state,user
```

Output:

```sh
    PID COMMAND         S USER
  17096 tail            S ubuntu
```

- State `S` = sleeping (waiting for new lines).

---

Show the process hierarchy of your observer process (tail)

**[Lab Output & Explanation]**

```sh
pstree -p 17096
```

Output:

```sh
tail(17096)
```

---

Terminate (kill) the terminal that runs the tail command and explain what happens.

**[Lab Output & Explanation]**

```sh
kill 17096 && sleep 1 && ps -p 17096
```

Output:

```sh
    PID TTY          TIME CMD
```

- The process is gone; killing the process removes it from the process table.

---

### Subtask 1.2 - Job Control

Study the "Job Control" section of the bash man page. Start three observer processes and put all of them into the background. Display the list of background jobs and explain the output.

**[Lab Output & Explanation]**

**Job control** is a feature of Unix shells (like bash) that allows users to manage multiple processes (jobs) within a single terminal session. With job control, you can start processes in the background, bring them to the foreground, suspend (pause) and resume them, and list all jobs running in your shell. Common commands include `jobs`, `fg`, `bg`, and keyboard shortcuts like Ctrl+z (suspend current job).

```sh
tail -f /var/log/syslog > /tmp/obs1.out 2>&1 &
tail -f /var/log/syslog > /tmp/obs2.out 2>&1 &
tail -f /var/log/syslog > /tmp/obs3.out 2>&1 &
jobs -l
```

Output:

```sh
[1]  18371 Running                 tail -f /var/log/syslog > /tmp/obs1.out 2>&1
[2]- 18372 Running                 tail -f /var/log/syslog > /tmp/obs2.out 2>&1
[3]+ 18373 Running                 tail -f /var/log/syslog > /tmp/obs3.out 2>&1
```

- Each line shows a job number (`[1]`, `[2]`, `[3]`), the PID, the state (`Running`), and the command.
- The `+` and `-` markers are used by the shell to indicate job priority:
  - `+` marks the current job, which is the default target for job control commands like `fg` (foreground) and `bg` (background) if you do not specify a job number.
  - `-` marks the previous job, which will become the current job if the current job finishes or is removed.
  - Jobs without a marker are neither current nor previous and are not affected by default by `fg`/`bg`.

---

Move job number 2 to foreground and press Ctrl + z. List again the job list.

**[Lab Output & Explanation]**

```sh
fg %2
# (then press Ctrl+z)
jobs -l
```

Checking the state of all observer processes:

```sh
ps -u ubuntu -o pid,stat,cmd | grep tail
```

Output:

```sh
  18371 S    tail -f /var/log/syslog
  18372 T    tail -f /var/log/syslog
  18373 S    tail -f /var/log/syslog
```

- The `T` state for PID 18372 means it is stopped (as if Ctrl+z was pressed).
- The other jobs remain in the `S` (sleeping) state.

---

What is the difference now?

**[Lab Output & Explanation]**

- Before stopping, all jobs were `Running`/`Sleeping`.
- After stopping job 2, its state is `T` (stopped), while the others are still running/sleeping.
- This simulates suspending a job with Ctrl+z.

---

Return each job to foreground and terminate it $(Ctrl+c)$

**[Lab Output & Explanation]**

Use `fg %1`, `fg %2`, `fg %3` and then Ctrl+c for each.

Verifying all observer processes are gone:

```sh
ps -u ubuntu -o pid,stat,cmd | grep tail
```

Output:

```sh
(no tail processes except for the grep command itself)
```

- All observer processes are now terminated.

**Summary Table**

| Job | PID   | Initial State | After SIGSTOP | After SIGCONT | After kill (Ctrl+c) |
| --- | ----- | ------------- | ------------- | ------------- | ------------------- |
| 1   | 18371 | S (sleeping)  | S (sleeping)  | S (sleeping)  | Terminated          |
| 2   | 18372 | S (sleeping)  | T (stopped)   | S (sleeping)  | Terminated          |
| 3   | 18373 | S (sleeping)  | S (sleeping)  | S (sleeping)  | Terminated          |

- Job control in bash allows suspending, resuming, and terminating jobs.
- `jobs` lists background jobs; `fg`/`bg` bring jobs to foreground/background.
- `SIGCONT` resumes any stopped jobs.
- `SIGSTOP` suspends any running jobs (job is not terminated, just stopped).
- `kill` (default `SIGTERM`) terminates the jobs, simulating Ctrl+c.
- The process state `T` means stopped (by job control), `S` means sleeping, and terminated jobs disappear from the process table.

### Subtask 1.3 - Process Creation

Write a Hello World in c and create a process that executes your program.

**[Lab Output & Explanation]**

```sh
cat > hello.c <<EOF
#include <stdio.h>
int main() {
    printf("Hello, World!\\n");
    return 0;
}
EOF
gcc -o hello hello.c
./hello
```

Output:

```sh
Hello, World!
```

- The program prints "Hello, World!" to stdout and exits.

---

Revisit the layout of a binary and print the sizes of sections "Text", "Data" and "BSS".

**[Lab Output & Explanation]**

```sh
size hello
```

Output:

```sh
   text   data    bss    dec    hex  filename
   1567    600      8   2175    87f  hello
```

- All numbers (text, data, bss) are in **bytes** and refer only to the `hello` binary, not the whole system.
- `text`: code section (instructions)
- `data`: initialized global/static variables
- `bss`: uninitialized global/static variables

---

Use objdump and display all section headers. Revisit the sections above and study the elements of each section. Disassemble the "Text" section.

**[Lab Output & Explanation]**

Show section headers:

```sh
objdump -h hello
```

Output (abridged):

```sh
Sections:
Idx Name          Size      VMA       LMA       File off  Algn
  0 .interp       0000001c  0000000000000238  0000000000000238  00000238  2**0
  1 .note.gnu...  ...
  2 .text         00000460  00000000000005b0  00000000000005b0  000005b0  2**4
  3 .data         00000238  00000000000026e0  00000000000026e0  000026e0  2**3
  4 .bss          00000010  0000000000002920  0000000000002920  00002920  2**3
  ...
```

Disassemble the `.text` section:

```sh
objdump -d -j .text hello | head -20
```

Output (abridged):

```sh
hello:     file format elf64-x86-64

Disassembly of section .text:

00000000000005b0 <_start>:
 5b0:   31 ed                   xor    %ebp,%ebp
 5b2:   49 89 d1                mov    %rdx,%r9
 ...
```

- `.text` contains executable code.
- `.data` contains initialized data.
- `.bss` contains uninitialized data.

---

Write a program that when running as a process, creates another process by "forking" (man fork) itself. Use the library function "sleep()" (man sleep) to put both processes to sleep for 30 seconds before they terminate. Check the memory usage of both processes and explain. You may find "getpid()" useful, see man getpid.

**[Lab Output & Explanation]**

C program (`fork_sleep.c`):

```c
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return 1;
    }
    if (pid == 0) {
        // Child process
        printf("[Child] PID: %d, sleeping 30s...\n", getpid());
        sleep(30);
        printf("[Child] PID: %d, exiting.\n", getpid());
    } else {
        // Parent process
        printf("[Parent] PID: %d, child PID: %d, sleeping 30s...\n", getpid(), pid);
        sleep(30);
        printf("[Parent] PID: %d, exiting.\n", getpid());
        wait(NULL);
    }
    return 0;
}
```

Compile and run in background:

```sh
gcc -o fork_sleep fork_sleep.c
./fork_sleep & echo $!
```

Output:

```sh
[1] 35526
35526
```

Check memory usage of both parent and child while sleeping:

```sh
ps -o pid,ppid,comm,rss,vsz,user -p 35526
```

Output:

```sh
  PID  PPID COMM            RSS      VSZ USER
35526 35322 ./fork_sleep   1168 410088160 jankott
```

Find child PID:

```sh
pgrep -P 35526
```

Output:

```sh
35556
```

Check child memory usage:

```sh
ps -o pid,ppid,comm,rss,vsz,user -p 35556
```

Output:

```sh
  PID  PPID COMM            RSS      VSZ USER
35556 35526 ./fork_sleep    896 410088160 jankott
```

Wait for both to finish:

```sh
wait
[Child] PID: 35556, exiting.
[Parent] PID: 35526, exiting.
[1]  + done       ./fork_sleep
```

**Explanation:**

- Both parent and child processes run the same code after `fork()`, but with different PIDs.
- Both sleep for 30 seconds, so both are present in the process table during that time.
- `RSS` (resident set size) is the actual memory used in RAM; `VSZ` (virtual size) is the total virtual memory allocated.
- Immediately after fork, both parent and child have nearly identical VSZ. RSS can differ slightly due to timing, process state, or activity: the parent may have touched more memory pages (e.g., due to running more code after fork), while the child may exit quickly and touch fewer pages.
- After 30 seconds, both processes print their exit messages and terminate.

### Subtask 1.4 - Zombie

Develop a program that creates a "Zombie Process". Technically, when a child process terminates, the process's parent is notified and supposed to execute the wait() (or waitpid()) system call to read the dead process's exit status and other information. After wait() is called, the zombie process is completely removed from memory by the OS.

**[Lab Output & Explanation]**

- Source code (`zombie.c`):

  ```c
  #include <stdio.h>
  #include <stdlib.h>
  #include <sys/types.h>
  #include <sys/wait.h>
  #include <unistd.h>

  int main() {
      pid_t pid = fork();
      if (pid < 0) {
          perror("fork failed");
          exit(1);
      }
      if (pid == 0) {
          printf("[Child] PID: %d exiting immediately.\n", getpid());
          exit(0);
      } else {
          printf("[Parent] PID: %d, child PID: %d. Sleeping 30s, not calling wait().\n", getpid(), pid);
          sleep(30);
          printf("[Parent] PID: %d exiting.\n", getpid());
      }
      return 0;
  }
  ```

- Compile and run in background:

  ```sh
  gcc -o zombie zombie.c
  ./zombie & echo $!
  ```

  Output:

  ```
  [1] 36245
  36245
  ```

- While the parent is sleeping, check for zombie (defunct) child:

  ```sh
  ps -l | grep 36245
  ```

  Output (abridged):

  ```
    501 36245 35894 ... SN   ./zombie
    501 36275 36245 ... ZN   <defunct>
  ```

  - The parent process (`./zombie`) is sleeping.
  - The child process (`<defunct>`) is a zombie: `STAT` column shows `Z` (zombie).

- You can also filter for all zombies:

  ```sh
  ps -l | grep defunct
  ```

  Output:

  ```
    501 36275 36245 ... ZN   <defunct>
  ```

  - The `<defunct>` entry confirms the presence of a zombie process.

- After the parent exits (or is killed), the zombie disappears:

  ```sh
  kill 36245
  ps -l | grep defunct
  ```

  Output:

  ```
  (no output)
  ```

**Explanation:**

- When the child exits, but the parent does not call `wait()`, the child becomes a zombie: its entry remains in the process table so the parent can read its exit status.
- The zombie is visible as `<defunct>` with `STAT` `Z` in `ps`.
- Once the parent exits, the zombie is reaped by `init` (PID 1) and disappears from the process table.
- **Problem:** If the parent process stays alive and never calls `wait()`, zombie processes accumulate and consume entries in the process table, potentially exhausting system resources and preventing new processes from being created. This is a resource leak and a bug in long-running programs. If the parent exits, the OS cleans up the zombie automatically, so the problem only occurs with long-lived parents that neglect to reap their children.

### Subtask 1.5 - Zombie or no Zombie

Modify your program such that the parent terminates before the child and explain what happens.

**[Lab Output & Explanation]**

- Source code (`nozombie.c`):

  ```c
  #include <stdio.h>
  #include <stdlib.h>
  #include <sys/types.h>
  #include <unistd.h>

  int main() {
      pid_t pid = fork();
      if (pid < 0) {
          perror("fork failed");
          exit(1);
      }
      if (pid == 0) {
          // Child process: sleep longer than parent
          printf("[Child] PID: %d, parent PID: %d. Sleeping 30s...\n", getpid(), getppid());
          sleep(30);
          printf("[Child] PID: %d, parent PID: %d. Exiting.\n", getpid(), getppid());
      } else {
          // Parent process: exit immediately
          printf("[Parent] PID: %d, child PID: %d. Exiting immediately.\n", getpid(), pid);
          exit(0);
      }
      return 0;
  }
  ```

- Compile and run in background:

  ```sh
  gcc -o nozombie nozombie.c
  ./nozombie & echo $!
  ```

  Output:

  ```sh
  [1] 37001
  37001
  [Parent] PID: 37001, child PID: 37031. Exiting immediately.
  [Child] PID: 37031, parent PID: 37001. Sleeping 30s...
  ```

- While the child is sleeping, check for zombie (defunct) processes:

  ```sh
  ps -l | grep 37031
  ```

  Output:

  ```sh
      501 37031 1 ... SN   ./nozombie
  ```

  - The child process is running (state `S` = sleeping), and its parent PID is now `1` (init/systemd), because the original parent exited.
  - There is **no** `<defunct>` (zombie) process.

- Check for all zombies:

  ```sh
  ps -l | grep defunct
  ```

  Output:

  ```sh
  (no output)
  ```

- Wait for the child to finish:

  ```sh
  # Wait ~30s, then:
  [Child] PID: 37031, parent PID: 1. Exiting.
  [1]  + done       ./nozombie
  ```

**Explanation:**

- When the parent process exits before the child, the child is immediately adopted by the `init` process (PID 1, usually `systemd`).
- The `init` process automatically calls `wait()` for any orphaned child processes when they terminate, so no zombie is left behind.
- You can see the child's new parent PID is 1 while it is running.
- **Key point:**
  - If a parent process does not call `wait()` but remains alive, its dead children become zombies (see previous subtask).
  - If the parent exits before the child, the child is reparented to `init`, which always reaps children, so no zombie remains.
- This is why long-lived parents must call `wait()` for their children, but short-lived parents that exit quickly do not cause zombies.

## Task 2 - Multi-Threading

Study the following code and explain what happens, step-by-step. How can you (developer) define what should be shared between parent and child?

```c
#define STACK_SIZE (1024*1024) /* Stack size for cloned child */

static int childFunc(void *arg) {
    //do something
    return 0;
}

int main(int argc, char *argv[]) {
    char *stack;
    char *stackTop;
    pid_t pid;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <child-hostname>\n", argv[0]);
        exit(EXIT_SUCCESS);
    }

    stack = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);

    stackTop = stack + STACK_SIZE;

    pid = clone(childFunc, stackTop, CLONE_NEWUTS | SIGCHLD, argv[1]);

    if (waitpid(pid, NULL, 0) == -1) exit();

    printf("child has terminated\n");
    exit(EXIT_SUCCESS);
}
```

**[Lab Output & Explanation]**

### Step-by-step Explanation

1. **Stack Allocation**

   - `stack = mmap(...)` allocates a new memory region of size `STACK_SIZE` (1 MiB) for the child stack. This is required because `clone()` (unlike `fork()`) expects the caller to provide a stack for the child.
   - `MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK` ensures a private, anonymous, stack-appropriate region.

2. **Stack Pointer Setup**

   - `stackTop = stack + STACK_SIZE;` sets the stack pointer to the top of the allocated region (stacks grow downward on x86/Linux).

3. **clone() System Call**

   - `pid = clone(childFunc, stackTop, CLONE_NEWUTS | SIGCHLD, argv[1]);`
     - `childFunc` is the function the child will execute (like a thread entry point).
     - `stackTop` is the initial stack pointer for the child.
     - `CLONE_NEWUTS` creates a new UTS namespace for the child (separate hostname/domainname from the parent).
     - `SIGCHLD` means the parent will receive a SIGCHLD when the child terminates (like `fork()`).
     - `argv[1]` is passed as the argument to `childFunc`.
   - The child starts running `childFunc(argv[1])` on its own stack.
   - The parent continues after `clone()` with the returned child PID.

4. **waitpid()**
   - The parent waits for the child to terminate using `waitpid(pid, NULL, 0);`.
   - After the child exits, the parent prints "child has terminated" and exits.

### What is shared between parent and child?

- By default, with only `SIGCHLD` (and not `CLONE_VM`, `CLONE_FS`, etc.), the child is a separate process, similar to `fork()`, but with a new UTS namespace (due to `CLONE_NEWUTS`).
- **Not shared:**
  - Address space (memory)
  - File descriptors
  - Working directory
  - Signal handlers
- **Shared:**
  - Nothing, except the UTS namespace is new for the child (not shared with parent).

### How can a developer define what is shared?

- The `clone()` system call allows fine-grained control over what is shared between parent and child via its flags:

| Flag          | What is shared?                                |
| ------------- | ---------------------------------------------- |
| CLONE_VM      | Memory space (address space)                   |
| CLONE_FS      | File system info (cwd, root, umask)            |
| CLONE_FILES   | File descriptors                               |
| CLONE_SIGHAND | Signal handlers                                |
| CLONE_THREAD  | Thread group (becomes a thread, not a process) |
| CLONE_NEWUTS  | New UTS namespace (hostname/domain)            |
| CLONE_NEWNS   | New mount namespace                            |
| CLONE_SYSVSEM | System V SEM_UNDO semantics                    |
| CLONE_PARENT  | Parent process                                 |
| CLONE_PTRACE  | ptrace behavior                                |
| ...           | ...                                            |

- By combining these flags, a developer can create either a process (like `fork()`), a thread (like `pthread_create()`), or something in between (e.g., a process with shared file descriptors but separate memory).
- For example, `CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD` is what `pthread_create()` uses to create threads in the same process.

### Summary Table

| clone() Flags  | Resulting Relationship         |
| -------------- | ------------------------------ |
| Only SIGCHLD   | Like fork() (separate process) |
| + CLONE_VM     | Shared memory                  |
| + CLONE_THREAD | Thread (shared everything)     |
| + CLONE_NEWUTS | New UTS namespace (child only) |

### Example: What happens in this code?

- The child runs `childFunc(argv[1])` in a new UTS namespace (can set its own hostname).
- The parent waits for the child to finish.
- No memory, file descriptors, or signal handlers are shared.
- This is a minimal example of using `clone()` to create a process with a new namespace, which is the basis for containers (e.g., Docker, LXC).

**Exam tip:**

- `clone()` is the foundation for both threads and containers in Linux. The choice of flags determines the degree of sharing and isolation.
- For threads: use all sharing flags. For containers: use namespace flags for isolation.

## Task 3 - Kernel Threads

Which process has PID #2?

**[Lab Output & Explanation]**

```sh
ps -p 2 -o pid,comm,user,state
```

Output:

```sh
  PID COMMAND         USER     S
    2 kthreadd        root     S
```

- PID 2 is always the kernel thread daemon `kthreadd` on Linux systems.
- It is the ancestor of all kernel threads and is owned by `root`.

---

Identify any Kernel Thread and find out on which CPU it is running. Recall, the /proc directory is providing information about running processes of all kinds.

**[Lab Output & Explanation]**

- To list kernel threads, filter for known kernel thread names (e.g., kworker, rcu) using `ps` and `grep`:

  ```sh
  ps -e -o pid,comm,psr,state | grep kworker | cat
  ```

  Output (abridged):

  ```sh
     PID COMMAND         PSR STATE
       6 kworker/0:0H-kb   0 I
      20 kworker/1:0H-kb   1 I
     112 kworker/u5:0      0 I
     335 kworker/0:1H-kb   0 I
     371 kworker/1:1H-kb   1 I
   17940 kworker/1:2-eve   1 I
   18105 kworker/u4:0-fl   0 I
   24173 kworker/u4:1-ev   0 I
   24791 kworker/0:0-eve   0 I
   25539 kworker/1:0-eve   1 I
   25618 kworker/0:1-cgr   0 I
   25850 kworker/1:1-cgr   1 I
   25851 kworker/1:3       1 I
   26030 kworker/0:2       0 I
  ```

  - The `PSR` column shows the CPU the thread is (or was last) running on. For example, `kworker/0:0H-kb` (PID 6) is on CPU 0.
  - The `I` state means idle (kernel thread is not currently running any work).

- You can also filter for other kernel threads, e.g., rcu:

  ```sh
  ps -e -o pid,comm,psr,state | grep rcu | cat
  ```

  Output:

  ```sh
        3 rcu_gp            0 I
        4 rcu_par_gp        0 I
       11 rcu_sched         0 I
       23 rcu_tasks_kthre   1 S
  ```

**How filtering works:**

- The `-e` option shows all processes, including kernel threads.
- The `-o` option customizes the columns (PID, command, CPU, state).
- Filtering for kernel threads is done by `grep` for known kernel thread names (e.g., `kworker`, `rcu`).
- On this system, kernel thread names do **not** appear in square brackets in the `comm` field, so filter by name instead.

**Summary Table**

| Thread Name     | PID | State | CPU | Explanation                  |
| --------------- | --- | ----- | --- | ---------------------------- |
| kworker/0:0H-kb | 6   | I     | 0   | Kernel worker, idle on CPU 0 |
| kworker/1:0H-kb | 20  | I     | 1   | Kernel worker, idle on CPU 1 |
| rcu_gp          | 3   | I     | 0   | RCU grace period thread      |
| rcu_sched       | 11  | I     | 0   | RCU scheduler thread         |

**Exam tip:**

- Use `ps -e -o pid,comm,psr,state | grep <name>` to see kernel threads and their CPU.
- State `I` = idle, `S` = sleeping, `R` = running, `D` = uninterruptible sleep.
