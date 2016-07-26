/*
 * Copyright 2014  Olivier de Gaalon <olivier.jg@gmail.com>
 * Copyright 2014  Milian Wolff <mail@milianw.de>
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
 */

#ifndef CLANGHELPERS_H
#define CLANGHELPERS_H

#include "clangprivateexport.h"

#include <language/duchain/duchainpointer.h>
#include <language/duchain/topducontext.h>

#include <clang-c/Index.h>

#include <functional>

class ParseSession;
class ClangIndex;

struct Import
{
    CXFile file;
    KDevelop::CursorInRevision location;
};

using Imports = QMultiHash<CXFile, Import>;
using IncludeFileContexts = QHash<CXFile, KDevelop::ReferencedTopDUContext>;

namespace ClangHelpers {

KDevelop::DeclarationPointer findDeclaration(CXSourceLocation cursor, KDevelop::QualifiedIdentifier id, const KDevelop::ReferencedTopDUContext& top);
KDevelop::DeclarationPointer findDeclaration(CXCursor cursor, const IncludeFileContexts& includes);
KDevelop::DeclarationPointer findDeclaration(CXType type, const IncludeFileContexts& includes);

/**
 * Try to look up the first reachable forward declaration for type @p type
 *
 * @param context The context where this search is happening
 * @param cursor The location from which we're searching
 */
KDevelop::DeclarationPointer findForwardDeclaration(CXType type, KDevelop::DUContext* context, CXCursor cursor);

/**
 * Wrapper for @ref clang_Cursor_getSpellingNameRange which sometimes reports invalid ranges
 */
KDevelop::RangeInRevision cursorSpellingNameRange(CXCursor cursor, const KDevelop::Identifier& id);

/**
 * @returns all the Imports for each file in the @param tu
 */
KDEVCLANGPRIVATE_EXPORT Imports tuImports(CXTranslationUnit tu);

/**
 * Recursively builds a duchain with the specified @param features for the
 * @param file and each of its @param imports using the TU from @param session.
 * The resulting contexts are placed in @param includedFiles.
 * @returns the context created for @param file
 */
KDEVCLANGPRIVATE_EXPORT KDevelop::ReferencedTopDUContext buildDUChain(
    CXFile file, const Imports& imports, const ParseSession& session,
    KDevelop::TopDUContext::Features features, IncludeFileContexts& includedFiles,
    ClangIndex* index = nullptr, const std::function<bool()>& abortFunction = {});

/**
 * @return List of possible header extensions used for definition/declaration fallback switching
 */
QStringList headerExtensions();

/**
 * @return List of possible source extensions used for definition/declaration fallback switching
 */
QStringList sourceExtensions();

/**
 * @return True if the given file @p path has the extension of a C++ source file
 */
KDEVCLANGPRIVATE_EXPORT bool isSource(const QString& path);

/**
 * @return True if the given file @p path has the extension of a C++ header file
 */
KDEVCLANGPRIVATE_EXPORT bool isHeader(const QString& path);

}

#endif //CLANGHELPERS_H
