#include <stdlib.h>
#include <Xlib.h>

/* Simple program to beep the X11 bell at the specified volumen (0 - 100)
   compile with:
   gcc beep.c -o beep -I/usr/include/X11 -L/usr/X11/lib -lX11
 */

int main(int argc, char **argv)
{
  unsigned short v = 50;
  Display *d = XOpenDisplay(getenv("DISPLAY"));
  if (d != NULL)
  {
    if (argc > 1) v = atoi(argv[1]);
    if (!v ) v = 100;
    XBell(d, v);
    XCloseDisplay(d);
  }
  else
  {
    printf("\a");
  }
  return 0;
}
