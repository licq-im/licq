// ICQ definitions:
#ifndef FEATURES_H
#define FEATURES_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


// Directory constants
const char DEFAULT_HOME_DIR[]                      = ".licq";
const char BASE_SHARE_DIR[]                        = "share/licq/";
const char BASE_LIB_DIR[]                          = "lib/licq/";
const char USER_DIR[]                              = "users/";
const char HISTORY_DIR[]                           = "history/";
const char TRANSLATION_DIR[]                       = "translations/";
const char UTILITY_DIR[]                           = "utilities/";
extern char BASE_DIR[];
extern char SHARE_DIR[];
extern char LIB_DIR[];
enum direction { D_SENDER, D_RECEIVER };

// Plugin constants
const char PLUGIN_SIGNAL                           = 'S';
const char PLUGIN_EVENT                            = 'E';
const char PLUGIN_SHUTDOWN                         = 'X';

// Miscellaneous constants
extern unsigned short DEBUG_LEVEL;
const unsigned short MAX_FILENAME_LEN              = 255;
const unsigned short MAX_CMD_LEN                   = 255;
const unsigned short MAX_DATA_LEN                  = 255;
const unsigned short MAX_MESSAGE_SIZE              = 450;
const unsigned short TIME_NOW                      = 0;
const unsigned short PING_FREQUENCY                = 120;
const unsigned short DEFAULT_SERVER_PORT           = 4000;
const unsigned short MAX_SERVER_RETRIES            = 3;
const unsigned short MAX_WAIT_ACK                  = 20;
const unsigned short MAX_WAIT_PLUGIN               = 10;
const unsigned short USER_HASH_SIZE                = 128;
const unsigned short SOCKET_HASH_SIZE              = 127;
const unsigned long  LOCALHOST                     = 0x0100007F;

// Thread constants
const unsigned short LOCK_N                        = 0;
const unsigned short LOCK_R                        = 1;
const unsigned short LOCK_W                        = 2;

#define PIPE_READ  0
#define PIPE_WRITE 1
#define EXIT_SUCCESS     0
#define EXIT_EXCEPTION   1
#define EXIT_THREADxFAIL 2
#define EXIT_STARTxSERVERxFAIL 3
#define EXIT_INSTALLxFAIL 4
#define EXIT_LOADxUSERSxFAIL 5


#endif
