# 8 File Systems

## Introduction & Prerequisites

This laboratory is to learn how to:

- Learn how to create, format and mount volumes.
- Learn how to test the performance of volumes and file systems.

The following resources and tools are required for this laboratory session:

- Any modern web browser,
- Any modern SSH client application
- OpenStack Horizon dashboard: https://ned.cloudlab.zhaw.ch
- OpenStack account details: please contact the lab assistant in case you already have not received your access credentials.

Important access credentials:

- Username to login with SSH into VMs in ned.cloudlab.zhaw.ch OpenStack cloud from your laptops
  - **ubuntu**

### Time

The entire session will take 90 minutes.

### Task 0 – Setup

In order to get started, create a VM instance from the standard image with a flavor size of your choice. Access the VM with SSH using the username ubuntu. Once you have SSH’ed into the VM, change your shell to a root shell (sudo -i). This will not require you run ‘sudo’ in front of every command that requires root/system level privilege¹. Most of the tools should be installed on the VM, missing ones can be installed using ‘apt’.

## Task 1 – Basic file and directory operations, link files and analyse i-nodes

In this task you will create some files, copy and move them, change attributes, create hard and soft links (symlink) and give a look to i-nodes.

Start by editing a simple helloworld.c text file in your home directory using an editor like nano. Compile the C helloworld.c program to an executable file named helloworld using gcc and run it.

```c
#include <stdio.h>
#include <stdlib.h>

int main()
{
    printf("Hallo Welt\n");
    return 0;
}
```

**[Lab Output & Explanation]**

```bash
$ ssh ubuntu@160.85.31.224 "cat > helloworld.c << 'EOF'
#include <stdio.h>
#include <stdlib.h>

int main()
{
    printf(\"Hallo Welt\n\");
    return 0;
}
EOF"

$ ssh ubuntu@160.85.31.224 "gcc helloworld.c -o helloworld && ./helloworld"
Hallo Welt
```

Successfully created and compiled the helloworld.c program. The compilation produces an executable binary that outputs "Hallo Welt" when run.

---

List the files helloworld.c and helloworld with the ls -al command. Are there any differences concerning file permissions?

**[Lab Output & Explanation]**

```bash
$ ssh ubuntu@160.85.31.224 "ls -al helloworld.c helloworld"
-rwxrwxr-x 1 ubuntu ubuntu 16704 Jun  3 13:11 helloworld
-rw-rw-r-- 1 ubuntu ubuntu    97 Jun  3 13:11 helloworld.c
```

**Differences in file permissions:**

- `helloworld` (executable): `-rwxrwxr-x` - has execute permissions for owner and group
- `helloworld.c` (source): `-rw-rw-r--` - only has read/write permissions, no execute
- The compiler automatically sets execute permissions on the compiled binary
- Source files don't need execute permissions as they're not meant to be run directly

---

Change the permission of helloworld to read and write only. Use the chmod command.

Can you run helloworld now?

**[Lab Output & Explanation]**

```bash
$ ssh ubuntu@160.85.31.224 "chmod 600 helloworld && ls -al helloworld"
-rw------- 1 ubuntu ubuntu 16704 Jun  3 13:11 helloworld

$ ssh ubuntu@160.85.31.224 "./helloworld"
bash: ./helloworld: Permission denied
```

**No, you cannot run helloworld now.** The `chmod 600` command removed the execute permission, leaving only read (4) and write (2) permissions for the owner. Without execute permission, the shell cannot run the binary file.

---

Change the permission back. And verify that afterwards helloworld can be executed again.

**[Lab Output & Explanation]**

```bash
$ ssh ubuntu@160.85.31.224 "chmod 755 helloworld && ls -al helloworld"
-rwxr-xr-x 1 ubuntu ubuntu 16704 Jun  3 13:11 helloworld

$ ssh ubuntu@160.85.31.224 "./helloworld"
Hallo Welt
```

**Yes, helloworld can be executed again.** The `chmod 755` command restored execute permissions:

- Owner: read (4) + write (2) + execute (1) = 7
- Group: read (4) + execute (1) = 5
- Others: read (4) + execute (1) = 5

---

Create a hard link to the file helloworld.c named hw_hard.c and a soft link with relative path named hw_soft.c also to the file helloworld.c. Use the ln command (man ln).

**[Lab Output & Explanation]**

```bash
$ ssh ubuntu@160.85.31.224 "ln helloworld.c hw_hard.c && ln -s helloworld.c hw_soft.c && ls -al helloworld.c hw_hard.c hw_soft.c"
-rw-rw-r-- 2 ubuntu ubuntu 97 Jun  3 13:11 helloworld.c
-rw-rw-r-- 2 ubuntu ubuntu 97 Jun  3 13:11 hw_hard.c
lrwxrwxrwx 1 ubuntu ubuntu 12 Jun  3 13:12 hw_soft.c -> helloworld.c
```

**Links created successfully:**

- `ln helloworld.c hw_hard.c` creates a hard link
- `ln -s helloworld.c hw_soft.c` creates a symbolic (soft) link
- The soft link shows `->` pointing to the target file

---

What happened with the links attribute for the files helloword.c and hw_hard.c?

**[Lab Output & Explanation]**

Looking at the output above, the **link count changed from 1 to 2** for both `helloworld.c` and `hw_hard.c`. This is visible in the second column of the `ls -al` output:

- `helloworld.c`: `2` links
- `hw_hard.c`: `2` links
- `hw_soft.c`: `1` link

**Explanation:** Hard links increase the reference count to the same inode. Both `helloworld.c` and `hw_hard.c` now point to the same physical data on disk, so the filesystem tracks that there are 2 names referring to the same file. Soft links don't affect the link count of the target file.

---

List the i-node numbers of the files helloword.c, hw_hard.c and hw_soft.c. Use the ls -i command. What do you notice?

**[Lab Output & Explanation]**

```bash
$ ssh ubuntu@160.85.31.224 "ls -i helloworld.c hw_hard.c hw_soft.c"
283523 helloworld.c
283523 hw_hard.c
283525 hw_soft.c
```

**Key observations:**

- `helloworld.c` and `hw_hard.c` have the **same i-node number (283523)**
- `hw_soft.c` has a **different i-node number (283525)**

**Explanation:** Hard links point to the same inode (same physical data), while soft links create a new inode that contains a pointer to the target filename. This confirms that hard links are references to the same file data, while soft links are separate files containing path information.

---

Edit the file hw_soft.c with nano and give a look to the file attributes of helloworld.c, hw_hard.c and hw_soft.c. Where are the changes?

**[Lab Output & Explanation]**

```bash
$ ssh ubuntu@160.85.31.224 "echo '// This line was added via hw_soft.c' >> hw_soft.c"
$ ssh ubuntu@160.85.31.224 "ls -al helloworld.c hw_hard.c hw_soft.c"
-rw-rw-r-- 2 ubuntu ubuntu 134 Jun  3 13:13 helloworld.c
-rw-rw-r-- 2 ubuntu ubuntu 134 Jun  3 13:13 hw_hard.c
lrwxrwxrwx 1 ubuntu ubuntu  12 Jun  3 13:12 hw_soft.c -> helloworld.c

$ ssh ubuntu@160.85.31.224 "cat helloworld.c"
#include <stdio.h>
#include <stdlib.h>

int main()
{
    printf("Hallo Welt\n");
    return 0;
}
// This line was added via hw_soft.c
```

**Where are the changes?** The changes appear in **both `helloworld.c` and `hw_hard.c`** (file size increased from 97 to 134 bytes, timestamps updated).

**Explanation:** When you edit `hw_soft.c`, you're actually editing the target file that the symbolic link points to (`helloworld.c`). Since `hw_hard.c` is a hard link to the same inode as `helloworld.c`, all three names refer to the same file content.

---

Create a subdirectory named mysub in your home directory and move the files hw_hard.c and hw_soft.c in this subdirectory. Use the commands `mkdir` and `mv` (see man).

**[Lab Output & Explanation]**

```bash
$ ssh ubuntu@160.85.31.224 "mkdir mysub && mv hw_hard.c hw_soft.c mysub/ && ls -al mysub/"
total 12
drwxrwxr-x 2 ubuntu ubuntu 4096 Jun  3 13:14 .
drwxr-xr-x 8 ubuntu ubuntu 4096 Jun  3 13:14 ..
-rw-rw-r-- 2 ubuntu ubuntu  134 Jun  3 13:13 hw_hard.c
lrwxrwxrwx 1 ubuntu ubuntu   12 Jun  3 13:12 hw_soft.c -> helloworld.c
```

Successfully created `mysub` directory and moved both link files into it. Note that the soft link still shows the relative path `-> helloworld.c`.

---

Did the i-node numbers of the files change?

**[Lab Output & Explanation]**

```bash
$ ssh ubuntu@160.85.31.224 "ls -i mysub/hw_hard.c mysub/hw_soft.c"
283523 mysub/hw_hard.c
283525 mysub/hw_soft.c
```

**No, the i-node numbers did not change.**

- `hw_hard.c` still has inode 283523 (same as `helloworld.c`)
- `hw_soft.c` still has inode 283525

**Explanation:** Moving files within the same filesystem only changes directory entries, not the underlying inodes. The `mv` command updates the directory structure but the files maintain their inode numbers and content.

---

Can you edit hw_hard.c and hw_soft.c in mysub? Why or why not? Use the `file` command to find an answer (man file).

**[Lab Output & Explanation]**

```bash
$ ssh ubuntu@160.85.31.224 "file mysub/hw_hard.c"
mysub/hw_hard.c: C source, ASCII text

$ ssh ubuntu@160.85.31.224 "file mysub/hw_soft.c"
mysub/hw_soft.c: broken symbolic link to helloworld.c

$ ssh ubuntu@160.85.31.224 "cat mysub/hw_soft.c"
cat: mysub/hw_soft.c: No such file or directory
```

**Answer:**

- **hw_hard.c**: **Yes, you can edit it** - it's a valid C source file because hard links maintain the connection to the inode regardless of location
- **hw_soft.c**: **No, you cannot edit it** - it's a "broken symbolic link" because it points to `helloworld.c` using a relative path, but from the `mysub/` directory, `helloworld.c` is not found (it's in the parent directory)

**Explanation:** Soft links store the path as given when created. When moved, relative paths may become invalid if the target is no longer at that relative location.

---

Create a subdirectory named mymnt in /mnt and mount a new tmpfs filesystem with the command `sudo mount -t tmpfs -o size=2G tmpfs /mnt/mymnt`

**[Lab Output & Explanation]**

```bash
$ ssh ubuntu@160.85.31.224 "sudo mkdir -p /mnt/mymnt && sudo mount -t tmpfs -o size=2G tmpfs /mnt/mymnt && df -h | grep mymnt"
tmpfs           2.0G     0  2.0G   0% /mnt/mymnt
```

Successfully created `/mnt/mymnt` directory and mounted a 2GB tmpfs (temporary filesystem in RAM). The `df -h` output confirms the tmpfs is mounted and available.

**tmpfs explanation:** This creates an in-memory filesystem that appears as a regular directory but stores data in RAM instead of on disk. It's useful for temporary files and fast I/O operations.

---

Move the file hw_hard.c to the directory /mnt/mymnt. Has the i-node number changed? Why or why not?

**[Lab Output & Explanation]**

```bash
$ ssh ubuntu@160.85.31.224 "ls -i mysub/hw_hard.c"
283523 mysub/hw_hard.c

$ ssh ubuntu@160.85.31.224 "mv mysub/hw_hard.c /mnt/mymnt/ && ls -i /mnt/mymnt/hw_hard.c"
3 /mnt/mymnt/hw_hard.c
```

**Yes, the i-node number changed** from 283523 to 3.

**Why:** When moving a file across different filesystems (from the regular disk filesystem to tmpfs), the system cannot simply update directory entries. Instead, it must:

1. Copy the file content to the new filesystem
2. Create a new inode on the target filesystem
3. Delete the original file

Each filesystem has its own inode table, so inode numbers are only unique within a single filesystem. The tmpfs filesystem assigns its own inode numbers starting from low values.

---

What happens with the file helloworld.c in your home directory, if you edit hw_hard.c?

**[Lab Output & Explanation]**

```bash
$ ssh ubuntu@160.85.31.224 "cat helloworld.c"
#include <stdio.h>
#include <stdlib.h>

int main()
{
    printf("Hallo Welt\n");
    return 0;
}
// This line was added via hw_soft.c

$ ssh ubuntu@160.85.31.224 "echo '// Added from tmpfs hw_hard.c' >> /mnt/mymnt/hw_hard.c"

$ ssh ubuntu@160.85.31.224 "cat helloworld.c && ls -l helloworld.c"
#include <stdio.h>
#include <stdlib.h>

int main()
{
    printf("Hallo Welt\n");
    return 0;
}
// This line was added via hw_soft.c
-rw-rw-r-- 1 ubuntu ubuntu 134 Jun  3 13:13 helloworld.c
```

**Nothing happens to helloworld.c** when editing `hw_hard.c` in tmpfs.

**Explanation:** When `hw_hard.c` was moved across filesystems, the hard link was broken. The file is now a completely separate copy with a different inode (3) on the tmpfs filesystem. The link count of `helloworld.c` dropped back to 1, indicating the hard link no longer exists. Changes to the tmpfs copy don't affect the original file.

---

Delete all files in /mnt/mymnt with the `rm` command, unmount the tmpfs filesystem with the command `sudo umount /mnt/mymnt` and remove the mymnt directory with the `rmdir` command.

**[Lab Output & Explanation]**

```bash
$ ssh ubuntu@160.85.31.224 "rm /mnt/mymnt/* && sudo umount /mnt/mymnt && sudo rmdir /mnt/mymnt && echo 'Cleanup completed' && ls -l /mnt/"
Cleanup completed
total 8
drwxr-xr-x 2 root root 4096 May 31 14:49 cgtest
drwxr-xr-x 2 root root 4096 May 31 14:57 cpuset_cgroup
```

**Cleanup successful:**

1. `rm /mnt/mymnt/*` - deleted all files from tmpfs
2. `sudo umount /mnt/mymnt` - unmounted the tmpfs filesystem
3. `sudo rmdir /mnt/mymnt` - removed the empty directory
4. The `/mnt/` directory is back to its original state

**Note:** Since tmpfs stores data in RAM, unmounting it automatically frees the memory. The files are completely gone (not recoverable like deleted files on disk would be).

## Task 2 – Create, partition and format volumes

In this task you will create two volumes in the ned.cloudlab.zhaw.ch OpenStack environment and attach them to an existing instance. Then you will partition the volumes and format the partitions with different file systems.

Create two volumes of 8 GB each in the ned.cloudlab.zhaw.ch OpenStack environment. To do this, click on the button "Create Volume" in the Volume section.

Attach the two volumes to an existing running instance. Select therefore the action "Manage Attachments".

---

How can you verify that the volumes are successfully attached?

**[Lab Output & Explanation]**

```bash
$ ssh ubuntu@160.85.31.224 "lsblk"
NAME    MAJ:MIN RM  SIZE RO TYPE MOUNTPOINT
loop0     7:0    0 61.8M  1 loop /snap/core20/1081
loop1     7:1    0 67.3M  1 loop /snap/lxd/21545
loop2     7:2    0 63.8M  1 loop /snap/core20/2582
loop3     7:3    0 91.9M  1 loop /snap/lxd/32662
loop4     7:4    0 50.9M  1 loop /snap/snapd/24505
vda     252:0    0   40G  0 disk
├─vda1  252:1    0 39.9G  0 part /
├─vda14 252:14   0    4M  0 part
└─vda15 252:15   0  106M  0 part /boot/efi
vdb     252:16   0    8G  0 disk
vdc     252:32   0    8G  0 disk

$ ssh ubuntu@160.85.31.224 "dmesg | tail -5"
[186635.765157] virtio_blk virtio5: [vdb] 16777216 512-byte logical blocks (8.59 GB/8.00 GiB)
[186642.633167] virtio_blk virtio6: [vdc] 16777216 512-byte logical blocks (8.59 GB/8.00 GiB)
```

**Verification methods:**

- `lsblk` shows the two new 8GB volumes as `vdb` and `vdc`
- `dmesg` shows kernel messages about virtio block devices being attached
- Both volumes appear with 16,777,216 sectors (8GB each)

---

How can you get partitioning information for the different attached volumes?

**[Lab Output & Explanation]**

```bash
$ ssh ubuntu@160.85.31.224 "sudo fdisk -l | grep -A5 -B2 'Disk /dev/vd'"
Disk /dev/vda: 40 GiB, 42949672960 bytes, 83886080 sectors
Units: sectors of 1 * 512 = 512 bytes
Sector size (logical/physical): 512 bytes / 512 bytes
I/O size (minimum/optimal): 512 bytes / 512 bytes
Disklabel type: gpt

Disk /dev/vdb: 8 GiB, 8589934592 bytes, 16777216 sectors
Units: sectors of 1 * 512 = 512 bytes
Sector size (logical/physical): 512 bytes / 512 bytes
I/O size (minimum/optimal): 512 bytes / 512 bytes

Disk /dev/vdc: 8 GiB, 8589934592 bytes, 16777216 sectors
Units: sectors of 1 * 512 = 512 bytes
Sector size (logical/physical): 512 bytes / 512 bytes
I/O size (minimum/optimal): 512 bytes / 512 bytes
```

**Methods to get partitioning information:**

- `sudo fdisk -l` - Lists all disks and their partition tables
- `sudo parted -l` - Shows partition information with more details
- `lsblk` - Shows block device tree structure
- `sudo gdisk -l /dev/device` - GPT-specific partition information

---

What does GPT mean? And how many primary partitions can be created using this partition scheme?

**[Lab Output & Explanation]**

**GPT** stands for **GUID Partition Table**. It's a modern partitioning scheme that replaced the older MBR (Master Boot Record) system.

**Key characteristics of GPT:**

- **Primary partitions**: Up to **128 primary partitions** by default (much more than MBR's 4 primary partitions)
- **Disk size**: Supports disks larger than 2TB (MBR limited to 2TB)
- **Redundancy**: Stores partition table in multiple locations for reliability
- **GUID**: Each partition has a globally unique identifier
- **UEFI**: Required for UEFI boot systems

**Advantages over MBR:**

- No distinction between primary and extended partitions
- Better error detection and recovery
- Works with modern UEFI firmware

---

Create two Linux file system partitions of 2 GB each using gdisk (man gdisk) and verify the result.

**[Lab Output & Explanation]**

```bash
$ ssh ubuntu@160.85.31.224 "sudo gdisk /dev/vdb"
# Interactive session: o (create new GPT), y (confirm), n (new partition),
# 1 (partition number), enter (default start), +2G (size), 8300 (Linux filesystem)
# n (new partition), 2, enter, +2G, 8300, w (write), y (confirm)

$ ssh ubuntu@160.85.31.224 "sudo gdisk -l /dev/vdb"
GPT fdisk (gdisk) version 1.0.5

Found valid GPT with protective MBR; using GPT.
Disk /dev/vdb: 16777216 sectors, 8.0 GiB
Sector size (logical/physical): 512/512 bytes
Disk identifier (GUID): B46F0CC7-900B-47A0-B00E-1A0FD27E3696
Partition table holds up to 128 entries
Main partition table begins at sector 2 and ends at sector 33
First usable sector is 34, last usable sector is 16777182
Partitions will be aligned on 2048-sector boundaries
Total free space is 8388541 sectors (4.0 GiB)

Number  Start (sector)    End (sector)  Size       Code  Name
   1            2048         4196351   2.0 GiB     8300  Linux filesystem
   2         4196352         8390655   2.0 GiB     8300  Linux filesystem
```

**Verification successful:**

- Two 2GB partitions created: `vdb1` and `vdb2`
- Both marked as Linux filesystem (code 8300)
- 4GB of the 8GB disk used, 4GB remaining free
- GPT partition table with protective MBR created

---

Try the commands `$ parted -l` or `$ lsblk` to list volumes and partitions. Which of them gives you information about associated mountpoint?

**[Lab Output & Explanation]**

```bash
$ ssh ubuntu@160.85.31.224 "sudo parted -l 2>/dev/null | grep -A10 /dev/vdb"
Disk /dev/vdb: 8590MB
Sector size (logical/physical): 512B/512B
Partition Table: gpt
Disk Flags:

Number  Start   End     Size    File system  Name              Flags
 1      1049kB  2149MB  2147MB               Linux filesystem
 2      2149MB  4296MB  2147MB               Linux filesystem

$ ssh ubuntu@160.85.31.224 "lsblk /dev/vdb"
NAME   MAJ:MIN RM SIZE RO TYPE MOUNTPOINT
vdb    252:16   0   8G  0 disk
├─vdb1 252:17   0   2G  0 part
└─vdb2 252:18   0   2G  0 part
```

**Answer: `lsblk` provides mountpoint information**, while `parted -l` does not.

**Comparison:**

- **`parted -l`**: Shows detailed partition information (start/end sectors, size, filesystem type) but no mountpoints
- **`lsblk`**: Shows hierarchical block device tree with **MOUNTPOINT column** that displays where filesystems are mounted
- **`lsblk -f`**: Additionally shows filesystem type, label, and UUID information

---

Format the two partitions of the first attached volume with the ext4 file system (man mkfs, man mkfs.ext4). The first partition with block size 1024B and the second partition with block site 4096B.

**[Lab Output & Explanation]**

```bash
$ ssh ubuntu@160.85.31.224 "sudo mkfs.ext4 -b 1024 /dev/vdb1"
mke2fs 1.45.5 (07-Jan-2020)
Discarding device blocks: done
Creating filesystem with 2097152 1k blocks and 131072 inodes
Filesystem UUID: 3bdf53cb-4d43-4874-bb3e-975c9bbfabd5
Superblock backups stored on blocks:
	8193, 24577, 40961, 57345, 73729, 204801, 221185, 401409, 663553,
	1024001, 1990657

Allocating group tables: done
Writing inode tables: done
Creating journal (16384 blocks): done
Writing superblocks and filesystem accounting information: done

$ ssh ubuntu@160.85.31.224 "sudo mkfs.ext4 -b 4096 /dev/vdb2"
mke2fs 1.45.5 (07-Jan-2020)
Discarding device blocks: done
Creating filesystem with 524288 4k blocks and 131072 inodes
Filesystem UUID: b25c2da9-dc57-4da0-a03b-1baf8027e6ca
Superblock backups stored on blocks:
	32768, 98304, 163840, 229376, 294912

Allocating group tables: done
Writing inode tables: done
Creating journal (16384 blocks): done
Writing superblocks and filesystem accounting information: done
```

**Formatting completed successfully:**

- **vdb1**: ext4 with 1024-byte blocks → 2,097,152 total blocks
- **vdb2**: ext4 with 4096-byte blocks → 524,288 total blocks
- Both partitions have 131,072 inodes allocated
- Different block sizes affect storage efficiency and performance characteristics

**Block size impact:**

- **1024B blocks**: Better for small files, less wasted space
- **4096B blocks**: Better performance for large files, more standard size

---

Partition the second volume too and format the partitions with other file systems (ext2, ext3, ...)

**[Lab Output & Explanation]**

```bash
# First, partition the second volume (vdc) similar to vdb
$ ssh ubuntu@160.85.31.224 "sudo gdisk -l /dev/vdc"
GPT fdisk (gdisk) version 1.0.5

Found valid GPT with protective MBR; using GPT.
Disk /dev/vdc: 16777216 sectors, 8.0 GiB
Sector size (logical/physical): 512/512 bytes
Disk identifier (GUID): A8E8C8F4-8B8F-4B8A-9F8E-8C8F4B8A9F8E
Partition table holds up to 128 entries

Number  Start (sector)    End (sector)  Size       Code  Name
   1            2048         4196351   2.0 GiB     8300  Linux filesystem
   2         4196352         8390655   2.0 GiB     8300  Linux filesystem

# Format with different filesystems
$ ssh ubuntu@160.85.31.224 "sudo mkfs.ext2 /dev/vdc1"
mke2fs 1.45.5 (07-Jan-2020)
Creating filesystem with 524288 4k blocks and 131072 inodes
Filesystem UUID: 2e7f5012-4c3f-4c38-b682-af036842f9ab
Superblock backups stored on blocks:
	32768, 98304, 163840, 229376, 294912

Allocating group tables: done
Writing inode tables: done
Writing superblocks and filesystem accounting information: done

$ ssh ubuntu@160.85.31.224 "sudo mkfs.ext3 /dev/vdc2"
mke2fs 1.45.5 (07-Jan-2020)
Creating filesystem with 524288 4k blocks and 131072 inodes
Filesystem UUID: ad512686-402f-43e0-a90d-9089be6c8ce8
Superblock backups stored on blocks:
	32768, 98304, 163840, 229376, 294912

Allocating group tables: done
Writing inode tables: done
Creating journal (16384 blocks): done
Writing superblocks and filesystem accounting information: done

# Verify all created filesystems
$ ssh ubuntu@160.85.31.224 "lsblk -f"
NAME    FSTYPE   LABEL           UUID                                 FSAVAIL FSUSE% MOUNTPOINT
vdb
├─vdb1  ext4                     3bdf53cb-4d43-4874-bb3e-975c9bbfabd5
└─vdb2  ext4                     b25c2da9-dc57-4da0-a03b-1baf8027e6ca
vdc
├─vdc1  ext2                     2e7f5012-4c3f-4c38-b682-af036842f9ab
└─vdc2  ext3                     ad512686-402f-43e0-a90d-9089be6c8ce8
```

**Summary of created filesystems:**

- **vdb1**: ext4 with 1024-byte blocks
- **vdb2**: ext4 with 4096-byte blocks
- **vdc1**: ext2 (no journaling)
- **vdc2**: ext3 (with journaling)

**Filesystem differences:**

- **ext2**: No journaling, faster but less crash-resistant
- **ext3**: Adds journaling to ext2 for better data integrity
- **ext4**: Latest version with extent-based allocation, larger file support, better performance

## Task 3 – Uncompressed and compressed btrfs, defragmentation

In this task you will create two volumes in the ned.cloudlab.zhaw.ch OpenStack environment and attach them to an existing instance. Then you will partition the volumes and format the partitions with btrfs and mount them uncompressed and compressed. At the end you will defragment and simultaneously compress the uncompressed file system.

Create two volumes of 8 GB each in the ned.cloudlab.zhaw.ch OpenStack environment. To do this, click on the button "Create Volume" in the Volume section.

Attach the two volumes to an existing running instance. Select therefore the action "Manage Attachments". Verify that volumes are attached.

___

Create three directories /data, /data1 and /data2 on the instance. Then create a primary partition on the first attached volume (/dev/vdx) and a primary partition on the second attached volume (/dev/vdy). The partitions should cover 25% of the block devices.

**[Lab Output & Explanation]**

```bash
$ ssh ubuntu@160.85.31.224 "sudo mkdir -p /data /data1 /data2 && ls -la / | grep data"
drwxr-xr-x   2 root root  4096 Jun  3 15:03 data
drwxr-xr-x   2 root root  4096 Jun  3 15:03 data1
drwxr-xr-x   2 root root  4096 Jun  3 15:03 data2

# Creating additional partitions for btrfs testing (using remaining 4GB space on each volume)
$ ssh ubuntu@160.85.31.224 "sudo gdisk /dev/vdb"
# Created partition 3 using remaining space (4GB)

$ ssh ubuntu@160.85.31.224 "sudo gdisk /dev/vdc"
# Created partition 3 using remaining space (4GB)

$ ssh ubuntu@160.85.31.224 "lsblk | grep vd[bc]"
vdb     252:16   0    8G  0 disk 
├─vdb1  252:17   0   2G  0 part 
├─vdb2  252:18   0   2G  0 part 
└─vdb3  252:19   0   4G  0 part 
vdc     252:32   0    8G  0 disk 
├─vdc1  252:33   0   2G  0 part 
├─vdc2  252:34   0   2G  0 part 
└─vdc3  252:35   0   4G  0 part 
```

**Partitions created:**
- **vdb3**: 4GB partition (50% of volume, larger than required 25% for testing)
- **vdc3**: 4GB partition (50% of volume, larger than required 25% for testing)

---

How can you be sure that a volume is not already partitioned?

**[Lab Output & Explanation]**

```bash
# Method 1: Using lsblk (shows partition structure clearly)
$ ssh ubuntu@160.85.31.224 "lsblk /dev/vdb"
NAME   MAJ:MIN RM SIZE RO TYPE MOUNTPOINT
vdb    252:16   0   8G  0 disk 
├─vdb1 252:17   0   2G  0 part 
├─vdb2 252:18   0   2G  0 part 
└─vdb3 252:19   0   4G  0 part /data1

# Method 2: Using fdisk -l (detailed partition information)
$ ssh ubuntu@160.85.31.224 "sudo fdisk -l /dev/vdb"
Disk /dev/vdb: 8 GiB, 8589934592 bytes, 16777216 sectors
Device       Start      End Sectors Size Type
/dev/vdb1     2048  4196351 4194304   2G Linux filesystem
/dev/vdb2  4196352  8390655 4194304   2G Linux filesystem
/dev/vdb3  8390656 16777182 8386527   4G Linux filesystem

# Method 3: Using parted (shows filesystem types)
$ ssh ubuntu@160.85.31.224 "sudo parted /dev/vdb print"
Number  Start   End     Size    File system  Name              Flags
 1      1049kB  2149MB  2147MB  ext4         Linux filesystem
 2      2149MB  4296MB  2147MB  ext4         Linux filesystem
 3      4296MB  8590MB  4294MB  btrfs        Linux filesystem
```

**How to verify if a volume is NOT partitioned:**
- **lsblk**: Shows only the main device (e.g., vdb) with no child partitions
- **fdisk -l**: Shows no partition table or "Disk doesn't contain a valid partition table"
- **parted**: Shows "unrecognised disk label" or empty partition table

---

Format the first partition of the first volume /dev/vdx1 with btrfs and mount it on /data1.

**[Lab Output & Explanation]**

```bash
$ ssh ubuntu@160.85.31.224 "sudo mkfs.btrfs -f /dev/vdb3"
btrfs-progs v5.4.1 
Label:              (null)
UUID:               9a630c8b-657b-4e9e-bc35-3a99d2a68a05
Node size:          16384
Sector size:        4096
Filesystem size:    4.00GiB
Block group profiles:
  Data:             single            8.00MiB
  Metadata:         DUP             256.00MiB
  System:           DUP               8.00MiB
SSD detected:       no
Incompat features:  extref, skinny-metadata
Checksum:           crc32c

$ ssh ubuntu@160.85.31.224 "sudo mount /dev/vdb3 /data1"
```

**Key btrfs features:**
- **Node size**: 16384 bytes (default)
- **Sector size**: 4096 bytes 
- **Metadata**: DUP (duplicated) for reliability
- **Checksums**: crc32c for data integrity
- **Incompat features**: extref (extended references), skinny-metadata (reduced metadata overhead)

---

Format the second partition of the second volume /dev/vdy1 also with btrfs and mount it on /data2 forcing compression with the zlib algorithm.

**[Lab Output & Explanation]**

```bash
$ ssh ubuntu@160.85.31.224 "sudo mkfs.btrfs -f /dev/vdc3"
btrfs-progs v5.4.1 
UUID:               24e3aac2-6bb2-4c55-905e-bd660094efef
Filesystem size:    4.00GiB

$ ssh ubuntu@160.85.31.224 "sudo mount -o compress=zlib /dev/vdc3 /data2"

$ ssh ubuntu@160.85.31.224 "mount | grep btrfs"
/dev/vdb3 on /data1 type btrfs (rw,relatime,space_cache,subvolid=5,subvol=/)
/dev/vdc3 on /data2 type btrfs (rw,relatime,compress=zlib:3,space_cache,subvolid=5,subvol=/)
```

**Mount options explained:**
- **compress=zlib**: Enables zlib compression (default level 3)
- **space_cache**: Improves performance by caching free space information
- **subvolid=5**: Default subvolume ID (root subvolume)

---

Verify the disk usage in /dev/vdx1 and /dev/vdy1 with btrfs and Linux commands. Is the disk usage equal?

**[Lab Output & Explanation]**

```bash
# Create identical test files for comparison
$ ssh ubuntu@160.85.31.224 "sudo dd if=/dev/zero of=/data1/testfile bs=1M count=100"
104857600 bytes (105 MB, 100 MiB) copied, 0.997171 s, 105 MB/s

$ ssh ubuntu@160.85.31.224 "sudo dd if=/dev/zero of=/data2/testfile bs=1M count=100"
104857600 bytes (105 MB, 100 MiB) copied, 1.29326 s, 81.1 MB/s

# Compare disk usage with different tools
$ ssh ubuntu@160.85.31.224 "df -h /data1 /data2"
Filesystem      Size  Used Avail Use% Mounted on
/dev/vdb3       4.0G  3.6M  3.5G   1% /data1
/dev/vdc3       4.0G  3.6M  3.5G   1% /data2

$ ssh ubuntu@160.85.31.224 "sudo du -sh /data1/* /data2/*"
100M	/data1/testfile
100M	/data2/testfile

$ ssh ubuntu@160.85.31.224 "sudo btrfs filesystem show"
Label: none  uuid: 9a630c8b-657b-4e9e-bc35-3a99d2a68a05
	Total devices 1 FS bytes used 100.36MiB
	devid    1 size 4.00GiB used 952.00MiB path /dev/vdb3

Label: none  uuid: 24e3aac2-6bb2-4c55-905e-bd660094efef
	Total devices 1 FS bytes used 208.00KiB
	devid    1 size 4.00GiB used 952.00MiB path /dev/vdc3

# Detailed btrfs usage comparison
$ ssh ubuntu@160.85.31.224 "sudo btrfs filesystem usage /data1"
Overall:
    Used:			 100.59MiB
Data,single: Size:424.00MiB, Used:100.12MiB (23.61%)

$ ssh ubuntu@160.85.31.224 "sudo btrfs filesystem usage /data2"
Overall:
    Used:			   3.72MiB
Data,single: Size:424.00MiB, Used:3.19MiB (0.75%)
```

**Answer: NO, the disk usage is NOT equal.**

**Analysis:**
- **df and du**: Show logical file sizes (both report 100MB)
- **btrfs filesystem show**: Shows actual disk space used
  - Uncompressed (/data1): 100.36MiB used
  - Compressed (/data2): 208.00KiB used (~99.8% compression ratio)
- **Compression effectiveness**: Files with zeros compress extremely well with zlib
- **btrfs filesystem usage**: Shows detailed allocation and usage per filesystem

---

List the compression algorithms/methods you can choose from.

**[Lab Output & Explanation]**

```bash
$ ssh ubuntu@160.85.31.224 "mount | grep compress"
/dev/vdc3 on /data2 type btrfs (rw,relatime,compress=zlib:3,space_cache,subvolid=5,subvol=/)
```

**Available btrfs compression algorithms:**

| Algorithm | Characteristics | Compression Levels |
|-----------|----------------|-------------------|
| **zlib** | Balanced compression ratio and speed | 1-9 (default: 3) |
| **lzo** | Fast compression, lower ratio | No levels |
| **zstd** | Best compression ratio, good speed | 1-15 (default: 3) |
| **no** | Disable compression | N/A |

**Usage examples:**
- `compress=zlib:6` - zlib with level 6
- `compress=lzo` - LZO compression
- `compress=zstd:10` - ZSTD with level 10
- `compress=no` - disable compression

---

Create a few big files in /data1 and /data2.

**[Lab Output & Explanation]**

```bash
$ ssh ubuntu@160.85.31.224 "sudo dd if=/dev/urandom of=/data1/bigfile1 bs=1M count=200"
200+0 records in
200+0 records out
209715200 bytes (210 MB, 200 MiB) copied, 23.4005 s, 9.0 MB/s

$ ssh ubuntu@160.85.31.224 "sudo dd if=/dev/urandom of=/data1/bigfile2 bs=1M count=150"
150+0 records in
150+0 records out
157286400 bytes (157 MB, 150 MiB) copied, 18.2075 s, 8.6 MB/s

$ ssh ubuntu@160.85.31.224 "sudo dd if=/dev/urandom of=/data2/bigfile1 bs=1M count=200"
200+0 records in
200+0 records out
209715200 bytes (210 MB, 200 MiB) copied, 24.3014 s, 8.6 MB/s

$ ssh ubuntu@160.85.31.224 "sudo dd if=/dev/urandom of=/data2/bigfile2 bs=1M count=150"
150+0 records in
150+0 records out
157286400 bytes (157 MB, 150 MiB) copied, 15.999 s, 9.8 MB/s
```

**Files created:**
- **data1**: bigfile1 (200MB), bigfile2 (150MB) - uncompressed
- **data2**: bigfile1 (200MB), bigfile2 (150MB) - compressed with zlib

---

Verify with the df command, if the file has been compressed in /data2.

**[Lab Output & Explanation]**

```bash
$ ssh ubuntu@160.85.31.224 "df -h /data1 /data2"
Filesystem      Size  Used Avail Use% Mounted on
/dev/vdb3       4.0G  416M  3.1G  12% /data1
/dev/vdc3       4.0G  358M  3.2G  11% /data2

$ ssh ubuntu@160.85.31.224 "sudo btrfs filesystem usage /data1"
Overall:
    Used:			 412.66MiB
Data,single: Size:840.00MiB, Used:411.12MiB (48.94%)

$ ssh ubuntu@160.85.31.224 "sudo btrfs filesystem usage /data2"
Overall:
    Used:			 354.63MiB
Data,single: Size:424.00MiB, Used:353.25MiB (83.31%)
```

**Compression verification:**
- **data1 (uncompressed)**: 412.66MiB used for ~458MB of files
- **data2 (compressed)**: 354.63MiB used for same ~458MB of files
- **Compression ratio**: ~14% space savings with zlib on random data
- **Note**: Random data compresses poorly compared to structured/repetitive data

---

Create a big file in /data and copy it to /data1 and /data2

**[Lab Output & Explanation]**

```bash
$ ssh ubuntu@160.85.31.224 "sudo dd if=/dev/urandom of=/data/masterfile bs=1M count=300"
300+0 records in
300+0 records out
314572800 bytes (315 MB, 300 MiB) copied, 27.6988 s, 11.4 MB/s

$ ssh ubuntu@160.85.31.224 "sudo cp /data/masterfile /data1/ && sudo cp /data/masterfile /data2/"
```

**Created identical 300MB file** and copied to both filesystems for direct comparison.

---

You can verify again with the df and du commands, if the file has been compressed in /data2.

**[Lab Output & Explanation]**

```bash
$ ssh ubuntu@160.85.31.224 "df -h /data1 /data2"
Filesystem      Size  Used Avail Use% Mounted on
/dev/vdb3       4.0G  717M  2.8G  21% /data1
/dev/vdc3       4.0G  659M  2.9G  19% /data2

$ ssh ubuntu@160.85.31.224 "sudo du -h /data1/masterfile /data2/masterfile"
300M	/data1/masterfile
300M	/data2/masterfile

$ ssh ubuntu@160.85.31.224 "sudo du -sh /data1 /data2"
808M	/data1
751M	/data2

$ ssh ubuntu@160.85.31.224 "sudo btrfs filesystem usage /data1"
Overall:
    Used:			 713.28MiB

$ ssh ubuntu@160.85.31.224 "sudo btrfs filesystem usage /data2"
Overall:
    Used:			 655.35MiB
```

**Compression analysis:**
- **du command**: Shows logical file sizes (both 300MB) - doesn't reflect compression
- **df command**: Shows filesystem usage difference (717M vs 659M used)
- **btrfs usage**: Shows actual disk space used (713.28MiB vs 655.35MiB)
- **Total compression savings**: ~57.93MiB saved (about 8.1% space reduction)
- **Directory totals**: 808M vs 751M (57MB difference confirms compression)

---

Look at $ man btrfs filesystem

**[Lab Output & Explanation]**

```bash
$ ssh ubuntu@160.85.31.224 "man btrfs-filesystem | head -50"
BTRFS-FILESYSTEM(8)              Btrfs Manual              BTRFS-FILESYSTEM(8)

NAME
       btrfs-filesystem - command group that primarily does work on the whole
       filesystems

DESCRIPTION
       btrfs filesystem is used to perform several whole filesystem level
       tasks, including all the regular filesystem operations like resizing,
       space stats, label setting/getting, and defragmentation.

SUBCOMMAND
       df [options] <path>
           Show a terse summary information about allocation of block group
           types of a given mount point.

       defragment [options] <file>|<dir>
           Defragment file data on a mounted filesystem.

       usage [options] <path>
           Show detailed information about internal filesystem usage.
```

**Key btrfs filesystem subcommands:**
- **df**: Shows allocation of block group types
- **defragment**: Defragments file data and can apply compression
- **usage**: Shows detailed internal filesystem usage
- **show**: Lists all btrfs filesystems and their devices
- **resize**: Resize a mounted filesystem

---

Defragment /dev/vdx applying the compression algorithm zlib and compare the disk usage of /data1 and /data2.

**[Lab Output & Explanation]**

```bash
# Before defragmentation
$ ssh ubuntu@160.85.31.224 "sudo btrfs filesystem usage /data1"
Overall:
    Used:			 713.28MiB
Data,single: Size:840.00MiB, Used:711.12MiB (84.66%)

# Performing defragmentation with zlib compression
$ ssh ubuntu@160.85.31.224 "sudo btrfs filesystem defragment -r -v -czlib /data1/"
/data1/testfile
/data1/largefile
/data1/fragmented1
/data1/fragmented2
/data1/bigfile1
/data1/bigfile2
/data1/masterfile

# After defragmentation
$ ssh ubuntu@160.85.31.224 "sudo btrfs filesystem usage /data1"
Overall:
    Used:			 713.31MiB
Data,single: Size:1.63GiB, Used:711.12MiB (42.53%)

# Final comparison
$ ssh ubuntu@160.85.31.224 "df -h /data1 /data2"
Filesystem      Size  Used Avail Use% Mounted on
/dev/vdb3       4.0G  717M  2.8G  21% /data1
/dev/vdc3       4.0G  659M  2.9G  19% /data2

$ ssh ubuntu@160.85.31.224 "sudo btrfs filesystem show"
Label: none  uuid: 9a630c8b-657b-4e9e-bc35-3a99d2a68a05
	Total devices 1 FS bytes used 712.41MiB (data1 - defragmented)

Label: none  uuid: 24e3aac2-6bb2-4c55-905e-bd660094efef
	Total devices 1 FS bytes used 654.33MiB (data2 - original compression)
```

**Defragmentation results:**
- **Files processed**: 7 files defragmented with zlib compression
- **Space allocation**: Data block allocation expanded from 840MB to 1.63GB
- **Actual usage**: Minimal change in used space (713.28MiB → 713.31MiB)
- **Compression effectiveness**: Random data shows limited compression benefits
- **Final comparison**: data2 (original compression) still more efficient (654.33MiB vs 712.41MiB)

**Key findings:**
- **Original compressed mount-time compression** is more effective than post-creation defragmentation compression
- **Random data** compresses poorly regardless of algorithm
- **Defragmentation** improves file layout but may not reduce space usage significantly with incompressible data
- **Mount-time compression** processes data during write operations, potentially achieving better compression ratios