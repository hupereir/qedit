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

#include "DocumentClass.h"
#include "DocumentClassManager.h"
#include "IndentPattern.h"
#include "QtUtil.h"
#include "XmlDef.h"
#include "XmlError.h"

#include <QtCore/QFile>
#include <algorithm>

//________________________________________________________
DocumentClassManager::DocumentClassManager( void ):
Counter( "DocumentClassManager" )
{ Debug::Throw( "DocumentclassManager::DocumentClassManager.\n" ); }

//________________________________________________________
void DocumentClassManager::clear( void )
{
    Debug::Throw( "DocumentClassManager::Clear.\n" );
    documentClasses_.clear();
    readError_ = "";
}

//________________________________________________________
bool DocumentClassManager::read( const File& filename )
{
    Debug::Throw() << "DocumentClassManager::read - file: " << filename << endl;

    // reset Read error
    readError_ = "";

    // try open file
    QFile file( filename );
    if ( !file.open( QIODevice::ReadOnly ) ) return false;

    // parse file
    XmlError error( filename );
    QDomDocument document;
    if ( !document.setContent( &file, &error.error(), &error.line(), &error.column() ) ) {
        file.close();
        readError_.clear();
        QTextStream( &readError_ )
            << "An error occured while parsing document classes." << endl
            << error
            << endl;
        return false;
    }

    QDomElement doc_element = document.documentElement();
    for(QDomNode node = doc_element.firstChild(); !node.isNull(); node = node.nextSibling() )
    {
        QDomElement element = node.toElement();
        if( element.isNull() ) continue;
        if( element.tagName() == XML::DOCUMENT_CLASS )
        {
            DocumentClass document_class( element );

            // look for document classes with same name
            List::iterator iter = std::find_if(
                documentClasses_.begin(),
                documentClasses_.end(),
                DocumentClass::SameNameFTor( document_class.name() ) );
            if( iter != documentClasses_.end() ) documentClasses_.erase( iter );

            // add new document class
            document_class.setFile( filename );
            documentClasses_.push_back( document_class );

            // reset IndentPattern counter (for debugging)
            IndentPattern::resetCounter();

        } else Debug::Throw() << "DocumentClassManager::read - unrecognized child " << element.tagName() << endl;
    }

    // store error
    readError_.clear();

    // sort classes (based on Name())
    std::sort( documentClasses_.begin(), documentClasses_.end(), DocumentClass::WeakLessThanFTor() );

    return true;

}

//________________________________________________________
bool DocumentClassManager::write( const QString& class_name, const File& filename ) const
{
    Debug::Throw() << "DocumentClassManager::write - class: " << class_name << " file: " << filename << endl;

    // try retrieve DocumentClass
    List::const_iterator iter = std::find_if( documentClasses_.begin(), documentClasses_.end(), DocumentClass::SameNameFTor( class_name ) );
    return ( iter == documentClasses_.end() ) ? false : write( *iter,  filename );

}

//________________________________________________________
bool DocumentClassManager::write( const DocumentClass& document_class, const File& filename ) const
{

    // try open file
    QFile out( filename );
    if( !out.open( QIODevice::WriteOnly ) ) return false;

    // create document
    QDomDocument document;

    // create main element
    QDomElement top = document.appendChild( document.createElement( XML::PATTERNS ) ).toElement();
    top.appendChild( document_class.domElement( document ) );

    out.write( document.toByteArray() );
    out.close();

    return true;

}

//________________________________________________________
bool DocumentClassManager::write( const File& path ) const
{
    Debug::Throw() << "DocumentClassManager::write - path: " << path << endl;

    if( !path.exists() )
    {
        Debug::Throw(0) << "DocumentClassManager::write - path " << path << " does not exist" << endl;
        return false;
    }

    if( !path.isDirectory() )
    {
        Debug::Throw(0) << "DocumentClassManager::write - path " << path << " is not a directory" << endl;
        return false;
    }


    for( List::const_iterator iter = documentClasses_.begin(); iter != documentClasses_.end(); ++iter )
    {
        File filename( iter->file().localName().addPath( path ) );
        Debug::Throw(0) << "DocumentClassManager::write - writing class " << iter->name() << " to file " << filename << endl;

        // try open file
        QFile out( filename );
        if( !out.open( QIODevice::WriteOnly ) ) continue;

        // create document
        QDomDocument document;

        // create main element
        QDomElement top = document.appendChild( document.createElement( XML::PATTERNS ) ).toElement();
        top.appendChild( iter->domElement( document ) );

        out.write( document.toByteArray() );
        out.close();

    }

    return true;

}

//________________________________________________________
DocumentClass DocumentClassManager::defaultClass( void ) const
{

    Debug::Throw( "DocumentClassManager::defaultClass.\n" );

    // try load default
    List::const_iterator iter = std::find_if(
        documentClasses_.begin(),
        documentClasses_.end(),
        DocumentClass::IsDefaultFTor() );
    if( iter != documentClasses_.end() )
    { return *iter; }

    // nothing found
    return DocumentClass();

}

//________________________________________________________
DocumentClass DocumentClassManager::find( const File& filename ) const
{
    Debug::Throw() << "DocumentClassManager::find - file: " << filename << endl;

    // try load class matching name
    List::const_iterator iter = std::find_if(
        documentClasses_.begin(),
        documentClasses_.end(),
        DocumentClass::MatchFileFTor( filename ) );
    if( iter != documentClasses_.end() ) return *iter;

    return defaultClass();

}

//________________________________________________________
DocumentClass DocumentClassManager::get( const QString& name ) const
{
    Debug::Throw() << "DocumentClassManager::Get - name: " << name << endl;

    // try load class matching name
    List::const_iterator iter = std::find_if(
        documentClasses_.begin(),
        documentClasses_.end(),
        DocumentClass::SameNameFTor( name ) );
    if( iter != documentClasses_.end() ) return *iter;

    // no match found
    return DocumentClass();
}


//________________________________________________________
bool DocumentClassManager::remove( const QString& name )
{
    Debug::Throw() << "DocumentClassManager::Remove - name: " << name << endl;

    // find class list matching name
    List::iterator iter = std::find_if(
        documentClasses_.begin(),
        documentClasses_.end(),
        DocumentClass::SameNameFTor( name ) );
    if( iter == documentClasses_.end() ) return false;

    documentClasses_.erase( iter );
    return true;

}
