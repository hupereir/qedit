// $Id$

/******************************************************************************
*
* this is free software; you can redistribute it and/or modify it under the
* terms of the GNU General Public License as published by the Free Software
* Foundation; either version 2 of the License, or (at your option) any later
* version.
*
* This software is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
*******************************************************************************/

/*!
  \file DocumentClassTextEditor.cpp
  \brief text display window
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QAbstractTextDocumentLayout>
#include <QPainter>
#include <QTextLayout>

#include "BlockDelimiterDisplay.h"
#include "CustomTextDocument.h"
#include "DocumentClass.h"
#include "DocumentClassManager.h"
#include "HighlightBlockData.h"
#include "HighlightBlockFlags.h"
#include "InformationDialog.h"
#include "QtUtil.h"
#include "Singleton.h"
#include "TextEditorMarginWidget.h"
#include "DocumentClassTextEditor.h"
#include "TextHighlight.h"
#include "TextIndent.h"
#include "TextMacro.h"
#include "TextSeparator.h"
#include "Util.h"
#include "XmlOptions.h"

using namespace std;

//___________________________________________________
QRegExp& DocumentClassTextEditor::_emptyLineRegExp( void )
{
  static QRegExp regexp( "(^\\s*$)" );
  return regexp;
}

//___________________________________________________
DocumentClassTextEditor::DocumentClassTextEditor( QWidget* parent ):
  AnimatedTextEditor( parent ),
  text_highlight_( 0 ),
  block_delimiter_display_( 0 )
{

  Debug::Throw("DocumentClassTextEditor::DocumentClassTextEditor.\n" );

  // disable rich text
  setAcceptRichText( false );

  // text highlight
  text_highlight_ = new TextHighlight( document() );

  // parenthesis highlight
  parenthesis_highlight_ = new ParenthesisHighlight( this );

  // text indent
  indent_ = new TextIndent( this );

  // block delimiter
  block_delimiter_display_ = new BlockDelimiterDisplay( this );
  connect( &textHighlight(), SIGNAL( needSegmentUpdate() ), &blockDelimiterDisplay(), SLOT( needUpdate() ) );

  // connections
  connect( this, SIGNAL( cursorPositionChanged() ), SLOT( _highlightParenthesis() ) );
  connect( this, SIGNAL( indent( QTextBlock, bool ) ), indent_, SLOT( indent( QTextBlock, bool ) ) );
  connect( this, SIGNAL( indent( QTextBlock, QTextBlock ) ), indent_, SLOT( indent( QTextBlock, QTextBlock ) ) );

  // connections
  // track contents changed for syntax highlighting
  connect( DocumentClassTextEditor::document(), SIGNAL( contentsChange( int, int, int ) ), SLOT( _setBlockModified( int, int, int ) ) );

  // track configuration modifications
  connect( Singleton::get().application(), SIGNAL( configurationChanged() ), SLOT( _updateConfiguration() ) );
  _updateConfiguration();

  Debug::Throw( "DocumentClassTextEditor::DocumentClassTextEditor - done.\n" );

}

//_____________________________________________________
DocumentClassTextEditor::~DocumentClassTextEditor( void )
{ Debug::Throw() << "DocumentClassTextEditor::~DocumentClassTextEditor - key: " << key() << endl; }

//_____________________________________________________
int DocumentClassTextEditor::blockCount( const QTextBlock& block ) const
{

  QTextBlockFormat block_format( block.blockFormat() );
  if( block_format.boolProperty( TextBlock::Collapsed ) && block_format.hasProperty( TextBlock::CollapsedData ) )
  {  return block_format.property( TextBlock::CollapsedData ).value<CollapsedBlockData>().blockCount(); }
  else return AnimatedTextEditor::blockCount( block );

}

//__________________________________________________________
void DocumentClassTextEditor::paintMargin( QPainter& painter )
{
  AnimatedTextEditor::paintMargin( painter );
  blockDelimiterDisplay().paint( painter );
}

//___________________________________________________________________________
bool DocumentClassTextEditor::ignoreBlock( const QTextBlock& block ) const
{

  // first check if block text match empty line
  if( isEmptyBlock( block ) ) return true;

  // try retrieve highlight data
  HighlightBlockData *data( dynamic_cast<HighlightBlockData*>( block.userData() ) );
  return ( data && data->ignoreBlock() );

}

//___________________________________________________________________________
void DocumentClassTextEditor::setDocumentClass( const DocumentClass& document_class )
{

  Debug::Throw( "DocumentClassTextEditor::_updateDocumentClass\n" );

  textHighlight().clear();
  textIndent().clear();
  textIndent().setBaseIndentation(0);
  _clearMacros();

  wrapModeAction().setChecked( document_class.wrap() );
  tabEmulationAction().setChecked( document_class.emulateTabs() );

  if( document_class.tabSize() > 0 )
  { _setTabSize( document_class.tabSize() ); }

  // store into class members
  textHighlight().setPatterns( document_class.highlightPatterns() );
  textHighlight().setParenthesis( document_class.parenthesis() );
  textHighlight().setBlockDelimiters( document_class.blockDelimiters() );
  textHighlight().setHighlightEnabled( true );

  textIndent().setPatterns( document_class.indentPatterns() );
  textIndent().setBaseIndentation( document_class.baseIndentation() );
  textIndent().setEnabled( true );

  _setMacros( document_class.textMacros() );

  // update block delimiters
  if( blockDelimiterDisplay().expandAllAction().isEnabled() ) blockDelimiterDisplay().expandAllAction().trigger();
  if( blockDelimiterDisplay().setBlockDelimiters( document_class.blockDelimiters() ) ) update();
  _updateMargin();

  // update enability for parenthesis matching
  textHighlight().setParenthesisEnabled(
    textHighlight().parenthesisHighlightColor().isValid() &&
    !textHighlight().parenthesis().empty() );

  parenthesisHighlight().setEnabled(
    textHighlight().parenthesisHighlightColor().isValid() &&
    !textHighlight().parenthesis().empty() );

  return;

}

//_____________________________________________
void DocumentClassTextEditor::processMacro( QString name )
{

  Debug::Throw() << "DocumentClassTextEditor::processMacro - " << name << endl;

  // retrieve macro that match argument name
  TextMacro::List::const_iterator macro_iter = find_if( macros_.begin(), macros_.end(), TextMacro::SameNameFTor( name ) );
  if( macro_iter == macros_.end() )
  {
    QString buffer;
    QTextStream( &buffer ) << "Unable to find macro named " << name;
    InformationDialog( this, buffer ).exec();
    return;
  }

  // retrieve text cursor
  QTextCursor cursor( textCursor() );
  if( !cursor.hasSelection() ) return;

  // retrieve blocks
  int position_begin( min( cursor.position(), cursor.anchor() ) );
  int position_end( max( cursor.position(), cursor.anchor() ) );
  QTextBlock begin( document()->findBlock( position_begin ) );
  QTextBlock end( document()->findBlock( position_end ) );

  // enlarge selection so that it matches begin and end of blocks
  position_begin = begin.position();
  if( position_end == end.position() )
  {
    position_end--;
    end = end.previous();
  } else {  position_end = end.position() + end.length() - 1; }

  // prepare text from selected blocks
  QString text;
  if( begin == end ) text = begin.text().mid( position_begin - begin.position(), position_end-position_begin );
  else {
    text = begin.text().mid( position_begin - begin.position() ) + "\n";
    for( QTextBlock block = begin.next(); block.isValid() && block!= end; block = block.next() )
    { text += block.text() + "\n"; }
    text += end.text().left( position_end - end.position() );
  }

  // process macro
  if( !macro_iter->processText( text ).first ) return;

  // update selection
  cursor.setPosition( position_begin );
  cursor.setPosition( position_end, QTextCursor::KeepAnchor );

  // insert new text
  cursor.insertText( text );

  // restore selection
  cursor.setPosition( position_begin );
  cursor.setPosition( position_begin + text.size(), QTextCursor::KeepAnchor );
  setTextCursor( cursor );

  return;

}

//_______________________________________________________
void DocumentClassTextEditor::rehighlight( void )
{
  Debug::Throw( "DocumentClassTextEditor::rehighlight.\n" );

  // set all block to modified
  for( QTextBlock block = document()->begin(); block.isValid(); block = block.next() )
  { _setBlockModified( block ); }

  textHighlight().setDocument( document() );
  Debug::Throw( "DocumentClassTextEditor::rehighlight. done.\n" );

}

//_______________________________________________________
bool DocumentClassTextEditor::event( QEvent* event )
{

  bool has_block_delimiters( hasBlockDelimiterDisplay() );

  // check that all needed widgets/actions are valid and checked.
  switch (event->type())
  {

    case QEvent::MouseButtonPress:
    if( has_block_delimiters ) blockDelimiterDisplay().mousePressEvent( static_cast<QMouseEvent*>( event ) );
    break;

    default: break;
  }

  return AnimatedTextEditor::event( event );

}

//_______________________________________________________
void DocumentClassTextEditor::keyPressEvent( QKeyEvent* event )
{

  // check if tab key is pressed
  if(
    event->key() == Qt::Key_Tab &&
    indent_->isEnabled() &&
    !( textCursor().hasSelection() || _boxSelection().state() == BoxSelection::FINISHED ) )
  { emit indent( textCursor().block(), false ); }
  else
  {

    // process key
    AnimatedTextEditor::keyPressEvent( event );

    // indent current paragraph when return is pressed
    if( indent_->isEnabled() && event->key() == Qt::Key_Return && !textCursor().hasSelection() )
    { emit indent( textCursor().block(), true ); }

    // reindent paragraph if needed
    /* remark: this is c++ specific. The list of keys should be set in the document class */
    if( indent_->isEnabled() && ( event->key() == Qt::Key_BraceRight || event->key() == Qt::Key_BraceLeft ) && !textCursor().hasSelection() )
    { emit indent( textCursor().block(), false ); }

  }

  return;
}

//________________________________________________
void DocumentClassTextEditor::paintEvent( QPaintEvent* event )
{
  AnimatedTextEditor::paintEvent( event );

  // handle block background
  QTextBlock first( cursorForPosition( event->rect().topLeft() ).block() );
  QTextBlock last( cursorForPosition( event->rect().bottomRight() ).block() );

  // create painter and translate from widget to viewport coordinates
  QPainter painter( viewport() );
  painter.setClipRect( event->rect() );
  painter.translate( -scrollbarPosition() );
  painter.setPen( _marginWidget().palette().color( QPalette::WindowText ) );

  // loop over found blocks
  for( QTextBlock block( first ); block != last.next() && block.isValid(); block = block.next() )
  {
    if( !block.blockFormat().boolProperty( TextBlock::Collapsed ) ) continue;

    QRectF block_rect( document()->documentLayout()->blockBoundingRect( block ) );
    block_rect.setWidth( viewport()->width() + scrollbarPosition().x() );
    QLineF line( QPointF( 0, block_rect.bottomLeft().y() ), block_rect.bottomRight() );
    painter.drawLine( line );
  }
  painter.end();

}

//_____________________________________________________________
void DocumentClassTextEditor::_setBlockModified( const QTextBlock& block )
{
  // check if highlight is enabled.
  if( !textHighlight().isHighlightEnabled() ) return;

  // retrieve associated block data if any
  // set block as modified so that its highlight content gets reprocessed.
  TextBlockData* data( static_cast<TextBlockData*>( block.userData() ) );
  if( data ) data->setFlag( TextBlock::MODIFIED, true );

}

//___________________________________________________________________________
bool DocumentClassTextEditor::_updateMargin( void )
{
  Debug::Throw( "DocumentClassTextEditor::_updateMargin.\n" );

  AnimatedTextEditor::_updateMargin();
  int left_margin( _leftMargin() );

  blockDelimiterDisplay().setOffset( left_margin );
  if( hasBlockDelimiterDisplay() ) left_margin += blockDelimiterDisplay().width();

  return _setLeftMargin( left_margin );

}

//___________________________________________________________________________
void DocumentClassTextEditor::_updateConfiguration( void )
{
  Debug::Throw( "DocumentClassTextEditor::_updateConfiguration.\n" );

  // parenthesis highlight
  textHighlight().setParenthesisHighlightColor( QColor( XmlOptions::get().raw( "PARENTHESIS_COLOR" ) ) );

  {
    QFont font;
    font.fromString( XmlOptions::get().raw( "FIXED_FONT_NAME" ) );
    int line_spacing = QFontMetrics( font ).lineSpacing() + 1;
    blockDelimiterDisplay().setWidth( line_spacing );
    _updateMargin();
  }

}

//_______________________________________________________
void DocumentClassTextEditor::_indentCurrentParagraph( void )
{
  Debug::Throw( "DocumentClassTextEditor::_indentCurrentParagraph.\n" );
  if( !indent_->isEnabled() ) return;
  emit indent( textCursor().block(), false );
}

//_____________________________________________________________
void DocumentClassTextEditor::_setBlockModified( int position, int, int added )
{
  QTextBlock begin( document()->findBlock( position ) );
  QTextBlock end(  document()->findBlock( position + added ) );

  for( QTextBlock block = begin; block.isValid() && block != end; block = block.next() )
  { _setBlockModified( block ); }

  _setBlockModified( end );

}

//__________________________________________________
void DocumentClassTextEditor::_highlightParenthesis( void )
{

  if( !( hasTextHighlight() && textHighlight().isParenthesisEnabled() ) ) return;

  // clear previous parenthesis
  parenthesisHighlight().clear();

  // retrieve TextCursor
  QTextCursor cursor( textCursor() );
  if( cursor.atBlockStart() ) return;

  // retrieve block
  QTextBlock block( cursor.block() );

  // store local position in block
  int position(cursor.position()-block.position());

  // retrieve text block data
  HighlightBlockData *data( dynamic_cast<HighlightBlockData*>( block.userData() ) );
  if( !data ) return;

  QString text( block.text() );
  const TextParenthesis::List& parenthesis( textHighlight().parenthesis() );

  // check against opening parenthesis
  bool found( false );
  TextParenthesis::List::const_iterator iter( find_if(
    parenthesis.begin(), parenthesis.end(),
    TextParenthesis::FirstElementFTor( text.left( position ) ) ) );


  if( iter != parenthesis.end() )
  {
    int increment( 0 );
    while( block.isValid() && !found )
    {
      // retrieve block text
      QString text( block.text() );

      // parse text
      while( (position = iter->regexp().indexIn( text, position ) ) >= 0 )
      {

        if( const_cast<QRegExp&>(iter->regexp()).cap() == iter->second() ) increment--;
        else if( const_cast<QRegExp&>(iter->regexp()).cap() == iter->first() ) increment++;

        if( increment < 0 )
        {
          found = true;
          break;
        }

        position += iter->regexp().matchedLength();

      }

      if( !found )
      {
        block = block.next();
        position = 0;
      }
    }
  }

  // if not found, check against closing parenthesis
  if( !( found || (iter =
    find_if(
    parenthesis.begin(), parenthesis.end(),
    TextParenthesis::SecondElementFTor( text.left( position ) ) )) == parenthesis.end()  ) )
  {

    int increment( 0 );
    position -= (iter->second().size() );
    while( block.isValid() && !found )
    {
      // retrieve block text
      QString text( block.text() );

      // parse text
      while( position >= 0 && (position = iter->regexp().lastIndexIn( text.left(position) ) ) >= 0 )
      {

        if( const_cast<QRegExp&>(iter->regexp()).cap() == iter->first() ) increment--;
        else if( const_cast<QRegExp&>(iter->regexp()).cap() == iter->second() ) increment++;

        if( increment < 0 )
        {
          found = true;
          break;
        }

      }

      if( !found )
      {
        block = block.previous();
        if( block.isValid() ) position = block.text().length() ;
      }

    }
  }

  if( found ) parenthesisHighlight().highlight( position + block.position(), iter->regexp().matchedLength() );

  return;

}
