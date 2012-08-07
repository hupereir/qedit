
// $Id$

/******************************************************************************
*
* Copyright (C) 2002 Hugo PEREIRA <mailto: hugo.pereira@free.fr>
*
* This is free software; you can redistribute it and/or modify it under the
* terms of the GNU General Public License as published by the Free Software
* Foundation; either version 2 of the License, or (at your option) any later
* version.
*
* This software is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
*******************************************************************************/

#include "GridLayout.h"
#include "AnimatedLineEditor.h"
#include "TextEditor.h"
#include "HighlightPatternDialog.h"
#include "HighlightPatternOptions.h"
#include "HighlightPatternType.h"
#include "InformationDialog.h"

#include <QtGui/QLabel>
#include <QtGui/QLayout>

//________________________________________________________________________
HighlightPatternDialog::HighlightPatternDialog( QWidget* parent ):
CustomDialog( parent )
{

    Debug::Throw( "HighlightPatternDialog::HighlightPatternDialog.\n" );
    setWindowTitle( "Highlight Pattern Settings - Qedit" );

    mainLayout().setSpacing(5);

    // name
    GridLayout* gridLayout( new GridLayout() );
    gridLayout->setSpacing( 5 );
    gridLayout->setMargin( 0 );
    gridLayout->setMaxCount( 2 );
    gridLayout->setColumnAlignment( 0, Qt::AlignRight|Qt::AlignVCenter );
    mainLayout().addLayout( gridLayout );

    gridLayout->addWidget( new QLabel( "Name: ", this ) );
    gridLayout->addWidget( nameEditor_ = new AnimatedLineEditor( this ) );

    // parent
    gridLayout->addWidget( new QLabel( "Parent pattern: ", this ) );
    gridLayout->addWidget( parentComboBox_ = new QComboBox( this ) );
    parentComboBox_->setEditable( false );
    parentComboBox_->addItem( HighlightPattern::noParentPattern_ );

    // styles
    gridLayout->addWidget( new QLabel( "Highlight style: ", this ) );
    gridLayout->addWidget( styleComboBox_ = new QComboBox( this ) );
    styleComboBox_->setEditable( false );

    // type
    gridLayout->addWidget( new QLabel( "Type: ", this ) );
    gridLayout->addWidget( patternType_ = new HighlightPatternType( this ) );

    // regular expressions
    gridLayout->addWidget( new QLabel( "Regular expression to match: ", this ) );
    gridLayout->addWidget( keywordRegexpEditor_ = new AnimatedLineEditor( this ) );

    gridLayout->addWidget( endRegexpLabel_ = new QLabel( "Ending regular expression: ", this ) );
    gridLayout->addWidget( endRegexpEditor_ = new AnimatedLineEditor( this ) );

    // options
    mainLayout().addWidget( patternOptions_ = new HighlightPatternOptions( this ) );
    connect( patternType_, SIGNAL( typeChanged( HighlightPattern::Type ) ), SLOT( _updateEditors( HighlightPattern::Type ) ) );

    // comments
    mainLayout().addWidget( new QLabel( "Comments: ", this ) );
    mainLayout().addWidget( commentsEditor_ = new TextEditor( this ) );

    _updateEditors( patternType_->type() );

}

//________________________________________________________________________
void HighlightPatternDialog::setPatterns( const HighlightPattern::List& patterns )
{
    Debug::Throw( "HighlightPatternDialog::setPatterns.\n" );

    // update parent_combobox
    parentComboBox_->clear();
    parentComboBox_->addItem( HighlightPattern::noParentPattern_ );
    for( HighlightPattern::List::const_iterator iter = patterns.begin(); iter != patterns.end(); ++iter )
    { parentComboBox_->addItem( iter->name() ); }

    //! select default parent pattern
    parentComboBox_->setCurrentIndex( parentComboBox_->findText( HighlightPattern::noParentPattern_ ) );
}

//________________________________________________________________________
void HighlightPatternDialog::setStyles( const HighlightStyle::Set& styles )
{
    Debug::Throw( "HighlightPatternDialog::setStyles.\n" );

    styles_ = styles;

    // update style_combobox
    styleComboBox_->clear();
    for( HighlightStyle::Set::const_iterator iter = styles_.begin(); iter != styles_.end(); ++iter )
    { styleComboBox_->addItem( iter->name() ); }

    return;

}

//________________________________________________________________________
void HighlightPatternDialog::setPattern( const HighlightPattern& pattern )
{
    Debug::Throw( "HighlightPatternDialog::setPattern.\n" );

    pattern_ = pattern;

    nameEditor_->setText( pattern_.name() );
    patternType_->setType( pattern_.type() );
    patternOptions_->setOptions( pattern_.flags() );

    // regular expressions
    keywordRegexpEditor_->setText( pattern_.keyword().pattern() );

    if( pattern.type() == HighlightPattern::RANGE_PATTERN )
    { endRegexpEditor_->setText( pattern_.end().pattern() ); }

    // style
    styleComboBox_->setCurrentIndex( styleComboBox_->findText( pattern_.style().name() ) );

    // parent
    if( pattern.parent().isEmpty() ) { parentComboBox_->setCurrentIndex( parentComboBox_->findText( HighlightPattern::noParentPattern_ ) ); }
    else { parentComboBox_->setCurrentIndex( parentComboBox_->findText( pattern_.parent() ) ); }

    // comments
    commentsEditor_->setPlainText( pattern.comments() );

    // editors enability
    _updateEditors( patternType_->type() );

    return;
}

//________________________________________________________________________
HighlightPattern HighlightPatternDialog::pattern( void )
{

    Debug::Throw( "HighlightPatternDialog::pattern.\n" );
    pattern_.setName( nameEditor_->text() );

    QString parent( parentComboBox_->itemText( parentComboBox_->currentIndex() ) );
    pattern_.setParent( parent == HighlightPattern::noParentPattern_ ? "":parent );

    // style
    HighlightStyle::Set::iterator styleIter ( styles_.find( HighlightStyle( styleComboBox_->itemText( styleComboBox_->currentIndex() ) ) ) );
    if( styleIter != styles_.end() ) pattern_.setStyle( *styleIter );
    else InformationDialog( this, "invalid style name" ).exec();

    pattern_.setFlags( patternOptions_->options() );
    pattern_.setType( patternType_->type() );

    pattern_.setKeyword( keywordRegexpEditor_->text() );
    if( pattern_.type() == HighlightPattern::RANGE_PATTERN )
    { pattern_.setEnd( endRegexpEditor_->text() ); }

    pattern_.setComments( commentsEditor_->toPlainText() );

    return pattern_;

}

//________________________________________________________________________
void HighlightPatternDialog::_updateEditors( HighlightPattern::Type type )
{
    Debug::Throw( "HighlightPatternDialog::_updateEditors.\n" );
    endRegexpLabel_->setEnabled( type == HighlightPattern::RANGE_PATTERN );
    endRegexpEditor_->setEnabled( type == HighlightPattern::RANGE_PATTERN );
}
