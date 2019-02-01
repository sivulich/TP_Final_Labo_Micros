#include <stdlib.h>
#include <windows.h>
#include <SDL.h>
#include "lvgl/lvgl.h"
#include "lv_drivers/display/monitor.h"
#include "lv_drivers/indev/mouse.h"
#include "lv_drivers/indev/keyboard.h"
#include "MP3Ui.h"

#include <stdio.h> 
#include "UI.h"
#include <conio.h>
#include <string.h>
#include "./helix/pub/mp3dec.h"
#include "./read_ID3_info/read_id3.h"

/**********************
*  STATIC PROTOTYPES
**********************/
static void hal_init(void);
static int tick_thread(void *data);

/**********************
*  STATIC VARIABLES
**********************/
static lv_indev_t * kb_indev;
static lv_indev_drv_t kb_drv;

unsigned char* id3TagInfo(const char* tag, unsigned char* mp3File, int headerSize, int* size)
{
	int i = 0, len = 0;
	char found = false;
	for (i = 0; i < (headerSize - 4); i++)
	{
		if (memcmp(mp3File + i, tag, 4) == 0)
		{
			//checkear 4 bytes de size, 2 bytes de flag, 1 byte de encoding
			//leer contenido hasta /0
			i += 4;
			if (*(mp3File + i) != 0)
				break;
			i += 4 + 2;
			switch (*(mp3File + i))
			{
			case 0: case 3:
				//printf("UTF-8\n");
				found = true;
				break;
			case 1: case 2:
				found = true;
				break;
			default:
				break;
			}
			i += 1;
			break;
		}
	}
	if (found)
		while (*(mp3File + i + len) != 0)
			len++;
	*size = len;
	//devolver contenido
	return (mp3File + i);
}


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

void printSongInfo(FIL* fp)
{
	char str[40];
	read_ID3_info(TITLE_ID3, str, sizeof(str), fp);
	printf("Title: %s\n", str);
	read_ID3_info(ALBUM_ID3, str, sizeof(str), fp);
	printf("Album: %s\n", str);
	read_ID3_info(ARTIST_ID3, str, sizeof(str), fp);
	printf("Artist: %s\n", str);
	read_ID3_info(LENGTH_ID3, str, sizeof(str), fp);
	printf("Length: %s\n", str);
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
	else
	{
		printSongInfo(fp);
	}
	unsigned char* mp3file;
	short outBuff[4 * 1152];
	int sz,lsz, frames = 0, skipped = 0,duration=0,nFrames=0,dmy;
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
		{
			break;
		}
		if (off > 0)
		{
			printf("Title of song: %s\n", id3TagInfo("TIT2",mp3file,off,&dmy));
			printf("Album of song: %s\n", id3TagInfo("TPE1", mp3file, off, &dmy));	
			printf("Length of song: %s\n", id3TagInfo("TLEN", mp3file, off, &dmy));
			lsz -= off;
		}
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
			nFrames++;
			MP3GetLastFrameInfo(dec, &info);
			if (duration == 0)
			{
				duration = ((double)lsz*8) / info.bitrate;
				printf("Duration is: %d:%d\n", duration/60,duration%60);
			}
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
	printf("Nframes %d\n", nFrames);
	return 0;
}
int main(int arc, char**)
{
	char* currPos;
	/*Initialize LittlevGL*/
	lv_init();

	/*Initialize the HAL for LittlevGL*/
	hal_init();

	/*Load a demo*/
	//demo_create();
	MP3UiCreate(&kb_drv);

	while (1) {
		/* Periodically call the lv_task handler.
		* It could be done in a timer interrupt or an OS task too.*/
		lv_task_handler();
		Sleep(10);       /*Just to let the system breathe */
	}


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

/**
* Initialize the Hardware Abstraction Layer (HAL) for the Littlev graphics library
*/
static void hal_init(void)
{
	/* Add a display
	* Use the 'monitor' driver which creates window on PC's monitor to simulate a display*/
	monitor_init();
	lv_disp_drv_t disp_drv;
	lv_disp_drv_init(&disp_drv);            /*Basic initialization*/
	disp_drv.disp_flush = monitor_flush;
	disp_drv.disp_fill = monitor_fill;
	disp_drv.disp_map = monitor_map;
	lv_disp_drv_register(&disp_drv);

	/* Add the mouse (or touchpad) as input device
	* Use the 'mouse' driver which reads the PC's mouse*/
	mouse_init();
	lv_indev_drv_t indev_drv;
	lv_indev_drv_init(&indev_drv);          /*Basic initialization*/
	indev_drv.type = LV_INDEV_TYPE_POINTER;
	indev_drv.read = mouse_read;         /*This function will be called periodically (by the library) to get the mouse position and state*/
	lv_indev_drv_register(&indev_drv);

	/* If the PC keyboard driver is enabled in`lv_drv_conf.h`
	* add this as an input device. It might be used in some examples. */
#if USE_KEYBOARD
	
	kb_drv.type = LV_INDEV_TYPE_KEYPAD;
	kb_drv.read = keyboard_read;
	kb_indev = lv_indev_drv_register(&kb_drv);
#endif

	/* Tick init.
	* You have to call 'lv_tick_inc()' in every milliseconds
	* Create an SDL thread to do this*/
	SDL_CreateThread(tick_thread, "tick", NULL);
}

/**
* A task to measure the elapsed time for LittlevGL
* @param data unused
* @return never return
*/
static int tick_thread(void *data)
{
	while (1) {
		lv_tick_inc(1);
		SDL_Delay(1);   /*Sleep for 1 millisecond*/
	}

	return 0;
}
