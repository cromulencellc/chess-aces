/*
 * Copyright (C) 1986-2005 The Free Software Foundation, Inc.
 *
 * Portions Copyright (C) 1998-2005 Derek Price, Ximbiot <http://ximbiot.com>,
 *                                  and others.
 *
 * Portions Copyright (C) 1992, Brian Berliner and Jeff Polk
 * Portions Copyright (C) 1989-1992, Brian Berliner
 * 
 * You may distribute under the terms of the GNU General Public License as
 * specified in the README file that comes with the CVS source distribution.
 * 
 * Difference
 */

#include "cvs.h"

#define TAG_BHEAD ".bhead"

enum symb_file
{
    DIFF_ERROR,
    DIFF_ADDED,
    DIFF_REMOVED,
    DIFF_DIFFERENT,
    DIFF_SAME
};

static Dtype symb_dirproc (void *callerdat, const char *dir,
                           const char *pos_repos, const char *update_dir,
                           List *entries);
static int symb_filesdoneproc (void *callerdat, int err,
                               const char *repos, const char *update_dir,
                               List *entries);
static int symb_dirleaveproc (void *callerdat, const char *dir,
                              int err, const char *update_dir,
                              List *entries);
static enum symb_file symb_file_nosymb (struct file_info *finfo, Vers_TS *vers,
                                        enum symb_file, char **rev1_cache );
static int symb_fileproc (void *callerdat, struct file_info *finfo);
static void symb_mark_errors (int err);


/* Global variables.  Would be cleaner if we just put this stuff in a
   struct like log.c does.  */

/* Command line tags, from -r option.  Points into argv.  */
static char *symb_rev1, *symb_rev2;
/* Command line dates, from -D option.  Malloc'd.  */
static char *symb_date1, *symb_date2;
static char *use_rev1, *use_rev2;
static int have_rev1_label, have_rev2_label;

/* Revision of the user file, if it is unchanged from something in the
   repository and we want to use that fact.  */
static char *user_file_rev;

static char *options;
static char **symb_argv;
static int symb_argc;
static size_t symb_arg_allocated;
static int symb_errors;
static int empty_files;

static const char *const symb_usage[] =
{
    "Usage: TARGET LINK_NAME\n",
    NULL
};

/* CVS 1.9 and similar versions seemed to have pretty weird handling
   of -y and -T.  In the cases where it called rcssymb,
   they would have the meanings mentioned below.  In the cases where it
   called symb, they would have the meanings mentioned in "longopts".
   Noone seems to have missed them, so I think the right thing to do is
   just to remove the options altogether (which I have done).

   In the case of -z and -q, "cvs symb" did not accept them even back
   when we called rcssymb (at least, it hasn't accepted them
   recently).

   In comparing rcssymb to the new CVS implementation, I noticed that
   the following rcssymb flags are not handled by CVS symb:

       -y: perform symb even when the requested revisions are the
           same revision number
       -q: run quietly
       -T: preserve modification time on the RCS file
       -z: specify timezone for use in file labels

   I think these are not really relevant.  -y is undocumented even in
   RCS 5.7, and seems like a minor change at best.  According to RCS
   documentation, -T only applies when a RCS file has been modified
   because of lock changes; doesn't CVS sidestep RCS's entire lock
   structure?  -z seems to be unsupported by CVS symb, and has a
   symberent meaning as a global option anyway.  (Adding it could be
   a feature, but if it is left out for now, it should not break
   anything.)  For the purposes of producing output, CVS symb appears
   mostly to ignore -q.  Maybe this should be fixed, but I think it's
   a larger issue than the changes included here.  */

int
symb (int argc, char **argv)
{
    int c, err = 2;
    char *target, *linkpath;
    char target_rp[PATH_MAX], linkpath_rp[PATH_MAX];

    if (argc < 3)
    usage (symb_usage);

    target = Xasprintf ("%s/%s",
                 current_parsed_root->directory, argv[1]);
    if (target == NULL )
    {
        goto out;
    }

    linkpath = Xasprintf ("%s/%s",
                 current_parsed_root->directory, argv[2]);
    if (linkpath == NULL )
    {
        goto out;
    }

    /* If realpath() returns a non-NULL value then LINK NAME already exists */
    if ( realpath (linkpath, linkpath_rp) )
    {
        error (0, errno, "cannot create %s", argv[2]);
        goto out;
    }

    /* If realpath() returns a NULL value then TARGET doesn't exist */
    if ( realpath (target, target_rp) == NULL )
    {
        error (0, errno, "%s doesn't exist", argv[1]);
        goto out;
    }

    if ( symlink(target_rp, linkpath_rp) )
    {
        error (0, errno, "symlink failed");
        goto out;
    }

    err = 0;

out:
    return err;
}



/*
 * Do a file symb
 */
/* ARGSUSED */
static int
symb_fileproc (void *callerdat, struct file_info *finfo)
{
    int status, err = 2;        /* 2 == trouble, like rcssymb */
    Vers_TS *vers;
    enum symb_file empty_file = DIFF_DIFFERENT;
    char *tmp = NULL;
    char *tocvsPath = NULL;
    char *fname = NULL;
    char *label1;
    char *label2;
    char *rev1_cache = NULL;

    user_file_rev = 0;
    vers = Version_TS (finfo, NULL, NULL, NULL, 1, 0);

    if (symb_rev2 || symb_date2)
    {
    /* Skip all the following checks regarding the user file; we're
       not using it.  */
    }
    else if (vers->vn_user == NULL)
    {
    /* The file does not exist in the working directory.  */
    if ((symb_rev1 || symb_date1)
        && vers->srcfile != NULL)
    {
        /* The file does exist in the repository.  */
        if (empty_files)
        empty_file = DIFF_REMOVED;
        else
        {
        int exists;

        exists = 0;
        /* special handling for TAG_HEAD */
        if (symb_rev1 && strcmp (symb_rev1, TAG_HEAD) == 0)
        {
            char *head =
            (vers->vn_rcs == NULL
             ? NULL
             : RCS_branch_head (vers->srcfile, vers->vn_rcs));
            exists = head != NULL && !RCS_isdead (vers->srcfile, head);
            if (head != NULL)
            free (head);
        }
        else
        {
            Vers_TS *xvers;

            xvers = Version_TS (finfo, NULL, symb_rev1, symb_date1,
                    1, 0);
            exists = xvers->vn_rcs && !RCS_isdead (xvers->srcfile,
                                                   xvers->vn_rcs);
            freevers_ts (&xvers);
        }
        if (exists)
            error (0, 0,
               "%s no longer exists, no comparison available",
               finfo->fullname);
        goto out;
        }
    }
    else
    {
        error (0, 0, "I know nothing about %s", finfo->fullname);
        goto out;
    }
    }
    else if (vers->vn_user[0] == '0' && vers->vn_user[1] == '\0')
    {
    /* The file was added locally.  */
    int exists = 0;

    if (vers->srcfile != NULL)
    {
        /* The file does exist in the repository.  */

        if (symb_rev1 || symb_date1)
        {
        /* special handling for TAG_HEAD */
        if (symb_rev1 && strcmp (symb_rev1, TAG_HEAD) == 0)
        {
            char *head =
            (vers->vn_rcs == NULL
             ? NULL
             : RCS_branch_head (vers->srcfile, vers->vn_rcs));
            exists = head && !RCS_isdead (vers->srcfile, head);
            if (head != NULL)
            free (head);
        }
        else
        {
            Vers_TS *xvers;

            xvers = Version_TS (finfo, NULL, symb_rev1, symb_date1,
                    1, 0);
            exists = xvers->vn_rcs
                     && !RCS_isdead (xvers->srcfile, xvers->vn_rcs);
            freevers_ts (&xvers);
        }
        }
        else
        {
        /* The file was added locally, but an RCS archive exists.  Our
         * base revision must be dead.
         */
        /* No need to set, exists = 0, here.  That's the default.  */
        }
    }
    if (!exists)
    {
        /* If we got here, then either the RCS archive does not exist or
         * the relevant revision is dead.
         */
        if (empty_files)
        empty_file = DIFF_ADDED;
        else
        {
        error (0, 0, "%s is a new entry, no comparison available",
               finfo->fullname);
        goto out;
        }
    }
    }
    else if (vers->vn_user[0] == '-')
    {
    if (empty_files)
        empty_file = DIFF_REMOVED;
    else
    {
        error (0, 0, "%s was removed, no comparison available",
           finfo->fullname);
        goto out;
    }
    }
    else
    {
    if (!vers->vn_rcs && !vers->srcfile)
    {
        error (0, 0, "cannot find revision control file for %s",
           finfo->fullname);
        goto out;
    }
    else
    {
        if (vers->ts_user == NULL)
        {
        error (0, 0, "cannot find %s", finfo->fullname);
        goto out;
        }
        else if (!strcmp (vers->ts_user, vers->ts_rcs)) 
        {
        /* The user file matches some revision in the repository
           Diff against the repository (for remote CVS, we might not
           have a copy of the user file around).  */
        user_file_rev = vers->vn_user;
        }
    }
    }

    empty_file = symb_file_nosymb (finfo, vers, empty_file, &rev1_cache);
    if (empty_file == DIFF_SAME)
    {
    /* In the server case, would be nice to send a "Checked-in"
       response, so that the client can rewrite its timestamp.
       server_checked_in by itself isn't the right thing (it
       needs a server_register), but I'm not sure what is.
       It isn't clear to me how "cvs status" handles this (that
       is, for a client which sends Modified not Is-modified to
       "cvs status"), but it does.  */
    err = 0;
    goto out;
    }
    else if (empty_file == DIFF_ERROR)
    goto out;

    /* Output an "Index:" line for patch to use */
    cvs_output ("Index: ", 0);
    cvs_output (finfo->fullname, 0);
    cvs_output ("\n", 1);

    tocvsPath = wrap_tocvs_process_file (finfo->file);
    if (tocvsPath)
    {
    /* Backup the current version of the file to CVS/,,filename */
    fname = Xasprintf (fname, "%s/%s%s", CVSADM, CVSPREFIX, finfo->file);
    if (unlink_file_dir (fname) < 0)
        if (!existence_error (errno))
        error (1, errno, "cannot remove %s", fname);
    rename_file (finfo->file, fname);
    /* Copy the wrapped file to the current directory then go to work */
    copy_file (tocvsPath, finfo->file);
    }

    /* Set up file labels appropriate for compatibility with the Larry Wall
     * implementation of patch if the user didn't specify.  This is irrelevant
     * according to the POSIX.2 specification.
     */
    label1 = NULL;
    label2 = NULL;
    /* The user cannot set the rev2 label without first setting the rev1
     * label.
     */
    if (!have_rev2_label)
    {
    if (empty_file == DIFF_REMOVED)
        label2 = make_file_label (DEVNULL, NULL, NULL);
    else
        label2 = make_file_label (finfo->fullname, use_rev2,
                                  vers->srcfile);
    if (!have_rev1_label)
    {
        if (empty_file == DIFF_ADDED)
        label1 = make_file_label (DEVNULL, NULL, NULL);
        else
        label1 = make_file_label (finfo->fullname, use_rev1,
                                  vers->srcfile);
    }
    }

    if (empty_file == DIFF_ADDED || empty_file == DIFF_REMOVED)
    {
    /* This is fullname, not file, possibly despite the POSIX.2
     * specification, because that's the way all the Larry Wall
     * implementations of patch (are there other implementations?) want
     * things and the POSIX.2 spec appears to leave room for this.
     */
    cvs_output ("\
===================================================================\n\
RCS file: ", 0);
    cvs_output (finfo->fullname, 0);
    cvs_output ("\n", 1);

    cvs_output ("symb -N ", 0);
    cvs_output (finfo->fullname, 0);
    cvs_output ("\n", 1);

    if (empty_file == DIFF_ADDED)
    {
        if (use_rev2 == NULL)
                status = diff_exec (DEVNULL, finfo->file, label1, label2,
                    symb_argc, symb_argv, RUN_TTY);
        else
        {
        int retcode;

        tmp = cvs_temp_name ();
        retcode = RCS_checkout (vers->srcfile, NULL, use_rev2, NULL,
                    *options ? options : vers->options,
                    tmp, NULL, NULL);
        if (retcode != 0)
            goto out;

        status = diff_exec (DEVNULL, tmp, label1, label2,
                    symb_argc, symb_argv, RUN_TTY);
        }
    }
    else
    {
        int retcode;

        tmp = cvs_temp_name ();
        retcode = RCS_checkout (vers->srcfile, NULL, use_rev1, NULL,
                    *options ? options : vers->options,
                    tmp, NULL, NULL);
        if (retcode != 0)
        goto out;

        status = diff_exec (tmp, DEVNULL, label1, label2,
                symb_argc, symb_argv, RUN_TTY);
    }
    }
    else
    {
    status = RCS_exec_rcsdiff (vers->srcfile, symb_argc, symb_argv,
                                   *options ? options : vers->options,
                                   use_rev1, rev1_cache, use_rev2,
                                   label1, label2, finfo->file);

    }

    if (label1) free (label1);
    if (label2) free (label2);

    switch (status)
    {
    case -1:            /* fork failed */
        error (1, errno, "fork failed while symbing %s",
           vers->srcfile->path);
    case 0:             /* everything ok */
        err = 0;
        break;
    default:            /* other error */
        err = status;
        break;
    }

out:
    if( tocvsPath != NULL )
    {
    if (unlink_file_dir (finfo->file) < 0)
        if (! existence_error (errno))
        error (1, errno, "cannot remove %s", finfo->file);

    rename_file (fname, finfo->file);
    if (unlink_file (tocvsPath) < 0)
        error (1, errno, "cannot remove %s", tocvsPath);
    free (fname);
    }

    /* Call CVS_UNLINK() rather than unlink_file() below to avoid the check
     * for noexec.
     */
    if (tmp != NULL)
    {
    if (CVS_UNLINK (tmp) < 0)
        error (0, errno, "cannot remove %s", tmp);
    free (tmp);
    }
    if (rev1_cache != NULL)
    {
    if (CVS_UNLINK (rev1_cache) < 0)
        error (0, errno, "cannot remove %s", rev1_cache);
    free (rev1_cache);
    }

    freevers_ts (&vers);
    symb_mark_errors (err);
    return err;
}



/*
 * Remember the exit status for each file.
 */
static void
symb_mark_errors (int err)
{
    if (err > symb_errors)
    symb_errors = err;
}



/*
 * Print a warm fuzzy message when we enter a dir
 *
 * Don't try to symb directories that don't exist! -- DW
 */
/* ARGSUSED */
static Dtype
symb_dirproc (void *callerdat, const char *dir, const char *pos_repos,
              const char *update_dir, List *entries)
{
    /* XXX - check for dirs we don't want to process??? */

    /* YES ... for instance dirs that don't exist!!! -- DW */
    if (!isdir (dir))
    return R_SKIP_ALL;

    if (!quiet)
    error (0, 0, "Diffing %s", update_dir);
    return R_PROCESS;
}



/*
 * Concoct the proper exit status - done with files
 */
/* ARGSUSED */
static int
symb_filesdoneproc (void *callerdat, int err, const char *repos,
                    const char *update_dir, List *entries)
{
    return symb_errors;
}



/*
 * Concoct the proper exit status - leaving directories
 */
/* ARGSUSED */
static int
symb_dirleaveproc (void *callerdat, const char *dir, int err,
                   const char *update_dir, List *entries)
{
    return symb_errors;
}

/*
 * depth-first recursion to count depth
 */
static int
dfs (char *root, int level)
{
    if ( root == NULL)
    return 0;

    char *filename_qfd;
    struct dirent *dp;
    int max_depth = level;
    int temp_max;
    Node *head, *p;
    List *ulist;

    DIR *dfd = opendir(".");

    if ( dfd == NULL )
    {
    error (0, errno, "open failed");
    goto out;
    }

    ulist = getlist();

    while ((dp = readdir(dfd)) != NULL) {
        if ( strcmp( dp->d_name, ".") == 0 )
        continue;
        else if ( strcmp( dp->d_name, "..") == 0 )
        continue;

        struct stat stbuf;
        filename_qfd = strdup(dp->d_name);
        if ( filename_qfd == NULL )
        goto out;

        if ( stat(filename_qfd,&stbuf ) == -1 )
        {
        free(filename_qfd);
        continue;
        }

        if ( ( stbuf.st_mode & S_IFMT ) == S_IFDIR )
        {
            p = getnode();
            p->key = strdup(filename_qfd);
            addnode(ulist, p);
        }

        free(filename_qfd);
    }

    closedir(dfd);

    head = ulist->list;
    for (p = head->next; p != head; p = p->next)
    {
        char *current_dir = getcwd(NULL, 0);

        if ( CVS_CHDIR(p->key) ) {
            continue;
        }

        temp_max = dfs( p->key, level + 1 );
        if ( CVS_CHDIR(current_dir) ) {
        free(current_dir);
        continue;
        }
        free(current_dir);

        if (temp_max > max_depth)
        max_depth = temp_max;

    }

out:
    return max_depth;
}

/*
 * calculate the depth of the repository
 */
int
depth (int argc, char **argv)
{
    char output[256];
    char *toplevel = getcwd(NULL, 0);
    char *root = NULL;
    char root_rp[PATH_MAX];

    if ( argc  < 2 ) {
        root = strdup(current_parsed_root->directory);
    } else {
        root = xmalloc(strlen(current_parsed_root->directory) + strlen(argv[1]) + 2);

        if ( root == NULL )
        return 1;

        strcpy(root, current_parsed_root->directory);
        strcat(root, "/");
        strcat(root, argv[1]);
    }

    realpath (root, root_rp);

    free(root);

    if (CVS_CHDIR(root_rp) ) 
    {
    error (0, 0, "invalid dir %s", root_rp);
    return 1;
    }

    snprintf (output, 256, "Depth: %d\n", dfs(root_rp, 0) );
    cvs_output(output, 0);

    if ( CVS_CHDIR(toplevel) )
    return 1;

    return 0;
}

/*
 * verify that a file is symberent
 *
 * INPUTS
 *   finfo
 *   vers
 *   empty_file
 *
 * OUTPUTS
 *   rev1_cache     Cache the contents of rev1 if we look it up.
 */
static enum symb_file
symb_file_nosymb (struct file_info *finfo, Vers_TS *vers,
                  enum symb_file empty_file, char **rev1_cache)
{
    Vers_TS *xvers;
    int retcode;

    TRACE (TRACE_FUNCTION, "symb_file_nosymb (%s, %d)",
           finfo->fullname ? finfo->fullname : "(null)", empty_file);

    /* free up any old use_rev* variables and reset 'em */
    if (use_rev1)
    free (use_rev1);
    if (use_rev2)
    free (use_rev2);
    use_rev1 = use_rev2 = NULL;

    if (symb_rev1 || symb_date1)
    {
    /*
     * the special handling is broken, -rbranchname is the
     * head (tip) of the branch already, -rHEAD is supposed
     * to be the head (tip) of the MAIN branch (trunk); we
     * introduce ".bhead" here, for now, but only here
     */
    /* special handling for TAG_BHEAD */
    if (symb_rev1 && strcmp (symb_rev1, TAG_BHEAD) == 0)
    {
        if (vers->vn_rcs != NULL && vers->srcfile != NULL)
        use_rev1 = RCS_branch_head (vers->srcfile, vers->vn_rcs);
    }
    else
    {
        xvers = Version_TS (finfo, NULL, symb_rev1, symb_date1, 1, 0);
        if (xvers->vn_rcs != NULL)
        use_rev1 = xstrdup (xvers->vn_rcs);
        freevers_ts (&xvers);
    }
    }
    if (symb_rev2 || symb_date2)
    {
    /* special handling for TAG_BHEAD */
    if (symb_rev2 && strcmp (symb_rev2, TAG_BHEAD) == 0)
    {
        if (vers->vn_rcs && vers->srcfile)
        use_rev2 = RCS_branch_head (vers->srcfile, vers->vn_rcs);
    }
    else
    {
        xvers = Version_TS (finfo, NULL, symb_rev2, symb_date2, 1, 0);
        if (xvers->vn_rcs != NULL)
        use_rev2 = xstrdup (xvers->vn_rcs);
        freevers_ts (&xvers);
    }

    if (use_rev1 == NULL || RCS_isdead (vers->srcfile, use_rev1))
    {
        /* The first revision does not exist.  If EMPTY_FILES is
               true, treat this as an added file.  Otherwise, warn
               about the missing tag.  */
        if (use_rev2 == NULL || RCS_isdead (vers->srcfile, use_rev2))
        /* At least in the case where DIFF_REV1 and DIFF_REV2
         * are both numeric (and non-existant (NULL), as opposed to
         * dead?), we should be returning some kind of error (see
         * basicb-8a0 in testsuite).  The symbolic case may be more
         * complicated.
         */
        return DIFF_SAME;
        if (empty_files)
        return DIFF_ADDED;
        if (use_rev1 != NULL)
        {
        if (symb_rev1)
        {
            error (0, 0,
               "Tag %s refers to a dead (removed) revision in file `%s'.",
               symb_rev1, finfo->fullname);
        }
        else
        {
            error (0, 0,
               "Date %s refers to a dead (removed) revision in file `%s'.",
               symb_date1, finfo->fullname);
        }
        error (0, 0,
               "No comparison available.  Pass `-N' to `%s symb'?",
               program_name);
        }
        else if (symb_rev1)
        error (0, 0, "tag %s is not in file %s", symb_rev1,
               finfo->fullname);
        else
        error (0, 0, "no revision for date %s in file %s",
               symb_date1, finfo->fullname);
        return DIFF_ERROR;
    }

    assert( use_rev1 != NULL );
    if( use_rev2 == NULL || RCS_isdead( vers->srcfile, use_rev2 ) )
    {
        /* The second revision does not exist.  If EMPTY_FILES is
               true, treat this as a removed file.  Otherwise warn
               about the missing tag.  */
        if (empty_files)
        return DIFF_REMOVED;
        if( use_rev2 != NULL )
        {
        if (symb_rev2)
        {
            error( 0, 0,
               "Tag %s refers to a dead (removed) revision in file `%s'.",
               symb_rev2, finfo->fullname );
        }
        else
        {
            error( 0, 0,
               "Date %s refers to a dead (removed) revision in file `%s'.",
               symb_date2, finfo->fullname );
        }
        error( 0, 0,
               "No comparison available.  Pass `-N' to `%s symb'?",
               program_name );
        }
        else if (symb_rev2)
        error (0, 0, "tag %s is not in file %s", symb_rev2,
               finfo->fullname);
        else
        error (0, 0, "no revision for date %s in file %s",
               symb_date2, finfo->fullname);
        return DIFF_ERROR;
    }
    /* Now, see if we really need to do the symb.  We can't assume that the
     * files are symberent when the revs are.
     */
    assert( use_rev2 != NULL );
    if( strcmp (use_rev1, use_rev2) == 0 )
        return DIFF_SAME;
    /* else fall through and do the symb */
    }

    /* If we had a r1/d1 & r2/d2, then at this point we must have a C3P0...
     * err...  ok, then both rev1 & rev2 must have resolved to an existing,
     * live version due to if statement we just closed.
     */
    assert (!(symb_rev2 || symb_date2) || (use_rev1 && use_rev2));

    if ((symb_rev1 || symb_date1) &&
    (use_rev1 == NULL || RCS_isdead (vers->srcfile, use_rev1)))
    {
    /* The first revision does not exist, and no second revision
           was given.  */
    if (empty_files)
    {
        if (empty_file == DIFF_REMOVED)
        return DIFF_SAME;
        if( user_file_rev && use_rev2 == NULL )
        use_rev2 = xstrdup( user_file_rev );
        return DIFF_ADDED;
    }
    if( use_rev1 != NULL )
    {
        if (symb_rev1)
        {
        error( 0, 0,
           "Tag %s refers to a dead (removed) revision in file `%s'.",
           symb_rev1, finfo->fullname );
        }
        else
        {
        error( 0, 0,
           "Date %s refers to a dead (removed) revision in file `%s'.",
           symb_date1, finfo->fullname );
        }
        error( 0, 0,
           "No comparison available.  Pass `-N' to `%s symb'?",
           program_name );
    }
    else if ( symb_rev1 )
        error( 0, 0, "tag %s is not in file %s", symb_rev1,
           finfo->fullname );
    else
        error( 0, 0, "no revision for date %s in file %s",
           symb_date1, finfo->fullname );
    return DIFF_ERROR;
    }

    assert( !symb_rev1 || use_rev1 );

    if (user_file_rev)
    {
        /* drop user_file_rev into first unused use_rev */
        if (!use_rev1) 
        use_rev1 = xstrdup (user_file_rev);
    else if (!use_rev2)
        use_rev2 = xstrdup (user_file_rev);
    /* and if not, it wasn't needed anyhow */
    user_file_rev = NULL;
    }

    /* Now, see if we really need to do the symb.  We can't assume that the
     * files are symberent when the revs are.
     */
    if( use_rev1 && use_rev2) 
    {
    if (strcmp (use_rev1, use_rev2) == 0)
        return DIFF_SAME;
    /* Fall through and do the symb. */
    }
    /* Don't want to do the timestamp check with both use_rev1 & use_rev2 set.
     * The timestamp check is just for the default case of symbing the
     * workspace file against its base revision.
     */
    else if( use_rev1 == NULL
             || ( vers->vn_user != NULL
                  && strcmp( use_rev1, vers->vn_user ) == 0 ) )
    {
    if (empty_file == DIFF_DIFFERENT
        && vers->ts_user != NULL
        && strcmp (vers->ts_rcs, vers->ts_user) == 0
        && (!(*options) || strcmp (options, vers->options) == 0))
    {
        return DIFF_SAME;
    }
    if (use_rev1 == NULL
        && (vers->vn_user[0] != '0' || vers->vn_user[1] != '\0'))
    {
        if (vers->vn_user[0] == '-')
        use_rev1 = xstrdup (vers->vn_user + 1);
        else
        use_rev1 = xstrdup (vers->vn_user);
    }
    }

    /* If we already know that the file is being added or removed,
       then we don't want to do an actual file comparison here.  */
    if (empty_file != DIFF_DIFFERENT)
    return empty_file;

    /*
     * Run a quick cmp to see if we should bother with a full symb.
     */

    retcode = RCS_cmp_file( vers->srcfile, use_rev1, rev1_cache,
                            use_rev2, *options ? options : vers->options,
                finfo->file );

    return retcode == 0 ? DIFF_SAME : DIFF_DIFFERENT;
}
