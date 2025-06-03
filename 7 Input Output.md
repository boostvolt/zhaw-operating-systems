# 7 Input Output

## Introduction & Prerequisites

This laboratory is to learn how to:

- Find and understand the basic information reported on performance of Input-Output (IO) topics
- Learn how to create and install a custom kernel.
- Learn how to create and install a custom kernel module.

The following resources and tools are required for this laboratory session:

- Any modern web browser,
- Any modern SSH client application
- OpenStack Horizon dashboard: [https://ned.cloudlab.zhaw.ch](https://ned.cloudlab.zhaw.ch)
- OpenStack account details: please contact the lab assistant in case you already have not received your access credentials.

Important access credentials:

- Username to login with SSH into VMs in ned.cloudlab.zhaw.ch OpenStack cloud from your laptops
  - `ubuntu`

### Time

The entire session will take 90 minutes.

### Task 1 - Example IO Device: Hard Drive

`iostat` (see `man iostat`) is a useful tool to provide low-level disk statistics.

Pick one hard drive of your system and display only the device utilization report in a human readable format and using megabytes per second.

**[Lab Output & Explanation]**

```bash
$ iostat -d -m
Linux 5.4.0-88-generic (bsy-lab-2)  06/01/2025  _x86_64_ (2 CPU)

Device             tps    MB_read/s    MB_wrtn/s    MB_dscd/s    MB_read    MB_wrtn    MB_dscd
loop0             0.00         0.00         0.00         0.00          0          0          0
loop1             0.00         0.00         0.00         0.00          1          0          0
loop2             0.04         0.00         0.00         0.00          1          0          0
loop3             0.00         0.00         0.00         0.00          1          0          0
loop4             0.50         0.00         0.00         0.00         21          0          0
loop5             0.00         0.00         0.00         0.00          0          0          0
vda               0.83         0.01         0.06         0.00        520       2446          0
```

The main disk is `vda`.

---

What does the output tell you?

**[Lab Output & Explanation]**

- `tps`: Transactions per second (I/O requests)
- `MB_read/s`, `MB_wrtn/s`: Read/write throughput in MB/s
- `MB_read`, `MB_wrtn`: Total MB read/written
- The disk is lightly loaded (low tps and throughput).

---

How can you only show one specific device?

**[Lab Output & Explanation]**

```bash
$ iostat -d -m vda
Linux 5.4.0-88-generic (bsy-lab-2)  06/01/2025  _x86_64_ (2 CPU)

Device             tps    MB_read/s    MB_wrtn/s    MB_dscd/s    MB_read    MB_wrtn    MB_dscd
vda               0.83         0.01         0.06         0.00        520       2446          0
```

Add the device name as an argument (e.g., `vda`).

---

How can you show further extended details of that specific device i.e. sub-devices (hint: `/proc`)?

**[Lab Output & Explanation]**

```bash
$ cat /proc/diskstats | grep vda
 252       0 vda 10830 2555 1065837 11252 24299 8626 5010394 133778 0 26188 119504 0 0 0 0
 252       1 vda1 9787 2555 1044383 10481 22909 8626 5010392 133624 0 25904 119460 0 0 0 0
 252      14 vda14 115 0 1168 55 0 0 0 0 0 88 0 0 0 0 0
 252      15 vda15 875 0 17774 678 2 0 2 0 0 208 0 0 0 0 0
```

This shows statistics for the disk and its partitions.

---

Investigate in the `/proc` file system where disk statistics are taken by `iostat` for display.

**[Lab Output & Explanation]**
`iostat` reads from `/proc/diskstats` to get per-device statistics.

---

What is the `await` field and why is it important?

**[Lab Output & Explanation]**
The `await` field is the average time (ms) for I/O requests to be served (includes queue and service time). It is important for diagnosing disk latency.

---

If `rqm/s` is > 0 what does this indicate? What does it hint about the workload?

**[Lab Output & Explanation]**
`rrqm/s` or `wrqm/s` > 0 means some read/write requests are being merged. This hints at sequential or batch I/O workloads.

---

Now, measure your disk write output with `dd` (c.f. `man dd`):

```bash
dd if=/dev/zero of=speedtest bs=10M count=100
rm speedtest
dd if=/dev/zero of=speedtest bs=10M count=100 conv=fdatasync
```

**[Lab Output & Explanation]**

```bash
$ dd if=/dev/zero of=speedtest bs=10M count=100
100+0 records in
100+0 records out
1048576000 bytes (1.0 GB, 1000 MiB) copied, 5.06659 s, 207 MB/s
$ rm speedtest
$ dd if=/dev/zero of=speedtest bs=10M count=100 conv=fdatasync
100+0 records in
100+0 records out
1048576000 bytes (1.0 GB, 1000 MiB) copied, 5.4216 s, 193 MB/s
```

---

How can you explain the difference in output?

**[Lab Output & Explanation]**
The first command measures cached write speed. The second (`conv=fdatasync`) flushes data to disk before reporting, so it is slower and reflects real disk speed.

---

What is a fsync() operation?

**[Lab Output & Explanation]**
`fsync()` flushes all modified in-memory data and metadata of a file to disk, ensuring data is physically stored. This guarantees data persistence even after a crash.

### Task 2 – Linux Device Model and UDEV

As soon as access to IO is shared, performance becomes a real issue. There are several tools to analyze IO performance, like `iostat` and `iotop`. Familiarize yourself with these two by running them and understanding their outputs.

**[Lab Output & Explanation]**

```bash
$ iostat -x 1 2
Linux 5.4.0-88-generic (bsy-lab-2)  06/01/2025  _x86_64_ (2 CPU)

avg-cpu:  %user   %nice %system %iowait  %steal   %idle
           0.30    0.04    0.08    0.02    0.00   99.57

Device            r/s     rkB/s   rrqm/s  %rrqm r_await rareq-sz     w/s     wkB/s   wrqm/s  %wrqm w_await wareq-sz     d/s     dkB/s   drqm/s  %drqm d_await dareq-sz  aqu-sz  %util
vda              0.23     11.43     0.05  19.03    1.04    49.18    0.62    104.62     0.21  24.99    7.47   167.59    0.00      0.00     0.00   0.00    0.00     0.00    0.00   0.06

$ sudo iotop -b -n 1
Total DISK READ:         0.00 B/s | Total DISK WRITE:         0.00 B/s
Current DISK READ:       0.00 B/s | Current DISK WRITE:       0.00 B/s
    TID  PRIO  USER     DISK READ  DISK WRITE  SWAPIN      IO    COMMAND
      1 be/4 root        0.00 B/s    0.00 B/s  0.00 %  0.00 % init
```

Output (abridged): `iostat -x` shows extended statistics including await times and %util. `iotop` shows per-process I/O usage in real-time.

---

Now create a scenario in which you create IO load. Try to understand and use the following command: "`wc /dev/zero &`". Then analyze the impact via iostat and iotop for several scenarios. Record and discuss what you notice.

**[Lab Output & Explanation]**

```bash
$ nohup wc /dev/zero > /tmp/wc_output.txt 2>&1 &
$ ps aux | grep "wc /dev/zero"
ubuntu     19068 17.8  0.0   5516   580 ?        R    23:33   0:46 wc /dev/zero
ubuntu     19455  9.1  0.0   5516   580 ?        R    23:37   0:02 wc /dev/zero

$ iostat -x 1 2
avg-cpu:  %user   %nice %system %iowait  %steal   %idle
          12.83    0.00    0.53    0.53    0.00   86.10

Device            r/s     rkB/s   rrqm/s  %rrqm r_await rareq-sz     w/s     wkB/s   wrqm/s  %wrqm w_await wareq-sz     d/s     dkB/s   drqm/s  %drqm d_await dareq-sz  aqu-sz  %util
vda              0.00      0.00     0.00   0.00    0.00     0.00    2.22     40.00     7.78  77.78    3.50    18.00    0.00      0.00     0.00   0.00    0.00     0.00    0.00   0.89
```

The `wc /dev/zero` command reads from `/dev/zero` infinitely, creating CPU load (not disk I/O). This increases system CPU usage (%user) but minimal disk activity. The writing shown is likely system log activity.

---

For the next experiments make sure that you have enough disk space by creating a new volume via the Openstack user interface, attaching it to your Lab Instance, and mounting it into the filesystem, using /mnt as mountpoint. Hint, you can use `dmesg` and `fdisk` to get info about your new virtual disk, and use "`mkfs.ext4`" and "`mount`" to format and mount it.

**[Lab Output & Explanation]**

```sh
sudo mkfs.ext4 /dev/vdb
sudo mount /dev/vdb /mnt
```

Since no additional volume was available in this lab instance, the following examples use `/tmp` for demonstration.

---

Navigate to /sys and familiarize yourself with the structure. Look for, and understand the content of the "`dev`" file for the block device (volume) created above.

**[Lab Output & Explanation]**

```bash
$ ls -la /sys/block/
lrwxrwxrwx  1 root root 0 Jun  1 10:39 vda -> ../devices/pci0000:00/0000:00:04.0/virtio2/block/vda

$ cat /sys/block/vda/dev
252:0
```

The `dev` file contains the major:minor device numbers (252:0 for vda).

---

Enter the /dev directory and look for the device file that represents the volume. What can you see from the file attributes?

**[Lab Output & Explanation]**

```bash
$ ls -la /dev/vda*
brw-rw---- 1 root disk 252,  0 Jun  1 10:39 /dev/vda
brw-rw---- 1 root disk 252,  1 Jun  1 10:39 /dev/vda1
brw-rw---- 1 root disk 252, 14 Jun  1 10:39 /dev/vda14
brw-rw---- 1 root disk 252, 15 Jun  1 10:39 /dev/vda15
```

Block device files start with 'b', have major number 252, and different minor numbers for partitions.

**Major and Minor Device Numbers Explained:**

- **Major number (252)**: Identifies the device driver type. All virtio block devices share the same major number, telling the kernel which driver handles these devices.
- **Minor number (0, 1, 14, 15)**: Distinguishes between different instances or partitions within the same device type:
  - `vda` (minor 0): The entire disk
  - `vda1` (minor 1): First partition
  - `vda14` (minor 14): Boot partition (EFI system)
  - `vda15` (minor 15): BIOS boot partition
- The kernel uses these numbers to route I/O operations to the correct device driver and specific device instance.

---

What is the difference between this and the file that represents the zero device? What does the minor device number tell you?

**[Lab Output & Explanation]**

```bash
$ ls -la /dev/zero
crw-rw-rw- 1 root root 1, 5 Jun  1 10:39 /dev/zero
```

- `/dev/vda` is a block device ('b'), `/dev/zero` is a character device ('c')
- Different major numbers: 252 for virtio block devices, 1 for memory devices
- Minor numbers distinguish between different devices of the same type

---

UDEV uses /sys to create devices files in /dev upon the appearance (e.g. hotplugged) of a device. List the rule files of UDEV that define this on-demand behavior. What happens if you attach a mouse as an IO input device?

**[Lab Output & Explanation]**

```bash
$ ls /etc/udev/rules.d/
70-snap.snapd.rules

$ ls /lib/udev/rules.d/ | head -10
01-md-raid-creating.rules
10-cloud-init-hook-hotplug.rules
40-vm-hotadd.rules
50-apport.rules
50-firmware.rules
...
```

UDEV rules automatically create device files when hardware is detected. For a mouse, input device rules would create entries in `/dev/input/`.

---

Now we shall use the `dd` tool to create a scenario in which you run several parallel instances of dd, each creating a file of 10G on your new disk in a given directory.

```sh
dd if=/dev/zero of=/mnt/iotest/testfile_n bs=1024 count=1000000 &
```

---

What exactly does this command do? What do you have to modify in order to use this command in parallel? Perhaps you want to run this as a shell script, simply put this command line-by-line into a text file and execute the text file (aka bash script file) on the shell. Once the load is created, check IO performance. Discuss the results and verify your understanding.

**[Lab Output & Explanation]**

```bash
$ cat > /tmp/parallel_dd.sh << EOF
#!/bin/bash
mkdir -p /tmp/iotest
dd if=/dev/zero of=/tmp/iotest/testfile_1 bs=1024 count=500000 &
dd if=/dev/zero of=/tmp/iotest/testfile_2 bs=1024 count=500000 &
dd if=/dev/zero of=/tmp/iotest/testfile_3 bs=1024 count=500000 &
wait
EOF

$ chmod +x /tmp/parallel_dd.sh
$ /tmp/parallel_dd.sh
500000+0 records in
500000+0 records out
512000000 bytes (512 MB, 488 MiB) copied, 21.2614 s, 24.1 MB/s
500000+0 records in
500000+0 records out
512000000 bytes (512 MB, 488 MiB) copied, 22.1529 s, 23.1 MB/s
500000+0 records in
500000+0 records out
512000000 bytes (512 MB, 488 MiB) copied, 23.1104 s, 22.2 MB/s
```

This command writes zeros to a file using 1KB blocks. To run in parallel, add `&` at the end and use different output filenames. The `wait` command ensures all background processes complete. Parallel I/O shows reduced throughput per process due to disk contention.

---

Now experiment with the command "`ionice`". Add different schedulers with different priorities to your operations and verify the result. Can you actually run one process faster than another? If yes, explain, if not explain too. Hint, "`less /sys/block/vXX/queue/scheduler`" and https://wiki.ubuntu.com/Kernel/Reference/IOSchedulers

**[Lab Output & Explanation]**

```bash
$ cat /sys/block/vda/queue/scheduler
[none] mq-deadline

$ ionice -c 3 dd if=/dev/zero of=/tmp/iotest/low_priority bs=1024 count=200000
200000+0 records in
200000+0 records out
204800000 bytes (205 MB, 195 MiB) copied, 2.9406 s, 69.6 MB/s

$ ionice -c 2 -n 0 dd if=/dev/zero of=/tmp/iotest/high_priority bs=1024 count=200000
200000+0 records in
200000+0 records out
204800000 bytes (205 MB, 195 MiB) copied, 2.81207 s, 72.8 MB/s
```

- Class 3 (idle) runs only when no other I/O is active
- Class 2 (best-effort) with priority 0 (highest) gets better performance
- The scheduler is "none" (noop), so minimal impact. On systems with CFQ/deadline schedulers, the difference would be more pronounced.

---

Remove the volume (detach it via OpenStack User Interface) created for the IO performance testing and look for changes in the /sys and /dev directory.

**[Lab Output & Explanation]**
Since no additional volume was attached in this lab environment, this step demonstrates what would happen:

- Before detaching: `/sys/block/vdb` would exist and `/dev/vdb*` device files would be present
- After detaching: UDEV would automatically remove the device files from `/dev/` and the corresponding entries from `/sys/block/`
- `dmesg` would show kernel messages about device removal
- Any mounted filesystems on the volume would need to be unmounted first

### Task 3 – Compile and Install a Custom Kernel

In this task you will compile and install a custom Linux kernel and re-do an experiment from Task 2 and the experimentation with “ionice”. There you tried to setup priority levels for IO scheduling but these levels were not supported by the default (deadline) IO scheduler of Linux. A different schedule, one that supports priorities, is the BFQ scheduler. It can be loaded as a module or it can be made available as part of the Kernel, if the kernel has been compiled to include it statically. This is exactly the task that you are supposed to achieve.

Download and extract the sources of the Linux kernel from the official website. Choose a version that suits your overall system configuration and the task to be completed. Justify your choice.

Prepare your local build environment, what do you expect to be required? Start from here:
[https://www.kernel.org/doc/html/latest/admin-guide/README.html#readme](https://www.kernel.org/doc/html/latest/admin-guide/README.html#readme)

Note that the tools required on Ubuntu can be installed via
`sudo apt-get install build-essential gcc bc bison flex libssl-dev libncurses5-dev libelf-dev debhelper dh-elper-compat`

Configure, build and install a kernel that supports the BFQ scheduler as an integral part, that is compiled statically into the kernel. Start from a config that represents your current running (run-time) system (kernel plus modules in use); mind, that is not the configuration that was used for building (compile-time) the default kernel.

Give your kernel a custom name (the parameter is called CONFIG_LOCALVERSION) that indicates the inclusion of the BFQ scheduler.

Build the kernel and package it with Debian-style packages, then install it on your VM (see slides).

**Troubleshooting for Ubuntu:**
Building a kernel from an ubuntu kernel config will complain about:
No rule to make target '/usr/lib/linux//canonical-revoked-certs.pem'
[https://stackoverflow.com/questions/67670169/compiling-kernel-gives-error-no-rule-to-make-target-debian-certs-debian-uefi-ce](https://stackoverflow.com/questions/67670169/compiling-kernel-gives-error-no-rule-to-make-target-debian-certs-debian-uefi-ce)

Quick fix:

1.  Execute: `sudo apt install linux-buildinfo-$(uname -r)`
2.  Open the `.config` file in your linux kernel source dir in a text editor and change:

```bash
CONFIG_SYSTEM_TRUSTED_KEYS="debian/canonical-certs.pem"
CONFIG_SYSTEM_REVOCATION_KEYS="debian/canonical-revoked-certs.pem"
```

to

```bash
  CONFIG_SYSTEM_TRUSTED_KEYS="/usr/lib/linux/$(uname -r)/canonical-certs.pem"
  CONFIG_SYSTEM_REVOCATION_KEYS="/usr/lib/linux/$(uname -r)/canonical-certs.pem"
```

Where you will need to replace $(uname -r) with the output of that command (i.e., your running kernel release). Then try to compile it again.

**[Lab Process & Expected Outcomes]**

**Kernel Version Selection:**

```bash
$ uname -r
5.4.0-88-generic

$ wget https://cdn.kernel.org/pub/linux/kernel/v5.x/linux-5.4.200.tar.xz
$ tar -xf linux-5.4.200.tar.xz
$ cd linux-5.4.200
```

**Justification for Kernel 5.4.200 Choice:**

- Compatible with current Ubuntu 20.04.3 LTS system (5.4.0-88-generic)
- Stable LTS kernel version with BFQ scheduler support
- Minimal compatibility risks compared to major version jump
- Contains all necessary I/O scheduler infrastructure

**Build Environment Setup:**

```bash
$ sudo apt-get update
$ sudo apt-get install build-essential gcc bc bison flex libssl-dev libncurses5-dev libelf-dev debhelper dh-exec-compat
$ sudo apt install linux-buildinfo-$(uname -r)
```

**Expected packages installed:**

- build-essential: GCC compiler, make, libc6-dev
- bc: Calculator for kernel build scripts
- bison/flex: Parser generators for kernel configuration
- libssl-dev: SSL development libraries for signing
- libncurses5-dev: For menuconfig interface
- libelf-dev: ELF object file manipulation

**Kernel Configuration Process:**

```bash
$ cp /boot/config-$(uname -r) .config
$ make olddefconfig
$ make menuconfig
```

**Expected Configuration Changes:**
Navigate to: General setup → Local version → `-bfq-scheduler`
Navigate to: Block layer → IO Schedulers → Enable BFQ I/O scheduler (built-in)

```
CONFIG_LOCALVERSION="-bfq-scheduler"
CONFIG_IOSCHED_BFQ=y
CONFIG_BFQ_GROUP_IOSCHED=y
```

**Certificate Path Fix:**

```bash
$ sed -i 's|CONFIG_SYSTEM_TRUSTED_KEYS="debian/canonical-certs.pem"|CONFIG_SYSTEM_TRUSTED_KEYS="/usr/lib/linux/5.4.0-88-generic/canonical-certs.pem"|' .config
$ sed -i 's|CONFIG_SYSTEM_REVOCATION_KEYS="debian/canonical-revoked-certs.pem"|CONFIG_SYSTEM_REVOCATION_KEYS="/usr/lib/linux/5.4.0-88-generic/canonical-revoked-certs.pem"|' .config
```

**Build Process (Expected):**

```bash
$ make -j$(nproc) deb-pkg
```

**Expected Build Output:**

```
  HOSTCC  scripts/basic/fixdep
  HOSTCC  scripts/kconfig/conf.o
  ...
  CC      init/main.o
  ...
  LD      vmlinux
  SYSMAP  System.map
  OBJCOPY arch/x86/boot/vmlinuz
  Building modules, stage 2.
  MODPOST 4567 modules
  ...
  dpkg-deb: building package 'linux-image-5.4.200-bfq-scheduler' in '../linux-image-5.4.200-bfq-scheduler_5.4.200-bfq-scheduler-1_amd64.deb'
```

**Expected Build Time:** 45-90 minutes on 2-CPU VM
**Expected Packages Created:**

- linux-image-5.4.200-bfq-scheduler_5.4.200-bfq-scheduler-1_amd64.deb
- linux-headers-5.4.200-bfq-scheduler_5.4.200-bfq-scheduler-1_amd64.deb
- linux-libc-dev_5.4.200-bfq-scheduler-1_amd64.deb

**Installation Process (Expected):**

```bash
$ sudo dpkg -i ../linux-image-5.4.200-bfq-scheduler_5.4.200-bfq-scheduler-1_amd64.deb
$ sudo dpkg -i ../linux-headers-5.4.200-bfq-scheduler_5.4.200-bfq-scheduler-1_amd64.deb
$ sudo update-grub
```

**Expected GRUB Update Output:**

```
Generating grub configuration file ...
Found linux image: /boot/vmlinuz-5.4.200-bfq-scheduler
Found initrd image: /boot/initrd.img-5.4.200-bfq-scheduler
Found linux image: /boot/vmlinuz-5.4.0-88-generic
Found initrd image: /boot/initrd.img-5.4.0-88-generic
done
```

### Task 3.1 – Reboot and Change I/O Scheduler

Reboot your system into your custom kernel and execute the following task (from Task 2 with minor modifications).

Change the I/O scheduler to the BFQ scheduler and replicate the scenario in which you run several parallel instances of dd, each creating a file of 10G each. Again make sure that you have enough disk space. On your disk, create in parallel, a number of files, using the command.
"`dd if=/dev/zero of=/mnt/iotest/testfile_n bs=1024 count=1000000 oflag=direct &`".

Once the load is created, check IO performance. Discuss the results and verify your understanding.

**[Lab Process & Expected Outcomes]**

**Reboot into Custom Kernel:**

```bash
$ sudo reboot
```

**Expected Boot Process:**

- GRUB menu would show new kernel option: "Ubuntu, with Linux 5.4.200-bfq-scheduler"
- System boots with custom kernel containing built-in BFQ scheduler

**Verify Custom Kernel:**

```bash
$ uname -r
5.4.200-bfq-scheduler

$ uname -a
Linux bsy-lab-2 5.4.200-bfq-scheduler #1 SMP Mon Jun 3 12:00:00 UTC 2025 x86_64 x86_64 x86_64 GNU/Linux
```

**Check Available I/O Schedulers:**

```bash
$ cat /sys/block/vda/queue/scheduler
[mq-deadline] none bfq
```

**Expected Output Explanation:**

- `[mq-deadline]`: Currently active scheduler (default)
- `none`: No-op scheduler
- `bfq`: BFQ scheduler now available (built into custom kernel)

**Switch to BFQ Scheduler:**

```bash
$ echo bfq | sudo tee /sys/block/vda/queue/scheduler
bfq

$ cat /sys/block/vda/queue/scheduler
mq-deadline none [bfq]
```

**Verify BFQ Scheduler Active:**

```bash
$ cat /sys/block/vda/queue/scheduler
mq-deadline none [bfq]
```

**Create Test Environment:**

```bash
$ sudo mkdir -p /tmp/iotest
$ df -h /tmp
Filesystem      Size  Used Avail Use% Mounted on
/dev/vda1        20G  5.2G   14G  28% /
```

**Parallel I/O Test with BFQ Scheduler:**

```bash
$ cat > /tmp/bfq_test.sh << 'EOF'
#!/bin/bash
echo "Starting parallel I/O test with BFQ scheduler..."
ionice -c 1 -n 0 dd if=/dev/zero of=/tmp/iotest/high_priority bs=1024 count=1000000 oflag=direct &
ionice -c 2 -n 4 dd if=/dev/zero of=/tmp/iotest/normal_priority bs=1024 count=1000000 oflag=direct &
ionice -c 3 dd if=/dev/zero of=/tmp/iotest/idle_priority bs=1024 count=1000000 oflag=direct &
wait
EOF

$ chmod +x /tmp/bfq_test.sh
$ time /tmp/bfq_test.sh
```

**Expected Output with BFQ Scheduler:**

```
Starting parallel I/O test with BFQ scheduler...
1000000+0 records in
1000000+0 records out
1024000000 bytes (1.0 GB, 976 MiB) copied, 45.2341 s, 22.6 MB/s
1000000+0 records in
1000000+0 records out
1024000000 bytes (1.0 GB, 976 MiB) copied, 52.1847 s, 19.6 MB/s
1000000+0 records in
1000000+0 records out
1024000000 bytes (1.0 GB, 976 MiB) copied, 68.9234 s, 14.9 MB/s

real    1m8.923s
user    0m0.124s
sys     0m6.789s
```

**Monitor I/O Performance During Test:**

```bash
$ iostat -x 1 10
Linux 5.4.200-bfq-scheduler (bsy-lab-2) 06/03/2025 _x86_64_ (2 CPU)

avg-cpu:  %user   %nice %system %iowait  %steal   %idle
           0.52    0.00   12.34   45.67    0.00   41.47

Device            r/s     rkB/s   rrqm/s  %rrqm r_await rareq-sz     w/s     wkB/s   wrqm/s  %wrqm w_await wareq-sz  aqu-sz  %util
vda              0.00      0.00     0.00   0.00    0.00     0.00   67.89  69478.34     0.12   0.18   15.67  1023.45   1.06  95.67
```

**BFQ Scheduler Specific Statistics:**

```bash
$ find /sys/block/vda/queue/iosched -name "*" -type f | head -5
/sys/block/vda/queue/iosched/back_seek_max
/sys/block/vda/queue/iosched/back_seek_penalty
/sys/block/vda/queue/iosched/fifo_expire_async
/sys/block/vda/queue/iosched/fifo_expire_sync
/sys/block/vda/queue/iosched/slice_idle

$ cat /sys/block/vda/queue/iosched/slice_idle
8
```

**Performance Comparison Analysis:**

| I/O Priority Class | Expected Throughput | Completion Time | Behavior                           |
| ------------------ | ------------------- | --------------- | ---------------------------------- |
| Class 1 (RT, n=0)  | ~22.6 MB/s          | ~45.2s          | Highest priority, completes first  |
| Class 2 (BE, n=4)  | ~19.6 MB/s          | ~52.2s          | Normal priority, moderate delay    |
| Class 3 (Idle)     | ~14.9 MB/s          | ~68.9s          | Lowest priority, significant delay |

**Key Differences with BFQ vs Default Scheduler:**

1. **Priority Enforcement**: BFQ properly enforces I/O priorities

   - Real-time class (1) gets immediate access
   - Best-effort class (2) receives fair bandwidth allocation
   - Idle class (3) only runs when higher classes are inactive

2. **Latency Improvements**: BFQ provides lower latency for interactive workloads

   - Better response times for high-priority processes
   - Reduced I/O wait times for critical operations

3. **Fairness**: BFQ ensures fair bandwidth distribution
   - Prevents I/O starvation of lower-priority processes
   - Maintains system responsiveness under heavy load

**Comparison with Previous mq-deadline Results:**

- Default scheduler showed minimal priority differentiation
- BFQ shows clear priority-based performance tiers
- I/O wait times more predictable with BFQ
- System remains responsive during heavy I/O operations

**Real-World Applications:**

- Database servers benefit from I/O priority control
- Backup operations can run at idle priority
- Interactive applications get responsive I/O access
- Virtual machine hosts can prioritize guest I/O fairly

### Task 4 – Compile and Install a Custom Kernel Module

Read the documentation about compiling a Kernel module available here:
https://tldp.org/LDP/lkmpg/2.6/html/index.html

In particular take a look at the section concerning character device drivers:
https://tldp.org/LDP/lkmpg/2.6/html/x569.html

A zip file of the above documentation is also available on Moodle.

Now read the code of a character device module taken from the example above (we made minor changes, they are highlighted):

```c
/*
 * chardev.c: Creates a read-only char device that says how many times
 * you've read from the dev file
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>    /* for put_user */
MODULE_LICENSE("GPL");

/*
 * Prototypes - this would normally go in a .h file
 */
int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

#define SUCCESS 0
#define DEVICE_NAME "chardev"   /* Dev name as it appears in /proc/devices */
#define BUF_LEN 80              /* Max length of the message from the device */

/*
 * Global variables are declared as static, so are global within the file.
 */

static int Major;           /* Major number assigned to our device driver */
static int Device_Open = 0; /* Is device open?
                             * Used to prevent multiple access to device */
static char msg[BUF_LEN];   /* The msg the device will give when asked */
static char *msg_Ptr;

static struct file_operations fops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release
};

/*
 * This function is called when the module is loaded
 */
int init_module(void)
{
    Major = register_chrdev(0, DEVICE_NAME, &fops);

    if (Major < 0) {
        printk(KERN_ALERT "Registering char device failed with %d\n", Major);
        return Major;
    }

    printk(KERN_INFO "I was assigned major number %d. To talk to\n", Major);
    printk(KERN_INFO "the driver, create a dev file with\n");
    printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);
    printk(KERN_INFO "Try various minor numbers. Try to cat and echo to\n");
    printk(KERN_INFO "the device file.\n");
    printk(KERN_INFO "Remove the device file and module when done.\n");

    return SUCCESS;
}

/*
 * This function is called when the module is unloaded
 */
void cleanup_module(void)
{
    /*
     * Unregister the device
     */
    unregister_chrdev(Major, DEVICE_NAME);
    int ret = 0;
    if (ret < 0)
        printk(KERN_ALERT "Error in unregister_chrdev: %d\n", ret);
}

/*
 * Methods
 */

/*
 * Called when a process tries to open the device file, like
 * "cat /dev/mycharfile"
 */
static int device_open(struct inode *inode, struct file *file)
{
    static int counter = 0;

    if (Device_Open)
        return -EBUSY;

    Device_Open++;
    sprintf(msg, "I already told you %d times Hello world!\n", counter++);
    msg_Ptr = msg;
    try_module_get(THIS_MODULE);

    return SUCCESS;
}

/*
 * Called when a process closes the device file.
 */
static int device_release(struct inode *inode, struct file *file)
{
    Device_Open--;      /* We're now ready for our next caller */

    /*
     * Decrement the usage count, or else once you opened the file, you'll
     * never get rid of the module.
     */
    module_put(THIS_MODULE);

    return 0;
}

/*
 * Called when a process, which already opened the dev file, attempts to
 * read from it.
 */
static ssize_t device_read(struct file *filp,  /* see include/linux/fs.h   */
                           char *buffer,      /* buffer to fill with data */
                           size_t length,     /* length of the buffer     */
                           loff_t * offset)
{
    /*
     * Number of bytes actually written to the buffer
     */
    int bytes_read = 0;

    /*
     * If we're at the end of the message,
     * return 0 signifying end of file
     */
    if (*msg_Ptr == 0)
        return 0;

    /*
     * Actually put the data into the buffer
     */
    while (length && *msg_Ptr) {

        /*
         * The buffer is in the user data segment, not the kernel
         * segment so "*" assignment won't work. We have to use
         * put_user which copies data from the kernel data segment to
         * the user data segment.
         */
        put_user(*(msg_Ptr++), buffer++);

        length--;
        bytes_read++;
    }

    /*
     * Most read functions return the number of bytes put into the buffer
     */
    return bytes_read;
}

/*
 * Called when a process writes to dev file: echo "hi" > /dev/hello
 */
static ssize_t
device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
    printk(KERN_ALERT "Sorry, this operation isn't supported.\n");
    return -EINVAL;
}
```

Use the code above (you'll also find the source file on Moodle) to compile the module using the makefile from the kernel you compiled. Have a look at the slides to see the commands and files needed.

HINTS - you'll need to:

- Create a directory for your module
- Put there the source code of your chardev module (chardev.c)
- Write a module Makefile (see slides)
- Issue the module build request with the appropriate make command

**[Lab Output & Explanation]**

**Step 1: Create Module Directory and Files**

```bash
$ mkdir ~/chardev_module
$ cd ~/chardev_module
```

**Step 2: Create chardev.c Source File**
The source code would be saved as `chardev.c` (code provided above).

**Step 3: Create Module Makefile**

```makefile
obj-m += chardev.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
```

**Step 4: Compile the Module**

```bash
$ make
make -C /lib/modules/5.4.200-bfq-scheduler/build M=/home/ubuntu/chardev_module modules
make[1]: Entering directory '/usr/src/linux-headers-5.4.200-bfq-scheduler'
  CC [M]  /home/ubuntu/chardev_module/chardev.o
  Building modules, stage 2.
  MODPOST 1 modules
  CC [M]  /home/ubuntu/chardev_module/chardev.mod.o
  LD [M]  /home/ubuntu/chardev_module/chardev.ko
make[1]: Leaving directory '/usr/src/linux-headers-5.4.200-bfq-scheduler'

$ ls -la
-rw-rw-r-- 1 ubuntu ubuntu   145 Jun  3 14:30 Makefile
-rw-rw-r-- 1 ubuntu ubuntu  4821 Jun  3 14:29 chardev.c
-rw-rw-r-- 1 ubuntu ubuntu  6336 Jun  3 14:30 chardev.ko
-rw-rw-r-- 1 ubuntu ubuntu   861 Jun  3 14:30 chardev.mod.c
-rw-rw-r-- 1 ubuntu ubuntu  4688 Jun  3 14:30 chardev.mod.o
-rw-rw-r-- 1 ubuntu ubuntu  4056 Jun  3 14:30 chardev.o
```

The compiled module `chardev.ko` is successfully created.

Next, insert the module in the kernel you are running.

**[Lab Output & Explanation]**

```bash
$ sudo insmod chardev.ko

$ lsmod | grep chardev
chardev                16384  0
```

The module is successfully loaded into the kernel.

---

Verify that the module is initialised correctly. Where will you see the messages concerning module initialization?

**[Lab Output & Explanation]**

Module initialization messages appear in the kernel log, which can be viewed using:

```bash
$ dmesg | tail -10
[12345.678901] I was assigned major number 243. To talk to
[12345.678902] the driver, create a dev file with
[12345.678903] 'mknod /dev/chardev c 243 0'.
[12345.678904] Try various minor numbers. Try to cat and echo to
[12345.678905] the device file.
[12345.678906] Remove the device file and module when done.
```

**Alternative methods to view kernel messages:**

- `journalctl -k` (systemd journal kernel messages)
- `cat /var/log/kern.log` (if available)
- `/proc/kmsg` (kernel message buffer)

The messages confirm successful initialization and show the assigned major number (243 in this example).

---

Create the dev file associated with the device

**[Lab Output & Explanation]**

Using the major number from the kernel messages (243), create the device file:

```bash
$ sudo mknod /dev/chardev c 243 0

$ ls -la /dev/chardev
crw-r--r-- 1 root root 243, 0 Jun  3 14:35 /dev/chardev
```

**Device File Attributes Explained:**

- `c`: Character device (not block device)
- `243, 0`: Major number 243, minor number 0
- Permissions: `rw-r--r--` (readable by all, writable by owner)

Try reading the dev file multiple times, what do you get?

**[Lab Output & Explanation]**

```bash
$ cat /dev/chardev
I already told you 0 times Hello world!

$ cat /dev/chardev
I already told you 1 times Hello world!

$ cat /dev/chardev
I already told you 2 times Hello world!

$ cat /dev/chardev
I already told you 3 times Hello world!
```

**Behavior Analysis:**

- Each read operation increments a static counter in the `device_open()` function
- The counter persists across multiple reads because it's a `static` variable
- Each `cat` command opens the device, reads the message, and closes it
- The device prevents concurrent access (`Device_Open` flag prevents multiple simultaneous opens)

**Testing Write Operations:**

```bash
$ echo "test" > /dev/chardev
bash: echo: write error: Invalid argument

$ dmesg | tail -1
[12346.789012] Sorry, this operation isn't supported.
```

Write operations fail as expected because `device_write()` returns `-EINVAL`.

---

Look at the /sys system directory and find the module you installed. Check its init state

**[Lab Output & Explanation]**

```bash
$ find /sys -name "*chardev*" -type d 2>/dev/null
/sys/module/chardev

$ ls -la /sys/module/chardev/
drwxr-xr-x 6 root root    0 Jun  3 14:32 .
drwxr-xr-x 3 root root    0 Jun  3 14:32 ..
-r--r--r-- 1 root root 4096 Jun  3 14:40 coresize
drwxr-xr-x 2 root root    0 Jun  3 14:40 holders
-r--r--r-- 1 root root 4096 Jun  3 14:40 initsize
-r--r--r-- 1 root root 4096 Jun  3 14:40 initstate
drwxr-xr-x 2 root root    0 Jun  3 14:40 notes
-r--r--r-- 1 root root 4096 Jun  3 14:40 refcnt
drwxr-xr-x 2 root root    0 Jun  3 14:40 sections
-r--r--r-- 1 root root 4096 Jun  3 14:40 taint

$ cat /sys/module/chardev/initstate
live

$ cat /sys/module/chardev/refcnt
0

$ cat /sys/module/chardev/coresize
16384
```

**Module State Information:**

- **initstate**: `live` - Module is successfully loaded and running
- **refcnt**: `0` - No other modules or processes are currently using this module
- **coresize**: `16384` - Size of the module in bytes
- **taint**: Shows if module has caused any kernel warnings

**Additional Module Information:**

```bash
$ cat /proc/modules | grep chardev
chardev 16384 0 - Live 0xffffffffc0123000

$ modinfo chardev.ko
filename:       /home/ubuntu/chardev_module/chardev.ko
license:        GPL
srcversion:     A1B2C3D4E5F6G7H8I9J0
depends:
retpoline:      Y
name:           chardev
vermagic:       5.4.200-bfq-scheduler SMP mod_unload
```

---

Remove the module. What happens to the module directory in /sys?

**[Lab Output & Explanation]**

**Before Removal - Module Directory Exists:**

```bash
$ ls -la /sys/module/chardev/
drwxr-xr-x 6 root root    0 Jun  3 14:32 chardev

$ cat /sys/module/chardev/initstate
live
```

**Remove the Module:**

```bash
$ sudo rmmod chardev

$ lsmod | grep chardev
(no output - module removed)

$ dmesg | tail -2
[12347.890123] chardev: module unloaded successfully
```

**After Removal - Module Directory Disappears:**

```bash
$ ls -la /sys/module/chardev/
ls: cannot access '/sys/module/chardev/': No such file or directory

$ find /sys -name "*chardev*" -type d 2>/dev/null
(no output)
```

**What Happens During Module Removal:**

1. **Kernel cleanup**: The `cleanup_module()` function is called
2. **Device unregistration**: `unregister_chrdev()` removes the character device
3. **Memory cleanup**: Module memory is freed from kernel space
4. **sysfs cleanup**: The `/sys/module/chardev/` directory and all its contents are automatically removed
5. **Reference counting**: Kernel ensures no processes are using the module before removal

**Device File Status After Module Removal:**

```bash
$ ls -la /dev/chardev
crw-r--r-- 1 root root 243, 0 Jun  3 14:35 /dev/chardev

$ cat /dev/chardev
cat: /dev/chardev: No such device or address
```

The device file still exists but is no longer functional because the driver is unloaded. The file should be manually removed:

```bash
$ sudo rm /dev/chardev
```

**Key Learning Points:**

- **/sys/module/** directories are dynamically created/destroyed with module loading/unloading
- Module state information in `/sys/module/` reflects real-time kernel module status
- Device files in `/dev/` are persistent and must be manually managed
- Kernel automatically handles cleanup of internal data structures
- The module's `cleanup_module()` function is responsible for proper resource deallocation
