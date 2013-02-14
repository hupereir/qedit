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

#include "FileRecordToolTipWidget.h"
#include "Debug.h"
#include "GridLayout.h"
#include "Singleton.h"
#include "TimeStamp.h"
#include "ToolTipWidgetItem.h"
#include "XmlOptions.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QFrame>
#include <QLayout>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionFrame>
#include <QToolTip>

//_______________________________________________________
FileRecordToolTipWidget::FileRecordToolTipWidget( QWidget* parent ):
    QWidget( parent, Qt::ToolTip | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint ),
    Counter( "FileRecordToolTipWidget" ),
    enabled_( true ),
    pixmapSize_( 96 ),
    mask_( Default )
{

    Debug::Throw( "FileRecordToolTipWidget::FileRecordToolTipWidget.\n" );
    setAttribute( Qt::WA_TranslucentBackground, true );

    // event filter on parent
    if( parent ) parent->installEventFilter( this );

    // change palete
    setPalette( QToolTip::palette() );
    setBackgroundRole( QPalette::ToolTipBase );
    setForegroundRole( QPalette::ToolTipText );

    // layout
    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->setMargin( 10 );
    hLayout->setSpacing( 10 );
    setLayout( hLayout );

    hLayout->addWidget( iconLabel_ = new QLabel( this ) );
    iconLabel_->setAlignment( Qt::AlignHCenter|Qt::AlignTop );

    QVBoxLayout* vLayout = new QVBoxLayout();
    vLayout->setMargin( 0 );
    vLayout->setSpacing( 5 );
    hLayout->addLayout( vLayout );

    // file
    vLayout->addWidget( fileLabel_ = new QLabel( this ) );
    fileLabel_->setAlignment( Qt::AlignCenter );
    QFont font( fileLabel_->font() );
    font.setBold( true );
    fileLabel_->setFont( font );
    fileLabel_->setMargin( 1 );

    // separator
    vLayout->addWidget( separator_ = new QFrame( this ) );
    separator_->setFrameStyle( QFrame::HLine );

    // grid layout
    GridLayout* gridLayout = new GridLayout();
    gridLayout->setMaxCount( 2 );
    gridLayout->setColumnAlignment( 0, Qt::AlignVCenter|Qt::AlignRight );
    gridLayout->setColumnAlignment( 1, Qt::AlignVCenter|Qt::AlignLeft );
    gridLayout->setMargin( 0 );
    gridLayout->setSpacing( 5 );
    vLayout->addLayout( gridLayout );

    // items
    ( pathItem_ = new ToolTipWidgetItem( this, gridLayout ) )->setKey( "Path:" );
    ( sizeItem_ = new ToolTipWidgetItem( this, gridLayout ) )->setKey( "Size:" );
    ( lastModifiedItem_ = new ToolTipWidgetItem( this, gridLayout ) )->setKey( "Modified:" );
    ( userItem_ = new ToolTipWidgetItem( this, gridLayout ) )->setKey( "Owner:" );
    ( groupItem_ = new ToolTipWidgetItem( this, gridLayout ) )->setKey( "Group:" );
    ( permissionsItem_ = new ToolTipWidgetItem( this, gridLayout ) )->setKey( "Permissions:" );

    // add stretch
    vLayout->addStretch( 1 );

    // configuration
    connect( Singleton::get().application(), SIGNAL( configurationChanged( void ) ), SLOT( _updateConfiguration( void ) ) );
    _updateConfiguration();

}

//_____________________________________________
void FileRecordToolTipWidget::setEnabled( bool value )
{
    Debug::Throw( "FileRecordToolTipWidget::setEnabled.\n" );
    if( enabled_ == value ) return;
    enabled_ = value;
    if( !enabled_ )
    {
        if( isVisible() ) hide();
        timer_.stop();
    }

}

//_______________________________________________________
void FileRecordToolTipWidget::setRecord( const FileRecord& record, const QIcon& icon )
{
    Debug::Throw( "FileRecordToolTipWidget::setRecord.\n" );

    // local storage
    icon_ = icon;
    record_ = record;

    // update icon
    if( !icon.isNull() )
    {

        iconLabel_->setPixmap( icon.pixmap( QSize( pixmapSize_, pixmapSize_ ) ) );
        iconLabel_->show();

    } else iconLabel_->hide();

    if( !record.file().isEmpty() )
    {
        // file and separator
        fileLabel_->show();
        fileLabel_->setText( record.file().localName() );
        separator_->show();

        // type
        pathItem_->setText( record.file().path() );

        // size
        if( (mask_&Size) && record.file().fileSize() > 0 && !( record.file().isDirectory() || record.file().isLink() ) )
        {

            sizeItem_->setText( record.file().sizeString() );

        } else sizeItem_->hide();

        // last modified
        TimeStamp lastModified;
        if( (mask_&Modified) && ( lastModified = record.file().lastModified() ).isValid() )
        {

            lastModifiedItem_->setText( lastModified.toString() );

        } else lastModifiedItem_->hide();

        // user
        QString user;
        if( (mask_&User) && !( user = record.file().userName() ).isEmpty() ) userItem_->setText( user );
        else userItem_->hide();

        // group
        QString group;
        if( (mask_&Group) && !( group = record.file().groupName() ).isEmpty() ) groupItem_->setText( group );
        else groupItem_->hide();

        // permissions
        QString permissions;
        if( (mask_&Permissions) && !( permissions = record.file().permissionsString() ).isEmpty() ) permissionsItem_->setText( permissions );
        else permissionsItem_->hide();

    } else {

        // file and separator
        fileLabel_->hide();
        separator_->hide();

        // items
        pathItem_->hide();
        sizeItem_->hide();
        lastModifiedItem_->hide();
        permissionsItem_->hide();
    }

}

//_______________________________________________________
bool FileRecordToolTipWidget::eventFilter( QObject* object, QEvent* event )
{

    if( object != parent() ) return QWidget::eventFilter( object, event );
    switch( event->type() )
    {
        case QEvent::Leave:
        case QEvent::HoverLeave:
        hide();
        break;

        default: break;
    }

    return QWidget::eventFilter( object, event );
}

//_______________________________________________________
void FileRecordToolTipWidget::hide( void )
{
    timer_.stop();
    QWidget::hide();
}

//_______________________________________________________
void FileRecordToolTipWidget::showDelayed( int delay )
{
    if( !enabled_ ) return;
    if( isVisible() ) hide();
    timer_.start( delay, this );
}

//_______________________________________________________
void FileRecordToolTipWidget::show( void )
{
    // stop timer
    timer_.stop();

    // check mouse is still in relevant rect
    if( !_checkMousePosition() ) return;

    // adjust position and show
    _adjustPosition();
    QWidget::show();
}

//_______________________________________________________
void FileRecordToolTipWidget::paintEvent( QPaintEvent* event )
{
    QPainter painter( this );
    painter.setClipRegion( event->region() );

    QStyleOptionFrame opt;
    opt.init(this);
    style()->drawPrimitive(QStyle::PE_PanelTipLabel, &opt, &painter, this );
    return QWidget::paintEvent( event );
}

//_______________________________________________________
void FileRecordToolTipWidget::mousePressEvent( QMouseEvent* event )
{

    hide();
    return QWidget::mousePressEvent( event );

}

//_____________________________________________
void FileRecordToolTipWidget::timerEvent( QTimerEvent* event )
{
    if( event->timerId() == timer_.timerId() )
    {

        timer_.stop();
        show();
        return;

    } else return QWidget::timerEvent( event );
}

//_______________________________________________________
bool FileRecordToolTipWidget::_checkMousePosition( void ) const
{ return rect_.contains( QCursor::pos() ); }

//_______________________________________________________
void FileRecordToolTipWidget::_adjustPosition( void )
{

    // get tooltip size
    const QSize size( sizeHint() );

    // desktop size
    QDesktopWidget* desktop( qApp->desktop() );
    QRect desktopGeometry( desktop->screenGeometry( desktop->screenNumber( parentWidget() ) ) );

    // set geometry
    int left = QCursor::pos().x();
    left = qMax( left, desktopGeometry.left() );
    left = qMin( left, desktopGeometry.right() - size.width() );

    // first try placing widget below item
    const int margin = 5;
    int top = rect_.bottom() + margin;
    if( top > desktopGeometry.bottom() - size.height() ) top = rect_.top() - margin - size.height();

    move( QPoint( left, top ) );

}

//_____________________________________________
void FileRecordToolTipWidget::_updateConfiguration( void )
{
    Debug::Throw( "FileRecordToolTipWidget::_updateConfiguration.\n" );
    if( XmlOptions::get().contains( "SHOW_TOOLTIPS" ) ) setEnabled( XmlOptions::get().get<bool>( "SHOW_TOOLTIPS" ) );
    if( XmlOptions::get().contains( "TOOLTIPS_PIXMAP_SIZE" ) ) setPixmapSize( XmlOptions::get().get<unsigned int>( "TOOLTIPS_PIXMAP_SIZE" ) );
    if( XmlOptions::get().contains( "TOOLTIPS_MASK" ) ) setMask( XmlOptions::get().get<unsigned int>( "TOOLTIPS_MASK" ) );
}
