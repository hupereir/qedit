// $Id$
#ifndef RecentFilesFrame_h
#define RecentFilesFrame_h

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

#include "Counter.h"
#include "FileRecordModel.h"

#include <QtGui/QAction>
#include <QtGui/QPaintEvent>
#include <cassert>

class FileList;
class TreeView;

//! editor windows navigator
/*!
displays an up-to-date list of recent files
as well as files opened in current session
*/
class RecentFilesFrame: public QWidget, public Counter
{

    //! Qt meta object declaration
    Q_OBJECT

    public:

    //! creator
    RecentFilesFrame( QWidget* parent, FileList&  );

    //! destructor
    ~RecentFilesFrame( void );

    //! list
    TreeView& list( void ) const
    {
        assert( list_ );
        return *list_;
    }

    //! select file in list
    void select( const File& );

    public slots:

    //! update
    void update( void );

    signals:

    //! signal emitted when a file is selected
    void fileSelected( FileRecord );

    //! signal emited when a file is selected
    void fileActivated( FileRecord );

    protected:

    //! enter event
    void enterEvent( QEvent* );

    //! recent files
    FileList& _recentFiles( void ) const
    {
        assert( recentFiles_ );
        return *recentFiles_;
    }

    //! model
    FileRecordModel& _model( void )
    { return model_; }

    private slots:

    //! update action
    void _updateActions( void );

    //! clean
    void _clean( void );

    //! open
    void _open( void );

    //! sessionFilesItem selected
    void _itemSelected( const QModelIndex& index );

    //! sessionFilesItem selected
    void _itemActivated( const QModelIndex& index );

    private:

    //!@name actions
    //@{

    //! install actions
    void _installActions( void );

    //! clean action
    QAction& _cleanAction( void ) const
    { return *cleanAction_; }

    //! open action
    QAction& _openAction( void ) const
    { return *openAction_; }

    //@}

    //! recent files
    FileList* recentFiles_;

    //! model
    FileRecordModel model_;

    //! list
    TreeView* list_;

    //@}

    //!@name actions
    //@{

    //! update
    QAction* updateAction_;

    //! clean action
    QAction* cleanAction_;

    //! open action
    QAction* openAction_;

    //@}

};

#endif
