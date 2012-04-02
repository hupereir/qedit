
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
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
*******************************************************************************/

#include "AnimatedLineEditor.h"
#include "BrowsedLineEditor.h"
#include "GridLayout.h"
#include "DocumentClassConfiguration.h"

#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLayout>

//____________________________________________________
DocumentClassConfiguration::DocumentClassConfiguration( QWidget* parent ):
QWidget( parent ),
Counter( "DocumentClassConfiguration" )
{
    Debug::Throw( "DocumentClassConfiguration::DocumentClassConfiguration.\n" );

    setLayout( new QVBoxLayout() );
    layout()->setSpacing(5);
    layout()->setMargin(5);

    // flags
    QGroupBox* box;
    layout()->addWidget( box = new QGroupBox( "Class definition", this ) );

    GridLayout* grid_layout = new GridLayout();
    grid_layout->setSpacing(5);
    grid_layout->setMargin(5);
    grid_layout->setMaxCount(2);
    grid_layout->setColumnAlignment( 0, Qt::AlignRight|Qt::AlignVCenter );
    box->setLayout( grid_layout );

    // file editor
    grid_layout->addWidget( new QLabel( "File: ", this ) );
    grid_layout->addWidget( fileEditor_ = new BrowsedLineEditor( this ) );
    fileEditor_->setAcceptMode( QFileDialog::AcceptSave );
    fileEditor_->setFileMode( QFileDialog::AnyFile );

    // name editor
    grid_layout->addWidget( new QLabel( "Name: ", this ) );
    grid_layout->addWidget( nameEditor_ = new AnimatedLineEditor( this ) );

    // icon editor
    grid_layout->addWidget( new QLabel( "Icon: ", this ) );
    grid_layout->addWidget( iconEditor_ = new AnimatedLineEditor( this ) );

    // patterns
    layout()->addWidget( box = new QGroupBox( "Matching patterns", this ) );

    grid_layout = new GridLayout();
    grid_layout->setSpacing(5);
    grid_layout->setMargin(5);
    grid_layout->setMaxCount(2);
    grid_layout->setColumnAlignment( 0, Qt::AlignRight|Qt::AlignVCenter );
    box->setLayout( grid_layout );

    grid_layout->addWidget( new QLabel( "File name pattern: ", this ) );
    grid_layout->addWidget( filePatternEditor_ = new AnimatedLineEditor( this ) );
    filePatternEditor_->setToolTip( "Regular expression used to determine document class from file name." );

    grid_layout->addWidget( new QLabel( "First line pattern: ", this ) );
    grid_layout->addWidget( firstLinePatternEditor_ = new AnimatedLineEditor( this ) );
    firstLinePatternEditor_->setToolTip( "Regular expression used to determine document class from the first line of the file." );

    // options
    layout()->addWidget( box = new QGroupBox( "Options", this ) );

    box->setLayout( new QVBoxLayout() );
    box->layout()->setMargin(5);
    box->layout()->setSpacing(5);

    // default
    box->layout()->addWidget( defaultCheckBox_ = new QCheckBox( "Default", box ) );
    defaultCheckBox_->setToolTip( "Use this document class when no other is found that match a given file" );
    defaultCheckBox_->setChecked( false );

    // wrap mode
    box->layout()->addWidget( wrapCheckBox_ = new QCheckBox( "Wrap", box ) );
    wrapCheckBox_->setChecked( false );

    // tab emulation
    box->layout()->addWidget( tabEmulationCheckbox_ = new QCheckBox( "Emulate tabs", box ) );
    tabEmulationCheckbox_->setChecked( false );

    QHBoxLayout *layout = new QHBoxLayout();
    layout->setMargin(0);
    layout->setSpacing(5);
    box->layout()->addItem( layout );

    grid_layout = new GridLayout();
    grid_layout->setSpacing(5);
    grid_layout->setMargin(0);
    grid_layout->setMaxCount(2);
    grid_layout->setColumnAlignment( 0, Qt::AlignRight|Qt::AlignVCenter );
    layout->addLayout( grid_layout );
    layout->addStretch(1);

    QLabel* label = new QLabel( "Tab size: ", box );
    grid_layout->addWidget( label );

    tabSizeSpinBox_ = new QSpinBox( box );
    tabSizeSpinBox_->setMinimum( 0 );
    tabSizeSpinBox_->setMaximum( 20 );
    tabSizeSpinBox_->setToolTip(
        "Tab size (in unit of space characters).\n "
        "When zero value is specified, the default qedit tab size is used." );
    label->setBuddy( tabSizeSpinBox_ );
    grid_layout->addWidget( tabSizeSpinBox_ );

    // base indentation
    grid_layout->addWidget( new QLabel( "Base indentation: ", box ) );
    grid_layout->addWidget( baseIndentationSpinBox_ = new QSpinBox( box ) );
    baseIndentationSpinBox_->setMinimum(0);
    baseIndentationSpinBox_->setValue(0);

}

//_________________________________________________________________________
void DocumentClassConfiguration::setDocumentClass( const DocumentClass& document_class )
{

    // name
    nameEditor_->setText( document_class.name() );

    // icon
    iconEditor_->setText( document_class.icon() );

    // file editor
    fileEditor_->editor().setText( document_class.file() );

    // file name matching pattern
    filePatternEditor_->setText( document_class.fileMatchingPattern().pattern() );

    // first line matching pattern
    firstLinePatternEditor_->setText( document_class.firstLineMatchingPattern().pattern() );

    // base indentation
    baseIndentationSpinBox_->setValue( document_class.baseIndentation() );

    // flags
    defaultCheckBox_->setChecked( document_class.isDefault() );
    wrapCheckBox_->setChecked( document_class.wrap() );
    tabEmulationCheckbox_->setChecked( document_class.emulateTabs() );
    tabSizeSpinBox_->setValue( document_class.tabSize() );

}

//_________________________________________________________________________
DocumentClass DocumentClassConfiguration::documentClass( void )
{
    DocumentClass out;
    out.setName( nameEditor_->text() );
    out.setIcon( iconEditor_->text() );
    out.setFile( fileEditor_->editor().text() );
    out.setFileMatchingPattern( filePatternEditor_->text() );
    out.setFirstLineMatchingPattern( firstLinePatternEditor_->text() );
    out.setBaseIndentation( baseIndentationSpinBox_->value() );
    out.setWrap( wrapCheckBox_->isChecked() );
    out.setEmulateTabs( tabEmulationCheckbox_->isChecked() );
    out.setTabSize( tabSizeSpinBox_->value() );
    out.setIsDefault( defaultCheckBox_->isChecked() );
    return out;
}
