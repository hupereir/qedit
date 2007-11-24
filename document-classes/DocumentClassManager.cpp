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
  \file DocumentClassManager.cpp
  \brief Store list of Document Class
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <qdom.h>
#include <qfile.h>

#include "DocumentClass.h"
#include "DocumentClassManager.h"
#include "IndentPattern.h" 
#include "QtUtil.h"
#include "XmlDef.h" 
#include "XmlError.h" 

using namespace std;

//________________________________________________________
void DocumentClassManager::clear( void )
{
  Debug::Throw( "DocumentClassManager::Clear.\n" );
  for( ClassList::iterator iter = document_classes_.begin(); iter != document_classes_.end(); iter++ )
  delete *iter;
  document_classes_.clear();
  read_error_ = "";
}

//________________________________________________________
bool DocumentClassManager::read( const File& filename )
{
  Debug::Throw() << "DocumentClassManager::read - file: " << filename << endl; 
  
  // reset Read error
  read_error_ = "";
  
  // try open file
  QFile file( filename.c_str() );
  if ( !file.open( QIODevice::ReadOnly ) ) return false;
  
  // parse file
  XmlError error( filename );
  QDomDocument document;
  if ( !document.setContent( &file, &error.error(), &error.line(), &error.column() ) ) {
    file.close();
    ostringstream what;
    what << "An error occured while parsing document classes." << endl;
    what << error;
    what << endl;
    read_error_ = what.str();
    return false;
  }
  
  // parse DomElement
  ostringstream what;
  QDomElement doc_element = document.documentElement();
  for(QDomNode node = doc_element.firstChild(); !node.isNull(); node = node.nextSibling() )
  {
    QDomElement element = node.toElement();
    if( element.isNull() ) continue;

    string tag_name( qPrintable( element.tagName() ) );
    if( tag_name == XML::DOCUMENT_CLASS ) 
    {
      DocumentClass* document_class = new DocumentClass( element );
      
      // look for document classes with same name 
      ClassList::iterator iter = find_if(
        document_classes_.begin(),
        document_classes_.end(),
        DocumentClass::SameNameFTor( document_class->name() ) );
      if( iter != document_classes_.end() )
      {
        delete *iter;
        document_classes_.erase( iter );
      }
      
      // add new document class
      document_class->setFile( filename );
      document_classes_.push_back( document_class );
      
      // reset IndentPattern counter (for debugging)
      IndentPattern::resetCounter();

    } else Debug::Throw() << "DocumentClassManager::read - unrecognized child " << tag_name << endl;
  }

  // store error
  read_error_ = what.str();
    
  // sort classes (based on Name())
  document_classes_.sort( DocumentClass::LowerThanFTor() );

  return true;
  
}

//________________________________________________________
bool DocumentClassManager::write( const std::string& class_name, const File& filename ) const
{
  Debug::Throw() << "DocumentClassManager::write - class: " << class_name << " file: " << filename << endl;
  
  // try retrieve DocumentClass
  ClassList::const_iterator iter = find_if( document_classes_.begin(), document_classes_.end(), DocumentClass::SameNameFTor( class_name ) );
  if( iter == document_classes_.end() ) return false;
  
  // try open file
  QFile out( filename.c_str() );
  if( !out.open( QIODevice::WriteOnly ) ) return false;
  
  // create document
  QDomDocument document;
  
  // create main element
  QDomElement top = document.appendChild( document.createElement( XML::PATTERNS.c_str() ) ).toElement();
  top.appendChild( (*iter)->domElement( document ) );
  
  out.write( document.toByteArray() );
  out.close();

  return true;

}

//________________________________________________________
bool DocumentClassManager::write( const File& filename ) const
{
  Debug::Throw() << "DocumentClassManager::write - file: " << filename << endl;
  
  
  // try open file
  QFile out( filename.c_str() );
  if( !out.open( QIODevice::WriteOnly ) ) return false;
  
  // create document
  QDomDocument document;
  
  // create main element
  QDomElement top = document.appendChild( document.createElement( XML::PATTERNS.c_str() ) ).toElement();
  for( ClassList::const_iterator iter = document_classes_.begin(); iter != document_classes_.end(); iter++ )
  { top.appendChild( (*iter)->domElement( document ) ); }
 
  out.write( document.toByteArray() );
  out.close();

  return true;

}

//________________________________________________________
const DocumentClass* DocumentClassManager::find( const File& filename ) const
{
  Debug::Throw() << "DocumentClassManager::find - file: " << filename << endl;  
  
  // try load class matching name
  ClassList::const_iterator iter = find_if(
    document_classes_.begin(),
    document_classes_.end(),
    DocumentClass::MatchFileFTor( filename ) );
  if( iter != document_classes_.end() ) return *iter;
  
  // try load default
  iter = find_if(
    document_classes_.begin(),
    document_classes_.end(),
    DocumentClass::IsDefaultFTor() );
  if( iter != document_classes_.end() ) return *iter;
  
  // nothing found
  return 0;
}

//________________________________________________________
const DocumentClass* DocumentClassManager::get( const string& name ) const
{  
  Debug::Throw() << "DocumentClassManager::Get - name: " << name << endl;
  
  // try load class matching name
  ClassList::const_iterator iter = find_if(
    document_classes_.begin(),
    document_classes_.end(),
    DocumentClass::SameNameFTor( name ) );
  if( iter != document_classes_.end() ) return *iter;
  
  // no match found
  return 0;
}
  

//________________________________________________________
bool DocumentClassManager::remove( const string& name )
{
  Debug::Throw() << "DocumentClassManager::Remove - name: " << name << endl; 
  
  // find class list matching name
  ClassList::iterator iter = find_if(
    document_classes_.begin(),
    document_classes_.end(),
    DocumentClass::SameNameFTor( name ) );
  if( iter == document_classes_.end() ) return false;
  
  // delete class and remove from list
  delete *iter;
  document_classes_.erase( iter );
  return true;
}
