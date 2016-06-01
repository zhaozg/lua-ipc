package = "luaipc"
version = "scm-0"
source = {
  url = "git://github.com/siffiejoe/lua-luaipc"
}
description = {
  summary = "Portable binding for various IPC mechanisms.",
  homepage = "https://github.com/siffiejoe/lua-luaipc/",
  license = "MIT"
}
-- we probably support more, but it lacks testing (and
-- probably the build details are different):
supported_platforms = { "linux", "windows", "freebsd", "macosx" }
dependencies = {
  "lua >= 5.1, < 5.4",
}

build = {
  type = "make",
  variables = {
    DLL_INSTALL_DIR = "$(LIBDIR)",
    LUA_INCDIR = "$(LUA_INCDIR)",
    CC = "$(CC)",
    CFLAGS = "$(CFLAGS) -DNDEBUG",
    LIBFLAG = "$(LIBFLAG)",
    LIB_EXTENSION = "$(LIB_EXTENSION)",
    EXTRAFLAGS = "",
    EXTRALIBS = "",
  },
  platforms = {
    linux = {
      variables = {
        EXTRAFLAGS = "-D_POSIX_C_SOURCE=200809L -pthread",
        EXTRALIBS = "-lrt"
      }
    },
    freebsd = {
      variables = {
        EXTRALIBS = "-lc"
      }
    },
    windows = {
      type = "builtin",
      modules = {
        ipc = {
          sources = {
            "ipc.c", "memfile.c", "strfile.c", "shm.c",
            "mmap.c", "sem.c", "flock.c", "proc.c",
          },
          defines = {
            "IPC_API=__declspec(dllexport)",
            --"NDEBUG",
            "_CRT_SECURE_NO_WARNINGS",
          },
        }
      }
    }
  }
}

