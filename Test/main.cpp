#include <cstdio>
#include <string.h>
void splitpath(const char *path, char*dir, char* fname, char*ext)
{
	if (path == NULL)
	{
		dir[0] = '\0';
		fname[0] = '\0';
		return;
	}

	if (path[strlen(path)] == '/')
	{
		strcpy(dir, path);
		fname[0] = '\0';
		return;
	}
	char *whole_name = (char *)rindex(path, '/');
	
	strcpy(dir, whole_name);
	if (whole_name != NULL)
	{
		char *pext = rindex(whole_name, '.');

		if (pext != NULL)
		{
			strcpy(ext, pext+1);
			
		}
	}

}
int main()
{
   
	char path[128] = "root/mav/we/w.mp3";
	char dir[128], name[128], ext[128];

	splitpath(path, dir, name, ext);
	printf("%s\t%s\t%s\n", dir, name, ext);
    return 0;
}