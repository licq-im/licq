#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "licq.h"

#ifdef USE_SOCKS5
#define SOCKS
#define INCLUDE_PROTOTYPES
#include <socks.h>
#endif


// define a global variable for the base directory containing the data and config subdirectories
char BASE_DIR[MAX_FILENAME_LEN];
char SHARE_DIR[MAX_FILENAME_LEN];
char LIB_DIR[MAX_FILENAME_LEN];
unsigned short DEBUG_LEVEL;

int main(int argc, char **argv)
{

#ifdef USE_SOCKS5
   SOCKSinit(argv[0]);
#endif

  CLicq licq(argc, argv);
  if (licq.Exception() != EXIT_SUCCESS) return licq.Exception();
  return licq.Main();
}
