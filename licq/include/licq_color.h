#ifndef ICQCOLOR_H
#define ICQCOLOR_H


//---Color-------------------------------------------------------------------
class CICQColor
{
public:
  CICQColor();

  unsigned long ForeRed() const;
  unsigned long ForeGreen() const;
  unsigned long ForeBlue() const;
  unsigned long BackRed() const;
  unsigned long BackGreen() const;
  unsigned long BackBlue() const;

  unsigned long Foreground() const;
  unsigned long Background() const;

  void Set(unsigned long fore, unsigned long back);
  void Set(CICQColor const* c);
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

  static void SetDefaultColors(CICQColor const* c);
  static void SetDefaultForeground(unsigned long);
  static void SetDefaultBackground(unsigned long);

protected:
  unsigned long foreground;
  unsigned long background;

  static unsigned long default_foreground;
  static unsigned long default_background;

};


#endif
