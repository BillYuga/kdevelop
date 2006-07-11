/* This file is part of KDevelop
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <QtTest/QtTest>
#include <QFlags>

#include "duchain.h"
#include "ducontext.h"
#include "dubuilder.h"
#include "typesystem.h"
#include "definition.h"
#include "documentrange.h"
#include "editorintegrator.h"
#include "dubuilder.h"

#include "parser.h"
#include "control.h"
#include "dumptree.h"
#include "tokens.h"
#include "testconfig.h"

using namespace KTextEditor;

class TestDUChain : public QObject
{
  Q_OBJECT

  // Parser
  Control control;
  DumpTree dumper;

  // Definition - use chain
  TypeEnvironment types;
  const AbstractType* type1;
  const AbstractType* type2;
  const AbstractType* type3;
  Definition* definition1;
  Definition* definition2;
  Definition* definition3;
  Definition* definition4;
  KUrl file1, file2;
  DUContext* topContext;

public:
  TestDUChain()
  {
  }

private slots:

  void initTestCase()
  {
    type1 = types.integralType(types.intern("definitionChain"));
    type2 = types.referenceType(type1);
    type3 = types.pointerType(type1);

    definition1 = new Definition(new DocumentRange(file1, Range(4,4,4,16)), Definition::LocalScope);
    definition1->setType(type1);
    definition1->setIdentifier("lazy");

    definition2 = new Definition(new DocumentRange(file1, Range(5,4,5,16)), Definition::ClassScope);
    definition2->setType(type2);
    definition2->setIdentifier("m_errorCode");

    definition3 = new Definition(new DocumentRange(file1, Range(6,4,6,16)), Definition::GlobalScope);
    definition3->setType(type3);
    definition3->setIdentifier("lazy");

    definition4 = new Definition(new DocumentRange(file1, Range(7,4,7,16)), Definition::ClassScope);
    definition4->setType(type2);
    definition4->setIdentifier("m_errorCode2");


    file1 = "file:///opt/kde4/src/kdevelop/languages/cpp/parser/duchain.cpp";
    file2 = "file:///opt/kde4/src/kdevelop/languages/cpp/parser/dubuilder.cpp";

    topContext = new DUContext(new DocumentRange(file1, Range(0,0,25,0)));
    DUChain::self()->addDocumentChain(file1, topContext);
  }

  void cleanupTestCase()
  {
    /*delete type1;
    delete type2;
    delete type3;*/

    delete definition1;
    delete definition2;
    delete definition3;
    delete definition4;

    delete topContext;
  }

  void testContextRelationships()
  {
    QCOMPARE(DUChain::self()->chainForDocument(file1), topContext);

    DUContext* firstChild = new DUContext(new DocumentRange(file1, Range(4,4, 10,3)));
    topContext->addChildContext(firstChild);

    QCOMPARE(firstChild->parentContexts().count(), 1);
    QCOMPARE(firstChild->parentContexts().first(), topContext);
    QCOMPARE(firstChild->childContexts().count(), 0);
    QCOMPARE(topContext->childContexts().count(), 1);
    QCOMPARE(topContext->childContexts().last(), firstChild);

    DUContext* secondChild = new DUContext(new DocumentRange(file1, Range(14,4, 19,3)));
    topContext->addChildContext(secondChild);

    QCOMPARE(topContext->childContexts().count(), 2);
    QCOMPARE(topContext->childContexts()[1], secondChild);

    DUContext* thirdChild = new DUContext(new DocumentRange(file1, Range(10,4, 14,3)));
    topContext->addChildContext(thirdChild);

    QCOMPARE(topContext->childContexts().count(), 3);
    QCOMPARE(topContext->childContexts()[1], thirdChild);

    delete topContext->takeChildContext(firstChild);
    QCOMPARE(topContext->childContexts().count(), 2);
    QCOMPARE(topContext->childContexts(), QList<DUContext*>() << thirdChild << secondChild);

    topContext->deleteChildContextsRecursively();
    QVERIFY(topContext->childContexts().isEmpty());
  }

  void testLocalDefinitions()
  {
    QCOMPARE(definition1->type(), type1);
    QCOMPARE(definition1->identifier(), QString("lazy"));
    QCOMPARE(definition1->scope(), Definition::LocalScope);
    QCOMPARE(topContext->localDefinitions().count(), 0);

    topContext->addDefinition(definition1);
    QCOMPARE(topContext->localDefinitions().count(), 1);
    QCOMPARE(topContext->localDefinitions()[0], definition1);

    topContext->addDefinition(definition2);
    QCOMPARE(topContext->localDefinitions().count(), 2);
    QCOMPARE(topContext->localDefinitions()[1], definition2);

    kDebug() << k_funcinfo << "Warning expected here (bug if not present)." << endl;
    topContext->addDefinition(definition2);
    QCOMPARE(topContext->localDefinitions().count(), 2);

    topContext->clearLocalDefinitions();
    QVERIFY(topContext->localDefinitions().isEmpty());
  }

  void testDefinitionChain()
  {
    topContext->addDefinition(definition1);
    topContext->addDefinition(definition2);

    DUContext* context1 = new DUContext(new DocumentRange(file1, Range(4,4, 14,3)));
    topContext->addChildContext(context1);
    DocumentCursor insideContext1(file1, KTextEditor::Cursor(5,9));

    QCOMPARE(topContext->findContext(insideContext1), context1);
    QCOMPARE(topContext->findDefinition(definition1->identifier(), insideContext1), definition1);
    QCOMPARE(context1->findDefinition(definition1->identifier(), insideContext1), definition1);

    context1->addDefinition(definition3);
    QCOMPARE(topContext->findDefinition(definition1->identifier(), insideContext1), definition1);
    QCOMPARE(context1->findDefinition(definition1->identifier(), insideContext1), definition3);

    context1->takeDefinition(definition3);
    QCOMPARE(topContext->findDefinition(definition1->identifier(), insideContext1), definition1);
    QCOMPARE(context1->findDefinition(definition1->identifier(), insideContext1), definition1);

    DUContext* subContext1 = new DUContext(new DocumentRange(file1, Range(5,4, 7,3)));
    topContext->addChildContext(subContext1);

    DUContext* subContext2 = new DUContext(new DocumentRange(file1, Range(9,4, 12,3)));
    topContext->addChildContext(subContext2);

    subContext1->addDefinition(definition3);

    subContext2->addDefinition(definition4);

    subContext1->takeDefinition(definition3);
    subContext2->takeDefinition(definition4);
    topContext->takeDefinition(definition1);
    topContext->takeDefinition(definition2);

    topContext->deleteChildContextsRecursively();
  }

  void testDeclareInt()
  {
    QByteArray method("int i;");

    DUContext* top = parse(method, DumpDUChain);

    QCOMPARE(top->childContexts().count(), 0);
    QCOMPARE(top->localDefinitions().count(), 1);

    Definition* def = top->localDefinitions().first();
    QCOMPARE(def->identifier(), QString("i"));

    //delete top;
  }

  void testDeclareFunction()
  {
    //                 0         1         2
    //                 012345678901234567890123456789
    QByteArray method("void A::t(int i) { i = i + 3; }");

    DUContext* top = parse(method, DumpDUChain);

    QCOMPARE(top->childContexts().count(), 1);
    QCOMPARE(top->localDefinitions().count(), 1);

    Definition* def = top->localDefinitions().first();
    QCOMPARE(def->identifier(), QString("A::t"));

    DUContext* fn = top->childContexts().first();
    QCOMPARE(fn->childContexts().count(), 1);
    QCOMPARE(fn->localDefinitions().count(), 1);

    def = fn->localDefinitions().first();
    QCOMPARE(def->identifier(), QString("i"));
    QCOMPARE(def->uses().count(), 2);

    DUContext* insideFn = fn->childContexts().first();
    QCOMPARE(insideFn->childContexts().count(), 0);
    QCOMPARE(insideFn->localDefinitions().count(), 0);

    //delete top;
  }

  void testDeclareFor()
  {
    //                 0         1         2         3         4         5
    //                 012345678901234567890123456789012345678901234567890123456789
    QByteArray method("int main() { for (int i = 0; i < 10; i++) {} }");

    DUContext* top = parse(method, DumpDUChain);

    QCOMPARE(top->childContexts().count(), 1);
    QCOMPARE(top->localDefinitions().count(), 1);

    Definition* def = top->localDefinitions().first();
    QCOMPARE(def->identifier(), QString("main"));

    DUContext* main = top->childContexts().first();
    QCOMPARE(main->childContexts().count(), 1);
    QCOMPARE(main->localDefinitions().count(), 0);

    DUContext* forCtx = main->childContexts().first();
    QCOMPARE(forCtx->childContexts().count(), 1);
    QCOMPARE(forCtx->localDefinitions().count(), 1);

    def = forCtx->localDefinitions().first();
    QCOMPARE(def->identifier(), QString("i"));
    QCOMPARE(def->uses().count(), 2);

    DUContext* insideFor = forCtx->childContexts().first();
    QCOMPARE(insideFor->childContexts().count(), 0);
    QCOMPARE(insideFor->localDefinitions().count(), 0);

    //delete top;
  }

public:
  enum DumpType {
    DumpNone = 0,
    DumpAST = 1,
    DumpDUChain = 2
  };
  Q_DECLARE_FLAGS(DumpTypes, DumpType)

private:
  DUContext* parse(const QByteArray& unit, DumpTypes dump = static_cast<DumpTypes>(DumpAST | DumpDUChain));
};

Q_DECLARE_OPERATORS_FOR_FLAGS(TestDUChain::DumpTypes)

DUContext* TestDUChain::parse(const QByteArray& unit, DumpTypes dump)
{
  kDebug() << "==== Beginning new test case...:" << endl << unit << endl << endl;
  pool mem_pool;

  Parser parser(&control);
  TranslationUnitAST* ast = parser.parse(unit.constData(), unit.size() + 1, &mem_pool);

  EditorIntegrator::addParsedSource(&parser.lexer, &parser.token_stream);

  if (dump & DumpAST) {
    kDebug() << "===== AST:" << endl;
    dumper.dump(ast, &parser.token_stream);
  }

  DUBuilder dubuilder(&parser.token_stream);
  DUContext* top = dubuilder.build(KUrl("file:///internal"), ast);

  if (dump & DumpDUChain) {
    kDebug() << "===== DUChain:" << endl;
    dumper.dump(top);
  }

  kDebug() << "===== Finished test case." << endl;

  return top;
}

#include "test_duchain.moc"

QTEST_MAIN(TestDUChain)
