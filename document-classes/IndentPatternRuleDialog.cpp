
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
#include "IndentPatternRuleDialog.h"

#include <QtGui/QLabel>
#include <QtGui/QLayout>

//________________________________________________________________________
IndentPatternRuleDialog::IndentPatternRuleDialog( QWidget* parent ):
CustomDialog( parent, OkButton|CancelButton|CustomDialog::Separator )
{

    Debug::Throw( "IndentPatternRuleDialog::IndentPatternRuleDialog.\n" );

    setWindowTitle( "Indentation Pattern Settings - Qedit" );

    mainLayout().setSpacing(5);

    GridLayout* grid_layout( new GridLayout() );
    grid_layout->setSpacing( 5 );
    grid_layout->setMargin( 0 );
    grid_layout->setMaxCount( 2 );
    grid_layout->setColumnAlignment( 0, Qt::AlignRight|Qt::AlignVCenter );
    mainLayout().addLayout( grid_layout );

    //
    grid_layout->addWidget( new QLabel( "Regular expression to match: ", this ) );
    grid_layout->addWidget( regexpEditor_ = new AnimatedLineEditor( this ) );

    grid_layout->addWidget( new QLabel( "Paragraph to match (before current): ", this ) );
    grid_layout->addWidget( paragraphSpinBox_ = new QSpinBox( this ) );
    paragraphSpinBox_->setMinimum( -10 );
    paragraphSpinBox_->setMaximum( 0 );
    paragraphSpinBox_->setValue( 0 );

    mainLayout().addWidget( caseCheckBox_ = new QCheckBox( "Case sensitivity", this ) );
    caseCheckBox_->setChecked( true );

}

//________________________________________________________________________
void IndentPatternRuleDialog::setRule( const IndentPattern::Rule& rule )
{
    Debug::Throw( "IndentPatternRuleDialog::setRule.\n" );

    regexpEditor_->setText( rule.pattern().pattern() );
    paragraphSpinBox_->setValue( rule.paragraph() );
    caseCheckBox_->setChecked( !rule.flag( IndentPattern::Rule::CASE_INSENSITIVE ) );
    return;
}

//________________________________________________________________________
IndentPattern::Rule IndentPatternRuleDialog::rule( void )
{

    Debug::Throw( "IndentPatternRuleDialog::parenthesis.\n" );
    IndentPattern::Rule out;
    out.setPattern( regexpEditor_->text() );
    out.setParagraph( paragraphSpinBox_->value() );
    out.setFlag( IndentPattern::Rule::CASE_INSENSITIVE, !caseCheckBox_->isChecked() );

    return out;

}
