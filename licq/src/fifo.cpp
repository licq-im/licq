/*
 * FIFO commands
 *
 * TODO things
 *	o file transfers
 * THOUGTHS 
 *	o a flag so message, url, etc can been forced to fail if buddy is not
 *	| in the list ?
 *
 * If anyone changes anything here try to update the README.FIFO
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include "time-fix.h"

#ifdef HAVE_ERRNO_H
#include <errno.h>
#else
extern int errno;
#endif

#include "licq_icq.h"
#include "licq_user.h"
#include "licq_constants.h"
#include "licq_file.h"
#include "licq_log.h"
#include "licq_translate.h"
#include "licq_packets.h"
#include "licq_plugind.h"
#include "licq.h"
#include "support.h"

#include "licq_icqd.h"


#define ReportMissingParams(cmdname) \
	(gLog.Info("%s `%s': missing arguments. try `help %s'\n",  \
	L_FIFOxSTR,cmdname,cmdname))
	
#define ReportBadBuddy(cmdname,szUin) \
	(gLog.Info("%s `%s': bad buddy string `%s'\n",L_FIFOxSTR,cmdname,szUin))

static const char* const HELP_STATUS = 
	"\tstatus <[*]<status>> <auto response>\n"
	"\t\tstatus: online, offline, na, away, occupied, dnd, ffc\n"
	"\t\tSets the status of the current licq session\n"
	"\t\tto that given (precede the the status by a\n"
	"\t\t\"*\" for invisible mode)\n";
static const char* const HELP_AUTO = 
	"\tauto_response <auto response>\n"
	"\t\tSets the auto response message without\n"
	"\t\tchanging the current status.\n";
static const char* const HELP_MSG =
	"\tmessage <buddy> <message>\n"
	"\t\tSend a message to the given buddy.\n";
static const char* const HELP_URL = 
	"\turl <buddy> <url> [<description>]\n"
	"\t\tSend a url to the given buddy.\n";
static const char* const HELP_REDIRECT = 
	"\tredirect <file>\n"
	"\t\tRedirects stderr for\n"
	"\t\tlicq to the given file\n";
static const char* const HELP_DEBUGLVL = 
	"\tdebuglvl <level>\n"
	"\t\tset what information is logged\n"
	"\t\tsee <level> in licq -h\n";
static const char* const HELP_ADDUSER = 
	"\tadduser <buddy>\n"
	"\t\t add a user to your contact list. note\n"
	"\t\tthat buddy must be an uin\n";
static const char* const HELP_USERINFO = 
	"\tuserinfo <buddy>\n"
	"\t\tupdates a buddy's user information\n";
static const char* const HELP_EXIT =
	"\texit\n"
	"\t\t causes the licq session to shutdown.\n";
static const char* const HELP_UIVIEWEVENT = 
	"\tui_viewevent [<buddy>]\n"
	"\t\tShows the oldest pending event\n";
static const char* const HELP_UIMESSAGE =
	"\tui_message <buddy>\n"
	"\t\tOpen the plugin message composer to <buddy>\n";
static const char* const HELP_HELP = 
	"\thelp <command>\n" 
	"\t\tprint commands help information.\n";

#define MAX_ARGV 	64

enum
{	CL_UNKNOWN=-1,
	CL_NONE=-2
};

struct command_t
{	const char *const szName;
	int (*fnc)(int, const char *const*, void *);
	const char *const szHelp;
	int bHelp;
};

static int process_tok(const command_t *table,const char *tok);

unsigned long
StringToStatus(char *_szStatus)
{	ICQOwner *o = gUserManager.FetchOwner(LOCK_R);
	unsigned long nStatus = o->AddStatusFlags(0);
	int i =0;
	static struct 
	{	const char *const name;
		const unsigned long nStatus;
	} table[]=
	{	{ "online",	ICQ_STATUS_ONLINE	},
		{ "away",	ICQ_STATUS_AWAY	},
		{ "na",		ICQ_STATUS_NA	},
		{ "occupied",	ICQ_STATUS_OCCUPIED	},
		{ "dnd",	ICQ_STATUS_DND	},
		{ "ffc",	ICQ_STATUS_FREEFORCHAT	},
		{ "offline",	ICQ_STATUS_OFFLINE	},
		{ NULL,		0	}
	};
	gUserManager.DropOwner();
	if (_szStatus[0] == '*')
	{ 	_szStatus++;
		nStatus |= ICQ_STATUS_FxPRIVATE;
	}
	for( i=0; table[i].name && strcasecmp(table[i].name,_szStatus)  ; i++)
		;

	return table[i].name ? nStatus|table[i].nStatus : INT_MAX ;
}


/* atouin
 *	Given an ascii string get the uin
 *	1. If all chars are digits test
 *		a) if bList flag is on check if it is in the list
 *		b) else return 
 *	2. If that fail try with alias
 * Params.
 * 	buff	bufer to convert
 *	bOnList	fail if buff is an uin number and it is not in user list
 *	nUin	address where the uin is saved
 */
bool
atouin( const char *buff, bool bOnList, unsigned long *nUin)
{	const char *p;

	if (buff == NULL) 
		return false; 

	for( p=buff; isdigit(*p) ; p++ )
		;
	if( *p == '\0' && bOnList && gUserManager.IsOnList(atol(buff)) || 
	    *p == '\0' && !bOnList)
		*nUin = atol(buff);
	else
 	{ 	*nUin = 0; 
 		FOR_EACH_USER_START(LOCK_R)
 		{	if (strcasecmp(buff, pUser->GetAlias()) == 0)
 			{ 	*nUin = pUser->Uin();
 				FOR_EACH_USER_BREAK;
 			}
 		}
 		FOR_EACH_USER_END

	} 		

	return !(*nUin == 0);
}

// status 
static int
fifo_status( int argc, const char *const *argv, void *data)
{	ICQOwner *o;
	CICQDaemon *d= (CICQDaemon *) data;;
	const char *szStatus = argv[1];
	bool bOffline;
	unsigned long nStatus;
	
	if( argc == 1 )
	{	ReportMissingParams(argv[0]);
		return -1;
	}
	
	// Determine the status to go to
	o = gUserManager.FetchOwner(LOCK_R);
	bOffline = o->StatusOffline();
	gUserManager.DropOwner();
	nStatus = StringToStatus(const_cast<char *>(szStatus));

	if (nStatus == INT_MAX)
	{ 	gLog.Warn("%s%s %s: command with invalid status \"%s\".\n",
		          L_WARNxSTR,L_FIFOxSTR,argv[0],szStatus);
			return -1;
	}
	else if (nStatus == ICQ_STATUS_OFFLINE)
	{	 if (!bOffline) 
			d->icqLogoff();
	}
	else
	{ 	if (bOffline)
			d->icqLogon(nStatus);
		else
			d->icqSetStatus(nStatus);
	}
	// Now set the auto response
	if( argc > 2 )
	{
		ICQOwner *o = gUserManager.FetchOwner(LOCK_W);
		o->SetAutoResponse(argv[2]);
		gUserManager.DropOwner();
	}

	return 0;
}


// auto_response <auto response>
static int
fifo_auto_response( int argc, const char *const *argv, void *data)
{	ICQOwner *o; 

	if( argc == 1 )
	{	ReportMissingParams(argv[0]);
		return -1;
	}
	
 	o = gUserManager.FetchOwner(LOCK_W);
	o->SetAutoResponse(argv[1]);
	gUserManager.DropOwner();

	return 0;
}

// message <buddy> <message>
static int
fifo_message ( int argc, const char *const *argv, void *data)
{	CICQDaemon *d = (CICQDaemon *) data;;
	unsigned long nUin; 
	const char *szUin = argv[1];
	
	if( argc < 3 )
	{	ReportMissingParams(argv[0]);
		return -1;
	}

 	if( atouin(szUin,false,&nUin) )
 		d->icqSendMessage(nUin, argv[2], false, false);
	else
		ReportBadBuddy(argv[0],szUin);
		
 	return 0;
}

//--
// url <buddy> <url> [<description>]
static int
fifo_url ( int argc, const char *const *argv, void *data)
{	unsigned long nUin;
	const char *szDescr;
	CICQDaemon *d = (CICQDaemon *) data;

	if( argc < 3 )
	{	ReportMissingParams(argv[0]);
		return -1;
	}

	if( atouin (argv[1],false,&nUin) )
	{	szDescr = (argc > 3) ? argv[3] : "" ;
		d->icqSendUrl(nUin, argv[2], szDescr, false, false);
	}
	else
		ReportBadBuddy(argv[0],argv[1]);

	return 0;
}

// redirect <file>
static int
fifo_redirect ( int argc, const char *const *argv, void *data)
{
	if( argc == 1 )
	{	ReportMissingParams(argv[0]);
		return -1;
	} 

	// TODO: its safe to call strerror ?
	if ( !Redirect(argv[1]) )
	{	 gLog.Warn("%s %s: redirection to \"%s\" failed: %s.\n",
		           L_WARNxSTR,argv[0], argv[1],strerror(errno));
    	}
    	else
    		gLog.Info("%s %s: output redirected to \"%s\".\n", L_INITxSTR,
    		          argv[0],argv[1]);

	return 0;
}

// debuglvl <level>
static int
fifo_debuglvl ( int argc, const char *const *argv, void *data)
{	
	int nRet = 0; 

	if( (nRet = (argc == 1)) )
		ReportMissingParams(argv[0]);
	else
		gLog.ModifyService( S_STDERR, atoi(argv[1]));

	return -nRet;
}

// adduser <buddy>
static int
fifo_adduser ( int argc, const char *const *argv, void *data)
{	unsigned long nUin;
	CICQDaemon *d = (CICQDaemon *) data;

	if( argc  == 1 )
	{	ReportMissingParams(argv[0]);
		return -1;
	}
	
	if( atouin(argv[1],false,&nUin) )
		d->AddUserToList(nUin);
	else
		ReportBadBuddy(argv[0],argv[1]);

	return 0;
}

// userinfo <buddy>
static int
fifo_userinfo ( int argc, const char *const *argv, void *data)
{	 unsigned long nUin;
	ICQUser *u;
	CICQDaemon *d = (CICQDaemon *) data;
	
	if ( argc == 1 )
	{	ReportMissingParams(argv[0]);
		return -1;
	}

	if( !atouin(argv[1],true,&nUin) )
	{	ReportBadBuddy(argv[0],argv[1]);
		return -1;
	}

	u = gUserManager.FetchUser(nUin, LOCK_R);
	if (u == NULL)
		gLog.Warn("%s %s: user %ld not on contact list, not retrieving"
		          "info.\n", L_WARNxSTR,argv[0],nUin);
	else
	{	gUserManager.DropUser(u);
		d->icqRequestMetaInfo(nUin);
	}

	return 0;
}

// exit
static int
fifo_exit ( int argc, const char *const *argv, void *data)
{	CICQDaemon *d = (CICQDaemon *) data;

	d->Shutdown();

	return 0;
}

// ui_viewevent [<buddy>]
static int
fifo_ui_viewevent ( int argc, const char *const *argv, void *data)
{	CICQDaemon *d = (CICQDaemon *) data; 
	unsigned long nUin;

	if( argc ==1 )
		nUin = 0;
	else if( !atouin(argv[1],true,&nUin) )
	{	ReportBadBuddy(argv[0],argv[1]);
		return -1;
	}

	d->PluginUIViewEvent(nUin);
	
	return 0;
}

// ui_message <buddy>
static int
fifo_ui_message ( int argc, const char *const *argv, void *data)
{	CICQDaemon *d = (CICQDaemon *) data;
 	unsigned long nUin;
	int nRet=0;

	if ( argc == 1 )
	{	ReportMissingParams(argv[0]);
		nRet = -1;
	} 
	else if( atouin(argv[1],true,&nUin) )
		d->PluginUIMessage(nUin);
	else
	{	ReportBadBuddy(argv[0],argv[1]);
		return -1;
	}
		
	return nRet;
}

static int
fifo_help ( int argc, const char *const *argv, void *data)
{	struct command_t *table = (struct command_t *)data;
	int i,j;

	if( argc == 1 )
	{	gLog.Info("%sFifo commands:\n",L_FIFOxSTR);
		for( i=0; table[i].fnc ; i++ )
			gLog.Info("%s%s\n",L_BLANKxSTR,table[i].szName);
		 gLog.Info("%s: Type `help command'\n",L_FIFOxSTR);
	}
	else 
	{
		for( i = 0 ; i < argc ; i++ )
		{	j=process_tok(table, argv[i] );
			if( j >= 0 )
				gLog.Info("%s %s: help for `%s'\n%s\n",
				L_FIFOxSTR,argv[0],argv[i],table[j].szHelp);
			else
				gLog.Info("%s %s: unknown command `%s'\n",
				          L_FIFOxSTR,argv[0],argv[i]);

		}
			
	}

	return 0;
}


static struct command_t fifocmd_table[]=
{
	{"status",	fifo_status,	HELP_STATUS,	0},
	{"auto_response",fifo_auto_response,HELP_AUTO,	0},
	{"message",	fifo_message,	HELP_MSG,	0},
	{"url",		fifo_url,	HELP_URL,	0},
	{"redirect",	fifo_redirect,	HELP_REDIRECT,	0},
	{"debuglvl",	fifo_debuglvl,	HELP_DEBUGLVL,	0},
	{"adduser",	fifo_adduser,	HELP_ADDUSER,	0},
	{"userinfo",	fifo_userinfo,	HELP_USERINFO,	0},
	{"exit",	fifo_exit,	HELP_EXIT,	0},
	{"ui_viewevent",fifo_ui_viewevent, HELP_UIVIEWEVENT,0},
	{"ui_message",	fifo_ui_message,HELP_UIMESSAGE,	0},
	{"help",	fifo_help,	HELP_HELP,	1},
	{NULL,	NULL,	NULL,	0	}
};

//-----ProcessFifo--------------------------------------------------------------
static char 
getQuotedChar( char c )
{	char ret;

	switch( c )
	{	case 'n':	ret = '\n'; break;
		case 't':	ret = '\t'; break;
		case 'v':	ret = '\v'; break;
		case 'b':	ret = '\b'; break;
		case 'r':	ret = '\r'; break;
		case 'f':	ret = '\f'; break;
		case 'a':	ret = '\a'; break;
		default:	ret = c;    break;
	}
	return ret;
}

static bool
line2argv( char *p, char **argv, int *argc, int size )
{	char *q;
	bool bQuote;

	/* trim */
	for( ; isspace(*p) ; p ++ )
		;
	for( q=p+strlen(p)-1; isspace(*q) ; q-- );
	q[1] = '\0'; 
	
	/* split: the ugly thing  */
	bQuote=false,*argc=0,argv[(*argc)++]=p,size--;
	for( q = p ; *argc < size && *p ; p++)
	{	if(!bQuote && isspace(*p))
		{	for( *(q++)='\0'; isspace(*(p+1)) ; p++ )
				;
			if( *(p+1) )
				argv[(*argc)++] = q; 
		}
		else if( *p == '"' )
			bQuote = !bQuote;
		else if( bQuote && *p == '\\' )
			*(q++) = getQuotedChar( *(++p) );
		else
			*(q++) = *p;
	}
	*q = '\0';

	// TODO:  ADD argv[argv] = NULL?

	return !bQuote;
}

static int
process_tok(const command_t *table,const char *tok)
{	int i;
	bool bExit;
	
	/* empty line */
	if(tok[0]==0)
		return CL_NONE;

	for( i=0, bExit=false ; !bExit && table[i].szName != NULL ; i++ )
	{	if( ! strcasecmp(table[i].szName,tok) )
			bExit =  true;
	}

	return  bExit  ? i -1 : CL_UNKNOWN;
}

void CICQDaemon::ProcessFifo(char *_szBuf)
{
#ifdef USE_FIFO
	int argc,index;
	char * argv[MAX_ARGV];
	char *szBuf = strdup(_szBuf);

	if( szBuf == NULL )
		return ;

	gLog.Info("%sReceived string: `%s'\n",L_FIFOxSTR,szBuf); 
	line2argv(szBuf,argv,&argc,sizeof(argv)/sizeof(argv[0]) );
	index=process_tok(fifocmd_table,argv[0]);

	switch( index )
	{	case CL_UNKNOWN:
			gLog.Info("%s: `%s' Unknown fifo  command. try with help",
			          L_FIFOxSTR,argv[0]);
			break;
		case CL_NONE:
			break;
		default:
			argv[0] = (char *)fifocmd_table[index].szName;
	       		if( fifocmd_table[index].fnc )
			{	if(fifocmd_table[index].bHelp)
					fifocmd_table[index].fnc(argc,argv,(void *)fifocmd_table);
				else
					fifocmd_table[index].fnc(argc,argv,this);
			}
			break;
	}

	free( szBuf );
	
#endif //USE_FIFO
}

