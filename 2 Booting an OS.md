# 2 Booting an OS

## Introduction & Prerequisites

This laboratory is to learn how to:

- Find out systemd configuration and unit files with their locations
- Interact with systemctl
- Write service unit files

The following resources and tools are required for this laboratory session:

- Any modern web browser,
- Any modern SSH client application
- OpenStack Horizon dashboard: <https://ned.cloudlab.zhaw.ch>
- OpenStack account details: please contact the lab assistant in case you have not yet received your access credentials.
- Important access credentials:
  - Username to login with SSH into VMs in ned.cloudlab.zhaw.ch OpenStack cloud from your laptops: `ubuntu`

**Help Materials**

<https://www.digitalocean.com/community/tutorials/understanding-systemd-units-and-unit-files>

**Time**

The entire session will take 90 minutes.

**Assessment**

No assessment foreseen

## Task 1 - Setup & Basic Tasks

Login into the VM.

**Note:** by default systemd related commands are "paged". You can scroll up and down the output of commands with arrow keys, and you can exit by pressing 'q'. You can also disable paging by appending "--no-pager" to the command.

Useful commands for this lab are:

- `systemctl`
- `journalctl`

### Subtask 1.1 See unit status and grouping

- Use the command to see the status of all units in the system

  **[Lab Output & Explanation]**

  Command:

  ```sh
  systemctl --no-pager
  ```

  Output (abridged):

  ```sh
    UNIT                            LOAD   ACTIVE SUB       DESCRIPTION
    ...
    ssh.service                     loaded active running   OpenBSD Secure Shell server
    ...
    cron.service                    loaded active running   Regular background program processing daemon
    ...
  179 loaded units listed. Pass --all to see loaded but inactive units, too.
  To show all installed unit files use 'systemctl list-unit-files'.
  ```

  Explanation:

  - `systemctl` is the main tool to interact with systemd.
  - `--no-pager` disables paging, so output is not interactive.
  - This lists all systemd units (services, devices, mounts, etc.), their status, and descriptions.

- Compare the output of the previous command with the output of `pstree`. Can you find the "bash" process in both? What is the difference?

  **[Lab Output & Explanation]**

  Command:

  ```sh
  pstree -p
  ```

  Output (abridged):

  ```sh
  systemd(1)-+-accounts-daemon(692)
             |-cron(695)
             |-sshd(819)---sshd(1795)---sshd(1885)---bash(3176)---pstree(1886)
             ...
  ```

  Explanation:

  - `pstree` shows the process hierarchy (parent/child relationships).
  - `systemd` (PID 1) is the root of all processes.
  - You can find `bash` as a child of `sshd`, which is a child of `systemd`.
  - **Difference:**
    - `systemctl` shows systemd's view of units (not just processes).
    - `pstree` shows actual running processes and their relationships.

### Subtask 1.2 - Targets

- Use the command to list all available target units

  **[Lab Output & Explanation]**

  Command:

  ```sh
  systemctl list-units --type=target --all --no-pager
  ```

  Output (abridged):

  ```sh
    UNIT                  LOAD   ACTIVE   SUB    DESCRIPTION
    ...
    multi-user.target     loaded active   active Multi-User System
    graphical.target      loaded active   active Graphical Interface
    ...
  36 loaded units listed.
  ```

  Explanation:

  - Targets are special units that group other units (like runlevels in SysVinit).
  - Common targets: `multi-user.target`, `graphical.target`, `basic.target`.

- Find out which command to use to find the default target for the local operating system. What is the default target in the VM?

  **[Lab Output & Explanation]**

  Command:

  ```sh
  systemctl get-default
  ```

  Output:

  ```sh
  graphical.target
  ```

  Explanation:

  - This shows which target the system boots into by default.
  - `graphical.target` is typical for systems with a GUI; `multi-user.target` for servers.

- Which command can you use to see the unit file defining the default target and its location?

  **[Lab Output & Explanation]**

  Command:

  ```sh
  systemctl cat $(systemctl get-default)
  ```

  Output (abridged):

  ```ini
  # /lib/systemd/system/graphical.target
  [Unit]
  Description=Graphical Interface
  Documentation=man:systemd.special(7)
  Requires=multi-user.target
  Wants=display-manager.service
  Conflicts=rescue.service rescue.target
  After=multi-user.target rescue.service rescue.target display-manager.service
  AllowIsolate=yes
  ```

  Explanation:

  - `systemctl cat` shows the contents and location of a unit file.
  - The file is usually in `/lib/systemd/system/` or `/etc/systemd/system/`.

- Looking at that unit file, can you tell which service unit should be started together with that target? Is it required? What's the difference between '`requires`' and '`wants`'?

  **[Lab Output & Explanation]**

  - `Requires=multi-user.target` (hard dependency)
  - `Wants=display-manager.service` (soft dependency)
  - **Exam tip:**
    - `Requires` is a hard dependency, `Wants` is a soft dependency.

- There is a command that lets you see the "dependencies" of each unit.
- What is the command to see the dependencies of the '`sysinit.target`'?

  **[Lab Output & Explanation]**

  Command:

  ```sh
  systemctl list-dependencies sysinit.target
  ```

  Output (abridged):

  ```sh
  sysinit.target
  ● ├─apparmor.service
  ● ├─blk-availability.service
  ● ├─dev-hugepages.mount
  ● ├─systemd-journald.service
  ● ├─systemd-modules-load.service
  ● └─...
  ```

  Explanation:

  - Shows all units that are dependencies (required or wanted) for `sysinit.target`.

- What do the listed dependencies mean (i.e., are they required or wanted)?

  **[Lab Output & Explanation]**

  Command:

  ```sh
  systemctl cat sysinit.target | grep -E 'Requires=|Wants='
  ```

  Output:

  ```sh
  Wants=local-fs.target swap.target
  ```

  - The dependencies shown by `systemctl list-dependencies sysinit.target` are units that are either _wanted_ or _required_ by `sysinit.target`.
  - In this case, the only explicit dependencies are `Wants=local-fs.target swap.target`, meaning these are _soft_ dependencies (systemd will try to start them, but sysinit.target does not fail if they fail).
  - All other units shown in the dependency tree are pulled in transitively (e.g., by local-fs.target or swap.target, or by other units' Wants/Requires).
  - **Summary:** For `sysinit.target`, the direct dependencies are _wanted_ (not required). To know if a dependency is required or wanted, check the unit file for `Requires=` (hard) or `Wants=` (soft) relationships.

  **Exam tip:**

  - `Requires=` means the dependency is required (hard dependency, failure causes the target to fail).
  - `Wants=` means the dependency is wanted (soft dependency, failure does not cause the target to fail).

- Use the `man` command to see which `systemctl` command allows to see which units depend on (i.e., either want or require) a specific unit. Which target(s) depends on the `sshd.service`?

  **[Lab Output & Explanation]**

  Command to find the relevant systemctl option (from the man page):

  ```sh
  man systemctl
  ```

  Output (abridged):

  ```
           Show reverse dependencies between units with list-dependencies,
           i.e. follow dependencies of type WantedBy=, RequiredBy=, PartOf=,
           BoundBy=, instead of Wants= and similar.
  ```

  Command to see which units depend on sshd.service:

  ```sh
  systemctl list-dependencies --reverse sshd.service --no-pager
  ```

  Output:

  ```sh
  sshd.service
  ● ├─cloud-init.service
  ● └─multi-user.target
  ●   └─graphical.target
  ```

  Explanation:

  - `systemctl list-dependencies --reverse <unit>` shows which units have a dependency (WantedBy=, RequiredBy=, etc.) on the specified unit.
  - In this case, `multi-user.target` and `graphical.target` (and `cloud-init.service`) depend (directly or indirectly) on `sshd.service`.
  - This means that when the system reaches these targets, `sshd.service` is started as part of them.

## Task 2 - Services

### Subtask 2.1 - Inspect a service

- Let's check the status of the `ssh.service` with `systemctl`

  **[Lab Output & Explanation]**

  Command:

  ```sh
  systemctl status ssh.service --no-pager
  ```

  Output (abridged):

  ```sh
  ● ssh.service - OpenBSD Secure Shell server
       Loaded: loaded (/lib/systemd/system/ssh.service; enabled; vendor preset: enabled)
       Active: active (running) since ...
     Main PID: 819 (sshd)
        Tasks: 1 (limit: 9513)
       Memory: 5.2M
       CGroup: /system.slice/ssh.service
               └─819 sshd: /usr/sbin/sshd -D [listener] 0 of 10-100 startups
  ```

  Explanation:

  - Shows status, main PID, and other info.

- Can you tell from the status command the PID of the ssh process? Can you tell on which port it is running from the logs?

  **[Lab Output & Explanation]**

  - The `systemctl status ssh.service` command shows the **PID** of the main sshd process (e.g., `Main PID: 819`).
  - To see the port, check the initial logs:
    ```sh
    journalctl -u ssh.service --no-pager
    ```
    Output (abridged):
    ```
    May 28 11:33:49 bsy-lab-2 sshd[819]: Server listening on 0.0.0.0 port 22.
    May 28 11:33:49 bsy-lab-2 sshd[819]: Server listening on :: port 22.
    ```
  - This confirms that sshd is listening on port 22 (both IPv4 and IPv6).
  - The port is only shown in the **initial startup logs** of the service, not in every log entry.

- In order to see the full logs of the service we can use a specific command. How do you see the logs of the `ssh.service` unit?

  **[Lab Output & Explanation]**

  Command:

  ```sh
  journalctl -u ssh.service --no-pager
  ```

- What command can you use to see the unit-file of the `ssh.service`? Can you also see its location on the file system?

  **[Lab Output & Explanation]**

  Command:

  ```sh
  systemctl cat ssh.service
  ```

  Output (abridged):

  ```ini
  # /lib/systemd/system/ssh.service
  [Unit]
  Description=OpenBSD Secure Shell server
  Documentation=man:sshd(8) man:sshd_config(5)
  After=network.target auditd.service
  ConditionPathExists=!/etc/ssh/sshd_not_to_be_run

  [Service]
  EnvironmentFile=-/etc/default/ssh
  ExecStartPre=/usr/sbin/sshd -t
  ExecStart=/usr/sbin/sshd -D $SSHD_OPTS
  ExecReload=/usr/sbin/sshd -t
  ExecReload=/bin/kill -HUP $MAINPID
  KillMode=process
  Restart=on-failure
  RestartPreventExitStatus=255
  Type=notify
  RuntimeDirectory=sshd
  RuntimeDirectoryMode=0755

  [Install]
  WantedBy=multi-user.target
  Alias=sshd.service
  ```

  Explanation:

  - Shows the unit file, its location, and configuration for the service.

### Subtask 2.2 - Killing a service: Restart

- The proper way to stop a service unit is with the "`systemctl stop`" command. But let's try to kill a service process manually.

- Use the "`kill -SIGKILL <PID>`" command (e.g., "`sudo kill -9 676`") to kill the "`cron.service`".

  **[Lab Output & Explanation]**

  Command:

  ```sh
  sudo kill -9 695 && sleep 2 && systemctl status cron.service --no-pager
  ```

  Output (abridged):

  ```sh
  ● cron.service - Regular background program processing daemon
       Active: active (running) since ...
     Main PID: 2848 (cron)
  ```

  Explanation:

  - After SIGKILL, systemd restarts the service if `Restart=on-failure` is set.

- If you check the status of the `cron` service before and after killing the `cron` process, what do you see?

  **[Lab Output & Explanation]**

  - Before: Service is running with original PID.
  - After SIGKILL: Service is running with a new PID (restarted by systemd).

- Let's now try to use a different signal (TERM) to kill `cron` (without '`-9`'): `sudo kill <PID>`

  **[Lab Output & Explanation]**

  Command:

  ```sh
  sudo kill $(pidof cron) && sleep 2 && systemctl status cron.service --no-pager
  ```

  Output (abridged):

  ```sh
  ● cron.service - Regular background program processing daemon
       Active: inactive (dead) since ...
     Main PID: 2848 (code=killed, signal=TERM)
  ```

  Explanation:

  - With SIGTERM, the process exits cleanly. If `Restart=on-failure`, systemd does NOT restart it.

- What happens in this case?

  **[Lab Output & Explanation]**

  - The service is not restarted after a clean exit (SIGTERM) due to the `Restart=on-failure` policy.

- Look at the `cron.service` unit file with the `systemctl cat` command. What are the restarting conditions?

  **[Lab Output & Explanation]**

  Command:

  ```sh
  systemctl cat cron.service
  ```

  Output (abridged):

  ```ini
  # /lib/systemd/system/cron.service
  [Service]
  ...
  Restart=on-failure
  ```

  Background:

  - With `Restart=on-failure`, only abnormal exits (e.g., SIGKILL) cause a restart, not clean exits (SIGTERM).

  **Summary Table:**

  | Restart setting | What triggers a restart?                               |
  | --------------- | ------------------------------------------------------ |
  | no              | Never restarts                                         |
  | always          | Always restarts, regardless of exit reason             |
  | on-success      | Only restarts on clean exit                            |
  | on-failure      | Restarts on non-zero exit code or unclean signal       |
  | on-abnormal     | Restarts on abnormal (unclean) exit, not on clean/TERM |
  | on-abort        | Restarts on uncaught signal (e.g., SIGABRT, SIGKILL)   |
  | on-watchdog     | Restarts on watchdog timeout                           |

## Task 3 - Create a Service Unit

### Subtask 3.1 - Unit File

Use as a reference the simple service unit file provided here:

```ini
[Unit]
Description=simple systemd Hello World

[Service]
Type=simple
ExecStart=/usr/bin/dockerd
Restart=on-failure

[Install]
WantedBy=multi-user.target
```

Create a service that writes the file “/home/ubuntu/service started” each time it gets started.

Hints:

- In which directory should you write the service unit file? (See for instance where other unit
  files are saved using the appropriate systemctl command)
- What command can you use to create a new file in the service unit?
- Use the appropriate command to load and start your service.
- Check its status.
- Verify it performed the intended action.

**[Lab Output & Explanation]**

1. **Choose the directory for the service unit file:**

   - Most custom service unit files go in `/etc/systemd/system/`.

2. **Create the service unit file:**

   - Example: `/etc/systemd/system/writefile.service`
   - Content:

     ```ini
     [Unit]
     Description=Write a file on service start

     [Service]
     Type=oneshot
     ExecStart=/usr/bin/bash -c 'echo "Service started at $(date)" >> "/home/ubuntu/service started"'

     [Install]
     WantedBy=multi-user.target
     ```

   - Command to create the file (on the server):

     ```sh
     echo -e '[Unit]\nDescription=Write a file on service start\n\n[Service]\nType=oneshot\nExecStart=/usr/bin/bash -c '\''echo "Service started at $(date)" >> "/home/ubuntu/service started"'\''\n\n[Install]\nWantedBy=multi-user.target' | sudo tee /etc/systemd/system/writefile.service
     ```

   - **Note:** If the filename contains spaces, always quote the path (e.g., `"/home/ubuntu/service started"`). If not quoted, the shell will interpret the space as an argument separator, resulting in the wrong file being created (as verified in the lab).

3. **Reload systemd to recognize the new service:**

   ```sh
   sudo systemctl daemon-reload
   ```

4. **Start the service:**

   ```sh
   sudo systemctl start writefile.service
   ```

5. **Check the status of the service:**

   ```sh
   systemctl status writefile.service --no-pager
   ```

   Output (abridged):

   ```sh
   ● writefile.service - Write a file on service start
        Loaded: loaded (/etc/systemd/system/writefile.service; enabled; vendor preset: enabled)
        Active: inactive (dead) ...
   ```

   - Since `Type=oneshot`, the service runs the command and then exits.

6. **Verify the file was written:**
   ```sh
   cat "/home/ubuntu/service started"
   ```
   Output (example):
   ```sh
   Service started at Wed May 29 12:34:56 UTC 2025
   ```
   - Each time you start the service, a new line is appended.

**Summary:**

- The service unit file should be placed in `/etc/systemd/system/`.
- Use `Type=oneshot` for a one-time action.
- Use `ExecStart` to run the file-writing command. **Quote filenames with spaces.**
- Always reload systemd after adding a new unit file.
- Check the file to confirm the service worked as intended.
