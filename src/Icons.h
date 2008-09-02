#ifndef Icons_h
#define Icons_h

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

/*!
  \file    Icons.h
  \brief   Icon filenames
  \author  Hugo Pereira
  \version $Revision$
  \date    $Date$
*/

#include <string>
#include "BaseIcons.h"
#include "FileSystemIcons.h"

//! namespace for icon static name wrappers
namespace ICONS
{
  
  static const std::string CLOSE = "fileclose.png"; 
  
  static const std::string DOCUMENTS = "documents.png"; 
  static const std::string HISTORY = "history.png"; 
  static const std::string FILESYSTEM = "user-home.png"; 
  
  static const std::string NEW = "filenew.png"; 
  static const std::string SAVE = "filesave.png";
  static const std::string SAVE_AS = "filesaveas.png"; 
  static const std::string SAVE_ALL = "save_all.png"; 
  static const std::string HTML="html.png";
  static const std::string PDF = "pdf.png";
  static const std::string INDENT="indent.png";
  static const std::string PRINT = "fileprint.png"; 
  static const std::string SPELLCHECK = "fonts.png";
  static const std::string VIEW_LEFTRIGHT = "view_left_right.png";
  static const std::string VIEW_TOPBOTTOM = "view_top_bottom.png";
  static const std::string VIEW_RIGHT = "view_right.png";
  static const std::string VIEW_BOTTOM = "view_bottom.png";
  static const std::string VIEW_REMOVE = "view_remove.png";
  static const std::string VIEW_DETACH = "window_fullscreen.png";
  
  static const std::string TAG = "bookmark.png";
  static const std::string UP = "up.png";
  static const std::string DOWN = "down.png";
    
};

#endif
