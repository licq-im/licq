#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <unistd.h>
#include <string.h>

// this is a big hack to avoid undefined function link
// to __va_arg_type_violation(void)
// va_list sucks!
#if defined(__PPC__)
#undef __OPTIMIZE__
#endif
#include <stdarg.h>

#include "window.h"

void CWindow::StartScreen()
{
   initscr();             /* initialize the curses library */
   nonl();                /* tell curses not to do NL->CR/NL on output */
   cbreak();              /* take input chars one at a time, no wait for \n */
   noecho();              /* don't echo input */
   /*
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
   */
}

void CWindow::EndScreen()
{
   endwin();
}

CWindow::CWindow(int _rows, int _cols, int _y, int _x, int _scrollback, int _useCDK)
{
  rows = _rows;
  cols = _cols;
  x = _x;
  y = _y;
  height = rows + _scrollback;
  pad = _scrollback > 0;
  active = false;
  if (pad)
  {
    win = newpad(height, cols);
    wmove(win, height - rows, 0);
  }
  else
  {
    win = newwin(rows, cols, y, x);
    wmove(win, 0, 0);
  }

  keypad(win, TRUE);
  wbkgd(win, COLOR_PAIR(COLOR_WHITE));

  cdkscreen = NULL;
  if (_useCDK)
  { 
    initCDKColor();
    cdkscreen = initCDKScreen(win);
    if (cdkscreen == NULL)
    {
      printf("Couldn't create cdk screen!\n");
      exit(1);
    }
  }

  sLastContact = USERID_NONE;
  nLastHistory = 1;
}

CWindow::~CWindow()
{
   delwin(win);
}

void CWindow::RefreshWin()
{
  if (!active) return;
  if (pad)
  {
    cur_y = height - rows;
    pnoutrefresh(win, cur_y, 0, y, x, rows + y, cols + x);
  }
  else
    wnoutrefresh(win);

  if (cdkscreen)
  {
    refreshCDKScreen(cdkscreen);
  }
  
  doupdate();
}


void CWindow::ScrollUp()
{
  if (!active || !pad) return;
  cur_y -= rows - SCROLLBACK_OVERLAP;
  if (cur_y < 0) cur_y = 0;
  pnoutrefresh(win, cur_y, 0, y, x, rows + y - 1, cols + x);
  doupdate();
}

void CWindow::ScrollDown()
{
  if (!active || !pad) return;
  cur_y += rows - SCROLLBACK_OVERLAP;
  if (cur_y > height - rows) cur_y = height - rows;
  pnoutrefresh(win, cur_y, 0, y, x, rows + y - 1, cols + x);
  doupdate();
}

CWindow& CWindow::operator<<(char d)
{
  waddch(win, d);
  RefreshWin();
  return(*this);
}

CWindow& CWindow::operator<<(unsigned char d)
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

void CWindow::wprintf(const char *formatIn, ...)
{
   va_list argp;
   char formatOut[1024], out[1024];
   unsigned short i = 0, j = 0;
   attr_t save = win->_attrs;

   va_start(argp, formatIn);

   while((formatIn[i]) && (formatIn[i] != '%') && j < 1022)  formatOut[j++] = formatIn[i++];

   formatOut[j] = '\0';
   *this << formatOut;

   while(i < strlen(formatIn))
   {
      j = 0;
      switch(formatIn[++i])
      {
      case 'C':   // set color
         i++;
//#if NCURSES_VERSION_PATCH < 990213
         win->_attrs &= ~A_COLOR;
         win->_attrs |= COLOR_PAIR(va_arg(argp, int));
//#else
//         wcolor_set(win, va_arg(argp, short), NULL);
//#endif
         while((formatIn[i]) && (formatIn[i] != '%') && j < 1022)  formatOut[j++] = formatIn[i++];
         formatOut[j] = '\0';
         *this << formatOut;
         break;
      case 'A':   // more generally set attribute
         i++;
         wattron(win, va_arg(argp, long));
         while((formatIn[i]) && (formatIn[i] != '%') && j < 1022)  formatOut[j++] = formatIn[i++];
         formatOut[j] = '\0';
         *this << formatOut;
         break;
      case 'B':   // more generally set attribute
         i++;
         wattron(win, A_BOLD);
         while((formatIn[i]) && (formatIn[i] != '%') && j < 1022)  formatOut[j++] = formatIn[i++];
         formatOut[j] = '\0';
         *this << formatOut;
         break;
      case 'b':   // more generally set attribute
         i++;
         wattroff(win, A_BOLD);
         while((formatIn[i]) && (formatIn[i] != '%') && j < 1022)  formatOut[j++] = formatIn[i++];
         formatOut[j] = '\0';
         *this << formatOut;
         break;
      case 'Z':   // more generally clear attribute
         i++;
         wattroff(win, va_arg(argp, long));
         while(((formatIn[i]) && formatIn[i] != '%') && j < 1022)  formatOut[j++] = formatIn[i++];
         formatOut[j] = '\0';
         *this << formatOut;
         break;
      case 'f': break; //can't cast a float right.
      default:
         formatOut[j++] = '%';
         while((formatIn[i]) && (formatIn[i] != '%') && j < 1022)  formatOut[j++] = formatIn[i++];
         formatOut[j] = '\0';
         snprintf(out, 1024, formatOut, va_arg(argp, long)); //use a double to make sure we get all of the arg (up to 64 bits).
         *this << out;
         break;
      }
   }
   va_end(argp);

   win->_attrs = save;
}


