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

---

Move job number 2 to foreground and press Ctrl + z. List again the job list.

---

What is the difference now?

---

Return each job to foreground and terminate it $(Ctrl+c)$

### Subtask 1.3 - Process Creation

Write a Hello World in c and create a process that executes your program.

---

Revisit the layout of a binary and print the sizes of sections "Text", "Data" and "BSS".

---

Use objdump and display all section headers. Revisit the sections above and study the elements of each section. Disassemble the "Text" section.

---

Write a program that when running as a process, creates another process by "forking" (man fork) itself. Use the library function "sleep()" (man sleep) to put both processes to sleep for 30 seconds before they terminate. Check the memory usage of both processes and explain. You may find "getpid()" useful, see man getpid.

### Subtask 1.4 - Zombie

Develop a program that creates a "Zombie Process". Technically, when a child process terminates, the process's parent is notified and supposed to execute the wait() (or waitpid()) system call to read the dead process's exit status and other information. After wait() is called, the zombie process is completely removed from memory by the OS.

### Subtask 1.5 - Zombie or no Zombie

Modify your program such that the parent terminates before the child and explain what happens.

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

## Task 3 - Kernel Threads

Which process has PID #2?

---

Identify any Kernel Thread and find out on which CPU it is running. Recall, the /proc directory is providing information about running processes of all kinds.

---

Select any thread called "Kworker" and explain the state by querying it with the ps command.
