/*
** Collett - Project Model Tests
** =============================
**
** This file is a part of Collett
** Copyright (C) 2026 Veronica Berglyd Olsen
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "counting.h"
#include "node.h"
#include "projectmodel.h"
#include "theme.h"
#include "tree.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QModelIndex>
#include <QSignalSpy>
#include <QtTest>

using namespace Collett;

class TestProjectModel : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void countsPropagate();
    void countsFollowMoves();
    void countsSurviveReload();
    void countsOnlyOnFiles();

private:
    int shownWords(ProjectModel *model, Node *node) const;
};

void TestProjectModel::initTestCase()
{
    QCoreApplication::setOrganizationName("CollettTest");
    QCoreApplication::setApplicationName("tst_projectmodel");
}

void TestProjectModel::cleanupTestCase()
{
    Theme::destroy();
}

/**! @brief The word count the view would show for a node.
 */
int TestProjectModel::shownWords(ProjectModel *model, Node *node) const
{
    return model->data(model->indexFromHandle(node->handle()).siblingAtColumn(1), Qt::DisplayRole).toInt();
}

/**! @brief A node's counts add up through its ancestors, and the view is told.
 */
void TestProjectModel::countsPropagate()
{
    Tree tree;
    ProjectModel *model = tree.model();

    Node *root = model->addRoot("Novel", ItemClass::NovelClass, QModelIndex());
    QVERIFY(root != nullptr);
    Node *chapter = model->addFile("Chapter", ItemLevel::ChapterLevel, model->indexFromHandle(root->handle()));
    QVERIFY(chapter != nullptr);

    // A scene nested under the chapter, placed explicitly
    Node *scene = chapter->createFile(tree.newHandle(), "Scene", ItemLevel::SceneLevel);
    model->insertChild(scene, model->indexFromHandle(chapter->handle()));
    QVERIFY(scene->parent() == chapter);

    QSignalSpy spy(model, &ProjectModel::dataChanged);

    // The scene's own count shows on the scene, the chapter and the root
    QVERIFY(model->updateCounts(scene->handle(), {50, 10, 2}));
    QCOMPARE(scene->counts().words, 10);
    QCOMPARE(scene->totals().words, 10);
    QCOMPARE(chapter->counts().words, 0);
    QCOMPARE(chapter->totals().words, 10);
    QCOMPARE(root->totals().words, 10);
    QCOMPARE(shownWords(model, scene), 10);
    QCOMPARE(shownWords(model, chapter), 10);
    QCOMPARE(shownWords(model, root), 10);

    // One notification per level, on the count column
    QCOMPARE(spy.count(), 3);
    for (int i = 0; i < 3; ++i) {
        QCOMPARE(spy.at(i).at(0).toModelIndex().column(), 1);
    }

    // The chapter's own words are added to the total
    QVERIFY(model->updateCounts(chapter->handle(), {20, 5, 1}));
    QCOMPARE(chapter->totals().words, 15);
    QCOMPARE(chapter->totals().characters, 70);
    QCOMPARE(chapter->totals().paragraphs, 3);
    QCOMPARE(root->totals().words, 15);

    // Unchanged counts and unknown handles do nothing
    spy.clear();
    QVERIFY(!model->updateCounts(chapter->handle(), {20, 5, 1}));
    QVERIFY(!model->updateCounts("0000000000000", {1, 1, 1}));
    QCOMPARE(spy.count(), 0);
}

/**! @brief Moving a node moves its counts to the new parent.
 */
void TestProjectModel::countsFollowMoves()
{
    Tree tree;
    ProjectModel *model = tree.model();

    Node *root = model->addRoot("Novel", ItemClass::NovelClass, QModelIndex());
    QModelIndex rootIndex = model->indexFromHandle(root->handle());
    Node *folderA = model->addFolder("A", rootIndex);
    Node *folderB = model->addFolder("B", rootIndex);
    Node *scene = model->addFile("Scene", ItemLevel::SceneLevel, model->indexFromHandle(folderA->handle()));
    QVERIFY(scene->parent() == folderA);

    QVERIFY(model->updateCounts(scene->handle(), {50, 10, 2}));
    QCOMPARE(folderA->totals().words, 10);
    QCOMPARE(folderB->totals().words, 0);
    QCOMPARE(root->totals().words, 10);

    QSignalSpy spy(model, &ProjectModel::dataChanged);
    model->multiMove({model->indexFromHandle(scene->handle())}, model->indexFromHandle(folderB->handle()), 0);
    QVERIFY(scene->parent() == folderB);
    QCOMPARE(folderA->totals().words, 0);
    QCOMPARE(folderB->totals().words, 10);
    QCOMPARE(root->totals().words, 10);
    QCOMPARE(shownWords(model, folderA), 0);
    QCOMPARE(shownWords(model, folderB), 10);

    // Both the old and the new parent chain were notified
    QVERIFY(spy.count() >= 4);
}

/**! @brief Totals are rebuilt from the stored counts when a tree is loaded.
 */
void TestProjectModel::countsSurviveReload()
{
    Tree tree;
    ProjectModel *model = tree.model();

    Node *root = model->addRoot("Novel", ItemClass::NovelClass, QModelIndex());
    Node *chapter = model->addFile("Chapter", ItemLevel::ChapterLevel, model->indexFromHandle(root->handle()));
    Node *scene = chapter->createFile(tree.newHandle(), "Scene", ItemLevel::SceneLevel);
    model->insertChild(scene, model->indexFromHandle(chapter->handle()));
    QVERIFY(scene->parent() == chapter);
    QVERIFY(model->updateCounts(chapter->handle(), {20, 5, 1}));
    QVERIFY(model->updateCounts(scene->handle(), {50, 10, 2}));

    QJsonObject data;
    tree.pack(data);

    Tree loaded;
    loaded.unpack(data);
    Node *lRoot = loaded.node(root->handle());
    Node *lChapter = loaded.node(chapter->handle());
    Node *lScene = loaded.node(scene->handle());
    QVERIFY(lRoot && lChapter && lScene);
    QCOMPARE(lScene->totals().words, 10);
    QCOMPARE(lChapter->counts().words, 5);
    QCOMPARE(lChapter->totals().words, 15);
    QCOMPARE(lRoot->totals().words, 15);
    QCOMPARE(lRoot->totals().characters, 70);
}

/**! @brief Counts stored on roots and folders are dropped on load and not saved.
 */
void TestProjectModel::countsOnlyOnFiles()
{
    QJsonObject file;
    file["m:type"] = "File";
    file["m:level"] = "Scene";
    file["m:handle"] = "0000000000003";
    file["u:name"] = "Scene";
    file["m:words"] = 4;
    file["m:characters"] = 16;

    QJsonObject folder;
    folder["m:type"] = "Folder";
    folder["m:handle"] = "0000000000002";
    folder["u:name"] = "Folder";
    folder["m:words"] = 50;
    folder["m:characters"] = 500;
    folder["x:items"] = QJsonArray{file};

    QJsonObject root;
    root["m:type"] = "Root";
    root["m:class"] = "Novel";
    root["m:handle"] = "0000000000001";
    root["u:name"] = "Novel";
    root["m:words"] = 100;
    root["m:characters"] = 1000;
    root["x:items"] = QJsonArray{folder};

    QJsonObject data;
    data["x:items"] = QJsonArray{root};

    Tree tree;
    tree.unpack(data);
    Node *lRoot = tree.node("0000000000001");
    Node *lFolder = tree.node("0000000000002");
    Node *lFile = tree.node("0000000000003");
    QVERIFY(lRoot && lFolder && lFile);

    // The stale counts are gone, and the totals come from the file alone
    QCOMPARE(lRoot->counts(), TextCounts());
    QCOMPARE(lFolder->counts(), TextCounts());
    QCOMPARE(lFile->counts().words, 4);
    QCOMPARE(lFolder->totals().words, 4);
    QCOMPARE(lRoot->totals().words, 4);
    QCOMPARE(lRoot->totals().characters, 16);

    // Saving writes counts for the file only
    QJsonObject saved;
    tree.pack(saved);
    QJsonObject sRoot = saved["x:items"].toArray().at(0).toObject();
    QJsonObject sFolder = sRoot["x:items"].toArray().at(0).toObject();
    QJsonObject sFile = sFolder["x:items"].toArray().at(0).toObject();
    QVERIFY(!sRoot.contains("m:words"));
    QVERIFY(!sRoot.contains("m:characters"));
    QVERIFY(!sFolder.contains("m:words"));
    QVERIFY(!sFolder.contains("m:characters"));
    QCOMPARE(sFile["m:words"].toInt(), 4);
    QCOMPARE(sFile["m:characters"].toInt(), 16);

    // The expanded flag is only written for nodes with children
    QVERIFY(sRoot.contains("m:expanded"));
    QVERIFY(sFolder.contains("m:expanded"));
    QVERIFY(!sFile.contains("m:expanded"));
}

QTEST_MAIN(TestProjectModel)
#include "tst_projectmodel.moc"
