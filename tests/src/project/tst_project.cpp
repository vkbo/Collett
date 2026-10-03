/*
** Collett - Project Tests
** =======================
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

#include "project.h"
#include "projectmodel.h"
#include "tree.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QTextCursor>
#include <QtTest>

using namespace Collett;

class TestProject : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void fileLayout();
    void reopen();
    void saveChangedOnly();
    void groupFileNames();
    void duplicateHandles();

private:
    QJsonObject readJson(const QString &path) const;
    void writeJson(const QString &path, const QJsonObject &data) const;
    QString addScene(Project &project, const QString &title, const QString &text) const;
};

void TestProject::initTestCase()
{
    QCoreApplication::setOrganizationName("CollettTest");
    QCoreApplication::setApplicationName("tst_project");
}

QJsonObject TestProject::readJson(const QString &path) const
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return QJsonObject();
    return QJsonDocument::fromJson(file.readAll()).object();
}

void TestProject::writeJson(const QString &path, const QJsonObject &data) const
{
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(QJsonDocument(data).toJson());
}

/**! @brief Add a scene at the end of the project. Returns its handle.
 */
QString TestProject::addScene(Project &project, const QString &title, const QString &text) const
{
    ProjectModel *model = project.model();
    const QString last = model->data(model->index(model->rowCount() - 1), ProjectModel::HandleRole).toString();
    Document *doc = project.openDocument(last);
    const QString added = project.splitDocument(last, doc->characterCount() - 1);
    model->setTitle(model->rowOf(added), title);
    QTextCursor(project.openDocument(added)).insertText(text);
    return added;
}

/**! @brief The structure file lists the groups and their files, and the
 * group file holds the documents with their text. There is no file per
 * document.
 */
void TestProject::fileLayout()
{
    QTemporaryDir dir;
    Project project;
    QCOMPARE(project.createProject(dir.path(), "Novel"), QString());
    const QString scene = addScene(project, "First", "Some text.");
    QVERIFY(project.saveProject());

    const QString root = dir.filePath("Novel");
    const QJsonArray groups = readJson(root + "/project/structure.json").value("x:groups").toArray();
    QCOMPARE(groups.size(), 1);
    const QJsonObject group = groups.at(0).toObject();
    QCOMPARE(group.value("m:class").toString(), QStringLiteral("Novel"));
    QCOMPARE(group.value("m:file").toString(), QStringLiteral("document1.json"));
    QVERIFY(!group.contains("x:items"));

    QCOMPARE(QDir(root + "/content").entryList(QDir::Files), QStringList({"document1.json"}));
    const QJsonObject content = readJson(root + "/content/document1.json");
    QCOMPARE(content.value("c:format").toString(), QStringLiteral("CollettDocument:1.0"));
    const QJsonArray items = content.value("x:items").toArray();
    QCOMPARE(items.size(), 2);

    const QJsonObject page = items.at(0).toObject();
    QCOMPARE(page.value("m:level").toString(), QStringLiteral("Page"));
    QVERIFY(page.value("x:content").toArray().at(0).toObject().value("u:txt").toString().endsWith("|Novel"));

    const QJsonObject item = items.at(1).toObject();
    QCOMPARE(item.value("m:handle").toString(), scene);
    QCOMPARE(item.value("m:level").toString(), QStringLiteral("Scene"));
    QCOMPARE(item.value("u:title").toString(), QStringLiteral("First"));
    QVERIFY(!item.value("m:created").toString().isEmpty());
    QVERIFY(!item.value("m:updated").toString().isEmpty());
    QVERIFY(!item.contains("m:order"));
    QCOMPARE(item.value("x:content").toArray().at(0).toObject().value("u:txt").toString(), QStringLiteral("t|Some text."));
}

/**! @brief A saved project opens with the same documents, in the same
 * order, with the same text, and nothing is modified.
 */
void TestProject::reopen()
{
    QTemporaryDir dir;
    QStringList handles;
    {
        Project project;
        QCOMPARE(project.createProject(dir.path(), "Novel"), QString());
        addScene(project, "One", "First scene.");
        addScene(project, "Two", "Second scene.");
        for (int i = 0; i < project.model()->rowCount(); ++i)
            handles.append(project.model()->data(project.model()->index(i), ProjectModel::HandleRole).toString());
        QVERIFY(project.closeProject());
    }

    Project project;
    QCOMPARE(project.openProjectAt(dir.filePath("Novel/CollettProject.collett")), QString());
    ProjectModel *model = project.model();
    QCOMPARE(model->rowCount(), 3);
    for (int i = 0; i < 3; ++i)
        QCOMPARE(model->data(model->index(i), ProjectModel::HandleRole).toString(), handles.at(i));
    QCOMPARE(model->data(model->index(2), ProjectModel::TitleRole).toString(), QStringLiteral("Two"));
    QCOMPARE(project.openDocument(handles.at(1))->toPlainText(), QStringLiteral("First scene."));
    QCOMPARE(project.openDocument(handles.at(2))->toPlainText(), QStringLiteral("Second scene."));

    QVERIFY(!project.tree()->isModified());
    QVERIFY(!project.tree()->groups().at(0)->isModified());
    for (const QString &handle : std::as_const(handles))
        QVERIFY(!project.openDocument(handle)->isModified());
}

/**! @brief A group file is only written when the group or the text of one
 * of its documents has changed.
 */
void TestProject::saveChangedOnly()
{
    QTemporaryDir dir;
    Project project;
    QCOMPARE(project.createProject(dir.path(), "Novel"), QString());
    const QString scene = addScene(project, "One", "Text.");
    QVERIFY(project.saveProject());

    const QString file = dir.filePath("Novel/content/document1.json");
    QVERIFY(QFile::remove(file));
    QVERIFY(project.saveProject());
    QVERIFY(!QFile::exists(file));

    // An edit to the text
    QTextCursor(project.openDocument(scene)).insertText("More ");
    QVERIFY(project.saveProject());
    QVERIFY(QFile::exists(file));

    // A change to the structure
    QVERIFY(QFile::remove(file));
    project.model()->setTitle(1, "Renamed");
    QVERIFY(project.saveProject());
    QCOMPARE(readJson(file).value("x:items").toArray().at(1).toObject().value("u:title").toString(), QStringLiteral("Renamed"));
}

/**! @brief Groups without a valid file name, or with one already taken,
 * get the first free one, and the structure is then modified.
 */
void TestProject::groupFileNames()
{
    auto group = [](const QString &cls, const QString &file) {
        return QJsonObject({{"m:class", cls}, {"u:name", cls}, {"m:file", file}});
    };

    Tree tree;
    tree.unpack({{"x:groups", QJsonArray({group("Novel", "document2.json"), group("Plot", "document2.json"), group("Character", "../other.json"), group("Location", "document1.json")})}});
    QStringList names;
    for (const Group *g : tree.groups())
        names.append(g->fileName());
    QCOMPARE(names, QStringList({"document2.json", "document3.json", "document4.json", "document1.json"}));
    QVERIFY(tree.isModified());

    tree.unpack({{"x:groups", QJsonArray({group("Novel", "document1.json")})}});
    QVERIFY(!tree.isModified());

    // A project without a novel group gets one, with a free file name
    tree.unpack({{"x:groups", QJsonArray({group("Plot", "document1.json")})}});
    QCOMPARE(tree.groups().size(), 2);
    QCOMPARE(tree.groups().at(0)->itemClass(), ItemClass::NovelClass);
    QCOMPARE(tree.groups().at(0)->fileName(), QStringLiteral("document2.json"));
    QVERIFY(tree.isModified());
}

/**! @brief A document with a handle that is already in use, or that is not
 * valid, is skipped when the group file is read.
 */
void TestProject::duplicateHandles()
{
    QTemporaryDir dir;
    {
        Project project;
        QCOMPARE(project.createProject(dir.path(), "Novel"), QString());
        QVERIFY(project.closeProject());
    }

    const QString file = dir.filePath("Novel/content/document1.json");
    QJsonObject content = readJson(file);
    QJsonArray items = content.value("x:items").toArray();
    QJsonObject copy = items.at(0).toObject();
    copy["u:title"] = "Copy";
    items.append(copy);
    QJsonObject invalid = copy;
    invalid["m:handle"] = "nope";
    items.append(invalid);
    content["x:items"] = items;
    writeJson(file, content);

    Project project;
    QCOMPARE(project.openProjectAt(dir.filePath("Novel/CollettProject.collett")), QString());
    QCOMPARE(project.model()->rowCount(), 1);
    QCOMPARE(project.model()->data(project.model()->index(0), ProjectModel::TitleRole).toString(), QString());
}

QTEST_MAIN(TestProject)
#include "tst_project.moc"
