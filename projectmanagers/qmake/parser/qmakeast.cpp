/* KDevelop QMake Support
 *
 * Copyright 2006 Andreas Pakulat <apaku@gmx.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "ast.h"

namespace QMake {

AST::AST(AST* parent, AST::Type type)
    : type(type)
    , startLine(-1)
    , endLine(-1)
    , startColumn(-1)
    , endColumn(-1)
    , start(-1)
    , end(-1)
    , parent(parent)
{
}

AST::~AST()
{
}

ValueAST::ValueAST(AST* parent)
    : AST(parent, AST::Value)
{
}

StatementAST::StatementAST(AST* parent, AST::Type type)
    : AST(parent, type)
{
}

StatementAST::~StatementAST()
{
}

AssignmentAST::AssignmentAST(AST* parent)
    : StatementAST(parent, AST::Assignment)
    , identifier(nullptr)
    , op(nullptr)
{
}

AssignmentAST::~AssignmentAST()
{
    delete identifier;
    identifier = nullptr;
    qDeleteAll(values);
    values.clear();
    delete op;
}

ScopeBodyAST::ScopeBodyAST(AST* parent, AST::Type type)
    : AST(parent, type)
{
}

ScopeBodyAST::~ScopeBodyAST()
{
    qDeleteAll(statements);
    statements.clear();
}

FunctionCallAST::FunctionCallAST(AST* parent)
    : ScopeAST(parent, AST::FunctionCall)
    , identifier(nullptr)
{
}

FunctionCallAST::~FunctionCallAST()
{
    delete identifier;
    identifier = nullptr;
    qDeleteAll(args);
    args.clear();
}

OrAST::OrAST(AST* parent)
    : ScopeAST(parent, AST::Or)
{
}

OrAST::~OrAST()
{
    qDeleteAll(scopes);
    scopes.clear();
}

ProjectAST::ProjectAST()
    : ScopeBodyAST(nullptr, AST::Project)
{
}

ProjectAST::~ProjectAST()
{
}

ScopeAST::ScopeAST(AST* parent, AST::Type type)
    : StatementAST(parent, type)
    , body(nullptr)
{
}

ScopeAST::~ScopeAST()
{
    delete body;
    body = nullptr;
}

SimpleScopeAST::SimpleScopeAST(AST* parent)
    : ScopeAST(parent, AST::SimpleScope)
    , identifier(nullptr)
{
}

SimpleScopeAST::~SimpleScopeAST()
{
    delete identifier;
    identifier = nullptr;
}
}
