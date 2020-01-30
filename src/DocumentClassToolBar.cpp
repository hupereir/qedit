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
* Any WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* this program.  If not, see <http://www.gnu.org/licenses/>.
*
*******************************************************************************/

#include "DocumentClassToolBar.h"

#include "Application.h"
#include "Debug.h"
#include "DocumentClass.h"
#include "DocumentClassManager.h"
#include "Singleton.h"

#include <QLabel>

//________________________________________________________________
DocumentClassToolBar::DocumentClassToolBar( QWidget* parent ):
CustomToolBar( tr( "Document Classes" ), parent, "DOCUMENT_CLASS_TOOLBAR" )
{

    Debug::Throw( "DocumentClassToolBar::DocumentClassToolBar.\n" );
    addWidget( combobox_ = new QComboBox( this ) );

    connect( &_comboBox(), QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DocumentClassToolBar::_currentIndexChanged );
    connect( Base::Singleton::get().application<Application>(), &Application::documentClassesChanged, this, &DocumentClassToolBar::_update );
    _update();
}

//________________________________________________________________
void DocumentClassToolBar::update( QString className )
{
    Debug::Throw( "DocumentClassToolBar::update.\n" );
    currentClass_ = className;
    _comboBox().setCurrentIndex( _comboBox().findText( className ) );
}

//________________________________________________________________
void DocumentClassToolBar::_currentIndexChanged( int index )
{
    Debug::Throw( "DocumentClassToolBar::_currentIndexChanged.\n" );
    QString className( _comboBox().itemText( index ) );
    if( className == currentClass_ ) return;
    currentClass_ = className;
    emit documentClassSelected( className );
}

//________________________________________________________________
void DocumentClassToolBar::_update()
{
    Debug::Throw( "DocumentClassToolBar::_update.\n" );

    // store current item
    QString currentClass( _comboBox().currentText() );

    // clear box
    _comboBox().clear();

    // add all document classes
    auto manager( Base::Singleton::get().application<Application>()->classManager() );
    auto classes( manager.classes() );
    for( const auto& documentClass:classes )
    { _comboBox().addItem( documentClass.name() ); }

    // try select old class if possible
    if( classes.size() > _comboBox().maxVisibleItems() )
    { _comboBox().setMaxVisibleItems( classes.size() ); }

    _comboBox().setCurrentIndex( _comboBox().findText( currentClass ) );

}
