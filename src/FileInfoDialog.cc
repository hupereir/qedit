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

/*!
   \file FileInfoDialog.cc
   \brief  file informations
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QLabel>
#include <QTabWidget>
#include <QCheckBox>
#include <sys/stat.h>

#include "Config.h"
#include "CustomPixmap.h"
#include "CustomGridLayout.h"
#include "Debug.h"
#include "Icons.h"
#include "FileInfoDialog.h"
#include "XmlOptions.h"
#include "QtUtil.h"
#include "TextDisplay.h"
#include "TimeStamp.h"

using namespace std;

//_________________________________________________________
FileInfoDialog::FileInfoDialog( TextDisplay* parent ):
  QDialog( parent )
{
  Debug::Throw( "FileInfoDialog::FileInfoDialog.\n" );

  // tell frame to delete on exit
  setAttribute( Qt::WA_DeleteOnClose );

  //! try load Question icon
  static CustomPixmap pixmap;
  static bool first( true );
  if( first )
  {
    first = false;
    list<string> path_list( XmlOptions::get().specialOptions<string>( "PIXMAP_PATH" ) );
    pixmap.find( ICONS::INFO, path_list );    
  }
  
  QBoxLayout* layout = new QHBoxLayout();
  layout->setMargin(10);
  layout->setSpacing(10);
  setLayout( layout );
  
  QLabel* label = new QLabel(this);
  label->setPixmap( pixmap );
  layout->addWidget( label );

  QTabWidget *tab_widget = new QTabWidget( this );
  layout->addWidget( tab_widget );
    
  // box to display additinal information
  QWidget *box = new QWidget( tab_widget );
  layout = new QVBoxLayout();
  layout->setMargin(5);
  layout->setSpacing( 5 );
  box->setLayout( layout );
  tab_widget->addTab( box, "&General" );
  
  CustomGridLayout* grid_layout = new CustomGridLayout();
  grid_layout->setSpacing( 5 );
  grid_layout->setMaxCount( 2 );
  layout->addLayout( grid_layout );
  
  // file name  
  const File& file( parent->file() ); 
  
  grid_layout->addWidget( label = new QLabel( "file: ", box ) );
  grid_layout->addWidget( label = new QLabel( file.empty() ? "untitled":file.localName().c_str(), box ) );
  QFont font( label->font() );
  font.setWeight( QFont::Bold );
  label->setFont( font );
  
  // path
  if( !file.empty() )
  { 
    grid_layout->addWidget( label = new QLabel( "path: ", box ) );
    grid_layout->addWidget( label = new QLabel( file.path().c_str(), box ) );
  }
      
  // size
  grid_layout->addWidget( label = new QLabel( "size: ", box ) );
  grid_layout->addWidget( label = new QLabel( file.exist() ? file.sizeString().c_str(): "0", box ) );
  
  // last accessed
  grid_layout->addWidget( label = new QLabel( "last accessed: ", box ) );
  grid_layout->addWidget( label = new QLabel( file.exist() ? TimeStamp( file.lastAccessed() ).string().c_str():"never", box ) );

  // last modification
  grid_layout->addWidget( label = new QLabel( "last modified: ", box ) );
  grid_layout->addWidget( label = new QLabel( file.exist() ? TimeStamp( file.lastModified() ).string().c_str():"never", box ) );
  
  // document class
  const string& class_name( parent->className() );
  if( !class_name.empty() )
  {
    grid_layout->addWidget( label = new QLabel( "document class: ", box ) );
    grid_layout->addWidget( label = new QLabel( class_name.c_str(), box ) );
  }
  
  grid_layout->setColumnStretch( 1, 1 );
  
  layout->addStretch( 1 );
  
  // permissions tab
  box = new QWidget( tab_widget );
  layout = new QVBoxLayout();
  layout->setMargin(5);
  layout->setSpacing( 5 );
  box->setLayout( layout );
  tab_widget->addTab( box, "Permissions" );
  
  box->layout()->addWidget( new QLabel( "<b>Permissions: </b>", box ) );
  
  grid_layout = new CustomGridLayout();
  grid_layout->setMargin(0);
  grid_layout->setSpacing( 5 );
  grid_layout->setMaxCount( 4 );
  box->layout()->addItem( grid_layout );
  
  grid_layout->addWidget( new QLabel( "", box ) );
  grid_layout->addWidget( new QLabel( "Read", box ), Qt::AlignHCenter );
  grid_layout->addWidget( new QLabel( "Write", box ), Qt::AlignHCenter ); 
  grid_layout->addWidget( new QLabel( "Execute", box ), Qt::AlignHCenter ); 
  
  grid_layout->addWidget( new QLabel( "User permissions", box ) );
  
  typedef std::map<mode_t, QCheckBox* > CheckBoxMap;
  CheckBoxMap checkboxes;
  grid_layout->addWidget( checkboxes[S_IRUSR] = new QCheckBox( box ), Qt::AlignHCenter ); 
  grid_layout->addWidget( checkboxes[S_IWUSR] = new QCheckBox( box ), Qt::AlignHCenter ); 
  grid_layout->addWidget( checkboxes[S_IXUSR] = new QCheckBox( box ), Qt::AlignHCenter ); 

  grid_layout->addWidget( new QLabel( "Group permissions", box ) );
  grid_layout->addWidget( checkboxes[S_IRGRP] = new QCheckBox( box ), Qt::AlignHCenter );  
  grid_layout->addWidget( checkboxes[S_IWGRP] = new QCheckBox( box ), Qt::AlignHCenter );  
  grid_layout->addWidget( checkboxes[S_IXGRP] = new QCheckBox( box ), Qt::AlignHCenter );  

  grid_layout->addWidget( new QLabel( "Others permissions", box ) );
  grid_layout->addWidget( checkboxes[S_IROTH] = new QCheckBox( box ), Qt::AlignHCenter );  
  grid_layout->addWidget( checkboxes[S_IWOTH] = new QCheckBox( box ), Qt::AlignHCenter );  
  grid_layout->addWidget( checkboxes[S_IXOTH] = new QCheckBox( box ), Qt::AlignHCenter );  
  
  mode_t permissions = file.permissions();
  for( CheckBoxMap::iterator iter = checkboxes.begin(); iter != checkboxes.end(); iter++ )
  { 
    iter->second->setChecked( permissions & iter->first ); 
    iter->second->setEnabled( false );
  }
  
  // group and user id
  if( file.exist() )
  {
   
    box->layout()->addWidget( new QLabel( "<b>Ownership: </b>", box ) );

    grid_layout = new CustomGridLayout();
    grid_layout->setMargin(0);
    grid_layout->setSpacing( 5 );
    grid_layout->setMaxCount( 2 );
    box->layout()->addItem( grid_layout );
    
    // user id
    grid_layout->addWidget( label = new QLabel( "user: ", box ) );
    grid_layout->addWidget( label = new QLabel( file.userName().c_str(), box ) );
    
    // group id
    grid_layout->addWidget( label = new QLabel( "group: ", box ) );
    grid_layout->addWidget( label = new QLabel( file.groupName().c_str(), box ) );
    grid_layout->setColumnStretch( 1, 1 );
    
  }
    
  // misc tab
  // permissions tab
  box = new QWidget( tab_widget );
  layout = new QVBoxLayout();
  layout->setMargin(5);
  layout->setSpacing( 5 );
  box->setLayout( layout );

  tab_widget->addTab( box, "&Miscellaneous" );
  
  grid_layout = new CustomGridLayout();
  grid_layout->setMargin(0);
  grid_layout->setSpacing( 5 );
  grid_layout->setMaxCount( 2 );
  layout->addLayout( grid_layout );
    
  // number of characters
  grid_layout->addWidget( new QLabel( "number of characters: ", box ) );
  grid_layout->addWidget( new QLabel( Str().assign<int>(parent->toPlainText().size()).c_str(), box ) );
  
  // number of lines
  grid_layout->addWidget( new QLabel( "number of lines: ", box ) );
  grid_layout->addWidget( new QLabel( Str().assign<int>(parent->blockCount()).c_str(), box ) );
  
  grid_layout->setColumnStretch( 1, 1 );
    
}
