#ifndef ICQCOLOR_H
#define ICQCOLOR_H


//---Color-------------------------------------------------------------------
class CICQColor
{
public:

  unsigned long ForeRed()  {  return foreground & 0xFF; }
  unsigned long ForeGreen() { return (foreground & 0xFF00) >> 8; }
  unsigned long ForeBlue()  { return ( foreground & 0xFF0000) >> 16; }
  unsigned long BackRed()  {  return background & 0xFF; }
  unsigned long BackGreen() { return (background & 0xFF00) >> 8; }
  unsigned long BackBlue()  { return ( background & 0xFF0000) >> 16; }
  unsigned long Foreground()  { return foreground; }
  unsigned long Background()  { return background; }


  CICQColor()
   {
     background = 0x00FFFFFF;
     foreground = 0x00000000;
   }

  void Set(unsigned long fore, unsigned long back)
   {
     foreground = fore;
     background = back;
   }

  void Set(CICQColor *c)
   {
     foreground = c->Foreground();
     background = c->Background();
   }

  void SetBackground(unsigned long r, unsigned long g, unsigned long b)
   {
     background = (r & 0xFF) | ((g & 0xFF) << 8) | ((b & 0xFF) << 16);
   }

  void SetForeground(unsigned long r, unsigned long g, unsigned long b)
   {
     foreground = (r & 0xFF) | ((g & 0xFF) << 8) | ((b & 0xFF) << 16);
   }

protected:
  unsigned long foreground;
  unsigned long background;

};


#endif
