/*
** Collett - Spell Check Tests
** ===========================
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

#include "nuspellengine.h"
#include "spellchecker.h"
#include "spellengine.h"
#include "storage.h"
#include "userdictionary.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>

using namespace Collett;

// The tests use a tiny dictionary shipped in tests/_data, so they do not
// depend on which system dictionaries happen to be installed.
static const QString DATA_DIR = QStringLiteral(TEST_DATA_DIR);
static const QString TEST_TAG = QStringLiteral("xx_TEST");

class TestSpellCheck : public QObject
{
    Q_OBJECT

private slots:
    void nullEngine();
    void nuspellLoadDictionary();
    void nuspellLoadLanguage();
    void nuspellLoadFailure();
    void nuspellAvailableDictionaries();
    void userDictionaryWords();
    void userDictionaryStems();
    void userDictionaryStorage();
    void spellCheckerFallback();
    void spellCheckerCheckWord();
    void spellCheckerAddWord();
    void spellCheckerAddStem();
    void spellCheckerListDictionaries();
    void languageName();
};

/**! @brief The null engine accepts every word and suggests nothing.
 */
void TestSpellCheck::nullEngine()
{
    NullSpellEngine engine;
    QVERIFY(!engine.isLoaded());
    QVERIFY(engine.language().isEmpty());
    QVERIFY(engine.spell("hello"));
    QVERIFY(engine.spell("xyzzy"));
    QVERIFY(engine.suggest("helo").isEmpty());
}

/**! @brief Load the test dictionary directly from its .aff file.
 */
void TestSpellCheck::nuspellLoadDictionary()
{
    NuspellEngine engine;
    QVERIFY(!engine.isLoaded());
    QVERIFY(engine.loadDictionary(DATA_DIR + "/xx_TEST.aff"));
    QVERIFY(engine.isLoaded());
    QCOMPARE(engine.language(), TEST_TAG);
    QCOMPARE(engine.provider(), QStringLiteral("Nuspell"));

    QVERIFY(engine.spell("hello"));
    QVERIFY(engine.spell("Hello"));
    QVERIFY(engine.spell("Frodo"));
    QVERIFY(engine.spell(QString::fromUtf8("blåbær")));
    QVERIFY(!engine.spell("frodo"));
    QVERIFY(!engine.spell("helo"));
    QVERIFY(!engine.spell("Frodon"));

    QVERIFY(engine.suggest("helo").contains("hello"));

    engine.unload();
    QVERIFY(!engine.isLoaded());
    QVERIFY(engine.language().isEmpty());
    QVERIFY(engine.spell("helo"));
}

/**! @brief Load the test dictionary by language tag via an extra search dir.
 */
void TestSpellCheck::nuspellLoadLanguage()
{
    NuspellEngine engine;
    QVERIFY(engine.loadLanguage(TEST_TAG, {DATA_DIR}));
    QVERIFY(engine.isLoaded());
    QCOMPARE(engine.language(), TEST_TAG);
    QVERIFY(engine.spell("world"));
    QVERIFY(!engine.spell("worlds"));
}

/**! @brief Failed loads leave the engine unloaded and accepting everything.
 */
void TestSpellCheck::nuspellLoadFailure()
{
    NuspellEngine engine;
    QVERIFY(!engine.loadDictionary(DATA_DIR + "/zz_NOPE.aff"));
    QVERIFY(!engine.isLoaded());
    QVERIFY(engine.spell("helo"));
    QVERIFY(engine.suggest("helo").isEmpty());

    QVERIFY(!engine.loadLanguage("zz_NOPE", {DATA_DIR}));
    QVERIFY(!engine.isLoaded());
    QVERIFY(!engine.loadLanguage("", {DATA_DIR}));
    QVERIFY(!engine.isLoaded());

    // A failed load after a successful one must also unload
    QVERIFY(engine.loadLanguage(TEST_TAG, {DATA_DIR}));
    QVERIFY(!engine.loadLanguage("zz_NOPE", {DATA_DIR}));
    QVERIFY(!engine.isLoaded());
    QVERIFY(engine.language().isEmpty());
}

/**! @brief Dictionary discovery finds the test dictionary in the extra dir.
 */
void TestSpellCheck::nuspellAvailableDictionaries()
{
    const QList<DictionaryInfo> dicts = NuspellEngine::availableDictionaries({DATA_DIR});
    bool found = false;
    for (const DictionaryInfo &dict : dicts) {
        if (dict.tag == TEST_TAG) {
            QCOMPARE(dict.path, DATA_DIR + "/xx_TEST.aff");
            found = true;
        }
    }
    QVERIFY(found);

    // The list is sorted and has no duplicate tags
    QStringList tags;
    for (const DictionaryInfo &dict : dicts) {
        tags.append(dict.tag);
    }
    QStringList sorted = tags;
    std::sort(sorted.begin(), sorted.end());
    QCOMPARE(tags, sorted);
    QCOMPARE(QSet<QString>(tags.cbegin(), tags.cend()).size(), tags.size());
}

/**! @brief Plain words are added once, trimmed, and matched exactly.
 */
void TestSpellCheck::userDictionaryWords()
{
    UserDictionary dict;
    QCOMPARE(dict.count(), 0);
    QVERIFY(!dict.contains("Gandalf"));

    QVERIFY(dict.add("Gandalf"));
    QVERIFY(!dict.add("Gandalf"));
    QVERIFY(dict.add("  Bilbo  "));
    QVERIFY(!dict.add(""));
    QVERIFY(!dict.add("   "));
    QCOMPARE(dict.count(), 2);

    QVERIFY(dict.contains("Gandalf"));
    QVERIFY(dict.contains("Bilbo"));
    QVERIFY(!dict.contains("gandalf"));
    QVERIFY(!dict.contains("Gandalfs"));

    QCOMPARE(dict.entries(), QStringList({"Bilbo", "Gandalf"}));

    QVERIFY(dict.remove("Gandalf"));
    QVERIFY(!dict.remove("Gandalf"));
    QVERIFY(!dict.contains("Gandalf"));
    QCOMPARE(dict.count(), 1);

    dict.clear();
    QCOMPARE(dict.count(), 0);
}

/**! @brief Stem entries accept any longer form.
 */
void TestSpellCheck::userDictionaryStems()
{
    UserDictionary dict;
    QVERIFY(UserDictionary::isStem("Frodo*"));
    QVERIFY(!UserDictionary::isStem("Frodo"));

    QVERIFY(dict.add("Frodo*"));
    QVERIFY(!dict.add("Frodo*"));
    QVERIFY(!dict.add("*"));
    QVERIFY(!dict.add(" * "));
    QCOMPARE(dict.count(), 1);

    QVERIFY(dict.contains("Frodo"));
    QVERIFY(dict.contains("Frodon"));
    QVERIFY(dict.contains("Frodolle"));
    QVERIFY(!dict.contains("Frod"));
    QVERIFY(!dict.contains("frodon"));

    // A stem and a plain word with the same text are distinct entries
    QVERIFY(dict.add("Frodo"));
    QCOMPARE(dict.entries(), QStringList({"Frodo", "Frodo*"}));

    QVERIFY(dict.remove("Frodo*"));
    QVERIFY(dict.contains("Frodo"));
    QVERIFY(!dict.contains("Frodon"));
}

/**! @brief The word list round-trips through the project storage.
 */
void TestSpellCheck::userDictionaryStorage()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    Storage store(tmpDir.filePath("Test.collett"));
    QVERIFY(store.isValid());

    // No storage, and no file yet
    UserDictionary noStore;
    QVERIFY(!noStore.load());
    QVERIFY(!noStore.save());

    UserDictionary dict(&store);
    QVERIFY(dict.load());
    QCOMPARE(dict.count(), 0);

    QVERIFY(dict.add("Gandalf"));
    QVERIFY(dict.add("Frodo*"));
    QVERIFY(dict.save());
    QVERIFY(QFileInfo::exists(tmpDir.filePath("project/dictionary.json")));

    QJsonObject data;
    QVERIFY(store.readDictionary(data));
    QCOMPARE(data.value("c:format").toString(), QStringLiteral("CollettUserDictionary"));
    QCOMPARE(data.value("c:words").toArray(), QJsonArray({"Frodo*", "Gandalf"}));

    UserDictionary loaded(&store);
    QVERIFY(loaded.load());
    QCOMPARE(loaded.count(), 2);
    QVERIFY(loaded.contains("Gandalf"));
    QVERIFY(loaded.contains("Frodon"));
    QVERIFY(!loaded.contains("Bilbo"));
}

/**! @brief Without a loadable language the checker accepts everything.
 */
void TestSpellCheck::spellCheckerFallback()
{
    SpellChecker checker;
    QSignalSpy spy(&checker, &SpellChecker::languageChanged);

    QVERIFY(!checker.isLoaded());
    QVERIFY(checker.language().isEmpty());
    QVERIFY(checker.provider().isEmpty());
    QVERIFY(checker.checkWord("xyzzy"));
    QVERIFY(checker.suggestWords("xyzzy").isEmpty());

    checker.setDictionaryPaths({DATA_DIR});
    checker.setLanguage("zz_NOPE");
    QCOMPARE(spy.count(), 1);
    QVERIFY(!checker.isLoaded());
    QCOMPARE(checker.requestedLanguage(), QStringLiteral("zz_NOPE"));
    QVERIFY(checker.language().isEmpty());
    QVERIFY(checker.checkWord("xyzzy"));

    checker.setLanguage(TEST_TAG);
    QCOMPARE(spy.count(), 2);
    QVERIFY(checker.isLoaded());
    QVERIFY(!checker.checkWord("xyzzy"));

    checker.setLanguage("");
    QCOMPARE(spy.count(), 3);
    QVERIFY(!checker.isLoaded());
    QVERIFY(checker.checkWord("xyzzy"));
}

/**! @brief Words are checked against the engine and results are cached.
 */
void TestSpellCheck::spellCheckerCheckWord()
{
    SpellChecker checker;
    checker.setDictionaryPaths({DATA_DIR});
    checker.setLanguage(TEST_TAG);
    QVERIFY(checker.isLoaded());
    QCOMPARE(checker.language(), TEST_TAG);
    QCOMPARE(checker.provider(), QStringLiteral("Nuspell"));

    QVERIFY(checker.checkWord(""));
    QVERIFY(checker.checkWord("hello"));
    QVERIFY(checker.checkWord("Collett"));
    QVERIFY(!checker.checkWord("helo"));
    QVERIFY(!checker.checkWord("helo")); // Cached
    QVERIFY(checker.suggestWords("helo").contains("hello"));

    // Changing language drops the cache
    checker.setLanguage("");
    QVERIFY(checker.checkWord("helo"));
}

/**! @brief Added words are accepted at once, and saved to the project.
 */
void TestSpellCheck::spellCheckerAddWord()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    Storage store(tmpDir.filePath("Test.collett"));
    QVERIFY(store.isValid());

    SpellChecker checker;
    QSignalSpy spy(&checker, &SpellChecker::userDictionaryChanged);
    checker.setDictionaryPaths({DATA_DIR});
    checker.setLanguage(TEST_TAG);
    checker.setStorage(&store);
    QCOMPARE(spy.count(), 1);

    QVERIFY(!checker.checkWord("Gandalf"));
    QVERIFY(checker.addWord("Gandalf"));
    QCOMPARE(spy.count(), 2);
    QVERIFY(checker.checkWord("Gandalf")); // Cache entry updated
    QVERIFY(!checker.addWord("Gandalf"));
    QVERIFY(!checker.addWord("  "));
    QCOMPARE(spy.count(), 2);

    // Not saved when asked not to
    QVERIFY(checker.addWord("Bilbo", false));
    QCOMPARE(spy.count(), 3);

    // Only the saved word is on disk
    QJsonObject data;
    QVERIFY(store.readDictionary(data));
    QCOMPARE(data.value("c:words").toArray(), QJsonArray({"Gandalf"}));

    // A fresh checker on the same storage picks the saved word up
    SpellChecker other;
    other.setDictionaryPaths({DATA_DIR});
    other.setLanguage(TEST_TAG);
    other.setStorage(&store);
    QVERIFY(other.checkWord("Gandalf"));
    QVERIFY(!other.checkWord("Bilbo"));
    QCOMPARE(other.userDictionary()->count(), 1);
}

/**! @brief Adding a stem invalidates earlier negative results.
 */
void TestSpellCheck::spellCheckerAddStem()
{
    SpellChecker checker;
    checker.setDictionaryPaths({DATA_DIR});
    checker.setLanguage(TEST_TAG);

    QVERIFY(checker.checkWord("Frodo"));
    QVERIFY(!checker.checkWord("Frodon"));
    QVERIFY(!checker.checkWord("Frodolle"));

    QVERIFY(checker.addWord("Frodo*", false));
    QVERIFY(checker.checkWord("Frodon"));
    QVERIFY(checker.checkWord("Frodolle"));
    QVERIFY(!checker.checkWord("Frod"));
}

/**! @brief Listing dictionaries gives tags with display names, sorted by name.
 */
void TestSpellCheck::spellCheckerListDictionaries()
{
    SpellChecker checker;
    checker.setDictionaryPaths({DATA_DIR});
    const QList<SpellChecker::Language> languages = checker.listDictionaries();

    bool found = false;
    QStringList names;
    for (const SpellChecker::Language &lang : languages) {
        names.append(lang.name);
        if (lang.tag == TEST_TAG) {
            QCOMPARE(lang.name, TEST_TAG); // Unknown to QLocale, so the tag is used
            found = true;
        }
    }
    QVERIFY(found);

    QStringList sorted = names;
    std::sort(sorted.begin(), sorted.end());
    QCOMPARE(names, sorted);
}

/**! @brief Language tags are turned into readable names when Qt knows them.
 */
void TestSpellCheck::languageName()
{
    QCOMPARE(SpellChecker::languageName("en_GB"), QStringLiteral("English (United Kingdom) [en_GB]"));
    QCOMPARE(SpellChecker::languageName("nb_NO"), QStringLiteral("Norwegian Bokmål (Norway) [nb_NO]"));
    QCOMPARE(SpellChecker::languageName("xx_TEST"), QStringLiteral("xx_TEST"));
    QCOMPARE(SpellChecker::languageName(""), QString());
}

QTEST_MAIN(TestSpellCheck)
#include "tst_spellcheck.moc"
