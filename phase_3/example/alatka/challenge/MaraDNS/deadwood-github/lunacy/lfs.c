/*
** LuaFileSystem
** Copyright Kepler Project 2003 - 2020
** (http://keplerproject.github.io/luafilesystem)
** Changes by Sam Trenholme for lunacy
**
** File system manipulation library.
** This library offers some (but **NOT** all) of these functions:
**   lfs.attributes (filepath [, attributename | attributetable])
**   lfs.chdir (path)
**   lfs.currentdir ()
**   lfs.dir (path)
**   lfs.link (old, new[, symlink])
**   lfs.lock (fh, mode)
**   lfs.lock_dir (path)
**   lfs.mkdir (path)
**   lfs.rmdir (path)
**   lfs.setmode (filepath, mode)
**   lfs.symlinkattributes (filepath [, attributename])
**   lfs.touch (filepath [, atime [, mtime]])
**   lfs.unlock (fh)
*/

#ifndef LFS_DO_NOT_USE_LARGE_FILE
#ifndef _WIN32
#ifndef _AIX
#define _FILE_OFFSET_BITS 64    /* Linux, Solaris and HP-UX */
#else
#define _LARGE_FILES 1          /* AIX */
#endif
#endif
#endif

#ifdef _WIN32
#define _WIN32_WINNT 0x600
#endif

#ifndef LFS_DO_NOT_USE_LARGE_FILE
#define _LARGEFILE64_SOURCE
#endif

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>

#ifdef _WIN32

#include <direct.h>
#include <windows.h>
#include <io.h>
#include <sys/locking.h>

#ifdef __BORLANDC__
#include <utime.h>
#else
#include <sys/utime.h>
#endif

#include <fcntl.h>

/* MAX_PATH seems to be 260. Seems kind of small. Is there a better one? */
#define LFS_MAXPATHLEN MAX_PATH

#else

#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <utime.h>
#include <sys/param.h>          /* for MAXPATHLEN */

#ifdef MAXPATHLEN
#define LFS_MAXPATHLEN MAXPATHLEN
#else
#include <limits.h>             /* for _POSIX_PATH_MAX */
#define LFS_MAXPATHLEN _POSIX_PATH_MAX
#endif

#endif

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "lfs.h"

#define LFS_VERSION "1.8.0"
#define LFS_LIBNAME "lfs"

#if LUA_VERSION_NUM >= 503      /* Lua 5.3+ */

#ifndef luaL_optlong
#define luaL_optlong luaL_optinteger
#endif

#endif

#if LUA_VERSION_NUM >= 502
#define new_lib(L, l) (luaL_newlib(L, l))
#else
#define new_lib(L, l) (lua_newtable(L), luaL_register(L, NULL, l))
#endif

/* Define 'strerror' for systems that do not implement it */
#ifdef NO_STRERROR
#define strerror(_)     "System unable to describe the error"
#endif

#define DIR_METATABLE "directory metatable"
typedef struct dir_data {
  int closed;
#ifdef _WIN32
  int hFile;
  char pattern[MAX_PATH + 1];
#else
  DIR *dir;
#endif
} dir_data;

#define LOCK_METATABLE "lock metatable"

#ifdef _WIN32

#ifdef __BORLANDC__
#define lfs_setmode(file, m)   (setmode(_fileno(file), m))
#define STAT_STRUCT struct stati64
#else
#define lfs_setmode(file, m)   (_setmode(_fileno(file), m))
#define STAT_STRUCT struct _stati64
#endif

#ifndef _S_IFLNK
#define _S_IFLNK 0x400
#endif

#ifndef S_ISDIR
#define S_ISDIR(mode)  (mode&_S_IFDIR)
#endif
#ifndef S_ISREG
#define S_ISREG(mode)  (mode&_S_IFREG)
#endif
#ifndef S_ISLNK
#define S_ISLNK(mode)  (mode&_S_IFLNK)
#endif
#ifndef S_ISSOCK
#define S_ISSOCK(mode)  (0)
#endif
#ifndef S_ISFIFO
#define S_ISFIFO(mode)  (0)
#endif
#ifndef S_ISCHR
#define S_ISCHR(mode)  (mode&_S_IFCHR)
#endif
#ifndef S_ISBLK
#define S_ISBLK(mode)  (0)
#endif

#define STAT_FUNC _stati64
#define LSTAT_FUNC lfs_win32_lstat

#else

#define _O_TEXT               0
#define _O_BINARY             0
#define lfs_setmode(file, m)   ((void)file, (void)m, 0)
#define STAT_STRUCT struct stat
#define STAT_FUNC stat
#define LSTAT_FUNC lstat

#endif

#ifdef _WIN32
#define lfs_mkdir _mkdir
#else
#define lfs_mkdir(path) (mkdir((path), \
    S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH))
#endif

#ifdef _WIN32

int lfs_win32_pusherror(lua_State * L)
{
  int en = GetLastError();
  lua_pushnil(L);
  if (en == ERROR_FILE_EXISTS || en == ERROR_SHARING_VIOLATION)
    lua_pushstring(L, "File exists");
  else
    lua_pushstring(L, strerror(en));
  return 2;
}

#define TICKS_PER_SECOND 10000000
#define EPOCH_DIFFERENCE 11644473600LL
time_t windowsToUnixTime(FILETIME ft)
{
  ULARGE_INTEGER uli;
  uli.LowPart = ft.dwLowDateTime;
  uli.HighPart = ft.dwHighDateTime;
  return (time_t) (uli.QuadPart / TICKS_PER_SECOND - EPOCH_DIFFERENCE);
}

int lfs_win32_lstat(const char *path, STAT_STRUCT * buffer)
{
  WIN32_FILE_ATTRIBUTE_DATA win32buffer;
  if (GetFileAttributesEx(path, GetFileExInfoStandard, &win32buffer)) {
    if (!(win32buffer.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
      return STAT_FUNC(path, buffer);
    }
    buffer->st_mode = _S_IFLNK;
    buffer->st_dev = 0;
    buffer->st_ino = 0;
    buffer->st_nlink = 0;
    buffer->st_uid = 0;
    buffer->st_gid = 0;
    buffer->st_rdev = 0;
    buffer->st_atime = windowsToUnixTime(win32buffer.ftLastAccessTime);
    buffer->st_mtime = windowsToUnixTime(win32buffer.ftLastWriteTime);
    buffer->st_ctime = windowsToUnixTime(win32buffer.ftCreationTime);
    buffer->st_size = 0;
    return 0;
  } else {
    return 1;
  }
}

#endif

/*
** Utility functions
*/
static int pusherror(lua_State * L, const char *info)
{
  lua_pushnil(L);
  if (info == NULL)
    lua_pushstring(L, strerror(errno));
  else
    lua_pushfstring(L, "%s: %s", info, strerror(errno));
  lua_pushinteger(L, errno);
  return 3;
}

static int pushresult(lua_State * L, int res, const char *info)
{
  if (res == -1) {
    return pusherror(L, info);
  } else {
    lua_pushboolean(L, 1);
    return 1;
  }
}


/*
** This function changes the working (current) directory
*/
static int change_dir(lua_State * L)
{
  const char *path = luaL_checkstring(L, 1);
  if (chdir(path)) {
    lua_pushnil(L);
    lua_pushfstring(L, "Unable to change working directory to '%s'\n%s\n",
                    path, chdir_error);
    return 2;
  } else {
    lua_pushboolean(L, 1);
    return 1;
  }
}

/*
** This function returns the current directory
** If unable to get the current directory, it returns nil
**  and a string describing the error
*/
static int get_dir(lua_State * L)
{
#ifdef NO_GETCWD
  lua_pushnil(L);
  lua_pushstring(L, "Function 'getcwd' not provided by system");
  return 2;
#else
  char *path = NULL;
  /* Passing (NULL, 0) is not guaranteed to work.
     Use a temp buffer and size instead. */
  size_t size = LFS_MAXPATHLEN; /* initial buffer size */
  int result;
  while (1) {
    char *path2 = realloc(path, size);
    if (!path2) {               /* failed to allocate */
      result = pusherror(L, "get_dir realloc() failed");
      break;
    }
    path = path2;
    if (getcwd(path, size) != NULL) {
      /* success, push the path to the Lua stack */
      lua_pushstring(L, path);
      result = 1;
      break;
    }
    if (errno != ERANGE) {      /* unexpected error */
      result = pusherror(L, "get_dir getcwd() failed");
      break;
    }
    /* ERANGE = insufficient buffer capacity, double size and retry */
    size *= 2;
  }
  free(path);
  return result;
#endif
}

/*
** Creates a directory.
** @param #1 Directory path.
*/
static int make_dir(lua_State * L)
{
  const char *path = luaL_checkstring(L, 1);
  return pushresult(L, lfs_mkdir(path), NULL);
}


/*
** Removes a directory.
** @param #1 Directory path.
*/
static int remove_dir(lua_State * L)
{
  const char *path = luaL_checkstring(L, 1);
  return pushresult(L, rmdir(path), NULL);
}


/*
** Directory iterator
*/
static int dir_iter(lua_State * L)
{
#ifdef _WIN32
  struct _finddata_t c_file;
#else
  struct dirent *entry;
#endif
  dir_data *d = (dir_data *) luaL_checkudata(L, 1, DIR_METATABLE);
  luaL_argcheck(L, d->closed == 0, 1, "closed directory");
#ifdef _WIN32
  if (d->hFile == 0L) {         /* first entry */
    if ((d->hFile = _findfirst(d->pattern, &c_file)) == -1L) {
      lua_pushnil(L);
      lua_pushstring(L, strerror(errno));
      d->closed = 1;
      return 2;
    } else {
      lua_pushstring(L, c_file.name);
      return 1;
    }
  } else {                      /* next entry */
    if (_findnext(d->hFile, &c_file) == -1L) {
      /* no more entries => close directory */
      _findclose(d->hFile);
      d->closed = 1;
      return 0;
    } else {
      lua_pushstring(L, c_file.name);
      return 1;
    }
  }
#else
  if ((entry = readdir(d->dir)) != NULL) {
    lua_pushstring(L, entry->d_name);
    return 1;
  } else {
    /* no more entries => close directory */
    closedir(d->dir);
    d->closed = 1;
    return 0;
  }
#endif
}


/*
** Closes directory iterators
*/
static int dir_close(lua_State * L)
{
  dir_data *d = (dir_data *) lua_touserdata(L, 1);
#ifdef _WIN32
  if (!d->closed && d->hFile) {
    _findclose(d->hFile);
  }
#else
  if (!d->closed && d->dir) {
    closedir(d->dir);
  }
#endif
  d->closed = 1;
  return 0;
}


/*
** Factory of directory iterators
*/
static int dir_iter_factory(lua_State * L)
{
  const char *path = luaL_checkstring(L, 1);
  dir_data *d;
  lua_pushcfunction(L, dir_iter);
  d = (dir_data *) lua_newuserdata(L, sizeof(dir_data));
  luaL_getmetatable(L, DIR_METATABLE);
  lua_setmetatable(L, -2);
  d->closed = 0;
#ifdef _WIN32
  d->hFile = 0L;
  if (strlen(path) > MAX_PATH - 2)
    luaL_error(L, "path too long: %s", path);
  else
    sprintf(d->pattern, "%s/*", path);
#else
  d->dir = opendir(path);
  if (d->dir == NULL)
    luaL_error(L, "cannot open %s: %s", path, strerror(errno));
#endif
#if LUA_VERSION_NUM >= 504
  lua_pushnil(L);
  lua_pushvalue(L, -2);
  return 4;
#else
  return 2;
#endif
}


/*
** Creates directory metatable.
*/
static int dir_create_meta(lua_State * L)
{
  luaL_newmetatable(L, DIR_METATABLE);

  /* Method table */
  lua_newtable(L);
  lua_pushcfunction(L, dir_iter);
  lua_setfield(L, -2, "next");
  lua_pushcfunction(L, dir_close);
  lua_setfield(L, -2, "close");

  /* Metamethods */
  lua_setfield(L, -2, "__index");
  lua_pushcfunction(L, dir_close);
  lua_setfield(L, -2, "__gc");

#if LUA_VERSION_NUM >= 504
  lua_pushcfunction(L, dir_close);
  lua_setfield(L, -2, "__close");
#endif
  return 1;
}


/*
** Convert the inode protection mode to a string.
*/
#ifdef _WIN32
static const char *mode2string(unsigned short mode)
{
#else
static const char *mode2string(mode_t mode)
{
#endif
  if (S_ISREG(mode))
    return "file";
  else if (S_ISDIR(mode))
    return "directory";
  else if (S_ISLNK(mode))
    return "link";
  else if (S_ISSOCK(mode))
    return "socket";
  else if (S_ISFIFO(mode))
    return "named pipe";
  else if (S_ISCHR(mode))
    return "char device";
  else if (S_ISBLK(mode))
    return "block device";
  else
    return "other";
}


/* inode protection mode */
static void push_st_mode(lua_State * L, STAT_STRUCT * info)
{
  lua_pushstring(L, mode2string(info->st_mode));
}

/* number of hard links to the file */
static void push_st_nlink(lua_State * L, STAT_STRUCT * info)
{
  lua_pushinteger(L, (lua_Integer) info->st_nlink);
}

/* file size, in bytes */
static void push_st_size(lua_State * L, STAT_STRUCT * info)
{
  lua_pushinteger(L, (lua_Integer) info->st_size);
}

typedef void (*_push_function)(lua_State * L, STAT_STRUCT * info);

struct _stat_members {
  const char *name;
  _push_function push;
};

struct _stat_members members[] = {
  { "mode", push_st_mode },
  //{ "dev", push_st_dev },
  { "nlink", push_st_nlink },
  //{ "uid", push_st_uid },
  //{ "gid", push_st_gid },
  { "size", push_st_size },
  //{ "permissions", push_st_perm },
#ifndef _WIN32
  //{ "blocks", push_st_blocks },
  //{ "blksize", push_st_blksize },
#endif
  { NULL, NULL }
};

/*
** Get file or symbolic link information
*/
static int _file_info_(lua_State * L,
                       int (*st)(const char *, STAT_STRUCT *))
{
  STAT_STRUCT info;
  const char *file = luaL_checkstring(L, 1);
  int i;

  if (st(file, &info)) {
    lua_pushnil(L);
    lua_pushfstring(L, "cannot obtain information from file '%s': %s",
                    file, strerror(errno));
    lua_pushinteger(L, errno);
    return 3;
  }
  if (lua_isstring(L, 2)) {
    const char *member = lua_tostring(L, 2);
    for (i = 0; members[i].name; i++) {
      if (strcmp(members[i].name, member) == 0) {
        /* push member value and return */
        members[i].push(L, &info);
        return 1;
      }
    }
    /* member not found */
    return luaL_error(L, "invalid attribute name '%s'", member);
  }
  /* creates a table if none is given, removes extra arguments */
  lua_settop(L, 2);
  if (!lua_istable(L, 2)) {
    lua_newtable(L);
  }
  /* stores all members in table on top of the stack */
  for (i = 0; members[i].name; i++) {
    lua_pushstring(L, members[i].name);
    members[i].push(L, &info);
    lua_rawset(L, -3);
  }
  return 1;
}


/*
** Get file information using stat.
*/
static int file_info(lua_State * L)
{
  return _file_info_(L, STAT_FUNC);
}


/*
** Get symbolic link information using lstat.
*/
static int link_info(lua_State * L)
{
  int ret;
  if (lua_isstring(L, 2) && (strcmp(lua_tostring(L, 2), "target") == 0)) {
    /*int ok = push_link_target(L);*/
    int ok = 0;
    return ok ? 1 : pusherror(L, "could not obtain link target");
  }
  ret = _file_info_(L, LSTAT_FUNC);
  if (ret == 1 && lua_type(L, -1) == LUA_TTABLE) {
    /*int ok = push_link_target(L);*/
    int ok = 0;
    if (ok) {
      lua_setfield(L, -2, "target");
    }
  }
  return ret;
}


/*
** Assumes the table is on top of the stack.
*/
static void set_info(lua_State * L)
{
  lua_pushliteral(L, "Copyright (C) 2003-2017 Kepler Project");
  lua_setfield(L, -2, "_COPYRIGHT");
  lua_pushliteral(L,
                  "LuaFileSystem is a Lua library developed to complement "
                  "the set of functions related to file systems offered by "
                  "the standard Lua distribution");
  lua_setfield(L, -2, "_DESCRIPTION");
  lua_pushliteral(L, "LuaFileSystem " LFS_VERSION);
  lua_setfield(L, -2, "_VERSION");
}


static const struct luaL_Reg fslib[] = {
  { "attributes", file_info },
  { "chdir", change_dir },
  { "currentdir", get_dir },
  { "dir", dir_iter_factory },
  { "mkdir", make_dir },
  { "rmdir", remove_dir },
  { "symlinkattributes", link_info },
  { NULL, NULL },
};

LUALIB_API int luaopen_lfs(lua_State * L)
{
  dir_create_meta(L);
  new_lib(L, fslib);
  lua_pushvalue(L, -1);
  lua_setglobal(L, LFS_LIBNAME);
  set_info(L);
  return 1;
}
