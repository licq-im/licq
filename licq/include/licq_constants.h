// ICQ definitions:
#ifndef CONSTANTS_H
#define CONSTANTS_H

// Directory constants
const char DEFAULT_HOME_DIR[]                      = ".licq";
const char USER_DIR[]                              = "users";
const char HISTORY_DIR[]                           = "history";
const char HISTORY_EXT[]                           = "history";
const char HISTORYxOLD_EXT[]                       = "history.removed";
const char TRANSLATION_DIR[]                       = "translations";
const char UTILITY_DIR[]                           = "utilities";
extern char BASE_DIR[];
extern char SHARE_DIR[];
extern char LIB_DIR[];
enum direction { D_SENDER, D_RECEIVER };
const char DEFAULT_SERVER_HOST[]                = "login.icq.com";

// Plugin constants
const char PLUGIN_SIGNAL                           = 'S';
const char PLUGIN_EVENT                            = 'E';
const char PLUGIN_SHUTDOWN                         = 'X';

// Miscellaneous constants
extern unsigned short DEBUG_LEVEL;
const unsigned short MAX_FILENAME_LEN              = 255;
const unsigned short MAX_HOSTNAME_LEN              = 255;
const unsigned short MAX_CMD_LEN                   = 1024;
const unsigned short MAX_DATA_LEN                  = 1024;
const unsigned short MAX_MESSAGE_SIZE              = 6800; // Maybe a little bigger?
const unsigned short MAX_OFFLINE_MESSAGE_SIZE      = 450;
const unsigned short PING_FREQUENCY                = 60;
const unsigned short UPDATE_FREQUENCY              = 60;
const unsigned short DEFAULT_SERVER_PORT           = 5190;
const unsigned short DEFAULT_SSL_PORT              = 443;
const unsigned short MAX_SERVER_RETRIES            = 6;
const unsigned short MAX_WAIT_ACK                  = 10;
const unsigned short MAX_WAIT_PLUGIN               = 10;
const unsigned short MIN_SEND_DELAY                = 2;
const unsigned short USER_HASH_SIZE                = 128;
const unsigned short SOCKET_HASH_SIZE              = 128;
const unsigned long  LOCALHOST                     = 0x0100007F;
const unsigned short LOGON_ATTEMPT_DELAY           = 300;
const unsigned short MAX_PING_TIMEOUTS             = 3;

#define PIPE_READ  0
#define PIPE_WRITE 1
#define TIME_NOW   0


#endif
