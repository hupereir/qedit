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
* Any WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* this program.  If not, see <http://www.gnu.org/licenses/>.
*
*******************************************************************************/

#include "TextDisplay.h"
#include "TextDisplay.moc"

#include "AnimatedTabWidget.h"
#include "AnimatedLineEditor.h"
#include "Application.h"
#include "AutoSave.h"
#include "AutoSaveThread.h"
#include "BaseContextMenu.h"
#include "BlockDelimiterDisplay.h"
#include "FileDialog.h"
#include "Color.h"
#include "CustomTextDocument.h"
#include "DocumentClass.h"
#include "DocumentClassManager.h"
#include "DocumentClassMenu.h"
#include "ElidedLabel.h"
#include "FileInformationDialog.h"
#include "FileModifiedDialog.h"
#include "FileRecordProperties.h"
#include "FileRemovedDialog.h"
#include "GridLayout.h"
#include "HighlightBlockData.h"
#include "HighlightBlockFlags.h"
#include "IconEngine.h"
#include "Icons.h"
#include "InformationDialog.h"
#include "LineNumberDisplay.h"
#include "QuestionDialog.h"
#include "QtUtil.h"
#include "Singleton.h"
#include "TextEditorMarginWidget.h"
#include "TextEncodingMenu.h"
#include "TextHighlight.h"
#include "TextIndent.h"
#include "TextMacro.h"
#include "TextSeparator.h"
#include "Util.h"
#include "XmlOptions.h"

#if WITH_ASPELL
#include "SpellDialog.h"
#include "SuggestionMenu.h"
#endif

#include <QApplication>
#include <QAbstractTextDocumentLayout>
#include <QCheckBox>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QMenu>
#include <QScrollBar>
#include <QTextCodec>
#include <QTextLayout>

//___________________________________________________
NewDocumentNameServer& TextDisplay::newDocumentNameServer( void )
{
    static NewDocumentNameServer server;
    return server;
}

//___________________________________________________
QRegExp& TextDisplay::_emptyLineRegExp( void )
{
    static QRegExp regexp( "(^\\s*$)" );
    return regexp;
}

//___________________________________________________
TextDisplay::TextDisplay( QWidget* parent ):
    AnimatedTextEditor( parent ),
    file_( "" ),
    workingDirectory_( Util::workingDirectory() ),

    // store property ids associated to property names
    // this is used to speed-up fileRecord access
    classNamePropertyId_( FileRecord::PropertyId::get( FileRecordProperties::CLASS_NAME ) ),
    iconPropertyId_( FileRecord::PropertyId::get( FileRecordProperties::ICON ) ),
    wrapPropertyId_( FileRecord::PropertyId::get( FileRecordProperties::WRAPPED ) ),
    dictionaryPropertyId_( FileRecord::PropertyId::get( FileRecordProperties::DICTIONARY ) ),
    filterPropertyId_( FileRecord::PropertyId::get( FileRecordProperties::FILTER ) ),

    textEncoding_( "ISO-8859-1" ),
    closed_( false ),
    isNewDocument_( false ),
    className_( "" ),
    ignoreWarnings_( false ),
    showBlockDelimiterAction_( 0 ),
    textHighlight_( 0 ),
    blockDelimiterDisplay_( 0 )
{

    Debug::Throw("TextDisplay::TextDisplay.\n" );

    // disable rich text
    setAcceptRichText( false );

    // text highlight
    textHighlight_ = new TextHighlight( document() );

    // parenthesis highlight
    parenthesisHighlight_ = new ParenthesisHighlight( this );

    // text indent
    indent_ = new TextIndent( this );

    // block delimiter
    blockDelimiterDisplay_ = new BlockDelimiterDisplay( this );
    connect( &textHighlight(), SIGNAL(needSegmentUpdate()), &blockDelimiterDisplay(), SLOT(needUpdate()) );

    // connections
    connect( this, SIGNAL(selectionChanged()), SLOT(_selectionChanged()) );
    connect( this, SIGNAL(cursorPositionChanged()), SLOT(_highlightParenthesis()) );
    connect( this, SIGNAL(indent(QTextBlock,bool)), indent_, SLOT(indent(QTextBlock,bool)) );
    connect( this, SIGNAL(indent(QTextBlock,QTextBlock)), indent_, SLOT(indent(QTextBlock,QTextBlock)) );

    #if WITH_ASPELL

    // install menus
    filterMenu_ = new SPELLCHECK::FilterMenu( this );
    dictionaryMenu_ = new SPELLCHECK::DictionaryMenu( this );

    #endif

    textEncodingMenu_ = new TextEncodingMenu( this );
    connect( textEncodingMenu_, SIGNAL(encodingChanged(QByteArray)), SLOT(_setTextEncoding(QByteArray)) );

    // actions
    _installActions();

    // connections
    // track contents changed for syntax highlighting
    connect( TextDisplay::document(), SIGNAL(contentsChange(int,int,int)), SLOT(_setBlockModified(int,int,int)) );
    connect( TextDisplay::document(), SIGNAL(modificationChanged(bool)), SLOT(_textModified()) );

    // track configuration modifications
    connect( Singleton::get().application(), SIGNAL(configurationChanged()), SLOT(_updateConfiguration()) );
    connect( Singleton::get().application(), SIGNAL(spellCheckConfigurationChanged()), SLOT(_updateSpellCheckConfiguration()) );
    connect( Singleton::get().application(), SIGNAL(documentClassesChanged()), SLOT(updateDocumentClass()) );
    _updateConfiguration();
    _updateSpellCheckConfiguration();

    Debug::Throw( "TextDisplay::TextDisplay - done.\n" );

}

//_____________________________________________________
TextDisplay::~TextDisplay( void )
{

    Debug::Throw() << "TextDisplay::~TextDisplay - key: " << key() << endl;
    if( !( isNewDocument() || file().isEmpty() ) && BASE::KeySet<TextDisplay>( this ).empty() )
    { Singleton::get().application<Application>()->fileCheck().removeFile( file() ); }

}

//_____________________________________________________
int TextDisplay::blockCount( const QTextBlock& block ) const
{

    QTextBlockFormat blockFormat( block.blockFormat() );
    if( blockFormat.boolProperty( TextBlock::Collapsed ) && blockFormat.hasProperty( TextBlock::CollapsedData ) )
    {  return blockFormat.property( TextBlock::CollapsedData ).value<CollapsedBlockData>().blockCount(); }
    else return AnimatedTextEditor::blockCount( block );

}

//_____________________________________________________
void TextDisplay::setModified( const bool& value )
{

    Debug::Throw() << "TextDisplay::setModified - value: " << value << endl;

    // do nothing if state is unchanged
    if( value == document()->isModified() )
    {
        Debug::Throw( "TextDisplay::setModified - unchanged.\n" );
        return;
    }

    if( value && isReadOnly() )
    {
        Debug::Throw( "TextDisplay::setModified - rejected.\n" );
        return;
    }

    document()->setModified( value );

    // ask for update in the parent frame
    if( isActive() && ( file().size() || isNewDocument() ) ) emit needUpdate( MODIFIED );

}

//_____________________________________________________
void TextDisplay::setReadOnly( const bool& value )
{
    Debug::Throw() << "TextDisplay::setReadOnly - value: " << value << endl;

    bool changed = (value != isReadOnly() );
    AnimatedTextEditor::setReadOnly( value );

    if( changed && isActive() ) emit needUpdate( READ_ONLY );
}

//______________________________________________________________________________
void TextDisplay::installContextMenuActions( BaseContextMenu* menu, const bool& )
{

    Debug::Throw( "TextDisplay::installContextMenuActions.\n" );

    // see if tagged blocks are present
    const bool hasTags( hasTaggedBlocks() );
    const bool hasSelection( textCursor().hasSelection() );
    const bool currentBlockTagged( hasTags && isCurrentBlockTagged() );

    // retrieve default context menu
    // second argument is to remove un-necessary actions
    AnimatedTextEditor::installContextMenuActions( menu, false );

    // add specific actions
    menu->insertAction( &wrapModeAction(), &showBlockDelimiterAction() );
    menu->addSeparator();

    // tags submenu
    tagBlockAction_->setText( hasSelection ? tr( "Tag Selected Blocks" ) : tr( "Tag Current Block" ) );
    nextTagAction_->setEnabled( hasTags );
    previousTagAction_->setEnabled( hasTags );
    clearTagAction_->setEnabled( currentBlockTagged );
    clearAllTagsAction_->setEnabled( hasTags );

    BaseContextMenu* tagMenu = new BaseContextMenu( menu );
    tagMenu->setHideDisabledActions( true );
    tagMenu->setTitle( tr( "Tags" ) );

    menu->addMenu( tagMenu );
    tagMenu->addAction( tagBlockAction_ );
    tagMenu->addAction( nextTagAction_ );
    tagMenu->addAction( previousTagAction_ );
    tagMenu->addAction( clearTagAction_ );
    tagMenu->addAction( clearAllTagsAction_ );

    // document class menu
    QMenu* documentClassMenu = new DocumentClassMenu( this );
    documentClassMenu->setTitle( tr( "Select Document Class" ) );
    connect( documentClassMenu, SIGNAL(documentClassSelected(QString)), SLOT(selectClassName(QString)) );
    menu->addMenu( documentClassMenu );

    return;
}

//__________________________________________________________
void TextDisplay::paintMargin( QPainter& painter )
{
    AnimatedTextEditor::paintMargin( painter );
    bool hasBlockDelimiters( hasBlockDelimiterDisplay() && hasBlockDelimiterAction() && showBlockDelimiterAction().isVisible() && showBlockDelimiterAction().isChecked() );
    if( hasBlockDelimiters ) blockDelimiterDisplay().paint( painter );
}

//___________________________________________________________________________
void TextDisplay::synchronize( TextDisplay* other )
{

    Debug::Throw( "TextDisplay::synchronize.\n" );

    // replace base class syntax highlighter prior to calling base class synchronization
    textHighlight_ = &other->textHighlight();

    /* this avoids calling to invalid block of memory which the textHighlight gets deleted
    when changing the document */

    // base class synchronization
    AnimatedTextEditor::synchronize( other );

    // restore connection with document
    // track contents changed for syntax highlighting
    connect( TextDisplay::document(), SIGNAL(contentsChange(int,int,int)), SLOT(_setBlockModified(int,int,int)) );
    connect( TextDisplay::document(), SIGNAL(modificationChanged(bool)), SLOT(_textModified()) );

    // indentation
    textIndent().setPatterns( other->textIndent().patterns() );
    textIndent().setBaseIndentation( other->textIndent().baseIndentation() );

    // parenthesis
    parenthesisHighlight().synchronize( other->parenthesisHighlight() );

    // block delimiters and line numbers
    blockDelimiterDisplay().synchronize( &other->blockDelimiterDisplay() );

    // actions
    textIndentAction().setChecked( other->textIndentAction().isChecked() );
    textHighlightAction().setChecked( other->textHighlightAction().isChecked() );
    parenthesisHighlightAction().setChecked( other->parenthesisHighlightAction().isChecked() );
    showLineNumberAction().setChecked( other->showLineNumberAction().isChecked() );
    showBlockDelimiterAction().setChecked( other->showBlockDelimiterAction().isChecked() );

    // macros
    _setMacros( other->macros() );

    // file
    _setIsNewDocument( other->isNewDocument() );
    _setFile( other->file() );
    _setLastSaved( lastSaved_ );

    // update class name
    setClassName( other->className() );
    setFileCheckData( other->fileCheckData() );

    if( parentWidget() != other->parentWidget() )
    { emit needUpdate( ACTIVE_DISPLAY_CHANGED ); }

}

//____________________________________________
void TextDisplay::setIsNewDocument( void )
{

    Debug::Throw( "TextDisplay::setIsNewDocument.\n" );

    // do nothing if already set
    if( isNewDocument() )
    {
        Debug::Throw( "TextDisplay::setIsNewDocument - done.\n" );
        return;
    }

    // generate filename
    File file( NewDocumentNameServer().get() );
    Debug::Throw() << "TextDisplay::setIsNewDocument - file: " << file << endl;

    // retrieve display and associated
    BASE::KeySet<TextDisplay> displays( this );
    displays.insert( this );
    foreach( TextDisplay* display, displays )
    {

        display->_setIsNewDocument( true );
        display->setClassName( className() );
        display->_updateDocumentClass( File(), true );
        display->_updateSpellCheckConfiguration();
        display->_setFile( file );

        // disable file info action
        display->filePropertiesAction().setEnabled( false );

    }
    Debug::Throw( "TextDisplay::setIsNewDocument - filename set.\n" );

    // perform first autosave
    Application& application( *Singleton::get().application<Application>() );
    application.autoSave().saveFiles( this );
    Debug::Throw( "TextDisplay::setIsNewDocument - done.\n" );

}

//____________________________________________
void TextDisplay::setFile( File file, bool checkAutoSave )
{

    Debug::Throw() << "TextDisplay::setFile " << file << endl;
    if( file.isEmpty() )
    {
        Debug::Throw(0) << "TextDisplay::setFile - invalid file:" << file << endl;
        return;
    }

    // reset class name
    QString className( _recentFiles().add( file ).property(classNamePropertyId_) );
    setClassName( className );

    // expand filename
    file = file.expand();

    // check is there is an "AutoSave" file matching with more recent modification time
    // here, when the diff is working, I could offer the possibility to show a diff between
    // the saved file and the backup
    bool restoreAutoSave( false );
    File tmp( file );

    File autosaved( AutoSaveThread::autoSaveName( tmp ) );
    if( checkAutoSave && autosaved.exists() &&
        ( !tmp.exists() ||
        ( autosaved.lastModified() > tmp.lastModified() && tmp.diff(autosaved) ) ) )
    {
        QString buffer = QString(
            tr( "A more recent version of file '%1'\n"
            "was found at %2.\n"
            "This probably means that the application crashed the last time "
            "The file was edited.\n"
            "Use autosaved version ?" ) ).arg( file ).arg( autosaved );

        if( QuestionDialog( this, buffer ).exec() )
        {
            restoreAutoSave = true;
            tmp = autosaved;
        }
    }

    // remove new document version from name server
    if( isNewDocument() ) { NewDocumentNameServer().remove( TextDisplay::file() ); }

    // retrieve display and associated, update document class
    // this is needed to avoid highlight glitch when oppening file
    BASE::KeySet<TextDisplay> displays( this );
    displays.insert( this );
    foreach( TextDisplay* display, displays )
    {
        display->_setIsNewDocument( false );
        display->_setFile( file );
        display->filePropertiesAction().setEnabled( true );

        display->setClassName( this->className() );
        display->_updateDocumentClass( file, false );
        display->_updateSpellCheckConfiguration( file );

    }

    // check file and try open.
    QFile in( tmp );
    if( in.open( QIODevice::ReadOnly ) )
    {

        // get encoding
        QTextCodec* codec( QTextCodec::codecForName( textEncoding_ ) );
        Q_ASSERT( codec );

        setPlainText( codec->toUnicode(in.readAll()) );
        in.close();

        // update flags
        setModified( false );
        _setIgnoreWarnings( false );

    }

    // save file if restored from autosaved.
    if( restoreAutoSave && !isReadOnly() ) save();

    // perform first autosave
    Application& application( *Singleton::get().application<Application>() );
    application.autoSave().saveFiles( this );
    Debug::Throw( "TextDisplay::setFile - done.\n" );

}

//_______________________________________________________
void TextDisplay::_setFile( const File& file )
{

    Debug::Throw() << "TextDisplay::_setFile - file: " << file << endl;

    file_ = file;
    if( !isNewDocument() && file.exists() )
    {

        _setLastSaved( file.lastModified() );
        _setWorkingDirectory( file.path() );
        _setIgnoreWarnings( false );

        // add file to file check
        Singleton::get().application<Application>()->fileCheck().addFile( file );

    }

    // check if file is read-only
    checkFileReadOnly();

    if( isActive() ) emit needUpdate( FILE_NAME );
    Debug::Throw( "TextDisplay::_setFile - done.\n" );

}

//___________________________________________________________________________
FileRemovedDialog::ReturnCode TextDisplay::checkFileRemoved( void )
{
    Debug::Throw() << "TextDisplay::checkFileRemoved - " << file() << endl;

    // check if warnings are enabled and file is removed. Do nothing otherwise
    if( _ignoreWarnings() || !_fileRemoved() ) return FileRemovedDialog::IGNORE;

    // disable check. This prevents recursion in macOS
    _setIgnoreWarnings( true );

    // ask action from dialog
    const int state( FileRemovedDialog( this, file() ).centerOnWidget( window() ).exec() );

    // restore check
    _setIgnoreWarnings( false );

    // process dialog action
    switch( state )
    {

        case FileRemovedDialog::RESAVE:
        // set document as modified (to force the file to be saved) and save
        setModified( true );
        save();
        break;

        case FileRemovedDialog::SAVE_AS:
        saveAs();
        break;

        case FileRemovedDialog::IGNORE:
        case FileRemovedDialog::CLOSE:
        {
            BASE::KeySet<TextDisplay> displays( this );
            displays.insert( this );
            foreach( TextDisplay* display, displays )
            {
                display->_setIgnoreWarnings( true );
                display->setModified( false );
            }
        }
        break;

        default: break;

    }

    return FileRemovedDialog::ReturnCode( state );

}


//___________________________________________________________________________
FileModifiedDialog::ReturnCode TextDisplay::checkFileModified( void )
{
    Debug::Throw() << "TextDisplay::checkFileModified - " << file() << endl;

    // check if warnings are enabled and file is modified. Do nothing otherwise
    if( _ignoreWarnings() || !_fileModified() ) return FileModifiedDialog::IGNORE;

    // disable check. This prevents recursion in macOS
    _setIgnoreWarnings( true );

    // ask action from dialog
    const int state( FileModifiedDialog( this, file() ).centerOnWidget( window() ).exec() );

    // restore check
    _setIgnoreWarnings( false );

    // perform dialog action
    switch( state )
    {

        case FileModifiedDialog::RESAVE:
        document()->setModified( true );
        save();
        break;

        case FileModifiedDialog::SAVE_AS:
        saveAs();
        break;

        case FileModifiedDialog::RELOAD:
        setModified( false );
        revertToSave();
        break;

        case FileModifiedDialog::IGNORE:
        {
            BASE::KeySet<TextDisplay> displays( this );
            displays.insert( this );
            foreach( TextDisplay* display, displays )
            { display->_setIgnoreWarnings( true ); }
        }
        break;

        default: break;
    }

    return FileModifiedDialog::ReturnCode( state );

}

//___________________________________________________________________________
void TextDisplay::checkFileReadOnly( void )
{
    Debug::Throw( "TextDisplay::checkFileReadOnly.\n" );
    setReadOnly( file().exists() && !file().isWritable() );
}

//____________________________________________
void TextDisplay::clearFileCheckData( void )
{

    Debug::Throw( "TextDisplay::clearFileCheckData.\n" );

    if( fileCheckData().flag() == FileCheck::Data::NONE ) return;

    // clear file check data
    BASE::KeySet<TextDisplay> displays( this );
    displays.insert( this );
    foreach( TextDisplay* display, displays )
    { display->setFileCheckData( FileCheck::Data() ); }

}

//___________________________________________________________________________
void TextDisplay::setFileCheckData( const FileCheck::Data& data )
{
    Debug::Throw( "TextDisplay::setFileCheckData.\n" );

    // check if data flag is different from stored
    bool flagsChanged( data.flag() != fileCheckData_.flag() );

    // update data
    fileCheckData_ = data;

    // emit file modification signal, to update session file frames
    if( flagsChanged ) emit needUpdate( MODIFIED );

}

//___________________________________________________________________________
AskForSaveDialog::ReturnCode TextDisplay::askForSave( const bool& enableAll )
{
    Debug::Throw( "TextDisplay::askForSave.\n" );

    if( !( document()->isModified() && _contentsChanged() ) ) return AskForSaveDialog::Yes;

    AskForSaveDialog::ReturnCodes flags( AskForSaveDialog::Yes | AskForSaveDialog::No | AskForSaveDialog::Cancel );
    if( enableAll ) flags |=  AskForSaveDialog::YesToAll | AskForSaveDialog::NoToAll;

    AskForSaveDialog dialog( this, file(), flags );
    dialog.setWindowTitle( tr( "Save Files - Qedit" ) );
    int state( dialog.centerOnParent().exec() );
    if( state == AskForSaveDialog::Yes ||  state == AskForSaveDialog::YesToAll ) save();
    else if( state == AskForSaveDialog::No ||  state == AskForSaveDialog::NoToAll ) setModified( false );

    return AskForSaveDialog::ReturnCode(state);

}


//___________________________________________________________________________
void TextDisplay::save( void )
{
    Debug::Throw( "TextDisplay::save.\n" );

    // do nothing if not modified
    if( !document()->isModified() ) return;

    // check file name
    if( file().isEmpty() || isNewDocument() ) return saveAs();

    // check is contents differ from saved file
    if( _contentsChanged() )
    {

        // make backup
        if( XmlOptions::get().get<bool>( "BACKUP" ) && file().exists() ) file().backup();

        // open output file
        QFile out( file() );
        if( !out.open( QIODevice::WriteOnly ) )
        {
            InformationDialog( this, QString( tr( "Cannot write to file '%1'. <Save> canceled." ) ).arg( file() ) ).exec();
            return;
        }

        // process automatic macros
        if( !noAutomaticMacrosAction().isChecked() && _hasAutomaticMacros() )
        {

            // first need to expand all collapsed blocks
            if( hasBlockDelimiterDisplay() )
            { blockDelimiterDisplay().expandAllBlocks(); }

            // process macros
            foreach( const TextMacro& macro, macros() )
            {
                if( macro.isAutomatic() )
                {

                    Debug::Throw() << "TextDisplay::save - processing macro named " << macro.name() << endl;
                    _processMacro( macro );
                }
            }
        }

        // get encoding
        QTextCodec* codec( QTextCodec::codecForName( textEncoding_ ) );
        Q_ASSERT( codec );

        // write file
        // make sure that last line ends with "end of line"
        QString text( toPlainText() );
        out.write( codec->fromUnicode( text ) );
        if( !text.isEmpty() && text[text.size()-1] != '\n' ) out.write( "\n" );

        // close
        out.close();

    }

    // update modification state and last_saved time stamp
    setModified( false );
    _setLastSaved( file().lastModified() );
    _setIgnoreWarnings( false );

    // re-add to file checker
    if( !file().isEmpty() )
    { Singleton::get().application<Application>()->fileCheck().addFile( file() ); }


    // retrieve associated displays, update saved time
    foreach( TextDisplay* display, BASE::KeySet<TextDisplay>( this ) )
    { display->_setLastSaved( file().lastModified() ); }

    // add file to menu
    if( !file().isEmpty() )
    { _recentFiles().get( file() ).addProperty( classNamePropertyId_, className() ); }

    return;

}

//___________________________________________________________________________
void TextDisplay::saveAs( void )
{
    Debug::Throw( "TextDisplay::saveAs.\n" );

    // define default file
    File defaultFile( file() );
    if( defaultFile.isEmpty() || isNewDocument() ) defaultFile = File( "Document" ).addPath( workingDirectory() );

    // create file dialog
    FileDialog dialog( this );
    dialog.setFileMode( QFileDialog::AnyFile );
    dialog.setAcceptMode( QFileDialog::AcceptSave );
    dialog.selectFile( defaultFile );
    File file( dialog.getFile() );
    if( file.isNull() ) return;
    else file = file.expand();

    // check if file is directory
    if( file.isDirectory() )
    {
        const QString buffer = QString( tr( "File '%1' is a directory. <Save> canceled." ) ).arg( file );
        InformationDialog( this, buffer ).exec();
        return;
    }

    // remove new document version from name server, and FileCheck, if needed
    if( isNewDocument() ) { NewDocumentNameServer().remove( TextDisplay::file() ); }
    else if( !TextDisplay::file().isEmpty() ) { Singleton::get().application<Application>()->fileCheck().removeFile( TextDisplay::file() ); }

    // update filename and document class for this and associates
    // the class name is reset, to allow a document class
    // matching the new filename to get loaded
    setClassName( "" );

    BASE::KeySet<TextDisplay> displays( this );
    displays.insert( this );
    foreach( TextDisplay* display, displays )
    {

        // update file
        display->_setIsNewDocument( false );
        display->setClassName( className() );
        display->_updateDocumentClass( file, false );
        display->_setFile( file );

        // enable file info action
        display->filePropertiesAction().setEnabled( true );

    }

    // set document as modified and save using new filename
    setModified( true );
    save();

    // rehighlight
    rehighlight();

}

//___________________________________________________________
void TextDisplay::revertToSave( void )
{

    Debug::Throw( "TextDisplay::revertToSave.\n" );

    // store scrollbar positions
    int x( horizontalScrollBar()->value() );
    int y( verticalScrollBar()->value() );

    // store cursor position but remove selection
    int position( textCursor().position() );

    setModified( false );
    setFile( file(), false );

    // restore
    horizontalScrollBar()->setValue( x );
    verticalScrollBar()->setValue( y );

    // adjust cursor postion
    position = qMin( position, toPlainText().size() );

    // restore cursor
    QTextCursor cursor( textCursor() );
    cursor.setPosition( position );
    setTextCursor( cursor );

}

//______________________________________________________________________________
void TextDisplay::_setTextEncoding( const QByteArray& value )
{

    // make sure that modified
    if( value == textEncoding_ ) return;

    // check validity
    QTextCodec* codec( QTextCodec::codecForName( value ) );
    if( !codec ) return;

    QTextCodec* oldCodec( QTextCodec::codecForName( textEncoding_ ) );
    if( !oldCodec ) return;

    Debug::Throw() << "TextDisplay::_setTextEncoding - old codec: " << textEncoding_ << endl;
    Debug::Throw() << "TextDisplay::_setTextEncoding - new codec: " << value << endl;
    if( file_.isEmpty() || isNewDocument_ )
    {

        textEncoding_ = value;
        return;

    } else {

        // save if modified
        QString buffer;
        if( document()->isModified() )
        {

            buffer = QString( tr(
                "Changing text encoding requires that the current document is reloaded.\n"
                "Discard changes to file '%1' ?" ) ).arg( file_.localName() );
            if( !QuestionDialog( this, buffer ).setWindowTitle( tr( "Reload Document - Qedit" ) ).exec() ) return;

        }

        // update
        textEncoding_ = value;

        // need to revert to save with new codec
        revertToSave();

    }

    return;

}

//_______________________________________________________
bool TextDisplay::hasLeadingTabs( void ) const
{
    Debug::Throw( "TextDisplay::hasLeadingTabs.\n" );

    // define regexp to perform replacement
    QRegExp wrong_tab_regexp( _hasTabEmulation() ? _normalTabRegExp():_emulatedTabRegExp() );
    for( QTextBlock block( document()->begin() ); block.isValid(); block = block.next() )
    { if( wrong_tab_regexp.indexIn( block.text() ) >= 0 ) return true; }

    return false;

}

//_______________________________________________________
QString TextDisplay::toPlainText( void ) const
{

    Debug::Throw( "TextDisplay::toPlainText.\n" );

    // check blockDelimiterAction
    if( !( showBlockDelimiterAction().isEnabled() && showBlockDelimiterAction().isChecked() ) )
    { return AnimatedTextEditor::toPlainText(); }

    // output string
    QString out;

    // loop over blocks
    for( QTextBlock block = document()->begin(); block.isValid(); block = block.next() )
    {

        // add current block
        out += block.text();
        if( block.next().isValid() || _blockIsCollapsed( block ) ) out += "\n";

        // add collapsed text
        out += _collapsedText( block );

    }

    return out;

}

//___________________________________________________________________________
bool TextDisplay::ignoreBlock( const QTextBlock& block ) const
{

    // first check if block text match empty line
    if( isEmptyBlock( block ) ) return true;

    // try retrieve highlight data
    HighlightBlockData *data( dynamic_cast<HighlightBlockData*>( block.userData() ) );
    return ( data && data->ignoreBlock() );

}

//___________________________________________________________________________
void TextDisplay::tagBlock( QTextBlock block, const unsigned int& tag )
{
    Debug::Throw( "TextDisplay::tagBlock.\n" );

    HighlightBlockData *data( dynamic_cast<HighlightBlockData*>( block.userData() ) );
    if( !data ) block.setUserData( data = new HighlightBlockData() );
    if( data->hasFlag( tag ) ) return;
    data->setFlag( tag, true );

    switch( tag )
    {
        case TextBlock::DIFF_ADDED:
        setBackground( block, diffAddedColor_ );
        break;

        case TextBlock::DIFF_CONFLICT:
        setBackground( block, diffConflictColor_ );
        break;

        case TextBlock::USER_TAG:
        setBackground( block, userTagColor_ );
        break;

        default: break;

    }

    return;

}

//___________________________________________________________________________
void TextDisplay::clearTag( QTextBlock block, const int& tags )
{
    Debug::Throw() << "TextDisplay::clearTag - key: " << key() << endl;
    TextBlockData *data( static_cast<TextBlockData*>( block.userData() ) );
    if( !data ) return;

    if( tags & TextBlock::DIFF_ADDED && data->hasFlag( TextBlock::DIFF_ADDED ) )
    {
        data->setFlag( TextBlock::DIFF_ADDED, false );
        clearBackground( block );
    }

    if( tags & TextBlock::DIFF_CONFLICT && data->hasFlag( TextBlock::DIFF_CONFLICT ) )
    {
        data->setFlag( TextBlock::DIFF_CONFLICT, false );
        clearBackground( block );
    }

    if( tags & TextBlock::USER_TAG && data->hasFlag( TextBlock::USER_TAG ) )
    {
        data->setFlag( TextBlock::USER_TAG, false );
        clearBackground( block );
    }

    Debug::Throw( "TextDisplay::clearTag - done.\n" );

}


//_____________________________________________________________
bool TextDisplay::isCurrentBlockTagged( void ) const
{

    Debug::Throw( "TextDisplay::isCurrentBlockTagged.\n" );

    QList<QTextBlock> blocks;
    QTextCursor cursor( textCursor() );
    if( cursor.hasSelection() )
    {

        QTextBlock first( document()->findBlock( qMin( cursor.position(), cursor.anchor() ) ) );
        QTextBlock last( document()->findBlock( qMax( cursor.position(), cursor.anchor() ) ) );
        for( QTextBlock block( first ); block.isValid() && block != last;  block = block.next() )
        { blocks << block; }
        if( last.isValid() ) blocks << last;

    } else blocks << cursor.block();

    foreach( const QTextBlock& block, blocks )
    {
        TextBlockData *data( static_cast<TextBlockData*>( block.userData() ) );
        if( data && data->hasFlag( TextBlock::DIFF_ADDED | TextBlock::DIFF_CONFLICT | TextBlock::USER_TAG ) ) return true;
    }

    return false;

}

//_____________________________________________________________
bool TextDisplay::hasTaggedBlocks( void ) const
{

    Debug::Throw( "TextDisplay::hasTaggedBlocks.\n" );

    // loop over block
    for( QTextBlock block( document()->begin() ); block.isValid(); block = block.next() )
    {
        TextBlockData *data( static_cast<TextBlockData*>( block.userData() ) );
        if( data && data->hasFlag( TextBlock::DIFF_ADDED | TextBlock::DIFF_CONFLICT | TextBlock::USER_TAG ) ) return true;
    }

    return false;
}

//___________________________________________________________________________
void TextDisplay::_updateDocumentClass( File file, bool new_document )
{

    Debug::Throw( "TextDisplay::_updateDocumentClass\n" );

    textHighlight().clear();
    textIndent().clear();
    textIndent().setBaseIndentation(0);
    _clearMacros();

    // default document class is empty
    DocumentClass document_class;
    Application& application( *Singleton::get().application<Application>() );

    // try load document class from className
    if( !className().isEmpty() )
    {
        Debug::Throw( "TextDisplay::updateDocumentClass - try use className().\n" );
        document_class = application.classManager().get( className() );
    }

    // try load from file
    if( document_class.name().isEmpty() && !( file.isEmpty() || new_document ) )
    {
        Debug::Throw( "TextDisplay::updateDocumentClass - try use filename.\n" );
        document_class = application.classManager().find( file );
    }

    // use default
    if( document_class.name().isEmpty() )
    {
        Debug::Throw( "TextDisplay::updateDocumentClass - using default.\n" );
        document_class = application.classManager().defaultClass();
    }

    // update class name
    setClassName( document_class.name() );

    // wrap mode
    if( !( file.isEmpty() || new_document ) )
    {

        FileRecord& record( _recentFiles().get( file ) );
        if( record.hasProperty( wrapPropertyId_ ) ) wrapModeAction().setChecked( Str( record.property( wrapPropertyId_ ) ).get<bool>() );
        else if( XmlOptions::get().get<bool>( "WRAP_FROM_CLASS" ) ) wrapModeAction().setChecked( document_class.wrap() );

    } else if( XmlOptions::get().get<bool>( "WRAP_FROM_CLASS" ) ) wrapModeAction().setChecked( document_class.wrap() );

    // need to update tab size here because at the time it was set in _updateConfiguration,
    // the font might not have been right
    _setTabSize( XmlOptions::get().get<int>("TAB_SIZE") );

    // tab emulation
    if( XmlOptions::get().get<bool>( "EMULATE_TABS_FROM_CLASS" ) )
    {
        tabEmulationAction().setChecked( document_class.emulateTabs() );
        if( document_class.tabSize() > 0 ) _setTabSize( document_class.tabSize() );
    }

    // enable actions consequently
    parenthesisHighlightAction().setVisible( !document_class.parenthesis().empty() );
    textHighlightAction().setVisible( !document_class.highlightPatterns().empty() );
    textIndentAction().setVisible( !document_class.indentPatterns().empty() );
    baseIndentAction().setVisible( document_class.baseIndentation() );

    // store into class members
    textHighlight().setPatterns( document_class.highlightPatterns() );
    textHighlight().setParenthesis( document_class.parenthesis() );
    textHighlight().setBlockDelimiters( document_class.blockDelimiters() );

    textIndent().setPatterns( document_class.indentPatterns() );
    textIndent().setBaseIndentation( document_class.baseIndentation() );
    _setMacros( document_class.textMacros() );

    // update block delimiters
    if( blockDelimiterDisplay().expandAllAction().isEnabled() ) blockDelimiterDisplay().expandAllAction().trigger();
    if( blockDelimiterDisplay().setBlockDelimiters( document_class.blockDelimiters() ) ) update();
    showBlockDelimiterAction().setVisible( !document_class.blockDelimiters().empty() );
    _updateMargin();

    // update enability for parenthesis matching
    textHighlight().setParenthesisEnabled(
        textHighlightAction().isChecked() &&
        textHighlight().parenthesisHighlightColor().isValid() &&
        !textHighlight().parenthesis().empty() );

    parenthesisHighlight().setEnabled(
        textHighlightAction().isChecked() &&
        textHighlight().parenthesisHighlightColor().isValid() &&
        !textHighlight().parenthesis().empty() );

    // add information to Menu
    if( !( file.isEmpty() || new_document ) )
    {
        FileRecord& record( _recentFiles().get( file ) );
        record.addProperty( classNamePropertyId_, className() );
        record.addProperty( wrapPropertyId_, Str().assign<bool>( wrapModeAction().isChecked() ) );
        if( !document_class.icon().isEmpty() ) record.addProperty( iconPropertyId_, document_class.icon() );
    }

    // rehighlight text entirely
    // because Pattern Ids may have changed even if the className has not changed.
    #if WITH_ASPELL
    if( textHighlight().isHighlightEnabled() && !textHighlight().spellParser().isEnabled() ) rehighlight();
    #else
    if( textHighlight().isHighlightEnabled() ) rehighlight();
    #endif

    // propagate
    emit needUpdate( DOCUMENT_CLASS );

    return;

}

//_____________________________________________
void TextDisplay::processMacro( QString name )
{

    Debug::Throw() << "TextDisplay::processMacro - " << name << endl;

    // retrieve macro that match argument name
    TextMacro::List::const_iterator macroIter = std::find_if( macros_.begin(), macros_.end(), TextMacro::SameNameFTor( name ) );
    if( macroIter == macros_.end() )
    {
        QString buffer;
        QTextStream( &buffer ) << "Unable to find macro named " << name;
        InformationDialog( this, buffer ).exec();
        return;
    }

    _processMacro( *macroIter );

}

//_______________________________________________________
void TextDisplay::rehighlight( void )
{
    Debug::Throw( "TextDisplay::rehighlight.\n" );

    // set all block to modified
    for( QTextBlock block = document()->begin(); block.isValid(); block = block.next() )
    { _setBlockModified( block ); }

    textHighlight().setDocument( document() );
    Debug::Throw( "TextDisplay::rehighlight. done.\n" );

}


//___________________________________________________________________________
void TextDisplay::clearAllTags( const int& flags )
{

    Debug::Throw( "AnimatedTextEditor::clearAllTags.\n" );

    setUpdatesEnabled( false );
    for( QTextBlock block( document()->begin() ); block.isValid(); block = block.next() )
    { clearTag( block, flags ); }
    setUpdatesEnabled( true );

    // get associated displays and update all
    // this is needed due to the setUpdatesEnabled above
    BASE::KeySet<TextDisplay> displays( this );
    displays.insert( this );
    foreach( TextDisplay* display, displays )
    { display->viewport()->update(); }

}

//_______________________________________
void TextDisplay::selectFilter( const QString& filter )
{
    Debug::Throw( "TextDisplay::selectFilter.\n" );

    #if WITH_ASPELL
    // local reference to interface
    SPELLCHECK::SpellInterface& interface( textHighlight().spellParser().interface() );

    if( filter == interface.filter() || !interface.hasFilter( filter ) ) return;

    // update interface
    interface.setFilter( filter );
    filterMenu_->select( filter );

    // update file record
    if( !( file().isEmpty() || isNewDocument() ) )
    { _recentFiles().get( file() ).addProperty( filterPropertyId_, interface.filter() ); }

    // rehighlight if needed
    if( textHighlight().spellParser().isEnabled() ) rehighlight();
    #else
    Q_UNUSED( filter )
    #endif

    return;

}

//_______________________________________
void TextDisplay::selectDictionary( const QString& dictionary )
{
    Debug::Throw( "TextDisplay::selectDictionary.\n" );

    #if WITH_ASPELL
    // local reference to interface
    SPELLCHECK::SpellInterface& interface( textHighlight().spellParser().interface() );

    if( dictionary == interface.dictionary() || !interface.hasDictionary( dictionary ) ) return;

    // update interface
    interface.setDictionary( dictionary );
    dictionaryMenu_->select( dictionary );

    // update file record
    if( !( file().isEmpty() || isNewDocument() ) )
    { _recentFiles().get( file() ).addProperty( dictionaryPropertyId_, interface.dictionary() ); }

    // rehighlight if needed
    if( textHighlight().spellParser().isEnabled() ) rehighlight();
    #else
    Q_UNUSED( dictionary )
    #endif

    return;

}


//________________________________________________________________
void TextDisplay::selectClassName( QString name )
{
    Debug::Throw( "TextDisplay::SelectClassName.\n" );

    // retrieve all displays matching active
    // and update class name
    BASE::KeySet<TextDisplay> displays( this );
    displays.insert( this );
    foreach( TextDisplay* display, displays )
    {
        display->setClassName( name );
        display->updateDocumentClass();
    }

    // rehighlight
    rehighlight();
}

//_______________________________________________________
bool TextDisplay::event( QEvent* event )
{

    bool hasBlockDelimiters( hasBlockDelimiterDisplay() && hasBlockDelimiterAction() && showBlockDelimiterAction().isVisible() && showBlockDelimiterAction().isChecked() );

    // check that all needed widgets/actions are valid and checked.
    switch (event->type())
    {

        case QEvent::MouseButtonPress:
        if( hasBlockDelimiters )
        {
            blockDelimiterDisplay().mousePressEvent( static_cast<QMouseEvent*>( event ) );
            event->accept();
            return true;
        }
        break;

        default: break;
    }

    return AnimatedTextEditor::event( event );

}

//_______________________________________________________
void TextDisplay::keyPressEvent( QKeyEvent* event )
{

    // check if tab key is pressed
    if(
        event->key() == Qt::Key_Tab &&
        indent_->isEnabled() &&
        !( textCursor().hasSelection() || _boxSelection().state() == BoxSelection::FINISHED ) )
    { emit indent( textCursor().block(), false ); }
    else
    {

        // process key
        AnimatedTextEditor::keyPressEvent( event );

        // indent current paragraph when return is pressed
        if( indent_->isEnabled() && event->key() == Qt::Key_Return && !textCursor().hasSelection() )
        { emit indent( textCursor().block(), true ); }

        // reindent paragraph if needed
        /* remark: this is c++ specific. The list of keys should be set in the document class */
        if( indent_->isEnabled() && ( event->key() == Qt::Key_BraceRight || event->key() == Qt::Key_BraceLeft ) && !textCursor().hasSelection() )
        { emit indent( textCursor().block(), false ); }

    }

    return;
}

//________________________________________________
void TextDisplay::contextMenuEvent( QContextMenuEvent* event )
{

    Debug::Throw( "AnimatedTextEditor::contextMenuEvent.\n" );

    if( _autoSpellContextEvent( event ) ) return;
    else {

        AnimatedTextEditor::contextMenuEvent( event );
        return;

    }

}

//________________________________________________
void TextDisplay::paintEvent( QPaintEvent* event )
{
    AnimatedTextEditor::paintEvent( event );

    // handle block background
    QTextBlock first( cursorForPosition( event->rect().topLeft() ).block() );
    QTextBlock last( cursorForPosition( event->rect().bottomRight() ).block() );

    // create painter and translate from widget to viewport coordinates
    QPainter painter( viewport() );
    painter.setClipRect( event->rect() );
    painter.translate( -scrollbarPosition() );
    painter.setPen( _marginWidget().palette().color( QPalette::WindowText ) );

    // loop over found blocks
    for( QTextBlock block( first ); block != last.next() && block.isValid(); block = block.next() )
    {
        if( !block.blockFormat().boolProperty( TextBlock::Collapsed ) ) continue;

        QRectF block_rect( document()->documentLayout()->blockBoundingRect( block ) );
        block_rect.setWidth( viewport()->width() + scrollbarPosition().x() );
        QLineF line( QPointF( 0, block_rect.bottomLeft().y() ), block_rect.bottomRight() );
        painter.drawLine( line );
    }
    painter.end();

}

//________________________________________________
bool TextDisplay::_autoSpellContextEvent( QContextMenuEvent* event )
{
    Debug::Throw( "TextDisplay::_autoSpellContextEvent.\n" );

    #if WITH_ASPELL

    // check autospell enability
    if( !textHighlight().spellParser().isEnabled() ) return false;

    // block and cursor
    QTextCursor cursor( cursorForPosition( event->pos() ) );
    QTextBlock block( cursor.block() );

    // block data
    HighlightBlockData* data( dynamic_cast<HighlightBlockData*>( block.userData() ) );
    if( !data ) return false;

    // try retrieve misspelled word
    SPELLCHECK::Word word( data->misspelledWord( cursor.position() - block.position() ) );
    if( word.isEmpty() || textHighlight().spellParser().interface().isWordIgnored( word ) )
    { return false; }

    // change selection to misspelled word
    cursor.setPosition( word.position() + block.position(), QTextCursor::MoveAnchor );
    cursor.setPosition( word.position() + word.size() + block.position(), QTextCursor::KeepAnchor );
    setTextCursor( cursor );

    // create suggestion menu
    SPELLCHECK::SuggestionMenu menu( this, word, isReadOnly() );
    menu.interface().setFilter( textHighlight().spellParser().interface().filter() );
    menu.interface().setDictionary( textHighlight().spellParser().interface().dictionary() );

    // set connections
    connect( &menu, SIGNAL(ignoreWord(QString)), SLOT(_ignoreMisspelledWord(QString)) );
    connect( &menu, SIGNAL(suggestionSelected(QString)), SLOT(_replaceMisspelledSelection(QString)) );

    // execute
    menu.exec( event->globalPos() );
    return true;

    #else
    Q_UNUSED( event )
    return false;
    #endif

}

//_____________________________________________________________________
void TextDisplay::_installActions( void )
{

    Debug::Throw( "TextDisplay::_installActions.\n" );

    // actions
    addAction( textIndentMacro_ = new QAction( IconEngine::get( ICONS::INDENT ), "Indent Text", this ) );
    textIndentMacro_->setCheckable( true );
    textIndentMacro_->setChecked( textIndent().isEnabled() );
    connect( textIndentMacro_, SIGNAL(toggled(bool)), SLOT(_toggleTextIndent(bool)) );

    addAction( textHighlightAction_ = new QAction( "Highlight Text", this ) );
    textHighlightAction_->setCheckable( true );
    textHighlightAction_->setChecked( textHighlight().isHighlightEnabled() );
    textHighlightAction_->setShortcut( Qt::Key_F8 );
    textHighlightAction_->setShortcutContext( Qt::WidgetShortcut );
    connect( textHighlightAction_, SIGNAL(toggled(bool)), SLOT(_toggleTextHighlight(bool)) );

    addAction( parenthesisHighlightAction_ = new QAction( "Highlight Parenthesis", this ) );
    parenthesisHighlightAction_->setCheckable( true );
    parenthesisHighlightAction_->setChecked( parenthesisHighlight().isEnabled() );
    connect( parenthesisHighlightAction_, SIGNAL(toggled(bool)), SLOT(_toggleParenthesisHighlight(bool)) );

    addAction( noAutomaticMacrosAction_ = new QAction( "Disable Automatic Actions", this ) );
    noAutomaticMacrosAction_->setCheckable( true );
    noAutomaticMacrosAction_->setChecked( false );
    noAutomaticMacrosAction_->setToolTip( "Do not execute automatic actions loaded from document class when saving document" );
    connect( noAutomaticMacrosAction_, SIGNAL(toggled(bool)), SLOT(_toggleIgnoreAutomaticMacros(bool)) );

    addAction( showBlockDelimiterAction_ =new QAction( "Show Block Delimiters", this ) );
    showBlockDelimiterAction_->setToolTip( "Show/hide block delimiters" );
    showBlockDelimiterAction_->setCheckable( true );
    showBlockDelimiterAction_->setShortcut( Qt::Key_F9 );
    showBlockDelimiterAction_->setShortcutContext( Qt::WidgetShortcut );
    connect( showBlockDelimiterAction_, SIGNAL(toggled(bool)), SLOT(_toggleShowBlockDelimiters(bool)) );

    // autospell
    addAction( autoSpellAction_ = new QAction( IconEngine::get( ICONS::SPELLCHECK ), "Automatic Spellcheck", this ) );
    autoSpellAction_->setShortcut( Qt::Key_F6 );
    autoSpellAction_->setShortcutContext( Qt::WidgetShortcut );
    autoSpellAction_->setCheckable( true );

    #if WITH_ASPELL
    autoSpellAction_->setChecked( textHighlight().spellParser().isEnabled() );
    connect( autoSpellAction_, SIGNAL(toggled(bool)), SLOT(_toggleAutoSpell(bool)) );
    #else
    autoSpellAction_->setVisible( false );
    #endif

    // spell checking
    addAction( spellcheckAction_ = new QAction( IconEngine::get( ICONS::SPELLCHECK ), "Spellcheck...", this ) );
    #if WITH_ASPELL
    connect( spellcheckAction_, SIGNAL(triggered()), SLOT(_spellcheck()) );
    #else
    spellcheckAction_->setVisible( false );
    #endif

    // indent selection
    addAction( indentSelectionAction_ = new QAction( IconEngine::get( ICONS::INDENT ), "Indent Selection", this ) );
    indentSelectionAction_->setShortcut( Qt::CTRL + Qt::Key_I );
    indentSelectionAction_->setShortcutContext( Qt::WidgetShortcut );
    connect( indentSelectionAction_, SIGNAL(triggered()), SLOT(_indentSelection()) );

    // base indentation
    addAction( baseIndentAction_ = new QAction( IconEngine::get( ICONS::INDENT ), "Add Base Indentation", this ) );
    baseIndentAction_->setShortcut( Qt::SHIFT + Qt::CTRL + Qt::Key_I );
    connect( baseIndentAction_, SIGNAL(triggered()), SLOT(_addBaseIndentation()) );

    // replace leading tabs
    addAction( leadingTabsAction_ = new QAction( "Replace leading tabs", this ) );
    connect( leadingTabsAction_, SIGNAL(triggered()), SLOT(_replaceLeadingTabs()) );

    // file information
    addAction( filePropertiesAction_ = new QAction( IconEngine::get( ICONS::INFORMATION ), "Properties...", this ) );
    filePropertiesAction_->setToolTip( "Display current file properties" );
    connect( filePropertiesAction_, SIGNAL(triggered()), SLOT(_fileProperties()) );

    #if WITH_ASPELL

    filterMenuAction_ = filterMenu_->menuAction();
    dictionaryMenuAction_ = dictionaryMenu_->menuAction();

    connect( filterMenu_, SIGNAL(selectionChanged(QString)), SLOT(selectFilter(QString)) );
    connect( dictionaryMenu_, SIGNAL(selectionChanged(QString)), SLOT(selectDictionary(QString)) );

    #endif

    textEncodingMenuAction_ = textEncodingMenu_->menuAction();

    // tag block action
    addAction( tagBlockAction_ = new QAction( IconEngine::get( ICONS::TAG ), "Tag Selected Blocks", this ) );
    connect( tagBlockAction_, SIGNAL(triggered()), SLOT(_tagBlock()) );

    // clear current block tags
    addAction( clearTagAction_ = new QAction( "Clear Current Tag", this ) );
    connect( clearTagAction_, SIGNAL(triggered()), SLOT(_clearTag()) );

    // clear all tags
    addAction( clearAllTagsAction_ = new QAction( "Clear All Tags", this ) );
    connect( clearAllTagsAction_, SIGNAL(triggered()), SLOT(clearAllTags()) );

    // next tag action
    addAction( nextTagAction_ = new QAction( IconEngine::get( ICONS::DOWN ), "Goto Next Tagged Block", this ) );
    connect( nextTagAction_, SIGNAL(triggered()), SLOT(_nextTag()) );
    nextTagAction_->setShortcut( Qt::ALT + Qt::Key_Down );
    nextTagAction_->setShortcutContext( Qt::WidgetShortcut );

    // previous tag action
    addAction( previousTagAction_ = new QAction( IconEngine::get( ICONS::UP ), "Goto Previous Tagged Block", this ) );
    connect( previousTagAction_, SIGNAL(triggered()), SLOT(_previousTag()) );
    previousTagAction_->setShortcut( Qt::ALT + Qt::Key_Up );
    previousTagAction_->setShortcutContext( Qt::WidgetShortcut );

}

//_____________________________________________________________
FileList& TextDisplay::_recentFiles( void ) const
{ return Singleton::get().application<Application>()->recentFiles(); }

//_____________________________________________
bool TextDisplay::_hasAutomaticMacros( void ) const
{ return std::find_if( macros().begin(), macros().end(), TextMacro::isAutomaticFTor() ) != macros().end(); }

//_____________________________________________
void TextDisplay::_processMacro( const TextMacro& macro )
{

    // check display
    if( !isActive() ) return;

    // check if readonly
    if( isReadOnly() ) return;

    // retrieve text cursor
    QTextCursor cursor( textCursor() );
    int cursor_position( 0 );
    int position_begin(0);
    int position_end(0);
    QTextBlock begin;
    QTextBlock end;

    bool hasSelection( cursor.hasSelection() );
    if( hasSelection )
    {

        // retrieve blocks
        position_begin = qMin( cursor.position(), cursor.anchor() );
        position_end = qMax( cursor.position(), cursor.anchor() );
        begin = document()->findBlock( position_begin );
        end = document()->findBlock( position_end );

        // enlarge selection so that it matches begin and end of blocks
        position_begin = begin.position();
        if( position_end == end.position() )
        {
            position_end--;
            end = end.previous();
        } else {  position_end = end.position() + end.length() - 1; }

        cursor_position = position_end;

    } else {

        begin = document()->firstBlock();
        position_begin = begin.position();

        end = document()->lastBlock();
        position_end = end.position() + end.length()-1;
        cursor_position = cursor.position();

    }

    // prepare text from selected blocks
    QString text;
    if( begin == end ) {

        text = begin.text().mid( position_begin - begin.position(), position_end-position_begin );

    } else {

        text = begin.text().mid( position_begin - begin.position() );
        if( begin.next().isValid() || _blockIsCollapsed( begin ) ) text += "\n";
        text += _collapsedText( begin );

        for( QTextBlock block = begin.next(); block.isValid() && block!= end; block = block.next() )
        {
            text += block.text();
            if( block.next().isValid() || _blockIsCollapsed( block ) ) text += "\n";
            text += _collapsedText( block );
        }

        // last block
        text += end.text().left( position_end - end.position() );

    }

    // process macro
    TextMacro::Result result( macro.processText( text, cursor_position ) );
    if( !result.first ) return;

    Debug::Throw() << "TextDisplay::processText - increment: " << result.second << endl;

    // update selection
    cursor.setPosition( position_begin );
    cursor.setPosition( position_end, QTextCursor::KeepAnchor );

    // insert new text
    cursor.insertText( text );

    // restore selection
    if( hasSelection )
    {
        cursor.setPosition( position_begin );
        cursor.setPosition( position_begin + text.size(), QTextCursor::KeepAnchor );
    } else {
        cursor.setPosition( cursor_position + result.second );
    }

    setTextCursor( cursor );

    // replace leading tabs in selection
    if( !_hasTabEmulation() ) { _replaceLeadingTabs( false ); }

    return;

}

//_____________________________________________________________
bool TextDisplay::_contentsChanged( void ) const
{

    Debug::Throw( "TextDisplay::_contentsChanged.\n" );

    // check file
    if( file().isEmpty() || isNewDocument() ) return true;

    // open file
    QFile in( file() );
    if( !in.open( QIODevice::ReadOnly ) ) return true;

    // dump file into character string
    QString file_text( in.readAll() );
    QString text( toPlainText() );

    return (text.size() != file_text.size() || text != file_text );

}

//____________________________________________
bool TextDisplay::_fileRemoved( void ) const
{
    Debug::Throw() << "TextDisplay::_fileRemoved - " << file() << endl;

    // check new document
    if( file().isEmpty() || isNewDocument() ) return false;
    if( !lastSaved().isValid() ) return false;

    /*
    for "afs" files, one should do the check anyway, disregarding
    fileCheckData, because the latter is not triggered properly
    when modifying the file remotely
    */

    // check file flag
    if( !( _fileIsAfs() || fileCheckData().flag() == FileCheck::Data::REMOVED || fileCheckData().flag() == FileCheck::Data::MODIFIED ) )
    { return false; }

    // make sure file is still removed
    if( !file().exists() ) return true;
    else {

        // file has been re-created in the meantime.
        // need to re-ad it to FileChecker
        Singleton::get().application<Application>()->fileCheck().addFile( file() );
        return false;

    }

}

//____________________________________________
bool TextDisplay::_fileModified( void )
{

    Debug::Throw() << "TextDisplay::_fileModified - " << file() << endl;

    // check file
    if( file().isEmpty() || isNewDocument() ) return false;
    if( !( _fileIsAfs() || fileCheckData().flag() == FileCheck::Data::REMOVED || fileCheckData().flag() == FileCheck::Data::MODIFIED ) )
    { return false; }

    if( !lastSaved().isValid() ) return false;

    // when file is on afs, or when file was removed (and recreated)
    // one need to use the filename modification timeStampl in place of the timeStamp contained in fileCheckData
    // because the last one was invalid
    const TimeStamp fileModified( _fileIsAfs() ? TimeStamp(file().lastModified()) : fileCheckData().timeStamp() );
    if( !fileModified.isValid() ) return false;
    if( !(fileModified > lastSaved_ ) ) return false;
    if( !_contentsChanged() ) return false;

    // update last_save to avoid chain questions
    lastSaved_ = fileModified;
    return true;

}

//_____________________________________________________________
void TextDisplay::_setBlockModified( const QTextBlock& block )
{
    // check if highlight is enabled.
    if( !textHighlight().isHighlightEnabled() ) return;

    // retrieve associated block data if any
    // set block as modified so that its highlight content gets reprocessed.
    TextBlockData* data( static_cast<TextBlockData*>( block.userData() ) );
    if( data ) data->setFlag( TextBlock::MODIFIED, true );

}

//_____________________________________________________________
void TextDisplay::_updateTaggedBlocks( void )
{

    Debug::Throw( "TextDisplay::_updateTaggedBlocks.\n" );

    // loop over block
    for( QTextBlock block( document()->begin() ); block.isValid(); block = block.next() )
    {

        TextBlockData *data( static_cast<TextBlockData*>( block.userData() ) );
        if( !( data && data->hasFlag( TextBlock::DIFF_ADDED | TextBlock::DIFF_CONFLICT | TextBlock::USER_TAG ) ) ) continue;

        if( data->hasFlag( TextBlock::DIFF_ADDED ) ) setBackground( block, diffAddedColor_ );
        if( data->hasFlag( TextBlock::DIFF_CONFLICT ) ) setBackground( block, diffConflictColor_ );
        if( data->hasFlag( TextBlock::USER_TAG ) ) setBackground( block, userTagColor_ );

    }

}

//___________________________________________________________________________
bool TextDisplay::_updateMargin( void )
{
    Debug::Throw( "TextDisplay::_updateMargin.\n" );

    AnimatedTextEditor::_updateMargin();
    int left_margin( _leftMargin() );

    blockDelimiterDisplay().setOffset( left_margin );
    if( showBlockDelimiterAction().isChecked() && showBlockDelimiterAction().isVisible() )
    { left_margin += blockDelimiterDisplay().width(); }

    return _setLeftMargin( left_margin );

}

//___________________________________________________________________________
bool TextDisplay::_toggleWrapMode( bool state )
{

    Debug::Throw() << "TextDisplay::_toggleWrapMode - " << (state ? "True":"False") << endl;
    if( !AnimatedTextEditor::_toggleWrapMode( state ) ) return false;

    if( !( file().isEmpty() || isNewDocument() ) )
    { _recentFiles().get( file() ).addProperty( wrapPropertyId_, Str().assign<bool>(state) ); }

    return true;

}

//___________________________________________________________________________
void TextDisplay::_updateConfiguration( void )
{
    Debug::Throw( "TextDisplay::_updateConfiguration.\n" );

    // indentation
    textIndentAction().setChecked( XmlOptions::get().get<bool>( "TEXT_INDENT" ) );

    // syntax highlighting
    textHighlightAction().setChecked( XmlOptions::get().get<bool>( "TEXT_HIGHLIGHT" ) );

    // parenthesis highlight
    textHighlight().setParenthesisHighlightColor( XmlOptions::get().get<BASE::Color>( "PARENTHESIS_COLOR" ) );
    parenthesisHighlightAction().setChecked( XmlOptions::get().get<bool>( "TEXT_PARENTHESIS" ) );

    // block delimiters, line numbers and margin
    showBlockDelimiterAction().setChecked( XmlOptions::get().get<bool>( "SHOW_BLOCK_DELIMITERS" ) );
    noAutomaticMacrosAction().setChecked( XmlOptions::get().get<bool>( "IGNORE_AUTOMATIC_MACROS" ) );

    {
        // font
        QFont font;
        font.fromString( XmlOptions::get().raw( "FIXED_FONT_NAME" ) );
        int line_spacing = QFontMetrics( font ).lineSpacing() + 1;
        blockDelimiterDisplay().setWidth( line_spacing );
        _updateMargin();
    }

    // encoding
    // todo: condition that on whether was modified by menu or not
    _setTextEncoding( XmlOptions::get().raw( "TEXT_ENCODING" ) );
    textEncodingMenu_->select( textEncoding_ );

    // retrieve diff colors
    diffConflictColor_ = XmlOptions::get().get<BASE::Color>( "DIFF_CONFLICT_COLOR" );
    diffAddedColor_ = XmlOptions::get().get<BASE::Color>( "DIFF_ADDED_COLOR" );
    userTagColor_ = XmlOptions::get().get<BASE::Color>( "TAGGED_BLOCK_COLOR" );

    // update paragraph tags
    _updateTaggedBlocks();

}

//___________________________________________________________________________
void TextDisplay::_updateSpellCheckConfiguration( File file )
{

    Debug::Throw() << "TextDisplay::_updateSpellCheckConfiguration - file: " << file << " new document:" << isNewDocument() << endl;

    #if WITH_ASPELL

    // spellcheck configuration
    bool changed( false );
    changed |= textHighlight().spellParser().setColor( QColor( XmlOptions::get().get<BASE::Color>("AUTOSPELL_COLOR") ) );
    changed |= textHighlight().spellParser().setFontFormat( (FORMAT::TextFormatFlags) XmlOptions::get().get<unsigned int>("AUTOSPELL_FONT_FORMAT") );
    textHighlight().updateSpellPattern();
    autoSpellAction().setEnabled( textHighlight().spellParser().color().isValid() );

    // reset filter and dictionaries menu
    dictionaryMenu_->reset();
    filterMenu_->reset();

    // store local reference to spell interface
    SPELLCHECK::SpellInterface& interface( textHighlight().spellParser().interface() );

    // load default filter and dictionaries
    QString filter( XmlOptions::get().raw("DICTIONARY_FILTER") );
    QString dictionary( XmlOptions::get().raw( "DICTIONARY" ) );

    // overwrite with file record
    if( file.isEmpty() ) file = TextDisplay::file();
    if( !( file.isEmpty() || isNewDocument() ) )
    {
        FileRecord& record( _recentFiles().get( file ) );
        if( record.hasProperty( filterPropertyId_ ) && interface.hasFilter( record.property( filterPropertyId_ ) ) )
        {
            filter = record.property( filterPropertyId_ );
            Debug::Throw() << "TextDisplay::_updateSpellCheckConfiguration - filter: " << filter << endl;
        }

        if( record.hasProperty( dictionaryPropertyId_ ) && interface.hasDictionary( record.property( dictionaryPropertyId_ ) ) )
        {
            dictionary = record.property( dictionaryPropertyId_ );
            Debug::Throw() << "TextDisplay::_updateSpellCheckConfiguration - dictionary: " << dictionary << endl;
        }

    }

    // see if one should/can change the dictionary and filter
    if( filter == interface.filter() || interface.setFilter( filter ) )
    {
        filterMenu_->select( filter );
        changed = true;
    }

    if( dictionary == interface.dictionary() || interface.setDictionary( dictionary ) )
    {
        dictionaryMenu_->select( dictionary );
        changed = true;
    }

    // rehighlight if needed
    if( changed && autoSpellAction().isChecked() && autoSpellAction().isEnabled() )
    { rehighlight(); }

    #else
    Q_UNUSED( file )
    #endif

}

//_______________________________________________________
void TextDisplay::_indentCurrentParagraph( void )
{
    Debug::Throw( "TextDisplay::_indentCurrentParagraph.\n" );
    if( !indent_->isEnabled() ) return;
    emit indent( textCursor().block(), false );
}

//_______________________________________________________
void TextDisplay::_toggleTextIndent( bool state )
{

    Debug::Throw( "TextDisplay::_toggleTextIndent.\n" );

    // update text indent
    textIndent().setEnabled( textIndentAction().isEnabled() && state );

    // propagate to other displays
    if( isSynchronized() )
    {
        // temporarely disable synchronization
        // to avoid infinite loop
        setSynchronized( false );

        foreach( TextDisplay* display, BASE::KeySet<TextDisplay>( this ) )
        { if( display->isSynchronized() ) display->textIndentAction().setChecked( state ); }
        setSynchronized( true );

    }

}


//_______________________________________________________
void TextDisplay::_toggleTextHighlight( bool state )
{

    Debug::Throw( "TextDisplay::_toggleTextHighlight.\n" );
    if( textHighlight().setHighlightEnabled( textHighlightAction().isEnabled() && state ) )
    { rehighlight(); }

    // propagate to other displays
    if( isSynchronized() )
    {
        // temporarely disable synchronization
        // to avoid infinite loop
        setSynchronized( false );

        foreach( TextDisplay* display, BASE::KeySet<TextDisplay>( this ) )
        { if( display->isSynchronized() ) display->textHighlightAction().setChecked( state ); }

        setSynchronized( true );

    }

}

//_______________________________________________________
void TextDisplay::_toggleParenthesisHighlight( bool state )
{

    Debug::Throw() << "TextDisplay::_toggleParenthesisHighlight -"
        << " state: " << state
        << " color: " << textHighlight().parenthesisHighlightColor().isValid()
        << " parenthesis: " << textHighlight().parenthesis().empty()
        << endl;

    // propagate to textHighlight
    textHighlight().setParenthesisEnabled(
        state &&
        textHighlight().parenthesisHighlightColor().isValid() &&
        !textHighlight().parenthesis().empty() );

    parenthesisHighlight().setEnabled(
        state &&
        textHighlight().parenthesisHighlightColor().isValid() &&
        !textHighlight().parenthesis().empty() );

    // propagate to other displays
    if( isSynchronized() )
    {
        // temporarely disable synchronization
        // to avoid infinite loop
        setSynchronized( false );

        foreach( TextDisplay* display, BASE::KeySet<TextDisplay>( this ) )
        { if( display->isSynchronized() ) display->parenthesisHighlightAction().setChecked( state ); }
        setSynchronized( true );

    }

    return;
}

//_______________________________________________________
void TextDisplay::_toggleAutoSpell( bool state )
{
    #if WITH_ASPELL
    Debug::Throw( "TextDisplay::_toggleAutoSpell.\n" );

    // enable menus
    // dictionaryMenuAction().setEnabled( state );
    // filterMenuAction().setEnabled( state );

    // propagate to textHighlight
    textHighlight().spellParser().setEnabled( state );
    rehighlight();

    // propagate to other displays
    if( isSynchronized() )
    {
        // temporarely disable synchronization
        // to avoid infinite loop
        setSynchronized( false );

        foreach( TextDisplay* display, BASE::KeySet<TextDisplay>( this ) )
        { if( display->isSynchronized() ) display->autoSpellAction().setChecked( state ); }
        setSynchronized( true );

    }

    return;
    #else
    Q_UNUSED( state )
    #endif
}

//_______________________________________________________
void TextDisplay::_toggleShowBlockDelimiters( bool state )
{

    _updateMargin();

    // need to expand all blocks if block delimiters is being hidden
    if( !state && hasBlockDelimiterDisplay() && blockDelimiterDisplay().expandAllAction().isEnabled() )
    { blockDelimiterDisplay().expandAllAction().trigger(); }

    // update options
    XmlOptions::get().set<bool>( "SHOW_BLOCK_DELIMITERS", state );

    // propagate to other displays
    if( isSynchronized() )
    {

        // temporarely disable synchronization
        // to avoid infinite loop
        setSynchronized( false );

        foreach( TextDisplay* display, BASE::KeySet<TextDisplay>( this ) )
        { if( display->isSynchronized() ) display->showBlockDelimiterAction().setChecked( state ); }
        setSynchronized( true );

    }

    return;
}

//_______________________________________________________
void TextDisplay::_toggleIgnoreAutomaticMacros( bool state )
{

    // update options
    XmlOptions::get().set<bool>( "IGNORE_AUTOMATIC_MACROS", state );

    return;
}

//_______________________________________________________
void TextDisplay::_spellcheck( void )
{
    Debug::Throw( "TextDisplay::_spellcheck.\n" );

    #if WITH_ASPELL
    // create dialog
    SPELLCHECK::SpellDialog dialog( this );
    dialog.interface().setIgnoredWords( textHighlight().spellParser().interface().ignoredWords() );

    // default dictionary from XmlOptions
    QString defaultFilter( XmlOptions::get().raw("DICTIONARY_FILTER") );
    QString defaultDictionary( XmlOptions::get().raw( "DICTIONARY" ) );

    // try overwrite with file record
    if( !( file().isEmpty()  || isNewDocument() ) )
    {

        FileRecord& record( _recentFiles().get( file() ) );
        if( !( record.hasProperty( filterPropertyId_ ) && dialog.setFilter( record.property( filterPropertyId_ ) ) ) )
        { dialog.setFilter( defaultFilter ); }

        if( !( record.hasProperty( dictionaryPropertyId_ ) && dialog.setDictionary( record.property( dictionaryPropertyId_ ) ) ) )
        { dialog.setDictionary( defaultDictionary ); }

    }  else {

        dialog.setFilter( defaultFilter );
        dialog.setDictionary( defaultDictionary );

    }

    // connections
    connect( &dialog, SIGNAL(filterChanged(QString)), SLOT(selectFilter(QString)) );
    connect( &dialog, SIGNAL(dictionaryChanged(QString)), SLOT(selectDictionary(QString)) );

    dialog.nextWord();
    dialog.exec();

    // try overwrite with file record
    if( !( file().isEmpty() || isNewDocument() ) )
    {
        _recentFiles().get( file() )
            .addProperty( filterPropertyId_, dialog.filter() )
            .addProperty( dictionaryPropertyId_, dialog.dictionary() );
    }

    textHighlight().spellParser().interface().mergeIgnoredWords( dialog.interface().ignoredWords() );

    #endif

}

//_______________________________________________________
void TextDisplay::_indentSelection( void )
{
    Debug::Throw( "TextDisplay::_indentSelection.\n" );

    // check activity, indentation and text selection
    if( !indent_->isEnabled() ) return;

    // retrieve text cursor
    QTextCursor cursor( textCursor() );
    if( !cursor.hasSelection() ) return;

    // retrieve blocks
    QTextBlock begin( document()->findBlock( qMin( cursor.position(), cursor.anchor() ) ) );
    QTextBlock end( document()->findBlock( qMax( cursor.position(), cursor.anchor() ) ) );

    // need to remove selection otherwise the first adding of a tab
    // will remove the entire selection.

    cursor.clearSelection();
    emit indent( begin, end );

    // select all indented blocks
    cursor.setPosition( begin.position(), QTextCursor::MoveAnchor );
    cursor.setPosition( end.position()+end.length()-1, QTextCursor::KeepAnchor );
    setTextCursor( cursor );

    return;
}

//_______________________________________________________
void TextDisplay::_addBaseIndentation( void )
{
    Debug::Throw( "TextDisplay::_addBaseIndentation.\n" );

    // check activity, indentation and text selection
    if( !indent_->baseIndentation() ) return;

    // define regexp to perform replacement
    QRegExp leading_space_regexp( "^\\s*" );
    QString replacement( indent_->baseIndentation(), ' ' );

    // define blocks to process
    QTextBlock begin;
    QTextBlock end;

    // retrieve cursor
    // retrieve text cursor
    QTextCursor cursor( textCursor() );
    if( !cursor.hasSelection() ) return;

    int position_begin( qMin( cursor.position(), cursor.anchor() ) );
    int position_end( qMax( cursor.position(), cursor.anchor() ) );
    begin = document()->findBlock( position_begin );
    end = document()->findBlock( position_end );

    // store blocks
    QList<QTextBlock> blocks;
    for( QTextBlock block = begin; block.isValid() && block != end; block = block.next() )
    { blocks << block; }
    blocks << end;

    // loop over blocks
    for( QList<QTextBlock>::iterator iter = blocks.begin(); iter != blocks.end(); ++iter )
    {
        // check block
        if( !iter->isValid() ) continue;

        // retrieve text
        QString text( iter->text() );

        // look for leading tabs
        if( leading_space_regexp.indexIn( text ) < 0 ) continue;

        // select with cursor
        QTextCursor cursor( *iter );
        cursor.movePosition( QTextCursor::StartOfBlock, QTextCursor::MoveAnchor );
        cursor.setPosition( cursor.position() + leading_space_regexp.matchedLength(), QTextCursor::KeepAnchor );

        cursor.insertText( replacement );


    }

    // indent
    emit indent( begin, end );

    // enable updates
    setUpdatesEnabled( true );

    return;
}

//_______________________________________________________
void TextDisplay::_replaceLeadingTabs( const bool& confirm )
{
    Debug::Throw( "TextDisplay::_replaceLeadingTabs.\n" );

    // ask for confirmation
    if( confirm )
    {

        QString buffer;
        QTextStream what( &buffer );
        if( _hasTabEmulation() ) what << "Replace all leading tabs with space characters ?";
        else what << "Replace all leading spaces with tab characters ?";
        if( !QuestionDialog( this, buffer ).exec() ) return;

    }

    // disable updates
    setUpdatesEnabled( false );

    // define regexp to perform replacement
    QRegExp wrong_tab_regexp( _hasTabEmulation() ? _normalTabRegExp():_emulatedTabRegExp() );
    QString wrong_tab( _hasTabEmulation() ? normalTabCharacter():emulatedTabCharacter() );

    // define blocks to process
    QTextBlock begin;
    QTextBlock end;

    // retrieve cursor
    QTextCursor cursor( textCursor() );
    if( cursor.hasSelection() )
    {

        int position_begin( qMin( cursor.position(), cursor.anchor() ) );
        int position_end( qMax( cursor.position(), cursor.anchor() ) );
        begin = document()->findBlock( position_begin );
        end = document()->findBlock( position_end );

    } else {

        begin = document()->begin();
        end = document()->end();

    }

    // store blocks
    QList<QTextBlock> blocks;
    for( QTextBlock block = begin; block.isValid() && block != end; block = block.next() )
    { blocks << block; }
    blocks << end;

    // loop over blocks
    for( QList<QTextBlock>::iterator iter = blocks.begin(); iter != blocks.end(); ++iter )
    {
        // check block
        if( !iter->isValid() ) continue;

        // retrieve text
        QString text( iter->text() );

        // look for leading tabs
        if( wrong_tab_regexp.indexIn( text ) < 0 ) continue;

        // select with cursor
        QTextCursor cursor( *iter );
        cursor.movePosition( QTextCursor::StartOfBlock, QTextCursor::MoveAnchor );
        cursor.setPosition( cursor.position() + wrong_tab_regexp.matchedLength(), QTextCursor::KeepAnchor );

        // create replacement string and insert.
        QString buffer;
        for( int i=0; i< int(wrong_tab_regexp.matchedLength()/wrong_tab.size()); i++ )
        { buffer += tabCharacter(); }
        cursor.insertText( buffer );

    }

    // enable updates
    setUpdatesEnabled( true );

    Debug::Throw( "TextDisplay::_replaceLeadingTabs - done.\n" );
    return;
}


//_______________________________________________________
void TextDisplay::_fileProperties( void )
{
    Debug::Throw( "TextDisplay::_fileProperties.\n" );
    if( file().isEmpty() || isNewDocument() ) return;

    // prior to showing the dialog
    // one should add needed tab for misc information
    const FileRecord& record(  _recentFiles().get( file() ) );
    FileInformationDialog dialog( this, record );

    // add additional informations frame
    QWidget* box( new QWidget() );
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setMargin(5);
    layout->setSpacing( 5 );
    box->setLayout( layout );
    Debug::Throw( "TextDisplay::_fileProperties - Miscellaneous tab booked.\n" );

    GridLayout* gridLayout = new GridLayout();
    gridLayout->setMargin(0);
    gridLayout->setSpacing( 5 );
    gridLayout->setMaxCount( 2 );
    gridLayout->setColumnAlignment( 0, Qt::AlignRight|Qt::AlignVCenter );
    layout->addLayout( gridLayout );

    // number of characters
    FileInformationDialog::Item* item;
    item = new FileInformationDialog::Item( box, gridLayout );
    item->setKey( "Number of characters:" );
    item->setValue( QString().setNum(toPlainText().size()) );

    // number of lines
    item = new FileInformationDialog::Item( box, gridLayout );
    item->setKey( "Number of lines:" );
    item->setValue( QString().setNum( AnimatedTextEditor::blockCount()) );

    gridLayout->addWidget( new QLabel( "Text highlighting:", box ) );
    {
        QCheckBox* checkbox( new QCheckBox( box ) );
        checkbox->setChecked( textHighlight().isHighlightEnabled() );
        checkbox->setEnabled( false );
        gridLayout->addWidget( checkbox );
    }

    gridLayout->addWidget( new QLabel( "Paragraph highlighting:", box ) );
    {
        QCheckBox* checkbox( new QCheckBox( box ) );
        checkbox->setChecked( blockHighlightAction().isChecked() );
        checkbox->setEnabled( false );
        gridLayout->addWidget( checkbox );
    }

    gridLayout->addWidget( new QLabel( "Parenthesis highlighting:", box ) );
    {
        QCheckBox* checkbox( new QCheckBox( box ) );
        checkbox->setChecked( textHighlight().isParenthesisEnabled() );
        checkbox->setEnabled( false );
        gridLayout->addWidget( checkbox );
    }

    gridLayout->addWidget( new QLabel( "Text indentation:", box ) );
    {
        QCheckBox* checkbox( new QCheckBox( box ) );
        checkbox->setChecked(  textIndent().isEnabled() );
        checkbox->setEnabled( false );
        gridLayout->addWidget( checkbox );
    }

    gridLayout->addWidget( new QLabel( "Text wrapping:", box ) );
    {
        QCheckBox* checkbox( new QCheckBox( box ) );
        checkbox->setChecked( wrapModeAction().isChecked() );
        checkbox->setEnabled( false );
        gridLayout->addWidget( checkbox );
    }

    gridLayout->addWidget( new QLabel( "Tab emulation:", box ) );
    {
        QCheckBox* checkbox( new QCheckBox( box ) );
        checkbox->setChecked( tabEmulationAction().isChecked() );
        checkbox->setEnabled( false );
        gridLayout->addWidget( checkbox );
    }

    // document class
    item = new FileInformationDialog::Item( box, gridLayout, FileInformationDialog::Elide );
    item->setKey( "Document class file name:" );
    const DocumentClass& documentClass( Singleton::get().application<Application>()->classManager().get( className() ) );
    item->setValue( documentClass.file() );

    // also assign icon to dialog
    dialog.setIcon( IconEngine::get( documentClass.icon() ) );

    // autosave
    item = new FileInformationDialog::Item( box, gridLayout, FileInformationDialog::Elide|FileInformationDialog::Selectable );
    item->setKey( "Auto-save file name:" );
    item->setValue( AutoSaveThread::autoSaveName( file() ) );

    layout->addStretch();

    dialog.tabWidget().addTab( box, "Information" );

    // execute dialog
    dialog.centerOnWidget( window() ).exec();

}

//_____________________________________________________________
void TextDisplay::_setBlockModified( int position, int, int added )
{
    QTextBlock begin( document()->findBlock( position ) );
    QTextBlock end(  document()->findBlock( position + added ) );

    for( QTextBlock block = begin; block.isValid() && block != end; block = block.next() )
    { _setBlockModified( block ); }

    _setBlockModified( end );

}

//__________________________________________________
void TextDisplay::_textModified( void )
{
    Debug::Throw( "TextDisplay::_textModified.\n" );

    // document should never appear modified
    // for readonly displays
    if( document()->isModified() && isReadOnly() ) document()->setModified( false );
    if( isActive() && ( file().size() || isNewDocument() ) ) emit needUpdate( MODIFIED );

}

//__________________________________________________
void TextDisplay::_ignoreMisspelledWord( QString word )
{
    Debug::Throw() << "TextDisplay::_ignoreMisspelledWord - word: " << word << endl;
    #if WITH_ASPELL
    textHighlight().spellParser().interface().ignoreWord( word );
    rehighlight();
    #endif
    return;

}

//__________________________________________________
void TextDisplay::_replaceMisspelledSelection( QString word )
{

    #if WITH_ASPELL
    Debug::Throw() << "TextDisplay::_replaceMisspelledSelection - word: " << word << endl;
    QTextCursor cursor( textCursor() );
    cursor.insertText( word );
    #else
    Q_UNUSED( word )
    #endif
    return;

}

//__________________________________________________
void TextDisplay::_highlightParenthesis( void )
{

    if( !( hasTextHighlight() && textHighlight().isParenthesisEnabled() ) ) return;

    // clear previous parenthesis
    {
        QList<QTextBlock> dirty( parenthesisHighlight().clear() );
        foreach( const QTextBlock& block, dirty )
        { textHighlight().rehighlightBlock( block ); }
    }

    // retrieve TextCursor
    QTextCursor cursor( textCursor() );
    if( cursor.atBlockStart() ) return;

    // retrieve block
    QTextBlock block( cursor.block() );

    // store local position in block
    int position(cursor.position()-block.position());

    // get current PatternLocation set
    PatternLocationSet locations;
    HighlightBlockData *data = dynamic_cast<HighlightBlockData*>( block.userData() );
    if( data ) locations = data->locations();

    QString text( block.text() );
    const TextParenthesis::List& parenthesis( textHighlight().parenthesis() );

    // check against opening parenthesis
    bool found( false );
    TextParenthesis::List::const_iterator iter( std::find_if(
        parenthesis.begin(), parenthesis.end(),
        TextParenthesis::FirstElementFTor( text.left( position ) ) ) );

    if( iter != parenthesis.end() )
    {

        // store commented state
        const bool isComment( locations.isCommented( position - iter->first().size() ) );

        int increment( 0 );
        while( block.isValid() && !found )
        {
            // retrieve block text
            QString text( block.text() );

            // append collapsed data if any
            if( data && data->hasFlag( TextBlock::COLLAPSED ) )
            {

                QTextBlockFormat blockFormat( block.blockFormat() );
                if( !blockFormat.hasProperty( TextBlock::CollapsedData ) )
                {
                    Debug::Throw(0) << "TextDisplay::_highlightParenthesis - missing CollapsedData property" << endl;
                    continue;
                }

                CollapsedBlockData collapsedBlockData( blockFormat.property( TextBlock::CollapsedData ).value<CollapsedBlockData>() );
                text += collapsedBlockData.toPlainText();

            }

            // parse text
            while( (position = iter->regexp().indexIn( text, position ) ) >= 0 )
            {

                // make sure comment status matches
                if( isComment == locations.isCommented( position ) )
                {
                    if( const_cast<QRegExp&>(iter->regexp()).cap() == iter->second() ) increment--;
                    else if( const_cast<QRegExp&>(iter->regexp()).cap() == iter->first() ) increment++;

                    if( increment < 0 )
                    {
                        found = true;
                        break;
                    }

                }

                position += iter->regexp().matchedLength();

            }

            if( !found )
            {
                // goto next block and update location set
                block = block.next();
                data = dynamic_cast<HighlightBlockData*>( block.userData() );
                if( data ) locations = data->locations();
                else locations.clear();

                position = 0;
            }
        }
    }

    // if not found, check against closing parenthesis
    if( !( found || (iter =
        std::find_if(
        parenthesis.begin(), parenthesis.end(),
        TextParenthesis::SecondElementFTor( text.left( position ) ) )) == parenthesis.end()  ) )
    {

        // store commented state
        const bool isComment( locations.isCommented( position - iter->second().size() ) );

        int increment( 0 );
        position -= (iter->second().size() );
        while( block.isValid() && !found )
        {
            // retrieve block text
            QString text( block.text() );

            // append collapsed data if any
            if( data && data->hasFlag( TextBlock::COLLAPSED ) )
            {

                QTextBlockFormat blockFormat( block.blockFormat() );
                if( !blockFormat.hasProperty( TextBlock::CollapsedData ) )
                {
                    Debug::Throw(0) << "TextDisplay::_highlightParenthesis - missing CollapsedData property" << endl;
                    continue;
                }
                CollapsedBlockData collapsedBlockData( blockFormat.property( TextBlock::CollapsedData ).value<CollapsedBlockData>() );
                text += collapsedBlockData.toPlainText();

            }

            if( position < 0 ) position = text.length();

            // parse text
            while( position >= 0 && (position = iter->regexp().lastIndexIn( text.left(position) ) ) >= 0 )
            {

                if( isComment == locations.isCommented( position ) )
                {

                    if( const_cast<QRegExp&>(iter->regexp()).cap() == iter->first() ) increment--;
                    else if( const_cast<QRegExp&>(iter->regexp()).cap() == iter->second() ) increment++;

                    if( increment < 0 )
                    {
                        found = true;
                        break;
                    }

                }

            }

            if( !found )
            {
                // goto previous block and update locationSet
                block = block.previous();
                data = dynamic_cast<HighlightBlockData*>( block.userData() );
                if( data ) locations = data->locations();
                else locations.clear();
                if( block.isValid() ) position = -1 ;
            }

        }
    }

    // highlight
    if( found && position < block.length() )
    {
        parenthesisHighlight().highlight( position + block.position(), iter->regexp().matchedLength() );
        textHighlight().rehighlightBlock( block );
    }

    return;

}

//__________________________________________________
void TextDisplay::_tagBlock( void )
{

    Debug::Throw( "TextDisplay::_tagBlock.\n" );
    QList<QTextBlock> blocks;
    QTextCursor cursor( textCursor() );
    if( cursor.hasSelection() )
    {

        QTextBlock first( document()->findBlock( qMin( cursor.position(), cursor.anchor() ) ) );
        QTextBlock last( document()->findBlock( qMax( cursor.position(), cursor.anchor() ) ) );
        for( QTextBlock block( first ); block.isValid() && block != last;  block = block.next() )
        { blocks << block; }
        if( last.isValid() ) blocks << last;

    } else blocks << cursor.block();

    // clear background for selected blocks
    for( QList<QTextBlock>::iterator iter = blocks.begin(); iter != blocks.end(); ++iter )
    { if( iter->isValid() ) tagBlock( *iter, TextBlock::USER_TAG ); }

}

//__________________________________________________
void TextDisplay::_nextTag( void )
{
    Debug::Throw( "TextDisplay::_nextTag.\n" );
    QTextCursor cursor( textCursor() );
    QTextBlock block( cursor.block() );
    TextBlockData* data;

    // first skipp blocks that have tags if the first one has
    while(
        block.isValid() &&
        (data = static_cast<TextBlockData*>( block.userData() ) ) &&
        data->hasFlag( TextBlock::ALL_TAGS ) )
    { block = block.next(); }

    // skip blocks with no tag
    while(
        block.isValid() &&
        !((data = static_cast<TextBlockData*>( block.userData() ) ) &&
        data->hasFlag( TextBlock::ALL_TAGS ) ) )
    { block = block.next(); }

    if( !block.isValid() )
    {
        InformationDialog( this, "No tagged block found." ).exec();
        return;
    }

    // update cursor
    cursor.setPosition( block.position() );
    setTextCursor( cursor );
    return;

}

//__________________________________________________
void TextDisplay::_previousTag( void )
{
    Debug::Throw( "TextDisplay::_previousTag.\n" );
    QTextCursor cursor( textCursor() );
    QTextBlock block( cursor.block() );
    TextBlockData* data;

    // first skip blocks that have tags if the first one has
    while(
        block.isValid() &&
        (data = static_cast<TextBlockData*>( block.userData() ) ) &&
        data->hasFlag( TextBlock::ALL_TAGS ) )
    { block = block.previous(); }

    // skip blocks with no tag
    while(
        block.isValid() &&
        !((data = static_cast<TextBlockData*>( block.userData() ) ) &&
        data->hasFlag( TextBlock::ALL_TAGS ) ) )
    { block = block.previous(); }

    if( !block.isValid() )
    {
        InformationDialog( this, "No tagged block found." ).exec();
        return;
    }

    // update cursor
    cursor.setPosition( block.position() );
    setTextCursor( cursor );
    return;

}

//___________________________________________________________________________
void TextDisplay::_clearTag( void )
{

    Debug::Throw( "AnimatedTextEditor::_clearTag.\n" );

    QList<QTextBlock> blocks;
    QTextCursor cursor( textCursor() );
    if( cursor.hasSelection() )
    {

        QTextBlock first( document()->findBlock( qMin( cursor.position(), cursor.anchor() ) ) );
        QTextBlock last( document()->findBlock( qMax( cursor.position(), cursor.anchor() ) ) );
        for( QTextBlock block( first ); block.isValid() && block != last;  block = block.next() )
        { blocks << block; }
        if( last.isValid() ) blocks << last;

    } else {

        // add previous blocks and current
        for( QTextBlock block( cursor.block() ); block.isValid(); block = block.previous() )
        {
            TextBlockData *data( static_cast<TextBlockData*>( block.userData() ) );
            if( data && data->hasFlag( TextBlock::DIFF_ADDED | TextBlock::DIFF_CONFLICT | TextBlock::USER_TAG ) ) blocks << block;
            else break;
        }


        // add previous blocks and current
        for( QTextBlock block( cursor.block().next() ); block.isValid(); block = block.next() )
        {
            TextBlockData *data( static_cast<TextBlockData*>( block.userData() ) );
            if( data && data->hasFlag( TextBlock::DIFF_ADDED | TextBlock::DIFF_CONFLICT | TextBlock::USER_TAG ) ) blocks << block;
            else break;
        }

    }

    // clear background for selected blocks
    for( QList<QTextBlock>::iterator iter = blocks.begin(); iter != blocks.end(); ++iter )
    { clearTag( *iter, TextBlock::ALL_TAGS ); }

}

//___________________________________________________________________________
bool TextDisplay::_blockIsCollapsed( const QTextBlock& block ) const
{

    Debug::Throw( "TextDisplay::_blockIsCollapsed.\n" );
    QTextBlockFormat blockFormat( block.blockFormat() );
    return blockFormat.boolProperty( TextBlock::Collapsed ) && blockFormat.hasProperty( TextBlock::CollapsedData );

}


//___________________________________________________________________________
QString TextDisplay::_collapsedText( const QTextBlock& block ) const
{

    Debug::Throw( "TextDisplay::_collapsedText.\n" );
    QString text;

    // retrieve associated block format
    QTextBlockFormat blockFormat( block.blockFormat() );
    if( blockFormat.boolProperty( TextBlock::Collapsed ) && blockFormat.hasProperty( TextBlock::CollapsedData ) )
    {

        CollapsedBlockData collapsedData( blockFormat.property( TextBlock::CollapsedData ).value<CollapsedBlockData>() );
        foreach( const CollapsedBlockData& child, collapsedData.children() )
        { text += child.toPlainText(); }

    }

    return text;

}

//___________________________________________________________________________
bool TextDisplay::_fileIsAfs( void ) const
{ return file().indexOf( "/afs" ) == 0; }
