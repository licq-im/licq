#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>

#include "window.h"

void CWindow::StartScreen(void)
{
   initscr();             /* initialize the curses library */
   nonl();                /* tell curses not to do NL->CR/NL on output */
   cbreak();              /* take input chars one at a time, no wait for \n */
   noecho();              /* don't echo input */
   if (has_colors())
   {
      start_color();
      init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
      init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);
      init_pair(COLOR_CYAN, COLOR_CYAN, COLOR_BLACK);
      init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
      init_pair(COLOR_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
      init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
      init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
      init_pair(COLOR_BLACK, COLOR_BLACK, COLOR_BLACK);
      init_pair(COLOR_YELLOW_BLUE, COLOR_YELLOW, COLOR_BLUE);
      init_pair(COLOR_WHITE_BLUE, COLOR_WHITE, COLOR_BLUE);
      init_pair(COLOR_CYAN_BLUE, COLOR_CYAN, COLOR_BLUE);
   }
}

void CWindow::EndScreen(void)
{
   endwin();
}

CWindow::CWindow(int _rows, int _cols, int _y, int _x, bool _pad)
{
  rows = _rows;
  cols = _cols;
  x = _x;
  y = _y;
  pad = _pad;
  active = false;
  if (pad)
    win = newpad(rows, cols);
  else
    win = newwin(rows, cols, y, x);
  keypad(win, TRUE);
  wbkgd(win, COLOR_PAIR(COLOR_WHITE));
  wmove(win, 0, 0);
}

CWindow::~CWindow(void)
{
   delwin(win);
}

void CWindow::RefreshWin(void)
{
  if (!active) return;
  if (pad)
    pnoutrefresh(win, 0, 0, y, x, rows, cols);
  else
    wnoutrefresh(win);
  doupdate();
}

CWindow& CWindow::operator<<(char d)
{
  waddch(win, d);
  RefreshWin();
  return(*this);
}


CWindow& CWindow::operator<<(const char *d)
{
  waddstr(win, d);
  RefreshWin();
  return(*this);
}

CWindow& CWindow::operator<<(unsigned long d)
{
  char s[64];
  sprintf(s, "%lu", d);
  return(*this << s);
}

CWindow& CWindow::operator<<(unsigned short d)
{
  char s[64];
  sprintf(s, "%d", d);
  return(*this << s);
}

void CWindow::wprintf(char *formatIn, ...)
{
   va_list argp;
   char formatOut[1024], out[1024];
   unsigned short i = 0, j = 0;
   attr_t a;
#if NCURSES_VERSION_PATCH < 990213
   a = wattr_get(win);
#else
   short p;
   wattr_get(win, &a, &p, NULL);
#endif
   va_start(argp, formatIn);

   while((formatIn[i]) && (formatIn[i] != '%'))  formatOut[j++] = formatIn[i++];

   formatOut[j] = '\0';
   *this << formatOut;

   while(i < strlen(formatIn))
   {
      j = 0;
      switch(formatIn[++i])
      {
      case 'C':   // set color
         i++;
#if NCURSES_VERSION_PATCH < 990213
         (void) va_arg(argp, short);
//         wcolor_set(win, va_arg(argp, short), NULL);
#else
         wcolor_set(win, va_arg(argp, short), NULL);
#endif
         while((formatIn[i]) && (formatIn[i] != '%'))  formatOut[j++] = formatIn[i++];
         formatOut[j] = '\0';
         *this << formatOut;
         break;
      case 'A':   // more generally set attribute
         i++;
         wattron(win, va_arg(argp, long));
         while((formatIn[i]) && (formatIn[i] != '%'))  formatOut[j++] = formatIn[i++];
         formatOut[j] = '\0';
         *this << formatOut;
         break;
      case 'Z':   // more generally clear attribute
         i++;
         wattroff(win, va_arg(argp, long));
         while(((formatIn[i]) && formatIn[i] != '%'))  formatOut[j++] = formatIn[i++];
         formatOut[j] = '\0';
         *this << formatOut;
         break;
      case 'f': break; //can't cast a float right.
      default:
         formatOut[j++] = '%';
         while((formatIn[i]) && (formatIn[i] != '%'))  formatOut[j++] = formatIn[i++];
         formatOut[j] = '\0';
         sprintf(out, formatOut, va_arg(argp, long)); //use a double to make sure we get all of the arg (up to 64 bits).
         *this << out;
         break;
      }
   }
   va_end(argp);
#if NCURSES_VERSION_PATCH < 990213
   wattr_set(win, a);
#else
   wattr_set(win, a, p, NULL);
#endif
}


