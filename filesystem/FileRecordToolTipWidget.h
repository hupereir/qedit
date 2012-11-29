#ifndef FileRecordToolTipWidget_h
#define FileRecordToolTipWidget_h

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
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*******************************************************************************/

#include "FileRecord.h"
#include "Counter.h"

#include <QtCore/QBasicTimer>
#include <QtCore/QTimerEvent>

#include <QtGui/QIcon>
#include <QtGui/QFrame>
#include <QtGui/QLabel>
#include <QtGui/QPaintEvent>
#include <QtGui/QWidget>

class ToolTipWidgetItem;

class FileRecordToolTipWidget: public QWidget, public Counter
{

    Q_OBJECT

    public:

    //! constructor
    FileRecordToolTipWidget( QWidget* );

    //! destructo
    virtual ~FileRecordToolTipWidget( void )
    {}

    //! enable state
    void setEnabled( bool );

    //! set data
    void setRecord( const FileRecord&, const QIcon& = QIcon() );

    //! index rect
    void setIndexRect( const QRect& rect )
    { rect_ = rect; }

    //! mask
    void setPixmapSize( int value )
    {
        if( pixmapSize_ == value ) return;
        pixmapSize_ = value;
        _reload();
    }

    //! information mask
    enum Type
    {
        None = 0,
        Size = 1<<0,
        Modified = 1<<1,
        User = 1<<2,
        Group = 1<<3,
        Permissions = 1<<4,
        Default = Size|Modified
    };

    Q_DECLARE_FLAGS(Types, Type)

    //! mask
    void setMask( unsigned int value )
    {
        if( mask_ == value ) return;
        mask_ = value;
        _reload();
    }

    // event filter
    virtual bool eventFilter( QObject*, QEvent* );

    public slots:

    //! hide
    virtual void hide( void );

    //! show
    virtual void show( void );

    //! show delayed
    void showDelayed( int = 500 );

    protected:

    //! paint
    virtual void paintEvent( QPaintEvent* );

    //! mouse press
    virtual void mousePressEvent( QMouseEvent* );

    //! timer event
    virtual void timerEvent( QTimerEvent* );

    //! check mouse position
    bool _checkMousePosition( void ) const;

    //! adjust position
    void _adjustPosition( void );

    //! reload
    virtual void _reload( void )
    { setRecord( record_, icon_ ); }

    private slots:

    //! update configuration
    void _updateConfiguration( void );

    private:

    //! enable state
    bool enabled_;

    //! pixmap size
    int pixmapSize_;

    //! information mask
    unsigned int mask_;

    //! index rect
    QRect rect_;

    //! local icon copy
    QIcon icon_;

    //! local fileInfo copy
    FileRecord record_;

    //! icon label
    QLabel* iconLabel_;

    //! file name label
    QLabel* fileLabel_;

    //! separator
    QFrame* separator_;

    //!@name items
    //@{
    ToolTipWidgetItem* pathItem_;
    ToolTipWidgetItem* sizeItem_;
    ToolTipWidgetItem* lastModifiedItem_;
    ToolTipWidgetItem* userItem_;
    ToolTipWidgetItem* groupItem_;
    ToolTipWidgetItem* permissionsItem_;
    //@}

    //! timer
    QBasicTimer timer_;

};

Q_DECLARE_OPERATORS_FOR_FLAGS( FileRecordToolTipWidget::Types )

#endif
