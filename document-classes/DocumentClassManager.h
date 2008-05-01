#ifndef DocumentClassManager_h
#define DocumentClassManager_h

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
  \file DocumentClassManager.h
  \brief Store list of Document Class
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QObject>
#include <QString>
#include <list>

#include "Counter.h"
#include "Debug.h"
#include "File.h"

class DocumentClass;

//! Store list of Document Class
class DocumentClassManager: public QObject, public Counter
{

  public:

  //! constructor
  DocumentClassManager( QObject* parent = 0 );

  //! destructor
  ~DocumentClassManager( void )
  {
    Debug::Throw( "DocumentclassManager::~DocumentClassManager.\n" );
    clear();
  }

  //! clear document classes
  void clear( void );

  //! read classes from file
  bool read( const File& file );

  //! read errors
  const QString& readError( void ) const
  { return read_error_; }
  
  //! write all classes to file
  bool write( const File& file ) const;

  //! write classes to file
  bool write( const QString& class_name, const File& file ) const;

  //! get class matching filename. Return 0 if not found
  DocumentClass find( const File& file ) const;

  //! get class matching name. Return 0 if none found
  DocumentClass get( const QString& name ) const;

  //! remove a class matching name.
  bool remove( const QString& name );

  //! shortcut to list of document classes
  typedef std::list<DocumentClass> ClassList;

  //! get all classes
  const ClassList& list( void ) const
  { return document_classes_; }

  private:

  //! list of document classes
  ClassList document_classes_;
  
  //! read error
  QString read_error_;
};

#endif
