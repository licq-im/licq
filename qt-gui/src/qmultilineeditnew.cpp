/**********************************************************************
** $Id$
**
** Definition of QMultiLineEditNew widget class
**
** Created : 961005
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qmultilineeditnew.h"
#include "qpainter.h"
#include "qscrollbar.h"
#include "qclipboard.h"
#include "qpixmap.h"
#include "qregexp.h"
#include "qapplication.h"
#include "qdragobject.h"
#include <ctype.h>


/*!
  \class QMultiLineEditNew qmultilineedit.h

  \brief The QMultiLineEditNew widget is a simple editor for inputting text.

  \ingroup realwidgets

  The QMultiLineEditNew widget provides multiple line text input and display.
  It is intended for moderate amounts of text. There are no arbitrary
  limitations, but if you try to handle megabytes of data, performance
  will suffer.

  Per default, the edit widget does not perform any word
  wrapping. This can be adjusted by calling setWordWrap(). Both
  dynamic wrapping according to the visible width or a fixed number of
  character or pixels is supported.

  The widget can be used to display text by calling setReadOnly(TRUE)

  The default key bindings are described in keyPressEvent(); they cannot
  be customized except by inheriting the class.

  <img src=qmlined-m.png> <img src=qmlined-w.png>
 */

struct QMultiLineData
{
    QMultiLineData() :
	isHandlingEvent(FALSE),
	edited(FALSE),
	maxLineWidth(0),
	align(Qt::AlignLeft),
	maxlines(-1),
	maxlinelen(-1),
	maxlen(-1),
	wrapmode( 0 ),
	wrapcol( -1 ),
	// This doesn't use font bearings, as textWidthWithTabs does that.
	// This is just an aesthetics value.
	// It should probably be QMAX(0,3-fontMetrics().minLeftBearing()) though,
	// as bearings give some border anyway.
	lr_marg(3),
	echomode(QMultiLineEditNew::Normal),
	val(0),
	dnd_primed(FALSE),
	dnd_forcecursor(FALSE),
	dnd_timer(0)
		{}
    bool isHandlingEvent;
    bool edited;
    int  maxLineWidth;
    int	 scrollTime;
    int	 scrollAccel;
    int  align;
    int  maxlines;
    int  maxlinelen;
    int  maxlen;
    int wrapmode;
    int wrapcol;
    int  lr_marg;
    QMultiLineEditNew::EchoMode echomode;
    const QValidator* val;

    bool dnd_primed; // If TRUE, user has pressed
    bool dnd_forcecursor; // If TRUE show cursor for DND feedback,
			    // even if !hasFocus()
    int	 dnd_timer;  // If it expires before release, start drag
};


static const int initialScrollTime = 50; // mark text scroll time
static const int initialScrollAccel = 5; // mark text scroll accel (0=fastest)
static const int scroll_margin = 16;     // auto-scroll edge in DND

#define WORD_WRAP ( (d->wrapmode & 3) != 0 )
#define DYNAMIC_WRAP ( (d->wrapmode & 3) == DynamicWrap )
#define FIXED_WIDTH_WRAP ( (d->wrapmode & 3) == FixedWidthWrap )
#define FIXED_COLUMN_WRAP ( (d->wrapmode & 3) == FixedColumnWrap )
#define BREAK_WITHIN_WORDS ( (d->wrapmode & BreakWithinWords) == BreakWithinWords )

static int tabStopDist( const QFontMetrics &fm )
{
    return 8*fm.width( QChar('x') );
}

static int textWidthWithTabs( const QFontMetrics &fm, const QString &s, uint start, uint nChars, int align )
{
    if ( s.isEmpty() )
	return 0;

    int dist = -fm.minLeftBearing();
    int i = start;
    int tabDist = -1; // lazy eval
    while ( (uint)i < s.length() && (uint)i < start+nChars ) {
	if ( s[i] == '\t' && align == Qt::AlignLeft ) {
	    if ( tabDist<0 )
		tabDist = tabStopDist(fm);
	    dist = ( dist/tabDist + 1 ) * tabDist;
	    i++;
	} else {
	    int ii = i;
	    while ( (uint)i < s.length() && (uint)i < start + nChars && ( align != Qt::AlignLeft || s[i] != '\t' ) )
		i++;
	    dist += fm.width( s.mid(ii,i-ii) );
	}
    }
    return dist;
}

static int xPosToCursorPos( const QString &s, const QFontMetrics &fm,
			    int xPos, int width, int align )
{
    int i = 0;
    int	  dist;
    int tabDist;

    if ( s.isEmpty() )
	return 0;
    if ( xPos > width )
	xPos = width;
    if ( xPos <= 0 )
	return 0;

    dist    = -fm.minLeftBearing();

    if ( align == Qt::AlignCenter )
	dist = ( width - textWidthWithTabs( fm, s, 0, s.length(), align ) ) / 2;
    else if ( align == Qt::AlignRight )
	dist = width - textWidthWithTabs( fm, s, 0, s.length(), align );

    int     distBeforeLastTab = dist;
    tabDist = tabStopDist(fm);
    while ( (uint)i < s.length() && dist < xPos ) {
	if ( s[i] == '\t' && align == Qt::AlignLeft ) {
	    distBeforeLastTab = dist;
	    dist = (dist/tabDist + 1) * tabDist;
	} else {
	    dist += fm.width( s[i] );
	}
	i++;
    }
    if ( dist > xPos ) {
	if ( dist > width ) {
	    i--;
	} else {
	    if ( s[i-1] == '\t' && align == Qt::AlignLeft ) { // dist equals a tab stop position
		if ( xPos - distBeforeLastTab < (dist - distBeforeLastTab)/2 )
		    i--;
	    } else {
		if ( fm.width(s[i-1])/2 < dist-xPos )
		    i--;
	    }
	}
    }
    return i;
}

/*!
  Creates a new, empty, QMultiLineEditNew.
*/

QMultiLineEditNew::QMultiLineEditNew( QWidget *parent , const char *name )
    :QTableView( parent, name, WNorthWestGravity )
{
    d = new QMultiLineData;
    QFontMetrics fm( font() );
    setCellHeight( fm.lineSpacing() );
    setNumCols( 1 );

    setNumRows( 0 );
    contents = new QList<QMultiLineEditNewRow>;
    contents->setAutoDelete( TRUE );

    cursorX = 0; cursorY = 0;
    curXPos = 0;

    setTableFlags( Tbl_autoVScrollBar|Tbl_autoHScrollBar|
		   Tbl_smoothVScrolling |
		   Tbl_clipCellPainting
		   );
    setFrameStyle( QFrame::WinPanel | QFrame::Sunken );
    setBackgroundMode( PaletteBase );
    setWFlags( WResizeNoErase );
    setKeyCompression( TRUE );
    setFocusPolicy( WheelFocus );
    setCursor( ibeamCursor );
    verticalScrollBar()->setCursor( arrowCursor );
    horizontalScrollBar()->setCursor( arrowCursor );
    readOnly 	   = FALSE;
    cursorOn	   = FALSE;
    dummy          = TRUE;
    markIsOn	   = FALSE;
    dragScrolling  = FALSE;
    dragMarking    = FALSE;
    textDirty	   = FALSE;
    wordMark	   = FALSE;
    overWrite	   = FALSE;
    markAnchorX    = 0;
    markAnchorY    = 0;
    markDragX      = 0;
    markDragY      = 0;
    blinkTimer     = 0;
    scrollTimer    = 0;
    d->scrollTime = 0;

    dummy = TRUE;
    int w = textWidth("");
    contents->append(  new QMultiLineEditNewRow("", w) );
    setNumRows( 1 );
    setWidth( w );
    setAcceptDrops(TRUE);
}

/*! \fn int QMultiLineEditNew::numLines() const

  Returns the number of lines in the editor. The count includes any
  empty lines at top and bottom, so for an empty editor this method
  will return 1.
*/

/*! \fn bool QMultiLineEditNew::atEnd() const

  Returns TRUE if the cursor is placed at the end of the text.
*/

/*! \fn bool QMultiLineEditNew::atBeginning() const

  Returns TRUE if the cursor is placed at the beginning of the text.
*/


/*!
  \fn int QMultiLineEditNew::lineLength( int line ) const
  Returns the number of characters at line number \a line.
*/

/*! \fn QString *QMultiLineEditNew::getString( int line ) const

  Returns a pointer to the text at line \a line.
*/

/*! \fn void QMultiLineEditNew::textChanged()

  This signal is emitted when the text is changed by an event or by a
  slot. Note that the signal is not emitted when you call a non-slot
  function such as insertLine().

  \sa returnPressed()
*/

/*! \fn void QMultiLineEditNew::returnPressed()

  This signal is emitted when the user presses the return or enter
  key. It is not emitted if isReadOnly() is TRUE.

  \sa textChanged()
*/


/*! \fn bool QMultiLineEditNew::isReadOnly() const

  Returns FALSE if this multi line edit accepts text input.
  Scrolling and cursor movements are accepted in any case.

  \sa setReadOnly() QWidget::isEnabled()
*/

/*! \fn bool QMultiLineEditNew::isOverwriteMode() const

  Returns TRUE if this multi line edit is in overwrite mode, i.e.
  if characters typed replace characters in the editor.

  \sa setOverwriteMode()
*/


/*! \fn void QMultiLineEditNew::setOverwriteMode( bool on )

  Sets overwrite mode if \a on is TRUE. Overwrite mode means
  that characters typed replace characters in the editor.

  \sa isOverwriteMode()
*/




/*!
  If \a on is FALSE, this multi line edit accepts text input.
  Scrolling and cursor movements are accepted in any case.

  \sa isReadOnly() QWidget::setEnabled()
*/

void QMultiLineEditNew::setReadOnly( bool on )
{
    if ( readOnly != on ) {
	readOnly = on;
	setCursor( on ? arrowCursor : ibeamCursor );
    }
}

/*!
  Returns the width in pixels of the longest text line in this editor.
*/
int QMultiLineEditNew::maxLineWidth() const
{
    return d->maxLineWidth;
}

/*!
  Destroys the QMultiLineEditNew
*/

QMultiLineEditNew::~QMultiLineEditNew()
{
    delete contents;
    delete d;
}

static QPixmap *buffer = 0;

static void cleanupMLBuffer()
{
    delete buffer;
    buffer = 0;
}

static QPixmap *getCacheBuffer( QSize sz )
{
    if ( !buffer ) {
	qAddPostRoutine( cleanupMLBuffer );
	buffer = new QPixmap;
    }

    if ( buffer->width() < sz.width() || buffer->height() < sz.height() )
	buffer->resize( sz );
    return buffer;
}

/*!
  Implements the basic drawing logic.
*/
void QMultiLineEditNew::paintCell( QPainter *painter, int row, int )
{
    const QColorGroup & g = colorGroup();
    QFontMetrics fm( painter->font() );
    QString s = stringShown(row);
    if ( s.isNull() ) {
	qWarning( "QMultiLineEditNew::paintCell: (%s) no text at line %d",
		 name( "unnamed" ), row );
	return;
    }
    QRect updateR = cellUpdateRect();
    QPixmap *buffer = getCacheBuffer( updateR.size() );
    ASSERT(buffer);
    buffer->fill ( g.base() );

    QPainter p( buffer );
    p.setFont( painter->font() );
    p.translate( -updateR.left(), -updateR.top() );

    p.setTabStops( tabStopDist(fm) );

    int yPos = 0;
    int markX1, markX2;				// in x-coordinate pixels
    markX1 = markX2 = 0;			// avoid gcc warning
    if ( markIsOn ) {
	int markBeginX, markBeginY;
	int markEndX, markEndY;
	getMarkedRegion( &markBeginY, &markBeginX, &markEndY, &markEndX );
	if ( row >= markBeginY && row <= markEndY ) {
	    if ( row == markBeginY ) {
		markX1 = markBeginX;
		if ( row == markEndY ) 		// both marks on same row
		    markX2 = markEndX;
		else
		    markX2 = s.length();	// mark till end of line
	    } else {
		if ( row == markEndY ) {
		    markX1 = 0;
		    markX2 = markEndX;
		} else {
		    markX1 = 0;			// whole line is marked
		    markX2 = s.length();	// whole line is marked
		}
	    }
	}
    }
    p.setPen( g.text() );
    QMultiLineEditNewRow* r = contents->at( row );
    int wcell = cellWidth() - 2*d->lr_marg;
    int wrow = r->w;
    int x = d->lr_marg;
    if ( d->align == Qt::AlignCenter )
	x += (wcell - wrow) / 2;
    else if ( d->align == Qt::AlignRight )
	x += wcell - wrow;
   p.drawText( x,  yPos, cellWidth()-d->lr_marg-x, cellHeight(),
		d->align == AlignLeft?ExpandTabs:0, s );
#if 0
    if ( r->newline )
	p.drawLine( d->lr_marg,  yPos+cellHeight()-2, cellWidth() - d->lr_marg, yPos+cellHeight()-2);
#endif
    if ( markX1 != markX2 ) {
	int sLength = s.length();
	int xpos1   =  mapToView( markX1, row );
	int xpos2   =  mapToView( markX2, row );
	int fillxpos1 = xpos1;
	int fillxpos2 = xpos2;
	if ( markX1 == 0 )
	    fillxpos1 -= 2;
	if ( markX2 == sLength )
	    fillxpos2 += 3;
	p.setClipping( TRUE );
	p.setClipRect( fillxpos1 - updateR.left(), 0,
		       fillxpos2 - fillxpos1, cellHeight(row) );
	p.fillRect( fillxpos1, 0, fillxpos2 - fillxpos1, cellHeight(row),
		    g.brush( QColorGroup::Highlight ) );
	p.setPen( g.highlightedText() );	
	p.drawText( x,  yPos, cellWidth()-d->lr_marg-x, cellHeight(),
		    d->align == AlignLeft?ExpandTabs:0, s );
	p.setClipping( FALSE );
    }

    if ( row == cursorY && cursorOn && !readOnly ) {
	int cursorPos = QMIN( (int)s.length(), cursorX );
	int cXPos   = mapToView( cursorPos, row );
	int cYPos   = 0;
	if ( hasFocus() || d->dnd_forcecursor ) {
	    p.setPen( g.text() );
/* styled?
	    p.drawLine( cXPos - 2, cYPos,
			cXPos + 2, cYPos );
*/
	    p.drawLine( cXPos    , cYPos,
			cXPos    , cYPos + fm.height() - 2);
/* styled?
	    p.drawLine( cXPos - 2, cYPos + fm.height() - 2,
			cXPos + 2, cYPos + fm.height() - 2);
*/

	    // TODO: set it other times, eg. when scrollbar moves view
	    QWMatrix wm = painter->worldMatrix();
	    setMicroFocusHint( int(wm.dx()+cXPos),
			       int (wm.dy()+cYPos),
			       1, fm.ascent() );
	}
    }
    p.end();
    painter->drawPixmap( updateR.left(), updateR.top(), *buffer,
			 0, 0, updateR.width(), updateR.height() );
}


/*!
  Returns the width in pixels of the string \a s.
  NOTE: only appropriate for whole lines.
*/

int QMultiLineEditNew::textWidth( const QString &s )
{
    int w = !s.isNull()
		? textWidthWithTabs( QFontMetrics( font() ), s, 0, s.length(), d->align ) : 0;
    return w + 2 * d->lr_marg;
}


/*!
  Returns the width in pixels of the text at line \a line.
*/

int QMultiLineEditNew::textWidth( int line )
{
    if ( d->echomode == Password) {
	QString s = stringShown(line);
	return textWidth( s );
    }
    QMultiLineEditNewRow* r = contents->at(line);
    return r?r->w:0;
}

/*!
  Starts the cursor blinking.
*/

void QMultiLineEditNew::focusInEvent( QFocusEvent * )
{
    stopAutoScroll();
    if ( !blinkTimer )
	blinkTimer = startTimer( QApplication::cursorFlashTime() / 2 );
    cursorOn = TRUE;
    updateCell( cursorY, 0, FALSE );
}


/*!
  Handles auto-copy of selection (X11 only).
*/

void QMultiLineEditNew::leaveEvent( QEvent * )
{
#if defined(_WS_X11_)
    if ( style() == WindowsStyle ) {
	// X11 users are very accustomed to "auto-copy"
	if ( echoMode() == Normal )
	    copy();
    }
#endif
}


/*!
  stops the cursor blinking.
*/

void QMultiLineEditNew::focusOutEvent( QFocusEvent * )
{
#if defined(_WS_X11_)
    if ( style() == WindowsStyle ) {
	// X11 users are very accustomed to "auto-copy"
	if ( echoMode() == Normal )
	    copy();
    }
#endif

    stopAutoScroll();
    killTimer( blinkTimer );
    blinkTimer = 0;
    if ( cursorOn ) {
	cursorOn = FALSE;
	updateCell( cursorY, 0, FALSE );
    }
}


/*!
  Cursor blinking, drag scrolling.
*/

void QMultiLineEditNew::timerEvent( QTimerEvent *t )
{
    if ( hasFocus() && t->timerId() == blinkTimer ) {
	cursorOn = !cursorOn;
	updateCell( cursorY, 0, FALSE );
    } else if ( t->timerId() == scrollTimer ) {
	QPoint p = mapFromGlobal( QCursor::pos() );
	if ( d->scrollAccel-- <= 0 && d->scrollTime ) {
	    d->scrollAccel = initialScrollAccel;
	    d->scrollTime--;
	    killTimer( scrollTimer );
	    scrollTimer = startTimer( d->scrollTime );
	}
	int l = QMAX(1,(initialScrollTime-d->scrollTime)/5);

	// auto scrolling is dual-use - for highlighting and DND
	int margin = d->dnd_primed ? scroll_margin : 0;
	bool mark = !d->dnd_primed;
	bool clear_mark = d->dnd_primed ? FALSE : !mark;

	for (int i=0; i<l; i++) {
	    if ( p.y() < margin ) {
		cursorUp( mark, clear_mark );
	    } else if ( p.y() > height()-margin ) {
		cursorDown( mark, clear_mark );
	    } else if ( p.x() < margin ) {
		cursorLeft( mark, clear_mark, FALSE );
	    } else if ( p.x() > width()-margin ) {
		cursorRight( mark, clear_mark, FALSE );
	    } else {
		stopAutoScroll();
		break;
	    }
	}
    } else if ( t->timerId() == d->dnd_timer ) {
	doDrag();
    }
}

void QMultiLineEditNew::doDrag()
{
    if ( d->dnd_timer ) {
	killTimer(d->dnd_timer);
	d->dnd_timer = 0;
    }
    QDragObject *drag_text = new QTextDrag(markedText(), this);
    if ( readOnly ) {
	drag_text->dragCopy();
    } else {
	if ( drag_text->drag() && QDragObject::target() != this ) {
	    del();
	    if ( textDirty && !d->isHandlingEvent )
		emit textChanged();
	}
    }
    d->dnd_primed = FALSE;
}

/*!
  If there is marked text, sets \a line1, \a col1, \a line2 and \a col2
  to the start and end of the marked region and returns TRUE. Returns
  FALSE if there is no marked text.
 */
bool QMultiLineEditNew::getMarkedRegion( int *line1, int *col1,
				      int *line2, int *col2 ) const
{
    if ( !markIsOn || !line1 || !line2 || !col1 || !col2 )
	return FALSE;
    if ( markAnchorY < markDragY ||
	 markAnchorY == markDragY && markAnchorX < markDragX) {
	*line1 = markAnchorY;
	*col1 = markAnchorX;
	*line2 = markDragY;
	*col2 = markDragX;
    } else {
	*line1 = markDragY;
	*col1 = markDragX;
	*line2 = markAnchorY;
	*col2 = markAnchorX;
    }
    return markIsOn;
}


/*!
  Returns TRUE if there is marked text.
*/

bool QMultiLineEditNew::hasMarkedText() const
{
    return markIsOn;
}


/*!
  Returns a copy of the marked text.
*/

QString QMultiLineEditNew::markedText() const
{
    int markBeginX, markBeginY;
    int markEndX, markEndY;
    if ( !getMarkedRegion( &markBeginY, &markBeginX, &markEndY, &markEndX ) )
	return QString();
    if ( markBeginY == markEndY ) { //just one line
	QString *s  = getString( markBeginY );
	return s->mid( markBeginX, markEndX - markBeginX );
    } else { //multiline
	QString *firstS, *lastS;
	firstS = getString( markBeginY );
	lastS  = getString( markEndY );
	int i;
	QString tmp;
	if ( firstS )
	    tmp += firstS->mid(markBeginX);
	if ( contents->at( markBeginY )->newline )
	    tmp += '\n';

	for( i = markBeginY + 1; i < markEndY ; i++ ) {
	    tmp += *getString(i);
	    if ( contents->at( i )->newline )
		tmp += '\n';
	}

	if ( lastS ) {
	    tmp += lastS->left(markEndX);
	} else {
	    tmp.truncate(tmp.length()-1);
	}

	return tmp;
    }
}



/*!
  Returns the text at line number \a line (possibly the empty string),
  or a \link QString::operator!() null string\endlink if \a line is invalid.
*/

QString QMultiLineEditNew::textLine( int line ) const
{
    QString *s = getString(line);
    if ( s ) {
	if ( s->isNull() )
	    return QString::fromLatin1("");
	else
	    return *s;
    } else
	return QString::null;
}


/*!
  Returns a copy of the whole text. If the multi line edit contains no
  text, a
  \link QString::operator!() null string\endlink
  is returned.
*/

QString QMultiLineEditNew::text() const
{
    QString tmp;
    for( int i = 0 ; i < (int)contents->count() ; i++ ) {
	tmp += *getString(i);
	if ( i+1 < (int)contents->count() && contents->at(i)->newline )
	    tmp += '\n';
    }
    return tmp;
}


/*!
  Selects all text without moving the cursor.
*/

void QMultiLineEditNew::selectAll()
{
    markAnchorX    = 0;
    markAnchorY    = 0;
    markDragY = numLines() - 1;
    markDragX = lineLength( markDragY );
    markIsOn = ( markDragX != markAnchorX ||  markDragY != markAnchorY );
    repaintDelayed();
}



/*!
  Deselects all text (i.e. removes marking) and leaves the cursor at the
  current position.
*/

void QMultiLineEditNew::deselect()
{
    turnMarkOff();
}


/*!
  Sets the text to \a s, removing old text, if any.
*/

void QMultiLineEditNew::setText( const QString &s )
{
    bool oldAuto = autoUpdate();
    setAutoUpdate( FALSE );
    clear();
    insertLine( s, -1 );
    emit textChanged();
    setAutoUpdate(oldAuto);
    if ( autoUpdate() )
	repaintDelayed( FALSE );
}


/*!
  Appends \a s to the text.
*/

void QMultiLineEditNew::append( const QString &s )
{
    insertLine( s, -1 );
    emit textChanged();
}

/*!
An override - pass wheel events to the vertical scrollbar
*/
void QMultiLineEditNew::wheelEvent( QWheelEvent *e ){
    QApplication::sendEvent( verticalScrollBar(), e);
}


/*!
  The key press event handler converts a key press to some line editor
  action.

  Here are the default key bindings when isReadOnly() is FALSE:
  <ul>
  <li><i> Left Arrow </i> Move the cursor one character leftwards
  <li><i> Right Arrow </i> Move the cursor one character rightwards
  <li><i> Up Arrow </i> Move the cursor one line upwards
  <li><i> Down Arrow </i> Move the cursor one line downwards
  <li><i> Page Up </i> Move the cursor one page upwards
  <li><i> Page Down </i> Move the cursor one page downwards
  <li><i> Backspace </i> Delete the character to the left of the cursor
  <li><i> Home </i> Move the cursor to the beginning of the line
  <li><i> End </i>	 Move the cursor to the end of the line
  <li><i> Delete </i> Delete the character to the right of the cursor
  <li><i> Shift - Left Arrow </i> Mark text one character leftwards
  <li><i> Shift - Right Arrow </i> Mark text one character rightwards
  <li><i> Control-A </i> Move the cursor to the beginning of the line
  <li><i> Control-B </i> Move the cursor one character leftwards
  <li><i> Control-C </i> Copy the marked text to the clipboard.
  <li><i> Control-D </i> Delete the character to the right of the cursor
  <li><i> Control-E </i> Move the cursor to the end of the line
  <li><i> Control-F </i> Move the cursor one character rightwards
  <li><i> Control-H </i> Delete the character to the left of the cursor
  <li><i> Control-K </i> Delete to end of line
  <li><i> Control-N </i> Move the cursor one line downwards
  <li><i> Control-P </i> Move the cursor one line upwards
  <li><i> Control-V </i> Paste the clipboard text into line edit.
  <li><i> Control-X </i> Cut the marked text, copy to clipboard.
  </ul>
  All other keys with valid ASCII codes insert themselves into the line.

  Here are the default key bindings when isReadOnly() is TRUE:
  <ul>
  <li><i> Left Arrow </i> Scrolls the table rightwards
  <li><i> Right Arrow </i> Scrolls the table rightwards
  <li><i> Up Arrow </i> Scrolls the table one line downwards
  <li><i> Down Arrow </i> Scrolls the table one line upwards
  <li><i> Page Up </i> Scrolls the table one page downwards
  <li><i> Page Down </i> Scrolls the table one page upwards
  </ul>

*/

void QMultiLineEditNew::keyPressEvent( QKeyEvent *e )
{
    textDirty = FALSE;
    d->isHandlingEvent = TRUE;
    int unknown = 0;
    if ( readOnly ) {
	int pageSize = viewHeight() / cellHeight();

	switch ( e->key() ) {
	case Key_Left:
	    setXOffset( xOffset() - viewWidth()/10 );
	    break;
	case Key_Right:
	    setXOffset( xOffset() + viewWidth()/10 );
	    break;
	case Key_Up:
	    setTopCell( topCell() - 1 );
	    break;
	case Key_Down:
	    setTopCell( topCell() + 1 );
	    break;
	case Key_Next:
	    setTopCell( topCell() + pageSize );
	    break;
	case Key_Prior:
	    setTopCell( QMAX( topCell() - pageSize, 0 ) );
	    break;
	default:
	    unknown++;
	}
	if ( unknown )
	    e->ignore();
	d->isHandlingEvent = FALSE;
	return;
    }
    if ( e->text().length() &&
	 ! ( e->state() & ControlButton ) &&
	 e->key() != Key_Return &&
	 e->key() != Key_Enter &&
	 e->key() != Key_Delete &&
	 e->key() != Key_Backspace &&
	 (!e->ascii() || e->ascii()>=32)
	 ) {
	insert( e->text() );
	//QApplication::sendPostedEvents( this, QEvent::Paint );
	if ( textDirty )
	    emit textChanged();
	d->isHandlingEvent = FALSE;
	return;
    }
    if ( e->state() & ControlButton ) {
	switch ( e->key() ) {
	case Key_A:
	    home( e->state() & ShiftButton );
	    break;
	case Key_B:
	    cursorLeft( e->state() & ShiftButton );
	    break;
	case Key_C:
	    if ( echoMode() == Normal )
		copy();
	    break;
	case Key_D:
	    del();
	    break;
	case Key_E:
	    end( e->state() & ShiftButton );
	    break;
	case Key_Left:
	    cursorWordBackward( e->state() & ShiftButton );
	    break;
	case Key_Right:
	    cursorWordForward( e->state() & ShiftButton );
	    break;
	case Key_Up:
	    cursorUp( e->state() & ShiftButton );
	    break;
	case Key_Down:
	    cursorDown( e->state() & ShiftButton );
	    break;
	case Key_F:
	    cursorRight( e->state() & ShiftButton );
	    break;
	case Key_H:
	    backspace();
	    break;
	case Key_K:
	    killLine();
	    break;
	case Key_N:
	    cursorDown( e->state() & ShiftButton );
	    break;
	case Key_P:
	    cursorUp( e->state() & ShiftButton );
	    break;
	case Key_V:
	    paste();
	    break;
	case Key_X:
	    cut();
	    break;
	default:
	    unknown++;
	}
    } else {
	switch ( e->key() ) {
	case Key_Left:
	    cursorLeft( e->state() & ShiftButton );
	    break;
	case Key_Right:
	    cursorRight( e->state() & ShiftButton );
	    break;
	case Key_Up:
	    cursorUp( e->state() & ShiftButton );
	    break;
	case Key_Down:
	    cursorDown( e->state() & ShiftButton );
	    break;
	case Key_Backspace:
	    backspace();
	    break;
	case Key_Home:
	    home( e->state() & ShiftButton );
	    break;
	case Key_End:
	    end( e->state() & ShiftButton );
	    break;
	case Key_Delete:
	    del();
	    break;
	case Key_Next:
	    pageDown( e->state() & ShiftButton );
	    break;
	case Key_Prior:
	    pageUp( e->state() & ShiftButton );
	    break;
	case Key_Enter:
	case Key_Return:
	    newLine();
	    emit returnPressed();
	    break;
	case Key_Tab:
	    qDebug("tab");
	    insert( e->text() );
	    break;
	default:
	    unknown++;
	}
    }
    if ( textDirty )
	emit textChanged();

    if ( unknown )				// unknown key
	e->ignore();

    d->isHandlingEvent = FALSE;
}


/*!
  Moves the cursor one page down.  If \a mark is TRUE, the text
  is marked.
*/

void QMultiLineEditNew::pageDown( bool mark )
{
    bool oldAuto = autoUpdate();
    if ( mark )
	setAutoUpdate( FALSE );

    if ( partiallyInvisible( cursorY ) )
	cursorY = topCell();
    int delta = cursorY - topCell();
    int pageSize = viewHeight() / cellHeight();
    int newTopCell = QMIN( topCell() + pageSize, numLines() - 1 - pageSize );

    if ( pageSize >= numLines() ) { // quick fix to handle small texts
	newTopCell = topCell();
    }
    if ( !curXPos )
	curXPos = mapToView( cursorX, cursorY );
    int oldY = cursorY;

    if ( mark && !hasMarkedText() ) {
	markAnchorX    = cursorX;
	markAnchorY    = cursorY;
    }
    if ( newTopCell != topCell() ) {
	cursorY = newTopCell + delta;
	cursorX = mapFromView( curXPos, cursorY );
	if ( mark )
	    newMark( cursorX, cursorY, FALSE );
	setTopCell( newTopCell );
    } else { // just move the cursor
	cursorY = QMIN( cursorY + pageSize, numLines() - 1);
	cursorX = mapFromView( curXPos, cursorY );
	if ( mark )
	    newMark( cursorX, cursorY, FALSE );
	makeVisible();
    }
    if ( oldAuto )
	if ( mark ) {
	    setAutoUpdate( TRUE );
	    repaintDelayed( FALSE );
	} else {
	    updateCell( oldY, 0, FALSE );
	}
    if ( !mark )
	turnMarkOff();
}


/*!
  Moves the cursor one page up.  If \a mark is TRUE, the text
  is marked.
*/

void QMultiLineEditNew::pageUp( bool mark )
{
    bool oldAuto = autoUpdate();
    if ( mark )
	setAutoUpdate( FALSE );
    if ( partiallyInvisible( cursorY ) )
	cursorY = topCell();
    int delta = cursorY - topCell();
    int pageSize = viewHeight() / cellHeight();
    bool partial = delta == pageSize && viewHeight() != pageSize * cellHeight();
    int newTopCell = QMAX( topCell() - pageSize, 0 );
    if ( pageSize > numLines() ) { // quick fix to handle small texts
	newTopCell = 0;
	delta = 0;
    }
    if ( mark && !hasMarkedText() ) {
	markAnchorX    = cursorX;
	markAnchorY    = cursorY;
    }
    if ( !curXPos )
	curXPos = mapToView( cursorX, cursorY );
    int oldY = cursorY;
    if ( newTopCell != topCell() ) {
	cursorY = QMIN( newTopCell + delta, numLines() - 1 );
	if ( partial )
	    cursorY--;
	cursorX = mapFromView( curXPos, cursorY );
	if ( mark )
	    newMark( cursorX, cursorY, FALSE );
	setTopCell( newTopCell );
    } else { // just move the cursor
	cursorY = QMAX( cursorY - pageSize, 0 );
	cursorX = mapFromView( curXPos, cursorY );
	if ( mark )
	    newMark( cursorX, cursorY, FALSE );
    }
    if ( oldAuto )
	if ( mark ) {
	    setAutoUpdate( TRUE );
	    repaintDelayed( FALSE );
	} else {
	    updateCell( oldY, 0, FALSE );
	}
    if ( !mark )
	turnMarkOff();
}


/*!
  Inserts \a txt at line number \a line, after character number \a col
  in the line.
  If \a txt contains newline characters, new lines are inserted.

  The cursor position is adjusted. If the insertion position is equal to
  the cursor position, the cursor is placed after the end of the new text.

 */

void QMultiLineEditNew::insertAt( const QString &txt, int line, int col, bool mark )
{
    dummy = FALSE;
    killTimer( blinkTimer );
    cursorOn = TRUE;
    int oldw = contentsRect().width();

    line = QMAX( QMIN( line, numLines() - 1), 0 );
    col = QMAX( QMIN( col,  lineLength( line )), 0 );

    QString itxt = txt;
    QMultiLineEditNewRow  *row = contents->at( line );
    if ( d->maxlen >= 0 && length() + int(txt.length()) > d->maxlen )
	itxt.truncate( d->maxlen - length() );

    row->s.insert( uint(col), itxt );

    if ( mark ) {
	markAnchorX = col;
	markAnchorY = line;
    }
    if ( cursorX == col && cursorY == line ) {
	cursorX += itxt.length();
    }
    QFontMetrics fm( font() );
    if ( !WORD_WRAP || ( col == 0 && itxt.contains('\n') == int(itxt.length())) )
	wrapLine( line, 0 );
    else if ( WORD_WRAP && itxt.find('\n')<0 && itxt.find('\t')<0
	      && (
		  ( DYNAMIC_WRAP && fm.width( itxt ) + row->w < contentsRect().width() -  2*d->lr_marg )
		  ||
		  ( FIXED_WIDTH_WRAP && ( d->wrapcol < 0 || fm.width( itxt ) + row->w < d->wrapcol ) )
		  ||
		  ( FIXED_COLUMN_WRAP && ( d->wrapcol < 0 || int(row->s.length()) < d->wrapcol ) )
		  )
	      && ( itxt.find(' ') < 0 || row->s.find(' ') >= 0 && row->s.find(' ') < col ) ){
	row->w = textWidth( row->s );
	setWidth( QMAX( maxLineWidth(), row->w) );
	updateCell( line, 0, FALSE );
    }
    else {
	if ( line > 0 && !contents->at( line-1)->newline )
	    rebreakParagraph( line-1 );
	else
	    rebreakParagraph( line );
    }
    if ( mark )
	newMark( cursorX, cursorY, FALSE );

    textDirty = TRUE;
    d->edited = TRUE;
    makeVisible();
    blinkTimer = startTimer(  QApplication::cursorFlashTime() / 2  );

    if ( autoUpdate() && DYNAMIC_WRAP && oldw != contentsRect().width() ) {
	setAutoUpdate( FALSE );
	rebreakAll();
	setAutoUpdate( TRUE );
	repaintDelayed( FALSE );
    }

}


/*!
  Inserts \a txt at line number \a line. If \a line is less than zero,
  or larger than the number of rows, the new text is put at the end.
  If \a txt contains newline characters, several lines are inserted.

  The cursor position is not changed.
*/

void QMultiLineEditNew::insertLine( const QString &txt, int line )
{
    QString s = txt;
    int oldXPos = cursorX;
    int oldYPos = cursorY;
    if ( line < 0 || line >= int( contents->count() ) ) {
	if ( !dummy )
	    s.prepend('\n');
	insertAt( s, numLines()-1, textLine( numLines()-1).length() );
    }
    else {
	s.append('\n');
	insertAt( s, line, 0 );
    }
    cursorX = oldXPos;
    cursorY = oldYPos;
}

/*!
  Deletes the line at line number \a line. If \a
  line is less than zero, or larger than the number of lines,
  no line is deleted.
*/

void QMultiLineEditNew::removeLine( int line )
{
    if ( line >= numLines()  )
	return;
    if ( cursorY >= line && cursorY > 0 )
	cursorY--;
    bool updt = autoUpdate() && rowIsVisible( line );
    QMultiLineEditNewRow* r = contents->at( line );
    ASSERT( r );
    bool recalc = r->w == maxLineWidth();
    contents->remove( line );
    if ( contents->count() == 0 ) {
	contents->append(  new QMultiLineEditNewRow("", 0) );
	dummy = TRUE;
    }
    setNumRows( contents->count() );
    if ( recalc )
	updateCellWidth();
    makeVisible();
    if (updt)
	repaintDelayed( FALSE );
    textDirty = TRUE;
    d->edited = TRUE;
}

/*!
  Inserts \a s at the current cursor position.
*/
void QMultiLineEditNew::insert( const QString& s )
{
    insert( s, FALSE );
}

/*!
  Inserts \a c at the current cursor position.
  (this function is provided for backward compatibility -
  it simply calls insert()).
*/
void QMultiLineEditNew::insertChar( QChar c )
{
    insert(c);
}

/*!
  Inserts \a c at the current cursor position.
*/

void QMultiLineEditNew::insert( const QString& str, bool mark )
{
    dummy = FALSE;
    bool wasMarkedText = hasMarkedText();
    if ( wasMarkedText )
	del();					// ## Will flicker
    QString *s = getString( cursorY );
    if ( cursorX > (int)s->length() )
	cursorX = s->length();
    else if ( overWrite && !wasMarkedText && cursorX < (int)s->length() )
	del();                                 // ## Will flicker
    insertAt(str, cursorY, cursorX, mark );
    makeVisible();
}

/*!
  Makes a line break at the current cursor position.
*/

void QMultiLineEditNew::newLine()
{
    insert("\n");
}

/*!
  Deletes text from the current cursor position to the end of the line.
*/

void QMultiLineEditNew::killLine()
{
    QMultiLineEditNewRow* r = contents->at( cursorY );
    if ( cursorX == (int)r->s.length() ) {
	del();
	return;
    } else {
	bool recalc = r->w == maxLineWidth();
	r->s.remove( cursorX, r->s.length() );
	r->w = textWidth( r->s );
	updateCell( cursorY, 0, FALSE );
	if ( recalc )
	    updateCellWidth();
	textDirty = TRUE;
	d->edited = TRUE;
    }
    curXPos  = 0;
    makeVisible();
    turnMarkOff();
}


/*!
  Moves the cursor one character to the left. If \a mark is TRUE, the text
  is marked. If \a wrap is TRUE, the cursor moves to the end of the
  previous line  if it is placed at the beginning of the current line.

  \sa cursorRight() cursorUp() cursorDown()
*/

void QMultiLineEditNew::cursorLeft( bool mark, bool wrap )
{
    cursorLeft(mark,!mark,wrap);
}
void QMultiLineEditNew::cursorLeft( bool mark, bool clear_mark, bool wrap )
{
    if ( cursorX != 0 || cursorY != 0 && wrap ) {
	if ( mark && !hasMarkedText() ) {
	    markAnchorX    = cursorX;
	    markAnchorY    = cursorY;
	}
	killTimer( blinkTimer );
	int ll = lineLength( cursorY );
	if ( cursorX > ll )
	    cursorX = ll;
	cursorOn = TRUE;
	cursorX--;
	if ( cursorX < 0 ) {
	    int oldY = cursorY;
	    if ( cursorY > 0 ) {
		cursorY--;
		cursorX = lineLength( cursorY );
	    } else {
		cursorY = 0; //### ?
		cursorX = 0;
	    }
	    updateCell( oldY, 0, FALSE );
	}
	if ( mark )
	    newMark( cursorX, cursorY, FALSE );
	blinkTimer = startTimer( QApplication::cursorFlashTime() / 2 );
	updateCell( cursorY, 0, FALSE );
    }
    curXPos  = 0;
    makeVisible();
    if ( clear_mark )
	turnMarkOff();
}

/*!
  Moves the cursor one character to the right.  If \a mark is TRUE, the text
  is marked. If \a wrap is TRUE, the cursor moves to the beginning of the next
  line if it is placed at the end of the current line.
  \sa cursorLeft() cursorUp() cursorDown()
*/

void QMultiLineEditNew::cursorRight( bool mark, bool wrap )
{
    cursorRight(mark,!mark,wrap);
}
void QMultiLineEditNew::cursorRight( bool mark, bool clear_mark, bool wrap )
{
    int strl = lineLength( cursorY );

    if ( cursorX < strl || cursorY < (int)contents->count() - 1 && wrap ) {
	if ( mark && !hasMarkedText() ) {
	    markAnchorX    = cursorX;
	    markAnchorY    = cursorY;
	}
	killTimer( blinkTimer );
	cursorOn = TRUE;
	cursorX++;
	if ( cursorX > strl ) {
	    int oldY = cursorY;
	    if ( cursorY < (int) contents->count() - 1 ) {
		cursorY++;
		cursorX = 0;
	    } else {
		cursorX = lineLength( cursorY );
	    }
	    updateCell( oldY, 0, FALSE );
	}
	if ( mark )
	    newMark( cursorX, cursorY, FALSE );
	updateCell( cursorY, 0, FALSE );
	blinkTimer = startTimer(  QApplication::cursorFlashTime() / 2  );
    }
    curXPos  = 0;
    makeVisible();
    if ( clear_mark )
	turnMarkOff();
}

/*!
  Moves the cursor up one line.  If \a mark is TRUE, the text
  is marked.
  \sa cursorDown() cursorLeft() cursorRight()
*/

void QMultiLineEditNew::cursorUp( bool mark )
{
    cursorUp(mark,!mark);
}
void QMultiLineEditNew::cursorUp( bool mark, bool clear_mark )
{
    if ( cursorY != 0 ) {
	if ( mark && !hasMarkedText() ) {
	    markAnchorX    = cursorX;
	    markAnchorY    = cursorY;
	}
	if ( !curXPos )
	    curXPos = mapToView( cursorX, cursorY );
	int oldY = cursorY;
	killTimer( blinkTimer );
	cursorOn = TRUE;
	cursorY--;
	if ( cursorY < 0 ) {
	    cursorY = 0;
	}
	cursorX = mapFromView( curXPos, cursorY );
	if ( mark )
	    newMark( cursorX, cursorY, FALSE );
	updateCell( oldY, 0, FALSE );
	updateCell( cursorY, 0, FALSE );
	blinkTimer = startTimer( QApplication::cursorFlashTime() / 2 );
    }
    makeVisible();
    if ( clear_mark )
	turnMarkOff();
}

/*!
  Moves the cursor one line down.  If \a mark is TRUE, the text
  is marked.
  \sa cursorUp() cursorLeft() cursorRight()
*/

void QMultiLineEditNew::cursorDown( bool mark )
{
    cursorDown(mark,!mark);
}
void QMultiLineEditNew::cursorDown( bool mark, bool clear_mark )
{
    int lastLin = contents->count() - 1;
    if ( cursorY != lastLin ) {
	if ( mark && !hasMarkedText() ) {
	    markAnchorX    = cursorX;
	    markAnchorY    = cursorY;
	}
	if ( !curXPos )
	    curXPos = mapToView( cursorX, cursorY );
	int oldY = cursorY;
	killTimer( blinkTimer );
	cursorOn = TRUE;
	cursorY++;
	if ( cursorY > lastLin ) {
	    cursorY = lastLin;
	}
	cursorX = mapFromView( curXPos, cursorY );
	if ( mark )
	    newMark( cursorX, cursorY, FALSE );
	updateCell( oldY, 0, FALSE );
	updateCell( cursorY, 0, FALSE );
	blinkTimer = startTimer(  QApplication::cursorFlashTime() / 2 );
    }
    makeVisible();
    if ( clear_mark )
	turnMarkOff();
}

/*!
  Turns off marked text
*/
void QMultiLineEditNew::turnMarkOff()
{
    if ( markIsOn ) {
	markIsOn = FALSE;
	repaintDelayed( FALSE );
    }
}




/*!
  Deletes the character on the left side of the text cursor and moves
  the cursor one position to the left. If a text has been marked by
  the user (e.g. by clicking and dragging) the cursor is put at the
  beginning of the marked text and the marked text is removed.
  \sa del()
*/

void QMultiLineEditNew::backspace()
{
    if ( hasMarkedText() ) {
	del();
    } else {
	if ( !atBeginning() ) {
	    cursorLeft( FALSE );
	    del();
	}
    }
    makeVisible();
}

/*!
  Deletes the character on the right side of the text cursor. If a
  text has been marked by the user (e.g. by clicking and dragging) the
  cursor is put at the beginning of the marked text and the marked
  text is removed.  \sa backspace()
*/

void QMultiLineEditNew::del()
{
    int markBeginX, markBeginY;
    int markEndX, markEndY;
    QRect oldContents = contentsRect();
    if ( getMarkedRegion( &markBeginY, &markBeginX, &markEndY, &markEndX ) ) {
	markIsOn    = FALSE;
	textDirty = TRUE;
	d->edited = TRUE;
	if ( markBeginY == markEndY ) { //just one line
	    QMultiLineEditNewRow *r = contents->at( markBeginY );
	    ASSERT(r);
	    bool recalc = r->w == maxLineWidth();
	    r->s.remove( markBeginX, markEndX - markBeginX );
	    r->w = textWidth( r->s );
	    cursorX  = markBeginX;
	    cursorY  = markBeginY;
	    if (autoUpdate() )
		updateCell( cursorY, 0, FALSE );
	    if ( recalc )
		updateCellWidth();
	} else { //multiline
	    bool oldAuto = autoUpdate();
	    setAutoUpdate( FALSE );
	    ASSERT( markBeginY >= 0);
	    ASSERT( markEndY < (int)contents->count() );

	    QMultiLineEditNewRow *firstR, *lastR;
	    firstR = contents->at( markBeginY );
	    lastR  = contents->at( markEndY );
	    ASSERT( firstR != lastR );
	    firstR->s.remove( markBeginX, firstR->s.length() - markBeginX  );
	    lastR->s.remove( 0, markEndX  );
	    firstR->s.append( lastR->s );  // lastS will be removed in loop below
	    firstR->w = textWidth( firstR->s );

	    for( int i = markBeginY + 1 ; i <= markEndY ; i++ )
		contents->remove( markBeginY + 1 );
	
	    if ( contents->isEmpty() )
		insertLine( QString::fromLatin1(""), -1 );

	    cursorX  = markBeginX;
	    cursorY  = markBeginY;
	    curXPos  = 0;

	    setNumRows( contents->count() );
	    updateCellWidth();
	    setAutoUpdate( oldAuto );
	    if ( autoUpdate() )
		repaintDelayed( FALSE );
	}
	markAnchorY = markDragY = cursorY;
	markAnchorX = markDragX = cursorX;
    } else {
	if ( !atEnd() ) {
	    textDirty = TRUE;
	    d->edited = TRUE;
	    QMultiLineEditNewRow *r = contents->at( cursorY );
	    if ( cursorX == (int) r->s.length() ) { // remove newline
		QMultiLineEditNewRow* other = contents->at( cursorY + 1 );
		if ( ! r->newline && cursorX )
		    r->s.truncate( r->s.length()-1 );
		bool needBreak = !r->s.isEmpty();
		r->s += other->s;
		r->newline =  other->newline;
		contents->remove( cursorY + 1 );
		if ( needBreak )
		    rebreakParagraph( cursorY, 1 );
		else
		    wrapLine( cursorY, 1 );
	    } else {
		bool recalc = r->w == maxLineWidth();
		r->s.remove( cursorX, 1 );
		rebreakParagraph( cursorY );
		if ( recalc )
		    updateCellWidth();
	    }
	}
    }
    if ( DYNAMIC_WRAP && oldContents != contentsRect() ) {
	if ( oldContents.width() != contentsRect().width() ) {
	    bool oldAuto = autoUpdate();
	    setAutoUpdate( FALSE );
	    rebreakAll();
	    setAutoUpdate( oldAuto );
	}
	if ( autoUpdate() )
	    repaintDelayed( FALSE );
    }
    curXPos  = 0;
    makeVisible();
}

/*!
  Moves the text cursor to the left end of the line. If \a mark is
  TRUE, text is marked towards the first position. If it is FALSE and
  the cursor is moved, all marked text is unmarked.

  \sa end()
*/

void QMultiLineEditNew::home( bool mark )
{
    if ( cursorX != 0 ) {
	if ( mark && !hasMarkedText() ) {
	    markAnchorX    = cursorX;
	    markAnchorY    = cursorY;
	}
	killTimer( blinkTimer );
	cursorX = 0;
	cursorOn = TRUE;
	if ( mark )
	    newMark( cursorX, cursorY, FALSE );
	updateCell( cursorY, 0, FALSE );
	blinkTimer = startTimer(  QApplication::cursorFlashTime() / 2 );
    }
    curXPos  = 0;
    if ( !mark )
	turnMarkOff();
    makeVisible();
}

/*!
  Moves the text cursor to the right end of the line. If mark is TRUE
  text is marked towards the last position.  If it is FALSE and the
  cursor is moved, all marked text is unmarked.

  \sa home()
*/

void QMultiLineEditNew::end( bool mark )
{
    int tlen = lineLength( cursorY );
    if ( cursorX != tlen ) {
	if ( mark && !hasMarkedText() ) {
	    markAnchorX    = cursorX;
	    markAnchorY    = cursorY;
	}
	killTimer( blinkTimer );
	cursorX = tlen;
	cursorOn  = TRUE;
	if ( mark )
	    newMark( cursorX, cursorY, FALSE );
	blinkTimer = startTimer(  QApplication::cursorFlashTime() / 2  );
	updateCell( cursorY, 0, FALSE );
    }
    curXPos  = 0;
    makeVisible();
    if ( !mark )
	turnMarkOff();
}

/*!
  Handles mouse press events.
*/

void QMultiLineEditNew::mousePressEvent( QMouseEvent *m )
{
    stopAutoScroll();

    if ( m->button() != MidButton && m->button() != LeftButton)
	return;

    int newX, newY;
    pixelPosToCursorPos( m->pos(), &newX, &newY );
    if (
	inMark(newX, newY)		// Click on highlighted text
	&& echoMode() == Normal		// No DnD of passwords, etc.
	&& m->pos().y() < totalHeight() // Click past the end is not dragging
    )
    {
	// The user might be trying to drag
	d->dnd_primed = TRUE;
	d->dnd_timer = startTimer( 250 );
    } else {
	wordMark = FALSE;
	dragMarking    = TRUE;
	setCursorPixelPosition(m->pos());
    }
}

void QMultiLineEditNew::pixelPosToCursorPos(QPoint p, int* x, int* y) const
{
    *y = findRow( p.y() );
    if ( *y < 0 ) {
	if ( p.y() < lineWidth() ) {
	    *y = topCell();
	} else {
	    *y = lastRowVisible();
	    p.setX(cellWidth());
	}
    }
    *y = QMIN( (int)contents->count() - 1, *y );
    QFontMetrics fm( font() );
    *x = xPosToCursorPos( stringShown( *y ), fm,
			       p.x() - d->lr_marg + xOffset(),
			       cellWidth() - 2 * d->lr_marg, d->align );
}

void QMultiLineEditNew::setCursorPixelPosition(QPoint p, bool clear_mark)
{
    int newY;
    pixelPosToCursorPos( p, &cursorX, &newY );
    curXPos        = 0;
    if ( clear_mark ) {
	markAnchorX    = cursorX;
	markAnchorY    = newY;
	bool markWasOn = markIsOn;
	markIsOn       = FALSE;
	if ( markWasOn ) {
	    cursorY = newY;
	    repaintDelayed( FALSE );
	    d->isHandlingEvent = FALSE;
	    return;
	}	
    }
    if ( cursorY != newY ) {
	int oldY = cursorY;
	cursorY = newY;
	updateCell( oldY, 0, FALSE );
    }
    updateCell( cursorY, 0, FALSE );		// ###
}

void QMultiLineEditNew::startAutoScroll()
{
    if ( !dragScrolling ) {
	d->scrollTime = initialScrollTime;
	d->scrollAccel = initialScrollAccel;
	scrollTimer = startTimer( d->scrollTime );
	dragScrolling = TRUE;
    }
}

void QMultiLineEditNew::stopAutoScroll()
{
    if ( dragScrolling ) {
	killTimer( scrollTimer );
	scrollTimer = 0;
	dragScrolling = FALSE;
    }
}

/*!
  Handles mouse move events.
*/
void QMultiLineEditNew::mouseMoveEvent( QMouseEvent *e )
{
    if ( d->dnd_primed ) {
	doDrag();
	return;
    }
    if ( !dragMarking )
	return;
    if ( rect().contains( e->pos() ) ) {
	stopAutoScroll();
    } else if ( !dragScrolling ) {
	startAutoScroll();
    }

    int newX, newY;
    pixelPosToCursorPos(e->pos(), &newX, &newY);

    if ( wordMark ) {
	QString s = stringShown( newY );
	int lim = s.length();
	if ( newX >= 0 && newX < lim ) {
	    int i = newX;
	    int startclass = charClass(s.at(i));
	    if ( markAnchorY < markDragY ||
		 ( markAnchorY == markDragY && markAnchorX < markDragX ) ) {
		// going right
		while ( i < lim && charClass(s.at(i)) == startclass )
		    i++;
	    } else {
		// going left
		while ( i >= 0 && charClass(s.at(i)) == startclass )
		    i--;
		i++;
	    }
	    newX = i;
	}
    }

    if ( markDragX == newX && markDragY == newY )
	return;
    int oldY = markDragY;
    newMark( newX, newY, FALSE );
    for ( int i = QMIN(oldY,newY); i <= QMAX(oldY,newY); i++ )
	updateCell( i, 0, FALSE );
}


/*!
  Handles mouse release events.
*/
void QMultiLineEditNew::mouseReleaseEvent( QMouseEvent *e )
{
    stopAutoScroll();
    if ( d->dnd_timer ) {
	killTimer( d->dnd_timer );
	d->dnd_timer = 0;
	d->dnd_primed = FALSE;
	setCursorPixelPosition(e->pos());
    }
    wordMark = FALSE;
    dragMarking   = FALSE;
    textDirty = FALSE;
    d->isHandlingEvent = TRUE;
    if ( markAnchorY == markDragY && markAnchorX == markDragX )
	markIsOn = FALSE;
#if defined(_WS_X11_)
    else if ( echoMode() == Normal )
	copy();
#else
    else if ( style() == MotifStyle && echoMode() == Normal )
	copy();
#endif

    if ( e->button() == MidButton && !readOnly ) {
#if defined(_WS_X11_)
	paste();		// Will repaint the cursor line.
#else
	if ( style() == MotifStyle )
	    paste();
#endif
    }
    d->isHandlingEvent = FALSE;

    if ( !readOnly && textDirty )
	emit textChanged();
}


/*!
  Handles double click events.
*/

void QMultiLineEditNew::mouseDoubleClickEvent( QMouseEvent *m )
{
    if ( m->button() == LeftButton ) {
	dragMarking    = TRUE;
	markWord( cursorX, cursorY );
	wordMark = TRUE;
	updateCell( cursorY, 0, FALSE );
    }
}

/*!
  Handles drag motion events, accepting text and positioning the cursor.
*/
void QMultiLineEditNew::dragMoveEvent( QDragMoveEvent* event )
{
    if ( readOnly ) return;
    event->accept( QTextDrag::canDecode(event) );
    d->dnd_forcecursor = TRUE;
    setCursorPixelPosition(event->pos(), FALSE);
    d->dnd_forcecursor = FALSE;
    QRect inside_margin(scroll_margin, scroll_margin,
		width()-scroll_margin*2, height()-scroll_margin*2);
    if ( !inside_margin.contains(event->pos()) ) {
	startAutoScroll();
    }
    if ( event->source() == this && event->action() == QDropEvent::Move )
	event->acceptAction();
}

/*!
  Handles drag leave events, cancelling any auto-scrolling.
*/
void QMultiLineEditNew::dragLeaveEvent( QDragLeaveEvent* )
{
    stopAutoScroll();
}

/*!
  Handles drop events, pasting text.
*/
void QMultiLineEditNew::dropEvent( QDropEvent* event )
{
    if ( readOnly ) return;
    QString text;
    if ( QTextDrag::decode(event, text) ) {
	if ( event->source() == this && event->action() == QDropEvent::Move )
	{
	    event->acceptAction();
	    // Careful not to tread on my own feet
	    int newX, newY;
	    pixelPosToCursorPos( event->pos(), &newX, &newY );
	    if ( afterMark( newX, newY ) ) {
		// The tricky case
		int x1, y1, x2, y2;
		getMarkedRegion( &y1, &x1, &y2, &x2 );
		if ( newY == y2 ) {
		    newY = y1;
		    newX = x1 + newX - x2;
		} else {
		    newY -= y2 - y1;
		}
		del();
		setCursorPosition(newY, newX);
		insert(text, TRUE);
	    } else if ( beforeMark( newX, newY ) ) {
		// Easy
		del();
		setCursorPosition(newY, newX);
		insert(text, TRUE);
	    } else {
		// Do nothing.
	    }
	} else {
	    setCursorPixelPosition(event->pos());
	    insert(text, TRUE);
	}
	// TODO: mark dropped text (probably in 3 cases above)
    }
}


/*!
  Returns TRUE if line \a line is invisible or partially invisible.
*/

bool QMultiLineEditNew::partiallyInvisible( int line )
{
    int y;
    if ( !rowYPos( line, &y ) )
	return TRUE;
    if ( y < 0 ) {
	//debug( "line %d occluded at top", line );
	return TRUE;
    } else if ( y + cellHeight() - 2 > viewHeight() ) {
	//debug( "line %d occluded at bottom", line );
	return TRUE;
    }
    return FALSE;
}

/*!
  Scrolls such that the cursor is visible
*/

void QMultiLineEditNew::makeVisible()
{
    if ( !autoUpdate() )
	return;

    if ( partiallyInvisible( cursorY ) ) {
	if ( cursorY >= lastRowVisible() )
	    setBottomCell( cursorY );
	else
	    setTopCell( cursorY );
    }
    int xPos = mapToView( cursorX, cursorY );
    if ( xPos < xOffset() ) {
	int of = xPos - 10; //###
	setXOffset( of );
    } else if ( xPos > xOffset() + viewWidth() ) {
	int of = xPos - viewWidth() + 10; //###
	setXOffset( of );
    }
}

/*!
  Computes the character position in line \a line which corresponds
  to pixel \a xPos
*/

int QMultiLineEditNew::mapFromView( int xPos, int line )
{
    QString s = stringShown( line );
    if ( !s )
	return 0;
    QFontMetrics fm( font() );
    int index = xPosToCursorPos( s, fm,
				 xPos - d->lr_marg,
				 cellWidth() - 2 * d->lr_marg, d->align );
    return index;
}

/*!
  Computes the pixel position in line \a line which corresponds to
  character position \a xIndex
*/

int QMultiLineEditNew::mapToView( int xIndex, int line )
{
    QString s = stringShown( line );
    //ASSERT( !!s );
    xIndex = QMIN( (int)s.length(), xIndex );
    QFontMetrics fm( font() );
    int wcell = cellWidth() - 2 * d->lr_marg;
    int wrow = contents->at( line )->w;
    int w = textWidthWithTabs( fm, s, 0, xIndex, d->align ) - 1;
    if ( d->align == Qt::AlignCenter )
	w += (wcell - wrow) / 2;
    else if ( d->align == Qt::AlignRight )
	w += wcell - wrow;
    return d->lr_marg + w;
}

/*!
  Traverses the list and finds an item with the maximum width, and
  updates the internal list box structures accordingly.
*/

void QMultiLineEditNew::updateCellWidth()
{
    QMultiLineEditNewRow* r = contents->first();
    int maxW = 0;
    int w;
    switch ( d->echomode ) {
      case Normal:
	while ( r ) {
	    w = r->w;
	    if ( w > maxW )
		maxW = w;
	    r = contents->next();
	}
	break;
      case Password: {
	    uint l = 0;
	    while ( r ) {
		l = QMAX(l,  r->s.length() );
		r = contents->next();
	    }
	    QString t;
	    t.fill(QChar('*'), l);
	    maxW = textWidth(t);
	}
	break;
      case NoEcho:
	maxW = textWidth(QString::fromLatin1(""));
    }
    setWidth( maxW );
}


/*!
  Sets the bottommost visible line to \a line.
*/

void QMultiLineEditNew::setBottomCell( int line )
{
    //debug( "setBottomCell %d", line );
    int rowY = cellHeight() * line;
    int newYPos = rowY +  cellHeight() - viewHeight();
    setYOffset( QMAX( newYPos, 0 ) );
}

/*!
  Copies text from the clipboard onto the current cursor position.
  Any marked text is first deleted.
*/
void QMultiLineEditNew::paste()
{
    //debug( "paste" );
    if ( hasMarkedText() )
	del();
    QString t = QApplication::clipboard()->text();
    if ( !t.isEmpty() ) {
	if ( hasMarkedText() )
	    turnMarkOff();

#if defined(_OS_WIN32_)
	// Need to convert CRLF to NL
	QRegExp crlf("\\r\\n");
	t.replace( crlf, "\n" );
#endif

	for (int i=0; (uint)i<t.length(); i++) {
	    if ( t[i] < ' ' && t[i] != '\n' && t[i] != '\t' )
		t[i] = ' ';
	}
	insertAt( t, cursorY, cursorX );
	markIsOn = FALSE;
	curXPos  = 0;
	makeVisible();
    }
    if ( textDirty && !d->isHandlingEvent )
	emit textChanged();
}


/*!
  Removes all text.
*/

void QMultiLineEditNew::clear()
{
    contents->clear();
    cursorX = cursorY = 0;
    contents->append(  new QMultiLineEditNewRow("", 0) );
    dummy = TRUE;
    markIsOn = FALSE;
    setWidth( 1 );
    if ( autoUpdate() )
	repaintDelayed();
    if ( !d->isHandlingEvent ) //# && not already empty
	emit textChanged();
}


/*!
  Reimplements QWidget::setFont() to update the list box line height.
*/

void QMultiLineEditNew::setFont( const QFont &font )
{
    QWidget::setFont( font );
    QFontMetrics fm( font );
    setCellHeight( fm.lineSpacing() );
    updateCellWidth();
}

/*!
  Sets a new marked text limit, does not repaint the widget.
*/

void QMultiLineEditNew::newMark( int posx, int posy, bool /*copy*/ )
{
    if ( markIsOn && markDragX == posx && markDragY == posy &&
	 cursorX   == posx && cursorY   == posy )
	return;
    markDragX  = posx;
    markDragY  = posy;
    cursorX    = posx;
    cursorY    = posy;
    markIsOn = ( markDragX != markAnchorX ||  markDragY != markAnchorY );
    if ( echoMode() == Normal )
	this->copy();
}

bool QMultiLineEditNew::beforeMark( int posx, int posy ) const
{
    int x1, y1, x2, y2;
    if ( !getMarkedRegion( &y1, &x1, &y2, &x2 ) )
	return FALSE;
    return
	(y1 > posy || y1 == posy && x1 > posx)
     && (y2 > posy || y2 == posy && x2 > posx);
}

bool QMultiLineEditNew::afterMark( int posx, int posy ) const
{
    int x1, y1, x2, y2;
    if ( !getMarkedRegion( &y1, &x1, &y2, &x2 ) )
	return FALSE;
    return
	(y1 < posy || y1 == posy && x1 < posx)
     && (y2 < posy || y2 == posy && x2 < posx);
}

bool QMultiLineEditNew::inMark( int posx, int posy ) const
{
    int x1, y1, x2, y2;
    if ( !getMarkedRegion( &y1, &x1, &y2, &x2 ) )
	return FALSE;
    return
	(y1 < posy || y1 == posy && x1 <= posx)
     && (y2 > posy || y2 == posy && x2 >= posx);
}

/*!
  Marks the word at character position \a posx, \a posy.
 */
void QMultiLineEditNew::markWord( int posx, int posy )
{
    QString& s = contents->at( posy )->s;
    int lim = s.length();
    int i = posx - 1;

    int startclass = i < 0 || i >= lim ? -1 : charClass( s[i] );

    while ( i >= 0 && charClass(s[i]) == startclass )
	i--;
    i++;
    markAnchorY = posy;
    markAnchorX = i;

    i = posx;
    while ( i < lim && charClass(s[i]) == startclass )
	i++;
    markDragX = i;
    markDragY = posy;
    markIsOn = ( markDragX != markAnchorX ||  markDragY != markAnchorY );
    if ( echoMode() == Normal )
	copy();
}

/*!
  This may become a protected virtual member in a future Qt.
  This implementation is an example of a useful classification
  that aids selection of common units like filenames and URLs.
*/
int QMultiLineEditNew::charClass( QChar ch )
{
    if ( !ch.isPrint() || ch.isSpace() ) return 1;
    else if ( ch.isLetter() || ch=='-' || ch=='+' || ch==':'
	    || ch=='.' || ch=='/' || ch=='\\'
	    || ch=='@' || ch=='$' || ch=='~' ) return 2;
    else return 3;
}

/*!
  Copies the marked text to the clipboard.  Will only copy
  if echoMode() is Normal.
*/

void QMultiLineEditNew::copy() const
{
    QString t = markedText();
    if ( !t.isEmpty() && echoMode() == Normal ) {
#if defined(_WS_X11_)
	disconnect( QApplication::clipboard(), SIGNAL(dataChanged()), this, 0);
#endif
#if defined(_OS_WIN32_)
	// Need to convert NL to CRLF
	QRegExp nl("\\n");
	t.replace( nl, "\r\n" );
#endif
	QApplication::clipboard()->setText( t );
#if defined(_WS_X11_)
	connect( QApplication::clipboard(), SIGNAL(dataChanged()),
		 this, SLOT(clipboardChanged()) );
#endif
    }
}

/* \obsolete

  Backward compatibility.
*/
void QMultiLineEditNew::copyText() const
{
    copy();
}


/*!
  Copies the selected text to the clipboard and deletes the selected text.
*/

void QMultiLineEditNew::cut()
{
    if ( hasMarkedText() ) {
	if ( echoMode() == Normal )
	    copy();
	del();
	if ( textDirty && !d->isHandlingEvent )
	    emit textChanged();
    }
}


/*!
  This private slot is activated when this line edit owns the clipboard and
  some other widget/application takes over the clipboard. (X11 only)
*/

void QMultiLineEditNew::clipboardChanged()
{
#if defined(_WS_X11_)
    disconnect( QApplication::clipboard(), SIGNAL(dataChanged()),
		this, SLOT(clipboardChanged()) );
    markIsOn = FALSE;
    repaintDelayed( FALSE );
#endif
}


 /*!
   Sets maxLineWidth() and maybe cellWidth() to \a w without updating the entire widget.
 */

 void QMultiLineEditNew::setWidth( int w )
 {
    if ( w ==d->maxLineWidth )
	return;
     bool u = autoUpdate();
     setAutoUpdate( FALSE );
     d->maxLineWidth = w;
     if ( d->align == AlignLeft )
	 setCellWidth( w );
     else
	 setCellWidth( QMAX( w, contentsRect().width() ) );
     setAutoUpdate( u );
     if ( autoUpdate() && d->align != AlignLeft )
	 repaintDelayed( FALSE );
 }


/*!
  Sets the cursor position to character number \a col in line number \a line.
  The parameters are adjusted to lie within the legal range.

  If \a mark is FALSE, the selection is cleared. otherwise it is extended

  \sa cursorPosition()
*/

void QMultiLineEditNew::setCursorPosition( int line, int col, bool mark )
{
    if ( mark && !hasMarkedText() ) {
	markAnchorX    = cursorX;
	markAnchorY    = cursorY;
    }
    int oldY = cursorY;
    cursorY = QMAX( QMIN( line, numLines() - 1), 0 );
    cursorX = QMAX( QMIN( col,  lineLength( cursorY )), 0 );
    curXPos = 0;
    makeVisible();
    updateCell( oldY, 0, FALSE );
    if ( mark )
	newMark( cursorX, cursorY, FALSE );
    else
	turnMarkOff();
}



/*! \obsolete

  Use getCursorPosition() instead.
*/

void QMultiLineEditNew::cursorPosition( int *line, int *col ) const
{
    getCursorPosition(line,col);
}


/*!
  Returns the current line and character
  position within that line, in the variables pointed to
  by \a line and \a col respectively.

  \sa setCursorPosition()
*/

void QMultiLineEditNew::getCursorPosition( int *line, int *col ) const
{
    if ( line )
	*line = cursorY;
    if ( col )
	*col = cursorX;
}


/*!
  Returns TRUE if the view updates itself automatically whenever it
  is changed in some way.

  \sa setAutoUpdate()
*/

bool QMultiLineEditNew::autoUpdate() const
{
    return QTableView::autoUpdate();
}


/*!
  Sets the auto-update option of multi-line editor to \e enable.

  If \e enable is TRUE (this is the default) then the editor updates
  itself automatically whenever it has changed in some way (generally,
  when text has been inserted or deleted).

  If \e enable is FALSE, the view does NOT repaint itself, or update
  its internal state variables itself when it is changed.  This can be
  useful to avoid flicker during large changes, and is singularly
  useless otherwise: Disable auto-update, do the changes, re-enable
  auto-update, and call repaint().

  \warning Do not leave the view in this state for a long time
  (i.e. between events ). If, for example, the user interacts with the
  view when auto-update is off, strange things can happen.

  Setting auto-update to TRUE does not repaint the view, you must call
  repaint() to do this (preferable repaint(FALSE) to avoid flicker).

  \sa autoUpdate() repaint()
*/

void QMultiLineEditNew::setAutoUpdate( bool enable )
{
    QTableView::setAutoUpdate( enable );
}

/*!
  Sets the fixed height of the QMultiLineEditNew so that \e lines text lines
  are visible given the current font.

  \sa setMaxLines(), setFixedHeight()
 */
void QMultiLineEditNew::setFixedVisibleLines( int lines )
{
    int ls = fontMetrics().lineSpacing();
    setFixedHeight( frameWidth()*2 + ls*lines );
    return;
}



/*!
  Returns the top center point where the cursor is drawn
*/

QPoint QMultiLineEditNew::cursorPoint() const
{
    QPoint cp( 0, 0 );

    QFontMetrics fm( font() );
    int col, row;
    col = row = 0;
    cursorPosition( &row, &col );
    QString line = textLine( row );
    ASSERT( line );
    cp.setX( d->lr_marg + textWidthWithTabs( fm, line, 0, col, d->align ) - 1 );
    cp.setY( (row * cellHeight()) + viewRect().y() );
    return cp;
}


/*!
  Specifies that this widget can use additional space, and that it can
  survive on less than sizeHint().
*/

QSizePolicy QMultiLineEditNew::sizePolicy() const
{
    if ( d->maxlines >= 0 && d->maxlines <= 6 ) {
	return QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
    } else {
	return QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    }
}


/*!
  Returns a size sufficient for a few lines of text, or any value set by
  setFixedVisibleLines().
*/
QSize QMultiLineEditNew::sizeHint() const
{
    int expected_lines;
    if ( d->maxlines >= 0 && d->maxlines <= 6 ) {
	expected_lines = d->maxlines;
    } else {
	expected_lines = 6;
    }
    QFontMetrics fm( font() );
    int h = fm.lineSpacing()*(expected_lines-1)+fm.height() + frameWidth()*2;
    int w = fm.width('x')*35;

    int maxh = maximumSize().height();
    if ( maxh < QWIDGETSIZE_MAX )
	h = maxh;

    return QSize( w, h );
}


/*!
  Returns a size sufficient for one character, and scroll bars.
*/

QSize QMultiLineEditNew::minimumSizeHint() const
{
    const int sbDim = 16; //###

     QFontMetrics fm( font() );
    int h = fm.lineSpacing() + frameWidth()*2;
    int w = fm.maxWidth();
    h += frameWidth();
    w += frameWidth();
    if ( testTableFlags(Tbl_hScrollBar|Tbl_autoHScrollBar) )
	w += sbDim;
    if ( testTableFlags(Tbl_vScrollBar|Tbl_autoVScrollBar) )
	h += sbDim;
    return QSize(w,h);
}



/*!
  Reimplemented for internal purposes
*/

void QMultiLineEditNew::resizeEvent( QResizeEvent *e )
{
    int oldw = contentsRect().width();
    QTableView::resizeEvent( e );
    if ( DYNAMIC_WRAP
	 && (e->oldSize().width() != width()
	     || oldw != contentsRect().width() ) ) {
	bool oldAuto = autoUpdate();
	setAutoUpdate( FALSE );
	rebreakAll();
	if ( oldw != contentsRect().width() )
	    rebreakAll();
	setAutoUpdate( oldAuto );
	if ( autoUpdate() )
	    repaintDelayed( FALSE );
    } else if ( d->align != AlignLeft ) {
	d->maxLineWidth = 0; // trigger update
	updateCellWidth();
    }
}

/*!
  Post a paint event
 */
void  QMultiLineEditNew::repaintDelayed( bool erase)
{
    QApplication::postEvent( this, new QPaintEvent( viewRect(), erase ) );

}

/*!
  Sets the alignment. Possible values are \c AlignLeft, \c AlignCenter
  and \c AlignRight.

  \sa alignment()
*/
void QMultiLineEditNew::setAlignment( int flags )
{
    if ( d->align != flags ) {
	d->align = flags;
	update();
    }
}

/*!
  Returns the alignment.

  \sa setAlignment()
*/
int QMultiLineEditNew::alignment() const
{
    return d->align;
}


/*!
  Not supported at this time.
*/
void QMultiLineEditNew::setValidator( const QValidator *v )
{
    d->val = v;
    // #### validate text now
}

/*!
  Not supported at this time.
*/
const QValidator * QMultiLineEditNew::validator() const
{
    return d->val;
}

/*!  Sets the edited flag of this line edit to \a on.  The edited flag
is never read by QMultiLineEditNew, but is changed to TRUE whenever the user
changes its contents.

This is useful e.g. for things that need to provide a default value,
but cannot find the default at once.  Just open the widget without the
best default and when the default is known, check the edited() return
value and set the line edit's contents if the user has not started
editing the line edit.  Another example is to detect whether the
contents need saving.

\sa edited()
*/
void QMultiLineEditNew::setEdited( bool e )
{
    d->edited = e;
}

/*!  Returns the edited flag of the line edit.  If this returns FALSE,
the contents has not been changed since the construction of the
QMultiLineEditNew (or the last call to setEdited( FALSE ), if any).  If
it returns true, the contents have been edited, or setEdited( TRUE )
has been called.

\sa setEdited()
*/
bool QMultiLineEditNew::edited() const
{
    return d->edited;
}

/*! \enum QMultiLineEditNew::EchoMode

  This enum type describes the ways in which QLineEdit can display its
  contents.  The currently defined values are: <ul>

  <li> \c Normal - display characters as they are entered.  This is
  the default.

  <li> \c NoEcho - do not display anything.

  <li> \c Password - display asterisks instead of the characters
  actually entered.

  </ul>

  \sa setEchoMode() echoMode() QLineEdit::EchoMode
*/


/*!
  Sets the echo mode to \a em.  The default is \c Normal.

  The display is updated according.

  \sa setEchoMode()
*/
void QMultiLineEditNew::setEchoMode( EchoMode em )
{
    if ( d->echomode != em ) {
	d->echomode = em;
	updateCellWidth();
	update();
    }
}

/*!
  Returns the currently set echo mode.

  \sa setEchoMode()
*/
QMultiLineEditNew::EchoMode QMultiLineEditNew::echoMode() const
{
    return d->echomode;
}


/*!
  Returns the string shown at line \a row, including
  processing of the echoMode().
*/

QString QMultiLineEditNew::stringShown(int row) const
{
    QString* s = getString(row);
    if ( !s ) return QString::null;
    switch ( d->echomode ) {
      case Normal:
	if (!*s) return QString::fromLatin1("");
	return *s;
      case Password:
	{
	    QString r;
	    r.fill(QChar('*'), (int)s->length());
	    if ( !r ) r = QString::fromLatin1("");
	    return r;
	}
      case NoEcho:
	return QString::fromLatin1("");
    }
    return QString::fromLatin1("");
}

/*!
  Sets the maximum text length  to \a m.  Use -1 for unlimited
  (the default).  Existing overlong text  will be truncated.

  \sa maxLength()
*/
void QMultiLineEditNew::setMaxLength(int m)
{
    d->maxlen = m;
}

/*!
  Returns the currently set text length limit, or -1 if there is
  no limit (this is the default).

  \sa setMaxLength()
*/
int QMultiLineEditNew::maxLength() const
{
    return d->maxlen;
}


/*!
  Returns the length of the current text.

  \sa setMaxLength()
 */
int QMultiLineEditNew::length() const
{
    int l = 0;
    for ( QMultiLineEditNewRow* r = contents->first(); r; r = contents->next() ) {
	l += r->s.length();
	if ( r->newline )
	    ++l;
    }
    return l;
}


/*!
  Sets the maximum length of lines to \a m.  Use -1 for unlimited
  (the default).  Existing long lines will be truncated.

  \sa maxLineLength()
*/
void QMultiLineEditNew::setMaxLineLength(int m)
{
    bool trunc = d->maxlinelen >= 0 && d->maxlinelen < m;
    d->maxlinelen = m;
    if ( trunc ) {
	QMultiLineEditNewRow* r = contents->first();
	while ( r ) {
	    r->s.truncate( m );
	    r = contents->next();
	}
	if ( cursorX > m ) cursorX = m;
	if ( markAnchorX > m ) markAnchorX = m;
	if ( markDragX > m ) markDragX = m;
	update();
	updateCellWidth();
    }
}

/*!
  Returns the currently set line length limit, or -1 if there is
  no limit (this is the default).

  \sa setMaxLineLength()
*/
int QMultiLineEditNew::maxLineLength() const
{
    return d->maxlinelen;
}

/*!
  Sets the maximum number of lines to \a m.  Use -1 for unlimited
  (the default).  Existing excess lines will be deleted.

  \sa maxLines()
*/
void QMultiLineEditNew::setMaxLines(int m)
{
    bool trunc = d->maxlines >= 0 && d->maxlines < m;
    d->maxlines = m;
    if ( trunc ) {
	if ( cursorY > m ) {
	    cursorX = 0;
	    cursorY = m;
	}
	if ( markAnchorY > m ) {
	    markAnchorX = 0;
	    markAnchorY = m;
	}
	if ( markDragY > m ) {
	    markDragX = 0;
	    markDragY = m;
	}
	while ( contents->remove(m) )
	    ;
	updateCellWidth();
	update();
    }
}

/*!
  Returns the currently set line limit, or -1 if there is
  no limit (the default).

  \sa setMaxLines()
*/
int QMultiLineEditNew::maxLines() const
{
    return d->maxlines;
}

/*!
  Sets the horizontal margin.

  \sa hMargin()
*/
void QMultiLineEditNew::setHMargin(int m)
{
    if ( m != d->lr_marg ) {
	d->lr_marg = m;
	updateCellWidth();
	update();
    }
}

/*!
  Returns the horizontal margin current set.  The default is 3.

  \sa setHMargin()
*/
int QMultiLineEditNew::hMargin() const
{
    return d->lr_marg;
}

/*!
  Not supported at this time.
*/
void QMultiLineEditNew::setSelection( int /*row_from*/, int /*col_from*/, int /*row_to*/, int /*col_to*/ )
{
    qFatal("Not implemented: setSelection");
}


/*!
  Moves the cursor one word to the right.  If \a mark is TRUE, the text
  is marked.
  \sa cursorWordBackward()
*/
void QMultiLineEditNew::cursorWordForward( bool mark )
{
    int x = cursorX;
    int y = cursorY;

    if ( x == lineLength( y ) || textLine(y).at(x).isSpace() ) {
	while ( x < lineLength( y ) && textLine(y).at(x).isSpace() )
	    ++x;
	if ( x == lineLength( y ) ) {
	    if ( y < (int)contents->count() - 1) {
		++y;
		x = 0;
		while ( x < lineLength( y ) && textLine(y).at(x).isSpace() )
		    ++x;
	    }
	}
    }
    else {
	while ( x < lineLength( y ) && !textLine(y).at(x).isSpace() )
	    ++x;
	int xspace = x;
	while ( xspace < lineLength( y ) && textLine(y).at(xspace).isSpace() )
	    ++xspace;
	if ( xspace <  lineLength( y ) )
	    x = xspace;
    }
    cursorOn = TRUE;
    int oldY = cursorY;
    setCursorPosition( y, x, mark );
    if ( oldY != cursorY )
	updateCell( oldY, 0, FALSE );
    updateCell( cursorY, 0, FALSE );
    blinkTimer = startTimer(  QApplication::cursorFlashTime() / 2  );
}

/*!
  Moves the cursor one word to the left.  If \a mark is TRUE, the text
  is marked.
  \sa cursorWordForward()
*/
void QMultiLineEditNew::cursorWordBackward( bool mark )
{
    int x = cursorX;
    int y = cursorY;

    while ( x > 0 && textLine(y).at(x-1).isSpace() )
	--x;

    if ( x == 0 ) {
	if ( y > 0 ) {
	    --y;
	    x = lineLength( y );
	    while ( x > 0  && textLine(y).at(x-1).isSpace() )
		--x;
	}
    }
    else {
	while ( x > 0  && !textLine(y).at(x-1).isSpace() )
	    --x;
    }
    cursorOn = TRUE;
    int oldY = cursorY;
    setCursorPosition( y, x, mark );
    if ( oldY != cursorY )
	updateCell( oldY, 0, FALSE );
    updateCell( cursorY, 0, FALSE );
    blinkTimer = startTimer(  QApplication::cursorFlashTime() / 2  );
}

#define DO_BREAK doBreak = TRUE; if ( lastSpace > a ) { \
    i = lastSpace; \
    linew = lastw; \
  } \
 else \
  i = QMAX( a, i-1 );

void QMultiLineEditNew::wrapLine( int line, int removed )
{
    QMultiLineEditNewRow* r = contents->at( line );
    int yPos;
    (void) rowYPos( line, &yPos );
    QFontMetrics fm( font() );
    int i  = 0;
    QString s = r->s;
    int a = 0;
    int l = line;
    int w = 0;
    int nlines = 0;
    int lastSpace = 0;
    bool doBreak = FALSE;
    int linew = 0;
    int lastw = 0;
    int tabDist = -1; // lazy eval
    while ( i < int(s.length()) ) {
	doBreak = FALSE;
	if ( s[i] == '\t' && d->align == Qt::AlignLeft ) {
	    if ( tabDist<0 )
		tabDist = tabStopDist(fm);
	    linew = ( linew/tabDist + 1 ) * tabDist;
	} else if ( s[i] != '\n' )
	    linew += fm.width( s[i] );
	if ( WORD_WRAP &&
	     ( BREAK_WITHIN_WORDS || lastSpace > a) && s[i] != '\n' ) {
	    if ( DYNAMIC_WRAP ) {
		if  (linew >= contentsRect().width() -  2*d->lr_marg) {
		    DO_BREAK
		}
	    } else if ( FIXED_COLUMN_WRAP ) {
		if ( d->wrapcol >= 0 && i-a >= d->wrapcol ) {
		    DO_BREAK
		}
	    } else if ( FIXED_WIDTH_WRAP ) {
		if ( d->wrapcol >= 0 && linew > d->wrapcol ) {
		    DO_BREAK
		}
	    }
	}
	if ( s[i] == '\n' || doBreak ) {
	    r->s = s.mid( a, i - a + (doBreak?1:0) );
	    r->w = linew - fm.minLeftBearing() + 2 * d->lr_marg;
	    if ( r->w > w )
		w = r->w;
	    if ( cursorY > l )
		++cursorY;
	    else if ( cursorY == line && cursorX >=a && cursorX <= i +  (doBreak?1:0)) {
		cursorY = l;
		cursorX -= a;
	    }
	    if ( markAnchorY > l )
		++markAnchorY;
	    else if ( markAnchorY == line && markAnchorX >=a && markAnchorX <= i +  (doBreak?1:0)) {
		markAnchorY = l;
		markAnchorX -= a;
	    }
	    a = i + 1;
	    lastSpace = a;
	    linew = 0;
	    bool oldnewline = r->newline;
	    r->newline = !doBreak;
	    r = new QMultiLineEditNewRow( QString::null, 0, oldnewline );
	    ++nlines;
	    contents->insert( l + 1, r );
	    ++l;
	}
	if ( s[i].isSpace() ) {
	    lastSpace = i;
	    lastw = linew;
	}
	if ( lastSpace <= a )
	    lastw = linew;
	
	++i;
    }
    if ( a < int(s.length()) ){
	r->s = s.mid( a, i - a  );
	r->w = linew - fm.minLeftBearing() + 2 * d->lr_marg;
    }
    if ( cursorY == line && cursorX >= a ) {
	cursorY = l;
	cursorX -= a;
    }
    if ( markAnchorY == line && markAnchorX >= a ) {
	markAnchorY = l;
	markAnchorX -= a;
    }
    if ( r->w > w )
	w = r->w;

    setWidth( QMAX( maxLineWidth(), w ) );
    bool oldAuto = autoUpdate();
    setAutoUpdate( FALSE );
    setNumRows( contents->count() );
    setAutoUpdate( oldAuto );

    yPos += (nlines+1)  * cellHeight();
    int sh = (nlines-removed)  * cellHeight();
    if ( sh && yPos >= contentsRect().top() && yPos < contentsRect().bottom() )
	QWidget::scroll( 0, sh,
			 QRect( contentsRect().left(), yPos,
				contentsRect().width(), contentsRect().bottom()
				- yPos ) );
    if ( autoUpdate() ) {
	for (int ul = 0; ul <= nlines; ++ul )
	    updateCell( line + ul, 0, FALSE );
    }
}

void QMultiLineEditNew::rebreakParagraph( int line, int removed )
{
    QMultiLineEditNewRow* r = contents->at( line );
    if ( WORD_WRAP ) {
	QMultiLineEditNewRow* other = 0;
	while (line < int(contents->count())-1 && !r->newline ) {
	    other = contents->at( line + 1 );
	    if ( cursorY > line ) {
		--cursorY;
		if ( cursorY == line ) {
		    cursorX += r->s.length();
		}
	    }
	    if ( markAnchorY > line ) {
		--markAnchorY;
		if ( markAnchorY == line ) {
		    markAnchorX += r->s.length();
		}
	    }
	    r->s.append( other->s );
	    r->newline = other->newline;
	    contents->remove( other );
	    ++removed;
	}
    }
    wrapLine( line, removed );
}

void QMultiLineEditNew::rebreakAll()
{
    if ( !WORD_WRAP )
	return;
    d->maxLineWidth = 0;
    for (int i = 0; i < int(contents->count()); ++i ) {
	rebreakParagraph( i );
	while ( i < int(contents->count() )
		&& !contents->at( i )->newline )
	    ++i;
    }
}

/*!
  Sets the word wrap mode. Possible values are \c NoWrap, \c
  DynamicWrap, \c FixedWidthWrap and \c FixedColumnWrap.

  Per default, wrapping keeps words intact. To allow breaking
  within words, the \c BreakWithinWords can be or'ed to one of the
  wrap modes.

  The default wrap mode is \c NoWrap.

  \sa wordWrap(), setWrapColumnOrWidth()
 */
void QMultiLineEditNew::setWordWrap( int mode )
{
    d->wrapmode = mode;
}

/*!
  Returns the current word wrap mode.

  \sa setWordWrap()
 */
int QMultiLineEditNew::wordWrap() const
{
    return d->wrapmode;
}

/*!
  Sets the wrap column or wrap width, depending on the wrapping mode.

  \sa setWordWrap()
 */
void QMultiLineEditNew::setWrapColumnOrWidth( int value )
{
    d->wrapcol = value;
}

/*!
  Returns the wrap column or wrap width, depending on the wrapping mode.

  \sa setWordWrap(), setWrapColumnOrWidth()
 */
int QMultiLineEditNew::wrapColumnOrWidth() const
{
    return d->wrapcol;
}


/*!
  Returns wether \a row is the last row in a paragraph.

  This function is only interesting in word wrap mode, otherwise its
  return value is always TRUE.

  \sa setWordWrap()
 */
bool QMultiLineEditNew::isEndOfParagraph( int row ) const
{
    return contents->at( row )->newline;
}

#include "moc/moc_qmultilineeditnew.h"
