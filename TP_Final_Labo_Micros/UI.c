#include "UI.h"
#include <dirent.h> 
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifdef __cplusplus
extern "C" {
#endif

	static char current[256][1025];
	static char file[1024]=".";

	struct dirent *de;  // Pointer for directory entry 

	static int i = 0, cnt = 0, newFile = 0, onFile=0, ini = 1,len=1;

	static DIR *tmp, *dr;

	int isDirectory(const char *file,const char* path) {
		struct stat statbuf;
		char temp[1024]="";
		strcat_s(temp,1024, path);
		strcat_s(temp,1024,"/");
		strcat_s(temp,1024, file);
		if (stat(temp, &statbuf) != 0)
			return 0;
		return S_ISDIR(statbuf.st_mode);
	}


	unsigned init() {
		dr = opendir(".");
		if (dr == NULL)  // opendir returns NULL if couldn't open directory 
			ini = 0;
		else
			while ((de = readdir(dr)) != NULL && cnt < 256)
			{
				strncpy_s(current[cnt++], 1024, de->d_name, 1024);
				current[cnt - 1][1024] = (char)isDirectory(current[cnt - 1],file);
			}
		return ini;

	};

	char input(char cmd) {
		if (onFile == 1)
		{
			return cmd;
		}
		if (cmd == 's')
		{
			i = i + 1 >= cnt ? cnt - 1 : i + 1;
			return 0;
		}
		else if (cmd == 'w')
		{
			i = i - 1 <= 0 ? 0 : i - 1;
			return 0;
		}

		else if (cmd == 'e')
		{
			if (i == 0 || (i == 1 && len==1))
				return 1;
			char temp[1024] = "";
			strcat_s(temp, 1024, file);
			strcat_s(temp, 1024, "/");
			strcat_s(temp, 1024, current[i]);
			tmp = opendir(temp);
			if (tmp != NULL)
			{
				if (i != 1)
				{
					strcat_s(file, 1024, "/");
					strcat_s(file, 1024, current[i]);
					len += strlen(current[i]) + 1;
				}
				else if (i == 1)
				{
					for (int k = len - 1; k > 0; k--)
					{
						if (file[k] == '/')
						{
							len = k;
							file[k] = 0;
							break;
						}
						file[k] = 0;

					}
				}
				cnt = 0;
				i = 0;
				dr = tmp;
				while ((de = readdir(dr)) != NULL && cnt < 256)
				{
					strncpy_s(current[cnt++], 1024, de->d_name, 1024);
					current[cnt - 1][1024] = (char)isDirectory(current[cnt - 1],file);
				}
				
				
			}
			else
			{
				strcat_s(file, 1024, "/");
				strcat_s(file, 1024, current[i]);
				len += strlen(current[i]) + 1;
				newFile = 1;
				onFile = 1;
			}
			return 0;
		}
		return cmd;
	};
	char* getPath() {
		return file;
	}
	char* getFile() {
		if (newFile == 1)
		{
			newFile = 0;
			return getPath();
		}
		return 0;
	};

	currarr  getCurrent(unsigned* sz, unsigned *pos)
	{
		*sz = cnt;
		*pos = i;
		return (currarr)current;
	}
	void close() {
		closedir(dr);

	};
	void exitFile()
	{
		if (onFile == 1)
		{
			onFile = 0;
			newFile = 0;
			for (int k = len - 1; k > 0; k--)
			{
				if (file[k] == '/')
				{
					len = k;
					file[k] = 0;
					break;
				}
				file[k] = 0;
			}
		}
	}
	void setPos(int p)
	{
		if(i>=0 && i<cnt)
			i = p;
	}

	UI_ UI = { .init = init,.input = input,.getFile = getFile,.getPath=getPath,.getCurrent = getCurrent,.close = close,.exitFile=exitFile, .setPos=setPos };
#ifdef __cplusplus
}
#endif