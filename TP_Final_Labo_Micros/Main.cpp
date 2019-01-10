#include <stdio.h> 
#include "UI.h"
#include <conio.h>
#include <stdlib.h>
int main(void)
{
	char* currPos;
	
	char cmd = 0;
	unsigned cnt=0,i=0;
	currarr current;
	UI.init();
	while (cmd != 'r')
	{
		UI.input(cmd);
		current = UI.getCurrent(&cnt,&i);
		system("cls");
		currPos = UI.getFile();
		if (currPos == 0)
		{
			printf("Current path: %s\n", UI.getPath());
			for (unsigned j = 0; j < cnt; j++)
			{
				if (i == j)
					printf("->");
				printf("%s\n", current[j]);

			}
			cmd = _getch();
		}
		else
		{
			printf("Selected the file %s\nPress any key to exit\n", currPos);
			_getch();
			UI.exitFile();
			cmd = 0;
		}
		
		
		
		
	}
	
	

	system("cls");
	printf("Thanks for using MP3\n");
	_getch();
	return 0;
}