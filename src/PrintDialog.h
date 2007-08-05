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

#include <QCheckBox>
#include <QRadioButton> 

#include "BrowsedLineEdit.h"
#include "CustomDialog.h"
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
  
  //! command
  std::string command( void ) const
  { return qPrintable( command_->text() ); }
  
  //! use a2ps
  bool useA2Ps( void ) const
  { return a2ps_checkbox_->isChecked(); }
  
  //! a2ps command
  std::string a2psCommand( void ) const
  { return qPrintable( a2ps_command_->text() ); }

  //! print command
  std::string printCommand( void ) const
  { return qPrintable( print_command_->text() ); }
  
  private slots:
  
  // update checkboxes
  void _updateCheckBoxes( bool state );
  
  // update print command
  void _updatePrintCommand( void );
   
  private:
   
  //! file to print
  File file_;
  
  //! a2ps checkbox
  QCheckBox* a2ps_checkbox_;

  //! file checkbox
  QRadioButton* file_checkbox_;
    
  //! printer checkbox
  QRadioButton* printer_checkbox_;
  
  //! a2ps command
  CustomLineEdit* a2ps_command_;
  
  //! postscript file
  BrowsedLineEdit* ps_file_;
  
  //! print command
  CustomLineEdit* print_command_;
  
  //! full command
  CustomLineEdit* command_;
      
};
#endif
