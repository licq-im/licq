// -*- c-basic-offset: 2 -*-
/* ----------------------------------------------------------------------------
 * Licq - A ICQ Client for Unix
 * Copyright (C) 1998 - 2003 Licq developers
 *
 * This program is licensed under the terms found in the LICENSE file.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef USE_HEBREW
/*
		Ok.. thats an update to the Hebew-Reversing module.
		It had a small bug in iseng()....
		Sorry about that.
		
		Plus! A new email all comments should be send to:
		^-RaveN-^ (raven@math.technion.ac.il)
*/


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>


static inline int iswhitespace(const char ch)
{
	return (ch == ' ' ||  ch == 8);
}

static inline int isheb(const char ch)
{
	return (((unsigned char)ch) >= 0xE0  &&  ((unsigned char)ch) <= 0xFA);
}

static inline int iseng(const char ch)
{
	return ((ch >= 'A'  &&  ch <= 'Z')  ||  (ch >= 'a'  &&  ch <= 'z'));
}


char *strhebpatch(char *dest, const char *src)
{
	short int mode = 0, imode;
	const char *hmark = NULL, *lmark, *nmark, *nlmark;
	char ch;
	char* temp_str = NULL;

	if (src == NULL)
		return NULL;
	
	if ((temp_str = (char*)malloc(strlen(src) + 1)) == NULL)
		return NULL;

	
	for (;;)
	{
		if (mode == 0)
		{
			if (isheb(*src))
			{
				hmark = src;
				mode = 1;
			}
			else
				*dest++ = *src;
		}
		else if (mode == 1)
		{
			if (*src == 0  ||  iseng(*src))
			{
				lmark = src-1;
				while ((!isheb(*lmark)) && (!ispunct(*lmark))) lmark--;
				src = lmark;
				imode = 0;
				nmark = NULL;

				while (lmark >= hmark)
				{
					ch = *lmark;

					if (imode == 0)
						switch (ch)
						{
							case '(': ch = ')'; break;
							case ')': ch = '('; break;
							case '{': ch = '}'; break;
							case '}': ch = '{'; break;
							case '[': ch = ']'; break;
							case ']': ch = '['; break;
						}

					if (imode == 0)
					{
						if (isdigit(ch))
						{
							imode = 1;
							nmark = lmark;			
						}
						else
							*dest++ = ch;
					}
					else
  						if (imode == 1  &&  (isheb(ch) || iswhitespace(ch) || ispunct(ch)))
						{
							nlmark = lmark+1;
							while (nlmark <= nmark)
								*dest++ = *nlmark++;
							imode = 0;
							lmark++;
						}
					lmark--;
				}
				
				hmark = NULL;
				mode = 0;
			}
		}
		if (!*src++)
			break;
	}
	return dest;
}

char *hebrew(char *pszStr)
{
	char* temp_str = NULL, *temp = NULL;

	if (pszStr == NULL)
		return NULL;

	if ((temp_str = (char*) malloc(strlen(pszStr)+1)) == NULL)
		return NULL;
	temp = temp_str;

	temp_str = strhebpatch(temp_str, pszStr);
 	return temp;
	
}
char* GetArg(char* input, int index)
{
	char *temp=NULL, *tmp=NULL;
	char *arg=NULL, *arg2=NULL;
	int i=0;

	temp = (char*) malloc(strlen(input)+1);
	tmp = temp;

	strcpy(temp, input);

	arg = strtok(temp, "\n");
	for(i = 0; (i < index) && (arg!=NULL); i++)
		arg = strtok(NULL, "\n");

	if(arg != NULL)
	{
		arg2 = (char*) malloc(strlen(arg)+1);
		strcpy(arg2, arg);
        }	
	free(tmp);
	return arg2;
}

char* hebrev(char* src)
{
        char* temp_str = NULL;
	char* temp = NULL;
        int i=0;
	int size = 0;
	if(src == NULL)
		return NULL;

	if((temp_str = (char*)malloc(strlen(src)+1))== NULL)
		return NULL;

	temp = GetArg(src, i);
	while(temp != NULL)
	{
		i++;
		temp = hebrew(temp);
		memcpy(temp_str + size , temp, strlen(temp));
		size += strlen(temp) ;
		temp_str[size++] = '\n';
	
		free(temp);
		temp = NULL;
		temp = GetArg(src, i);
	}
	free(temp);
	temp_str[size]= '\0';	
	return temp_str;
}

#endif // hebrew support: USE_HEBREW
