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
#include "editor.h"
#include "parenmatcher.h"
#include "globals.h"
#include "CApplication.h"
#include <stddef.h>  
#include <qfile.h>
#include <q3popupmenu.h>
#include <qprinter.h>
#include <q3simplerichtext.h>
#include <q3paintdevicemetrics.h>
//Added by qt3to4:
#include <Q3CString>
#include <private/qrichtext_p.h>


#ifdef DEBUG_LEVEL
#if DEBUG_LEVEL < 4
#undef DEBUG
#endif
#else
#ifdef DEBUG
#undef DEBUG
#endif
#endif

Editor::Editor(QWidget *parent, const char *name)
: Q3TextEdit(parent, name), hasError(false)
{
#ifdef DEBUG
  qDebug("Editor::Editor()");
#endif

  document()->setFormatter(new QTextFormatterBreakInWords);    
  setHScrollBarMode(Q3ScrollView::AlwaysOff);
  setVScrollBarMode(Q3ScrollView::AlwaysOn);
  document()->setUseFormatCollection(false);
  parenMatcher = new ParenMatcher;
  connect(this, SIGNAL(cursorPositionChanged(QTextCursor *)),
    this, SLOT(cursorPosChanged(QTextCursor *)));
  document()->addSelection(Error);
  document()->addSelection(Step);
  document()->setSelectionColor(Error, Qt::red);
  document()->setSelectionColor(Step, Qt::yellow);
  document()->setInvertSelectionText(Error, false);
  document()->setInvertSelectionText(Step, false);
  document()->addSelection(ParenMatcher::Match);
  document()->addSelection(ParenMatcher::Mismatch);
  document()->setSelectionColor(ParenMatcher::Match, QColor(204, 232, 195));
  document()->setSelectionColor(ParenMatcher::Mismatch, Qt::magenta);
  document()->setInvertSelectionText(ParenMatcher::Match, false);
  document()->setInvertSelectionText(ParenMatcher::Mismatch, false);
}

Editor::~Editor()
{
#ifdef DEBUG
  qDebug("Editor::~Editor()");
#endif

  delete parenMatcher;
}

void Editor::cursorPosChanged(QTextCursor *c)
{
#ifdef DEBUG
  qDebug("Editor::cursorPosChanged()");
#endif

  if (parenMatcher->match(c))
    repaintChanged();
  if (hasError)
  {
    emit clearErrorMarker();
    hasError = false;
  }
}

void Editor::load(const QString &fn)
{
#ifdef DEBUG
  qDebug("Editor::load()");
#endif

  filename = fn;
  QFile f(filename);
  if (!f.open(QIODevice::ReadOnly))
    return;
  Q3CString txt;
  txt.resize(f.size());
  f.readBlock(txt.data(), f.size());
  setText(txt);
}

void Editor::clear()
{
  removeSelection();
  setText("");
}

void Editor::save(const QString &fn)
{
#ifdef DEBUG
  qDebug("Editor::save()");
#endif

  if (!filename.isEmpty())
    filename = fn;
}

void Editor::configChanged()
{
#ifdef DEBUG
  qDebug("Editor::setErrorSelection()");
#endif

  document()->invalidate();
  viewport()->repaint(false);
}

void Editor::setErrorSelection(int line)
{
#ifdef DEBUG
  qDebug("Editor::setErrorSelection()");
#endif

  QTextParagraph *p = document()->paragAt(line);
  if (!p)
    return;
  QTextCursor c(document());
  c.setParagraph(p);
  c.setIndex(0);
  document()->removeSelection(Error);
  document()->setSelectionStart(Error, c);
  c.gotoLineEnd();
  document()->setSelectionEnd(Error, c);
  hasError = true;
  viewport()->repaint(false);
}

void Editor::setStepSelection(int line)
{
#ifdef DEBUG
  qDebug("Editor::setStepSelection()");
#endif

  QTextParagraph *p = document()->paragAt(line);
  if (!p)
    return;
  QTextCursor c(document());
  c.setParagraph(p);
  c.setIndex(0);
  document()->removeSelection(Step);
  document()->setSelectionStart(Step, c);
  c.gotoLineEnd();
  document()->setSelectionEnd(Step, c);
  viewport()->repaint(false);
}

void Editor::clearStepSelection()
{
#ifdef DEBUG
  qDebug("Editor::clearStepSelection()");
#endif

  document()->removeSelection(Step);
  viewport()->repaint(false);
}

void Editor::doChangeInterval()
{
#ifdef DEBUG
  qDebug("Editor::doChangeInterval()");
#endif

  emit intervalChanged();
  Q3TextEdit::doChangeInterval();
}

void Editor::commentSelection()
{
#ifdef DEBUG
  qDebug("Editor::commentSelection()");
#endif

  QTextParagraph *start = document()->selectionStartCursor(QTextDocument::Standard).paragraph();
  QTextParagraph *end = document()->selectionEndCursor(QTextDocument::Standard).paragraph();
  if (!start || !end)
    start = end = textCursor()->paragraph();
  while (start)
  {
    if (start == end && textCursor()->index()== 0)
      break;
    start->insert(0, "//");
    if (start == end)
      break;
    start = start->next();
  }
  document()->removeSelection(QTextDocument::Standard);
  repaintChanged();
  setModified(true);
}

void Editor::uncommentSelection()
{
#ifdef DEBUG
  qDebug("Editor::uncommentSelection()");
#endif

  QTextParagraph *start = document()->selectionStartCursor(QTextDocument::Standard).paragraph();
  QTextParagraph *end = document()->selectionEndCursor(QTextDocument::Standard).paragraph();
  if (!start || !end)
    start = end = textCursor()->paragraph();
  while (start)
  {
    if (start == end && textCursor()->index()== 0)
      break;
    while (start->at(0)->c == '/')
      start->remove(0, 1);
    if (start == end)
      break;
    start = start->next();
  }
  document()->removeSelection(QTextDocument::Standard);
  repaintChanged();
  setModified(true);
}

void Editor::print()
{
#ifndef QT_NO_PRINTER
#ifdef DEBUG
  qDebug("Editor::print()");
#endif

  QPrinter printer(QPrinter::HighResolution);
  printer.setFullPage(true);
  if (printer.setup(this))
  {
    QPainter p(&printer);
    if(!p.isActive())
      return;
    Q3PaintDeviceMetrics metrics(p.device());
    int dpiy = metrics.logicalDpiY();
    int margin = (int) ((2/2.54)*dpiy);
    QRect body(margin, margin, metrics.width() - 2 * margin, metrics.height() - 2 * margin);
    QString t = text();
    if (textFormat()== Qt::PlainText)
    {
      t = charReplace(t, '<', "&lt;");
      t = charReplace(t, '>', "&gt;");
    }
    t = charReplace(t, '\n', "<br>");
    Q3SimpleRichText richText(t, myApp()->printerFont(), context(), styleSheet(), mimeSourceFactory(), body.height());
    richText.setWidth(&p, body.width());

    QRect view( body );
    int page = 1;
    do {
      richText.draw(&p, body.left(), body.top(), view, colorGroup());
      view.moveBy(0, body.height());
      p.translate(0 , -body.height());
      p.drawText(view.right() - p.fontMetrics().width(QString::number(page)), view.bottom() + p.fontMetrics().ascent() + 5, QString::number(page));
      if (view.top() >= richText.height())
        break;
      printer.newPage();
      page++;
    } while (true);
  }
#endif
}
