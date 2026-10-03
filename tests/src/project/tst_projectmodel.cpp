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

#include "group.h"
#include "node.h"
#include "projectmodel.h"

#include <QSignalSpy>
#include <QtTest>

using namespace Collett;

class TestProjectModel : public QObject
{
    Q_OBJECT

private slots:
    void init();
    void cleanup();
    void numbering();
    void unnumberedChapter();
    void folding();
    void blockSize();
    void moveBlock();
    void moveBlockFolded();
    void moveBlockUnfolds();
    void setCounts();
    void insertAndTake();
    void sync();

private:
    Group *m_group = nullptr;
    ProjectModel *m_model = nullptr;

    void build(const QString &layout);
    QString order() const;
    QVariant value(int row, int role) const { return m_model->data(m_model->index(row), role); };
};

/**! @brief Build a group from a layout string, one letter per document:
 * T for a page, P for a partition, C for a chapter and S for a scene. Each
 * document's handle and title is its letter and row.
 */
void TestProjectModel::build(const QString &layout)
{
    delete m_model;
    delete m_group;
    m_group = new Group("Novel", ItemClass::NovelClass);
    for (qsizetype i = 0; i < layout.size(); ++i) {
        const QChar c = layout.at(i);
        const ItemLevel level = c == 'T'   ? ItemLevel::PageLevel
                                : c == 'P' ? ItemLevel::PartitionLevel
                                : c == 'C' ? ItemLevel::ChapterLevel
                                           : ItemLevel::SceneLevel;
        const QString name = QString("%1%2").arg(c).arg(i);
        m_group->appendItem(new Node(name, name, level));
    }
    m_model = new ProjectModel();
    m_model->setGroup(m_group);
}

/**! @brief The handles in row order, with hidden rows in brackets.
 */
QString TestProjectModel::order() const
{
    QStringList rows;
    for (int i = 0; i < m_model->rowCount(); ++i) {
        const QString handle = value(i, ProjectModel::HandleRole).toString();
        rows.append(value(i, ProjectModel::HiddenRole).toBool() ? "(" + handle + ")" : handle);
    }
    return rows.join(' ');
}

void TestProjectModel::init()
{
    m_group = nullptr;
    m_model = nullptr;
}

void TestProjectModel::cleanup()
{
    delete m_model;
    delete m_group;
}

/**! @brief Chapters are numbered through the novel, and scenes from 1 in
 * each chapter or partition. Pages and partitions have no number.
 */
void TestProjectModel::numbering()
{
    build("TCSSCSPSCS");
    const QList<int> numbers = {0, 1, 1, 2, 2, 1, 0, 1, 3, 1};
    const QList<int> chapters = {0, 1, 1, 1, 2, 2, 0, 0, 3, 3};
    for (int i = 0; i < numbers.size(); ++i) {
        QCOMPARE(value(i, ProjectModel::NumberRole).toInt(), numbers.at(i));
        QCOMPARE(value(i, ProjectModel::ChapterNumberRole).toInt(), chapters.at(i));
    }
}

/**! @brief An unnumbered chapter is skipped by the numbering, and its scenes
 * have no chapter number.
 */
void TestProjectModel::unnumberedChapter()
{
    build("CSCSCS");
    QSignalSpy structure(m_model, &ProjectModel::structureChanged);
    m_model->setNumbered(2, false);
    QCOMPARE(structure.count(), 1);
    QCOMPARE(value(2, ProjectModel::NumberedRole).toBool(), false);
    QCOMPARE(value(2, ProjectModel::NumberRole).toInt(), 0);
    QCOMPARE(value(3, ProjectModel::ChapterNumberRole).toInt(), 0);
    QCOMPARE(value(4, ProjectModel::NumberRole).toInt(), 2);
    QCOMPARE(value(5, ProjectModel::ChapterNumberRole).toInt(), 2);

    // Scenes are always numbered
    m_model->setNumbered(1, false);
    QCOMPARE(value(1, ProjectModel::NumberedRole).toBool(), true);
}

/**! @brief A folded chapter hides its scenes, and a folded partition hides
 * everything up to the next partition. Only documents with something under
 * them can fold.
 */
void TestProjectModel::folding()
{
    build("PCSCSPCS");
    QCOMPARE(value(0, ProjectModel::FoldableRole).toBool(), true);
    QCOMPARE(value(2, ProjectModel::FoldableRole).toBool(), false);

    m_model->toggleExpanded(1);
    QCOMPARE(order(), "P0 C1 (S2) C3 S4 P5 C6 S7");

    m_model->toggleExpanded(0);
    QCOMPARE(order(), "P0 (C1) (S2) (C3) (S4) P5 C6 S7");

    m_model->toggleExpanded(0);
    QCOMPARE(order(), "P0 C1 (S2) C3 S4 P5 C6 S7");

    // A scene cannot fold
    m_model->toggleExpanded(4);
    QCOMPARE(order(), "P0 C1 (S2) C3 S4 P5 C6 S7");

    // An empty chapter cannot fold
    build("CC");
    QCOMPARE(value(0, ProjectModel::FoldableRole).toBool(), false);
}

/**! @brief A row moves together with the rows it hides.
 */
void TestProjectModel::blockSize()
{
    build("CSSCS");
    QCOMPARE(m_model->blockSize(0), 1);
    QCOMPARE(m_model->blockSize(1), 1);
    m_model->toggleExpanded(0);
    QCOMPARE(m_model->blockSize(0), 3);
    QCOMPARE(m_model->blockSize(3), 1);
    QCOMPARE(m_model->blockSize(9), 0);
}

/**! @brief Rows move to before another row, and moves that go nowhere are
 * refused.
 */
void TestProjectModel::moveBlock()
{
    build("CSSCS");
    QSignalSpy structure(m_model, &ProjectModel::structureChanged);

    // Down: the scene at 1 goes before row 5, the end
    QVERIFY(m_model->moveBlock(1, 1, 5));
    QCOMPARE(order(), "C0 S2 C3 S4 S1");
    QCOMPARE(structure.count(), 1);

    // Up: the last scene goes before row 1
    QVERIFY(m_model->moveBlock(4, 1, 1));
    QCOMPARE(order(), "C0 S1 S2 C3 S4");
    QCOMPARE(value(2, ProjectModel::NumberRole).toInt(), 2);

    // Moving to where it already is, or into itself, is refused
    QVERIFY(!m_model->moveBlock(1, 1, 1));
    QVERIFY(!m_model->moveBlock(1, 1, 2));
    QVERIFY(!m_model->moveBlock(1, 2, 2));
    QVERIFY(!m_model->moveBlock(1, 1, 9));
    QVERIFY(!m_model->moveBlock(4, 2, 0));
    QCOMPARE(order(), "C0 S1 S2 C3 S4");
}

/**! @brief A folded chapter moves with its scenes and stays folded.
 */
void TestProjectModel::moveBlockFolded()
{
    build("CSCSS");
    m_model->toggleExpanded(2);
    QCOMPARE(order(), "C0 S1 C2 (S3) (S4)");

    QVERIFY(m_model->moveBlock(2, m_model->blockSize(2), 0));
    QCOMPARE(order(), "C2 (S3) (S4) C0 S1");
    QCOMPARE(value(0, ProjectModel::ExpandedRole).toBool(), false);
}

/**! @brief A move never hides a document that was in view. Dropping a folded
 * chapter between another chapter and its scenes unfolds it, and dropping a
 * scene into a folded chapter unfolds that chapter.
 */
void TestProjectModel::moveBlockUnfolds()
{
    build("CSCS");
    m_model->toggleExpanded(0);
    QCOMPARE(order(), "C0 (S1) C2 S3");

    QVERIFY(m_model->moveBlock(0, 2, 3));
    QCOMPARE(order(), "C2 C0 S1 S3");
    QCOMPARE(value(1, ProjectModel::ExpandedRole).toBool(), true);

    build("CSCS");
    m_model->toggleExpanded(0);
    QVERIFY(m_model->moveBlock(3, 1, 2));
    QCOMPARE(order(), "C0 S1 S3 C2");
    QCOMPARE(value(0, ProjectModel::ExpandedRole).toBool(), true);
}

/**! @brief New counts update the words and mark the structure as changed,
 * but the same counts do nothing.
 */
void TestProjectModel::setCounts()
{
    build("CS");
    QSignalSpy structure(m_model, &ProjectModel::structureChanged);
    QSignalSpy data(m_model, &ProjectModel::dataChanged);

    TextCounts counts;
    counts.words = 42;
    counts.characters = 200;
    m_model->setCounts(1, counts);
    QCOMPARE(value(1, ProjectModel::WordsRole).toInt(), 42);
    QCOMPARE(structure.count(), 1);
    QCOMPARE(data.count(), 1);

    m_model->setCounts(1, counts);
    QCOMPARE(structure.count(), 1);
    QCOMPARE(data.count(), 1);

    m_model->setCounts(9, counts);
    QCOMPARE(structure.count(), 1);
}

/**! @brief Inserted documents get numbered, and taken documents are handed
 * back to the caller.
 */
void TestProjectModel::insertAndTake()
{
    build("CSS");
    m_model->insertNode(2, new Node("new", "", ItemLevel::SceneLevel));
    QCOMPARE(order(), "C0 S1 new S2");
    QCOMPARE(m_model->rowOf("new"), 2);
    QCOMPARE(value(3, ProjectModel::NumberRole).toInt(), 3);

    Node *node = m_model->takeNode(1);
    QVERIFY(node);
    QCOMPARE(node->handle(), QStringLiteral("S1"));
    QCOMPARE(order(), "C0 new S2");
    QCOMPARE(m_model->rowOf("S1"), -1);
    delete node;

    QCOMPARE(m_model->takeNode(9), nullptr);
}

/**! @brief The model follows a list of documents: rows that are gone are
 * removed, rows move into the new order, new rows are made, and each row
 * takes its values. The views are told with the matching signals.
 */
void TestProjectModel::sync()
{
    build("CSSC");
    QStringList created;
    QStringList disposed;
    auto create = [&created](const Document::Item &item) {
        created.append(item.handle);
        return new Node(item.handle, QString(), item.level);
    };
    auto dispose = [&disposed](Node *node) {
        disposed.append(node->handle());
        delete node;
    };
    auto item = [](const QString &handle, ItemLevel level, const QString &title = QString()) {
        Document::Item item;
        item.handle = handle;
        item.title = title.isEmpty() ? handle : title;
        item.level = level;
        return item;
    };

    QSignalSpy removed(m_model, &ProjectModel::rowsRemoved);
    QSignalSpy moved(m_model, &ProjectModel::rowsMoved);
    QSignalSpy inserted(m_model, &ProjectModel::rowsInserted);
    QSignalSpy structure(m_model, &ProjectModel::structureChanged);

    // Nothing changes
    m_model->sync({item("C0", ChapterLevel), item("S1", SceneLevel), item("S2", SceneLevel), item("C3", ChapterLevel)}, create, dispose);
    QCOMPARE(structure.count(), 0);

    // S1 goes, S2 moves last, N is new, and C3 becomes a scene with a title
    Document::Item scene = item("C3", SceneLevel, "Renamed");
    scene.hardBreak = true;
    m_model->sync({item("C0", ChapterLevel), item("N", SceneLevel), scene, item("S2", SceneLevel)}, create, dispose);
    QCOMPARE(order(), QStringLiteral("C0 N C3 S2"));
    QCOMPARE(disposed, QStringList({"S1"}));
    QCOMPARE(created, QStringList({"N"}));
    QCOMPARE(removed.count(), 1);
    QCOMPARE(inserted.count(), 1);
    QCOMPARE(moved.count(), 1);
    QVERIFY(structure.count() > 0);
    QCOMPARE(value(2, ProjectModel::TitleRole).toString(), QStringLiteral("Renamed"));
    QCOMPARE(value(2, ProjectModel::LevelRole).toInt(), int(SceneLevel));
    QVERIFY(value(2, ProjectModel::HardBreakRole).toBool());
    QCOMPARE(value(3, ProjectModel::NumberRole).toInt(), 3);
}

QTEST_GUILESS_MAIN(TestProjectModel)
#include "tst_projectmodel.moc"
