/*
   Copyright (C) 2002 by Roberto Raggi <raggi@cli.di.unipi.it>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   version 2, License as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "qeditor_arghint.h"
#include "qeditor_view.h"
#include "qeditor_part.h"

#include <qaccel.h>
#include <qlabel.h>
#include <qintdict.h>
#include <qlayout.h>
#include <qregexp.h>

#include <kdebug.h>

struct QEditorArgHintData{
    QIntDict<QLabel> labelDict;
    QLayout* layout;
};


using namespace std;

QEditorArgHint::QEditorArgHint( QWidget* parent, const char* name )
    : QFrame( parent, name, WType_Popup ),
      m_escAccel( 0 )
{
    setBackgroundColor( black );

    d = new QEditorArgHintData();
    d->labelDict.setAutoDelete( TRUE );
    d->layout = new QVBoxLayout( this, 1, 2 );
    d->layout->setAutoAdd( TRUE );

    m_markCurrentFunction = true;

    setFocusPolicy( StrongFocus );
    setFocusProxy( parent );

    reset( -1, -1 );
}

QEditorArgHint::~QEditorArgHint()
{
    delete( d );
    d = 0;
}

void QEditorArgHint::setArgMarkInfos( const QString& wrapping, const QString& delimiter )
{
    m_wrapping = wrapping;
    m_delimiter = delimiter;
    m_markCurrentFunction = true;
}

void QEditorArgHint::reset( int line, int col )
{
    m_functionMap.clear();
    m_currentFunction = -1;
    d->labelDict.clear();

    m_currentLine = line;
    m_currentCol = col - 1;

    m_escAccel = new QAccel( (QWidget*) parent() );
    m_escAccel->insertItem( Key_Escape, 1 );
    m_escAccel->setEnabled( true );
    connect( m_escAccel, SIGNAL(activated(int)), this, SLOT(slotDone()) );
}

void QEditorArgHint::slotDone()
{
    hide();
    if( m_escAccel ){
        delete( m_escAccel );
        m_escAccel = 0;
    }

    m_currentLine = m_currentCol = -1;

    emit argHintHidden();
}

void QEditorArgHint::cursorPositionChanged( QEditorView* view, int line, int col )
{
    if( m_currentCol == -1 || m_currentLine == -1 ){
        slotDone();
        return;
    }

    int nCountDelimiter = 0;
    int count = 0;

    QString currentTextLine = view->doc()->textLine( line );
    QString text = currentTextLine.mid( m_currentCol, col - m_currentCol );
    QRegExp strconst_rx( "\"[^\"]*\"" );
    QRegExp chrconst_rx( "'[^']*'" );

    text = text
        .replace( strconst_rx, "\"\"" )
        .replace( chrconst_rx, "''" );

    int index = 0;
    while( index < (int)text.length() ){
        if( text[index] == m_wrapping[0] ){
            ++count;
        } else if( text[index] == m_wrapping[1] ){
            --count;
        } else if( count > 0 && text[index] == m_delimiter[0] ){
            ++nCountDelimiter;
        }
        ++index;
    }

    if( (m_currentLine > 0 && m_currentLine != line) || (m_currentLine < col) || (count == 0) ){
        slotDone();
        return;
    }

    // setCurArg ( nCountDelimiter + 1 );

}

void QEditorArgHint::addFunction( int id, const QString& prot )
{
    m_functionMap[ id ] = prot;
    QLabel* label = new QLabel( prot.stripWhiteSpace().simplifyWhiteSpace(), this );
    label->setBackgroundColor( QColor (255, 255, 238) );
    label->show();
    d->labelDict.insert( id, label );

    if( m_currentFunction < 0 )
        setCurrentFunction( id );
}

void QEditorArgHint::setCurrentFunction( int currentFunction )
{
    if( m_currentFunction != currentFunction ){

        if( currentFunction < 0 )
            currentFunction = (int)m_functionMap.size() - 1;

        if( currentFunction > (int)m_functionMap.size()-1 )
            currentFunction = 0;

        if( m_markCurrentFunction && m_currentFunction >= 0 ){
            QLabel* label = d->labelDict[ m_currentFunction ];
            label->setFont( font() );
        }

        m_currentFunction = currentFunction;

        if( m_markCurrentFunction ){
            QLabel* label = d->labelDict[ currentFunction ];
            QFont fnt( font() );
            fnt.setBold( TRUE );
            label->setFont( fnt );
        }

        adjustSize();
    }
}

void QEditorArgHint::show()
{
    QFrame::show();
    adjustSize();
}

bool QEditorArgHint::eventFilter( QObject*, QEvent* e )
{
    if( isVisible() && e->type() == QEvent::KeyPress ){
        QKeyEvent* ke = static_cast<QKeyEvent*>( e );
        if( (ke->state() & ControlButton) && ke->key() == Key_Left ){
            setCurrentFunction( currentFunction() - 1 );
            ke->accept();
            return TRUE;
        } else if( (ke->state() & ControlButton) && ke->key() == Key_Right ){
            setCurrentFunction( currentFunction() + 1 );
            ke->accept();
            return TRUE;
        }
    }
    return FALSE;
}
