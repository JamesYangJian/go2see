#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "base64.h"

//static char base64chars[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
//                              "abcdefghijklmnopqrstuvwxyz0123456789+/";

static char base64chars[65] = "0123456789abcdefghijklmnopqrstuvwxyz"
                              "ABCDEFGHIJKLMNOPQRSTUVWXYZ.-";
/*
 * Name: base64decode
 *
 * Description: Decodes local BASE-64 encoded string
 */
int base64decode(void *dst,char *src,int maxlen)
{
 int bitval,bits;
 int val;
 int len,x,y;

 len = strlen(src);
 bitval=0;
 bits=0;
 y=0;

 for(x=0;x<len;x++)
  {
/*
   if ((src[x]>='A')&&(src[x]<='Z')) val=src[x]-'A'; else
   if ((src[x]>='a')&&(src[x]<='z')) val=src[x]-'a'+26; else
   if ((src[x]>='0')&&(src[x]<='9')) val=src[x]-'0'+52; else
   if (src[x]=='+') val=62; else
   if (src[x]=='/') val=63; else
    val=-1;
*/

   if ((src[x]>='0')&&(src[x]<='9')) val=src[x]-'0'; else
   if ((src[x]>='a')&&(src[x]<='z')) val=src[x]-'a'+10; else
   if ((src[x]>='A')&&(src[x]<='Z')) val=src[x]-'A'+36; else
   if (src[x]=='.') val=62; else
   if (src[x]=='-') val=63; else
    val=-1;

   if (val>=0)
    {
     bitval=bitval<<6;
     bitval+=val;
     bits+=6;
     while (bits>=8)
      {
       if (y<maxlen)
        ((char *)dst)[y++]=(bitval>>(bits-8))&0xFF;
       bits-=8;
       bitval &= (1<<bits)-1;
      }
    }
  }
 if (y<maxlen)
   ((char *)dst)[y++]=0;
 return y;
}


/*
 * Name: base64encode()
 *
 * Description: Encodes a buffer using BASE64.
 */
void base64encode(unsigned char *from, char *to, int len)
{
  while (len) {
    unsigned long k;
    int c, kPad;

    c = (len < 3) ? len : 3;
    kPad = 3 - c;
    k = 0;
    len -= c;
    while (c--)
        k = (k << 8) | *from++;
    /*kPad add by yangjian to fix the bug 20070619*/
    while (kPad--)
        k = k << 8;
    *to++ = base64chars[ (k >> 18) & 0x3f ];
    *to++ = base64chars[ (k >> 12) & 0x3f ];
    *to++ = base64chars[ (k >> 6) & 0x3f ];
    *to++ = base64chars[ k & 0x3f ];
  }
  *to++ = 0;
}


