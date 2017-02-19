#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <3ds.h>
unsigned char *buffer;
char *lobi;
static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                '4', '5', '6', '7', '8', '9', '+', '/'};
static char *decoding_table = NULL;
static int mod_table[] = {0, 2, 1};

void build_decoding_table() {

  decoding_table = malloc(256);

  for (int i = 0; i < 64; i++)
    decoding_table[(unsigned char) encoding_table[i]] = i;
}

char *base64_encode(const unsigned char *data,
                    size_t input_length,
                    size_t *output_length) {

  *output_length = 4 * ((input_length + 2) / 3);

  char *encoded_data = malloc(*output_length);
  if (encoded_data == NULL) return NULL;

  for (int i = 0, j = 0; i < input_length;) {

    uint32_t octet_a = i < input_length ? (unsigned char)data[i++] : 0;
    uint32_t octet_b = i < input_length ? (unsigned char)data[i++] : 0;
    uint32_t octet_c = i < input_length ? (unsigned char)data[i++] : 0;

    uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

    encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
    encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
    encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
    encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
  }

  for (int i = 0; i < mod_table[input_length % 3]; i++)
    encoded_data[*output_length - 1 - i] = '=';

  return encoded_data;
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
char *ReadFile(const char *name)
{ size_t fileLen;
  FILE *file;
  file = fopen(name , "rb");
  if (!file)
    {
      printf( "Unable to open file %s", name);
      return 'e';
    }    
  fseek(file, 0, SEEK_END);
  fileLen=ftell(file);
  fseek(file, 0, SEEK_SET);                                       
  buffer=(char *)malloc(fileLen+1);
  if (!buffer)
    {
      fprintf(stderr, "Memory error!");
      fclose(file);
      return 'e';
    }                     
  fread(buffer, fileLen, 1, file);
  size_t l =0;
  lobi=base64_encode(buffer,fileLen,&l);
  fclose(file);
  return lobi;
}

Result http_upload(const char *url,char *buff)
{
     Result ret = 0;
	u32  readsize=0, size=0;
    u8 *buf, *lastbuf;
    httpcContext context;
	int ds = 0;
	ret = httpcOpenContext(&context, HTTPC_METHOD_POST, url, 1);
    if (ret != 0)
	{printf("\x1b[31;1merror 0x%08X\n",(int)ret);
	return 1;
	}
	
    ret = httpcAddRequestHeaderField(&context, (char*)"User-Agent", (char*)"IMGURUP3DS/1.1");
    if (ret != 0){
		 printf("\x1b[31;1merror in ARHF 0x%08X\n",(int)ret);
		  return 1;
         }
	ret = httpcAddRequestHeaderField(&context,(char*)"Authorization",(char*)"Client-ID 1ec629ae0a99590");	 
     if (ret != 0){
		 printf("\x1b[31;1merror in ARHF 0x%08X\n",(int)ret);
		  return 1;
         }
		 
	ret = httpcSetSSLOpt(&context, 1 << 9);
    if (ret != 0){
		 printf("\x1b[31;1merror in SSLO 0x%08X\n",(int)ret);
		  return 1;
         }
		 
	ret= httpcAddPostDataAscii (&context, (char*)"image",buff);
	printf("\x1b[32;1mRet 0x%08X\n",(int)ret);
    ret= httpcAddPostDataAscii (&context, (char*)"type","base64");
	printf("\x1b[32;1mRet 0x%08X\n",(int)ret);
    ret = httpcBeginRequest(&context);
    if (ret != 0){
		 printf("\x1b[31;1merror in HPR 0x%08X\x1b[37;1m\n",(int)ret);
		  return 1;
         }

	gfxFlushBuffers();
	gfxSwapBuffers();
    buf = (u8*)malloc(0x1000);
    if (buf == NULL)
	{
		printf("\x1b[31;1merror while setting buffer\n\x1b[37;1m");
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
	printf("\x1b[32;1m ERR returned:%s\n\x1b[37;1m", text_ex((char*)buf,"<error>","</error>"));
	gfxFlushBuffers();
	gfxSwapBuffers();
	
	if(buf==NULL){ // realloc() failed.
		httpcCloseContext(&context);
		free(lastbuf);
	    return 1;
		}
    httpcCloseContext(&context);
 
	free(buf);
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
    gfxInitDefault();
    httpcInit(0x500000); // Buffer size when POST/PUT.
    consoleInit(GFX_BOTTOM,NULL);
	SwkbdState swkbd;
	printf("Welcome to IMGURUP-3DS\n");
	printf("Press A to bring up the keyboard\n");
	
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
			if(kDown & KEY_A)
			{
			    char *path=tl(swkbd,"Enter the location of the photo Ex:-/in.jpg,/3ds/k.jpg");
		        char *a=ReadFile(path);
	            printf("Uploading %s\n",path);
                ret=http_upload("https://api.imgur.com/3/image.xml",a);
			    if(ret==0)
			    {
		         printf("Press A to bring up Keyboard once again.\n");
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
