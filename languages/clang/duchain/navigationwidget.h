/*
 * This file is part of KDevelop
 * Copyright 2014 Milian Wolff <mail@milianw.de>
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
 */

#ifndef NAVIGATIONWIDGET_H
#define NAVIGATIONWIDGET_H

#include "clangprivateexport.h"

#include "macrodefinition.h"

#include <language/duchain/navigation/abstractnavigationwidget.h>

namespace KDevelop
{
class DocumentCursor;
class IncludeItem;
}

class KDEVCLANGPRIVATE_EXPORT ClangNavigationWidget : public KDevelop::AbstractNavigationWidget
{
public:
    ClangNavigationWidget(const KDevelop::DeclarationPointer& declaration,
                          KDevelop::AbstractNavigationWidget::DisplayHints hints = KDevelop::AbstractNavigationWidget::NoHints);
    ClangNavigationWidget(const MacroDefinition::Ptr& macro, const KDevelop::DocumentCursor& expansionLocation,
                          KDevelop::AbstractNavigationWidget::DisplayHints hints = KDevelop::AbstractNavigationWidget::NoHints);
    ClangNavigationWidget(const KDevelop::IncludeItem& includeItem, KDevelop::TopDUContextPointer topContext,
                          const QString& htmlPrefix = QString(), const QString& htmlSuffix = QString(),
                          KDevelop::AbstractNavigationWidget::DisplayHints hints = KDevelop::AbstractNavigationWidget::NoHints);
    virtual ~ClangNavigationWidget() = default;

    /// Used by @see AbstractIncludeFileCompletionItem
    static QString shortDescription(const KDevelop::IncludeItem& includeItem);
};

#endif // NAVIGATIONWIDGET_H
