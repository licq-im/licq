#ifndef ICQCOLOR_H
#define ICQCOLOR_H


//---Color-------------------------------------------------------------------
class CICQColor
{
public:

  unsigned long fore_red, fore_green, fore_blue;
  unsigned long back_red, back_green, back_blue;

  // Plugins should not need these functions
  CICQColor()
   {
     fore_red = fore_green = fore_blue = 0xFF;
     back_red = back_green = back_blue = 0x00;
   }

  CICQColor(unsigned long fore, unsigned long back) {}

  void Set(unsigned long fore, unsigned long back)
   {
     fore_red = fore & 0xFF;
     fore_green = (fore & 0xFF00) >> 8;
     fore_blue = (fore & 0xFF0000) >> 16;
     back_red = back & 0xFF;
     back_green = (back & 0xFF00) >> 8;
     back_blue = (back & 0xFF0000) >> 16;
   }

  unsigned long foreground()
   {
     return ((fore_red << 0) & 0xFF) + ((fore_green << 8) & 0xFF00) +
      ((fore_blue << 16) & 0xFF0000);
   }

  unsigned long background()
   {
     return ((back_red << 0) & 0xFF) + ((back_green << 8) & 0xFF00) +
      ((back_blue << 16) & 0xFF0000);
   }

};


#endif
