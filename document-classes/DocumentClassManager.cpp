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
* Any WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* this program.  If not, see <http://www.gnu.org/licenses/>.
*
*******************************************************************************/

#include "DocumentClass.h"
#include "DocumentClassManager.h"
#include "IndentPattern.h"
#include "QtUtil.h"
#include "XmlDef.h"
#include "XmlDocument.h"

#include <QFile>
#include <algorithm>

//________________________________________________________
DocumentClassManager::DocumentClassManager():
    Counter( QStringLiteral("DocumentClassManager") )
{ Debug::Throw( QStringLiteral("DocumentclassManager::DocumentClassManager.\n") ); }

//________________________________________________________
void DocumentClassManager::clear()
{
    Debug::Throw( QStringLiteral("DocumentClassManager::Clear.\n") );
    documentClasses_.clear();
    readError_.clear();
}

//________________________________________________________
bool DocumentClassManager::read( const File& filename )
{
    Debug::Throw() << "DocumentClassManager::read - file: " << filename << endl;

    // reset Read error
    readError_.clear();

    // try open file
    QFile file( filename );
    if ( !file.open( QIODevice::ReadOnly ) ) return false;

    // parse file
    XmlDocument document;
    if( !document.setContent( &file ) )
    {
        readError_ = QString( QObject::tr( "An error occured while parsing document classes.\n %1" ) ).arg( document.error() );
        return false;
    }

    const auto top = document.get().documentElement();
    for( auto&& node = top.firstChild(); !node.isNull(); node = node.nextSibling() )
    {
        const auto element = node.toElement();
        if( element.isNull() ) continue;
        if( element.tagName() == Xml::DocumentClass )
        {
            DocumentClass documentClass( element );

            // look for document classes with same name
            const auto iter = std::find_if(
                documentClasses_.begin(),
                documentClasses_.end(),
                DocumentClass::SameNameFTor( documentClass.name() ) );
            if( iter != documentClasses_.end() ) documentClasses_.erase( iter );

            // add new document class
            documentClass.setFile( filename );
            documentClass.setIsBuildIn( filename.startsWith( ':' ) );
            documentClasses_.append( documentClass );

            // reset IndentPattern counter (for debugging)
            IndentPattern::resetCounter();

        }
    }

    // store error
    readError_.clear();

    // sort classes (based on Name())
    std::sort( documentClasses_.begin(), documentClasses_.end(), DocumentClass::WeakLessThanFTor() );

    return true;

}

//________________________________________________________
bool DocumentClassManager::write( const QString& className, const File& filename ) const
{
    Debug::Throw() << "DocumentClassManager::write - class: " << className << " file: " << filename << endl;

    // try retrieve DocumentClass
    const auto iter = std::find_if( documentClasses_.begin(), documentClasses_.end(), DocumentClass::SameNameFTor( className ) );
    return ( iter == documentClasses_.end() ) ? false : write( *iter,  filename );

}

//________________________________________________________
bool DocumentClassManager::write( const DocumentClass& documentClass, const File& filename ) const
{

    // try open file
    QFile out( filename );
    if( !out.open( QIODevice::WriteOnly ) ) return false;

    // create document
    QDomDocument document;

    // create main element
    auto top = document.appendChild( document.createElement( Xml::Patterns ) ).toElement();
    top.appendChild( documentClass.domElement( document ) );

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

    for( const auto& documentClass:documentClasses_ )
    {
        File filename( documentClass.file().localName().addPath( path ) );
        Debug::Throw(0) << "DocumentClassManager::write - writing class " << documentClass.name() << " to file " << filename << endl;

        // try open file
        QFile out( filename );
        if( !out.open( QIODevice::WriteOnly ) ) continue;

        // create document
        QDomDocument document;

        // create main element
        auto top = document.appendChild( document.createElement( Xml::Patterns ) ).toElement();
        top.appendChild( documentClass.domElement( document ) );

        out.write( document.toByteArray() );
        out.close();

    }

    return true;

}

//________________________________________________________
DocumentClass DocumentClassManager::defaultClass() const
{

    Debug::Throw( QStringLiteral("DocumentClassManager::defaultClass.\n") );

    const auto iter = std::find_if( documentClasses_.begin(), documentClasses_.end(), DocumentClass::IsDefaultFTor() );
    return (iter == documentClasses_.end()) ? DocumentClass():*iter;

}

//________________________________________________________
DocumentClass DocumentClassManager::find( const File& filename ) const
{
    Debug::Throw() << "DocumentClassManager::find - file: " << filename << endl;

    // try load class matching name
    const auto iter = std::find_if(
        documentClasses_.begin(),
        documentClasses_.end(),
        DocumentClass::MatchFileFTor( filename ) );
    return iter == documentClasses_.end() ? defaultClass():*iter;

}

//________________________________________________________
DocumentClass DocumentClassManager::get( const QString& name ) const
{
    Debug::Throw() << "DocumentClassManager::Get - name: " << name << endl;

    // try load class matching name
    const auto iter = std::find_if(
        documentClasses_.begin(),
        documentClasses_.end(),
        DocumentClass::SameNameFTor( name ) );
    return  iter == documentClasses_.end() ? DocumentClass():*iter;

}

//________________________________________________________
bool DocumentClassManager::remove( const QString& name )
{
    Debug::Throw() << "DocumentClassManager::Remove - name: " << name << endl;

    // find class list matching name
    const auto iter = std::find_if(
        documentClasses_.begin(),
        documentClasses_.end(),
        DocumentClass::SameNameFTor( name ) );
    if( iter == documentClasses_.end() ) return false;

    documentClasses_.erase( iter );
    return true;
}
