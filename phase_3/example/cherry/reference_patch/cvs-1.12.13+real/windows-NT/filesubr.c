/* filesubr.c --- subroutines for dealing with files
   Jim Blandy <jimb@cyclic.com>

   This file is part of GNU CVS.

   GNU CVS is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.  */

/* These functions were moved out of subr.c because they need different
   definitions under operating systems (like, say, Windows NT) with different
   file system semantics.  */

#include <assert.h>
#include <io.h>
#include <sys/socket.h>  /* This does: #include <windows.h> */

#include "cvs.h"
#include "setenv.h"

#include "JmgStat.h"

#undef mkdir

static int deep_remove_dir( const char *path );

/* Copies "from" to "to".  Note that the functionality here is similar
   to the win32 function CopyFile, but (1) we copy LastAccessTime and
   CopyFile doesn't, (2) we set file attributes to the default set by
   the C library and CopyFile copies them.  Neither #1 nor #2 was intentional
   as far as I know, but changing them could be confusing, unless there
   is some reason they should be changed (this would need more
   investigation).  */
void
copy_file (from, to)
    const char *from;
    const char *to;
{
    struct stat sb;
    struct utimbuf t;
    int fdin, fdout;

    if (trace)
#ifdef SERVER_SUPPORT
	(void) fprintf (stderr, "%c-> copy(%s,%s)\n",
			(server_active) ? 'S' : ' ', from, to);
#else
	(void) fprintf (stderr, "-> copy(%s,%s)\n", from, to);
#endif
    if (noexec)
	return;

    if ((fdin = open (from, O_RDONLY | O_BINARY)) < 0)
	error (1, errno, "cannot open %s for copying", from);
    if (fstat (fdin, &sb) < 0)
	error (1, errno, "cannot fstat %s", from);
    if ((fdout = open (to, O_CREAT | O_TRUNC | O_RDWR | O_BINARY,
		       (int) sb.st_mode & 07777)) < 0)
	error (1, errno, "cannot create %s for copying", to);
    if (sb.st_size > 0)
    {
	char buf[BUFSIZ];
	int n;

	for (;;) 
	{
	    n = read (fdin, buf, sizeof(buf));
	    if (n == -1)
	    {
#ifdef EINTR
		if (errno == EINTR)
		    continue;
#endif
		error (1, errno, "cannot read file %s for copying", from);
	    }
            else if (n == 0) 
		break;
  
	    if (write(fdout, buf, n) != n) {
		error (1, errno, "cannot write file %s for copying", to);
	    }
	}

#ifdef HAVE_FSYNC
	if (fsync (fdout)) 
	    error (1, errno, "cannot fsync file %s after copying", to);
#endif
    }

    if (close (fdin) < 0) 
	error (0, errno, "cannot close %s", from);
    if (close (fdout) < 0)
	error (1, errno, "cannot close %s", to);

    /* now, set the times for the copied file to match those of the original */
    memset ((char *) &t, 0, sizeof (t));
    t.actime = sb.st_atime;
    t.modtime = sb.st_mtime;
    (void) utime (to, &t);
}


static char *tmpdir_env;
/*
 * Return seperator (\) terminated path to system temporary directory.
 */
const char *
get_system_temp_dir (void)
{
	if (! tmpdir_env)
	{
		DWORD dwBufferSize, dwReturn;

		dwReturn = 0;
		dwBufferSize = 64;
		do {
			if (dwReturn >= dwBufferSize)
			{
				dwBufferSize = dwReturn + 4;
			}

			tmpdir_env = xrealloc (tmpdir_env, dwBufferSize);
			if (tmpdir_env)
			{
				dwReturn = GetTempPath (dwBufferSize, tmpdir_env);
				if (dwReturn <= 0)
				{
					free (tmpdir_env);
					tmpdir_env = NULL;
				}
			}
		} while (tmpdir_env && dwReturn >= dwBufferSize);
	}

	return tmpdir_env;
}


void
push_env_temp_dir (void)
{
	const char *tmpdir = get_cvs_tmp_dir ();

	if (tmpdir_env && strcmp (tmpdir_env, tmpdir))
		setenv ("TMP", tmpdir, 1);
}


/* FIXME-krp: these functions would benefit from caching the char * &
   stat buf.  */

/*
 * Returns non-zero if the argument file is a directory, or is a symbolic
 * link which points to a directory.
 */
int
isdir (file)
    const char *file;
{
    struct stat sb;

    if (stat (file, &sb) < 0)
	return (0);
    return (S_ISDIR (sb.st_mode));
}

/*
 * Returns non-zero if the argument file is a symbolic link.
 */
int
islink (file)
    const char *file;
{
#ifdef S_ISLNK
    struct stat sb;

    if (lstat (file, &sb) < 0)
	return (0);
    return (S_ISLNK (sb.st_mode));
#else
    return (0);
#endif
}

/*
 * Returns non-zero if the argument file exists.
 */
int
isfile (file)
    const char *file;
{
    return isaccessible(file, F_OK);
}

/*
 * Returns non-zero if the argument file is readable.
 */
int
isreadable (file)
    const char *file;
{
    return isaccessible(file, R_OK);
}

/*
 * Returns non-zero if the argument file is writable.
 */
int
iswritable (file)
    const char *file;
{
    return isaccessible(file, W_OK);
}

/*
 * Returns non-zero if the argument file is accessable according to
 * mode.  If compiled with SETXID_SUPPORT also works if cvs has setxid
 * bits set.
 */
int
isaccessible (file, mode)
    const char *file;
    const int mode;
{
#ifdef SETXID_SUPPORT
    struct stat sb;
    int umask = 0;
    int gmask = 0;
    int omask = 0;
    int uid;
    
    if (stat(file, &sb) == -1)
	return 0;
    if (mode == F_OK)
	return 1;

    uid = geteuid();
    if (uid == 0)		/* superuser */
    {
	if (mode & X_OK)
	    return sb.st_mode & (S_IXUSR|S_IXGRP|S_IXOTH);
	else
	    return 1;
    }
	
    if (mode & R_OK)
    {
	umask |= S_IRUSR;
	gmask |= S_IRGRP;
	omask |= S_IROTH;
    }
    if (mode & W_OK)
    {
	umask |= S_IWUSR;
	gmask |= S_IWGRP;
	omask |= S_IWOTH;
    }
    if (mode & X_OK)
    {
	umask |= S_IXUSR;
	gmask |= S_IXGRP;
	omask |= S_IXOTH;
    }

    if (sb.st_uid == uid)
	return (sb.st_mode & umask) == umask;
    else if (sb.st_gid == getegid())
	return (sb.st_mode & gmask) == gmask;
    else
	return (sb.st_mode & omask) == omask;
#else
    return access(file, mode) == 0;
#endif
}



/*
 * Make a directory and die if it fails
 */
void
make_directory (name)
    const char *name;
{
    struct stat sb;

    if (stat (name, &sb) == 0 && (!S_ISDIR (sb.st_mode)))
	    error (0, 0, "%s already exists but is not a directory", name);
    if (!noexec && mkdir (name) < 0)
	error (1, errno, "cannot make directory %s", name);
}

/*
 * Make a path to the argument directory, printing a message if something
 * goes wrong.
 */
void
make_directories (name)
    const char *name;
{
    char *cp;

    if (noexec)
	return;

    if (mkdir (name) == 0 || errno == EEXIST)
	return;
    if (errno != ENOENT)
    {
	error (0, errno, "cannot make path to %s", name);
	return;
    }
    if ((cp = strrchr (name, '/')) == NULL)
	return;
    *cp = '\0';
    make_directories (name);
    *cp++ = '/';
    if (*cp == '\0')
	return;
    (void) mkdir (name);
}

/* Create directory NAME if it does not already exist; fatal error for
   other errors.  Returns 0 if directory was created; 1 if it already
   existed.  */
int
mkdir_if_needed (name)
    const char *name;
{
    if (mkdir (name) < 0)
    {
	if (errno != EEXIST
#ifdef EACCESS
	    /* This was copied over from the OS/2 code; I would guess it
	       isn't needed here but that has not been verified.  */
	    && errno != EACCESS
#endif
#ifdef EACCES
	    /* This is said to be needed by NT on Alpha or PowerPC
	       (not sure what version) --August, 1996.  */
	    && errno != EACCES
#endif
	    )
	    error (1, errno, "cannot make directory %s", name);
	return 1;
    }
    return 0;
}

/*
 * Change the mode of a file, either adding write permissions, or removing
 * all write permissions.  Adding write permissions honors the current umask
 * setting.
 */
void
xchmod (fname, writable)
    const char *fname;
    int writable;
{
    struct stat sb;
    mode_t mode, oumask;

    if (stat (fname, &sb) < 0)
    {
	if (!noexec)
	    error (0, errno, "cannot stat %s", fname);
	return;
    }
    if (writable)
    {
	oumask = umask (0);
	(void) umask (oumask);
	mode = sb.st_mode | ~oumask & (((sb.st_mode & S_IRUSR) ? S_IWUSR : 0) |
				       ((sb.st_mode & S_IRGRP) ? S_IWGRP : 0) |
				       ((sb.st_mode & S_IROTH) ? S_IWOTH : 0));
    }
    else
    {
	mode = sb.st_mode & ~(S_IWRITE | S_IWGRP | S_IWOTH);
    }

    if (trace)
#ifdef SERVER_SUPPORT
	(void) fprintf (stderr, "%c-> chmod(%s,%o)\n",
			(server_active) ? 'S' : ' ', fname, mode);
#else
	(void) fprintf (stderr, "-> chmod(%s,%o)\n", fname, mode);
#endif
    if (noexec)
	return;

    if (chmod (fname, mode) < 0)
	error (0, errno, "cannot change mode of file %s", fname);
}


/* Rename for NT which works for read only files.  Apparently if we are
   accessing FROM and TO via a Novell network, this is an issue.  */
int
wnt_rename (from, to)
    const char *from;
    const char *to;
{
    int result, save_errno;
    int readonly = !iswritable (from);

    if (readonly)
    {
	if (chmod (from, S_IWRITE) < 0)
	    return -1;
    }
    result = rename (from, to);
    save_errno = errno;
    if (readonly)
    {
	if (result == 0)
	{
	    if (chmod (to, S_IREAD) < 0)
		return -1;
	}
	else
	{
	    /* We have a choice of which error to report, if there is
	       one here too; report the one from rename ().  */
	    chmod (from, S_IREAD);
	}
	errno = save_errno;
    }
    return result;
}

/*
 * Rename a file and die if it fails
 */
void
rename_file (from, to)
    const char *from;
    const char *to;
{
    if (trace)
#ifdef SERVER_SUPPORT
	(void) fprintf (stderr, "%c-> rename(%s,%s)\n",
			(server_active) ? 'S' : ' ', from, to);
#else
	(void) fprintf (stderr, "-> rename(%s,%s)\n", from, to);
#endif
    if (noexec)
	return;

    /* Win32 unlink is stupid --- it fails if the file is read-only  */
    chmod(to, S_IWRITE);
    unlink(to);
    if (CVS_RENAME (from, to) < 0)
	error (1, errno, "cannot rename file %s to %s", from, to);
}

/*
 * unlink a file, if possible.
 */
int
unlink_file (f)
    const char *f;
{
    if (trace)
#ifdef SERVER_SUPPORT
	(void) fprintf (stderr, "%c-> unlink(%s)\n",
			(server_active) ? 'S' : ' ', f);
#else
	(void) fprintf (stderr, "-> unlink(%s)\n", f);
#endif
    if (noexec)
	return (0);

    /* Win32 unlink is stupid - it fails if the file is read-only */
    chmod (f, _S_IWRITE);
    return (unlink (f));
}

/*
 * Unlink a file or dir, if possible.  If it is a directory do a deep
 * removal of all of the files in the directory.  Return -1 on error
 * (in which case errno is set).
 */
int
unlink_file_dir (f)
    const char *f;
{
    if (trace)
#ifdef SERVER_SUPPORT
	(void) fprintf (stderr, "%c-> unlink_file_dir(%s)\n",
			(server_active) ? 'S' : ' ', f);
#else
	(void) fprintf (stderr, "-> unlink_file_dir(%s)\n", f);
#endif
    if (noexec)
	return (0);

    /* Win32 unlink is stupid - it fails if the file is read-only */
    chmod (f, _S_IWRITE);
    if (unlink (f) != 0)
    {
	/* Under Windows NT, unlink returns EACCES if the path
           is a directory.  Under Windows 95, it returns ENOENT.
           Under Windows XP, it can return ENOTEMPTY. */
        if (errno == EISDIR || errno == EACCES || errno == ENOENT
            || errno == ENOTEMPTY)
                return deep_remove_dir (f);
        else
		/* The file wasn't a directory and some other
		 * error occured
		 */
                return -1;
    }
    /* We were able to remove the file from the disk */
    return 0;
}

/* Remove a directory and everything it contains.  Returns 0 for
 * success, -1 for failure (in which case errno is set).
 */

static int
deep_remove_dir (path)
    const char *path;
{
    DIR		  *dirp;
    struct dirent *dp;
    char	   buf[PATH_MAX];

    /* ENOTEMPTY for NT (obvious) but EACCES for Win95 (not obvious) */
    if (rmdir (path) != 0)
    {
	if (errno == ENOTEMPTY || errno == EACCES)
	{
	    if ((dirp = CVS_OPENDIR (path)) == NULL)
		/* If unable to open the directory return
		 * an error
		 */
		return -1;

	    while ((dp = CVS_READDIR (dirp)) != NULL)
	    {
		if (strcmp (dp->d_name, ".") == 0 ||
			    strcmp (dp->d_name, "..") == 0)
		    continue;

		sprintf (buf, "%s/%s", path, dp->d_name);

		/* Win32 unlink is stupid - it fails if the file is read-only */
		chmod (buf, _S_IWRITE);
		if (unlink (buf) != 0 )
		{
		    /* Under Windows NT, unlink returns EACCES if the path
		     * is a directory.  Under Windows 95, it returns ENOENT.
                     * Under Windows XP, it can return ENOTEMPTY.  It
		     * isn't really clear to me whether checking errno is
		     * better or worse than using _stat to check for a
                     * directory.
		     * We aren't really trying to prevent race conditions here
		     * (e.g. what if something changes between readdir and
		     * unlink?)
                     */
		    if (errno == EISDIR || errno == EACCES || errno == ENOENT
                        || errno == ENOTEMPTY)
		    {
			if (deep_remove_dir (buf))
			{
			    closedir (dirp);
			    return -1;
			}
		    }
		    else
		    {
			/* buf isn't a directory, or there are
			 * some sort of permision problems
			 */
			CVS_CLOSEDIR (dirp);
			return -1;
		    }
		}
	    }
	    CVS_CLOSEDIR (dirp);
	    return rmdir (path);
	}
	else
	    return -1;
    }
    /* Was able to remove the directory return 0 */
    return 0;
}

/* Read NCHARS bytes from descriptor FD into BUF.
   Return the number of characters successfully read.
   The number returned is always NCHARS unless end-of-file or error.  */
static size_t
block_read (fd, buf, nchars)
    int fd;
    char *buf;
    size_t nchars;
{
    char *bp = buf;
    size_t nread;

    do 
    {
	nread = read (fd, bp, nchars);
	if (nread == (size_t)-1)
	{
#ifdef EINTR
	    if (errno == EINTR)
		continue;
#endif
	    return (size_t)-1;
	}

	if (nread == 0)
	    break; 

	bp += nread;
	nchars -= nread;
    } while (nchars != 0);

    return bp - buf;
} 

    
/*
 * Compare "file1" to "file2". Return non-zero if they don't compare exactly.
 */
int
xcmp (file1, file2)
    const char *file1;
    const char *file2;
{
    char *buf1, *buf2;
    struct stat sb1, sb2;
    int fd1, fd2;
    int ret;

    if ((fd1 = open (file1, O_RDONLY | O_BINARY)) < 0)
	error (1, errno, "cannot open file %s for comparing", file1);
    if ((fd2 = open (file2, O_RDONLY | O_BINARY)) < 0)
	error (1, errno, "cannot open file %s for comparing", file2);
    if (fstat (fd1, &sb1) < 0)
	error (1, errno, "cannot fstat %s", file1);
    if (fstat (fd2, &sb2) < 0)
	error (1, errno, "cannot fstat %s", file2);

    /* A generic file compare routine might compare st_dev & st_ino here 
       to see if the two files being compared are actually the same file.
       But that won't happen in CVS, so we won't bother. */

    if (sb1.st_size != sb2.st_size)
	ret = 1;
    else if (sb1.st_size == 0)
	ret = 0;
    else
    {
	/* FIXME: compute the optimal buffer size by computing the least
	   common multiple of the files st_blocks field */
	size_t buf_size = 8 * 1024;
	size_t read1;
	size_t read2;

	buf1 = xmalloc (buf_size);
	buf2 = xmalloc (buf_size);

	do 
	{
	    read1 = block_read (fd1, buf1, buf_size);
	    if (read1 == (size_t)-1)
		error (1, errno, "cannot read file %s for comparing", file1);

	    read2 = block_read (fd2, buf2, buf_size);
	    if (read2 == (size_t)-1)
		error (1, errno, "cannot read file %s for comparing", file2);

	    /* assert (read1 == read2); */

	    ret = memcmp(buf1, buf2, read1);
	} while (ret == 0 && read1 == buf_size);

	free (buf1);
	free (buf2);
    }
	
    (void) close (fd1);
    (void) close (fd2);
    return (ret);
}

/* Generate a unique temporary filename.  Returns a pointer to a newly
 * malloc'd string containing the name.  Returns successfully or not at
 * all.
 *
 *     THIS FUNCTION IS DEPRECATED!!!  USE cvs_temp_file INSTEAD!!!
 *
 * and yes, I know about the way the rcs commands use temp files.  I think
 * they should be converted too but I don't have time to look into it right
 * now.
 */
char *
cvs_temp_name ()
{
    char *fn;
    FILE *fp;

    fp = cvs_temp_file (&fn);
    if (fp == NULL)
	error (1, errno, "Failed to create temporary file");
    if (fclose (fp) == EOF)
	error (0, errno, "Failed to close temporary file %s", fn);
    return fn;
}

/* Generate a unique temporary filename and return an open file stream
 * to the truncated file by that name
 *
 *  INPUTS
 *	filename	where to place the pointer to the newly allocated file
 *   			name string
 *
 *  OUTPUTS
 *	filename	dereferenced, will point to the newly allocated file
 *			name string.  This value is undefined if the function
 *			returns an error.
 *
 *  RETURNS
 *	An open file pointer to a read/write mode empty temporary file with the
 *	unique file name or NULL on failure.
 *
 *  ERRORS
 *	on error, errno will be set to some value either by CVS_FOPEN or
 *	whatever system function is called to generate the temporary file name
 */
/* FIXME: This should use the mkstemp() function from the lib/mkstemp.c file
 * from the GNULIB project.
 */
FILE *cvs_temp_file (char ** filename)
{
    char *fn;
    FILE *fp;

    /* FIXME - I'd like to be returning NULL here in noexec mode, but I think
     * some of the rcs & diff functions which rely on a temp file run in
     * noexec mode too.
     */

    /* assert (filename != NULL); */

    fn = _tempnam (getenv("TEMP"), "cvs");
    if (fn == NULL) fp = NULL;
    else
    if ((fp = CVS_FOPEN (fn, "w+")) == NULL)
    {
        free (fn);
        fn = NULL;
    }

    /* tempnam returns a pointer to a newly malloc'd string, so there's
     * no need for a xstrdup
     */

    *filename = fn;
    return fp;
}



/* Return a pointer into PATH's last component.  */
const char *
last_component (const char *path)
{
    const char *scan;
    const char *last = 0;

    for (scan = path; *scan; scan++)
        if (ISSLASH (*scan))
	    last = scan;

    if (last && (last != path))
        return last + 1;
    else
        return path;
}


/* NT has two evironment variables, HOMEPATH and HOMEDRIVE, which,
   when combined as ${HOMEDRIVE}${HOMEPATH}, give the unix equivalent
   of HOME.  Some NT users are just too unixy, though, and set the
   HOME variable themselves.  Therefore, we check for HOME first, and
   then try to combine the other two if that fails.

   Looking for HOME strikes me as bogus, particularly if the only reason
   is to cater to "unixy users".  On the other hand, if the reasoning is
   there should be a single variable, rather than requiring people to
   set both HOMEDRIVE and HOMEPATH, then it starts to make a little more
   sense.

   Win95: The system doesn't set HOME, HOMEDRIVE, or HOMEPATH (at
   least if you set it up as the "all users under one user ID" or
   whatever the name of that option is).  Based on thing overheard on
   the net, it seems that users of the pserver client have gotten in
   the habit of setting HOME (if you don't use pserver, you can
   probably get away without having a reasonable return from
   get_homedir.  Of course you lose .cvsrc and .cvsignore, but many
   users won't notice).  So it would seem that we should be somewhat
   careful if we try to change the current behavior.

   NT 3.51 or NT 4.0: I haven't checked this myself, but I am told
   that HOME gets set, but not to the user's home directory.  It is
   said to be set to c:\users\default by default.  */

char *
get_homedir (void)
{
    char *homedir;

    homedir = getenv ("HOME");

    if (homedir == NULL)
	homedir = woe32_home_dir ();

    return homedir;
}

/* Compose a path to a file in the home directory.  This is necessary because
 * of different behavior on UNIX, Windows, and VMS.  See more notes in
 * vms/filesubr.c.
 *
 * A more clean solution would be something more along the lines of a
 * "join a directory to a filename" kind of thing which was not specific to
 * the homedir.  This should aid portability between UNIX, Mac, Windows, VMS,
 * and possibly others.  This is already handled by Perl - it might be
 * interesting to see how much of the code was written in C since Perl is under
 * the GPL and the Artistic license - we might be able to use it.
 */
char *
strcat_filename_onto_homedir (dir, file)
    const char *dir;
    const char *file;
{
    char *path = xmalloc (strlen (dir) + 1 + strlen(file) + 1);
    sprintf (path, "%s\\%s", dir, file);
    return path;
}

/* See cvs.h for description.  */
void
expand_wild (argc, argv, pargc, pargv)
    int argc;
    char **argv;
    int *pargc;
    char ***pargv;
{
    int i;
    int new_argc;
    char **new_argv;
    /* Allocated size of new_argv.  We arrange it so there is always room for
	   one more element.  */
    int max_new_argc;

    new_argc = 0;
    /* Add one so this is never zero.  */
    max_new_argc = argc + 1;
    new_argv = (char **) xmalloc (max_new_argc * sizeof (char *));
    for (i = 0; i < argc; ++i)
    {
	HANDLE h;
	WIN32_FIND_DATA fdata;

	/* These variables help us extract the directory name from the
           given pathname. */

	char *last_forw_slash, *last_back_slash, *end_of_dirname;
	int dirname_length = 0;

	if ( strcmp( argv[i], "." ) == 0 )
	{
	    new_argv[new_argc] = (char *) xmalloc ( 2 );
	    strcpy( new_argv[ new_argc++ ], "." );
	    continue;
	}

	/* FindFirstFile doesn't return pathnames, so we have to do
	   this ourselves.  Luckily, it's no big deal, since globbing
	   characters under Win32s can only occur in the last segment
	   of the path.  For example,
                /a/path/q*.h                      valid
	        /w32/q*.dir/cant/do/this/q*.h     invalid */

	/* Win32 can handle both forward and backward slashes as
           filenames -- check for both. */
	     
	last_forw_slash = strrchr (argv[i], '/');
	last_back_slash = strrchr (argv[i], '\\');

#define cvs_max(x,y) ((x >= y) ? (x) : (y))

	/* FIXME: this comparing a NULL pointer to a non-NULL one is
	   extremely ugly, and I strongly suspect *NOT* sanctioned by
	   ANSI C.  The code should just use last_component instead.  */
	end_of_dirname = cvs_max (last_forw_slash, last_back_slash);

	if (end_of_dirname == NULL)
	  dirname_length = 0;	/* no directory name */
	else
	  dirname_length = end_of_dirname - argv[i] + 1; /* include slash */

	h = FindFirstFile (argv[i], &fdata);
	if (h == INVALID_HANDLE_VALUE)
	{
	    if (GetLastError () == ENOENT)
	    {
		/* No match.  The file specified didn't contain a wildcard (in which case
		   we clearly should return it unchanged), or it contained a wildcard which
		   didn't match (in which case it might be better for it to be an error,
		   but we don't try to do that).  */
		new_argv [new_argc++] = xstrdup (argv[i]);
		if (new_argc == max_new_argc)
		{
		    max_new_argc *= 2;
		    new_argv = xrealloc (new_argv, max_new_argc * sizeof (char *));
		}
	    }
	    else
	    {
		error (1, errno, "cannot find %s", argv[i]);
	    }
	}
	else
	{
	    while (1)
	    {
		new_argv[new_argc] =
		    (char *) xmalloc (strlen (fdata.cFileName) + 1
				      + dirname_length);

		/* Copy the directory name, if there is one. */

		if (dirname_length)
		{
		    strncpy (new_argv[new_argc], argv[i], dirname_length);
		    new_argv[new_argc][dirname_length] = '\0';
		}
		else
		    new_argv[new_argc][0] = '\0';

		/* Copy the file name. */
		
		if (fncmp (argv[i] + dirname_length, fdata.cFileName) == 0)
		    /* We didn't expand a wildcard; we just matched a filename.
		       Use the file name as specified rather than the filename
		       which exists in the directory (they may differ in case).
		       This is needed to make cvs add on a directory consistently
		       use the name specified on the command line, but it is
		       probably a good idea in other contexts too.  */
		    strcpy (new_argv[new_argc], argv[i]);
		else
		    strcat (new_argv[new_argc], fdata.cFileName);

		new_argc++;

		if (new_argc == max_new_argc)
		{
		    max_new_argc *= 2;
		    new_argv = xrealloc (new_argv, max_new_argc * sizeof (char *));
		}
		if (!FindNextFile (h, &fdata))
		{
		    if (GetLastError () == ERROR_NO_MORE_FILES)
			break;
		    else
			error (1, errno, "cannot find %s", argv[i]);
		}
	    }
	    if (!FindClose (h))
		error (1, GetLastError (), "cannot close %s", argv[i]);
	}
    }
    *pargc = new_argc;
    *pargv = new_argv;
}

/* undo config.h stat macro */
#undef stat
extern int stat (const char *file, struct wnt_stat *sb);

/* see config.h stat macro */
int
wnt_stat (const char *file, struct wnt_stat *sb)
{
    int retval;

    retval = stat (file, sb);
    if (retval < 0)
		return retval;

	/* Win32 processes file times in a 64 bit format
       (see Win32 functions SetFileTime and GetFileTime).
       If the file time on a file doesn't fit into the
       32 bit time_t format, then stat will set that time
       to -1.  This would be OK, except that functions
       like ctime() don't check for validity.  So what we
       do here is to give a error on -1.  A cleaner solution
       might be to change CVS's interfaces to return a time
       in RCS format (for example), and then implement it
       on Win32 via GetFileTime, but that would be a lot of
       hair and I'm not sure there is much payoff.  */
    if (sb->st_mtime == (time_t) -1)
		error (1, 0, "invalid modification time for %s", file);
    if (sb->st_ctime == (time_t) -1)
	/* I'm not sure what this means on windows.  It
	   might be a creation time (unlike unix)....  */
		error (1, 0, "invalid ctime for %s", file);
    if (sb->st_atime == (time_t) -1)
		error (1, 0, "invalid access time for %s", file);

    if (!GetUTCFileModTime (file, &sb->st_mtime))
		error (1, 0, "Failed to retrieve modification time for %s", file);

    return retval;
}
