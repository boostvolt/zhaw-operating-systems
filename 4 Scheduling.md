# 4 Scheduling

## Introduction & Prerequisites

This laboratory is to learn how to:

- Create some basic schedules

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

## Task 1 - Basic Scheduling Theory

### Subtask 1.1 – FIFO and RR Schedules

Burst Time is an expression for the runtime of a task representing the computing time and not including any wait times for I/O which may increase the actual completion time of the task.

Given the following task list, determine the FIFO and RR schedules. Assume a quantum (q) of 2.

| Task | Arrival Time | Burst Time |
| ---- | ------------ | ---------- |
| T1   | 0            | 10         |
| T2   | 3            | 6          |
| T3   | 7            | 1          |
| T4   | 8            | 3          |

**[Lab Output & Explanation]**

#### FIFO (First-In-First-Out) Schedule

- Tasks are scheduled in order of arrival.
- Each task runs to completion before the next starts.

| Time | Running Task | Notes                |
| ---- | ------------ | -------------------- |
| 0    | T1           | T1 arrives, starts   |
| 10   | T2           | T1 finishes, T2 next |
| 16   | T3           | T2 finishes, T3 next |
| 17   | T4           | T3 finishes, T4 next |
| 20   | -            | T4 finishes          |

**Timeline:**

```
0        10      16        17      20
|---T1---|---T2---|---T3---|---T4---|
```

- T1 runs from 0–10 (T2, T3, T4 have not yet arrived).
- T2 starts at 10 (arrived at 3), runs to 16.
- T3 starts at 16 (arrived at 7), runs to 17.
- T4 starts at 17 (arrived at 8), runs to 20.

---

#### RR (Round Robin) Schedule (Quantum q = 2)

- Each task gets up to 2 units per turn.
- If not finished, it goes to the back of the queue.

| Time | Queue          | Running | Remaining Burst Times (T1/T2/T3/T4) |
| ---- | -------------- | ------- | ----------------------------------- |
| 0    | T1             | T1      | 10/6/1/3                            |
| 2    | T1, T2         | T1      | 8/6/1/3                             |
| 4    | T2, T1         | T2      | 8/4/1/3                             |
| 6    | T1, T2         | T1      | 6/4/1/3                             |
| 8    | T2, T1, T3, T4 | T2      | 6/2/1/3                             |
| 10   | T1, T3, T4, T2 | T1      | 4/2/1/3                             |
| 12   | T3, T4, T2, T1 | T3      | 4/2/0/3 (T3 finishes)               |
| 13   | T4, T2, T1     | T4      | 4/2/0/1                             |
| 15   | T2, T1, T4     | T2      | 4/0/0/1 (T2 finishes)               |
| 17   | T1, T4         | T1      | 2/0/0/1                             |
| 19   | T4, T1         | T4      | 2/0/0/0 (T4 finishes)               |
| 20   | T1             | T1      | 0/0/0/0 (T1 finishes)               |

**Timeline:**

```
0        2        4        6        8       10       12       13       15       17       19       20
|---T1---|---T2---|---T1---|---T2---|---T1---|---T3---|---T4---|---T2---|---T1---|---T4---|---T1---|
```

- Tasks are added to the queue as they arrive.
- Each gets 2 units per turn, or less if they finish sooner.
- The order changes as new tasks arrive and as tasks finish.

### Subtask 1.2 – Rate Monotonic Schedule

WCET stands for Worst Case Execution Time. It represents the maximum possible runtime of a piece of code or a process/thread independently of the runtime environment and any scheduling factors. WCET generally includes wait times for I/O. It is used in real-time systems and embedded systems where I/O access times tend to be deterministic i.e completed in constant-time. I/O may include disk access times but rarely depends on I/O from a (non-deterministic) user.

Given the following task table, answer the following questions and complete the exercise for a Rate Monotonic Scheduler

| Task | WCET (C) | Period (T) |
| ---- | -------- | ---------- |
| T1   | 10       | 20         |
| T2   | 10       | 50         |
| T3   | 5        | 30         |

**[Lab Output & Explanation]**

#### Which task has the highest priority?

- In Rate Monotonic Scheduling (RMS), the task with the shortest period has the highest priority.
- **T1** (Period 20) has the highest priority.

#### Is there a guaranteed schedule?

- RMS guarantees schedulability if the total utilization $U \leq n(2^{1/n} - 1)$ for n tasks.
- Utilization: $U = \frac{10}{20} + \frac{10}{50} + \frac{5}{30} = 0.5 + 0.2 + 0.1667 = 0.8667$
- For 3 tasks: $3(2^{1/3} - 1) \approx 0.7797$
- $0.8667 > 0.7797$ → **No guaranteed schedule**.

#### How is the schedule order determined?

Rate Monotonic Scheduling (RMS) Principles:

- **Priority Assignment:** Tasks with shorter periods have higher priority.
  - T1 (Period 20) > T3 (Period 30) > T2 (Period 50)
- **Preemption:** A higher-priority task will preempt a lower-priority one as soon as it is released (at the start of its period).

Step-by-Step Schedule Construction (First 50 units):

| Time  | Task Running | Reason (Priority)        |
| ----- | ------------ | ------------------------ |
| 0–10  | T1           | Highest (Period 20)      |
| 10–15 | T3           | Next highest (Period 30) |
| 15–20 | Idle/T2\*    | No high-priority ready   |
| 20–30 | T1           | T1 released again        |
| 30–35 | T3           | T3 released again        |
| 35–40 | Idle/T2\*    | No high-priority ready   |
| 40–50 | T1           | T1 released again        |
| 50    | T2           | T2 finally runs          |

_If T2 starts in an idle slot, it will be preempted by T1 or T3 as soon as they are released._

At time 50, all three tasks (T1, T2, T3) are released:

- T1: period 20, released at 0, 20, 40, 60, ...
- T3: period 30, released at 0, 30, 60, ...
- T2: period 50, released at 0, 50, 100, ...
