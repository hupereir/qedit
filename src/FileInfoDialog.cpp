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
   \file FileInfoDialog.cpp
   \brief  file informations
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QLabel>
#include <QTabWidget>
#include <QCheckBox>
#include <QPushButton>

#include "AutoSaveThread.h"
#include "Config.h"
#include "CustomPixmap.h"
#include "GridLayout.h"
#include "Debug.h"
#include "FileList.h"
#include "FileRecord.h"
#include "IconEngine.h"
#include "Icons.h"
#include "FileInfoDialog.h"
#include "FileRecordProperties.h"
#include "QtUtil.h"
#include "TextDisplay.h"
#include "TextHighlight.h"
#include "TimeStamp.h"

using namespace std;

//_________________________________________________________
FileInfoDialog::FileInfoDialog( TextDisplay* parent, FileList& file_list ):
  BaseDialog( parent )
{
  Debug::Throw( "FileInfoDialog::FileInfoDialog.\n" );

  setLayout( new QVBoxLayout() );
  layout()->setSpacing(10);
  layout()->setMargin(10);

  QTabWidget *tab_widget = new QTabWidget( this );
  layout()->addWidget( tab_widget );
  Debug::Throw( "FileInfoDialog::FileInfoDialog - tabWidget booked.\n" );
    
  // box to display additinal information
  QWidget *box;
  tab_widget->addTab( box = new QWidget(), "&General" );
  Debug::Throw( "FileInfoDialog::FileInfoDialog - general tab created.\n" );

  QHBoxLayout* h_layout = new QHBoxLayout();
  h_layout->setMargin(10);
  h_layout->setSpacing(10);
  box->setLayout( h_layout );
  
  //! try load Question icon
  CustomPixmap pixmap = CustomPixmap().find( ICONS::INFORMATION );  

  QLabel* label = new QLabel(box);
  label->setPixmap( pixmap );
  h_layout->addWidget( label, 0, Qt::AlignTop );

  QVBoxLayout* layout = new QVBoxLayout();
  layout->setMargin(0);
  layout->setSpacing( 5 );
  h_layout->addLayout( layout, 1 );
  
  GridLayout* grid_layout = new GridLayout();
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
  grid_layout->addWidget( label = new QLabel( file.exists() ? file.sizeString().c_str(): "0", box ) );
  
  // last accessed
  grid_layout->addWidget( label = new QLabel( "last accessed: ", box ) );
  grid_layout->addWidget( label = new QLabel( file.exists() ? TimeStamp( file.lastAccessed() ).string().c_str():"never", box ) );

  // last modification
  grid_layout->addWidget( label = new QLabel( "last modified: ", box ) );
  grid_layout->addWidget( label = new QLabel( file.exists() ? TimeStamp( file.lastModified() ).string().c_str():"never", box ) );
  
  // document class
  const QString& class_name( parent->className() );
  if( !class_name.isEmpty() )
  {
    grid_layout->addWidget( label = new QLabel( "document class: ", box ) );
    grid_layout->addWidget( label = new QLabel( class_name, box ) );
  }
  
  FileRecord record( file_list.get( file ) );
  if( record.hasProperty( FileRecordProperties::DICTIONARY ) )
  {
    grid_layout->addWidget( label = new QLabel( "spell-check dictionary: ", box ) );
    grid_layout->addWidget( label = new QLabel( record.property( FileRecordProperties::DICTIONARY ).c_str(), box ) );
  }

  if( record.hasProperty( FileRecordProperties::FILTER ) )
  {
    grid_layout->addWidget( label = new QLabel( "spell-check filter: ", box ) );
    grid_layout->addWidget( label = new QLabel( record.property( FileRecordProperties::FILTER ).c_str(), box ) );
  }
  
  grid_layout->setColumnStretch( 1, 1 );
  
  layout->addStretch( 1 );
  Debug::Throw( "FileInfoDialog::FileInfoDialog - General tab filled.\n" );
  
  // permissions tab
  tab_widget->addTab( box = new QWidget(), "Permissions" );
  layout = new QVBoxLayout();
  layout->setMargin(5);
  layout->setSpacing( 5 );
  box->setLayout( layout );
  Debug::Throw( "FileInfoDialog::FileInfoDialog - Permissions tab created.\n" );
  
  layout->addWidget( new QLabel( "<b>Permissions: </b>", box ) );
  
  grid_layout = new GridLayout();
  grid_layout->setMargin(0);
  grid_layout->setSpacing( 5 );
  grid_layout->setMaxCount( 4 );
  layout->addItem( grid_layout );
  
  grid_layout->addWidget( new QLabel( "", box ) );
  grid_layout->addWidget( new QLabel( "Read", box ), Qt::AlignHCenter );
  grid_layout->addWidget( new QLabel( "Write", box ), Qt::AlignHCenter ); 
  grid_layout->addWidget( new QLabel( "Execute", box ), Qt::AlignHCenter ); 
    
  typedef std::map< QFile::Permission, QCheckBox* > CheckBoxMap;
  CheckBoxMap checkboxes;

  grid_layout->addWidget( new QLabel( "Owner permissions", box ) );
  grid_layout->addWidget( checkboxes[QFile::ReadOwner ]  = new QCheckBox( box ), Qt::AlignHCenter ); 
  grid_layout->addWidget( checkboxes[QFile::WriteOwner ]  = new QCheckBox( box ), Qt::AlignHCenter ); 
  grid_layout->addWidget( checkboxes[QFile::ExeOwner  ]  = new QCheckBox( box ), Qt::AlignHCenter ); 

  // on unix, right now, Qt does not return the current user permissions. Disable them from the dialog  
  #ifndef Q_WS_X11
  grid_layout->addWidget( new QLabel( "User permissions", box ) );
  grid_layout->addWidget( checkboxes[QFile::ReadUser ]  = new QCheckBox( box ), Qt::AlignHCenter ); 
  grid_layout->addWidget( checkboxes[QFile::WriteUser]  = new QCheckBox( box ), Qt::AlignHCenter ); 
  grid_layout->addWidget( checkboxes[QFile::ExeUser  ]  = new QCheckBox( box ), Qt::AlignHCenter ); 
  #endif
  
  grid_layout->addWidget( new QLabel( "Group permissions", box ) );
  grid_layout->addWidget( checkboxes[QFile::ReadGroup  ] = new QCheckBox( box ), Qt::AlignHCenter );  
  grid_layout->addWidget( checkboxes[QFile::WriteGroup ] = new QCheckBox( box ), Qt::AlignHCenter );  
  grid_layout->addWidget( checkboxes[QFile::ExeGroup   ] = new QCheckBox( box ), Qt::AlignHCenter );  

  grid_layout->addWidget( new QLabel( "Others permissions", box ) );
  grid_layout->addWidget( checkboxes[QFile::ReadOther  ] = new QCheckBox( box ), Qt::AlignHCenter );  
  grid_layout->addWidget( checkboxes[QFile::WriteOther ] = new QCheckBox( box ), Qt::AlignHCenter );  
  grid_layout->addWidget( checkboxes[QFile::ExeOther   ] = new QCheckBox( box ), Qt::AlignHCenter );  
  
  QFile::Permissions permissions( file.permissions() );
  for( CheckBoxMap::iterator iter = checkboxes.begin(); iter != checkboxes.end(); iter++ )
  { 
    iter->second->setChecked( permissions & iter->first ); 
    iter->second->setEnabled( false );
  }
  
  #ifdef Q_WS_X11
  // add a read-only checkbox since user permissions may not be available
  grid_layout->addWidget( new QLabel( "read-only", box ) );
  QCheckBox* checkbox( new QCheckBox( box ) );
  grid_layout->addWidget( checkbox );
  checkbox->setEnabled( false );
  checkbox->setChecked( parent->isReadOnly() );
  #endif
  
  // group and user id
  if( file.exists() )
  {
   
    layout->addWidget( new QLabel( "<b>Ownership: </b>", box ) );

    grid_layout = new GridLayout();
    grid_layout->setMargin(0);
    grid_layout->setSpacing( 5 );
    grid_layout->setMaxCount( 2 );
    layout->addItem( grid_layout );
    
    // user id
    grid_layout->addWidget( label = new QLabel( "owner: ", box ) );
    grid_layout->addWidget( label = new QLabel( file.userName().c_str(), box ) );
    
    // group id
    grid_layout->addWidget( label = new QLabel( "group: ", box ) );
    grid_layout->addWidget( label = new QLabel( file.groupName().c_str(), box ) );
    grid_layout->setColumnStretch( 1, 1 );
    
  }
  
  layout->addStretch(1);

  Debug::Throw( "FileInfoDialog::FileInfoDialog - Permissions tab filled.\n" );

  // misc tab
  // permissions tab
  tab_widget->addTab( box = new QWidget(), "&Miscellaneous" );
  layout = new QVBoxLayout();
  layout->setMargin(5);
  layout->setSpacing( 5 );
  box->setLayout( layout );
  Debug::Throw( "FileInfoDialog::FileInfoDialog - Miscellaneous tab booked.\n" );
  
  grid_layout = new GridLayout();
  grid_layout->setMargin(0);
  grid_layout->setSpacing( 5 );
  grid_layout->setMaxCount( 2 );
  layout->addLayout( grid_layout );
    
  // number of characters
  grid_layout->addWidget( new QLabel( "number of characters: ", box ) );
  grid_layout->addWidget( new QLabel( Str().assign<int>(parent->toPlainText().size()).c_str(), box ) );
  
  // number of lines
  grid_layout->addWidget( new QLabel( "number of lines: ", box ) );
  grid_layout->addWidget( new QLabel( Str().assign<int>(parent->TextEditor::blockCount()).c_str(), box ) );

  grid_layout->addWidget( new QLabel( "Current paragraph highlighting: ", box ) );
  grid_layout->addWidget( new QLabel( (parent->blockHighlightAction().isChecked() ? "true":"false" ), box ) );

  grid_layout->addWidget( new QLabel( "Text highlighting: ", box ) );
  grid_layout->addWidget( new QLabel( (parent->textHighlight().isHighlightEnabled() ? "true":"false" ), box ) );
  
  grid_layout->addWidget( new QLabel( "Matching parenthesis highlighting: ", box ) );
  grid_layout->addWidget( new QLabel( (parent->textHighlight().isParenthesisEnabled() ? "true":"false" ), box ) );

  grid_layout->addWidget( new QLabel( "Text indent: ", box ) );
  grid_layout->addWidget( new QLabel( (parent->textIndent().isEnabled() ? "true":"false" ), box ) );

  grid_layout->addWidget( new QLabel( "Text wrapping: ", box ) );
  grid_layout->addWidget( new QLabel( (parent->wrapModeAction().isChecked() ? "true":"false" ), box ) );
  
  grid_layout->addWidget( new QLabel( "Tab emulation: ", box ) );
  grid_layout->addWidget( new QLabel( (parent->tabEmulationAction().isChecked() ? "true":"false" ), box ) );

  grid_layout->setColumnStretch( 1, 1 );
  
  // autosave
  if( !file.empty() )
  { 
    layout->addWidget( new QLabel( "Auto-save filename: ", box ) );
    layout->addWidget( new QLabel( AutoSaveThread::autoSaveName( file ).c_str(), box ) );
  }
  
  layout->addStretch();
  
  // close button 
  QPushButton *button = new QPushButton( IconEngine::get( ICONS::DIALOG_CLOSE ), "&Close", this );
  FileInfoDialog::layout()->addWidget( button );
  connect( button, SIGNAL( clicked() ), SLOT( close() ) );
  
  adjustSize();
  
}
