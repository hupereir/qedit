#ifndef FileCheckData_h
#define FileCheckData_h

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
#include "File.h"
#include "TimeStamp.h"

//* used to monitor file changes
class FileCheckData: private Base::Counter<FileCheckData>
{
    public:

    //* flag
    enum class Flag
    {
        None,
        Removed,
        Modified
    };

    //* constructor
    explicit FileCheckData( File file = File(), Flag flag = Flag::None, TimeStamp stamp = TimeStamp() ):
        Counter( "FileCheckData" ),
        file_( file ),
        flag_( flag ),
        timeStamp_( stamp )
    {}

    //* file
    void setFile( const File& file )
    { file_ = file; }

    //* file
    const File& file() const
    { return file_; }

    //* flag
    void setFlag( const Flag& flag )
    { flag_ = flag; }

    //* flag
    const Flag& flag() const
    { return flag_; }

    //* timestamp
    void setTimeStamp( const TimeStamp& stamp )
    { timeStamp_ = stamp; }

    //* timestamp
    const TimeStamp& timeStamp() const
    { return timeStamp_; }

    private:

    //* file
    File file_;

    //* flag
    Flag flag_ = Flag::None;

    //* timestamp
    TimeStamp timeStamp_;

};

#endif
