#ifndef WINDOW_H
#define WINDOW_H

#include <iostream.h>
#include <curses.h>

//#define COLOR_HIGHLIGHT COLOR_MAGENTA

#define COLOR_YELLOW_BLUE COLOR_YELLOW + 8
#define COLOR_WHITE_BLUE  COLOR_WHITE + 8
#define COLOR_CYAN_BLUE   COLOR_CYAN + 8

enum InputState { STATE_COMMAND, STATE_PENDING, STATE_MLE, STATE_LE };

class ICQEvent;
class CLicqConsole;
class CData;

class CWindow
{
public:
  CWindow(int _rows, int _cols, int _y, int _x, bool _pad);
  ~CWindow(void);
  void RefreshWin(void);
  void SetActive(bool _active) { active = _active; RefreshWin(); }
  bool Active(void)  { return active; }
  CWindow& operator<<(char d);
  CWindow& operator<<(const char *d);
  CWindow& operator<<(unsigned long d);
  CWindow& operator<<(unsigned short d);
  void wprintf(char *formatIn, ...);
  WINDOW *Win(void)  { return win; }
  static void StartScreen(void);
  static void EndScreen(void);

  void (CLicqConsole::*fProcessInput)(int);
  ICQEvent *event;
  InputState state;
  CData *data;
protected:
  WINDOW *win;
  bool pad, active;
  int rows, cols, x, y;
};

#endif
