#include <stdio.h> 
#include "UI.h"
#include <conio.h>
#include <stdlib.h>
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
int main(void)
{
	char* currPos;
	
	char cmd = 0;
	unsigned cnt=0,i=0;
	currarr current;
	UI.init();
	HMP3Decoder dec = MP3InitDecoder();
	MP3FrameInfo info;
	FILE* fp,*fpo;
	fopen_s(&fp,"Test.mp3", "rb");
	fopen_s(&fpo, "Out.out", "w");
	if (fp == NULL)
	{
		printf("Couldnt open file\nPress any key to exit\n");
		_getch();
		return 0;
	}
	printf("Opened MP3 file!\n");
	unsigned char* mp3file;
	short outBuff[4 * 1152];
	int sz,frames=0,skipped=0;
	fseek(fp, 0L, SEEK_END);
	sz = ftell(fp);
	rewind(fp);
	printf("The Mp3 file has a size of: %d\nAttempting to allocate file size in ram\n",sz);
	mp3file =(unsigned char*) malloc(sz);
	if (mp3file == NULL)
	{
		printf("Couldnt =(\nPress any key to exit\n");
		_getch();
		return 0;
	}
	printf("Could =)\nTransfering file to memory:\n");
	for (int i = 0; i < sz; i++)
	{
		mp3file[i] = fgetc(fp);
		if (i % 100000 == 0)
			putchar('#');
	}
	printf("\nFinished\nNow decoding and writing to file\n");
	while (sz > 0)
	{
		int off = MP3FindSyncWord(mp3file, sz);
		if (off == -1)
			break;
		mp3file += off;
		sz -= off;
		int code = MP3Decode(dec, &mp3file, &sz, outBuff, 0);
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
				fprintf(fpo, "%d\n", out);
			}

			printInfo(info);
			printf("Size is now: %d\n", sz);
			frames++;
		}
		else
		{
			if (sz >= 209)
			{
				mp3file += 209;
				sz -= 209;
				printf("Failed frame with code: %d\n",code);
				//int off=MP3FindSyncWord(mp3file, sz);
				//if (off == -1)
					//break;
				//mp3file += off;
				//sz -= off;
				skipped++;
			}
			else
				break;
		}


	}
	printf("Decoded %d frames and skipped %d, %d bytes where left\n",frames,skipped,sz);
	fclose(fp);
	fclose(fpo);
	/*while (cmd != 'r')
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
	}*/
	printf("Thanks for using MP3\n");
	_getch();
	return 0;
}