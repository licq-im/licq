// Skin Spec 0.1

#ifndef SKIN_H
#define SKIN_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qrect.h>
#include <qpixmap.h>

#include "file.h"

#define QTGUI_DIR "qt-gui/"

struct Border
{
   unsigned short top, bottom;
   unsigned short left, right;
};

class CFrameSkin
{
public:
   struct Border border;
   unsigned short frameStyle;
   bool maintainBorder;
   bool hasMenuBar;
   bool transparent;
   char *pixmap;
   char *mask;
};

struct Rect
{
   signed short x1, y1;
   signed short x2, y2;
};

struct Color
{
   char *fg, *bg;
};

struct CColors
{
  char *online, *offline, *away, *background, *gridlines, *newuser;
};

class CShapeSkin
{
public:
   struct Rect rect;
   struct Color color;
};

class CButtonSkin : public CShapeSkin
{
public:
   char *pixmapUpFocus, *pixmapUpNoFocus, *pixmapDown;
   char *caption;
};

class CLabelSkin : public CShapeSkin
{
public:
   char *pixmap;
   unsigned short frameStyle;
   bool transparent;
   unsigned short margin;
};

class CComboSkin : public CShapeSkin { };

class CListSkin : public CShapeSkin { };

class CSkin
{
public:
   CSkin(const char *);
   ~CSkin(void);
   CFrameSkin frame;
   CButtonSkin btnSys;
   CLabelSkin lblStatus, lblMsg;
   CComboSkin cmbGroups;
   CListSkin lstUsers;
   CColors colors;
   char *szSkinName;

   // Functions
   QRect borderToRect(CShapeSkin *, QWidget *);
   QRect borderToRect(CShapeSkin *, QPixmap *);
   int frameWidth(void);
   int frameHeight(void);

protected:
   void SetDefaultValues();
};


#endif
