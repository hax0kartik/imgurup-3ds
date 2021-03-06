#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <3ds.h>
#include <stdarg.h>
#include "common.h"
unsigned char *buffer;
char *lobi;
char links[1024];

void error(int error,const char *format, ...)
{
   va_list aptr;
   va_start (aptr, format);
   char buf[500];
   vsprintf(buf, format, aptr);
   va_end (aptr);
   errorConf err;
   errorInit(&err,ERROR_TEXT_LANGUAGE_WORD_WRAP, CFG_LANGUAGE_EN );
   errorCode(&err, error);
   errorText(&err, buf);
   errorDisp(&err);
}

void wlink(const char *a,const char *b){  
	FILE *pFile;
	char buffer[256];
	char *k=strrchr(a,'/');
	sprintf(buffer,"\n%s : %s",a,k);
	pFile=fopen("upload_links.txt", "a+");
	fprintf(pFile, "%s", buffer);
	fclose(pFile);
}

char *text_ex(char *s,char *PATTERN1,char *PATTERN2)
{
	char *target = NULL;
    char *start, *end;

    if ( start = strstr( s, PATTERN1 ))
    {
        start += strlen( PATTERN1 );
        if ( end = strstr( start, PATTERN2 ))
        {
			target = ( char * )malloc( end - start + 1 );
			memcpy( target, start, end - start );
			target[end - start] = '\0';
        }		
    }
	return target;
} 
char *ReadFile(const char *name, u32 *size)
{ 
	size_t fileLen;
	FILE *file;
	file = fopen(name , "rb");
	if (!file)
    {
      error(12,"Unable to open file %s", name);
      return 'e';
    }    
	fseek(file, 0, SEEK_END);
	fileLen=ftell(file);
	fseek(file, 0, SEEK_SET);                                       
	buffer=(char *)malloc(fileLen+1);
	if (!buffer)
	{
		error(13,"%s","Memory error!");
		fclose(file);
		return 'e';
    }                     
	fread(buffer, fileLen, 1, file);
	*size = fileLen;
	fclose(file);
	return buffer;
}

Result http_upload(const char *url, char *buff, u32 buff_size)
{
	Result ret = 0;
	u32  readsize=0, size=0;
    u8 *buf, *lastbuf;
    httpcContext context;
	int ds = 0;
	ret = httpcOpenContext(&context, HTTPC_METHOD_POST, url, 1);
    if (ret != 0)
	{    error((int)ret,"error 0x%08X\n",(int)ret);
	return 1;
	}
	
    ret = httpcAddRequestHeaderField(&context, (char*)"User-Agent", (char*)"IMGURUP3DS/1.1");
    if (ret != 0){
		  error((int)ret,"error in ARHF 0x%08X\n",(int)ret);
		  return 1;
         }
	ret = httpcAddRequestHeaderField(&context,(char*)"Authorization",(char*)"Client-ID 1ec629ae0a99590");	 
     if (ret != 0){
		  error((int)error,"error in ARHF 0x%08X\n",(int)ret);
		  return 1;
         }
		 
	ret = httpcSetSSLOpt(&context, 1 << 9);
    if (ret != 0){
		  error((int)ret,"error in SSLO 0x%08X\n",(int)ret);
		  return 1;
         }
		 
	ret= httpcAddPostDataBinary(&context, (char*)"image", buff ,buff_size);
	printf("\x1b[32;1mRet 0x%08X\n",(int)ret);
    ret= httpcAddPostDataAscii (&context, (char*)"type","file");
	printf("\x1b[32;1mRet 0x%08X\n",(int)ret);
    ret = httpcBeginRequest(&context);
    if (ret != 0){
		 error((int)ret,"error in HPR 0x%08Xn",(int)ret);
		  return 1;
         }

	gfxFlushBuffers();
	gfxSwapBuffers();
    buf = (u8*)malloc(0x1000);
    if (buf == NULL)
	{
		error(98,"error while setting buffer\n");
        return -1;
    //memset(buf, 0, size);
	}
    do {
		// This download loop resizes the buffer as data is read.
		ret = httpcDownloadData(&context, buf+size, 0x1000, &readsize);
		size += readsize;
		//draw(contentsize,size);
		
		if (ret == (s32)HTTPC_RESULTCODE_DOWNLOADPENDING){
			   
				lastbuf = buf; // Save the old pointer, in case realloc() fails.
				buf = realloc(buf, size + 0x1000);
				if(buf==NULL){ 
					httpcCloseContext(&context);
					free(lastbuf);
				    return -2;
					}
			}
	} while (ret == (s32)HTTPC_RESULTCODE_DOWNLOADPENDING);	
     if (ret!=0)
	 {
		 httpcCloseContext(&context);
		 return 1;
     }
	lastbuf = buf;
	buf = realloc(buf, size);
	printf("\n");
	printf("size : %d \n",(int)size);
	if(size>500)
	printf("\x1b[32;1mlink :%s\n\x1b[37;1m",text_ex((char*)buf,"<link>","</link>"));
    else 
	{
	error(123, text_ex((char*)buf,"<error>","</error>"));
     ret = 1;
	}
	gfxFlushBuffers();
	gfxSwapBuffers();
	
	if(buf==NULL){ // realloc() failed.
		httpcCloseContext(&context);
		free(lastbuf);
	    return 1;
		}
    httpcCloseContext(&context);
 
	free(buf);
	if(ret==0)
	strcpy(links,text_ex((char*)buf,"<link>","</link>"));
    return 0;
}
char mybuf[100];
char *tl(SwkbdState swkbd ,char *texgen)
{        
            SwkbdButton button = SWKBD_BUTTON_NONE;
            swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 2, -1);
            swkbdSetHintText(&swkbd, texgen);
            swkbdSetValidation(&swkbd, SWKBD_NOTEMPTY_NOTBLANK, 0, 0);
	         button = swkbdInputText(&swkbd, mybuf, sizeof(mybuf));
			//printf("%d\n",w);
			return mybuf;
}
int main()
{
    Result ret=0;
	u32 size = 0;
	osSetSpeedupEnable(true);
    gfxInitDefault();
    httpcInit(0x500000); // Buffer size when POST/PUT.
    consoleInit(GFX_BOTTOM,NULL);
	consoleInit(GFX_TOP, &topScreen);
	SwkbdState swkbd;
	printf("Welcome to IMGURUP-3DS\n");
	printf("Press B to open the xplorer\n");
	
    gfxFlushBuffers();
    // Main loop
    while (aptMainLoop())
    {
        gspWaitForVBlank();
        hidScanInput();
        // Your code goes here
		u32 kDown = hidKeysDown();
		if(ret==0)
		{
			
            if(kDown & KEY_B)
			{
			    gfxFlushBuffers();
                gfxSwapBuffers();
				consoleSelect(&topScreen);
				xplorer();
				consoleSelect(&topScreen);
				printf("\x1b[2J");
	            printf("Uploading :%s\n",current_file);
				if(current_file[0]!=0)
				{	
				    char *a=ReadFile(current_file, &size);
				    ret=http_upload("https://api.imgur.com/3/image.xml", a, size);
					if(ret==0)
					{ 
				    wlink(links, current_file);
					for(int i=0;i<=511;i++)current_file[i]='\0';
					printf("Press B to open the explorer.\n");
					}
				}
			}				
		    
	    }
        if (kDown & KEY_START)
            break; // break in order to return to hbmenu
        // Flush and swap framebuffers
        gfxFlushBuffers();
        gfxSwapBuffers();
    }
    // Exit services
    httpcExit();
    gfxExit();
    return 0;
}
