#ifndef WINDOW_H
#define WINDOW_H

#include <iostream.h>
#include <curses.h>

#define COLOR_YELLOW_BLUE COLOR_YELLOW + 8
#define COLOR_WHITE_BLUE  COLOR_WHITE + 8
#define COLOR_CYAN_BLUE   COLOR_CYAN + 8

#define SCROLLBACK_OVERLAP 10

enum InputState { STATE_COMMAND, STATE_PENDING, STATE_MLE, STATE_LE, STATE_QUERY };

class CICQEventTag;
class CLicqConsole;
class CData;

class CWindow
{
public:
  CWindow(int _rows, int _cols, int _y, int _x, int _scrollback);
  ~CWindow();
  void RefreshWin();
  void ScrollUp();
  void ScrollDown();
  void SetActive(bool _active) { active = _active; RefreshWin(); }
  bool Active()  { return active; }
  CWindow& operator<<(char d);
  CWindow& operator<<(unsigned char d);
  CWindow& operator<<(const char *d);
  CWindow& operator<<(unsigned long d);
  CWindow& operator<<(unsigned short d);
  void wprintf(char *formatIn, ...);
  WINDOW *Win()  { return win; }
  static void StartScreen();
  static void EndScreen();
  int Rows() { return rows; }
  int Cols() { return cols; }

  void (CLicqConsole::*fProcessInput)(int);
  CICQEventTag *event;
  InputState state;
  CData *data;
  unsigned long nLastUin;
  unsigned short nLastHistory;
protected:
  WINDOW *win;
  bool pad, active;
  int rows, cols, x, y, height, cur_y;
};

#endif
