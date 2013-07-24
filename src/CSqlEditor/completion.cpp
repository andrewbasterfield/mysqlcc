/* Copyright (C) 2003 MySQL AB & Jorge del Conde

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.
    
  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
  MA 02111-1307, USA 
*/
/*************************************************************************
** Original code Copyright (C)2000 Trolltech AS.  All rights reserved.
**
** Modified by Jorge del Conde <jorge@mysql.com>
**************************************************************************/

#include "completion.h"
#include "paragdata.h"
#include "editor.h"
#include <stddef.h>
#include <q3listbox.h>
#include <q3vbox.h>
#include <qmap.h>
#include <qapplication.h>
#include <qregexp.h>
#include <qsizegrip.h>
#include <qtimer.h>
//Added by qt3to4:
#include <Q3Frame>
#include <Q3ValueList>
#include <QKeyEvent>
#include <QEvent>
#include <private/qrichtext_p.h>

static QColor getColor( const QString &type )
{
  if ( type == "function" || type == "slot")
    return Qt::blue;
  if ( type == "variable" )
    return Qt::darkRed;
  if ( type == "property" )
    return Qt::darkGreen;
  return Qt::black;
}

class CompletionItem : public Q3ListBoxItem
{
public:
  CompletionItem( Q3ListBox *lb, const QString &txt, const QString &t, const QString &p,
    const QString &pre, const QString &p2 )
    : Q3ListBoxItem( lb ), type( t ), postfix( p ), prefix( pre ), postfix2( p2 ),
    parag( 0 ), lastState( FALSE ) { setText( txt ); }
  ~CompletionItem() { delete parag; }
  void paint( QPainter *painter ) {
    if ( lastState != isSelected() ) {
      delete parag;
      parag = 0;
    }
    lastState = isSelected();
    if ( !parag )
      setupParagraph();
    parag->paint( *painter, listBox()->colorGroup() );
  }
  
  int height( const Q3ListBox * ) const {
    if ( !parag )
      ( (CompletionItem*)this )->setupParagraph();
    return parag->rect().height();
  }
  int width( const Q3ListBox * ) const {
    if ( !parag )
      ( (CompletionItem*)this )->setupParagraph();
    return parag->rect().width() - 2;
  }
  QString text() const { return Q3ListBoxItem::text() + postfix; }
  
private:
  void setupParagraph();
  QString type, postfix, prefix, postfix2;
  QTextParagraph *parag;
  bool lastState;
  
};

void CompletionItem::setupParagraph() {
#ifdef DEBUG
  qDebug("CompletionItem::setupParagraph()");
#endif

  if ( !parag ) {
    QTextFormatter *formatter;
    formatter = new QTextFormatterBreakWords;
    formatter->setWrapEnabled( FALSE );
    parag = new QTextParagraph( 0 );
    parag->pseudoDocument()->pFormatter = formatter;
    parag->insert( 0, " " + type + ( type.isEmpty() ? " " : "\t" ) + prefix +
      Q3ListBoxItem::text() + postfix + postfix2 );
    bool selCol = isSelected() && listBox()->colorGroup().highlightedText() != listBox()->colorGroup().text();
    QColor sc = selCol ? listBox()->colorGroup().highlightedText() : getColor( type );
    QTextFormat *f1 = parag->formatCollection()->format( listBox()->font(), sc );
    QTextFormat *f3 = parag->formatCollection()->format( listBox()->font(), isSelected() ?
      listBox()->colorGroup().highlightedText() :
    listBox()->colorGroup().text() );
    QFont f( listBox()->font() );
    f.setBold( TRUE );
    QTextFormat *f2 =
      parag->formatCollection()->format( f, isSelected() ? listBox()->colorGroup().highlightedText() :
    listBox()->colorGroup().text() );
    parag->setFormat( 1, type.length() + 1, f1 );
    parag->setFormat( type.length() + 2, prefix.length() + Q3ListBoxItem::text().length(), f2 );
    if ( !postfix.isEmpty() )
      parag->setFormat( type.length() + 2 + prefix.length() + Q3ListBoxItem::text().length(),
      postfix.length(), f3 );
    parag->setFormat( type.length() + 2 + prefix.length() + Q3ListBoxItem::text().length() + postfix.length(),
      postfix2.length(), f3 );
    f1->removeRef();
    f2->removeRef();
    f3->removeRef();
    parag->format();
  }
}


EditorCompletion::EditorCompletion( Editor *e )
{
#ifdef DEBUG
  qDebug("EditorCompletion::EditorCompletion()");
#endif

  case_sensitive = true;
  enabled = TRUE;
  lastDoc = 0;
  completionPopup = new Q3VBox( 0, 0, Qt::WType_Popup );
  completionPopup->setFrameStyle( Q3Frame::Box | Q3Frame::Plain );
  completionPopup->setLineWidth( 1 );
  completionListBox = new Q3ListBox( completionPopup, "editor_completion_lb" );
  completionListBox->setFrameStyle( Q3Frame::NoFrame );
  completionListBox->installEventFilter( this );
  completionListBox->setHScrollBarMode( Q3ScrollView::AlwaysOn );
  completionListBox->setVScrollBarMode( Q3ScrollView::AlwaysOn );
  completionListBox->setCornerWidget( new QSizeGrip( completionListBox, "editor_cornerwidget" ) );
  completionPopup->installEventFilter( this );
  completionPopup->setFocusProxy( completionListBox );
  completionOffset = 0;
  curEditor = e;
  curEditor->installEventFilter( this );
}

EditorCompletion::~EditorCompletion()
{
#ifdef DEBUG
  qDebug("EditorCompletion::~EditorCompletion()");
#endif

  delete completionPopup;
}

void EditorCompletion::addCompletionEntry( const QString &s, QTextDocument *, bool strict )
{
#ifdef DEBUG
  qDebug("EditorCompletion::addCompletionEntry()");
#endif

  QChar key = case_sensitive ? QChar(s[0]) : QChar(s.lower()[0]);
  QMap<QChar, QStringList>::Iterator it = completionMap.find( key );
 
  if ( it == completionMap.end() )
  {
    completionMap.insert( key, QStringList( s ) );
  }
  else
  {
    if ( strict )
    {
      QStringList::Iterator sit;
      for ( sit = (*it).begin(); sit != (*it).end(); )
      {
        QStringList::Iterator it2 = sit;
        ++sit;
        if ( (*it2).length() > s.length() && (*it2).left( s.length() ) == s )
        {
          if ( (*it2)[ (int)s.length() ].isLetter() && (*it2)[ (int)s.length() ].upper() != (*it2)[ (int)s.length() ] )
            return;
        }
        else
          if ( s.length() > (*it2).length() && s.left( (*it2).length() ) == *it2 )
          {
            if ( s[ (int)(*it2).length() ].isLetter() && s[ (int)(*it2).length() ].upper() != s[ (int)(*it2).length() ] )
              (*it).remove( it2 );
          }
      }
    }
    (*it).append( s );
  }
}

Q3ValueList<CompletionEntry> EditorCompletion::completionList( const QString &s, QTextDocument *doc ) const
{
#ifdef DEBUG
  qDebug("EditorCompletion::completionList()");
#endif

  if ( doc )
    ( (EditorCompletion*)this )->updateCompletionMap( doc );
  
  QChar key = case_sensitive ? QChar(s[0]) : QChar(s.lower()[0]);
  QMap<QChar, QStringList>::ConstIterator it = completionMap.find( key );
  if ( it == completionMap.end() )
    return Q3ValueList<CompletionEntry>();
  QStringList::ConstIterator it2 = (*it).begin();
  Q3ValueList<CompletionEntry> lst;
  int len = s.length();
  for ( ; it2 != (*it).end(); ++it2 ) {
    CompletionEntry c;
    c.type = "";
    c.text = *it2;
    c.postfix = "";
    c.prefix = "";
    c.postfix2 = "";
    
    if (!case_sensitive || (int)(*it2).length() > len && (*it2).left( len ) == s && lst.find( c ) == lst.end())
      lst << c;
  }
  return lst;
}

void EditorCompletion::updateCompletionMap( QTextDocument *doc )
{
#ifdef DEBUG
  qDebug("EditorCompletion::updateCompletionMap()");
#endif

  /*
    It's not wise to modify the completion map if the editor is not in case_sensitive mode
  */

  if (!case_sensitive)
    return;

  bool strict = TRUE;
  if ( doc != lastDoc )
    strict = FALSE;
  lastDoc = doc;
  QTextParagraph *s = doc->firstParagraph();
  if ( !s->extraData() )
    s->setExtraData( new ParagData );
  while ( s ) {
    if ( s->length() == ( (ParagData*)s->extraData() )->lastLengthForCompletion ) {
      s = s->next();
      continue;
    }
    
    QChar c;
    QString buffer;
    for ( int i = 0; i < s->length(); ++i ) {
      c = s->at( i )->c;
      if ( c.isLetter() || c.isNumber() || c == '_' || c == '#' ) {
        buffer += c;
      } else {
        addCompletionEntry( buffer, doc, strict );
        buffer = QString::null;
      }
    }
    if ( !buffer.isEmpty() )
      addCompletionEntry( buffer, doc, strict );
    
    ( (ParagData*)s->extraData() )->lastLengthForCompletion = s->length();
    s = s->next();
  }
}

bool EditorCompletion::doCompletion()
{
#ifdef DEBUG
  qDebug("EditorCompletion::doCompletion()");
#endif

  searchString = "";
  if ( !curEditor )
    return FALSE;
  
  QTextCursor *cursor = curEditor->textCursor();
  QTextDocument *doc = curEditor->document();
  /*  Don't need this for the SQL Editor
  if ( cursor->index() > 0 && cursor->paragraph()->at( cursor->index() - 1 )->c == '.' )
    return doObjectCompletion();
  */
  
  int idx = cursor->index();
  if ( idx == 0 )
    return FALSE;
  QChar c = cursor->paragraph()->at( idx - 1 )->c;
  if ( !c.isLetter() && !c.isNumber() && c != '_' && c != '#' && c != '.')
    return FALSE;
  
  QString s;
  idx--;
  completionOffset = 1;
  for (;;) {
    s.prepend( QString( cursor->paragraph()->at( idx )->c ) );
    idx--;
    if ( idx < 0 )
      break;
    if ( !cursor->paragraph()->at( idx )->c.isLetter() &&
	     !cursor->paragraph()->at( idx )->c.isNumber() &&
       cursor->paragraph()->at( idx )->c != '_' &&
       cursor->paragraph()->at( idx )->c != '#' &&
       cursor->paragraph()->at( idx )->c != '.' )
       break;
    completionOffset++;
  }
  
  searchString = case_sensitive ? s : s.lower();
    
  Q3ValueList<CompletionEntry> lst( completionList( s, doc ) );
  if ( lst.count() > 1 )
  {
    QTextStringChar *chr = cursor->paragraph()->at( cursor->index() );
    int h = cursor->paragraph()->lineHeightOfChar( cursor->index() );
    int x = cursor->paragraph()->rect().x() + chr->x;
    int y, dummy;
    cursor->paragraph()->lineHeightOfChar( cursor->index(), &dummy, &y );
    y += cursor->paragraph()->rect().y();
    completionListBox->clear();
    for ( Q3ValueList<CompletionEntry>::ConstIterator it = lst.begin(); it != lst.end(); ++it )
      (void)new CompletionItem( completionListBox, (*it).text, (*it).type, (*it).postfix,
      (*it).prefix, (*it).postfix2 );
    cList = lst;
    completionPopup->resize( completionListBox->sizeHint() +
      QSize( completionListBox->verticalScrollBar()->width() + 4,
      completionListBox->horizontalScrollBar()->height() + 4 ) );
    completionListBox->setCurrentItem( 0 );
    completionListBox->setFocus();
    if ( curEditor->mapToGlobal( QPoint( 0, y ) ).y() + h + completionPopup->height() < QApplication::desktop()->height() )
      completionPopup->move( curEditor->mapToGlobal( curEditor->contentsToViewport( QPoint( x, y + h ) ) ) );
    else
      completionPopup->move( curEditor->mapToGlobal( curEditor->contentsToViewport( QPoint( x, y - completionPopup->height() ) ) ) );
    completionPopup->show();
  }
  else
    if ( lst.count() == 1 )
    {
      if (case_sensitive)
        curEditor->insert(lst.first().text.mid( completionOffset, 0xFFFFFF ), bool(TRUE));
      else
      {
        for (unsigned int x = 1; x <= searchString.length(); x++)
          curEditor->doKeyboardAction(Q3TextEdit::ActionBackspace);  
        curEditor->insert(lst.first().text, bool(TRUE));
      }
    }
    else
    {
      return FALSE;
    }
  return TRUE;
}


bool EditorCompletion::eventFilter( QObject *o, QEvent *e )
{
  if ( !enabled )
    return FALSE;
  if ( o->inherits( "Editor" ) && e->type() == QEvent::KeyPress ) {
    curEditor = (Editor*)o;
    QKeyEvent *ke = (QKeyEvent*)e;
    if ( ke->key() == Qt::Key_Tab ) {
      QString s = curEditor->textCursor()->paragraph()->string()->toString().
        left( curEditor->textCursor()->index() );
      if ( s.simplifyWhiteSpace().isEmpty() ) {
        curEditor->indent();
        int i = 0;
        for ( ; i < curEditor->textCursor()->paragraph()->length() - 1; ++i ) {
          if ( curEditor->textCursor()->paragraph()->at( i )->c != ' ' &&
            curEditor->textCursor()->paragraph()->at( i )->c != '\t' )
            break;
        }
        curEditor->drawCursor( FALSE );
        curEditor->textCursor()->setIndex( i );
        curEditor->drawCursor( TRUE );
        return TRUE;
      }
    }
    if ( ke->text().length() && !( ke->state() & Qt::AltModifier ) &&
	     ( !ke->ascii() || ke->ascii() >= 32 ) ||
       ( ke->text() == "\t" && !( ke->state() & Qt::ControlModifier ) ) ) {
      if ( ke->key() == Qt::Key_Tab ) {
        if ( curEditor->textCursor()->index() == 0 &&
          curEditor->textCursor()->paragraph()->isListItem() )
          return FALSE;
        if ( doCompletion() )
          return TRUE;
      } else if ( ke->key() == Qt::Key_Period ||
        ke->key() == Qt::Key_Greater &&
        curEditor->textCursor()->index() > 0 &&
        curEditor->textCursor()->paragraph()->at( curEditor->textCursor()->index() - 1 )->c == '-' ) {
        doObjectCompletion();
      }
    }
  } else if ( o == completionPopup || o == completionListBox ||
    o == completionListBox->viewport() ) {
    if ( e->type() == QEvent::KeyPress ) {
      QKeyEvent *ke = (QKeyEvent*)e;
      if ( ke->key() == Qt::Key_Enter || ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Tab ) {
        if ( ke->key() == Qt::Key_Tab && completionListBox->count() > 1 &&
          completionListBox->currentItem() < (int)completionListBox->count() - 1 ) {
          completionListBox->setCurrentItem( completionListBox->currentItem() + 1 );
          return TRUE;
        }
        completeCompletion();
        return TRUE;
      } else if ( ke->key() == Qt::Key_Left || ke->key() == Qt::Key_Right ||
        ke->key() == Qt::Key_Up || ke->key() == Qt::Key_Down ||
        ke->key() == Qt::Key_Home || ke->key() == Qt::Key_End ||
        ke->key() == Qt::Key_PageUp || ke->key() == Qt::Key_PageDown ) {
        return FALSE;
      } else if ( ke->key() != Qt::Key_Shift && ke->key() != Qt::Key_Control &&
        ke->key() != Qt::Key_Alt ) {
        int l = searchString.length();
        if ( ke->key() == Qt::Key_Backspace ) {
          searchString.remove( searchString.length() - 1, 1 );
        } else {
          searchString += ke->text();
          l = 1;
        }
        if ( !l || !continueComplete() ) {
          completionPopup->close();
          curEditor->setFocus();
        }
        QApplication::sendEvent( curEditor, e );
        return TRUE;
      }
    } else if ( e->type() == QEvent::MouseButtonDblClick ) {
      completeCompletion();
      return TRUE;
    }
  }
  
  return FALSE;
}

void EditorCompletion::completeCompletion()
{
#ifdef DEBUG
  qDebug("EditorCompletion::completeCompletion()");
#endif

  QString s;
  if (case_sensitive)
    s = completionListBox->currentText().mid( searchString.length() );
  else
  {
    s = completionListBox->currentText();
    for (unsigned int i = 1; i <= searchString.length(); i++)
      curEditor->doKeyboardAction(Q3TextEdit::ActionBackspace);
  }

  curEditor->insert(s, bool(TRUE));
  completionPopup->close();
  curEditor->setFocus();

  int idx = curEditor->textCursor()->index();
  int i = s.find( '(' );
  if ( i != -1 && i < (int)s.length() )
    curEditor->setCursorPosition( curEditor->textCursor()->paragraph()->paragId(), idx + i + 1 );
}

void EditorCompletion::setCurrentEdior( Editor *e )
{
#ifdef DEBUG
  qDebug("EditorCompletion::setCurrentEdior()");
#endif

  curEditor = e;
  curEditor->installEventFilter( this );
}

void EditorCompletion::addEditor( Editor *e )
{
#ifdef DEBUG
  qDebug("EditorCompletion::addEditor()");
#endif

  e->installEventFilter( this );
}

bool EditorCompletion::doObjectCompletion()
{
#ifdef DEBUG
  qDebug("EditorCompletion::doObjectCompletion()");
#endif

  searchString = "";
  QString object;
  int i = curEditor->textCursor()->index();
  i--;
  QTextParagraph *p = curEditor->textCursor()->paragraph();
  for (;;) {
    if ( i < 0 )
      break;
    if ( p->at( i )->c == ' ' || p->at( i )->c == '\t' )
      break;
    object.prepend( p->at( i )->c );
    i--;
  }
  
  if ( object[ (int)object.length() - 1 ] == '-' )
    object.remove( object.length() - 1, 1 );
  
  if ( object.isEmpty() )
    return FALSE;
  return doObjectCompletion( object );
}

bool EditorCompletion::doObjectCompletion( const QString & )
{
#ifdef DEBUG
  qDebug("EditorCompletion::doObjectCompletion(const QString &)");
#endif

  return FALSE;
}

static void strip( QString &txt )
{
  int i = txt.find( "(" );
  if ( i == -1 )
    return;
  txt = txt.left( i );
}

bool EditorCompletion::continueComplete()
{
#ifdef DEBUG
  qDebug("EditorCompletion::continueComplete()");
#endif

  if ( searchString.isEmpty() ) {
    completionListBox->clear();
    for ( Q3ValueList<CompletionEntry>::ConstIterator it = cList.begin(); it != cList.end(); ++it )
      (void)new CompletionItem( completionListBox, (*it).text, (*it).type,
      (*it).postfix, (*it).prefix, (*it).postfix2 );
    completionListBox->setCurrentItem( 0 );
    completionListBox->setSelected( completionListBox->currentItem(), TRUE );
    return TRUE;
  }
  
  Q3ListBoxItem *i = completionListBox->findItem( searchString );
  if ( !i )
    return FALSE;
  
  QString txt1 = i->text();
  QString txt2 = searchString;
  strip( txt1 );
  strip( txt2 );
  if ( txt1 == txt2 && !i->next() )
    return FALSE;
  
  Q3ValueList<CompletionEntry> res;
  for ( Q3ValueList<CompletionEntry>::ConstIterator it = cList.begin(); it != cList.end(); ++it )
  {
    if (case_sensitive)
    {
      if ( (*it).text.left( searchString.length() ) == searchString )
        res << *it;
    }
    else
      if ( (*it).text.lower().left( searchString.length() ) == searchString.lower() )
        res << *it;
  }
  if ( res.isEmpty() )
    return FALSE;
  completionListBox->clear();
  for ( Q3ValueList<CompletionEntry>::ConstIterator it2 = res.begin(); it2 != res.end(); ++it2 )
    (void)new CompletionItem( completionListBox, (*it2).text, (*it2).type,
    (*it2).postfix, (*it2).prefix, (*it2).postfix2 );
  completionListBox->setCurrentItem( 0 );
  completionListBox->setSelected( completionListBox->currentItem(), TRUE );
  return TRUE;
}

Q3ValueList<QStringList> EditorCompletion::functionParameters( const QString &, QChar &, QString &, QString & )
{
#ifdef DEBUG
  qDebug("EditorCompletion::functionParameters()");
#endif

  return Q3ValueList<QStringList>();
}

void EditorCompletion::setContext( QObjectList *, QObject * )
{
#ifdef DEBUG
  qDebug("EditorCompletion::setContext()");
#endif

}

void EditorCompletion::showCompletion( const Q3ValueList<CompletionEntry> &lst )
{
  QTextCursor *cursor = curEditor->textCursor();
  QTextStringChar *chr = cursor->paragraph()->at( cursor->index() );
  int h = cursor->paragraph()->lineHeightOfChar( cursor->index() );
  int x = cursor->paragraph()->rect().x() + chr->x;
  int y, dummy;
  cursor->paragraph()->lineHeightOfChar( cursor->index(), &dummy, &y );
  y += cursor->paragraph()->rect().y();
  completionListBox->clear();
  for ( Q3ValueList<CompletionEntry>::ConstIterator it = lst.begin(); it != lst.end(); ++it )
    (void)new CompletionItem( completionListBox, (*it).text, (*it).type,
    (*it).postfix, (*it).prefix, (*it).postfix2 );
  cList = lst;
  completionPopup->resize( completionListBox->sizeHint() +
    QSize( completionListBox->verticalScrollBar()->width() + 4,
    completionListBox->horizontalScrollBar()->height() + 4 ) );
  completionListBox->setCurrentItem( 0 );
  completionListBox->setFocus();
  if ( curEditor->mapToGlobal( QPoint( 0, y ) ).y() + h + completionPopup->height() < QApplication::desktop()->height() )
    completionPopup->move( curEditor->mapToGlobal( curEditor->
    contentsToViewport( QPoint( x, y + h ) ) ) );
  else
    completionPopup->move( curEditor->mapToGlobal( curEditor->
    contentsToViewport( QPoint( x, y - completionPopup->height() ) ) ) );
  
  completionPopup->show();
}
