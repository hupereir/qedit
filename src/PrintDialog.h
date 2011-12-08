#ifndef PrintDialog_h
#define PrintDialog_h

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
*
*******************************************************************************/

#include "BrowsedLineEditor.h"
#include "CustomDialog.h"
#include "CustomComboBox.h"
#include "LineEditor.h"
#include "File.h"

#include <QtGui/QRadioButton>
#include <QtGui/QCheckBox>
#include <QtGui/QSpinBox>
#include <QtCore/QString>

//! print document
class PrintDialog: public CustomDialog
{

    //! Qt meta object declaration
    Q_OBJECT;

    public:

    //! constructor
    PrintDialog( QWidget* parent );

    //! file
    void setFile( const File& file );

    //! max line size
    void setMaximumLineSize( const int& value )
    {
        if( value <= 0 ) wrapCheckBox_->setChecked( false );
        else {
            wrapCheckBox_->setChecked( true );
            maximumLineSize_->setValue( value );
        }

    }

    //! maximum line size
    int maximumLineSize( void ) const
    { return wrapCheckBox_->isChecked() ? maximumLineSize_->value():0; }

    //! print mode
    enum Mode
    {
        PDF,
        HTML
    };

    //! mode
    void setMode( const Mode& mode )
    {
        if( mode == PDF ) pdfCheckBox_->setChecked( true );
        else htmlCheckBox_->setChecked( true );
    }

    //! mode
    Mode mode( void ) const
    { return pdfCheckBox_->isChecked() ? PDF:HTML; }

    //! file
    QString destinationFile( void ) const
    { return destinationEditor_->editor().text(); }

    //! use command
    void setUseCommand( const bool& value )
    { commandCheckBox_->setChecked( value ); }

    //! use command
    bool useCommand( void ) const
    { return commandCheckBox_->isChecked(); }

    //! command
    QString command( void ) const
    { return commandEditor_->currentText(); }

    //! set command manually
    void setCommand( QString command )
    { commandEditor_->setEditText( command ); }

    //! add commands to the combo-box list
    void addCommand( QString command )
    { commandEditor_->addItem( command ); }

    private slots:

    //! update checkboxes
    void _updateCheckBoxes( void );

    //! update print command
    void _updateFile( void );

    //! browse print command
    void _browseCommand( void );

    protected:

    //! destination
    BrowsedLineEditor& _destinationEditor( void ) const
    { return *destinationEditor_; }

    //! command editor
    CustomComboBox& _commandEditor( void ) const
    { return *commandEditor_; }

    private:

    //! a2ps checkbox
    QRadioButton* htmlCheckBox_;

    //! a2ps checkbox
    QRadioButton* pdfCheckBox_;

    //! postscript file
    BrowsedLineEditor* destinationEditor_;

    //! wrap lines
    QCheckBox* wrapCheckBox_;

    //! max line size
    QSpinBox* maximumLineSize_;

    //! command check box
    QCheckBox* commandCheckBox_;

    //! print command
    CustomComboBox* commandEditor_;

};
#endif
