// ICQ definitions:
#ifndef CONSTANTS_H
#define CONSTANTS_H

// Directory constants
const char USER_DIR[]                              = "users";
const char HISTORY_DIR[]                           = "history";
const char HISTORY_EXT[]                           = "history";
const char HISTORYxOLD_EXT[]                       = "history.removed";
const char TRANSLATION_DIR[]                       = "translations";
const char UTILITY_DIR[]                           = "utilities";
extern char BASE_DIR[];
extern char SHARE_DIR[];
extern char LIB_DIR[];

// Plugin constants
const char PLUGIN_SIGNAL                           = 'S';
const char PLUGIN_EVENT                            = 'E';
const char PLUGIN_SHUTDOWN                         = 'X';

// Miscellaneous constants
const unsigned short MAX_FILENAME_LEN              = 1024;

#define PIPE_READ  0
#define PIPE_WRITE 1


#endif
