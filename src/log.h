#ifndef __LOG_H__
#define __LOG_H__

#define LOG_PRINTF_TIME     0
#define LOG_PRINTF          1

#if LOG_PRINTF_TIME
#define LOG (printf("%s:%u %s:%s:\t", __FILE__, __LINE__, __DATE__, __TIME__), printf)
#elif LOG_PRINTF
#define LOG(msg...)  printf("["LOG_HEAD"]"msg)
#else
#define LOG /\/LOG
#endif

void log_byte(unsigned char *buf,int len);
unsigned short char2short(char c1,char c2);
unsigned short limit(unsigned short  input, unsigned short  min, unsigned short  max);

#endif