// $Id$
#ifndef CustomTextDocument_h
#define CustomTextDocument_h

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
   \file CustomTextDocument.h
   \brief customized TextDocument to derive from BASE::Key
   \author Hugo Pereira
   \version $Revision$
   \date $Date$
*/

#include <QTextDocument>

#include "Counter.h"
#include "Key.h"

//! customized TextDocument to derive from BASE::Key
class CustomTextDocument: public QTextDocument, public BASE::Key, public Counter
{
  
  public:
  
  //! constructor
  CustomTextDocument( QObject* parent = 0 ):
    QTextDocument( parent ),
    Counter( "CustomTextDocument" )
  { Debug::Throw( "CustomTextDocument::CustomTextDocument.\n" ); }
  
  //! destructor
  virtual ~CustomTextDocument( void )
  { Debug::Throw( "CustomTextDocument::~CustomTextDocument.\n" ); }
  
};

#endif
