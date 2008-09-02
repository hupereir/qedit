#ifndef FileSystemIcons_h
#define FileSystemIcons_h

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

//! namespace for icon static name wrappers
namespace ICONS
{
  
  static const std::string INFO = "info.png";

  static const std::string DOCUMENT = "document.png"; 
  static const std::string FOLDER = "folder.png";
  static const std::string LINK = "link_overlay.png";

  static const std::string PARENT_DIRECTORY = "parent_directory.png";
  static const std::string PREVIOUS_DIRECTORY = "back.png";
  static const std::string NEXT_DIRECTORY = "forward.png";
  static const std::string HOME_DIRECTORY = "gohome.png";
  
};

#endif
