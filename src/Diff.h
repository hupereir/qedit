#ifndef Diff_h
#define Diff_h

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

#include "Counter.h"
#include "CustomProcess.h"
#include "Debug.h"
#include "File.h"


#include <QPair>
#include <QSet>

#include <array>

class TextDisplay;

//* make diff between two files, stores conflict and added lines for both
class Diff: public QObject, private Base::Counter<Diff>
{

    //* Qt meta object declaration
    Q_OBJECT

    public:

    //* constructor
    explicit Diff( QObject* );

    //* store files from text Displays
    void setTextDisplays( TextDisplay& first, TextDisplay& second )
    {
        files_[0].setDisplay( first );
        files_[1].setDisplay( second );
    }

    //* run process
    /** returns true if command could run succesfully */
    bool run();

    //* error message
    const QString& error() const
    { return error_; }

    private:

    //* paragraph range
    using Range = QPair<int,int>;

    //* parse the diff output
    void _parseOutput( int, QProcess::ExitStatus );

    //* clear ranges
    void _clear();

    //* parse a diff line
    void _parseLine( const QString& line );

    //* parse a diff line
    static Range _parseRange( const QString& range );

    //* file specific diff informations
    class FileInformation
    {

        public:

        //* default constructor
        explicit FileInformation();

        //* destructor
        ~FileInformation();

        //* set display
        void setDisplay( TextDisplay& display );

        //* file
        const File& file() const
        { return file_; }

        //* add added range
        void insertAddedRange( Diff::Range range )
        {
            for( int index = range.first; index <= range.second; ++index )
            { added_.insert( index ); }
        }

        //* add conflict range
        void insertConflictRange( Diff::Range range )
        {
            for( int index = range.first; index <= range.second; ++index )
            { conflicts_.insert( index ); }
        }


        //* clear
        void clear()
        {
            added_.clear();
            conflicts_.clear();
        }

        //* highlight text display based on conflicts and added paragraphs
        void highlightDisplay();

        //* list
        using List = QVector<FileInformation>;

        private:

        //* text display
        TextDisplay* display_ = nullptr;

        //* file
        File file_;

        /**
        \brief true when file is a temporary and should be deleted
        at destruction
        */
        bool isTemporary_ = false;

        //* range list
        using BlockSet = QSet<int>;

        //* added paragraphs
        BlockSet added_;

        //* conflict paragraphs
        BlockSet conflicts_;

    };

    //* file specific diff information
    std::array<FileInformation,2> files_;

    //* process
    CustomProcess process_;

    //* possible error string
    QString error_;

};

#endif
