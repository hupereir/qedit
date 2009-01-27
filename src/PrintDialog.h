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
#include <QString>
#include <QStringList>

#include "BrowsedLineEditor.h"
#include "CustomDialog.h"
#include "CustomComboBox.h"
#include "LineEditor.h"
#include "File.h"

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
  { return wrap_checkbox_->isChecked() ? maximum_line_size_->value():0; }
  
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
  { return _destinationEditor().editor().text(); }
  
  //! use command
  void setUseCommand( const bool& value )
  { command_checkbox_->setChecked( value ); }
  
  //! use command 
  bool useCommand( void ) const
  { return command_checkbox_->isChecked(); }
  
  //! command
  QString command( void ) const
  { return _commandEditor().currentText(); }
  
  //! set command manually
  void setCommand( QString command )
  { _commandEditor().setEditText( command ); }
  
  //! add commands to the combo-box list
  void addCommand( QString command )
  { _commandEditor().addItem( command ); }
  
  //! commands
  QStringList commands( void ) const
  { 
    Debug::Throw() << "PrintDialog::commands - maxCount: " << _commandEditor().QComboBox::count() << std::endl;
    QStringList out;
    for( int row = 0; row < _commandEditor().QComboBox::count(); row++ )
    { out.push_back( _commandEditor().itemText( row ) ); }
    
    return out;
  }

  private slots:
  
  //! update checkboxes
  void _updateCheckBoxes( void );
  
  //! update print command
  void _updateFile( void );
   
  //! browse print command
  void _browseCommand( void );
  
  protected:
  
  //! destination
  BrowsedLineEditor& _destinationEditor( void ) const
  { return *destination_editor_; }
  
  //! command editor
  CustomComboBox& _commandEditor( void ) const
  { return *command_editor_; }
  
  private:
   
  //! a2ps checkbox
  QRadioButton* html_checkbox_;
  
  //! a2ps checkbox
  QRadioButton* pdf_checkbox_;
    
  //! postscript file
  BrowsedLineEditor* destination_editor_;
  
  //! wrap lines
  QCheckBox* wrap_checkbox_;
  
  //! max line size
  QSpinBox* maximum_line_size_;
  
  //! command check box
  QCheckBox* command_checkbox_;
  
  //! print command
  CustomComboBox* command_editor_;
  
};
#endif
