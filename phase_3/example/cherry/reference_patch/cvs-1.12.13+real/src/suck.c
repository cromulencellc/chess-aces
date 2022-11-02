/*-
 * Copyright (c) 2011
 *	Thorsten Glaser <tg@mirbsd.org>
 *
 * Provided that these terms and disclaimer and all copyright notices
 * are retained or reproduced in an accompanying document, permission
 * is granted to deal in this work without restriction, including un-
 * limited rights to use, publicly perform, distribute, sell, modify,
 * merge, give away, or sublicence.
 *
 * This work is provided "AS IS" and WITHOUT WARRANTY of any kind, to
 * the utmost extent permitted by applicable law, neither express nor
 * implied; without malicious intent or gross negligence. In no event
 * may a licensor, author or contributor be held liable for indirect,
 * direct, other damage, loss, or other issues arising in any way out
 * of dealing in the work, even if advised of the possibility of such
 * damage or existence of a defect, except proven that it results out
 * of said person's immediate fault when using the work as intended.
 *-
 * Download a ,v file from the repository
 */

#include "cvs.h"

#ifdef HAVE_MMAP
#include <sys/mman.h>

#ifndef MAP_FILE
#define MAP_FILE 0
#endif

#ifndef MAP_FAILED
#define MAP_FAILED ((void *)-1)
#endif
#endif

static const char * const suck_usage[] = {
	"Usage: %s %s module/filename\n",
	NULL
};

int
suck(int argc, char **argv)
{
	size_t m, n;
	int fd;
	char *buf, *cp, *fn;
	struct stat sb;
	FILE *fp;
	RCSNode *rcs;

	if (argc != 2)
		usage(suck_usage);

#ifdef CLIENT_SUPPORT
	if (current_parsed_root->isremote) {
		start_server();

		if (!supported_request("suck"))
			error(1, 0, "server does not support %s", "suck");

		send_arg(argv[1]);
		send_to_server("suck\012", 0);

		return (get_responses_and_close());
	}
#endif

	/* check for ../ attack */
	if (pathname_levels(argv[1]) > 0)
		error(1, 0, "path %s outside of repository", argv[1]);

	/* repo + / + module/file */
	cp = Xasprintf("%s/%s", current_parsed_root->directory, argv[1]);

	/* find the slash */
	if ((fn = cp + (last_component(cp) - cp)) == cp)
		usage(suck_usage);

	/* repo/module + file */
	fn[-1] = '\0';

	/* check if it's a valid RCS file, not /etc/passwd or somesuch */
	if ((rcs = RCS_parse(fn, cp)) == NULL) {
		error(1, 0, "not a valid RCS file: %s/%s", cp, fn);
		return (1);
	}

	/* save the real pathname of the RCS file for later */
	fn = xstrdup(rcs->path);

	/* free up resources allocated until now */
	freercsnode(&rcs);
	free(cp);

	/* attempt to open the file ourselves */
	if ((fp = CVS_FOPEN(fn, FOPEN_BINARY_READ)) == NULL)
		error(1, errno, "Could not open RCS archive %s", fn);
	if (fstat(fd = fileno(fp), &sb) < 0)
		error(1, errno, "Could not stat RCS archive %s", fn);

	/*XXX this code will fail for large files */

	/* attempt to slurp entire file into memory */
#ifdef HAVE_MMAP
	buf = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (buf == NULL || buf == MAP_FAILED) {
		error(0, errno, "Could not map memory to RCS archive %s", fn);
#endif
	/* backup: just read */
		cp = buf = xmalloc(n = sb.st_size);
		while (n) {
			m = read(fd, cp, n);
			if (m == (size_t)-1)
				error(1, errno,
				    "Could not read RCS archive %s", fn);
			cp += m;
			n -= m;
		}
#ifdef HAVE_MMAP
	}
#endif

	/* write real pathname plus newline as text */
	cvs_output(fn + strlen(current_parsed_root->directory) + 1, 0);
	cvs_output("\n", 1);

	/* write file content as binary */
	cvs_output_binary(buf, sb.st_size);

	/* release all resources allocated */
#ifdef HAVE_MMAP
	munmap(buf, sb.st_size);
#endif
	fclose(fp);
	free(fn);

	/* success */
	return (0);
}
