#ifndef __BASE64_H__
#define __BASE64_H__

extern int base64decode(void *dst,char *src,int maxlen);
extern void base64encode(unsigned char *from, char *to, int len);

#endif /*__BASE64_H__*/

