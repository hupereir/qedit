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

#include "InformationWidget.h"

#include <QPainter>

//___________________________________________________________
InformationWidget::InformationWidget( QWidget* parent ):
QWidget( parent ),
Counter( "InformationWidget" )
{}

//___________________________________________________________
void InformationWidget::paintEvent( QPaintEvent* event )
{
    QPainter painter( this );
    painter.setClipRegion( event->region() );

    auto outlineColor = palette().color( QPalette::Highlight );
    auto backgroundColor( outlineColor );
    backgroundColor.setAlphaF( 0.3 );

    painter.setRenderHints( QPainter::Antialiasing );
    painter.setPen( QPen( outlineColor, 2 ) );
    painter.setBrush( backgroundColor );

    painter.drawRoundedRect( QRectF( rect() ).adjusted( 2, 2, -2, -2 ), 3, 3 );
}
