#ifndef TextMacro_h
#define TextMacro_h

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

#include "Counter.h"
#include "Debug.h"
#include "Functors.h"

#include <QDomElement>
#include <QDomDocument>
#include <QAction>
#include <QList>
#include <QRegularExpression>

//* Text modification macro
class TextMacro final: private Base::Counter<TextMacro>
{

    public:

    //* list
    using List = QList<TextMacro>;
    using ListIterator = QListIterator<TextMacro>;

    //* constructor
    explicit TextMacro(): Counter( QStringLiteral("TextMacro") )
    {}

    //* constructor from DomElement
    explicit TextMacro( const QDomElement& );

    //* dom element
    QDomElement domElement( QDomDocument& parent ) const;

    //* Id
    int id() const
    { return id_; }

    //* set id
    void setId( int id )
    { id_ = id; }

    //* equal to operator
    bool operator == ( const TextMacro& other ) const
    {
        return
            name() == other.name() &&
            accelerator() == other.accelerator() &&
            isSeparator() == other.isSeparator() &&
            rules() == other.rules();

    }

    //* reset counter
    static void resetCounter()
    { idCounter_ = 0; }

    //* name
    const QString& name() const
    { return name_; }

    //* name
    void setName( const QString& name )
    { name_ = name; }

    //* accelerator
    const QString& accelerator() const
    { return accelerator_; }

    //* accelerator
    void setAccelerator( const QString& value )
    { accelerator_ = value; }

    //* separator flag
    bool isSeparator() const
    { return isSeparator_; }

    //* separator
    void setIsSeparator( bool value = true )
    { isSeparator_ = value; }

    //* automatic flag
    bool isAutomatic() const
    { return isAutomatic_; }

    //* automatic
    void setIsAutomatic( bool value = true )
    { isAutomatic_ = value; }

    //* result class
    /**
    first is whether any changes where applied or not
    second is the net increment in terms of number of characters between input and output text
    */
    using Result = QPair<bool, int>;
    static Result& addToResult( Result& first, const Result& second )
    {
        first.first |= second.first;
        first.second += second.second;
        return first;
    }

    //* modify text passed as argument. Return true if text is modified
    Result processText( QString& text, int position = -1 ) const;

    //* modify text passed as argument. Return true if text is modified
    bool isValid() const
    {
        if( isSeparator() ) return true;
        if( rules_.empty() ) return false;
        for( const auto& rule:rules_ )
        { if( !rule.isValid() ) return false; }
        return true;
    }

    //* used to check whether some automatic macros are present
    using isAutomaticFTor = Base::Functor::UnaryTrue<TextMacro, &TextMacro::isAutomatic>;

    //* used to get macro by name
    using SameNameFTor = Base::Functor::Unary<TextMacro, const QString&, &TextMacro::name>;

    //* return action
    QAction* action() const;

    //* used to store regular expression and corresponding replacement text
    class Rule final: private Base::Counter<Rule>
    {

        public:

        using List = QList<Rule>;
        using ListIterator = QListIterator<Rule>;

        //* constructor
        explicit Rule( const QDomElement& = QDomElement() );

        //* dom element
        QDomElement domElement( QDomDocument& parent ) const;

        //* equal to operator
        bool operator == ( const Rule& rule ) const
        {
            return
                pattern_.pattern() == rule.pattern_.pattern() &&
                replaceText() == rule.replaceText() &&
                split() == rule.split();
        }

        //* less than operator
        bool operator < ( const Rule& rule ) const
        {
            if( pattern_.pattern() != rule.pattern_.pattern() ) return pattern_.pattern() < rule.pattern_.pattern();
            if( replaceText() != rule.replaceText() ) return replaceText() < rule.replaceText();
            if( split() != rule.split() ) return split() < rule.split();
            return false;
        }

        //* validity
        bool isValid() const
        { return pattern_.isValid(); }

        //* modify text passed as argument. Return number of changed characters
        Result processText( QString& text, int position ) const;

        //* pattern
        const QRegularExpression& pattern() const
        { return pattern_; }

        //* parent name
        void setPattern( const QString& pattern )
        { pattern_.setPattern( pattern ); }

        //* replacemenet text
        const QString& replaceText() const
        { return replaceText_; }

        //* set replacement text
        void setReplaceText( const QString& text )
        { replaceText_ = text; }

        bool split() const
        {  return !noSplitting_; }

        //* splitting flag
        /**
        it is used to decide if the text should be splitted
        line by line and the rule applied independently on each line
        */
        void setNoSplitting()
        {  noSplitting_ = true; }

        private:

        //* modify text passed as argument. Return number of changed characters
        Result _processText( QString& text, int position ) const;

        //*@name flags
        //@{

        //* splitting flag
        /**
        it is used to decide if the text should be splitted
        line by line and the rule applied independently on each line
        */
        bool noSplitting_ = false;

        //@}
        //* match pattern
        QRegularExpression pattern_;

        //* replacement text
        QString replaceText_;

    };

    //* rules
    const Rule::List& rules() const
    { return rules_; }

    //* rules
    void setRules( const Rule::List& rules )
    { rules_ = rules; }

    //* add a rule
    void addRule( const Rule& rule )
    { rules_ << rule; }

    private:

    //* unique id counter
    static int idCounter_;

    //* unique id
    int id_ = 0;

    //* macro name
    QString name_;

    //* accelerator
    QString accelerator_;

    //* separator flag
    bool isSeparator_ = false;

    //* automatic
    /** if true the macro gets automatically executed before saving modifications */
    bool isAutomatic_ = false;

    //* list of replacement
    Rule::List rules_;

};

#endif
