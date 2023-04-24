/** main.cpp
 *
 * Copyright 2023 Roger D. Voss
 *
 * Created by github user roger-dv on 09/28/2019.
 * Updated by github user roger-dv on 04/16/2023.
 *
 * Licensed under the MIT License - refer to LICENSE project document.
 *
 * A program that invokes lstat() on command-line arguments (assuming
 * they're file paths); for symbolic links will recursively descend on
 * de-referencing them until reaching actual file. The indentation level
 * of console output reflects the level of recursion. The lstat() info
 * returned on a de-referenced file is written to the console (also at
 * appropriate indentation per the recursion level).
 *
 * The implementation is adapted from the stat/lstat/fstat man page example.
 *
 * Code uses C++17 std::filesystem. Tested on Linux - but not on Windows.
*/
#include <cstdio>
#include <cstdlib>
#include <string_view>
#include <filesystem>
#include <sys/stat.h>
#include <unistd.h>
#include <ctime>
#include <cerrno>
#include <cstring>

namespace fs = std::filesystem;

static void stat_filepath(fs::path base_dir, std::string_view filepath, int depth = 2);

int main(int argc, const char **argv) {
  if (argc < 2) {
    puts("Expect one or more filepath arguments");
    return EXIT_FAILURE;
  }

  // each argument passed on command line is assumed to be a filepath to stat process
  for(int i = 1; i < argc; i++) {
    const std::string_view filepath{argv[i]};
    printf("\"%s\" ==>>\n", filepath.data());
    stat_filepath({}, filepath);
  }
}

static void stat_filepath(fs::path base_dir, std::string_view filepath, int depth) {
  static const char * const spc = "";

  std::string alt_full_path{}; // is used for when making second attempt lstat() call
  struct stat sb;
  for(int i = 2; i-- > 0;) { // will allow maximum of two attempts calling lstat()
    int rc = lstat(filepath.data(), &sb);
    if (rc == -1) {
      const auto ec = errno;
      if (ec == ENOENT && i > 0 && !base_dir.empty()) { // if filepath not found then try prepending base_dir path
        // if filepath is not a rooted path then proceed to prepend base_dir path
        if (const fs::path fs_filepath{filepath.data()}; !fs_filepath.has_root_path()) {
          base_dir /= fs_filepath;
          alt_full_path = base_dir.string();
          filepath = alt_full_path;
          continue; // try lstat() call again
        }
      }
      fprintf(stderr, "%*sERROR: %s(): on call to %s(): \"%s\"; ec=%d; %s\n",
              depth, spc, __FUNCTION__, "lstat", filepath.data(), ec, strerror(ec));
      return;
    }
    break; // lstat() call succeeded so break out of for loop
  }

  // what kind of filepath is it?
  switch (sb.st_mode & S_IFMT) {
    case S_IFBLK:  printf("%*sblock device: \"%s\"\n", depth, spc, filepath.data());      break;
    case S_IFCHR:  printf("%*scharacter device: \"%s\"\n", depth, spc, filepath.data());  break;
    case S_IFDIR:  printf("%*sdirectory: \"%s\"\n", depth, spc, filepath.data());         break;
    case S_IFIFO:  printf("%*sFIFO/pipe: \"%s\"\n", depth, spc, filepath.data());         break;
    case S_IFLNK: {
      // need special treatment of symbolic links to fully de-reference them
      printf("%*ssymlink: \"%s\"\n", depth, spc, filepath.data());
      const size_t full_buf_size = 2048;
      const size_t buf_size = full_buf_size - 1;
      char * const sym_ln_buf = reinterpret_cast<char*>(alloca(full_buf_size)); // stack allocate sym-link data buffer
      const size_t n = readlink(filepath.data(), sym_ln_buf, buf_size);
      if (n == static_cast<size_t>(-1)) {
        const auto ec = errno;
        fprintf(stderr, "%*sERROR: %s(): on call to %s(): \"%s\"; ec=%d; %s\n",
                depth, spc, __FUNCTION__, "readlink", filepath.data(), ec, strerror(ec));
      } else {
        sym_ln_buf[n] = '\0'; // insures is null char terminated C string
        if (n == buf_size) {
          fprintf(stderr, "%*sWARN: \"%s\" may be a truncated file name due to buffer size limit\n",
                  depth, spc, sym_ln_buf);
        }
        fs::path parent_dir_path{}; // empty path
        if (fs::path sym_ln_path{sym_ln_buf}; !sym_ln_path.has_root_path()) {
          fs::path fs_filepath{filepath.data()};
          if (fs_filepath.has_parent_path()) {
            parent_dir_path = fs_filepath.parent_path();
          }
        }
        // invoke recursively to fully de-reference symbolic links
        stat_filepath(std::move(parent_dir_path), {sym_ln_buf, n}, depth + 2);
      }
      return;
    }
    case S_IFREG:  printf("%*sregular file: \"%s\"\n", depth, spc, filepath.data());    break;
    case S_IFSOCK: printf("%*ssocket: \"%s\"\n", depth, spc, filepath.data());          break;
    default:       printf("%*sunknown?: \"%s\"\n", depth, spc, filepath.data());        break;
  }

  // print filepath stat details to console
  printf("%*sI-node number:            %ld\n", depth, spc, (long) sb.st_ino);
  printf("%*sMode:                     %lo (octal)\n", depth, spc, (unsigned long) sb.st_mode);
  printf("%*sLink count:               %ld\n", depth, spc, (long) sb.st_nlink);
  printf("%*sOwnership:                UID=%ld   GID=%ld\n", depth, spc, (long) sb.st_uid, (long) sb.st_gid);
  printf("%*sPreferred I/O block size: %ld bytes\n", depth, spc, (long) sb.st_blksize);
  printf("%*sFile size:                %lld bytes\n", depth, spc, (long long) sb.st_size);
  printf("%*sBlocks allocated:         %lld\n", depth, spc, (long long) sb.st_blocks);
  printf("%*sLast status change:       %s", depth, spc, ctime(&sb.st_ctime));
  printf("%*sLast file access:         %s", depth, spc, ctime(&sb.st_atime));
  printf("%*sLast file modification:   %s", depth, spc, ctime(&sb.st_mtime));
}