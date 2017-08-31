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
* You should have received a copy of the GNU General Public License alog with
* this program.  If not, see <http://www.gnu.org/licenses/>.
*
*******************************************************************************/

#include "TextDisplay.h"

#include "LineEditor.h"
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
#include "IconNames.h"
#include "InformationDialog.h"
#include "LineNumberDisplay.h"
#include "QuestionDialog.h"
#include "QtUtil.h"
#include "Singleton.h"
#include "TextBlockRange.h"
#include "TextEditorMarginWidget.h"
#include "TextEncodingDialog.h"
#include "TextEncodingWidget.h"
#include "TextEncodingMenu.h"
#include "TextHighlight.h"
#include "TextIndent.h"
#include "TextMacro.h"
#include "TextSeparator.h"
#include "Util.h"
#include "XmlOptions.h"

#if USE_ASPELL
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

#include <new>
#include <numeric>

//________________________________________________________
QByteArray safeUncompress( const QByteArray& content )
{
    try
    {

        return qUncompress( content );

    } catch( std::bad_alloc& exception ) {

        Debug::Throw() << "safeUncompress - caught bad_alloc exception: " << exception.what() << endl;
        return QByteArray();

    }

}

//___________________________________________________
NewDocumentNameServer& TextDisplay::newDocumentNameServer()
{
    static NewDocumentNameServer server;
    return server;
}

//___________________________________________________
QRegExp& TextDisplay::_emptyLineRegExp()
{
    static QRegExp regexp( "(^\\s*$)" );
    return regexp;
}

//___________________________________________________
TextDisplay::TextDisplay( QWidget* parent ):
    TextEditor( parent ),
    workingDirectory_( Util::workingDirectory() ),

    // store property ids associated to property names
    // this is used to speed-up fileRecord access
    classNamePropertyId_( FileRecord::PropertyId::get( FileRecordProperties::ClassName ) ),
    iconPropertyId_( FileRecord::PropertyId::get( FileRecordProperties::Icon ) ),
    wrapPropertyId_( FileRecord::PropertyId::get( FileRecordProperties::Wrapped ) ),
    dictionaryPropertyId_( FileRecord::PropertyId::get( FileRecordProperties::Dictionary ) ),
    filterPropertyId_( FileRecord::PropertyId::get( FileRecordProperties::Filter ) )
{

    Debug::Throw("TextDisplay::TextDisplay.\n" );

    // disable rich text
    setAcceptRichText( false );

    // text highlight
    textHighlight_ = new TextHighlight( document() );

    // parenthesis highlight
    parenthesisHighlight_ = new ParenthesisHighlight( this );

    // text indent
    textIndent_ = new TextIndent( this );

    // block delimiter
    blockDelimiterDisplay_ = new BlockDelimiterDisplay( this );
    connect( &textHighlight(), SIGNAL(needSegmentUpdate()), blockDelimiterDisplay_, SLOT(needUpdate()) );

    // connections
    connect( this, SIGNAL(selectionChanged()), SLOT(_selectionChanged()) );
    connect( this, SIGNAL(cursorPositionChanged()), SLOT(_highlightParenthesis()) );
    connect( this, SIGNAL(indent(QTextBlock,bool)), textIndent_, SLOT(indent(QTextBlock,bool)) );
    connect( this, SIGNAL(indent(QTextBlock,QTextBlock)), textIndent_, SLOT(indent(QTextBlock,QTextBlock)) );

    #if USE_ASPELL

    // install menus
    filterMenu_ = new SpellCheck::FilterMenu( this );
    dictionaryMenu_ = new SpellCheck::DictionaryMenu( this );

    #endif

    textEncodingMenu_ = new TextEncodingMenu( this );
    connect( textEncodingMenu_, SIGNAL(encodingChanged(QByteArray)), SLOT(_setTextEncoding(QByteArray)) );

    // actions
    _installActions();

    // connections
    // track contents changed for syntax highlighting
    connect( document(), SIGNAL(contentsChange(int,int,int)), SLOT(_setBlockModified(int,int,int)) );
    connect( document(), SIGNAL(modificationChanged(bool)), SLOT(_textModified()) );

    // track configuration modifications
    connect( Base::Singleton::get().application(), SIGNAL(configurationChanged()), SLOT(_updateConfiguration()) );
    connect( Base::Singleton::get().application(), SIGNAL(spellCheckConfigurationChanged()), SLOT(_updateSpellCheckConfiguration()) );
    connect( Base::Singleton::get().application(), SIGNAL(documentClassesChanged()), SLOT(updateDocumentClass()) );
    _updateConfiguration();
    _updateSpellCheckConfiguration();

    Debug::Throw( "TextDisplay::TextDisplay - done.\n" );

}

//_____________________________________________________
TextDisplay::~TextDisplay()
{

    Debug::Throw() << "TextDisplay::~TextDisplay - key: " << key() << endl;
    if( !( isNewDocument() || file_.isEmpty() ) && Base::KeySet<TextDisplay>( this ).empty() )
    { Base::Singleton::get().application<Application>()->fileCheck().removeFile( file_ ); }

}

//_____________________________________________________
int TextDisplay::blockCount( const QTextBlock& block ) const
{

    QTextBlockFormat blockFormat( block.blockFormat() );
    if( blockFormat.boolProperty( TextBlock::Collapsed ) && blockFormat.hasProperty( TextBlock::CollapsedData ) )
    { return blockFormat.property( TextBlock::CollapsedData ).value<CollapsedBlockData>().blockCount(); }
    else return TextEditor::blockCount( block );

}

//_____________________________________________________
void TextDisplay::setModified( bool value )
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
    if( isActive() && ( !file_.isEmpty() || isNewDocument() ) ) emit needUpdate( Modified );

}

//_____________________________________________________
void TextDisplay::setReadOnly( bool value )
{
    Debug::Throw() << "TextDisplay::setReadOnly - value: " << value << endl;

    bool changed = (value != isReadOnly() );
    TextEditor::setReadOnly( value );

    if( changed && isActive() ) emit needUpdate( ReadOnly );
}

//______________________________________________________________________________
void TextDisplay::installContextMenuActions( BaseContextMenu* menu, bool )
{

    Debug::Throw( "TextDisplay::installContextMenuActions.\n" );

    // see if tagged blocks are present
    const bool hasTags( hasTaggedBlocks() );
    const bool hasSelection( textCursor().hasSelection() );
    const bool currentBlockTagged( hasTags && isCurrentBlockTagged() );

    // retrieve default context menu
    // second argument is to remove un-necessary actions
    TextEditor::installContextMenuActions( menu, false );

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
    TextEditor::paintMargin( painter );
    bool hasBlockDelimiters( hasBlockDelimiterDisplay() && hasBlockDelimiterAction() && showBlockDelimiterAction_->isVisible() && showBlockDelimiterAction_->isChecked() );
    if( hasBlockDelimiters ) blockDelimiterDisplay_->paint( painter );
}

//___________________________________________________________________________
void TextDisplay::synchronize( TextDisplay* other )
{

    Debug::Throw( "TextDisplay::synchronize.\n" );

    // replace base class syntax highlighter prior to calling base class synchronization
    /* this prevents from calling to invalid block of memory which the textHighlight gets deleted
    when changing the document */
    textHighlight_ = &other->textHighlight();

    // base class synchronization
    TextEditor::synchronize( other );

    // restore connection with document
    // track contents changed for syntax highlighting
    connect( TextDisplay::document(), SIGNAL(contentsChange(int,int,int)), SLOT(_setBlockModified(int,int,int)) );
    connect( TextDisplay::document(), SIGNAL(modificationChanged(bool)), SLOT(_textModified()) );

    // indentation
    textIndent_->setPatterns( other->textIndent_->patterns() );
    textIndent_->setBaseIndentation( other->textIndent_->baseIndentation() );

    // parenthesis
    parenthesisHighlight_->synchronize( other->parenthesisHighlight() );

    // block delimiters and line numbers
    blockDelimiterDisplay_->synchronize( &other->blockDelimiterDisplay() );

    // actions
    textIndentAction_->setChecked( other->textIndentAction_->isChecked() );
    textHighlightAction_->setChecked( other->textHighlightAction_->isChecked() );
    parenthesisHighlightAction_->setChecked( other->parenthesisHighlightAction_->isChecked() );
    showLineNumberAction().setChecked( other->showLineNumberAction().isChecked() );
    showBlockDelimiterAction_->setChecked( other->showBlockDelimiterAction_->isChecked() );

    // macros
    _setMacros( other->macros() );

    // file
    _setIsNewDocument( other->isNewDocument() );
    _setFile( other->file_ );
    _setLastSaved( other->lastSaved_ );
    _setUseCompression( other->useCompression_ );

    // update class name
    setClassName( other->className() );
    setFileCheckData( other->fileCheckData_ );

    if( parentWidget() != other->parentWidget() )
    { emit needUpdate( ActiveDisplayChanged ); }

}

//____________________________________________
void TextDisplay::setIsNewDocument()
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
    Base::KeySet<TextDisplay> displays( this );
    displays.insert( this );
    for( const auto& display:displays )
    {

        display->_setIsNewDocument( true );
        display->setClassName( className() );
        display->_updateDocumentClass( File(), true );
        display->_updateSpellCheckConfiguration();
        display->_setFile( file );

        // disable file info action
        display->filePropertiesAction_->setEnabled( false );

    }
    Debug::Throw( "TextDisplay::setIsNewDocument - filename set.\n" );

    // perform first autosave
    auto application( Base::Singleton::get().application<Application>() );
    application->autoSave().saveFiles( this );
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
    file.expand();

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
        auto buffer = tr( "A more recent version of file '%1'\n"
            "was found at %2.\n"
            "This probably means that the application crashed the last time "
            "The file was edited.\n"
            "Use autosaved version ?" ).arg( file, autosaved );

        if( QuestionDialog( this, buffer ).exec() )
        {
            restoreAutoSave = true;
            tmp = autosaved;
        }
    }

    // remove new document version from name server
    if( isNewDocument() ) { NewDocumentNameServer().remove( file_ ); }

    // retrieve display and associated, update document class
    // this is needed to avoid highlight glitch when oppening file
    Base::KeySet<TextDisplay> displays( this );
    displays.insert( this );
    for( const auto& display:displays )
    {
        display->_setIsNewDocument( false );
        display->_setFile( file );
        display->filePropertiesAction_->setEnabled( true );

        display->setClassName( this->className() );
        display->_updateDocumentClass( file, false );
        display->_updateSpellCheckConfiguration( file );
    }

    Debug::Throw( "TextDisplay::setFile - updated displays.\n" );

    // check file and try open.
    QFile in( tmp );
    bool useCompression = false;
    if( in.open( QIODevice::ReadOnly ) )
    {

        // read content, try uncompress
        auto content( in.readAll() );
        auto uncompressed = safeUncompress( content );

        if( !uncompressed.isEmpty() )
        {
            auto buffer = tr( "File '%1' is compressed.\nUncompress before opening ?" ).arg( file );
            auto&& dialog = QuestionDialog( this, buffer );
            dialog.setOptionName( "COMPRESSION_DIALOG" );
            dialog.okButton().setText( tr( "Yes" ) );
            dialog.cancelButton().setText( tr( "No" ) );
            if( dialog.exec() )
            {
                useCompression = true;
                content = uncompressed;
            }
        }

        Debug::Throw( "TextDisplay::setFile - file read.\n" );

        // get encoding
        auto codec( QTextCodec::codecForName( textEncoding_ ) );
        setPlainText( codec->toUnicode(content) );
        in.close();

        // update flags
        setModified( false );
        _setIgnoreWarnings( false );

        Debug::Throw( "TextDisplay::setFile - content set.\n" );

    }

    // assign compression flag to all cloned displays
    for( const auto& display:displays )
    { display->_setUseCompression( useCompression ); }

    // save file if restored from autosaved.
    if( restoreAutoSave && !isReadOnly() ) save();

    // perform first autosave
    auto application( Base::Singleton::get().application<Application>() );
    application->autoSave().saveFiles( this );
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
        Base::Singleton::get().application<Application>()->fileCheck().addFile( file );

    }

    // check if file is read-only
    checkFileReadOnly();

    if( isActive() ) emit needUpdate( FileName );
    Debug::Throw( "TextDisplay::_setFile - done.\n" );

}

//___________________________________________________________________________
FileRemovedDialog::ReturnCode TextDisplay::checkFileRemoved()
{
    Debug::Throw() << "TextDisplay::checkFileRemoved - " << file_ << endl;

    // check if warnings are enabled and file is removed. Do nothing otherwise
    if( _ignoreWarnings() || !_fileRemoved() ) return FileRemovedDialog::Ignore;

    // disable check. This prevents recursion in macOS
    _setIgnoreWarnings( true );

    // ask action from dialog
    FileRemovedDialog dialog( this, file() );
    dialog.centerOnParent();
    auto state( dialog.exec() );

    // restore check
    _setIgnoreWarnings( false );

    // process dialog action
    switch( state )
    {

        case FileRemovedDialog::SaveAgain:
        // set document as modified (to force the file to be saved) and save
        setModified( true );
        save();
        break;

        case FileRemovedDialog::SaveAs:
        saveAs();
        break;

        case FileRemovedDialog::Ignore:
        case FileRemovedDialog::Close:
        {
            Base::KeySet<TextDisplay> displays( this );
            displays.insert( this );
            for( const auto& display:displays )
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
FileModifiedDialog::ReturnCode TextDisplay::checkFileModified()
{
    Debug::Throw() << "TextDisplay::checkFileModified - " << file_ << endl;

    // check if warnings are enabled and file is modified. Do nothing otherwise
    if( _ignoreWarnings() || !_fileModified() ) return FileModifiedDialog::Ignore;

    // disable check. This prevents recursion in macOS
    _setIgnoreWarnings( true );

    // ask action from dialog
    FileModifiedDialog dialog( this, file() );
    dialog.centerOnParent();
    auto state( dialog.exec() );

    // restore check
    _setIgnoreWarnings( false );

    // perform dialog action
    switch( state )
    {

        case FileModifiedDialog::SaveAgain:
        document()->setModified( true );
        save();
        break;

        case FileModifiedDialog::SaveAs:
        saveAs();
        break;

        case FileModifiedDialog::Reload:
        setModified( false );
        revertToSave();
        break;

        case FileModifiedDialog::Ignore:
        {
            Base::KeySet<TextDisplay> displays( this );
            displays.insert( this );
            for( const auto& display:displays )
            { display->_setIgnoreWarnings( true ); }
        }
        break;

        default: break;
    }

    return FileModifiedDialog::ReturnCode( state );

}

//___________________________________________________________________________
void TextDisplay::checkFileReadOnly()
{
    Debug::Throw( "TextDisplay::checkFileReadOnly.\n" );
    setReadOnly( file_.exists() && !file_.isWritable() );
}

//____________________________________________
void TextDisplay::clearFileCheckData()
{

    Debug::Throw( "TextDisplay::clearFileCheckData.\n" );

    if( fileCheckData_.flag() == FileCheck::Data::None ) return;

    // clear file check data
    Base::KeySet<TextDisplay> displays( this );
    displays.insert( this );
    for( const auto& display:displays )
    { display->setFileCheckData( FileCheck::Data() ); }

}

//___________________________________________________________________________
void TextDisplay::setFileCheckData( FileCheck::Data data )
{
    Debug::Throw( "TextDisplay::setFileCheckData.\n" );

    // check if data flag is different from stored
    bool flagsChanged( data.flag() != fileCheckData_.flag() );

    // update data
    fileCheckData_ = data;

    // emit file modification signal, to update session file frames
    if( flagsChanged ) emit needUpdate( Modified );

}

//___________________________________________________________________________
AskForSaveDialog::ReturnCode TextDisplay::askForSave( bool enableAll )
{
    Debug::Throw( "TextDisplay::askForSave.\n" );

    if( !( document()->isModified() && _contentsChanged() ) ) return AskForSaveDialog::Yes;

    AskForSaveDialog::ReturnCodes flags( AskForSaveDialog::Yes | AskForSaveDialog::No | AskForSaveDialog::Cancel );
    if( enableAll ) flags |=  AskForSaveDialog::YesToAll | AskForSaveDialog::NoToAll;

    AskForSaveDialog dialog( this, file_, flags );
    dialog.setWindowTitle( tr( "Save Files - Qedit" ) );
    dialog.centerOnParent();
    int state( dialog.exec() );
    if( state == AskForSaveDialog::Yes ||  state == AskForSaveDialog::YesToAll ) save();
    else if( state == AskForSaveDialog::No ||  state == AskForSaveDialog::NoToAll ) setModified( false );

    return AskForSaveDialog::ReturnCode(state);

}


//___________________________________________________________________________
void TextDisplay::save()
{
    Debug::Throw( "TextDisplay::save.\n" );

    // do nothing if not modified
    if( !document()->isModified() ) return;

    // check file name
    if( file_.isEmpty() || isNewDocument() ) return saveAs();

    // check is contents differ from saved file
    if( _contentsChanged() )
    {

        // make backup
        if( XmlOptions::get().get<bool>( "BACKUP" ) && file_.exists() ) file_.backup();

        // open output file
        QFile out( file_ );
        if( !out.open( QIODevice::WriteOnly ) )
        {
            InformationDialog( this, tr( "Cannot write to file '%1'. <Save> canceled." ).arg( file_ ) ).exec();
            return;
        }

        // process automatic macros
        if( !noAutomaticMacrosAction_->isChecked() && _hasAutomaticMacros() )
        {

            // first need to expand all collapsed blocks
            if( hasBlockDelimiterDisplay() )
            { blockDelimiterDisplay_->expandAllBlocks(); }

            // process macros
            for( const auto& macro:macros() )
            { if( macro.isAutomatic() ) _processMacro( macro ); }

        }

        // make sure that last line ends with "end of line"
        auto text( toPlainText() );
        if( !text.isEmpty() && text[text.size()-1] != '\n' ) text += '\n';

        QTextCodec* codec( QTextCodec::codecForName( textEncoding_ ) );
        auto content( codec->fromUnicode( text ) );
        if( useCompression_ ) content = qCompress( content );
        out.write( content );

        // close
        out.close();

    }

    // update modification state and last_saved time stamp
    setModified( false );
    _setLastSaved( file_.lastModified() );
    _setIgnoreWarnings( false );

    // re-add to file checker
    if( !file_.isEmpty() )
    { Base::Singleton::get().application<Application>()->fileCheck().addFile( file_ ); }


    // retrieve associated displays, update saved time
    for( const auto& display:Base::KeySet<TextDisplay>( this ) )
    { display->_setLastSaved( file_.lastModified() ); }

    // add file to menu
    if( !file_.isEmpty() )
    { _recentFiles().get( file_ ).addProperty( classNamePropertyId_, className() ); }

    return;

}

//___________________________________________________________________________
void TextDisplay::saveAs()
{
    Debug::Throw( "TextDisplay::saveAs.\n" );

    // define default file
    auto defaultFile( file_ );
    if( defaultFile.isEmpty() || isNewDocument() ) defaultFile = File( "Document" ).addPath( workingDirectory() );

    // create file dialog
    FileDialog dialog( this );
    dialog.setFileMode( QFileDialog::AnyFile );
    dialog.setAcceptMode( QFileDialog::AcceptSave );
    dialog.selectFile( defaultFile );
    File file( dialog.getFile() );
    if( file.isEmpty() ) return;
    else file.expand();

    // check if file is directory
    if( file.isDirectory() )
    {
        auto buffer = tr( "File '%1' is a directory. <Save> canceled." ).arg( file );
        InformationDialog( this, buffer ).exec();
        return;
    }

    // remove new document version from name server, and FileCheck, if needed
    if( isNewDocument() ) { NewDocumentNameServer().remove( file_ ); }
    else if( !file_.isEmpty() ) { Base::Singleton::get().application<Application>()->fileCheck().removeFile( file_ ); }

    // update filename and document class for this and associates
    // the class name is reset, to allow a document class
    // matching the new filename to get loaded
    setClassName( "" );

    Base::KeySet<TextDisplay> displays( this );
    displays.insert( this );
    for( const auto& display:displays )
    {

        // update file
        display->_setIsNewDocument( false );
        display->setClassName( className() );
        display->_updateDocumentClass( file, false );
        display->_setFile( file );

        // enable file info action
        display->filePropertiesAction_->setEnabled( true );

    }

    // set document as modified and save using new filename
    setModified( true );
    save();

    // rehighlight
    rehighlight();

}

//___________________________________________________________
void TextDisplay::revertToSave()
{

    Debug::Throw( "TextDisplay::revertToSave.\n" );

    // store scrollbar positions
    int x( horizontalScrollBar()->value() );
    int y( verticalScrollBar()->value() );

    // store cursor position but remove selection
    int position( textCursor().position() );

    setModified( false );
    setFile( file_, false );

    // restore
    horizontalScrollBar()->setValue( x );
    verticalScrollBar()->setValue( y );

    // adjust cursor postion
    position = std::min( position, toPlainText().size() );

    // restore cursor
    auto cursor( textCursor() );
    cursor.setPosition( position );
    setTextCursor( cursor );

}

//______________________________________________________________________________
void TextDisplay::_textEncoding()
{
    TextEncodingDialog dialog( this );
    dialog.setWindowTitle( tr( "TextEncoding - Qedit" ) );
    dialog.encodingWidget().setEncoding( textEncoding_ );
    if( dialog.exec() ) _setTextEncoding( dialog.encodingWidget().encoding() );
    return;
}

//______________________________________________________________________________
void TextDisplay::_setTextEncoding( const QByteArray& value )
{

    // make sure that modified
    if( value == textEncoding_ ) return;

    // check validity
    auto codec( QTextCodec::codecForName( value ) );
    if( !codec ) return;

    auto oldCodec( QTextCodec::codecForName( textEncoding_ ) );
    if( !oldCodec ) return;

    Debug::Throw() << "TextDisplay::_setTextEncoding - old codec: " << textEncoding_ << endl;
    Debug::Throw() << "TextDisplay::_setTextEncoding - new codec: " << value << endl;
    if( file_.isEmpty() || isNewDocument_ )
    {

        textEncoding_ = value;
        return;

    } else {

        // save if modified
        if( document()->isModified() )
        {

            auto buffer = tr(
                "Changing text encoding requires that the current document is reloaded.\n"
                "Discard changes to file '%1' ?" ).arg( file_.localName() );
            QuestionDialog dialog( this, buffer );
            dialog.setWindowTitle( tr( "Reload Document - Qedit" ) );
            dialog.centerOnParent();
            if( !dialog.exec() ) return;

        }

        // update
        textEncoding_ = value;

        // need to revert to save with new codec
        revertToSave();

    }

    return;

}

//_______________________________________________________
bool TextDisplay::hasLeadingTabs() const
{
    Debug::Throw( "TextDisplay::hasLeadingTabs.\n" );

    // define regexp to perform replacement
    QRegExp wrongTabRegExp( _hasTabEmulation() ? _normalTabRegExp():_emulatedTabRegExp() );
    const TextBlockRange range( document() );
    return std::any_of( range.begin(), range.end(),
        [&wrongTabRegExp](const QTextBlock& block)
        { return wrongTabRegExp.indexIn( block.text() ) >= 0; } );

}

//_______________________________________________________
QString TextDisplay::toPlainText() const
{

    Debug::Throw( "TextDisplay::toPlainText.\n" );

    // check blockDelimiterAction
    if( !( showBlockDelimiterAction_->isEnabled() && showBlockDelimiterAction_->isChecked() ) )
    { return TextEditor::toPlainText(); }

    // output string
    const TextBlockRange range( document() );
    return std::accumulate( range.begin(), range.end(), QString(),
        [this]( QString current, const QTextBlock& block )
        {
            current += block.text();
            if( block.next().isValid() || _blockIsCollapsed( block ) ) current += "\n";

            // add collapsed text
            current += _collapsedText( block );
            return current;
        }
    );
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
void TextDisplay::tagBlock( QTextBlock block, int tag )
{
    Debug::Throw( "TextDisplay::tagBlock.\n" );

    auto data( dynamic_cast<HighlightBlockData*>( block.userData() ) );
    if( !data ) block.setUserData( data = new HighlightBlockData );
    if( data->hasFlag( tag ) ) return;
    data->setFlag( tag, true );

    switch( tag )
    {
        case TextBlock::DiffAdded:
        setBackground( block, diffAddedColor_ );
        break;

        case TextBlock::DiffConflict:
        setBackground( block, diffConflictColor_ );
        break;

        case TextBlock::User:
        setBackground( block, userTagColor_ );
        break;

        default: break;

    }

    return;

}

//___________________________________________________________________________
void TextDisplay::clearTag( QTextBlock block, int tags )
{
    Debug::Throw() << "TextDisplay::clearTag - key: " << key() << endl;
    TextBlockData *data( static_cast<TextBlockData*>( block.userData() ) );
    if( !data ) return;

    if( tags & TextBlock::DiffAdded && data->hasFlag( TextBlock::DiffAdded ) )
    {
        data->setFlag( TextBlock::DiffAdded, false );
        clearBackground( block );
    }

    if( tags & TextBlock::DiffConflict && data->hasFlag( TextBlock::DiffConflict ) )
    {
        data->setFlag( TextBlock::DiffConflict, false );
        clearBackground( block );
    }

    if( tags & TextBlock::User && data->hasFlag( TextBlock::User ) )
    {
        data->setFlag( TextBlock::User, false );
        clearBackground( block );
    }

    Debug::Throw( "TextDisplay::clearTag - done.\n" );

}


//_____________________________________________________________
bool TextDisplay::isCurrentBlockTagged() const
{

    Debug::Throw( "TextDisplay::isCurrentBlockTagged.\n" );

    const auto cursor( textCursor() );
    const TextBlockRange range =
        cursor.hasSelection() ?
        TextBlockRange(
            document()->findBlock( std::min( cursor.position(), cursor.anchor() ) ),
            document()->findBlock( std::max( cursor.position(), cursor.anchor() ) ).next()):
        TextBlockRange( cursor.block(), cursor.block().next() );

    return std::any_of( range.begin(), range.end(),
        [] (const QTextBlock& block )
        {
            auto data( static_cast<TextBlockData*>( block.userData() ) );
            return data && data->hasFlag( TextBlock::DiffAdded | TextBlock::DiffConflict | TextBlock::User );
        });

}

//_____________________________________________________________
bool TextDisplay::hasTaggedBlocks() const
{

    Debug::Throw( "TextDisplay::hasTaggedBlocks.\n" );
    const TextBlockRange range( document() );
    return std::any_of( range.begin(), range.end(),
        []( const QTextBlock& block )
        {
            auto data( static_cast<TextBlockData*>( block.userData() ) );
            return( data && data->hasFlag( TextBlock::DiffAdded | TextBlock::DiffConflict | TextBlock::User ) );
        } );

    return false;
}

//___________________________________________________________________________
void TextDisplay::_updateDocumentClass( File file, bool newDocument )
{

    Debug::Throw( "TextDisplay::_updateDocumentClass\n" );

    textHighlight_->clear();
    textIndent_->clear();
    textIndent_->setBaseIndentation(0);
    _clearMacros();

    // default document class is empty
    DocumentClass documentClass;
    auto application( Base::Singleton::get().application<Application>() );

    // try load document class from className
    if( !className().isEmpty() )
    {
        Debug::Throw( "TextDisplay::updateDocumentClass - try use className().\n" );
        documentClass = application->classManager().get( className() );
    }

    // try load from file
    if( documentClass.name().isEmpty() && !( file.isEmpty() || newDocument ) )
    {
        Debug::Throw( "TextDisplay::updateDocumentClass - try use filename.\n" );
        documentClass = application->classManager().find( file );
    }

    // use default
    if( documentClass.name().isEmpty() )
    {
        Debug::Throw( "TextDisplay::updateDocumentClass - using default.\n" );
        documentClass = application->classManager().defaultClass();
    }

    // update class name
    setClassName( documentClass.name() );

    // wrap mode
    if( !( file.isEmpty() || newDocument ) )
    {

        FileRecord& record( _recentFiles().get( file ) );
        if( record.hasProperty( wrapPropertyId_ ) ) wrapModeAction().setChecked( record.property( wrapPropertyId_ ).toInt() );
        else if( XmlOptions::get().get<bool>( "WRAP_FROM_CLASS" ) ) wrapModeAction().setChecked( documentClass.wrap() );

    } else if( XmlOptions::get().get<bool>( "WRAP_FROM_CLASS" ) ) wrapModeAction().setChecked( documentClass.wrap() );

    // need to update tab size here because at the time it was set in _updateConfiguration,
    // the font might not have been right
    _setTabSize( XmlOptions::get().get<int>("TAB_SIZE") );

    // tab emulation
    if( XmlOptions::get().get<bool>( "EMULATE_TABS_FROM_CLASS" ) )
    {
        tabEmulationAction().setChecked( documentClass.emulateTabs() );
        if( documentClass.tabSize() > 0 ) _setTabSize( documentClass.tabSize() );
    }

    // enable actions consequently
    parenthesisHighlightAction_->setVisible( !documentClass.parenthesis().empty() );
    textHighlightAction_->setVisible( !documentClass.highlightPatterns().empty() );
    textIndentAction_->setVisible( !documentClass.indentPatterns().empty() );
    baseIndentAction_->setVisible( documentClass.baseIndentation() );

    // store into class members
    textHighlight_->setPatterns( documentClass.highlightPatterns() );
    textHighlight_->setParenthesis( documentClass.parenthesis() );
    textHighlight_->setBlockDelimiters( documentClass.blockDelimiters() );

    textIndent_->setPatterns( documentClass.indentPatterns() );
    textIndent_->setBaseIndentation( documentClass.baseIndentation() );
    _setMacros( documentClass.textMacros() );

    // update block delimiters
    if( blockDelimiterDisplay_->expandAllAction().isEnabled() ) blockDelimiterDisplay_->expandAllAction().trigger();
    if( blockDelimiterDisplay_->setBlockDelimiters( documentClass.blockDelimiters() ) ) update();
    showBlockDelimiterAction_->setVisible( !documentClass.blockDelimiters().empty() );
    _updateMargin();

    // update enability for parenthesis matching
    textHighlight_->setParenthesisEnabled(
        textHighlightAction_->isChecked() &&
        textHighlight_->parenthesisHighlightColor().isValid() &&
        !textHighlight_->parenthesis().empty() );

    parenthesisHighlight_->setEnabled(
        textHighlightAction_->isChecked() &&
        textHighlight_->parenthesisHighlightColor().isValid() &&
        !textHighlight_->parenthesis().empty() );

    // add information to Menu
    if( !( file.isEmpty() || newDocument ) )
    {
        auto& record( _recentFiles().get( file ) );
        record.addProperty( classNamePropertyId_, className() );
        record.addProperty( wrapPropertyId_, QString::number( wrapModeAction().isChecked() ) );
        if( !documentClass.icon().isEmpty() ) record.addProperty( iconPropertyId_, documentClass.icon() );
    }

    // rehighlight text entirely
    // because Pattern Ids may have changed even if the className has not changed.
    #if USE_ASPELL
    if( textHighlight_->isHighlightEnabled() && !textHighlight_->spellParser().isEnabled() ) rehighlight();
    #else
    if( textHighlight_->isHighlightEnabled() ) rehighlight();
    #endif

    // propagate
    emit needUpdate( DocumentClassFlag );

    return;

}

//_____________________________________________
void TextDisplay::processMacro( QString name )
{

    Debug::Throw() << "TextDisplay::processMacro - " << name << endl;

    // retrieve macro that match argument name
    auto macroIter = std::find_if( macros_.begin(), macros_.end(), TextMacro::SameNameFTor( name ) );
    if( macroIter == macros_.end() )
    {
        InformationDialog( this, tr( "Unable to find macro '%1'").arg( name ) ).exec();
        return;
    }

    _processMacro( *macroIter );

}

//_______________________________________________________
void TextDisplay::rehighlight()
{
    Debug::Throw( "TextDisplay::rehighlight.\n" );

    // set all block to modified
    for( const auto& block:TextBlockRange( document() ) )
    { _setBlockModified( block ); }

    textHighlight_->setDocument( document() );
    Debug::Throw( "TextDisplay::rehighlight. done.\n" );

}


//___________________________________________________________________________
void TextDisplay::clearAllTags( int flags )
{

    Debug::Throw( "TextDisplay::clearAllTags.\n" );

    setUpdatesEnabled( false );

    for( const auto& block:TextBlockRange( document() ) )
    { clearTag( block, flags ); }

    setUpdatesEnabled( true );

    // get associated displays and update all
    // this is needed due to the setUpdatesEnabled above
    Base::KeySet<TextDisplay> displays( this );
    displays.insert( this );
    for( const auto& display:displays )
    { display->viewport()->update(); }

}

//_______________________________________
void TextDisplay::selectFilter( const QString& filter )
{
    Debug::Throw( "TextDisplay::selectFilter.\n" );

    #if USE_ASPELL
    // local reference to interface
    SpellCheck::SpellInterface& interface( textHighlight_->spellParser().interface() );

    if( filter == interface.filter() || !interface.hasFilter( filter ) ) return;

    // update interface
    interface.setFilter( filter );
    filterMenu_->select( filter );

    // update file record
    if( !( file_.isEmpty() || isNewDocument() ) )
    { _recentFiles().get( file_ ).addProperty( filterPropertyId_, interface.filter() ); }

    // rehighlight if needed
    if( textHighlight_->spellParser().isEnabled() ) rehighlight();
    #else
    Q_UNUSED( filter )
    #endif

    return;

}

//_______________________________________
void TextDisplay::selectDictionary( const QString& dictionary )
{
    Debug::Throw( "TextDisplay::selectDictionary.\n" );

    #if USE_ASPELL
    // local reference to interface
    SpellCheck::SpellInterface& interface( textHighlight_->spellParser().interface() );

    if( dictionary == interface.dictionary() || !interface.hasDictionary( dictionary ) ) return;

    // update interface
    interface.setDictionary( dictionary );
    dictionaryMenu_->select( dictionary );

    // update file record
    if( !( file_.isEmpty() || isNewDocument() ) )
    { _recentFiles().get( file_ ).addProperty( dictionaryPropertyId_, interface.dictionary() ); }

    // rehighlight if needed
    if( textHighlight_->spellParser().isEnabled() ) rehighlight();
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
    Base::KeySet<TextDisplay> displays( this );
    displays.insert( this );
    for( const auto& display:displays )
    {
        display->setClassName( name );
        display->updateDocumentClass();
    }

    // rehighlight
    rehighlight();
}

//_______________________________________________________
void TextDisplay::setFocusDelayed()
{ QMetaObject::invokeMethod( this, "setFocus", Qt::QueuedConnection ); }

//_______________________________________________________
bool TextDisplay::event( QEvent* event )
{

    bool hasBlockDelimiters( hasBlockDelimiterDisplay() && hasBlockDelimiterAction() && showBlockDelimiterAction_->isVisible() && showBlockDelimiterAction_->isChecked() );

    // check that all needed widgets/actions are valid and checked.
    switch (event->type())
    {

        case QEvent::MouseButtonPress:
        if( hasBlockDelimiters )
        {
            blockDelimiterDisplay_->mousePressEvent( static_cast<QMouseEvent*>( event ) );
            event->accept();
            return true;
        }
        break;

        default: break;
    }

    return TextEditor::event( event );

}

//_______________________________________________________
void TextDisplay::keyPressEvent( QKeyEvent* event )
{

    // check if tab key is pressed
    if(
        event->key() == Qt::Key_Tab &&
        textIndent_->isEnabled() &&
        !( textCursor().hasSelection() || _boxSelection().state() == BoxSelection::State::Finished ) )
    { emit indent( textCursor().block(), false ); }
    else
    {

        // process key
        TextEditor::keyPressEvent( event );

        // indent current paragraph when return is pressed
        if( textIndent_->isEnabled() && event->key() == Qt::Key_Return && !textCursor().hasSelection() )
        { emit indent( textCursor().block(), true ); }

        // reindent paragraph if needed
        /* remark: this is c++ specific. The list of keys should be set in the document class */
        if( textIndent_->isEnabled() && ( event->key() == Qt::Key_BraceRight || event->key() == Qt::Key_BraceLeft ) && !textCursor().hasSelection() )
        { emit indent( textCursor().block(), false ); }

    }

    return;
}

//________________________________________________
void TextDisplay::contextMenuEvent( QContextMenuEvent* event )
{

    Debug::Throw( "TextEditor::contextMenuEvent.\n" );

    if( _autoSpellContextEvent( event ) ) return;
    else {

        TextEditor::contextMenuEvent( event );
        return;

    }

}

//________________________________________________
void TextDisplay::paintEvent( QPaintEvent* event )
{
    TextEditor::paintEvent( event );

    // create painter and translate from widget to viewport coordinates
    QPainter painter( viewport() );
    painter.setClipRect( event->rect() );
    painter.translate( -scrollbarPosition() );
    painter.setPen( _marginWidget().foregroundColor() );

    // loop over found blocks
    TextBlockRange range(
        cursorForPosition( event->rect().topLeft() ).block(),
        cursorForPosition( event->rect().bottomRight() ).block().next() );

    for( const auto& block:range )
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

    #if USE_ASPELL

    // check autospell enability
    if( !textHighlight_->spellParser().isEnabled() ) return false;

    // block and cursor
    QTextCursor cursor( cursorForPosition( event->pos() ) );
    QTextBlock block( cursor.block() );

    // block data
    HighlightBlockData* data( dynamic_cast<HighlightBlockData*>( block.userData() ) );
    if( !data ) return false;

    // try retrieve misspelled word
    SpellCheck::Word word( data->misspelledWord( cursor.position() - block.position() ) );
    if( word.isEmpty() || textHighlight_->spellParser().interface().isWordIgnored( word.get() ) )
    { return false; }

    // change selection to misspelled word
    cursor.setPosition( word.position() + block.position(), QTextCursor::MoveAnchor );
    cursor.setPosition( word.position() + word.length() + block.position(), QTextCursor::KeepAnchor );
    setTextCursor( cursor );

    // create suggestion menu
    SpellCheck::SuggestionMenu menu( this, word.get(), isReadOnly() );
    menu.interface().setFilter( textHighlight_->spellParser().interface().filter() );
    menu.interface().setDictionary( textHighlight_->spellParser().interface().dictionary() );

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
void TextDisplay::_installActions()
{

    Debug::Throw( "TextDisplay::_installActions.\n" );

    // actions
    addAction( textIndentAction_ = new QAction( IconEngine::get( IconNames::Indent ), "Indent Text", this ) );
    textIndentAction_->setCheckable( true );
    textIndentAction_->setChecked( textIndent_->isEnabled() );
    connect( textIndentAction_, SIGNAL(toggled(bool)), SLOT(_toggleTextIndent(bool)) );

    addAction( textHighlightAction_ = new QAction( "Highlight Text", this ) );
    textHighlightAction_->setCheckable( true );
    textHighlightAction_->setChecked( textHighlight_->isHighlightEnabled() );
    textHighlightAction_->setShortcut( Qt::Key_F8 );
    textHighlightAction_->setShortcutContext( Qt::WidgetShortcut );
    connect( textHighlightAction_, SIGNAL(toggled(bool)), SLOT(_toggleTextHighlight(bool)) );

    addAction( parenthesisHighlightAction_ = new QAction( "Highlight Parenthesis", this ) );
    parenthesisHighlightAction_->setCheckable( true );
    parenthesisHighlightAction_->setChecked( parenthesisHighlight_->isEnabled() );
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
    addAction( autoSpellAction_ = new QAction( IconEngine::get( IconNames::SpellCheck ), "Automatic Spell Checking", this ) );
    autoSpellAction_->setShortcut( Qt::Key_F6 );
    autoSpellAction_->setShortcutContext( Qt::WidgetShortcut );
    autoSpellAction_->setCheckable( true );

    #if USE_ASPELL
    autoSpellAction_->setChecked( textHighlight_->spellParser().isEnabled() );
    connect( autoSpellAction_, SIGNAL(toggled(bool)), SLOT(_toggleAutoSpell(bool)) );
    #else
    autoSpellAction_->setVisible( false );
    #endif

    // spell checking
    addAction( spellcheckAction_ = new QAction( IconEngine::get( IconNames::SpellCheck ), "Check Spelling...", this ) );
    #if USE_ASPELL
    connect( spellcheckAction_, SIGNAL(triggered()), SLOT(_spellcheck()) );
    #else
    spellcheckAction_->setVisible( false );
    #endif

    // indent selection
    addAction( indentSelectionAction_ = new QAction( IconEngine::get( IconNames::Indent ), "Indent Selection", this ) );
    indentSelectionAction_->setShortcut( Qt::CTRL + Qt::Key_I );
    indentSelectionAction_->setShortcutContext( Qt::WidgetShortcut );
    connect( indentSelectionAction_, SIGNAL(triggered()), SLOT(_indentSelection()) );

    // base indentation
    addAction( baseIndentAction_ = new QAction( IconEngine::get( IconNames::Indent ), "Add Base Indentation", this ) );
    baseIndentAction_->setShortcut( Qt::SHIFT + Qt::CTRL + Qt::Key_I );
    connect( baseIndentAction_, SIGNAL(triggered()), SLOT(_addBaseIndentation()) );

    // replace leading tabs
    addAction( leadingTabsAction_ = new QAction( "Replace leading tabs", this ) );
    connect( leadingTabsAction_, SIGNAL(triggered()), SLOT(_replaceLeadingTabs()) );

    // file information
    addAction( filePropertiesAction_ = new QAction( IconEngine::get( IconNames::Information ), "Properties...", this ) );
    filePropertiesAction_->setToolTip( "Display current file properties" );
    connect( filePropertiesAction_, SIGNAL(triggered()), SLOT(_fileProperties()) );

    #if USE_ASPELL

    filterMenuAction_ = filterMenu_->menuAction();
    dictionaryMenuAction_ = dictionaryMenu_->menuAction();

    connect( filterMenu_, SIGNAL(selectionChanged(QString)), SLOT(selectFilter(QString)) );
    connect( dictionaryMenu_, SIGNAL(selectionChanged(QString)), SLOT(selectDictionary(QString)) );

    #endif

    addAction( textEncodingAction_ = new QAction( "Text Encoding...", this ) );
    connect( textEncodingAction_, SIGNAL(triggered()), SLOT(_textEncoding()) );

    textEncodingMenuAction_ = textEncodingMenu_->menuAction();

    // tag block action
    addAction( tagBlockAction_ = new QAction( IconEngine::get( IconNames::Tag ), "Tag Selected Blocks", this ) );
    connect( tagBlockAction_, SIGNAL(triggered()), SLOT(_tagBlock()) );

    // clear current block tags
    addAction( clearTagAction_ = new QAction( "Clear Current Tag", this ) );
    connect( clearTagAction_, SIGNAL(triggered()), SLOT(_clearTag()) );

    // clear all tags
    addAction( clearAllTagsAction_ = new QAction( "Clear All Tags", this ) );
    connect( clearAllTagsAction_, SIGNAL(triggered()), SLOT(clearAllTags()) );

    // next tag action
    addAction( nextTagAction_ = new QAction( IconEngine::get( IconNames::Down ), "Goto Next Tagged Block", this ) );
    connect( nextTagAction_, SIGNAL(triggered()), SLOT(_nextTag()) );
    nextTagAction_->setShortcut( Qt::ALT + Qt::Key_Down );
    nextTagAction_->setShortcutContext( Qt::WidgetShortcut );

    // previous tag action
    addAction( previousTagAction_ = new QAction( IconEngine::get( IconNames::Up ), "Goto Previous Tagged Block", this ) );
    connect( previousTagAction_, SIGNAL(triggered()), SLOT(_previousTag()) );
    previousTagAction_->setShortcut( Qt::ALT + Qt::Key_Up );
    previousTagAction_->setShortcutContext( Qt::WidgetShortcut );

}

//_____________________________________________________________
FileList& TextDisplay::_recentFiles() const
{ return Base::Singleton::get().application<Application>()->recentFiles(); }

//_____________________________________________
bool TextDisplay::_hasAutomaticMacros() const
{ return std::any_of( macros().begin(), macros().end(), TextMacro::isAutomaticFTor() ); }

//_____________________________________________
void TextDisplay::_processMacro( const TextMacro& macro )
{

    // check display
    if( !isActive() ) return;

    // check if readonly
    if( isReadOnly() ) return;

    // retrieve text cursor
    auto cursor( textCursor() );
    int cursorPosition(0);
    int positionBegin(0);
    int positionEnd(0);
    QTextBlock begin;
    QTextBlock end;

    bool hasSelection( cursor.hasSelection() );
    if( hasSelection )
    {

        // retrieve blocks
        positionBegin = std::min( cursor.position(), cursor.anchor() );
        positionEnd = std::max( cursor.position(), cursor.anchor() );
        begin = document()->findBlock( positionBegin );
        end = document()->findBlock( positionEnd );

        // enlarge selection so that it matches begin and end of blocks
        positionBegin = begin.position();
        if( positionEnd == end.position() )
        {
            positionEnd--;
            end = end.previous();
        } else {  positionEnd = end.position() + end.length() - 1; }

        cursorPosition = positionEnd;

    } else {

        begin = document()->firstBlock();
        positionBegin = begin.position();

        end = document()->lastBlock();
        positionEnd = end.position() + end.length()-1;
        cursorPosition = cursor.position();

    }

    // prepare text from selected blocks
    QString text;
    if( begin == end ) {

        text = begin.text().mid( positionBegin - begin.position(), positionEnd-positionBegin );

    } else {

        text = begin.text().mid( positionBegin - begin.position() );
        if( begin.next().isValid() || _blockIsCollapsed( begin ) ) text += "\n";
        text += _collapsedText( begin );

        const TextBlockRange range( begin.next(), end );
        text += std::accumulate( range.begin(), range.end(), QString(),
            [this]( QString text, const QTextBlock& block )
            {
                text += block.text();
                if( block.next().isValid() || _blockIsCollapsed( block ) ) text += "\n";
                text += _collapsedText( block );
                return text;
            });

        // last block
        text += end.text().left( positionEnd - end.position() );

    }

    // process macro
    auto result( macro.processText( text, cursorPosition ) );
    if( !result.first ) return;

    Debug::Throw() << "TextDisplay::processText - increment: " << result.second << endl;

    // update selection
    cursor.setPosition( positionBegin );
    cursor.setPosition( positionEnd, QTextCursor::KeepAnchor );

    // insert new text
    cursor.insertText( text );

    // restore selection
    if( hasSelection )
    {

        cursor.setPosition( positionBegin );
        cursor.setPosition( positionBegin + text.size(), QTextCursor::KeepAnchor );

    } else {

        cursor.setPosition( cursorPosition + result.second );

    }

    setTextCursor( cursor );

    // replace leading tabs in selection
    _replaceLeadingTabs( false );

    return;

}

//_____________________________________________________________
bool TextDisplay::_contentsChanged() const
{

    Debug::Throw( "TextDisplay::_contentsChanged.\n" );

    // check file
    if( file_.isEmpty() || isNewDocument() ) return true;

    // open file
    QFile in( file_ );
    if( !in.open( QIODevice::ReadOnly ) ) return true;

    // read file content
    auto fileContent( in.readAll() );
    if( useCompression_ ) fileContent = qUncompress( fileContent );

    // apply codec
    QTextCodec* codec( QTextCodec::codecForName( textEncoding_ ) );
    auto fileText = codec->toUnicode(fileContent);
    auto text( toPlainText() );

    return (text.size() != fileText.size() || text != fileText );

}

//____________________________________________
bool TextDisplay::_fileRemoved() const
{
    Debug::Throw() << "TextDisplay::_fileRemoved - " << file_ << endl;

    // check new document
    if( file_.isEmpty() || isNewDocument() ) return false;
    if( !lastSaved().isValid() ) return false;

    /*
    for "afs" files, one should do the check anyway, disregarding
    fileCheckData, because the latter is not triggered properly
    when modifying the file remotely
    */

    // check file flag
    if( !( _fileIsAfs() || fileCheckData_.flag() == FileCheck::Data::Removed || fileCheckData_.flag() == FileCheck::Data::Modified ) )
    { return false; }

    // make sure file is still removed
    if( !file_.exists() ) return true;
    else {

        // file has been re-created in the meantime.
        // need to re-ad it to FileChecker
        Base::Singleton::get().application<Application>()->fileCheck().addFile( file_ );
        return false;

    }

}

//____________________________________________
bool TextDisplay::_fileModified()
{

    Debug::Throw() << "TextDisplay::_fileModified - " << file_ << endl;

    // check file
    if( file_.isEmpty() || isNewDocument() ) return false;
    if( !( _fileIsAfs() || fileCheckData_.flag() == FileCheck::Data::Removed || fileCheckData_.flag() == FileCheck::Data::Modified ) )
    { return false; }

    if( !lastSaved().isValid() ) return false;

    // when file is on afs, or when file was removed (and recreated)
    // one need to use the filename modification timeStampl in place of the timeStamp contained in fileCheckData
    // because the last one was invalid
    const TimeStamp fileModified( _fileIsAfs() ? TimeStamp(file_.lastModified()) : fileCheckData_.timeStamp() );
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
    if( !textHighlight_->isHighlightEnabled() ) return;

    // retrieve associated block data if any
    // set block as modified so that its highlight content gets reprocessed.
    TextBlockData* data( static_cast<TextBlockData*>( block.userData() ) );
    if( data ) data->setFlag( TextBlock::BlockModified, true );

}

//_____________________________________________________________
void TextDisplay::_updateTaggedBlocks()
{

    Debug::Throw( "TextDisplay::_updateTaggedBlocks.\n" );

    // loop over block
    for( const auto& block:TextBlockRange( document() ) )
    {
        auto data( static_cast<TextBlockData*>( block.userData() ) );
        if( !( data && data->hasFlag( TextBlock::DiffAdded | TextBlock::DiffConflict | TextBlock::User ) ) ) continue;

        if( data->hasFlag( TextBlock::DiffAdded ) ) setBackground( block, diffAddedColor_ );
        if( data->hasFlag( TextBlock::DiffConflict ) ) setBackground( block, diffConflictColor_ );
        if( data->hasFlag( TextBlock::User ) ) setBackground( block, userTagColor_ );
    }

}

//___________________________________________________________________________
bool TextDisplay::_updateMargin()
{
    Debug::Throw( "TextDisplay::_updateMargin.\n" );

    TextEditor::_updateMargin();
    int left_margin( _leftMargin() );

    blockDelimiterDisplay_->setOffset( left_margin );
    if( showBlockDelimiterAction_->isChecked() && showBlockDelimiterAction_->isVisible() )
    { left_margin += blockDelimiterDisplay_->width(); }

    return _setLeftMargin( left_margin );

}

//___________________________________________________________________________
bool TextDisplay::_toggleWrapMode( bool state )
{

    Debug::Throw() << "TextDisplay::_toggleWrapMode - " << (state ? "True":"False") << endl;
    if( !TextEditor::_toggleWrapMode( state ) ) return false;

    if( !( file_.isEmpty() || isNewDocument() ) )
    { _recentFiles().get( file_ ).addProperty( wrapPropertyId_, QString::number(state) ); }

    return true;

}

//___________________________________________________________________________
void TextDisplay::_updateConfiguration()
{
    Debug::Throw( "TextDisplay::_updateConfiguration.\n" );

    // indentation
    textIndentAction_->setChecked( XmlOptions::get().get<bool>( "TEXT_INDENT" ) );

    // syntax highlighting
    textHighlightAction_->setChecked( XmlOptions::get().get<bool>( "TEXT_HIGHLIGHT" ) );

    // parenthesis highlight
    textHighlight_->setParenthesisHighlightColor( XmlOptions::get().get<Base::Color>( "PARENTHESIS_COLOR" ) );
    parenthesisHighlightAction_->setChecked( XmlOptions::get().get<bool>( "TEXT_PARENTHESIS" ) );

    // block delimiters, line numbers and margin
    showBlockDelimiterAction_->setChecked( XmlOptions::get().get<bool>( "SHOW_BLOCK_DELIMITERS" ) );
    noAutomaticMacrosAction_->setChecked( XmlOptions::get().get<bool>( "IGNORE_AUTOMATIC_MACROS" ) );

    {
        // font
        QFont font;
        font.fromString( XmlOptions::get().raw( "FIXED_FONT_NAME" ) );
        int line_spacing = QFontMetrics( font ).lineSpacing() + 1;
        blockDelimiterDisplay_->setWidth( line_spacing );
        _updateMargin();
    }

    // encoding
    // todo: condition that on whether was modified by menu or not
    _setTextEncoding( XmlOptions::get().raw( "TEXT_ENCODING" ) );
    textEncodingMenu_->select( textEncoding_ );

    // retrieve diff colors
    diffConflictColor_ = XmlOptions::get().get<Base::Color>( "DIFF_CONFLICT_COLOR" );
    diffAddedColor_ = XmlOptions::get().get<Base::Color>( "DIFF_ADDED_COLOR" );
    userTagColor_ = XmlOptions::get().get<Base::Color>( "TAGGED_BLOCK_COLOR" );

    // update paragraph tags
    _updateTaggedBlocks();

}

//___________________________________________________________________________
void TextDisplay::_updateSpellCheckConfiguration( File file )
{

    Debug::Throw() << "TextDisplay::_updateSpellCheckConfiguration - file: " << file << " new document:" << isNewDocument() << endl;

    #if USE_ASPELL

    // spellcheck configuration
    bool changed( false );
    changed |= textHighlight_->spellParser().setColor( QColor( XmlOptions::get().get<Base::Color>("AUTOSPELL_COLOR") ) );
    changed |= textHighlight_->spellParser().setFontFormat( (Format::TextFormatFlags) XmlOptions::get().get<int>("AUTOSPELL_FONT_FORMAT") );
    textHighlight_->updateSpellPattern();
    autoSpellAction_->setEnabled( textHighlight_->spellParser().color().isValid() );

    // reset filter and dictionaries menu
    dictionaryMenu_->reset();
    filterMenu_->reset();

    // store local reference to spell interface
    SpellCheck::SpellInterface& interface( textHighlight_->spellParser().interface() );

    // load default filter and dictionaries
    QString filter( XmlOptions::get().raw("DICTIONARY_FILTER") );
    QString dictionary( XmlOptions::get().raw( "DICTIONARY" ) );

    // overwrite with file record
    if( file.isEmpty() ) file = file_;
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
    if( changed && autoSpellAction_->isChecked() && autoSpellAction_->isEnabled() )
    { rehighlight(); }

    #else
    Q_UNUSED( file )
    #endif

}

//_______________________________________________________
void TextDisplay::_indentCurrentParagraph()
{
    Debug::Throw( "TextDisplay::_indentCurrentParagraph.\n" );
    if( !textIndent_->isEnabled() ) return;
    emit indent( textCursor().block(), false );
}

//_______________________________________________________
void TextDisplay::_toggleTextIndent( bool state )
{

    Debug::Throw( "TextDisplay::_toggleTextIndent.\n" );

    // update text indent
    textIndent_->setEnabled( textIndentAction_->isEnabled() && state );

    // propagate to other displays
    if( isSynchronized() )
    {
        // temporarely disable synchronization
        // to avoid infinite loop
        setSynchronized( false );

        for( const auto& display:Base::KeySet<TextDisplay>( this ) )
        { if( display->isSynchronized() ) display->textIndentAction_->setChecked( state ); }
        setSynchronized( true );

    }

}


//_______________________________________________________
void TextDisplay::_toggleTextHighlight( bool state )
{

    Debug::Throw( "TextDisplay::_toggleTextHighlight.\n" );
    if( textHighlight_->setHighlightEnabled( textHighlightAction_->isEnabled() && state ) )
    { rehighlight(); }

    // propagate to other displays
    if( isSynchronized() )
    {
        // temporarely disable synchronization
        // to avoid infinite loop
        setSynchronized( false );

        for( const auto& display:Base::KeySet<TextDisplay>( this ) )
        { if( display->isSynchronized() ) display->textHighlightAction_->setChecked( state ); }

        setSynchronized( true );

    }

}

//_______________________________________________________
void TextDisplay::_toggleParenthesisHighlight( bool state )
{

    Debug::Throw() << "TextDisplay::_toggleParenthesisHighlight -"
        << " state: " << state
        << " color: " << textHighlight_->parenthesisHighlightColor().isValid()
        << " parenthesis: " << textHighlight_->parenthesis().empty()
        << endl;

    // propagate to textHighlight
    textHighlight_->setParenthesisEnabled(
        state &&
        textHighlight_->parenthesisHighlightColor().isValid() &&
        !textHighlight_->parenthesis().empty() );

    parenthesisHighlight_->setEnabled(
        state &&
        textHighlight_->parenthesisHighlightColor().isValid() &&
        !textHighlight_->parenthesis().empty() );

    // propagate to other displays
    if( isSynchronized() )
    {
        // temporarely disable synchronization
        // to avoid infinite loop
        setSynchronized( false );

        for( const auto& display:Base::KeySet<TextDisplay>( this ) )
        { if( display->isSynchronized() ) display->parenthesisHighlightAction_->setChecked( state ); }
        setSynchronized( true );

    }

    return;
}

//_______________________________________________________
void TextDisplay::_toggleAutoSpell( bool state )
{
    #if USE_ASPELL
    Debug::Throw( "TextDisplay::_toggleAutoSpell.\n" );

    // propagate to textHighlight
    textHighlight_->spellParser().setEnabled( state );
    rehighlight();

    // propagate to other displays
    if( isSynchronized() )
    {
        // temporarely disable synchronization
        // to avoid infinite loop
        setSynchronized( false );

        for( const auto& display:Base::KeySet<TextDisplay>( this ) )
        { if( display->isSynchronized() ) display->autoSpellAction_->setChecked( state ); }
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
    if( !state && hasBlockDelimiterDisplay() && blockDelimiterDisplay_->expandAllAction().isEnabled() )
    { blockDelimiterDisplay_->expandAllAction().trigger(); }

    // update options
    XmlOptions::get().set<bool>( "SHOW_BLOCK_DELIMITERS", state );

    // propagate to other displays
    if( isSynchronized() )
    {

        // temporarely disable synchronization
        // to avoid infinite loop
        setSynchronized( false );

        for( const auto& display:Base::KeySet<TextDisplay>( this ) )
        { if( display->isSynchronized() ) display->showBlockDelimiterAction_->setChecked( state ); }
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
void TextDisplay::_spellcheck()
{
    Debug::Throw( "TextDisplay::_spellcheck.\n" );

    #if USE_ASPELL
    // create dialog
    SpellCheck::SpellDialog dialog( this );
    dialog.interface().setIgnoredWords( textHighlight_->spellParser().interface().ignoredWords() );

    // default dictionary from XmlOptions
    QString defaultFilter( XmlOptions::get().raw("DICTIONARY_FILTER") );
    QString defaultDictionary( XmlOptions::get().raw( "DICTIONARY" ) );

    // try overwrite with file record
    if( !( file_.isEmpty()  || isNewDocument() ) )
    {

        FileRecord& record( _recentFiles().get( file_ ) );
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
    if( !( file_.isEmpty() || isNewDocument() ) )
    {
        _recentFiles().get( file_ )
            .addProperty( filterPropertyId_, dialog.filter() )
            .addProperty( dictionaryPropertyId_, dialog.dictionary() );
    }

    textHighlight_->spellParser().interface().mergeIgnoredWords( dialog.interface().ignoredWords() );

    #endif

}

//_______________________________________________________
void TextDisplay::_indentSelection()
{
    Debug::Throw( "TextDisplay::_indentSelection.\n" );

    // check activity, indentation and text selection
    if( !textIndent_->isEnabled() ) return;

    // retrieve text cursor
    auto cursor( textCursor() );
    if( !cursor.hasSelection() ) return;

    // retrieve blocks
    QTextBlock begin( document()->findBlock( std::min( cursor.position(), cursor.anchor() ) ) );
    QTextBlock end( document()->findBlock( std::max( cursor.position(), cursor.anchor() ) ) );

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
void TextDisplay::_addBaseIndentation()
{
    Debug::Throw( "TextDisplay::_addBaseIndentation.\n" );

    // check activity, indentation and text selection
    if( !textIndent_->baseIndentation() ) return;

    // define regexp to perform replacement
    QRegExp leadingSpaceRegExp( "^\\s*" );
    QString replacement( textIndent_->baseIndentation(), ' ' );

    // retrieve cursor
    // retrieve text cursor
    auto cursor( textCursor() );
    if( !cursor.hasSelection() ) return;

    // store blocks
    const auto begin = document()->findBlock( std::min( cursor.position(), cursor.anchor() ) );
    const auto end = document()->findBlock( std::max( cursor.position(), cursor.anchor() ) );
    for( const auto& block:TextBlockRange(begin, end.next() ) )
    {
        // check block
        if( !block.isValid() ) continue;

        // retrieve text
        QString text( block.text() );

        // look for leading tabs
        if( leadingSpaceRegExp.indexIn( text ) < 0 ) continue;

        // select with cursor
        QTextCursor cursor( block );
        cursor.movePosition( QTextCursor::StartOfBlock, QTextCursor::MoveAnchor );
        cursor.setPosition( cursor.position() + leadingSpaceRegExp.matchedLength(), QTextCursor::KeepAnchor );
        cursor.insertText( replacement );
    }

    // indent
    emit indent( begin, end );

    // enable updates
    setUpdatesEnabled( true );

    return;
}

//_______________________________________________________
void TextDisplay::_replaceLeadingTabs( bool confirm )
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
    QRegExp wrongTabRegExp( _hasTabEmulation() ? _normalTabRegExp():_emulatedTabRegExp() );
    QString wrongTab( _hasTabEmulation() ? normalTabCharacter():emulatedTabCharacter() );

    // define blocks to process
    QTextBlock begin;
    QTextBlock end;

    auto cursor( textCursor() );
    const TextBlockRange range = cursor.hasSelection() ?
        TextBlockRange(
        document()->findBlock( std::min( cursor.position(), cursor.anchor() ) ),
        document()->findBlock( std::max( cursor.position(), cursor.anchor() ) ).next() ):
        TextBlockRange( document() );

    // loop over blocks
    for( const auto& block:range )
    {
        // check block
        if( !block.isValid() ) continue;

        // retrieve text
        QString text( block.text() );

        // look for leading tabs
        if( wrongTabRegExp.indexIn( text ) < 0 ) continue;

        // select with cursor
        QTextCursor cursor( block );
        cursor.movePosition( QTextCursor::StartOfBlock, QTextCursor::MoveAnchor );
        cursor.setPosition( cursor.position() + wrongTabRegExp.matchedLength(), QTextCursor::KeepAnchor );

        // create replacement string and insert.
        QString buffer;
        for( int i=0; i< int(wrongTabRegExp.matchedLength()/wrongTab.size()); i++ )
        { buffer += tabCharacter(); }
        cursor.insertText( buffer );

    }

    // enable updates
    setUpdatesEnabled( true );

    Debug::Throw( "TextDisplay::_replaceLeadingTabs - done.\n" );
    return;
}


//_______________________________________________________
void TextDisplay::_fileProperties()
{
    Debug::Throw( "TextDisplay::_fileProperties.\n" );
    if( file_.isEmpty() || isNewDocument() ) return;

    // prior to showing the dialog
    // one should add needed tab for misc information
    auto record(  _recentFiles().get( file_ ) );
    FileInformationDialog dialog( this, record );

    // add additional informations frame
    auto box( new QWidget );
    auto layout = new QVBoxLayout;
    layout->setMargin(5);
    layout->setSpacing( 5 );
    box->setLayout( layout );
    Debug::Throw( "TextDisplay::_fileProperties - Miscellaneous tab booked.\n" );

    auto gridLayout = new GridLayout;
    gridLayout->setMargin(0);
    gridLayout->setSpacing( 5 );
    gridLayout->setMaxCount( 2 );
    gridLayout->setColumnAlignment( 0, Qt::AlignRight|Qt::AlignVCenter );
    layout->addLayout( gridLayout );

    // number of characters
    auto item = new GridLayoutItem( box, gridLayout );
    item->setKey( "Number of characters:" );
    item->setText( QString::number(toPlainText().size()) );

    // number of lines
    item = new GridLayoutItem( box, gridLayout );
    item->setKey( "Number of lines:" );
    item->setText( QString::number( TextEditor::blockCount()) );

    gridLayout->addWidget( new QLabel( "Text highlighting:", box ) );
    {
        auto checkbox( new QCheckBox( box ) );
        checkbox->setChecked( textHighlight_->isHighlightEnabled() );
        checkbox->setEnabled( false );
        gridLayout->addWidget( checkbox );
    }

    gridLayout->addWidget( new QLabel( "Paragraph highlighting:", box ) );
    {
        auto checkbox( new QCheckBox( box ) );
        checkbox->setChecked( blockHighlightAction().isChecked() );
        checkbox->setEnabled( false );
        gridLayout->addWidget( checkbox );
    }

    gridLayout->addWidget( new QLabel( "Parenthesis highlighting:", box ) );
    {
        auto checkbox( new QCheckBox( box ) );
        checkbox->setChecked( textHighlight_->isParenthesisEnabled() );
        checkbox->setEnabled( false );
        gridLayout->addWidget( checkbox );
    }

    gridLayout->addWidget( new QLabel( "Text indentation:", box ) );
    {
        auto checkbox( new QCheckBox( box ) );
        checkbox->setChecked(  textIndent_->isEnabled() );
        checkbox->setEnabled( false );
        gridLayout->addWidget( checkbox );
    }

    gridLayout->addWidget( new QLabel( "Text wrapping:", box ) );
    {
        auto checkbox( new QCheckBox( box ) );
        checkbox->setChecked( wrapModeAction().isChecked() );
        checkbox->setEnabled( false );
        gridLayout->addWidget( checkbox );
    }

    gridLayout->addWidget( new QLabel( "Tab emulation:", box ) );
    {
        auto checkbox( new QCheckBox( box ) );
        checkbox->setChecked( tabEmulationAction().isChecked() );
        checkbox->setEnabled( false );
        gridLayout->addWidget( checkbox );
    }

    // document class
    item = new GridLayoutItem( box, gridLayout, GridLayoutItem::Flag::Elide );
    item->setKey( "Document class file name:" );
    auto documentClass( Base::Singleton::get().application<Application>()->classManager().get( className() ) );
    item->setText( documentClass.file() );

    // also assign icon to dialog
    dialog.setIcon( IconEngine::get( documentClass.icon() ) );

    // autosave
    item = new GridLayoutItem( box, gridLayout, GridLayoutItem::Flag::Elide|GridLayoutItem::Flag::Selectable );
    item->setKey( "Auto-save file name:" );
    item->setText( AutoSaveThread::autoSaveName( file_ ) );

    layout->addStretch();

    dialog.tabWidget().addTab( box, "Information" );

    // execute dialog
    dialog.centerOnParent();
    dialog.exec();

}

//_____________________________________________________________
void TextDisplay::_setBlockModified( int position, int, int added )
{
    for( const auto& block:TextBlockRange(
        document()->findBlock( position ),
        document()->findBlock( position + added ).next() ) )
    { _setBlockModified( block ); }
}

//__________________________________________________
void TextDisplay::_textModified()
{
    Debug::Throw( "TextDisplay::_textModified.\n" );

    // document should never appear modified
    // for readonly displays
    if( document()->isModified() && isReadOnly() ) document()->setModified( false );
    if( isActive() && ( !file_.isEmpty() || isNewDocument() ) ) emit needUpdate( Modified );

}

//__________________________________________________
void TextDisplay::_ignoreMisspelledWord( QString word )
{
    Debug::Throw() << "TextDisplay::_ignoreMisspelledWord - word: " << word << endl;
    #if USE_ASPELL
    textHighlight_->spellParser().interface().ignoreWord( word );
    rehighlight();
    #endif
    return;

}

//__________________________________________________
void TextDisplay::_replaceMisspelledSelection( QString word )
{

    #if USE_ASPELL
    Debug::Throw() << "TextDisplay::_replaceMisspelledSelection - word: " << word << endl;
    auto cursor( textCursor() );
    cursor.insertText( word );
    #else
    Q_UNUSED( word )
    #endif
    return;

}

//__________________________________________________
void TextDisplay::_highlightParenthesis()
{

    if( !( hasTextHighlight() && textHighlight_->isParenthesisEnabled() ) ) return;

    // clear previous parenthesis
    {
        auto dirty( parenthesisHighlight_->clear() );
        for( const auto& block:dirty )
        { textHighlight_->rehighlightBlock( block ); }
    }

    // retrieve TextCursor
    auto cursor( textCursor() );
    if( cursor.atBlockStart() ) return;

    // retrieve block
    auto block( cursor.block() );

    // store local position in block
    int position(cursor.position()-block.position());

    // get current PatternLocation set
    PatternLocationSet locations;
    auto data = dynamic_cast<HighlightBlockData*>( block.userData() );
    if( data ) locations = data->locations();

    auto text( block.text() );
    const auto& parenthesis( textHighlight_->parenthesis() );

    // check against opening parenthesis
    bool found( false );
    auto iter( std::find_if(
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
            if( data && data->hasFlag( TextBlock::BlockCollapsed ) )
            {

                const auto blockFormat( block.blockFormat() );
                if( !blockFormat.hasProperty( TextBlock::CollapsedData ) )
                {
                    Debug::Throw(0) << "TextDisplay::_highlightParenthesis - missing CollapsedData property" << endl;
                    block = block.next();
                    continue;
                }

                CollapsedBlockData collapsedBlockData( blockFormat.property( TextBlock::CollapsedData ).value<CollapsedBlockData>() );
                text += collapsedBlockData.toPlainText();

            }

            // parse text
            for( ;(position = iter->regexp().indexIn( text, position ) ) >= 0; position += iter->regexp().matchedLength() )
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
            auto text( block.text() );

            // append collapsed data if any
            if( data && data->hasFlag( TextBlock::BlockCollapsed ) )
            {

                const auto blockFormat( block.blockFormat() );
                if( !blockFormat.hasProperty( TextBlock::CollapsedData ) )
                {
                    Debug::Throw(0) << "TextDisplay::_highlightParenthesis - missing CollapsedData property" << endl;
                    block = block.next();
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
        parenthesisHighlight_->highlight( position + block.position(), iter->regexp().matchedLength() );
        textHighlight_->rehighlightBlock( block );
    }

    return;

}

//__________________________________________________
void TextDisplay::_tagBlock()
{

    Debug::Throw( "TextDisplay::_tagBlock.\n" );

    const auto cursor( textCursor() );
    const TextBlockRange range = cursor.hasSelection() ?
        TextBlockRange(
            document()->findBlock( std::min( cursor.position(), cursor.anchor() ) ),
            document()->findBlock( std::max( cursor.position(), cursor.anchor() ) ).next() ):
        TextBlockRange( cursor.block(), cursor.block().next() );

    // clear background for selected blocks
    for( const auto& block:range )
    { tagBlock( block, TextBlock::User ); }

}

//__________________________________________________
void TextDisplay::_nextTag()
{
    Debug::Throw( "TextDisplay::_nextTag.\n" );
    auto cursor( textCursor() );
    QTextBlock block( cursor.block() );
    TextBlockData* data;

    // first skipp blocks that have tags if the first one has
    for(;
        block.isValid() &&
        (data = static_cast<TextBlockData*>( block.userData() ) ) &&
        data->hasFlag( TextBlock::All ); block = block.next() )
    {}

    // skip blocks with no tag
    for(;
        block.isValid() &&
        !((data = static_cast<TextBlockData*>( block.userData() ) ) &&
        data->hasFlag( TextBlock::All ) ); block = block.next())
    {}

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
void TextDisplay::_previousTag()
{
    Debug::Throw( "TextDisplay::_previousTag.\n" );
    auto cursor( textCursor() );
    QTextBlock block( cursor.block() );
    TextBlockData* data;

    // first skip blocks that have tags if the first one has
    for(;
        block.isValid() &&
        (data = static_cast<TextBlockData*>( block.userData() ) ) &&
        data->hasFlag( TextBlock::All ); block = block.previous() )
    {}

    // skip blocks with no tag
    for(;
        block.isValid() &&
        !((data = static_cast<TextBlockData*>( block.userData() ) ) &&
        data->hasFlag( TextBlock::All ) ); block = block.previous())
    {}

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
void TextDisplay::_clearTag()
{

    Debug::Throw( "TextEditor::_clearTag.\n" );

    auto cursor( textCursor() );
    if( cursor.hasSelection() )
    {

        for( const auto& block:TextBlockRange(
            document()->findBlock( std::min( cursor.position(), cursor.anchor() ) ),
            document()->findBlock( std::max( cursor.position(), cursor.anchor() ) ).next() ) )
        { clearTag( block, TextBlock::All ); }

    } else {

        // add previous blocks and current
        for( auto block( cursor.block() ); block.isValid(); block = block.previous() )
        {
            auto data( static_cast<TextBlockData*>( block.userData() ) );
            if( data && data->hasFlag( TextBlock::DiffAdded | TextBlock::DiffConflict | TextBlock::User ) )
            {

                clearTag( block, TextBlock::All );

            } else break;
        }


        // add previous blocks and current
        for( auto block( cursor.block().next() ); block.isValid(); block = block.next() )
        {
            auto data( static_cast<TextBlockData*>( block.userData() ) );
            if( data && data->hasFlag( TextBlock::DiffAdded | TextBlock::DiffConflict | TextBlock::User ) )
            {

                clearTag( block, TextBlock::All );

            } else break;
        }

    }

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

    // retrieve associated block format
    const auto blockFormat( block.blockFormat() );
    if( blockFormat.boolProperty( TextBlock::Collapsed ) && blockFormat.hasProperty( TextBlock::CollapsedData ) )
    {

        const auto collapsedData( blockFormat.property( TextBlock::CollapsedData ).value<CollapsedBlockData>() );
        const auto children( collapsedData.children() );
        return std::accumulate( children.begin(), children.end(), QString(),
            []( const QString& text, const CollapsedBlockData& child )
            { return text + child.toPlainText(); } );

    } else return QString();

}

//___________________________________________________________________________
bool TextDisplay::_fileIsAfs() const
{ return file_.get().indexOf( "/afs" ) == 0; }
