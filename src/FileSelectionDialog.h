// $Id$
#ifndef _FileSelectionDialog_h_
#define _FileSelectionDialog_h_
 
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
  \file FileSelectionDialog.h
  \brief QDialog used to select opened files
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QDialog>
#include <list>

#include "Counter.h"
#include "TreeWidget.h"
#include "File.h"
#include "TextSelection.h"

//! QDialog used to select opened files
class FileSelectionDialog: public QDialog, public Counter
{
  
  //! Qt macro
  Q_OBJECT
      
  public:
          
  //! constructor
  FileSelectionDialog( QWidget* parent, const TextSelection& );
  
  signals:
  
  //! emitted when a file is selected
  void fileSelected( std::list<File> files, TextSelection );
  
  private slots:
  
  //! perform replacement
  /*! emits fileSelected signal for all selected files in the list */
  void _replace( void );
  
  //! exit application
  void _cancel( void )
  { done( QDialog::Rejected ); }
  
  private:

  enum { FILE, CLASS };
  
  //! list of files
  TreeWidget* list_;
  
  //! text selection
  TextSelection selection_;
  
};

#endif
