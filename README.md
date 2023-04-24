# `stat-ln-deref`

A program that invokes `lstat()` on command-line arguments (assuming they're file paths); for symbolic links will recursively descend on de-referencing them until reaching actual file or directory.

The indentation level of console output reflects the level of recursion. The `lstat()` info returned on a de-referenced file is written to the console (also at appropriate indentation per the recursion level).

The implementation is adapted from the `stat/lstat/fstat` man page example.

The source code uses C++17 std::filesystem; is built with cmake; was tested on Linux - but not on Windows or MacOS.

This program is useful for investigating shared library usage, e.g:

```sh
$ ldd coroutines
	linux-vdso.so.1 (0x00007ffed3b3a000)
	libc++.so.1 => /lib/x86_64-linux-gnu/libc++.so.1 (0x00007f755e800000)
	libc++abi.so.1 => /lib/x86_64-linux-gnu/libc++abi.so.1 (0x00007f755e400000)
	libm.so.6 => /lib/x86_64-linux-gnu/libm.so.6 (0x00007f755e719000)
	libgcc_s.so.1 => /lib/x86_64-linux-gnu/libgcc_s.so.1 (0x00007f755eb37000)
	libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f755e1d8000)
	libpthread.so.0 => /lib/x86_64-linux-gnu/libpthread.so.0 (0x00007f755eb30000)
	librt.so.1 => /lib/x86_64-linux-gnu/librt.so.1 (0x00007f755eb2b000)
	libatomic.so.1 => /lib/x86_64-linux-gnu/libatomic.so.1 (0x00007f755eb21000)
	/lib64/ld-linux-x86-64.so.2 (0x00007f755eb89000)
```

So this program, `coroutines`, has been built with clang++/llvm and is dynamically linking the `libc++` standard library (as well as `libc++abi`). Can use this program to follow the symbolic links of these (and will follow on `libc.so.6` too):

```sh
$ ./stat-ln-deref /lib/x86_64-linux-gnu/libc++.so.1 /lib/x86_64-linux-gnu/libc++abi.so.1 /lib/x86_64-linux-gnu/libc.so.6
"/lib/x86_64-linux-gnu/libc++.so.1" ==>>
  symlink: "/lib/x86_64-linux-gnu/libc++.so.1"
    symlink: "/lib/x86_64-linux-gnu/libc++.so.1.0"
      regular file: "/usr/local/clang-16.0.0/lib/x86_64-unknown-linux-gnu/libc++.so.1.0"
      I-node number:            16290158
      Mode:                     100644 (octal)
      Link count:               1
      Ownership:                UID=0   GID=0
      Preferred I/O block size: 4096 bytes
      File size:                1267840 bytes
      Blocks allocated:         2480
      Last status change:       Wed Apr  5 17:00:26 2023
      Last file access:         Mon Apr 24 11:15:15 2023
      Last file modification:   Sat Mar 18 17:50:03 2023
"/lib/x86_64-linux-gnu/libc++abi.so.1" ==>>
  symlink: "/lib/x86_64-linux-gnu/libc++abi.so.1"
    symlink: "/lib/x86_64-linux-gnu/libc++abi.so.1.0"
      regular file: "/usr/local/clang-16.0.0/lib/x86_64-unknown-linux-gnu/libc++abi.so.1.0"
      I-node number:            16290150
      Mode:                     100644 (octal)
      Link count:               1
      Ownership:                UID=0   GID=0
      Preferred I/O block size: 4096 bytes
      File size:                364432 bytes
      Blocks allocated:         712
      Last status change:       Wed Apr  5 17:00:26 2023
      Last file access:         Mon Apr 24 11:15:15 2023
      Last file modification:   Sat Mar 18 17:49:54 2023
"/lib/x86_64-linux-gnu/libc.so.6" ==>>
  regular file: "/lib/x86_64-linux-gnu/libc.so.6"
  I-node number:            16260351
  Mode:                     100644 (octal)
  Link count:               1
  Ownership:                UID=0   GID=0
  Preferred I/O block size: 4096 bytes
  File size:                2216304 bytes
  Blocks allocated:         4336
  Last status change:       Sat Jul 30 12:05:36 2022
  Last file access:         Mon Apr 24 00:15:01 2023
  Last file modification:   Wed Jul  6 16:23:23 2022
```

Can see that the `libc.so.6` shared library goes straight to a regular file (i.e., no symbolic link indirection).