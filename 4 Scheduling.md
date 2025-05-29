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

### Subtask 1.2 – Rate Monotonic Schedule

WCET stands for Worst Case Execution Time. It represents the maximum possible runtime of a piece of code or a process/thread independently of the runtime environment and any scheduling factors. WCET generally includes wait times for I/O. It is used in real-time systems and embedded systems where I/O access times tend to be deterministic i.e completed in constant-time. I/O may include disk access times but rarely depends on I/O from a (non-deterministic) user.

Given the following task table, answer the following questions and complete the exercise for a Rate Monotonic Scheduler

| Task | WCET (C) | Period (T) |
| ---- | -------- | ---------- |
| T1   | 10       | 20         |
| T2   | 10       | 50         |
| T3   | 5        | 30         |

Which task has the highest priority?

---

Is there a guaranteed schedule?

---

Determine the schedule
