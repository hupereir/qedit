#ifndef PrintDialog_h
#define PrintDialog_h

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
  \file PrintDialog.h
  \brief print document
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QRadioButton>
#include <QCheckBox>
#include <QSpinBox>

#include "BrowsedLineEdit.h"
#include "CustomDialog.h"
#include "CustomComboBox.h"
#include "CustomLineEdit.h"
#include "File.h"
#include "QtUtil.h"

//! print document
class PrintDialog: public CustomDialog
{

  //! Qt meta object declaration
  Q_OBJECT;
  
  public:
      
  //! constructor
  PrintDialog( QWidget* parent );
  
  //! file
  void setFile( const File& file ); 
  
  //! max line size
  void setMaximumLineSize( const int& value )
  { 
    if( value <= 0 ) wrap_checkbox_->setChecked( false );
    else {
      wrap_checkbox_->setChecked( true );
      maximum_line_size_->setValue( value ); 
    }
    
  }
  
  //! maximum line size
  int maximumLineSize( void ) const
  { return wrap_checkbox_->isChecked() ? 0:maximum_line_size_->value(); }
  
  //! print mode
  enum Mode
  {
    //! to PDF
    PDF,
    
    //! to HTML
    HTML
    
  };
  
  //! mode
  void setMode( const Mode& mode )
  { 
    if( mode == PDF ) pdf_checkbox_->setChecked( true ); 
    else html_checkbox_->setChecked( true );
  }
  
  //! mode
  Mode mode( void ) const
  { return pdf_checkbox_->isChecked() ? PDF:HTML; }
  
  //! file
  QString destinationFile( void ) const
  { return destination_->editor().text(); }
  
  //! use command
  void setUseCommand( const bool& value )
  { command_checkbox_->setChecked( value ); }
  
  //! use command 
  bool useCommand( void ) const
  { return command_checkbox_->isChecked(); }
  
  //! command
  QString command( void ) const
  { return command_->currentText(); }
  
  //! add commands
  void addCommand( const std::string& command )
  { command_->addItem( command.c_str() ); }
  
  //! commands
  std::list< std::string > commands( void ) const
  { 
    Debug::Throw() << "PrintDialog::commands - maxCount: " << command_->maxCount() << std::endl;
    std::list< std::string > out;
    for( int row = 0; row < command_->maxCount(); row++ )
    { out.push_back( qPrintable( command_->itemText( row ) ) ); }
    
    return out;
  }

  private slots:
  
  // update checkboxes
  void _updateCheckBoxes( void );
  
  // update print command
  void _updateFile( void );
   
  private:
   
  //! a2ps checkbox
  QRadioButton* html_checkbox_;
  
  //! a2ps checkbox
  QRadioButton* pdf_checkbox_;
    
  //! postscript file
  BrowsedLineEdit* destination_;
  
  //! wrap lines
  QCheckBox* wrap_checkbox_;
  
  //! max line size
  QSpinBox* maximum_line_size_;
  
  //! command check box
  QCheckBox* command_checkbox_;
  
  //! print command
  CustomComboBox* command_;
  
};
#endif
