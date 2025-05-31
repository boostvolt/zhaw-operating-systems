# 5 Resource Control

## Introduction & Prerequisites

This laboratory is to learn how to:

- Find and understand the subsystems managed by the kernel
- Identify cgroups hierarchies created by systemd
- Create cgroups and associate processes to them

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

### Task 1 - Understand Cgroups Version One

#### Subtask 1.1 - Default System Configuration

Analyze the default cgroup configuration of Ubuntu. Which subsystems are supported by the Ubuntu kernel? Explain the output.

**[Lab Output & Explanation]**

Command:

```sh
cat /proc/cgroups
```

Output (abridged):

```
#subsys_name    hierarchy       num_cgroups     enabled
cpuset          11              1               1
cpu             3               92              1
cpuacct         3               92              1
blkio           2               92              1
memory          9               177             1
... (see full output above)
```

- The Ubuntu kernel supports the following cgroup subsystems: cpuset, cpu, cpuacct, blkio, memory, devices, freezer, net_cls, perf_event, net_prio, hugetlb, pids, rdma. 'enabled' means the controller is active. 'num_cgroups' shows how many cgroups exist for each controller.

---

Which hierarchies are provided by default? Which subsystems are configured at which hierarchy?

**[Lab Output & Explanation]**

Command:

```sh
mount | grep cgroup
```

Output (abridged):

```
cgroup2 on /sys/fs/cgroup/unified type cgroup2 ...
cgroup on /sys/fs/cgroup/systemd type cgroup ... name=systemd
cgroup on /sys/fs/cgroup/blkio type cgroup ... blkio
cgroup on /sys/fs/cgroup/cpu,cpuacct type cgroup ... cpu,cpuacct
cgroup on /sys/fs/cgroup/devices type cgroup ... devices
cgroup on /sys/fs/cgroup/perf_event type cgroup ... perf_event
cgroup on /sys/fs/cgroup/freezer type cgroup ... freezer
cgroup on /sys/fs/cgroup/net_cls,net_prio type cgroup ... net_cls,net_prio
cgroup on /sys/fs/cgroup/hugetlb type cgroup ... hugetlb
cgroup on /sys/fs/cgroup/memory type cgroup ... memory
cgroup on /sys/fs/cgroup/rdma type cgroup ... rdma
cgroup on /sys/fs/cgroup/cpuset type cgroup ... cpuset
cgroup on /sys/fs/cgroup/pids type cgroup ... pids
```

- Each cgroup controller is mounted in its own hierarchy under /sys/fs/cgroup. Some hierarchies combine multiple controllers (e.g., cpu,cpuacct).

---

Navigate to the default CPU hierarchy and check how many cgroups are present. What may be the purpose of these cgroups? Who created them? You may find the command `systemd-cgls` useful.

**[Lab Output & Explanation]**

Command:

```sh
systemd-cgls
```

Output (abridged):

```
Control group /:
-.slice
├─user.slice
│ └─user-1000.slice
│   ├─user@1000.service ...
├─init.scope
│ └─1 /sbin/init
└─system.slice
  ├─irqbalance.service ...
  ├─cron.service
  │ └─594 /usr/sbin/cron -f
  ...
```

- There are multiple cgroups under the root, mainly user.slice, system.slice, and init.scope. These are created and managed by systemd to organize system and user processes for resource control and accounting.

---

How many processes are in cgroup user.slice/system.slice/init.slice?

**[Lab Output & Explanation]**

Command:

```sh
systemd-cgls /user.slice   | grep -E '^[│ └─]*[0-9]+ ' | wc -l
systemd-cgls /system.slice | grep -E '^[│ └─]*[0-9]+ ' | wc -l
systemd-cgls /init.scope   | grep -E '^[│ └─]*[0-9]+ ' | wc -l
```

Output:

```
4
20
1
```

- There are 4 processes in user.slice, 20 in system.slice, and 1 in init.scope. This is determined by counting the process entries (lines with a PID) in each cgroup using `systemd-cgls` and filtering for process lines. This method is precise and exam-appropriate for reporting cgroup process membership.

---

Run the following commands and explain the output: `ps xawf -eo pid,user,cgroup,args`

**[Lab Output & Explanation]**

Command:

```sh
ps xawf -eo pid,user,cgroup,args | head -7
```

Output (abridged):

```
    PID USER     CGROUP                      COMMAND
      2 root     -                           [kthreadd]
      3 root     -                            \_ [rcu_gp]
      1 root     12:pids:/init.scope,9:memor /sbin/init
    351 root     12:pids:/system.slice/syste /lib/systemd/systemd-journald
    381 root     12:pids:/system.slice/syste /lib/systemd/systemd-udevd
```

- Kernel threads (e.g., kthreadd, rcu_gp) are not assigned to any cgroup (shown as '-').
- User processes (e.g., /sbin/init, systemd-journald) are assigned to specific cgroups, as shown in the CGROUP column (e.g., '12:pids:/init.scope,9:memory:/init.scope'). This means the process is managed by the pids and memory controllers in the 'init.scope' cgroup.

  **Note on the numbers (e.g., 12 and 9) in the CGROUP column:**

  - These numbers are the hierarchy IDs assigned by the kernel to each cgroup controller mount. You can see the mapping in `/proc/cgroups` (second column).
  - Example:
    | Number | Controller | Example Path | Meaning |
    |--------|------------|----------------|---------------------------------------|
    | 12 | pids | /init.scope | pids controller, cgroup /init.scope |
    | 9 | memory | /init.scope | memory controller, cgroup /init.scope |
  - This helps the kernel and tools map processes to the correct cgroup controller and path.

---

Check the configuration for a process called "cron" using "ps" and "grep". Explain both, the configuration for the "cron" and "ps" process, in terms of systemd and cgroup configuration.

**[Lab Output & Explanation]**

Command:

```sh
ps -ef | grep cron
```

Output:

```
root   594   1  0 May28 ? 00:00:00 /usr/sbin/cron -f
```

- The cron process (PID 594) is started by systemd (PPID 1). It is managed under system.slice/cron.service in the cgroup hierarchy.

Command:

```sh
ps -ef | grep ps
```

Output (abridged):

```
ubuntu 28845 28844 0 12:50 ? 00:00:00 ps -ef
```

- The 'ps' process is a user command, typically running under user.slice or a user session cgroup.

---

Verify your understanding by using the following commands: `systemd-cgtop` and `systemd-cgls`

**[Lab Output & Explanation]**

Command:

```sh
systemd-cgtop -n 1 -b | head -20
```

Output (abridged):

```
system.slice/cron.service 1 - 4.0M -
...
```

- systemd-cgtop shows live resource usage per cgroup. The cron service is visible under system.slice/cron.service, confirming its cgroup placement.

---

Can you identify the "cron" service?

**[Lab Output & Explanation]**

Command:

```sh
systemd-cgls | grep cron
```

Output:

```
├─cron.service
│ └─594 /usr/sbin/cron -f
```

- Yes, the cron service is clearly identified in the cgroup tree as system.slice/cron.service, with its process listed as a child.

#### Subtask 1.2 - Default System Configuration by Systemd

Obviously, systemd creates a number of cgroups.

Identify the respective unit files and explain their configuration.

**[Lab Output & Explanation]**

Systemd manages cgroups using special unit files called "slices" and "scopes". The main top-level slices are:

- `user.slice` (for user sessions)
- `system.slice` (for system services)
- `init.scope` (for the systemd process itself)

---

**user.slice**

Unit file:

```sh
cat /lib/systemd/system/user.slice
```

Output:

```
[Unit]
Description=User and Session Slice
Documentation=man:systemd.special(7)
Before=slices.target
```

Active configuration:

```sh
systemctl show user.slice
```

Output (abridged):

```
ControlGroup=/user.slice
MemoryCurrent=266432512
TasksCurrent=11
MemoryAccounting=yes
TasksAccounting=yes
Description=User and Session Slice
... (see full output above)
```

- `user.slice` is a static slice for all user sessions. It can be used to set resource limits (CPU, memory, tasks) for all user processes. By default, most resource controls are not set, but accounting is enabled for memory and tasks.

---

**system.slice**

Unit file: (no static file, managed internally by systemd)

Active configuration:

```sh
systemctl show system.slice
```

Output (abridged):

```
ControlGroup=/system.slice
MemoryCurrent=1183330304
TasksCurrent=58
MemoryAccounting=yes
TasksAccounting=yes
Description=System Slice
... (see full output above)
```

- `system.slice` is the parent for all system service units (e.g., cron, sshd). Like user.slice, it can be used to set resource limits for all system services collectively. Most limits are not set by default, but accounting is enabled.

---

**init.scope**

Unit file: (no static file, managed internally by systemd)

Active configuration:

```sh
systemctl show init.scope
```

Output (abridged):

```
ControlGroup=/init.scope
MemoryCurrent=5386240
TasksCurrent=1
MemoryAccounting=yes
TasksAccounting=yes
Description=System and Service Manager
... (see full output above)
```

- `init.scope` contains only the systemd process (PID 1). It is a special scope, not a slice, and is managed directly by systemd. Resource accounting is enabled, but no limits are set by default.

---

**Summary Table:**

| Unit         | Type  | Resource Controls (default) | Description                 |
| ------------ | ----- | --------------------------- | --------------------------- |
| user.slice   | slice | Accounting (memory, tasks)  | All user sessions           |
| system.slice | slice | Accounting (memory, tasks)  | All system services         |
| init.scope   | scope | Accounting (memory, tasks)  | The systemd process (PID 1) |

- You can override or extend these configurations by creating drop-in files in `/etc/systemd/system/<unit>.d/` or by using `systemctl set-property`.

**Slice vs Scope (systemd):**

- **Slice:**
  - A persistent cgroup tree node used to organize and apply resource controls to groups of services, user sessions, or other slices.
  - Examples: `user.slice`, `system.slice`, `machine.slice`.
  - Slices are typically defined statically and exist for the system's lifetime.
- **Scope:**
  - A transient cgroup created for externally managed processes (not started by systemd itself), such as user login sessions or single processes started by other services.
  - Examples: `session-2.scope`, `init.scope`.
  - Scopes are created dynamically and exist only as long as the managed process runs.

**Exam tip:**

- Use **slices** for persistent resource partitioning and **scopes** for tracking and controlling externally created processes.

---

Modify the user.slice configuration such that all processes created by users do not receive more than 20% of CPU share. Verify your configuration by creating more load than your system can handle, e.g. by creating a number of "`wc /dev/zero &`" background processes (& send the process directly into the background). What happened to the "cron" process?

**[Lab Output & Explanation]**

Command:

```sh
sudo systemctl set-property user.slice CPUQuota=20%
```

Output:

```
ubuntu@bsy-lab-2:~$ sudo systemctl set-property user.slice CPUQuota=20%
```

- This command sets a CPU quota of 20% for all processes in the user.slice cgroup. This means all user processes collectively cannot use more than 20% of total CPU time.

---

Command:

```sh
systemctl show user.slice | grep CPUQuota
```

Output:

```
CPUQuotaPerSecUSec=200ms
CPUQuotaPeriodUSec=infinity
DropInPaths=/etc/systemd/system.control/user.slice.d/50-CPUQuota.conf
```

- `CPUQuotaPerSecUSec=200ms` confirms the 20% CPU quota is active (20% of 1 second = 200ms). The configuration is stored as a drop-in file for user.slice.

---

Command:

```sh
for i in {1..8}; do nohup wc /dev/zero > /dev/null 2>&1 & done
```

Output (abridged):

```
[1] 31458
[2] 31459
[3] 31460
[4] 31461
[5] 31462
[6] 31463
[7] 31464
[8] 31465
```

- This starts 8 background processes, each running `wc /dev/zero` to generate CPU load. The PIDs are shown for each background job.

---

Command:

```sh
ps -C cron -o pid,ppid,cmd,%cpu
```

Output:

```
    PID    PPID CMD                         %CPU
    594       1 /usr/sbin/cron -f            0.0
```

- The cron process is unaffected by the user.slice CPU limit because it runs under `system.slice`, not `user.slice`. Its CPU usage remains at 0.0%.

---

Command:

```sh
ps -C wc -o pid,ppid,cmd,%cpu | head -10
```

Output (abridged):

```
    PID    PPID CMD                         %CPU
  31220   31218 wc /dev/zero                 2.4
  31221   31217 wc /dev/zero                 2.4
  31222   31216 wc /dev/zero                 2.4
  31223   31213 wc /dev/zero                 2.4
  31224   31212 wc /dev/zero                 2.4
  31225   31215 wc /dev/zero                 2.4
  31226   31211 wc /dev/zero                 2.4
  31227   31214 wc /dev/zero                 2.3
  31458   29249 wc /dev/zero                 1.2
```

- Each `wc` process is limited to a small fraction of CPU, confirming the 20% total CPU quota is enforced for all user.slice processes.

---

Command:

```sh
systemd-cgtop -n 1 -b | head -20
```

Output (abridged):

```
/                               151    -    1.5G    -   -
init.scope                        1    -    5.1M    -   -
system.slice                     48    -    1.0G    -   -
system.slice/cron.service         1    -    3.9M    -   -
... (other services omitted) ...
```

- `systemd-cgtop` shows that `system.slice` (where cron runs) is not affected by the user.slice CPU limit. The user.slice cgroup (not shown in this output) would be limited to 20% CPU, while system.slice and its services (like cron) are not restricted by this setting.

---

Free one `wc` process from the limits imposed by the user.slice related cgroup.

**[Lab Output & Explanation]**

To free a process from the user.slice CPU limit, you can launch it in a different slice (e.g., system.slice) using `systemd-run`:

Command:

```sh
sudo systemd-run --scope -p Slice=system.slice wc /dev/zero > /dev/null 2>&1 &
```

- This starts a new `wc /dev/zero` process in the `system.slice` cgroup, so it is not subject to the `user.slice` CPU quota.

Command:

```sh
ps -C wc -o pid,ppid,cmd,%cpu | head -10
```

Output (abridged):

```
    PID    PPID CMD                         %CPU
  32573   32572 /usr/bin/wc /dev/zero       85.4
... (other user.slice wc processes omitted) ...
```

- The process with high CPU usage (85.4%) is the one started in `system.slice` and is not limited by the 20% quota.

Command:

```sh
systemd-cgls | grep wc
```

Output (abridged):

```
│ └─32573 /usr/bin/wc /dev/zero
```

- This confirms the process is running in `system.slice` and is not affected by the user.slice limit.

### Task 2 - Create a Custom Control

Create an environment to control CPU access (CPU affinity) from scratch, i.e. not using any preconfigured cgroups. Create a tmpfs under `/mnt` and use this directory for your cgroup hierarchy.

**[Lab Output & Explanation]**

Command:

```sh
sudo mkdir -p /mnt/cpuset_cgroup && sudo mount -t tmpfs none /mnt/cpuset_cgroup
sudo mount -t cgroup -o cpuset cpuset /mnt/cpuset_cgroup
ls /mnt/cpuset_cgroup
```

Output (abridged):

```
cgroup.clone_children  cpuset.cpus  cpuset.mems  ...  tasks
```

- This creates a new cpuset cgroup hierarchy on a tmpfs under `/mnt`.

**Summary Table**

| Option | Meaning                                                               |
| ------ | --------------------------------------------------------------------- |
| -p     | Create parent directories as needed (for mkdir)                       |
| -t     | Specify filesystem type (for mount)                                   |
| -o     | Specify mount options (for mount, here selects the cpuset controller) |

---

How many CPUs can be controlled on your system per default?

**[Lab Output & Explanation]**

Command:

```sh
cat /mnt/cpuset_cgroup/cpuset.cpus
```

Output:

```
0-1
```

- There are 2 CPUs (CPU 0 and CPU 1) available for control on this system.

---

Create M processes, with M being the number of CPUs (N) on your system plus one (`M=N+1`). Those processes shall consume 100% CPU load. Now configure your cgroups to only allow these processes to use half of the CPUs available on your system. Mind the following prerequisite (command to be executed once the controller is mounted and the cgroup created): `echo 0 > cpuset.mems`

**[Lab Output & Explanation]**

Command:

```sh
sudo mkdir /mnt/cpuset_cgroup/half
# Set allowed CPUs and memory nodes for the new cgroup
echo 0 | sudo tee /mnt/cpuset_cgroup/half/cpuset.cpus
echo 0 | sudo tee /mnt/cpuset_cgroup/half/cpuset.mems
```

Output:

```
0
0
```

- This restricts the new cgroup to only CPU 0 and memory node 0.

Command:

```sh
for i in $(seq 1 3); do nohup bash -c "while :; do :; done" > /dev/null 2>&1 & done
pgrep -f "while :; do :; done"
```

Output (abridged):

```
33584
33585
33586
... (other PIDs omitted) ...
```

- This starts 3 busy-loop processes (M=3 for N=2 CPUs).

Command:

```sh
echo 33584 | sudo tee -a /mnt/cpuset_cgroup/half/tasks
echo 33585 | sudo tee -a /mnt/cpuset_cgroup/half/tasks
echo 33586 | sudo tee -a /mnt/cpuset_cgroup/half/tasks
```

Output:

```
33584
33585
33586
```

- These processes are now restricted to CPU 0 by the cpuset cgroup.

---

Verify and explain the effect by looking at the processes via "`top`". Also double-check the CPU assignments (affinity) with the command "`taskset`". Disable those CPUs, via "`chcpu`", that you allowed in your cpuset and explain what happens with those processes pinned onto them.

**[Lab Output & Explanation]**

Command:

```sh
top -b -n 1 | head -20
```

Output (abridged):

```
%Cpu(s):100.0 us,  0.0 sy,  0.0 ni,  0.0 id, ...
    PID USER      PR  NI    VIRT    RES    SHR S  %CPU  %MEM     TIME+ COMMAND
  33584 ubuntu    20   0    ...     R      42.3   ...   ...      bash
  33585 ubuntu    20   0    ...     R      42.3   ...   ...      bash
  33586 ubuntu    20   0    ...     R      42.3   ...   ...      bash
```

- The three processes are competing for CPU 0, each getting roughly a third of the available CPU time.

Command:

```sh
taskset -cp 33584 && taskset -cp 33585 && taskset -cp 33586
```

Output:

```
pid 33584's current affinity list: 0
pid 33585's current affinity list: 0
pid 33586's current affinity list: 0
```

- All three processes are pinned to CPU 0 as expected.

Command:

```sh
sudo chcpu -d 1
```

Output:

```
CPU 1 disabled
```

- Disabling CPU 1 does not affect the cpuset-restricted processes, since they are already pinned to CPU 0.

Command:

```sh
top -b -n 1 | head -20
```

Output (abridged):

```
%Cpu(s):100.0 us,  0.0 sy,  0.0 ni,  0.0 id, ...
33584 ubuntu    20   0    ...   R  42.3   ...   ...    bash
33585 ubuntu    20   0    ...   R  42.3   ...   ...    bash
33586 ubuntu    20   0    ...   R  42.3   ...   ...    bash
```

- The processes continue to run, still sharing CPU 0. If you tried to disable CPU 0, the system would not allow it (as it is not hot-pluggable), and the processes would be stuck if their only allowed CPU was offline.

Command:

```sh
sudo chcpu -d 0
```

Output:

```
chcpu: CPU 0 is not hot pluggable
```

- CPU 0 cannot be disabled, so the processes remain runnable. If you could disable all CPUs assigned to a cpuset, the processes would be un-runnable (stuck in D state), but this is prevented for CPU 0.
