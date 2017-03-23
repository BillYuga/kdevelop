/*
 * Copyright 2014 Kevin Funk <kfunk@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef CLANGUTILS_H
#define CLANGUTILS_H

#include <util/path.h>

#include <KTextEditor/View>

#include <clang-c/Index.h>

#include "clangprivateexport.h"

namespace ClangUtils
{
    /**
     * Finds the most specific CXCursor which applies to the specified line and column
     * in the given translation unit and file.
     *
     * @param line The 0-indexed line number at which to search.
     * @param column The 0-indexed column number at which to search.
     * @param unit The translation unit to examine.
     * @param file The file in the translation unit to examine.
     *
     * @return The cursor at the specified location
     */
    CXCursor getCXCursor(int line, int column, const CXTranslationUnit& unit, const CXFile& file);

    enum DefaultArgumentsMode
    {
        FixedSize, ///< The vector will have length equal to the number of arguments to the function
                   /// and any arguments without a default parameter will be represented with an empty string.
        MinimumSize ///< The vector will have a length equal to the number of default values
    };

    /**
     * Given a cursor representing a function, returns a vector containing the string
     * representations of the default arguments of the function which are defined at
     * the occurrence of the cursor. Note that this is not necessarily all of the default
     * arguments of the function.
     *
     * @param cursor The cursor to examine
     * @return a vector of QStrings representing the default arguments, or an empty
     *         vector if cursor does not represent a function
     */
    QVector<QString> getDefaultArguments(CXCursor cursor, DefaultArgumentsMode mode = FixedSize);

    /**
     * @return true when the cursor kind references a named scope.
     */
    bool isScopeKind(CXCursorKind kind);

    /**
     * Given a cursor and destination context, returns the string representing the
     * cursor's scope at its current location.
     *
     * @param cursor The cursor to examine
     * @param context The destination context from which the cursor should be referenced.
     *                By default this will be set to the cursors lexical parent.
     * @return the cursor's scope as a string
     */
    KDEVCLANGPRIVATE_EXPORT QString getScope(CXCursor cursor, CXCursor context = clang_getNullCursor());

    /**
     * Given a cursor representing some sort of function, returns its signature. The
     * effect of this function when passed a non-function cursor is undefined.
     *
     * @param cursor The cursor to work with
     * @param scope The scope of the cursor (e.g. "SomeNS::SomeClass")
     * @return A QString of the function's signature
     */
    QString getCursorSignature(CXCursor cursor, const QString& scope, const QVector<QString>& defaultArgs = QVector<QString>());

    /**
     * Given a cursor representing the template argument list, return a
     * list of the argument types.
     *
     * @param cursor The cursor to work with
     * @return A QStringList of the template's arguments
     */
    KDEVCLANGPRIVATE_EXPORT QStringList templateArgumentTypes(CXCursor cursor);

    /**
     * Extract the raw contents of the range @p range
     *
     * @note This will return the exact textual representation of the code,
     *   no whitespace stripped, etc.
     *
     * TODO: It would better if we'd be able to just memcpy parts of the file buffer
     * that's stored inside Clang (cf. llvm::MemoryBuffer for files), but libclang
     * doesn't offer API for that. This implementation here is a lot more expensive.
     *
     * @param unit Translation unit this range is part of
     */
    KDEVCLANGPRIVATE_EXPORT QByteArray getRawContents(CXTranslationUnit unit, CXSourceRange range);

    /**
     * @brief Return true if file @p file1 and file @p file2 are equal
     *
     * @see clang_File_isEqual
     */
    inline bool isFileEqual(CXFile file1, CXFile file2)
    {
#if CINDEX_VERSION_MINOR >= 28
        return clang_File_isEqual(file1, file2);
#else
        // note: according to the implementation of clang_File_isEqual, file1 and file2 can still be equal,
        // regardless of whether file1 == file2 is true or not
        // however, we didn't see any problems with pure pointer comparisons until now, so fall back to that
        return file1 == file2;
#endif
    }

    /**
     * @brief Return true if the cursor @p cursor refers to an explicitly deleted/defaulted function
     * such as the default constructor in "struct Foo { Foo() = delete; }"
     *
     * TODO: do we need isExplicitlyDefaulted() + isExplicitlyDeleted()?
     * Currently this is only used by the implements completion to hide deleted+defaulted functions so
     * we don't need to know the difference. We need to tokenize the source code because there is no
     * such API in libclang so having one function to check both cases is more efficient (only tokenize once)
     */
    bool isExplicitlyDefaultedOrDeleted(CXCursor cursor);

    /**
    * Extract the range of the path-spec inside the include-directive in line @p line
    *
    * Example: line = "#include <vector>" => returns {0, 10, 0, 16}
    *
    * @param originalRange This is the range that the resulting range will be based on
    *
    * @return Range pointing to the path-spec of the include or invalid range if there is no #include directive on the line.
    */
    KDEVCLANGPRIVATE_EXPORT KTextEditor::Range rangeForIncludePathSpec(const QString& line, const KTextEditor::Range& originalRange = KTextEditor::Range());

    enum SpecialQtAttributes {
        NoQtAttribute,
        QtSignalAttribute,
        QtSlotAttribute
    };
    /**
     * Given a cursor representing a CXXmethod
     */
    SpecialQtAttributes specialQtAttributes(CXCursor cursor);
};

#endif // CLANGUTILS_H
