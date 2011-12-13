#ifndef HelpText_h
#define HelpText_h

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
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License along with
* software; if not, write to the Free Software Foundation, Inc., 59 Temple
* Place, Suite 330, Boston, MA  02111-1307 USA
*
*
*******************************************************************************/

/*!
\file HelpText.h
\brief QEdit Help text
\author Hugo Pereira
\version $Revision$
\date $Date$
*/

//! help text
static const QString helpText[] = {
    //_________________________________________________________
    "Introduction",
    "<h2>Introduction</h2><p>QEdit is a Graphical text editor designed for text files and programming source files. it provides standard menus, dialogs, toolbars and shortcut to perform all actions related to text editing and offers contextual support for easier editing of various programming languages files, such as: <ul><li>C/C++;</li><li>Makefiles and shell scripts;</li><li>Perl scripts;</li><li>HTML and XML;</li><li>diff files.</li></ul><p>Each of the above is considered as a <i>Document Class</i> and associated to a set of syntax highlighting patterns, indentation rules, text edition macros, and parenthesis matching highlighting.<p> QEdit is highly customizable to fit a wide range of editing habits. Most of its features have been ported from existing text editors, such as <i>Emacs</i> or <i>NEdit</i>, although not all of the features of the previous two editors can be found. Some features have been originally designed for QEdit that cannot be easily found in the above two editors, such as: <ul><li> an original interface to <i>aspell</i> for spell checking, either using a dialog or via syntax highlighting of the misspelled words;</li>"
    "<li> an independent thread to perform periodic backups of the edited files in the <i>/tmp</i> directory rather than in the working directory </li>"
    "<li> an independent thread to list and check the validity of previously opened files.</li>"
    "<li> an editable help system, that allows any user to add his own experience with the editor.</li>"
    "<li> vertically, horizontally or combined multiple view system that allow more efficient comparison between sections of files. Multiple views allow to display either duplicated (and synchronized) copy of the same file (as in NEdit), or different files, as in Emacs.</li>"
    "</ul><p>QEdit is based on Qt and is nicely integrated in a <i>KDE</i> or <i>Gnome</i> environment by adopting the same look and feel appearance as the current desktop.",

    //_________________________________________________________
    "Main Window",
    "<h2>Text Edition Main Window</h2><p>This section describes the most useful features of the main edition window. More details are given elsewhere in the dedicated sections of this help.<h3>Cut, Copy and Paste</h3><p>Cut, Copy and Paste actions are available either from the toolbar, the <i>Edit</i> menu, the pop-up menu opened when right-clicking on the text, or the Ctrl+X Ctrl+C and Ctrl+V key accelerators, respectively. The Cut and Copy actions are enabled only if there is an active selection in the current editor. The paste action is enabled only if some text is available from the clipboard.</p><h3>Undo and Redo</h3><o>Undo and Redo actions are available either from the toolbar, the <i>Edit</i> menu, the pop-up menu opened when right-clicking on the text, or the Ctrl+Z and Shift+Ctrl+Z key accelerators, respectively. The undo/redo lists are kept synchronized between multiple views of the same files. The actions above are enabled only when undoing/redoing is available in the history.</p><h3>Automatic indentation</h3><p>Automatic indentation is available for document classes associated to some file types, such as c/c++ and Perl script files. It is configurable in the corresponding document classes input file loaded at start-up.  When enabled, automatic indentation rules are applied on the current line by pressing the Tab character. It is also performed when the return key is pressed, to start a new line. Automatic indentation can be disabled either in the <i>Default Configuration</i> dialog, in which case it will be disabled for all the following edition sessions, or in the <i>Settings</i> menu, in which case it is disabled for the current session only.</p><h3>Text selection</h3><p>Normal selection starts when the left button of the mouse is pressed. It is modified (extended or reduced) by moving the mouse pointer with the left button still pressed. The selection ends and is copied to the selection clipboard when the button is released. A drag and drop system allows to move the selected text either in the same window or in a separated file.</p><p> Box selection is available provided that the font used for editing has a fixed width. Box selection starts when the left button of the mouse is pressed together with the Ctrl key, and modified  by moving the mouse pointer with the above still pressed. The selection ends and is copied to the selection clipboard when either the left mouse button or the Control key is released. The cut, copy and paste actions, as well as the drag and drop ability are also available for box selection. </p><h3>Automatic save</h3><p>Automatic save of the file is performed at fixed interval of time when the text is modified. It is handled by a separate thread in order not to create any overhead. The auto saved files are located by default in the /tmp directory and recovered automatically in case the editing session ends abruptly.</p><h3>Automatic spell checking</h3><p>Automatic spell checking can be turned on in the <i>Settings</i> menu. It replaces (temporarily) any syntax highlighting patterns and identifies all words that are misspelled in the current text. Correctly spelled words can be selected and used as a replacement when right-clicking on a misspelled word. The dictionary language and document  class associated to the spell-checker is modified in the <i>Settings</i> menu. Selected dictionaries and filter are stored together with the opened file history. When no values are found, default values are used, that can be modified in the <i>Default Configuration</i> dialog.</p><h3>Printing</h3><p>Printing is achieved via built-in Qt's printing support. Syntax highlighted patterns are also rendered to the output document.</p>",

    //_________________________________________________________
    "Main Menu",
    "<h2>Main Menu</h2><p>The Main Menu is located at the top of the editor. It contains the following sub-menus:<h3>File menu</h3><p>The File menu provides basic file operations, such as New/Open/Save/Print and  the corresponding key accelerators, as well as the management of multiple views in the current window. Pressing the Close button closes the current edition window. Pressing the Exit button closes all edition windows and exit QEdit. The following two features are also available:</p><ul><li>the possibility to open an old file, based on an historic of all opened files. The size of the historic is set in the <i>Default Configuration</i> dialog. Only the most recently opened files are kept. Previously opened files that cannot be found on disk appear in grey in the corresponding sub-menu. They will be discarded from the historic at the next session. They can also be manually removed by pressing the Clean button at the top of the sub-menu.</li>"
    "<li>the possibility to manually assign the document class of the current file by selecting the class name in the corresponding sub-menu, in place of the default document class assigned based on either the file extension or its contents.</li>"
    "</ul><h3>Edit menu</h3><p>the Edit menu provides basic edition operations, such as Undo/Redo, Cut/Copy and Paste of the current selection and conversion to upper/lower case, as well as the corresponding key accelerators.</p><h3>Search menu</h3><p>the Search menu provides text find/replace operations and the possibility to select a paragraph by its line number in the text, as well as the corresponding key accelerators.</p><h3>Settings menu</h3><p>The settings menu contains QEdit <i>Default Configuration</i> dialog and the <i>Spell Check Configuration</i> dialog that defines which language and file type is used for spell checking (this feature is available only if QEdit was compiled with <i>aspell</i> support). Several check boxes are also provided to control which features of the current document class should be enabled/disabled in the current session.</p><h3>Tools menu</h3><p>The Tools menu provides high-level text modification macros and edition utilities that are common to all document classes. Such macros apply to large portion of the text and perform actions such as text indentation, spell checking (via a pop-up dialog), diff of opened files, etc.</p><h3>Macro menu</h3><li>The Macro menu contains high level edition macros associated to the current document class. Such macros usually apply to large portion of the text and perform actions such as applying comments (as specified by the document class language), adding/removing tabulations, cleaning the end of line from empty spaces, etc. The Spell check button opens the spell checking dialog used to correct all misspelled words in the document (this feature is available only if QEdit was compiled with <i>aspell</i> support). Some of the macros are associated to key accelerators depending on the current document class configuration file;</p><h3>Window menu</h3><p>The Window menu provides information about the active  edit file and fast access to all other edited files in the session.</p><h3>Help menu</h3><p>the Help menu provides access to this help, displays information on the version of QEdit that is being used, and the version of <i>Qt</i> against which it was compiled. It also has a <i>debug</i> sub-menu used for maintenance.</p>",

    //_________________________________________________________
    "Toolbars",
    "<h2>Toolbars</h2><p>Several toolbars are located (by default) at the top of the editor window to provide easy access to the most commonly used features. They are: <ul><li>the <i>File</i> toolbar, used to create a new file, edit an existing file, save an edited file, and print the current file (either to postscript or to a printer). The same actions are available in the <i>File</i> menu.</li>"
    "<li>the <i>Edition</i> toolbar, used to cut, copy or paste the current text selection, and undo/redo the last actions. The same actions are available in the <i>Edit</i> menu.</li><li>the <i>Tools</i> toolbar, used to open the spell checking dialog and display information on the currently edited file.</li>"
    "<li>the <i>Multiple views</i> toolbar, used to clone the current file in the separate view, open a new file in a separate view, close the current view, or detach the current view to make it an independent window. The same actions are available in the <i>File</i> menu. </li></ul><p>By right-clicking on the toolbars a menu is opened that allows to show/hide the displayed toolbars. The toolbars can also be dragged and dropped around the editor window to change their location. The default visibility and location of the toolbars can be set in the <i>configuration</i> dialog, under the <i>toolbars</i> tab.",

    //_________________________________________________________
    "Status bar",
    "<h2>Status Bar</h2><p>The status bar is located at the bottom of the editor window and displays the full name of the current edited file, the position of the cursor in the text, as well as the current date and time. The file name can be selected by double clicking on it and copied to the clipboard for use in other applications.</p>",

    //_________________________________________________________
    "Default Configuration",
    "<h2>Default Configuration</h2><p>The default configuration dialog is accessible from the <i>Settings</i> menu. It consists of several tabs in which a large set of options can be modified by the user to suit his editing habits.<h3>Base</h3><p>the Base tab allows to modify the font used to display the widgets and the text; the application icon, the location where the toolbar icons are found (since the name of the toolbar icons match the one used for most of the desktop icon themes, this option allows to pick a set of icons that match the current desktop icon theme. Besides, it allows to try find additional icons that are foreseen in QEdit, but not included in the distribution); the appearance of the toolbar buttons, and the background color of the items displayed in lists.</p><h3>Document classes</h3><p>the Document classes tab allows to specify from which files the document classes used to define the set of actions associated to a specific file type are loaded. Such files are written in XML format and can be edited by hand. New files can be added and loaded in place of the default. Future versions of QEdit should have a Document class manager that allows for easier editing of these files.</p><h3>Display</h3><p>The Display tab allows which features of the document classes should be enabled by default. This includes the syntax highlighting, the matching parenthesis highlighting, the automatic indentation, tab emulation, etc. It also allows to configure some of the colors used for the highlighting.</p><h3>Multiple views</h3><p>The Multiple views tag allows to configure the default open mode of QEdit (either in a new window or as a new view in an existing window, as well as the default orientation (horizontal or vertical) of new views in a window. The corresponding buttons in the <i>Multiple views</i> toolbar allows to change this default behavior for the current session only.</p><p>A different color can be used as a background for inactive views as opposed to the active view, to make the later more easily identifiable. This color is also configurable here.</p><h3>Toolbars</h3><p>The Toolbars tab allows to decide which toolbars should be visible/hidden by default in every new window, as well as the side of the window at which they should appear</p><h3>Misc</h3><p>the Misc (miscelaneous) tab allows to modify additional settings that don't enter the above categories such as: the backup and auto-save configurations, the default window size and the sorting of the previously opened files in the <i>Open previous</i> menu.</p></ul>",

    //_________________________________________________________
    "Document Classes",
    "<h2>Document Classes</h2>\n"
    "<p>Document classes consist of a set of recognition patterns associated to a different type of files and used to facilitate its editing. The different patterns cover: \n"
    "<ul>\n"
    "<li>syntax highlighting patterns;</li>\n"
    "<li>indentation rules;</li>\n"
    "<li>text edition macros;</li>\n"
    "<li>matching parenthesis highlighting</li>\n"
    "<li>and logical blocks markers.</li>\n"
    "</ul>\n"
    "\n"
    "<p>The following type of files are supported by default:\n"
    "<ul>\n"
    "<li>C/C++;</li><li>Makefiles and shell scripts;</li>\n"
    "<li>Perl scripts;</li><\n"
    "li>HTML and XML;</li\n"
    "><li>diff files.</li>\n"
    "</ul>\n"
    "\n"
    "<p>Each document class is defined in an external XML file loaded at start-up and associated to the file being edited. By default, the association between a file and its document class is based on the file extension or the contents of its first lines. This association can be changed manually via the <i>Set document class</i> sub-menu in the <i>File</i> menu. The list of files that are loaded at start-up is configurable in the <i>Default configuration</i> dialog accessible from the <i>Settings</i> menu. ",

    //_________________________________________________________
    "Logical block markers",
    "<h2>Logical block markers</h2>\n"
    "\n"
    "<p>For some document classes, blocks of text are well separated in logical structures. For instance, <i>c/c++</i> logical structures are delimited by curly brackets, while <i>LaTeX</i> structures are delimited by <i>begin</i> and <i>end</i> tags.</p>\n"
    "\n"
    "<p>QEdit offers the possibility to identify these structures and represents them as vertical segments located at the left of the edition window. Such segments are called <i>block delimiters</i>. When available, these delimiters can be made visible by checking the corresponding option in the <i>Settings</i> menu, or in the <i>Default configuration</i> dialog.</p>\n"
    "\n"
    "<p>When visible, logical structures can be collapsed or expanded at will by clicking on the marker corresponding to the beginning of each block delimiter, or by selecting the corresponding action in the <tools> menu.</p>\n"
    "\n"
    "<p>When a logical block is collapsed, only its first line is displayed in the editor window, and an horizontal line is drawn to indicate that some text is hidden at this location. The first line is still editable. However, when one deletes this line, the full logical block gets erased.</p>\n"
    "\n"
    "\n"
    "<p>In the current implementation, the action of expanding or collapsing one or several logical blocks is included in the undo/redo history. This allows notably to easily recover from inconsiderate removal of collapsed blocks. As a side effect, such actions mark the file as <i>modified</i> even if its contents is actually unchanged. It is unfortunate, but harmless.</p> \n"
    "",

    //_________________________________________________________
    "Spell Check Configuration",
    "<h2>Spell Check Configuration</h2><p>The spell check configuration dialog is accessible from the <i>Settings</i> menu. It is divided in two boxes. The first box is dedicated to the general configuration of the spell checker and contains the following options:<ul><li>the <i>dictionary</i> which defines the language used for spell checking</li>"
    "<li>the <i>filter</i> which defines the type of file that is being checked. For each file type, specific keywords (such as commands for a LaTeX file) are discarded from the spell checking;</li>"
    "<li>the <i>aspell</i></i> command (as typed from the command line in any terminal), which is used to retrieve the default list of dictionaries and filters.</li>"
    "</ul><p>The second box is dedicated to the automatic spell checking configuration and contains the following options: <ul><li>the <i>maximum number of suggestions</i> displayed in the pop-up menu opened by right clicking on a misspelled word when automatic spell checking is enabled;</li>"
    "<li>the <i>font color</i> and <i>format</i> (bold, underline, italic, ...) used to highlight misspelled words when automatic spell checking is enabled;</li>"
    "</ul><p>Modifications to the settings available in the spell check configuration dialog apply to the current edition session and all later sessions. The filter and dictionary used for the current file can also be modified in the <i>Settings</i>menu directly. The modifications performed to these menus apply to the current file only.</p><p>The spell checking of the current document is performed either using the interactive dialog accessible from the <i>Macro</i> menu, or by selecting the automatic spell check button in the <i>Settings</i> menu. </p>",

    //_________________________________________________________
    "Automatic Spell Check",
    "<h2>Automatic Spell Check</h2><p>The automatic spell check is turned on/off in the <i>Settings</i> menu. when enabled, the current syntax highlighting is turned off and misspelled words are (by default) shown in red and underlined. Right clicking on a misspelled word opens a menu where a list of correctly spelled suggestions is displayed that can be used to replace the current word. Alternatively, the misspelled word can be <i>ignored</i>, meaning that it will not appear red anymore, as long as this edition session is active, or <i>added to the dictionary</i>, meaning that it will be considered correctly spelled for this session and all the following (as well as for other applications using aspell). <p>The dictionary used by the automatic spell check, the syntax highlighting pattern used for misspelled words as well as the document type (used to define a set of words to be ignored, such as commands in a LaTeX file), are selected in the <i>Settings</i> menu.",

    //_________________________________________________________
    "Printing",
    "<h2>Printing</h2><p>The print dialog is opened using the print button in either the <i>toolbar</i> or the <i>File</i> menu. Printing is achieved using built-in Qt's printing support. Syntax highlighting patterns are also rendered to the output document.</p>",

    //_________________________________________________________
    "Tab Emulation",
    "<h2>Tab Emulation</h2><p>Tab emulation is used to replace the standard tab character by a fixed amount of space characters. This is useful for portability of the edited document with respect to other applications, such as Emacs. By default tab emulation is on. Tab emulation is enabled/disabled in the <i>Default configuration</i> window. The number of space characters used as a replacement when emulation is on is also selected there. <p>The automatic text indentation rules generally depend on the number of leading tab characters found in the previous paragraph. It uses the current tab emulation setup to determine the number of tab characters. So that indentation is only accurate for a document that uses the same convention for tab characters as the current setup of QEdit. In case of inconsistency, one can either <ul><li> change the editor setting to match the edited file;</li><li> or use the <i>replace in window</i> command with the correct selection and replacement strings to modify the edited text so that it matches the current editor configuration</li></ul><p>Additionally, a text macro called <i>Replace leading tabs</i> is found in the <i>tools</i> menu, which replaces all <i>true</i> leading tab characters with the current emulated tabs.",

    //_________________________________________________________
    "Regular Expressions",
    "<h2>Regular Expressions</h2><p>Regular expressions provide an efficient way of looking for complicated patterns in a text, using a compact syntax which is interpreted by the Qt built-in regular expression engine. Regular expressions are used extensively to define the syntax highlighting patterns, indentation rules and matching parenthesis in the document class configuration files. In general QEdit users do not need to know about the syntax used by regular expressions unless they wish to modify the document classes configuration files or create a new one. <p>Regular expressions can also be used in the editor <i>find</i> and <i>replace</i> dialogs to find text patterns more sophisticated than simple words. <p>The syntax used for regular expression is documented elsewhere on the web and notably in the Qt on-line documentation page. ",

    //_________________________________________________________
    "Multiple Views",
    "<h2>Multiple Views</h2><p>Multiple views have two usage:</p><ul><li> they allow to display different sections of the same file in a single editor window.<li> they allow to display different files in the same window.</li></ul><p>Multiple views of the <i>same</i> file are opened either via the <i>clone</i> button in the <i>File</i> menu, in which case the layout (vertical or horizontal) of the new view is controlled by the default orientation option from the <i>default configuration</i> dialog, or directly via the <i>Multiple views</i> toolbar. </p><p>When multiple views are opened, the text appearing in the two view is automatically synchronized. Modifications performed to one of the view is immediately ported to the second. Matching parenthesis highlighting is also common to both views, which allows to check for closing parenthesis after large sections of text by looking for the matching parenthesis in the other window.<p>Multiple views of <i>different</i> files are opened either from the usual <i>open</i>, <i>open previous</i> and <i>new</i> buttons in the <i>File</i> menu, provided that the <i>default open mode</i> in the <i>default configuration</i> dialog is set to <i>open in new view</i>, or via the <i>Multiple views</i> toolbar. When two different files are opened in the same window, they can be diffed (using the command-line <i>diff</i> command) to highlight conflict or added text blocks.",

    //_________________________________________________________
    "Automatic Backup",
    "<h2>Automatic Backup</h2><p>To prevent loss of information and enforce file recovery whenever the application is terminated abruptly, an automatic save of the files is performed on a regular basis in the <i>/tmp</i> directory. <p>The backup file name is constructed based on the original file name and the user editing it. For instance, the file <p><i>/home/hpereira/QEdit/src/HelpText.h</i> <p>edited by user <i>hpereira</i> is backed up at<p><i>/tmp/hpereira_home_hpereira_src_QEdit_HelpText.h</i>. <p>The file can be later recovered either by hand or using QEdit directly, since QEdit always check at start-up if there a backup file with a later modification time than the one of the file being opened. <p>The default backup time interval is 20 seconds and can be modified in the default configuration dialog. The default directory where the backup files are saved can also be modified from there.<p>Backup are performed only when the file has actually been modified. They are handled in a separate thread so that they don't create any overload.",

    //_________________________________________________________
    "Mouse Buttons",
    "<h2>Mouse Buttons</h2><p>By default the mouse buttons are associated to the following actions:<ul><li>Pressing the <i>left button</i> gives focus to a display, places the cursor at the mouse position, and clears any selection in the display. Moving the mouse while the left button is pressed selects the text following the cursor and copies it to the <i>selection</i> clipboard. The selection ends when the button is released. A double click with the left button selects the word pointed to by the mouse. A triple click selects the entire paragraph. A quadruple click selects the whole text.</li>"
    "<li>Pressing the <i>middle button</i> places the cursor at the mouse position and insert a copy of the currently selected text or the <i>selection</i> clipboard, if any. Moving the mouse with the Middle button pressed moves the cursor accordingly. The selection is copied to the display when the button is released, at the place where the cursor is.</li>"
    "<li>Pressing the <i>right button</i> opens a menu to perform basic edition actions, such as undo/redo, cut/copy/paste, etc. Moving the mouse with the right button pressed does nothing.</li>"
    "</ul><p>The action of the buttons may change when Meta keys, such as the Control key or the Alt key are pressed simultaneously. Currently, only one such modification is available: <ul><li> pressing <i>left Button + Ctrl</i> starts a box selection from the place where the mouse pointer is. The selection is extended by moving the mouse pointer with both buttons pressed and it ends when one of the two buttons is released. The box selection is then copied to the <i>selection</i> clipboard and can be inserted anywhere else in the text</li></ul>",

    //_________________________________________________________
    "Accelerators",
    "<h2>Accelerators</h2><p>Key accelerators provide shortcuts to the most commonly used actions by pressing a predefined sequence of keys. They are case insensitive.<p>The following accelerators are available,<ul><li><i>Ctrl+N</i> create a new file;</li><li><i>Ctrl+O</i> open an existing file;</li><li><i>Ctrl+W</i> close current edition window;</li><li><i>Ctrl+S</i> save current file;</li><li><i>Ctrl+P</i> Print current document;</li><li><i>Ctrl+Q</i> close all edition windows and exit QEdit;</li><li><i>Ctrl+Z</i> undo last action;</li><li><i>Shift+Ctrl+Z</i> redo last action;</li><li><i>Ctrl+X</i> cut current selection;</li><li><i>Ctrl+C</i> copy current selection;</li><li><i>Ctrl+V</i> paste clipboard at the current cursor location;</li><li><i>Ctrl+A</i> select all text;</li><li><i>Ctrl+U</i> convert current selection to upper-case;</li><li><i>Shift+Ctrl+U</i> convert current selection to lower-case;</li><li><i>Ctrl+F</i> raise the text find dialog;</li><li><i>Ctrl+G</i> find the next occurrence of the searched string in the forward direction;</li><li><i>Shift+Ctrl+G</i> find the next occurrence of the searched string in the backward direction;</li><li><i>Ctrl+H</i> find the next occurrence of the current selection in the forward direction;</li><li><i>Shift+Ctrl+H</i> find the next occurrence of the current selection in the backward direction;</li><li><i>Ctrl+R</i> raise the <i>replace</i> dialog;</li><li><i>Ctrl+T</i> find the next occurrence of the searched string in the forward direction and replace with the current replacement text;</li><li><i>Shift+Ctrl+T</i> find the next occurrence of the searched string in the backward direction and replace with the current replacement text;</li><li><i>Ctrl+L</i> select a paragraph by its line number;</li></ul><p>Additional key accelerators are provided with the text edition macros provided by the current document class and are set in the corresponding document class configuration file.",

    //_________________________________________________________
    "Command line",
    "<h2>Command Line</h2>"
    "<p>QEdit usage is the following: </p>"
    "<p>qedit [options] [file1] [file2] [...]</p>"
    "<p>With [options] being one or several of the following:</p>"
    "<ul>"
    "<li><i>--help</i> displays this help and exit</li>"
    "<li><i>--tabbed</i> opens files in same window</li>"
    "<li><i>--diff</i> opens files in same window and perform diff</li>"
    "<li><i>--autospell</i> switch autospell on for all files</li>"
    "<li><i>--filter \"filter name\"</i> select filter for autospell</li>"
    "<li><i>--dictionary \"dictionary name\"</i> select dictionary for autospell</li>"
    "<li><i>--close</i> close displays matching file names and exit</li>"
    "<li><i>--replace</i> replace existing instance with new one.</li>"
    "<li><i>--no-server</i> ignore server mode. runs new application instance.</li>"
    "<li><i>--abort<i > exit existing instance.</li>"
    "</ul>"
    "<p>and [file1] [file2] [...] the list of files to be opened. </p>"
    "<p><b>Remarks:</b></p>"
    "<ul>"
    "<li>When no files are specified, a blank window is opened. "
    "<li>Unless the server mode is disabled, opening a file that was already oppened during the same session simply raises the corresponding window.</li>"
    "</ul>",

    0
};


#endif
