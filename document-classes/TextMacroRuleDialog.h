#ifndef _TextMacroRuleDialog_h_
#define _TextMacroRuleDialog_h_

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
  \file TextMacroRuleDialog.h
  \brief Syntax highlighting delimiter editing dialog
  \author Hugo Pereira
  \version $Revision$
  \date $Date$
*/

#include <QCheckBox>

#include "CustomDialog.h"
#include "TextMacro.h"

class AnimatedLineEditor;

//! Syntax highlighting delimiter editing dialog
class TextMacroRuleDialog: public CustomDialog
{

  public:

  //! constructor
  TextMacroRuleDialog( QWidget* parent );

  //! delimiter
  void setRule( const TextMacro::Rule& );

  //! delimiter
  TextMacro::Rule rule( void );

  private:

  //! regexp editor
  AnimatedLineEditor* regexp_editor_;

  //! end editor
  AnimatedLineEditor* text_editor_;

  //! rexexp editor
  QCheckBox* split_checkbox_;

};

#endif
