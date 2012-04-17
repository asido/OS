#include <libc.h>
#include <fs/vfs.h>

int ls_main(int argc, char **argv)
{
	struct fileinfo **file_list;
	char *dir;
	int i;

	if (argc == 1)
		dir = "/";
	else
		dir = argv[1];

	file_list = get_file_list(dir);
	if (!file_list)
	{
		printf("Error retrieving file list\n");
		return 1;
	}

	for (i = 0; file_list[i]; i++)
	{
		if (strcmp(dir, "/") != 0)
			printf("%s", dir);
		if (file_list[i]->filename[0] != '/')
			putchar('/');
		printf("%s\n", file_list[i]->filename);
	}

	free(file_list);

	return 0;
}
