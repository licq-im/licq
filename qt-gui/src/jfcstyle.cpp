// (c) 2000 Keith Brown

// $Source$
// $Author$
// $Date$
// $Revision$

// This file may be distributed under the terms of the Q Public License
// as defined by Troll Tech AS of Norway and appearing in the file
// LICENSE.QPL included in the packaging of this file.


#include <limits.h>
#include <qapplication.h>
#include <qbutton.h>
#include <qcombobox.h>
#include <qlistbox.h>
#define INCLUDE_MENUITEM_DEF
#include <qmenudata.h>
#undef INCLUDE_MENUITEM_DEF
#include <qpainter.h>
#include <qpalette.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <qscrollbar.h>
#include <qtabbar.h>
#include <qwidget.h>

#include "jfcstyle.h"

/*!
  \class JFCScheme jfcstyle.h
  \brief Provides color scheme support for JFCStyle class.

  The JFC look and feel defines 8 colors in a color scheme: 3 primary, 3
  secondary and black and white. Use this class with the JFCStyle class to
  map custom color schemes that follow the look and feel guidelines.
  The default Metal scheme is provided as a static member "Default" along with
  a green "Jade" scheme and a gray scale "HiContrast".
*/

/*!
  The default purple and grey Metal scheme.
*/
JFCScheme JFCScheme::Default( QColor( 102, 102, 153 ),
			      QColor( 153, 153, 204 ),
			      QColor( 204, 204, 255 ),
			      QColor( 102, 102, 102 ),
			      QColor(153, 153, 153 ),
			      QColor( 204, 204, 204 ) );
/*!
  A green scheme.
*/
JFCScheme JFCScheme::Jade( QColor( 41, 101, 98 ), QColor( 90, 153, 156 ),
			   QColor( 148, 206, 205 ), QColor( 102, 102, 102 ),
			   QColor( 153, 153, 153 ), QColor( 204, 204, 204 ) );

JFCScheme JFCScheme::HiContrast( Qt::black, QColor( 204, 204, 204 ),
				 Qt::white, QColor( 102, 102, 102 ),
				 QColor( 204, 204, 204 ), Qt::white );

/*!
  Creates a color scheme with the default Metal colors.
*/
JFCScheme::JFCScheme()
{
  *this = Default;
}

/*!
  Creates a color scheme with the given primary and secondary colors.
*/
JFCScheme::JFCScheme( const QColor& primary1,
		      const QColor& primary2,
		      const QColor& primary3,
		      const QColor& secondary1,
		      const QColor& secondary2,
		      const QColor& secondary3 )
{
  setColors( primary1, primary2, primary3, secondary1, secondary2,
	     secondary3 );
}

/*!
  Creates a copy of the given scheme.
*/
JFCScheme::JFCScheme( const JFCScheme& scheme )
{
  *this = scheme;
}

/*!
  Assignment operator. Assigns colors of one scheme to another.
*/
const JFCScheme& JFCScheme::operator=( const JFCScheme& rhs )
{
  colors_[ Primary1 ] = rhs.colors_[ Primary1 ];
  colors_[ Primary2 ] = rhs.colors_[ Primary2 ];
  colors_[ Primary3 ] = rhs.colors_[ Primary3 ];
  colors_[ Secondary1 ] = rhs.colors_[ Secondary1 ];
  colors_[ Secondary2 ] = rhs.colors_[ Secondary2 ];
  colors_[ Secondary3 ] = rhs.colors_[ Secondary3 ];
  return *this;
}

/*!
  Returns the color assigned to the given role.
*/
QColor JFCScheme::getColor( SchemeColorRole role ) const
{
  return colors_[ role ];
}

/*!
  Assigns a color to a given role.
*/
void JFCScheme::setColor( SchemeColorRole role, const QColor& color )
{
  colors_[ role ] = color;
}

/*!
  Sets the colors of the scheme.
*/
void JFCScheme::setColors( const QColor& primary1,
			   const QColor& primary2,
			   const QColor& primary3,
			   const QColor& secondary1,
			   const QColor& secondary2,
			   const QColor& secondary3 )
{
  colors_[ Primary1 ] = primary1;
  colors_[ Primary2 ] = primary2;
  colors_[ Primary3 ] = primary3;
  colors_[ Secondary1 ] = secondary1;
  colors_[ Secondary2 ] = secondary2;
  colors_[ Secondary3 ] = secondary3;
}

/*!
  \class JFCStyle jfcstyle.h
  \brief Provides a GUI style based on Java Foundation Classes "Metal".

  A custom GUI style for the Qt toolkit that you can use with your application
  to make it look like a Java Swing application. It isn't perfect, but it's
  pretty close! The current implementation of styles for Qt will not allow a
  flawless imitation anyhow.
*/

JFCScheme JFCStyle::scheme_;

/*!
  Creates a JFCStyle with the default "purple" and gray Metal scheme.
*/

JFCStyle::JFCStyle()
{
}

/*!
  Creates a JFCStyle with the given color scheme.
*/
JFCStyle::JFCStyle( const JFCScheme& scheme )
{
  scheme_ = scheme;
}

/*!
  Sets up individual widgets as required.
*/
void JFCStyle::polish( QWidget* widget )
{
  QPalette temp = widget->palette();
  // this is a hack to get rid of the etched Windows style
  // in disabled label text -- much simpler than reimplementing
  // QStyle::drawItem :-)
  temp.setColor( QPalette::Disabled, QColorGroup::Light,
		 scheme_.getColor( JFCScheme::Secondary3 ) );
  widget->setPalette( temp );
  // JFC label text is drawn in the primary color -- default is purple
  if( widget->isA( "QLabel" ) )
    {
      temp.setColor( QPalette::Normal, QColorGroup::Text,
		     scheme_.getColor( JFCScheme::Primary1 ) );
      widget->setPalette( temp );
    }
  // These widgets use a normal font instead of bold
  else if( widget->isA( "QListBox" ) || widget->isA( "QListView" ) ||
	   widget->isA( "QLineEdit" ) || widget->isA( "QMultiLineEdit" ) )
    {
      QFont fn = widget->font();
      fn.setBold( FALSE );
      widget->setFont( fn );
    }
  // QWindowsstyle highlights the text in a combobutton when it has the
  // focus...we hide this by setting the highlight color to the button
  // color. In addition, JFC combo boxes use a shade of the secondary
  // color for the background of the listbox portion.
  else if( widget->isA( "QComboBox" ) )
    {
      temp.setColor( QColorGroup::Highlight,
		     scheme_.getColor( JFCScheme::Secondary3 ) );
      widget->setPalette( temp );
      QListBox* lbox = ( ( QComboBox* ) widget )->listBox();
      if( lbox )
	{
	  QPalette lboxpal = lbox->palette();
	  lboxpal.setColor( QColorGroup::Base,
			    scheme_.getColor( JFCScheme::Secondary3 ) );
	  lboxpal.setColor( QColorGroup::Highlight,
			    scheme_.getColor( JFCScheme::Primary2 ) );
	  lbox->setPalette( lboxpal );
	}
    }
  // QWindowsstyle menu bar tracks the mouse and raises the menu item when
  // the pointer passes over it...Java does not
  else if( widget->isA( "QMenuBar" ) )
    {
      widget->setMouseTracking( FALSE );
    }
  // JFC tickmarks are drawn in Primary 1
  else if( widget->isA( "QSlider" ) )
    {
      temp.setColor( QColorGroup::Foreground, 
		     scheme_.getColor( JFCScheme::Primary1 ) );
      widget->setPalette( temp );
    }
}

/*!
  Sets up the application as required. This pretty much mostly involves
  saving the current font and palette and mapping some of the colors to the
  application palette and setting the font bold. JFC look and feel guidelines
  specify a 12 point font, but I'd rather not mess with a concrete size...
*/
void JFCStyle::polish( QApplication* app )
{
  oldPalette_ = app->palette();
 QPalette palette = oldPalette_;
   palette.setColor( QColorGroup::Background,
		    scheme_.getColor( JFCScheme::Secondary3 ) );
   palette.setColor( QColorGroup::Base, Qt::white );
   palette.setColor( QColorGroup::ButtonText, Qt::black );
   palette.setColor( QPalette::Disabled, QColorGroup::ButtonText,
                    scheme_.getColor( JFCScheme::Secondary2 ) );
   palette.setColor( QColorGroup::Button,
		    scheme_.getColor( JFCScheme::Secondary3 ) );
   palette.setColor( QColorGroup::Dark,
		    scheme_.getColor( JFCScheme::Secondary1 ) );
   palette.setColor( QColorGroup::Highlight,
		    scheme_.getColor( JFCScheme::Primary3 ) );
   palette.setColor( QColorGroup::HighlightedText, Qt::black );
   palette.setColor( QPalette::Disabled, QColorGroup::HighlightedText,
		    scheme_.getColor( JFCScheme::Secondary2 ) );
   palette.setColor( QColorGroup::Light, Qt::white );
   palette.setColor( QColorGroup::Mid,
		    scheme_.getColor( JFCScheme::Secondary2 ) );
   palette.setColor( QPalette::Normal, QColorGroup::Text, Qt::black );
   palette.setColor( QPalette::Disabled, QColorGroup::Text,
		    scheme_.getColor( JFCScheme::Secondary2 ) );
   app->setPalette( palette, TRUE );
   oldFont_ = app->font();
   QFont newFont( oldFont_ );
   newFont.setBold( TRUE );
   app->setFont( newFont, TRUE );
}

/*!
  Restores the original application font and palette.
*/
void JFCStyle::unPolish( QApplication* app )
{
   app->setPalette( oldPalette_, TRUE );
   app->setFont( oldFont_, TRUE );
}

/*
void JFCStyle::polish( QPalette& palette )
{
}
*/

/*!
  Draws an etched separator line. ( Is this exclusive to popup menus? )
*/
void JFCStyle::drawSeparator( QPainter* p, int x1, int y1, int x2, int y2,
			     const QColorGroup&, bool, int, int )
{
   p->setPen( scheme_.getColor( JFCScheme::Primary2 ) );
   p->drawLine( x1 - 1, y1, x2 + 1, y2 );
   p->setPen( Qt::white );
   p->drawLine( x1 - 1, y1 + 1, x2 + 1, y2 + 1 );
}

/*!
  Draws a plain button. Simply passes arguments to drawPanel.
*/
void JFCStyle::drawButton( QPainter* p, int x, int y, int w, int h,
			  const QColorGroup& g, bool sunken,
			  const QBrush* fill )
{
   // caller expects button color to be used for background
   QColorGroup cg( g );
   cg.setColor( QColorGroup::Background, cg.button() );
   drawPanel( p, x, y, w, h, cg, sunken, 1, fill );
}

/*!
  Returns the portion of a button available for drawing contents.
*/
QRect JFCStyle::buttonRect( int x, int y, int w, int h )
{
   return QRect( x + 2, y + 2, w - 4, h - 4 );
}

/*!
  Draws a bevel button. Simply passes arguments to drawButton.
*/
void JFCStyle::drawBevelButton( QPainter* p, int x, int y, int w, int h,
			      const QColorGroup& g, bool sunken,
			      const QBrush* fill )
{
   // caller expects button color to be used for background
   QColorGroup cg( g );
   cg.setColor( QColorGroup::Background, cg.button() );
   drawButton( p, x, y, w, h, cg, sunken, fill );
}

/*!
  Draws a panel. Since all buttons and such in JFC are drawn in the same 3D
  etched style as a panel, a lot of other drawing routines just call this one.
*/
void JFCStyle::drawPanel( QPainter* p, int x, int y, int w, int h,
			  const QColorGroup& g, bool sunken, int,
			  const QBrush* )
{
  p->fillRect( x, y, w, h, g.background() );
  p->setPen( g.dark() );
  p->drawLine( x, y, x + w - 2, y );
  p->drawLine( x, y, x, y + h - 2 );
  p->drawLine( x + 2, y + h - 2, x + w - 2, y + h - 2 );
  p->drawLine( x + w - 2, y + h - 2, x + w - 2, y + 2 );
  p->setPen( g.light() );
  p->drawLine( x + 1, y + 1, x + w - 3, y + 1 );
  p->drawLine( x + 1, y + 1, x + 1, y + h - 3 );
  p->drawLine( x + 1, y + h - 1, x + w - 1, y + h - 1 );
  p->drawLine( x + w - 1, y + 1, x + w - 1, y + h - 1 );
  if( sunken )
    {
      p->fillRect( x, y, w - 1, h - 1, g.mid() );
      p->setPen( g.dark() );
      p->drawRect( x, y, w - 1, h - 1 );
    }
}

/*!
  Draws the panel for a popup menu. Has a thin border of the the medium
  primary color.
*/
void JFCStyle::drawPopupPanel( QPainter* p, int x, int y, int w, int h,
			       const QColorGroup&, int, const QBrush* )
{
  p->fillRect( x, y, w, h, scheme_.getColor( JFCScheme::Secondary3 ) );
  p->setPen( scheme_.getColor( JFCScheme::Primary2 ) );
  p->drawRect( x, y, w, h );
  p->setPen( Qt::white );
  p->drawLine( x + 1, y + 1, x + w - 2, y + 1 );
  //p->drawLine( x + 1, y + 1, x + 1, y + h - 2 );
}

/*!
  Returns the size of a radio button.
*/
QSize JFCStyle::exclusiveIndicatorSize() const
{
  return QSize(13, 13);
}

/*!
  Draws a radio button.
*/
void JFCStyle::drawExclusiveIndicator( QPainter* p, int x,
				       int y, int w, int h,
				       const QColorGroup&, bool on,
				       bool down, bool enabled )
{
  // could use ellipse and line drawing functions, but I don't know if the
  // same pixels will be colored on all platforms, so drawing point arrays
  // is probably safest to get the desired effect.
  //
  // the dark circular outline
  static QCOORD button[] =
  {
    4,0, 5,0, 6,0, 7,0,
    2,1, 3,1, 8,1, 9,1,
    1,2, 10,2,
    1,3, 10,3,
    0,4, 11,4,
    0,5, 11,5,
    0,6, 11,6,
    0,7, 11,7,
    1,8, 10,8,
    1,9, 10,9,
    2,10, 3,10, 8,10, 9,10,
    4,11, 5,11, 6,11, 7,11
  };
  // bright highlights
  static QCOORD hilite[] =
  {
    4,1, 5,1, 6,1, 7,1, 10,1,
    2,2, 3,2, 8,2, 9,2, 11,2,
    2,3, 11,3,
    1,4, 12,4,
    1,5, 12,5,
    1,6, 12,6,
    1,7, 12,7,
    2,8, 11,8,
    2,9, 11,9,
    10,10,
    2,11, 3,11, 8,11, 9,11,
    4,12, 5,12, 6,12, 7,12
  };
  // the dark fill when the control is pressed
  static QCOORD depression[] =
  {
    4,1, 5,1, 6,1, 7,1,
    2,2, 3,2, 4,2, 5,2, 6,2, 7,2, 8,2, 9,2,
    2,3, 3,3, 4,3, 5,3, 6,3, 7,3, 8,3, 9,3,
    1,4, 2,4, 3,4, 4,4, 5,4, 6,4, 7,4, 8,4, 9,4, 10,4,
    1,5, 2,5, 3,5, 4,5, 5,5, 6,5, 7,5, 8,5, 9,5, 10,5,
    1,6, 2,6, 3,6, 4,6, 5,6, 6,6, 7,6, 8,6, 9,6, 10,6,
    1,7, 2,7, 3,7, 4,7, 5,7, 6,7, 7,7, 8,7, 9,7, 10,7,
    2,8, 3,8, 4,8, 5,8, 6,8, 7,8, 8,8, 9,8,
    2,9, 3,9, 4,9, 5,9, 6,9, 7,9, 8,9, 9,9,
    4,10, 5,10, 6,10, 7,10
  };
  // the black "on" indicator dot in the middle
  static QCOORD dot[] =
  {
    4,3, 5,3, 6,3, 7,3,
    3,4, 4,4, 5,4, 6,4, 7,4, 8,4,
    3,5, 4,5, 5,5, 6,5, 7,5, 8,5,
    3,6, 4,6, 5,6, 6,6, 7,6, 8,6,
    3,7, 4,7, 5,7, 6,7, 7,7, 8,7,
    4,8, 5,8, 6,8, 7,8
  };
  QColor bg( scheme_.getColor( JFCScheme::Secondary3 ) );
  QColor text( Qt::black );
  QColor grayed( scheme_.getColor( JFCScheme::Secondary2 ) );
  QColor highlight( Qt::white );
  // clear the control background
  p->fillRect( x, y, w, h, bg );
  // draw dark gray etched circular outline
  QPointArray a( sizeof( button ) / ( sizeof( QCOORD ) * 2 ), button );
  a.translate( x, y );
  if( enabled )
    p->setPen( text );
  else
    p->setPen( grayed );
  p->drawPoints( a );
  // draw highlights
  if( enabled )
    {
      a.setPoints( sizeof( hilite ) / ( sizeof( QCOORD ) * 2 ), hilite );
      a.translate( x, y );
      p->setPen( highlight );
      p->drawPoints( a );
    }
  if( down ) // draw dark gray depression
    {
      a.setPoints( sizeof( depression ) / ( sizeof( QCOORD ) * 2 ),
		   depression );
      a.translate( x, y );
      p->setPen( grayed );
      p->drawPoints( a );
    }
  if( on ) // draw black indicator dot
    {
      a.setPoints( sizeof( dot ) / ( sizeof( QCOORD ) * 2 ), dot );
      a.translate( x, y );
      if( enabled )
	p->setPen( text );
      else
	p->setPen( grayed );
      p->drawPoints( a );
    }
}

/*!
  Returns the size of a check box.
*/
QSize JFCStyle::indicatorSize() const
{
  return QSize( 13, 13 );
}

/*!
  Draws a check box. The triple state check box is not part of JFC, so I
  basically wing it here...
*/
void JFCStyle::drawIndicator( QPainter* p, int x, int y, int w, int h,
			      const QColorGroup&, int state, bool down,
			      bool enabled )
{
  QColorGroup g( Qt::black, scheme_.getColor( JFCScheme::Secondary3 ),
		 Qt::white, scheme_.getColor( JFCScheme::Secondary1 ),
		 scheme_.getColor( JFCScheme::Secondary2 ),
		 Qt::black, Qt::white );
  // clear the control background
  p->fillRect( x, y, w, h, g.background() );
  if( enabled )
    drawButton( p, x, y, w, h, g, down );
  else
    {
      p->setPen( g.mid() );
      p->drawRect( x, y, w, h );
    }
  // gray depressed third state ( kinda like me on Monday )
  if( state == QButton::NoChange )
    drawButton( p, x, y, w, h, g, TRUE );
  if( state != QButton::Off )
    // draw check mark
    drawCheckMark( p, x + 3, y + 3, 0, 0, g, FALSE, !enabled );
}

/*!
  Draws a focus rect on a control. In JFC this is a plain rectangle in the
  primary color. Unfortunately, this is not called by QTab.
*/
void JFCStyle::drawFocusRect( QPainter* p, const QRect& r,
			      const QColorGroup&, const QColor*, bool )
{
  p->setPen( scheme_.getColor( JFCScheme::Primary1 ) );
  p->drawRect( r );
}

/*!
  Draws the button portion of a combo button.
*/
void JFCStyle::drawComboButton( QPainter* p, int x, int y, int w, int h,
				const QColorGroup&, bool sunken,
				bool editable, bool /* enabled */,
				const QBrush* )
{
  QColorGroup g( Qt::black, scheme_.getColor( JFCScheme::Secondary3 ),
		 Qt::white, scheme_.getColor( JFCScheme::Secondary1 ),
		 scheme_.getColor( JFCScheme::Secondary2 ),
		 Qt::black, Qt::white );
  drawPanel( p, x, y, w, h, g, editable ? FALSE : sunken );
  if( editable )
    drawPanel( p, x + w - 22, y, 21, h, g, sunken );
  // more control over the shape if we just draw the arrow here ourselves
  static QCOORD arrow[] =
  {
    0,0, 1,0, 2,0, 3,0, 4,0, 5,0, 6,0, 7,0, 8,0, 9,0,
    1,1, 2,1, 3,1, 4,1, 5,1, 6,1, 7,1, 8,1,
    2,2, 3,2, 4,2, 5,2, 6,2, 7,2,
    3,3, 4,3, 5,3, 6,3,
    4,4, 5,4
  };

  QPointArray a( sizeof( arrow ) / ( sizeof( QCOORD ) * 2 ), arrow );
  a.translate( x + w - 17, y + 8  );
  p->setPen( g.text() );
  p->drawPoints( a );
}

/*!
  Returns the portion of a combobutton available for drawing contents.
*/
QRect JFCStyle::comboButtonRect( int x, int y, int w, int h )
{
  return QRect( x + 2, y + 2, w - 4 - 21, h - 4 );
}

/*!
  Returns the focus rect for a combobutton. Unfortunately, this doesn't take
  a pointer to the combo so we dont know if it is editable or not.
*/
QRect JFCStyle::comboButtonFocusRect( int x, int y, int w, int h )
{
  return QRect( x + 3, y + 3, w - 6, h - 6 );
}

/*!
  Draws a push button according to whether it is the default button or not.
*/
void JFCStyle::drawPushButton( QPushButton* b, QPainter* p)
{
  int x, y, w, h;
  QColorGroup g( Qt::black, scheme_.getColor( JFCScheme::Secondary3 ),
		 Qt::white, scheme_.getColor( JFCScheme::Secondary1 ),
		 scheme_.getColor( JFCScheme::Secondary2 ),
		 Qt::black, Qt::white );
  b->rect().rect( &x, &y, &w, &h );
  // default button in JFC has an extra, dark gray border around it
  if( b->isDefault() )
    {
      drawPanel( p, x, y, w, h, g, FALSE );
      drawButton( p, x + 1, y + 1, w - 2, h - 2, g, b->isDown() || b->isOn() );
      p->setPen( g.dark() );
      p->drawRect( x + 1, y + 1, w - 2, h - 2 );
    }
  else
    drawButton( p, x, y, w, h, g, b->isDown() || b->isOn() );
}

/*!
  Returns the amount the button contents shift when pressed for 3D effects.
  JFC buttons do not shift their contents.
*/
void JFCStyle::getButtonShift( int& x, int& y )
{
  x = y = 0;
}

/*!
  Returns the default frame width. Used for...???
*/
int JFCStyle::defaultFrameWidth() const
{
  return 1;
}

/*!
  Returns vertical and horizontal spacing and overlap for drawing tabs. There
  is a bug with Qt <= 2.1 so this does not get called.
*/
void JFCStyle::tabbarMetrics( const QTabBar*, int& hframe, int& vframe,
			      int& overlap )
{
  hframe = 16;
  vframe = 6;
  overlap = 1;
}

/*!
  Draws a single tab. Tabs are screwed in Qt styles. There is no way to change
  the way labels or keyboard focus are drawn without subclassing the tab bar
  widget.
*/
void JFCStyle::drawTab( QPainter* p, const QTabBar* tb, QTab* t,
			bool selected )
{
  int x1 = t->r.left();
  int y1 = t->r.top();
  int x2 = t->r.right();
  int y2 = t->r.bottom();
  int w = t->r.width();
  int h = t->r.height();
  QPen basePen, hilightPen,
    outlinePen( scheme_.getColor( JFCScheme::Secondary1 ) );
  QBrush baseBrush;

  if( selected ) // current tab is drawn in normal background color
    {
      basePen.setColor( scheme_.getColor( JFCScheme::Secondary3 ) );
      hilightPen.setColor( Qt::white );
      baseBrush = QBrush( scheme_.getColor( JFCScheme::Secondary3 ) );
    }
  else // inactive tabs are drawn with a dark background
    {
      basePen.setColor( scheme_.getColor( JFCScheme::Secondary2 ) );
      hilightPen.setColor( scheme_.getColor( JFCScheme::Secondary3 ) );
      baseBrush = QBrush( scheme_.getColor( JFCScheme::Secondary2 ) );
    }
  bool onTop = ( tb->shape() == QTabBar::RoundedAbove || 
		 tb->shape() == QTabBar::TriangularAbove );

  QPointArray ptsHilight, ptsOutline, ptsDogear;
  QRect tabFillR;
  // calculate the points we'll use to draw the highlight, outline and
  // background fill depending on whether tabs are on top or bottom
  // (thank goodness we don't have to worry about sideways yet :-) )
  if( onTop )
    {
      ptsHilight.setPoints( 4, x1+1,y2, x1+1,y1+6, x1+6,y1+1, x2-1,y1+1 );
      if( selected )
	ptsOutline.setPoints( 5, x1,y2, x1,y1+6, x1+6,y1, x2,y1, x2,y2 );
      else
	ptsOutline.setPoints( 6, x1,y2, x1,y1+6, x1+6,y1, x2,y1, x2,y2, 
			      x1,y2 );
      ptsDogear.setPoints( 8, x1+2,y2, x1+2,y1+6, x1+3,y2, x1+3,y1+5,
			   x1+4,y2, x1+4,y1+4, x1+5,y2, x1+5,y1+3 );
      tabFillR.setRect( x1+6, y1+2, w-7, h-2 );
    }
  else
    {
      ptsHilight.setPoints( 3, x1+1,y1, x1+1,y2-6, x1+6,y2-1 );
      if( selected )
	ptsOutline.setPoints( 5, x1,y1, x1,y2-6, x1+6,y2, x2,y2, x2,y1 );
      else
	ptsOutline.setPoints( 6, x1,y1, x1,y2-6, x1+6,y2, x2,y2, x2,y1,
			      x1,y1 );
      ptsDogear.setPoints( 10, x1+2,y1, x1+2,y2-6, x1+3,y1+1, x1+3,y2-5,
			   x1+4,y1+1, x1+4,y2-4, x1+5,y1+1,x1+5,y2-3,
			   x1+6,y1+1, x1+6,y2-2 );
      tabFillR.setRect( x1+7, y1+1, w-8, h-2 );
    }
  // draw the highlights
  p->setPen( hilightPen );
  p->drawPolyline( ptsHilight );
  // draw the background with dog-eared tab
  p->setPen( basePen );
  p->drawLineSegments( ptsDogear );
  p->fillRect( tabFillR, baseBrush );
  // draw the outline of the tab
  p->setPen( outlinePen );
  p->drawPolyline( ptsOutline );
}

/*!
  Returns some size and position parameters for drawing scrollbar components.
*/
void JFCStyle::scrollBarMetrics( const QScrollBar* sb, int& sliderMin,
				 int& sliderMax, int& sliderLength,
				 int& buttonDim )
{
  const int minSliderLength = 15;
  bool horizontal = ( sb->orientation() == QScrollBar::Horizontal );
  int length = horizontal ? sb->width() : sb->height();
  // scroll-by-line buttons are square and the same size across as the
  // narrow dimension of the scrollbar
  buttonDim = horizontal ? sb->height() : sb->width();
  // the minimum position of the top or left side of the slider
  sliderMin = buttonDim;
  // the longest the slider can be...the extent of the trough between
  // the scroll-by-line buttons
  int maxLength = length - ( buttonDim * 2 );

  // if the range is 0, the slider is maxed out
  if( sb->maxValue() == sb->minValue() )
    sliderLength = maxLength;
  // else figure out how long the slider should be (proporionally)
  else
    {
      uint range = sb->maxValue() - sb->minValue();
      sliderLength = ( sb->pageStep() * maxLength ) /
	( range + sb->pageStep() );
      // make it an odd number (for drawing the grip )
      sliderLength -= !(sliderLength % 2 );
      if( sliderLength < minSliderLength || range > INT_MAX / 2 )
	sliderLength = minSliderLength;
      if( sliderLength > maxLength )
	sliderLength = maxLength;
    }
  // the maximum position of the top or left side of the slider
  sliderMax = sliderMin + maxLength - sliderLength;
}

/*!
  Draws the scrollbar controls that need painting.
*/
void JFCStyle::drawScrollBarControls( QPainter* p, const QScrollBar* sb,
				      int sliderStart, uint controls,
				      uint activeControl )
{
  QColorGroup g( Qt::black, scheme_.getColor( JFCScheme::Secondary3 ),
		 Qt::white, scheme_.getColor( JFCScheme::Secondary1 ),
		 scheme_.getColor( JFCScheme::Secondary2 ),
		 Qt::black, Qt::white );

  int sliderMin, sliderMax, sliderLength, buttonDim;
  // get some important values
  scrollBarMetrics( sb, sliderMin, sliderMax, sliderLength, buttonDim );

  // sanity check taken from Qt code -- is this really necessary??? oh well
  if( sliderStart > sliderMax )
    sliderStart = sliderMax;

  // calculate the layout of the individual controls
  QRect addB, subB, addPageR, subPageR, sliderR;
  int addX, addY, subX, subY;
  bool horizontal = ( sb->orientation() == QScrollBar::Horizontal );
  int length = horizontal ? sb->width() : sb->height();
  int extent = horizontal ? sb->height() : sb->width();

  if( horizontal )
    {
      subY = addY = ( extent - buttonDim ) / 2;
      subX = 0;
      addX = length - buttonDim;
    }
  else
    {
      subX = addX = ( extent - buttonDim ) / 2;
      subY = 0;
      addY = length - buttonDim;
    }

  subB.setRect( subX, subY, buttonDim, buttonDim );
  addB.setRect( addX, addY, buttonDim, buttonDim );

  int sliderEnd = sliderStart + sliderLength;
  int sliderWidth = extent;
  if( horizontal )
    {
      subPageR.setRect( subB.right() + 1, 0, sliderStart - subB.right() - 1,
			sliderWidth );
      addPageR.setRect( sliderEnd, 0, addX - sliderEnd , sliderWidth );
      sliderR.setRect( sliderStart, 0, sliderLength, sliderWidth );
    }
  else
    {
      subPageR.setRect( 0, subB.bottom() + 1, sliderWidth,
			sliderStart - subB.bottom() - 1 );
      addPageR.setRect( 0, sliderEnd, sliderWidth, addY - sliderEnd );
      sliderR.setRect( 0, sliderStart, sliderWidth, sliderLength );
    }
  // draw the line down (or right) button
  if( controls & AddLine )
    drawScrollLineButton( p, addB, g, horizontal ? Qt::RightArrow :
			  Qt::DownArrow, activeControl == AddLine );
  // draw the line up (or left) button
  if( controls & SubLine )
    drawScrollLineButton( p, subB, g, horizontal ? Qt::LeftArrow :
			  Qt::UpArrow, activeControl == SubLine );
  // draw the top or left portion of the slider trough
  if( controls & SubPage )
    {
      p->fillRect( subPageR, g.background() );
      p->setPen( g.dark() );
      if( horizontal && sliderR.left() > subPageR.left() )
	{
	  p->drawLine( subPageR.left(), subPageR.top(), subPageR.right(),
		       subPageR.top() );
	  p->setPen( g.mid() );
	  p->drawLine( subPageR.left(), subPageR.top() + 1, subPageR.left(),
		       subPageR.bottom() );
	  if( sliderR.left() > subPageR.left() + 1 )
	    p->drawLine( subPageR.left(), subPageR.top() + 1,
			 subPageR.right(), subPageR.top() + 1 );
	}
      else if( !horizontal && sliderR.top() > subPageR.top() )
	{
	  p->drawLine( subPageR.left(), subPageR.top(), subPageR.left(),
		       subPageR.bottom() );
	  p->setPen( g.mid() );
	  p->drawLine( subPageR.left() + 1, subPageR.top() + 1,
		       subPageR.left() + 1, subPageR.bottom() );
	  if( sliderR.top() > subPageR.top() + 1 )
	    p->drawLine( subPageR.left() + 1, subPageR.top(),
			 subPageR.right(), subPageR.top() );
	}
    }
  // draw the bottom or right portion of the slider trough
  if( controls & AddPage )
    {
      p->fillRect( addPageR, g.background() );
      p->setPen( g.dark() );
      if( horizontal && addPageR.left() < addX )
	{
	  p->drawLine( addPageR.left(), addPageR.top(), addPageR.right(),
		       addPageR.top() );
	  p->setPen( g.mid() );
	  p->drawLine( addPageR.left() + 1, addPageR.top() + 1,
		       addPageR.right(), addPageR.top() + 1 );
	  if( addPageR.left() < addX - 1 )
	    p->drawLine( addPageR.left(), addPageR.top() + 1,
			 addPageR.left(), addPageR.bottom() );
	}
      else if( !horizontal && addPageR.top() < addY )
	{
	  p->drawLine( addPageR.left(), addPageR.top(), addPageR.left(),
		       addPageR.bottom() );
	  p->setPen( g.mid() );
	  p->drawLine( addPageR.left() + 1, addPageR.top() + 1,
		       addPageR.left() + 1, addPageR.bottom() );
	  if( addPageR.top() < addY - 1 )
	    p->drawLine( addPageR.left() + 1, addPageR.top(),
			 addPageR.right(), addPageR.top() );
	}
    }
  // draw the slider
  if( controls & Slider )
    {
      g.setColor( QColorGroup::Button,
		  scheme_.getColor( JFCScheme::Primary2 ) );
      g.setColor( QColorGroup::Light,
		  scheme_.getColor( JFCScheme::Primary3 ) );
      g.setColor( QColorGroup::Dark,
		  scheme_.getColor( JFCScheme::Primary1 ) );
      drawScrollSlider( p, sliderR, g, horizontal );
    }
}

/*!
  Returns the width of a slider button.
*/
int JFCStyle::sliderLength() const
{
  return 15;
}

/*!
  Draws the slider "thumb".
*/
void JFCStyle::drawSlider( QPainter* p, int x, int y, int w, int h,
			   const QColorGroup&, Orientation orient,
			   bool, bool )
{
  QColor bg( scheme_.getColor( JFCScheme::Secondary3 ) );
  // erase the control area
  p->fillRect( x, y, w, h, bg );
  if( orient == Horizontal )
    {
      // draw the outline
      p->setPen( Qt::black );
      p->drawLine( x + 1, y, x + 13, y );
      p->drawLine( x, y + 1, x, y + 8 );
      p->drawLine( x, y + 8, x + 7, y + 15 );
      p->drawLine( x + 7, y + 15, x + 14, y + 8 );
      p->drawLine( x + 14, y + 8, x + 14, y + 1 );
      // draw highlights
      p->setPen( Qt::white );
      p->drawLine( x + 1, y + 1, x + 13, y + 1 );
      p->drawLine( x + 1, y + 1, x + 1, y + 8 );
      // fills in the portion of the groove that was erased
      p->drawLine( x, y + 10, x + 1, y + 10 );
      p->drawLine( x + 13, y + 10, x + 14, y + 10 );
      p->setPen( scheme_.getColor( JFCScheme::Secondary1 ) );
      p->drawPoint( x, y + 9 );
      p->drawPoint( x + 14, y + 9 );
      // draw the "thumb" grip
      drawGrip( p, QRect( x + 2, y + 2, 10, 6 ), bg, Qt::white,
		Qt::black );
    }
  else
    {
      // draw the outline
      p->setPen( Qt::black );
      p->drawLine( x + 1, y, x + 8, y );
      p->drawLine( x + 8, y, x + 15, y + 7 );
      p->drawLine( x + 15, y + 7, x + 8, y + 14 );
      p->drawLine( x + 8, y + 14, x + 1, y + 14 );
      p->drawLine( x, y + 13, x, y + 1 );
      // draw highlights
      p->setPen( Qt::white );
      p->drawLine( x + 1, y + 1, x + 1, y + 13 );
      p->drawLine( x + 1, y + 1, x + 8, y + 1 );
      // fills in the portion of the groove that was erased
      p->drawLine( x + 10, y, x + 10, y + 1 );
      p->drawLine( x + 10, y + 13, x + 10, y + 14 );
      p->setPen( scheme_.getColor( JFCScheme::Secondary1 ) );
      p->drawPoint( x + 9, y );
      p->drawPoint( x + 9, y + 14 );
      // draws the "thumb" grip
      drawGrip( p, QRect( x + 2, y + 2, 6, 10 ), bg, Qt::white,
		Qt::black );
    }
}

/*!
  Draws a slider groove.
*/
void JFCStyle::drawSliderGroove( QPainter* p, int x, int y, int w, int h,
				 const QColorGroup&, QCOORD c,
				 Orientation orient )
{
  // erase the background
  p->fillRect( x, y, w, h, scheme_.getColor( JFCScheme::Secondary3 ) );
  // adjust the rect to a narrow groove around the center axis
  if( orient == Horizontal )
    {
      y = y + c - 4;
      h = 7;
    }
  else
    {
      x = x + c - 4;
      w = 7;
    }
  // draw the highlight
  p->setPen( Qt::white );
  p->drawRect( x + 1, y + 1, w - 1, h - 1 );
  // draw the outline
  p->setPen( scheme_.getColor( JFCScheme::Secondary1 ) );
  p->drawRect( x, y, w -1, h - 1 );
  // draw the medium shadow on the inside
  p->setPen( scheme_.getColor( JFCScheme::Secondary2 ) );
  p->drawLine( x + 1, y + 1, x + w - 2, y + 1 );
  p->drawLine( x + 1, y + 1, x + 1, y + h - 2 );
}

/*!
  Override the default Windows behaviour. Prevents the slider from jumping back
  when the mouse is moved off the button.
*/
int JFCStyle::maximumSliderDragDistance() const
{
  return -1;
}

/*!
  Returns the width ( or height ) of a splitter widget.
*/
int JFCStyle::splitterWidth() const
{
  return 11;
}

/*!
  Draws a splitter with the drag grip.
*/
void JFCStyle::drawSplitter( QPainter* p, int x, int y, int w, int h,
			     const QColorGroup&, Orientation orient )
{
  p->setPen( Qt::white );
  if( orient == Horizontal )
    p->drawLine( x, y, x + w - 1, y );
  else
    p->drawLine( x, y, x, y + h - 1 );
  drawGrip( p, QRect( x + 3, y + 3, w - 6, h - 6 ),
	    scheme_.getColor( JFCScheme::Secondary3 ), Qt::white, Qt::black );
}

/*!
  Prepares a popup menu. Simply sets mouse tracking to true so we can draw
  the JFC style colored and indented active menu items.
*/
void JFCStyle::polishPopupMenu( QPopupMenu* p )
{
  p->setMouseTracking( TRUE );
}

// some global constants for drawing menu items
const int menuHMargin = 4;
const int menuVMargin = 3;
//const int checkMargin = 5;
const int checkWidth = 10;
const int checkHeight = 10;
const int tabSpacing = 10;

/*!
  Calculates the spacing to add for extra decorations like checkmarks and
  submenu indicators.
*/
int JFCStyle::extraPopupMenuItemWidth( bool checkable, int maxpmw,
				       QMenuItem* mi,
				       const QFontMetrics& )
{
  int w = 2 * menuHMargin; // start with the horizontal border around the item

  if( mi->isSeparator() )
    return w; // really a don't care
  else if( mi->pixmap() )
    // add the width of the pixmap if pixmap only
    w += mi->pixmap()->width();
  if( !mi->text().isNull() )
    {
      // add minimum width of space between text and accelerator text
      if( mi->text().find('\t') >= 0 )
	w += tabSpacing;
    }
  // if we have an iconset, add the maximum icon width
  if( maxpmw )
    w += maxpmw;
  // adjust if a check mark is wider than the iconset
  if( checkable && maxpmw < checkWidth )
    w += ( checkWidth - maxpmw );
  // add space between icon or checkmark and menu item
  if( maxpmw > 0 || checkable )
    w += menuHMargin;
  return w;
}

/*!
  Returns the width of the little arrow indicating a submenu item.
*/
int JFCStyle::popupSubmenuIndicatorWidth( const QFontMetrics& )
{
  return 4;
}

/*!
  Returns the height of a popup menu item.
*/
int JFCStyle::popupMenuItemHeight( bool, QMenuItem* mi,
				   const QFontMetrics& fm )
{
  if( mi->isSeparator() ) // just enough room to draw it
    return 2;
  return fm.height() + 2 * menuVMargin; // vertical space around text
}

/*!
  Draws a popup menu item.
*/
void JFCStyle::drawPopupMenuItem( QPainter* p, bool checkable, int maxpmw,
				  int tab, QMenuItem* mi,
				  const QPalette&, bool act,
				  bool enabled, int x, int y, int w,
				  int h )
{
  bool dis = !enabled;
  if( checkable )
    maxpmw = QMAX( maxpmw, checkWidth );
  int checkColWidth = maxpmw;
  if( mi->isSeparator() )
    {
      drawSeparator( p, x, y, x + w - 1, y, QColorGroup() );
      return;
    }
  if( act )
    {
      p->setPen( scheme_.getColor( JFCScheme::Secondary1 ) );
      p->drawLine( x, y + 1, x + w - 1, y + 1 );
      p->fillRect( x, y + 2, w, h - 4,
		   scheme_.getColor( JFCScheme::Primary2 ) );
      p->setPen( Qt::white );
      p->drawLine( x, y + h - 2, x + w - 1, y + h - 2 );
    }
  else
    {
      p->fillRect( x, y + 1, w, h - 1,
		   scheme_.getColor( JFCScheme::Secondary3 ) );
      p->setPen( Qt::white );
      p->drawLine( x, y, x, y + h - 1 );
    }
  if( mi->iconSet() )
    {
      QIconSet::Mode mode = dis? QIconSet::Disabled : QIconSet::Normal;
      if( act && !dis )
	mode = QIconSet::Active;
      QPixmap pixmap = mi->iconSet()->pixmap( QIconSet::Small, mode );
      int pixw = pixmap.width();
      int pixh = pixmap.height();
      QRect cr( x + menuHMargin, y, checkColWidth, h );
      QRect pmr( 0, 0, pixw, pixh );
      pmr.moveCenter( cr.center() /* nudge up a little */ -= QPoint( 0, 1 ));
      p->drawPixmap( pmr.topLeft(), pixmap );
    }
  else if( checkable )
    {
      if( mi->isChecked() )
	{
	  QColorGroup g;
	  if( act )
	    {
	      g.setColor( QColorGroup::Button,
			  scheme_.getColor( JFCScheme::Primary2 ) );
	      g.setColor( QColorGroup::Dark, Qt::black );
	      g.setColor( QColorGroup::Light,
			  scheme_.getColor( JFCScheme::Secondary3 ) );
	    }
	  else
	    {
	      g.setColor( QColorGroup::Button,
			  scheme_.getColor( JFCScheme::Secondary3 ) );
	      g.setColor( QColorGroup::Dark,
			  scheme_.getColor( JFCScheme::Secondary1 ) );
	      g.setColor( QColorGroup::Light, Qt::white );
	    }
	  drawButton( p, x + menuHMargin, y + ( h - checkHeight ) / 2,
		      10, 10, g );
	  drawCheckMark( p, x + menuHMargin + 2 , y +
			 ( h - checkHeight ) / 2, 0, 0, QColorGroup(),
			 act, !enabled );
	}
    }
  p->setPen( Qt::black );
  QString s = mi->text();
  int textX = x + menuHMargin + checkColWidth;
  if( checkable || maxpmw > 0 )
    textX += menuHMargin;
  if( !s.isNull() )
    {
      QFont textFont = p->font();
      int textHeight = p->fontMetrics().height();
      int textPoints = textFont.pointSize();
      int textY = y + ( h - textHeight ) / 2 - /* nudge up a little */ 1;
      int t = s.find( '\t' );
      const int textFlags = AlignVCenter | ShowPrefix | DontClip |
	SingleLine;
      if( t >= 0 )
	{
	  QFont tabFont( textFont );
	  tabFont.setPointSize( textPoints - 2 );
	  tabFont.setBold( FALSE );
	  p->setFont( tabFont );
	  int tabHeight = p->fontMetrics().height();
	  int tabY = textY + ( textHeight - tabHeight ) / 2;
	  if( !act )
	    p->setPen( scheme_.getColor( JFCScheme::Primary1 ) );
	  p->drawText( x + w - tab - menuHMargin, tabY, tab,
		       tabHeight, textFlags, s.mid( t + 1 ) );
	  p->setFont( textFont );
	}
      p->setPen( Qt::black );
      p->drawText( textX, textY, w - menuHMargin + 1, textHeight,
		   textFlags, s, t );
    }
  else if ( mi->pixmap() )
    {
      QPixmap *pixmap = mi->pixmap();
      if( pixmap->depth() == 1 )
	p->setBackgroundMode( OpaqueMode );
      p->drawPixmap( textX + menuHMargin, y + menuVMargin, *pixmap );
      if( pixmap->depth() == 1 )
	p->setBackgroundMode( TransparentMode );
    }
  if( mi->popup() )
    {
      int arrowX = x + w - menuHMargin - 4 - 1;
      int arrowY = y + ( h - 8 ) / 2;
      drawScrollArrow( p, Qt::RightArrow, arrowX, arrowY );
    }
}

/*!
  Draws the scroll-by-line button at either end of a scroll bar.
*/
void JFCStyle::drawScrollLineButton( QPainter* p, const QRect& r,
				     const QColorGroup&,
				     Qt::ArrowType dir, bool down )
{
  QColor dark( scheme_.getColor( JFCScheme::Secondary1 ) );
  QColor mid( scheme_.getColor( JFCScheme::Secondary2 ) );
  QColor bg( scheme_.getColor( JFCScheme::Secondary3 ) );
  p->fillRect( r, down ? mid : bg );
  int x, y, w, h;
  r.rect( &x, &y, &w, &h  );
  if( !down )
    {
      p->setPen( dark );
      p->drawRect( r );
      p->setPen( Qt::white );
      p->drawLine( x + 1, y + 1, x + w - 2, y + 1 );
      p->drawLine( x + 1, y + 1, x + 1, y + h - 2 );
      p->setPen( bg );
      p->drawPoint( x + 1, y + h - 1  );
      p->drawPoint( x + w - 1, y + 1  );
    }
  else
    {
      p->setPen( dark );
      p->drawLine( r.left(), r.top(), r.left(), r.bottom() );
      p->drawLine( r.left(), r.top(), r.right(), r.top() );
      if( dir != Qt::UpArrow && dir != Qt::DownArrow )
	p->drawLine( r.right(), r.top(), r.right(), r.bottom() );
      else
	p->drawLine( r.left(), r.bottom(), r.right(), r.bottom() );
    }
  int arrowWidth = ( dir == Qt::UpArrow || dir == Qt::DownArrow ) ?
    8 : 4;
  int arrowHeight = ( dir == Qt::UpArrow || dir == Qt::DownArrow ) ?
    4 : 8;
  int arrowX = r.left() + ( r.width() - arrowWidth ) / 2;
  int arrowY = r.top() + ( r.height() - arrowHeight ) / 2;
  drawScrollArrow( p, dir, arrowX, arrowY );
}

/*!
  Draws the scroll bar slider with the grip texture.
*/
void JFCStyle::drawScrollSlider( QPainter* p, const QRect& r,
				 const QColorGroup&, bool horizontal )
{
  QColor dark( scheme_.getColor( JFCScheme::Primary1 ) );
  QColor mid( scheme_.getColor( JFCScheme::Primary2 ) );
  QColor light( scheme_.getColor( JFCScheme::Primary3 ) );
  p->fillRect( r, mid );
  p->setPen( dark );
  p->drawRect( r );
  p->setPen( light );
  if( horizontal )
    {
      p->drawLine( r.left() + 1, r.top() + 1, r.left() + 1, r.bottom() );
      p->drawLine( r.left() + 1, r.top() + 1, r.right() - 2, r.top() + 1 );
    }
  else
    {
      p->drawLine( r.left() + 1, r.top() + 1, r.right(), r.top() + 1 );
      p->drawLine( r.left() + 1, r.top() + 1, r.left() + 1, r.bottom() - 2 );
    }
  drawGrip( p, QRect( r.left() + 3 + ( horizontal ? 1 : 0 ),
		      r.top() + 3 + ( horizontal ? 0 : 1 ),
		      r.width() - 6 - ( horizontal ? 1 : 0 ),
		      r.height() - 6 - ( horizontal ? 0 : 1 ) ), mid,
	    light, dark );
}

/*!
  Draws a JFC style checkmark.
*/
void JFCStyle::drawCheckMark( QPainter* p, int x, int y, int, int,
			      const QColorGroup&, bool, bool dis )
{
  static QCOORD check [] =
  {
    6,0,
      5,1, 6,1,
      0,2, 1,2, 4,2, 5,2,
      0,3, 1,3, 3,3, 4,3,
      0,4, 1,4, 2,4, 3,4,
      0,5, 1,5, 2,5,
      0,6, 1,6
      };

  QPointArray a( sizeof( check ) / ( sizeof( QCOORD ) * 2 ), check );
  a.translate( x, y );
  if( !dis )
    p->setPen( Qt::black );
  else
    p->setPen( scheme_.getColor( JFCScheme::Secondary2 ) );
  p->drawPoints( a );
}

/*!
  Draws the distinctive drag 'grip' texture on controls such as  scroll bars,
  sliders and splitter widgets.
*/
void JFCStyle::drawGrip( QPainter* p, const QRect& r, const QColor& bg,
			 const QColor& fg, const QColor& shadow )
{
  // We create a 4x4 pixmap, paint the repeating grip pattern on it using the
  // foreground, background and shadow colors, then create a brush with the
  // pixmap and fill the rectangle with it. Voila!
  //
  QPixmap pm( 4, 4);
  pm.fill( bg );
  QPainter pp( &pm );
  pp.setPen( fg );
  pp.drawPoint( 0,0 );
  pp.drawPoint( 2,2 );
  pp.setPen( shadow );
  pp.drawPoint( 1,1 );
  pp.drawPoint( 3,3 );
  p->setBrushOrigin( r.topLeft() );
  p->fillRect( r, QBrush( QColor(), pm ) );
}

/*!
  Draws the arrow in the scroll-by-line buttons of a scroll bars.
*/
void JFCStyle::drawScrollArrow( QPainter* p, Qt::ArrowType type, int x, int y )
{
  static QCOORD downArrow[] =
  {
    0,0, 1,0, 2,0, 3,0, 4,0, 5,0, 6,0, 7,0,
      1,1, 2,1, 3,1, 4,1, 5,1, 6,1,
      2,2, 3,2, 4,2, 5,2,
      3,3, 4,3,
      };
  static QCOORD upArrow[] =
  {
    3,0, 4,0,
      2,1, 3,1, 4,1, 5,1,
      1,2, 2,2, 3,2, 4,2, 5,2, 6,2,
      0,3, 1,3, 2,3, 3,3, 4,3, 5,3, 6,3, 7,3
      };
  static QCOORD rightArrow[] =
  {
    0,0,
      0,1, 1,1,
      0,2, 1,2, 2,2,
      0,3, 1,3, 2,3, 3,3,
      0,4, 1,4, 2,4, 3,4,
      0,5, 1,5, 2,5,
      0,6, 1,6,
      0,7,
      };
  static QCOORD leftArrow[] =
  {
    3,0,
      2,1, 3,1,
      1,2, 2,2, 3,2,
      0,3, 1,3, 2,3, 3,3,
      0,4, 1,4, 2,4, 3,4,
      1,5, 2,5, 3,5,
      2,6, 3,6,
      3,7,
      };

  QPointArray a;
  switch( type )
    {
    case Qt::DownArrow:
      a.setPoints( sizeof( downArrow ) / ( sizeof( QCOORD ) * 2 ), downArrow );
      break;
    case Qt::UpArrow:
      a.setPoints( sizeof( upArrow ) / ( sizeof( QCOORD ) * 2 ), upArrow );
      break;
    case Qt::RightArrow:
      a.setPoints( sizeof( rightArrow ) /
		   ( sizeof( QCOORD ) * 2 ), rightArrow );
      break;
    case Qt::LeftArrow:
      a.setPoints( sizeof( leftArrow ) /
		   ( sizeof( QCOORD ) * 2 ), leftArrow );
      break;
    }
  a.translate( x, y );
  p->setPen( Qt::black );
  p->drawPoints( a );
}

void JFCStyle::setScheme( const JFCScheme& scheme )
{
  qApp->setStyle( new JFCStyle( scheme ) );
}

