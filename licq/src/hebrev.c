// -------------------------------------------------
// THIS FILE IS JACOB SHAPIRO'S CODE 
// -------------------------------------------------
// Last Edited: 15/12/2000
// Editor Details: 
// Email = yash-s@inter.net.il
// ICQ = 58019576
// nick = Psycho_pr
// *** SPECIAL THANKS TO Re'em Bensimhon ****

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef USE_HEBREW

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

// DEFINITIONS
#define isheb(c)(((((unsigned char) c)>=224) && (((unsigned char) c)<=250)) ? 1 : 0)
#define isnewline(c)    (((((unsigned char) c)=='\n' ||  ((unsigned char) c)== '\r')) ? 1 : 0)


/* The algorithem is very simple. first thing i do, is duplicate 
 * the string. after i did that, i scan the string, char by char 
 * for hebrew characters. when i found a hebrew char, i determent 
 * where the hebrew word or block ends by scanning all the chars 
 * after it to see if they are hebrew or spaces or any other 
 * printable char that is hebrew. after i know the block size of the 
 * hebrew block, i start taking chars from the end of the block at 
 * the old str and put the at the start of the block of the new 
 * string. That i do for the whole string till i'm doen, then i just 
 * return the str.
        have fun, jacob.
*/
char *hebrev(char *pszStr)
{
	unsigned long len = strlen(pszStr), i, blockstart, blockend;
	char *pszNew = malloc(len+1);
	memcpy(pszNew, pszStr, len+1);
	for (i = 0; i <= len; i++)
	{
		if (isheb(*(pszStr+i)))
		{
			/* okay, we found a hebrew char, now we need 
			* to find the end of it.*/
			blockstart=i;
			for (blockend=i; blockend<len; blockend++)
			{
				if (!( isheb(*(pszStr+blockend)) || 
				   isblank(*(pszStr+blockend)) || 
				   ispunct(*(pszStr+blockend)) ))
					break;
			}
			/* okay, now we have the block start and the 
			*  block end, all we have to do is copy chars
			*  now... */
			for (blockend--; i<=blockend; i++)
			{
				/* now we copy every (i) char for 
				*  pszStr into 
				*  (blockend-(i-blockstart)) 
				*  char at pszNew */
		        	*(pszNew+(blockend-(i-blockstart))) 
				       = *(pszStr+i);
			}
			/* now that we're done with that, and i is 
			 * allready advanced out of the block (i hope)
			 * , we can continue the main loop*/
		}
	}
	return pszNew;
}

#endif // USE_HEBREW
