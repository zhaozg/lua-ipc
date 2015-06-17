package = "luaipc"
version = "scm-0"
source = {
  url = "${SRCURL}"
}
description = {
  summary = "Portable binding for various IPC mechanisms.",
  homepage = "${HPURL}",
  license = "MIT"
}
-- we probably support more, but it lacks testing (and
-- probably the build details are different):
supported_platforms = { "linux", "windows" }
dependencies = {
  "lua >= 5.1, < 5.4",
}

local win_defines = {
  "IPC_API=__declspec(dllexport)",
  "NDEBUG",
  "_CRT_SECURE_NO_WARNINGS",
}

build = {
  type = "make",
  variables = {
    DLL_INSTALL_DIR = "$(LIBDIR)",
    LUA_INCDIR = "$(LUA_INCDIR)",
    CC = "$(CC)",
    CFLAGS = "$(CFLAGS)",
    LIBFLAG = "$(LIBFLAG)",
    LIB_EXTENSION = "$(LIB_EXTENSION)",
    EXTRALIBS = "",
  },
  platforms = {
    linux = {
      variables = {
        EXTRALIBS = "-lrt"
      }
    },
    windows = {
      type = "builtin",
      modules = {
        [ "ipc.strfile" ] = {
          sources = { "strfile.c", "memfile.c", "ipc.c" },
          defines = win_defines,
        },
        [ "ipc.shm" ] = {
          sources = { "shm.c", "memfile.c", "ipc.c" },
          defines = win_defines,
        },
        [ "ipc.mmap" ] = {
          sources = { "mmap.c", "memfile.c", "ipc.c" },
          defines = win_defines,
        },
        [ "ipc.sem" ] = {
          sources = { "sem.c", "ipc.c" },
          defines = win_defines,
        },
        [ "ipc.filelock" ] = {
          sources = { "flock.c", "ipc.c" },
          defines = win_defines,
        },
      }
    }
  }
}

