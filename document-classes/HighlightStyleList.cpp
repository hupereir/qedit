
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
 
/*!
  \file HighlightStyleList.h
  \brief List box for HighlightStyles
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QLayout>

#include "ListWidget.h"
#include "HighlightStyleDialog.h"
#include "HighlightStyleList.h"
#include "HighlightStyleItem.h"
#include "QtUtil.h"

using namespace std;

//____________________________________________________
HighlightStyleList::HighlightStyleList( QWidget* parent ):
  QWidget( parent ),
  Counter( "HighlightStyleList" ),
  modified_( false )
{
  Debug::Throw( "HighlightStyleList::HighlightStyleList.\n" );
  
  QVBoxLayout* v_layout = new QVBoxLayout();
  v_layout->setSpacing(5);
  v_layout->setMargin(10);
  setLayout( v_layout );
  v_layout->addWidget( list_ = new ListWidget( this ), 1 );
  connect( list_, SIGNAL( itemSelectionChanged() ), SLOT( _updateButtons() ) );
  connect( list_, SIGNAL( itemActivated( QListWidgetItem* ) ), SLOT( _edit() ) ); 
  
  QPushButton* button;
  v_layout->addWidget( button = new QPushButton( "&Add", this ) );
  button->setToolTip( "Add a new highlight style to the list" );
  connect( button, SIGNAL( clicked() ), SLOT( _add() ) );
  
  v_layout->addWidget( edit_button_ = new QPushButton( "&Edit", this ) );
  edit_button_->setToolTip( "Edit selected highlight style" );
  connect( edit_button_, SIGNAL( clicked() ), SLOT( _edit() ) );

  v_layout->addWidget( remove_button_ = new QPushButton( "&Remove", this ) );
  remove_button_->setToolTip( "Remove selected highlight style" );
  connect( remove_button_, SIGNAL( clicked() ), SLOT( _remove() ) );
  
  _updateButtons();
  
}

//____________________________________________________
void HighlightStyleList::setStyles( const HighlightStyle::Set& styles ) 
{

  Debug::Throw( "HighlightStyleList::setStyles.\n" );
  list_->clear();
  
  for( HighlightStyle::Set::const_iterator iter = styles.begin(); iter != styles.end(); iter++ )
  { new HighlightStyleItem( list_, *iter ); }
  
  modified_ = false;
  
}

//____________________________________________________
HighlightStyle::Set HighlightStyleList::styles( void ) 
{
  
  Debug::Throw( "HighlightStyleList::styles.\n" );
  HighlightStyle::Set out;
  QList<HighlightStyleItem*> items( list_->items<HighlightStyleItem>() );
  for( QList<HighlightStyleItem*>::iterator iter( items.begin() ); iter != items.end(); iter++ )
  { out.insert( (*iter)->style() ); }
  
  return out;
  
}

//____________________________________________________
void HighlightStyleList::_updateButtons( void )
{
  Debug::Throw( "HighlightStyleList::_updateButtons.\n" );
  bool has_selection( !list_->QListWidget::selectedItems().empty() );  
  edit_button_->setEnabled( has_selection );
  remove_button_->setEnabled( has_selection );
}

//____________________________________________________
void HighlightStyleList::_add( void )
{
  Debug::Throw( "HighlightStyleList::_add.\n" );
}

//____________________________________________________
void HighlightStyleList::_edit( void )
{
  Debug::Throw( "HighlightStyleList::_edit.\n" );
 
  HighlightStyleItem* item( list_->currentItem<HighlightStyleItem>() );
  if( !item )
  {
    QtUtil::infoDialog( this, "No item selected. <Edit> canceled." );
    return;
  }  
  
  HighlightStyleDialog dialog( this );
  dialog.setStyle( item->style() );
  if( dialog.exec() == QDialog::Rejected ) 
  { return; }
  
  HighlightStyle style( dialog.style() );
  if( style.differs( item->style() ) ) 
  {
    item->update( dialog.style() );
    modified_ = true; 
  }
  
}

//____________________________________________________
void HighlightStyleList::_remove( void )
{
  Debug::Throw( "HighlightStyleList::_remove.\n" );

  HighlightStyleItem* item( list_->currentItem<HighlightStyleItem>() );
  if( !item )
  {
    QtUtil::infoDialog( this, "No item selected. <Remove> canceled." );
    return;
  }  
  
  delete item;
  modified_ = true;
  
}
