// (c) 2000 Keith Brown

// $Source$
// $Author$
// $Date$
// $Revision$

// This file may be distributed under the terms of the Q Public License
// as defined by Troll Tech AS of Norway and appearing in the file
// LICENSE.QPL included in the packaging of this file.

#ifndef JFCSTYLE_INCLUDED
# define JFCSTYLE_INCLUDED

# include <qfont.h>
# include <qpalette.h>
# include <qwindowsstyle.h>

class JFCScheme
{

 public:

  enum SchemeColorRole
  {
    Primary1, 
    Primary2, 
    Primary3, 
    Secondary1, 
    Secondary2, 
    Secondary3,
    NSchemeColors
  };

  JFCScheme();

  JFCScheme( const QColor& primary1, const QColor& primary2, 
	     const QColor& primary3, const QColor& secondary1, 
	     const QColor& secondary2, const QColor& secondary3 );

  JFCScheme( const JFCScheme& rhs );

  const JFCScheme& operator=( const JFCScheme& rhs );

  QColor getColor( SchemeColorRole role ) const;

  void setColor( SchemeColorRole role, const QColor& color );

  void setColors( const QColor& primary1, const QColor& primary2,
		  const QColor& primary3, const QColor& secondary1,
		  const QColor& secondary2, const QColor& secondary3 );

  static JFCScheme Default;
  static JFCScheme Jade;
  static JFCScheme HiContrast;

 private:

  QColor colors_[ NSchemeColors ];
};

class JFCStyle : public QWindowsStyle
{

 public:

  JFCStyle();

  JFCStyle( const JFCScheme& scheme );

  void polish( QWidget* widget );

  void polish( QApplication* app );

  void unPolish( QApplication* app );

  void drawSeparator( QPainter* p, int x1, int y1, int x2, int y2,
		      const QColorGroup& g, bool sunken = TRUE,
		      int lineWidth = 1, int midLineWidth = 0 );

  void drawButton( QPainter* p, int x, int y, int w, int h,
		   const QColorGroup& g, bool sunken = FALSE,
		   const QBrush* fill = 0 );
 
  QRect buttonRect( int x, int y, int w, int h );

  void drawBevelButton( QPainter* p, int x, int y, int w, int h,
		       const QColorGroup& g, bool sunken = FALSE,
		       const QBrush* fill = 0 );

  void drawPanel( QPainter* p, int x, int y, int w, int h,
		  const QColorGroup& g, bool sunken = FALSE, int lineWidth = 1,
		  const QBrush* fill = 0 );

  void drawPopupPanel( QPainter* p, int x, int y, int w, int h,
		       const QColorGroup& g, int lineWidth = 2,
		       const QBrush* fill = 0 );
  QSize exclusiveIndicatorSize() const;

  void drawExclusiveIndicator( QPainter* p, int x, int y, int w, int h,
			       const QColorGroup& g, bool on, 
			       bool down = FALSE, bool enabled = TRUE );

  QSize indicatorSize() const;

  void drawIndicator( QPainter* p, int x, int y, int w, int h,
		      const QColorGroup& g, int state, bool down = FALSE,
		      bool enabled = TRUE );

  void drawFocusRect( QPainter*, const QRect&, const QColorGroup&,
		      const QColor* bg = 0, bool atBorder = FALSE );

  void drawComboButton( QPainter* p, int x, int y, int w, int h,
			const QColorGroup& g, bool sunken = FALSE,
			bool editable = FALSE, bool enabled = TRUE,
			const QBrush* fill = 0 );

  QRect comboButtonRect( int x, int y, int w, int h );

  QRect comboButtonFocusRect( int x, int y, int w, int h );

  void drawPushButton( QPushButton* b, QPainter* p );

  void getButtonShift( int&, int& );

  int defaultFrameWidth() const;

  void tabbarMetrics( const QTabBar* tb, int& hframe, int& vframe,
		      int& overlap );

  void drawTab( QPainter* p, const QTabBar* tb, QTab* t, bool selected );

  void scrollBarMetrics( const QScrollBar* sb, int& sliderMin, 
			 int& sliderMax, int& sliderLength, int& buttonDim);

  void drawScrollBarControls( QPainter* p, const QScrollBar* sb, 
			      int sliderStart, uint controls, 
			      uint activeControl );

  int sliderLength() const;

  void drawSlider( QPainter* p, int x, int y, int w, int h, 
		   const QColorGroup& g, Orientation orient, bool tickAbove, 
		   bool tickBelow );

  void drawSliderGroove( QPainter* p, int x, int y, int w, int h, 
			 const QColorGroup& g, QCOORD c, Orientation orient );

  int maximumSliderDragDistance() const;

  int splitterWidth() const;
  
  void drawSplitter( QPainter* p, int x, int y, int w, int h, 
		     const QColorGroup& g, Orientation orient );
  
  void polishPopupMenu( QPopupMenu* );

  int extraPopupMenuItemWidth( bool checkable, int maxpmw, QMenuItem* mi, 
			       const QFontMetrics& fm );
  int popupSubmenuIndicatorWidth( const QFontMetrics& fm);

  int popupMenuItemHeight( bool checkable, QMenuItem* mi, 
			   const QFontMetrics& fm );

  void drawPopupMenuItem( QPainter* p, bool checkable, int maxpmw, int tab, 
			  QMenuItem* mi, const QPalette& pal, bool act, 
			  bool enabled, int x, int y, int w, int h );

  static void setScheme( const JFCScheme& scheme );

 protected:

  void drawCheckMark( QPainter* p, int x, int y, int w, int h,
		      const QColorGroup& g, bool act, bool dis );

  void drawGrip( QPainter* p, const QRect& r, const QColor& bg,
		 const QColor& fg, const QColor& shadow );

  void drawScrollArrow( QPainter* p, Qt::ArrowType type, int x, int y );

  void drawScrollLineButton( QPainter* p, const QRect& r, const QColorGroup& g,
			     Qt::ArrowType dir, bool down );

  void drawScrollSlider( QPainter* p, const QRect& r, const QColorGroup& g,
			 bool horizontal );
 private:

  static JFCScheme scheme_;
  QPalette oldPalette_;
  QFont oldFont_;
};

#endif

