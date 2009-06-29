#ifndef DocumentClassToolBar_h
#define DocumentClassToolBar_h

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
****************************************************************************/

/*!
   \file DocumentClassToolBar.h
   \brief editor main window
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/
#include <QComboBox>

#include "CustomToolBar.h"

class DocumentClassToolBar: public CustomToolBar
{
  
  Q_OBJECT
  
  public:
    
  //! constructor
  DocumentClassToolBar( QWidget* parent );
  
  //! update selection
  void update( QString );

  signals:

  //! emmited every time a document class is selected
  void documentClassSelected( QString );

  private slots:

  //! update list
  void _update( void );
  
  //! current index changed
  void _currentIndexChanged( int );
  
  private:
  
  //! document class selection combo box
  QComboBox& _comboBox( void ) const
  { return *combobox_; }
  
  //! document class selection combo box
  QComboBox* combobox_;
  
  //! current class
  QString current_class_;
  
};

#endif
