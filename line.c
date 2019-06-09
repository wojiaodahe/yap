#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <error.h>
#include <string.h>

#define error_exit(_errmsg_)	error(EXIT_FAILURE, errno, _errmsg_)

#define PATH_LEN	1024

int total;

int process_dir(void);
int process_file(const char *filename);

int main()
{
	process_dir();

	printf("total: %d\n", total);
	return 0;
}

int process_dir(void)
{
	DIR *dp = NULL;
	char *path = NULL;
	struct stat statbuff;
	struct dirent * dirbuf = NULL;
	int namlen = 0;

	path = getcwd(NULL, 0);

	if (NULL == (dp = opendir(path)))
		error_exit("opendir");

	while (NULL != (dirbuf = readdir(dp))) {
		/* get file stat */
		if (-1 == stat(dirbuf->d_name, &statbuff))
			error_exit(dirbuf->d_name);

		if ('.' == dirbuf->d_name[0])
			continue;
		if (S_ISDIR(statbuff.st_mode)) {
			/* if dir, then recursion */
			if (-1 == chdir(dirbuf->d_name))
				error_exit(dirbuf->d_name);
			process_dir();
		} else {
			/*regular file, comput line*/
			namlen = strlen(dirbuf->d_name);
#if 1
			if (strcmp(".c", dirbuf->d_name+namlen-2) 
					&& strcmp(".h", dirbuf->d_name+namlen-2)
					&& strcmp(".sh", dirbuf->d_name+namlen-3)
					&& strcmp("Makefile", dirbuf->d_name)
					) 
				continue;
#endif
			namlen = process_file(dirbuf->d_name);
/* exclude sqlite3 */
			if (namlen < 5000) {
				printf("%-30s %d\n", dirbuf->d_name, namlen);
				total += namlen;
			}
		}
	}
	closedir(dp);
	chdir("..");
	return 0;
}

int process_file(const char *filename)
{
	FILE *fp = NULL;
	char tmp[1024];
	int line = 0;

	if (NULL == (fp = fopen(filename, "r")))
		error_exit(filename);
	
	while (NULL != fgets(tmp, 1024, fp))
		line ++;

	return line;
}
