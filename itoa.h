/*
 *        'itoa.h'
 *     (C) 2014.4 GordonChen
 *
 */
#ifndef _ITOA_H
#define _ITOA_H

#include <stdio.h>
#include <string.h>
#include <math.h>

//将整数转换为字符串
void itoa(int n, char s[])
{
	int i, sign;
	void reverse(char s[]);
	sign = n;
	i = 0;
	do
	{
		s[i++] = abs(n % 10) + '0';
	}
	while ((n /= 10) != 0);
	if (sign < 0)
		s[i++] = '-';
	s[i] = '\0';
	reverse(s);
}

void reverse(char s[])
{
	char tmp;
	int len = strlen(s);
	int i, j;
	i = 0;
	j = len - 1;
	while (i < j)
	{
		tmp = s[i];
		s[i] = s[j];
		s[j] = tmp;
		i++;
		j--;
	}
}

#endif
