#ifndef ViewHtmlDialog_h
#define ViewHtmlDialog_h
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
   \file ViewHtmlDialog.h
   \brief view HTML file
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <string>
#include <QCheckBox>

#include "BrowsedLineEdit.h"
#include "CustomDialog.h"
#include "CustomLineEdit.h"
#include "File.h"
#include "QtUtil.h"

//! view html file
class ViewHtmlDialog: public CustomDialog
{
  
  //! Qt meta object declaration
  Q_OBJECT;

  public:
      
  //! constructor
  ViewHtmlDialog( QWidget* parent );
  
  //! filename
  void setFile( const File& file )
  { file_->editor().setText( file.c_str() ); }
  
  //! filename
  File file( void ) const
  { return File( qPrintable( file_->editor().text() ) ); }
  
  //! Html command
  void setCommand( const std::string& command )
  { command_->editor().setText( command.c_str() ); }
  
  //! Html command
  std::string command( void ) const
  { return qPrintable( command_->editor().text() ); }
  
  //! set if command is to be used
  void setUseCommand( const bool& value )
  { 
    use_command_->setChecked( value ); 
    _update( value );
  }
  
  //! true if command is to be used
  bool useCommand( void ) const
  { return use_command_->isChecked(); }
  
  private slots:
  
  //! update command visibility
  void _update( bool state )
  { command_->setEnabled( state ); }
  
  private:
  
  //! file editor
  BrowsedLineEdit* file_;
  
  //! command editor
  BrowsedLineEdit* command_;
  
  //! use command
  QCheckBox* use_command_;
  
  
  
};

#endif
