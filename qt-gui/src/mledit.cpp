#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mledit.h"
#include <iostream.h>


bool myisprint(char c)
{
   return (c >= 32);
}


MLEditWrap::MLEditWrap (bool wordWrap, QWidget *parent=NULL, const char *name=NULL) : QMultiLineEdit(parent, name)
{
   word_wrap_is_set = wordWrap;
   fill_column_is_set = true;
   autoIndentMode = false;
   //fill_column_value = wrapSize;   // set in the resize event
}


void MLEditWrap::setWrapSize(int n)
{
   fill_column_value = n;
}


void MLEditWrap::append(const char *s)
{
  setAutoUpdate(false);
  for (unsigned long i = 0; i < strlen(s); i++)
  {
    if (s[i] == '\n') appendChar(' ');
    appendChar(s[i]);
  }
  setAutoUpdate(true);
  repaint();
}


void MLEditWrap::goToEnd(void)
{
   setCursorPosition(numLines() - 1, lineLength(numLines() - 1) - 1);
}


void MLEditWrap::appendChar(char c)
{
   // if we aren't at the end, then go there
   if (!atEnd()) goToEnd();
   
   if(fill_column_is_set && word_wrap_is_set )
   {

      // word break algorithm
      if(myisprint(c) || c == '\t' || c == '\n')
      {
         if (c == '\n')
         {
	         mynewLine();
	      }
	      else if (c == '\t')
         {
            QMultiLineEdit::insertChar((char)'\t');
	      }
	      else
         {
   	      QMultiLineEdit::insertChar(c);
   	   }

	      int templine,tempcol;
	      getCursorPosition(&templine,&tempcol);     

	      computePosition();
	      //setAutoUpdate(false);
	
	      bool did_break = format(par);
	      int num_of_rows = 0;

	      if (did_break)
         {
            QString newpar;

	         int cursorline = templine;  //Matthias
	         int cursorcol = 0; //Matthias
	         bool cursor_found = false; //Matthias
	         for( int k = 0; k < (int)par.count(); k++)
            {
	            QString tmp = par.at(k); //Matthias
	            { //Matthias
	               int f = tmp.find((char)1);
	               if (f>-1)
                  {
		               cursor_found = true;
		               cursorcol = f;
		               tmp.remove(f,1);
	               }
	            }
	            newpar += tmp; //Matthias
	            if (!cursor_found) cursorline++; //Matthias
	            if(k != (int)par.count() -1 )
	            newpar += '\n';
	         }
	         insertLine(newpar,templine);
	         newpar = "";
	         num_of_rows = par.count();
	         par.clear();
	         setCursorPosition(cursorline, cursorcol); //Matthias
	      }

	      //setAutoUpdate(true);

	      // Let's try to reduce flicker by updating only what we need to update
	      /*
         if (did_break)
         {
	         int y1  = -1;
	         int y2  = -1;

	         rowYPos(templine,&y1);
	         rowYPos(templine + num_of_rows ,&y2);

   	      if(y1 == -1) y1 = 0;

	         y2 = this->height();

	         repaint(0,y1,this->width(),y2);
         }*/

	      computePosition();
	      return;
      }
    
      QMultiLineEdit::insertChar(c); 
      computePosition();
      return;

   } // end do_wordbreak && fillcolumn_set
  

  // fillcolumn but no wordbreak

  if (fill_column_is_set){

    if (c == '\t')
    {
      QMultiLineEdit::insertChar((char)'\t');
      return;
    }

    if(c == '\n')
    {
      mynewLine();
      return;
    }

    if(myisprint(c))
    {
      if( col_pos >= fill_column_value )
      { 
	      mynewLine();
      }
    }

    QMultiLineEdit::insertChar(c);
    return;

  }

  // default action
  if(c == '\n'){
    
    mynewLine();
    return;

  }

  if (c == '\t'){
    if (isReadOnly())
      return;
    QMultiLineEdit::insertChar((char)'\t');
    return;
  }

  QMultiLineEdit::insertChar(c);
}


void MLEditWrap::backspace(void)
{
   QMultiLineEdit::backspace();
}

void MLEditWrap::newLine(void)
{
   QMultiLineEdit::newLine();
}



void MLEditWrap::computePosition(void)
{
  int line, col, coltemp;

  getCursorPosition(&line,&col);
  QString linetext = textLine(line);

  // O.K here is the deal: The function getCursorPositoin returns the character
  // position of the cursor, not the screenposition. I.e,. assume the line
  // consists of ab\tc then the character c will be on the screen on position 8
  // whereas getCursorPosition will return 3 if the cursors is on the character c.
  // Therefore we need to compute the screen position from the character position.
  // That's what all the following trouble is all about:
  
  coltemp  = 	col;
  int pos  = 	0;
  int find = 	0;
  int mem  = 	0;
  bool found_one = false;

  // if you understand the following algorithm you are worthy to look at the
  // kedit+ sources -- if not, go away ;-)

  while(find >=0 && find <= coltemp- 1 ){
    find = linetext.find('\t', find, TRUE );
    if( find >=0 && find <= coltemp - 1 ){
      found_one = true;
      pos = pos + find - mem;
      pos = pos + 8  - pos % 8;
      mem = find;
      find ++;
    }
  }

  pos = pos + coltemp - mem ;  // add the number of characters behind the
                               // last tab on the line.

  if (found_one){
    pos = pos - 1;
  }

  line_pos = line;
  col_pos = pos;

}


void MLEditWrap::keyPressEvent (QKeyEvent *e)
{
   emit keyPressed(e);
   if ((e->state() & ControlButton) && (e->key() == Key_Return || e->key() == Key_Enter))
   {
     emit signal_CtrlEnterPressed();
     return;
   }
/*
   if (e->key() == Key_Insert)
  {
    this->setOverwriteMode(!this->isOverwriteMode());
    return;
  }
*/
  if(fill_column_is_set && word_wrap_is_set ){

    // word break algorithm
    if(myisprint(e->ascii()) || e->key() == Key_Tab || e->key() == Key_Return || 
       e->key() == Key_Enter){

	if (e->key() == Key_Return || e->key() == Key_Enter){ 
	  mynewLine();
	}
	else{
	  if (e->key() == Key_Tab){
	    if (isReadOnly())
	      return;
	    QMultiLineEdit::insertChar((char)'\t');
	  }
	  else{
	    QMultiLineEdit::keyPressEvent(e); 
	  }
	}

	int templine,tempcol;
	getCursorPosition(&templine,&tempcol);

	computePosition();
	setAutoUpdate(false);
	
	bool did_break = format(par);
	int num_of_rows = 0;

	if(did_break){

	  QString newpar;

	  int cursorline = templine;  //Matthias
	  int cursorcol = 0; //Matthias
	  bool cursor_found = false; //Matthias
	  for( int k = 0; k < (int)par.count(); k++){
	    QString tmp = par.at(k); //Matthias
	    { //Matthias
	      int f = tmp.find((char)1);
	      if (f>-1){
		cursor_found = true;
		cursorcol = f;
		tmp.remove(f,1);
	      }
	    }
	    newpar += tmp; //Matthias
	    if (!cursor_found) cursorline++; //Matthias
	    if(k != (int)par.count() -1 )
	      newpar += '\n';
	  }
	  insertLine(newpar,templine);
	  newpar = "";
	  num_of_rows = par.count();
	  par.clear();
	  setCursorPosition(cursorline, cursorcol); //Matthias
	}

	setAutoUpdate(true);

	// Let's try to reduce flicker by updating only what we need to update
	if(did_break){
	  int y1  = -1;
	  int y2  = -1;

	  rowYPos(templine,&y1);
	  rowYPos(templine + num_of_rows ,&y2);

	  if(y1 == -1)
	    y1 = 0;

	    y2 = this->height();

	  repaint(0,y1,this->width(),y2);
	}

	computePosition();
	return;
    }
    
    QMultiLineEdit::keyPressEvent(e); 
    computePosition();
    return;

  } // end do_wordbreak && fillcolumn_set
  

  // fillcolumn but no wordbreak

  if (fill_column_is_set){

    if (e->key() == Key_Tab){
      if (isReadOnly())
	return;
      QMultiLineEdit::insertChar((char)'\t');
      return;
    }

    if(e->key() == Key_Return || e->key() == Key_Enter){
    
      mynewLine();
      return;

    }

    if(myisprint(e->ascii())){
    
      if( col_pos >= fill_column_value ){ 
	  mynewLine();
      }

    }

    QMultiLineEdit::keyPressEvent(e);
    return;

  }

  // default action
  if(e->key() == Key_Return || e->key() == Key_Enter){
    
    mynewLine();
    return;

  }

  if (e->key() == Key_Tab){
    if (isReadOnly())
      return;
    QMultiLineEdit::insertChar((char)'\t');
    return;
  }

  QMultiLineEdit::keyPressEvent(e);

}


bool MLEditWrap::format(QStrList& par)
{
  QString mstring;
  QString pstring;

  int space_pos;
  int right; /* char to right of space */

  int templine,tempcol;

  getCursorPosition(&templine,&tempcol);
  mstring = textLine(templine);

  /*  if((int)mstring.length() <= fill_column_value)
    return false;*/

  int l = 0;
  int k = 0;

  for( k = 0, l = 0; k < (int) mstring.length() && l <= fill_column_value; k++)
  {
    
    if(mstring.local8Bit()[k] == '\t')
      l +=8 - l%8;
    else
      l ++;

  }

  if( l <= fill_column_value)
    return false;

  // ########################## TODO consider a autoupdate(false) aroudn getpar #####
  getpar(templine,par);
  // ########################## TODO consider a autoupdate(false) aroudn getpar #####

  /*
    printf("\n");
    for ( int i = 0 ; i < (int)par.count() ; i ++){
       printf("%s\n",par.at(i));
    }
    printf("\n");
    */

  for ( int i = 0 ; i < (int)par.count() ; i ++){
    //    printf("par.count %d line %d\n",par.count(),i);
    k = 0;
    l = 0;
    int last_ok = 0;
    pstring = par.at(i);

    /*    if((int)pstring.length() <= fill_column_value)
      break;*/
    space_pos = -1; //Matthias
    for( k = 0, l = 0; k < (int) pstring.length() /* && l <= fill_column_value */; k++){//Matthias: commented out
    
      if(pstring.local8Bit()[k] == '\t')
        l +=8 - l%8;
      else
	l ++;

      if ((!space_pos||l<=fill_column_value) && 
	  (pstring.local8Bit()[k]==' '||pstring.local8Bit()[k]=='\t')) //Matthias
	space_pos = k;

      if( l <= fill_column_value )
	last_ok = k;
    }

    if( l <= fill_column_value)
      break;

    right = col_pos - space_pos - 1;
  
    if( space_pos == -1 ){ 
      /* Matthias: commented it out. Was broken, unfortunately

      // no space to be found on line, just break, what else could we do?
      par.remove(i);
      par.insert(i,pstring.left(last_ok+1));

      if(i < (int)par.count() - 1){
	QString temp1 = par.at(i+1);
	QString temp2;
	if(autoIndentMode){
	  temp1 = temp1.mid(prefixString(temp1).length(),temp1.length());
	}
	temp2 = pstring.mid(last_ok +1,pstring.length()) + (QString) " " + temp1;
	temp1 = temp2.copy();
	if(autoIndentMode)
	  temp1 = prefixString(pstring) + temp1;
	par.remove(i+1);
	par.insert(i+1,temp1);
      }
      else{
	if(autoIndentMode)
	  par.append(prefixString(pstring) + pstring.mid(last_ok + 1,pstring.length()));
	else
	  par.append(pstring.mid(last_ok +1,pstring.length()));
      }
      if(i==0){
	cursor_offset = pstring.length() - last_ok -1;
	if(autoIndentMode)
	  cursor_offset += prefixString(pstring).length();
	//printf("CURSOROFFSET1 %d\n",cursor_offset);
      }
      */
    }
    else{
    
      par.remove(i);
      par.insert(i,pstring.left(space_pos));
      

      if(i < (int)par.count() - 1){
	QString temp1 = par.at(i+1);
	QString temp2;
	if(autoIndentMode){
	  temp1 = temp1.mid(prefixString(temp1).length(),temp1.length());
	}
	temp2 = pstring.mid(space_pos +1,pstring.length()) + (QString) " " + temp1;
	temp1 = temp2.copy();
	if(autoIndentMode)
	  temp1 = prefixString(pstring) + temp1;
	par.remove(i+1);
	par.insert(i+1,temp1);
      }
      else{
	if(autoIndentMode)
	  par.append(prefixString(pstring) + pstring.mid(space_pos + 1,pstring.length()));
	else
	  par.append(pstring.mid(space_pos+1,pstring.length()));
      }
      if(i==0){
	cursor_offset = pstring.length() - space_pos -1;
	if(autoIndentMode)
	  cursor_offset += prefixString(pstring).length();
	//	printf("CURSOROFFSET2 %d\n",cursor_offset);
      }
    }
    
  }

  return true;

}


void MLEditWrap::getpar(int line,QStrList& par)
{ 
  int templine,tempcol; //Matthias
  getCursorPosition(&templine,&tempcol);//Matthias

  QString linestr;
  par.clear();
  int num = numLines();
  for(int i = line ; i < num ; i++){
    linestr = textLine(line);
    if (i == templine){
      // Matthias
      // cursor is in this line. At least it was. Insert mark.
      linestr.insert(tempcol, (char)1);
    }
    if(linestr.isEmpty())
      break;
    par.append(linestr);
    removeLine(line);
  }

}


void MLEditWrap::mynewLine(void)
{
//  if (isReadOnly())
//    return;

  if(!autoIndentMode){ // if not indent mode
    newLine();
    return;
  }

  int line,col, line2;
  bool found_one = false;

  getCursorPosition(&line,&col);
  line2 = line;

  QString string, string2;

  while(line >= 0){

    string  = textLine(line);
    string2 = string.stripWhiteSpace();

    if(!string2.isEmpty()){
      string = prefixString(string);
      found_one = TRUE;
      break;
    }

    line --;
  }
      
  // string will now contain those whitespace characters that I need to insert
  // on the next line. 

  if(found_one){
    newLine();
    insertAt(string.local8Bit(),line2 + 1,0);
  }
  else{
    newLine();
  }
}


QString MLEditWrap::prefixString(QString string)
{
  // This routine return the whitespace before the first non white space
  // character in string. This is  used in mynewLine() for indent mode.
  // It is assumed that string contains at least one non whitespace character
  // ie \n \r \t \v \f and space

  //  printf(":%s\n",string.data());

  int size = string.length();
  char* buffer = (char*) malloc(size + 1);
  strncpy (buffer, string.local8Bit(),size - 1);
  buffer[size] = '\0';

  int i;
  for (i = 0 ; i < size; i++){
    if(!isspace(buffer[i]))
      break;
  }

  buffer[i] = '\0';

  QString returnstring = buffer;
  
  free(buffer);

  //  printf(":%s:\n",returnstring.data());
  return returnstring;

}


//-----MLEditWrap::resizeEvent---------------------------------------------------------------------
void MLEditWrap::resizeEvent (QResizeEvent *e)
{
  QMultiLineEdit::resizeEvent(e); 
  fill_column_value = (int)(width() / fontMetrics().maxWidth());
  if (!fontInfo().fixedPitch()) fill_column_value *= 2;
}

#include "moc/moc_mledit.h"
