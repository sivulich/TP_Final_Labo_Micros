#include <stdio.h> 
#include "UI.h"
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include "./helix/pub/mp3dec.h"
void printInfo(MP3FrameInfo info)
{
	printf("\nDecoded a frame with:\nBitrate: %d\nBitsPerSample: %d\nLayer: %d\nNchans: %d\nOutSamp: %d\nSampRate: %d\nVersion: %d\n",
		info.bitrate,
		info.bitsPerSample,
		info.layer,
		info.nChans,
		info.outputSamps,
		info.samprate,
		info.version
	);
	return;
}
int checkMP3file(char* fn, unsigned sz)
{
	if (fn[sz - 1] == '3' && fn[sz - 2] == 'p' && fn[sz - 3] == 'm'&& fn[sz - 4] == '.')
		return 1;
	return 0;
}
int handleMp3File(char* fn)
{
	HMP3Decoder dec = MP3InitDecoder();
	MP3FrameInfo info;
	FILE* fp, *fpo;
	fopen_s(&fp, fn, "rb");
	fopen_s(&fpo, "Out.out", "w");
	if (fp == NULL)
	{
		printf("Couldnt open file\nPress any key to exit\n");
		_getch();
		return 1;
	}
	unsigned char* mp3file;
	short outBuff[4 * 1152];
	int sz,lsz, frames = 0, skipped = 0;
	fseek(fp, 0L, SEEK_END);
	sz = ftell(fp);
	rewind(fp);
	mp3file = (unsigned char*)malloc(sz);
	if (mp3file == NULL)
	{
		printf("Couldnt allocate memory\nPress any key to continue\n");
		_getch();
		return 1;
	}
	for (int i = 0; i < sz; i++)
	{
		mp3file[i] = fgetc(fp);
		if (i % 100000 == 0)
			putchar('#');
	}
	lsz = sz;
	putchar('\n');
	while (sz > 0)
	{
		int off = MP3FindSyncWord(mp3file, sz);
		if (off == -1)
			break;
		mp3file += off;
		sz -= off;
		int code = MP3Decode(dec, &mp3file, &sz, outBuff, 0);
		if (sz < 0.9*lsz)
		{
			putchar('x');
			lsz = sz;
		}
		if (code == 0)
		{
			MP3GetLastFrameInfo(dec, &info);
			for (int k = 0; k < info.outputSamps / info.nChans; k++)
			{
				int out = 0;
				for (int c = 0; c < info.nChans; c++)
				{
					int samp = 0;
					for (int b = 0; b < info.bitsPerSample / 16; b++)
					{
						samp = (samp << 16) + outBuff[k*info.nChans*info.bitsPerSample / 16 + c * info.bitsPerSample / 16 + b];
					}
					out += samp;
				}
				out /= info.nChans;
				fprintf(fpo,"%d,", out);
			}
			frames++;
		}
		else
		{
			if (sz >= 209)
			{
				mp3file += 209;
				sz -= 209;
				printf("Failed frame with code: %d\n", code);
				skipped++;
			}
			else
				break;
		}
	}
	putchar('\n');
	printInfo(info);
	fclose(fp);
	fclose(fpo);
}
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
			if (checkMP3file(currPos, strlen(currPos)))
			{
				printf("Decoding file:\n");
				handleMp3File(currPos);
				printf("Done\n");
			}
			else
				printf("Not an MP3 file\n");
			_getch();
			UI.exitFile();
			cmd = 0;
		}
	}
	printf("Thanks for using MP3\n");
	_getch();
	return 0;
}