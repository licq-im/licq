#ifndef ICQCOLOR_H
#define ICQCOLOR_H


//---Color-------------------------------------------------------------------
class CICQColor
{
public:
  CICQColor();

  unsigned long ForeRed();
  unsigned long ForeGreen();
  unsigned long ForeBlue();
  unsigned long BackRed();
  unsigned long BackGreen();
  unsigned long BackBlue();

  unsigned long Foreground();
  unsigned long Background();

  void Set(unsigned long fore, unsigned long back);
  void Set(CICQColor *c);
  void SetBackground(unsigned long r, unsigned long g, unsigned long b);
  void SetForeground(unsigned long r, unsigned long g, unsigned long b);
  void SetToDefault();

  // Default colors for sending messages
  static unsigned long DefaultForeRed();
  static unsigned long DefaultForeGreen();
  static unsigned long DefaultForeBlue();
  static unsigned long DefaultBackRed();
  static unsigned long DefaultBackGreen();
  static unsigned long DefaultBackBlue();

  static unsigned long DefaultForeground();
  static unsigned long DefaultBackground();

  static void SetDefaultColors(CICQColor *c);
  static void SetDefaultForeground(unsigned long);
  static void SetDefaultBackground(unsigned long);

protected:
  unsigned long foreground;
  unsigned long background;

  static unsigned long default_foreground;
  static unsigned long default_background;

};


#endif
