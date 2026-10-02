/*
** Collett - Editor Runnables
** ==========================
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

#pragma once

#include "collett.h"
#include "counting.h"
#include "spellchecker.h"
#include "textblock.h"

#include <QList>
#include <QMetaType>
#include <QRunnable>
#include <QString>

namespace Collett {

/**! @brief Dispatches background word count jobs, one at a time.
 *
 * A job counts a snapshot of the document blocks in the thread pool, and
 * the result comes back on the GUI thread through the countsReady signal.
 * Only one job runs at a time. While it is in flight, isBusy() is true and
 * count() refuses new jobs, so the caller must retry later.
 */
class WordCounterDispatcher : public QObject
{
    Q_OBJECT

public:
    explicit WordCounterDispatcher(QObject *parent = nullptr);
    ~WordCounterDispatcher();

    // Methods
    bool count(const CountBlockList &blocks);

    // Getters
    bool isBusy() const { return m_busy; };

signals:
    void countsReady(const Collett::TextCounts &counts);

private slots:
    void onFinished(const Collett::TextCounts &counts);

private:
    bool m_busy = false;
};

/**! @brief The off-GUI thread word counter.
 *
 * A one-shot runnable that counts a batch of block snapshots in the thread
 * pool, and emits the result when done.
 */
class BackgroundWordCounter : public QObject, public QRunnable
{
    Q_OBJECT

public:
    explicit BackgroundWordCounter(const CountBlockList &blocks);

    void run() override;

signals:
    void finished(const Collett::TextCounts &counts);

private:
    CountBlockList m_blocks;
};

/**! @brief A block text snapshot sent to the background text check.
 *
 * The index identifies the block within the job, so the editor can map the
 * result back without the worker touching any document objects.
 */
struct TextCheckPayload
{
    int index = 0;
    QString text;
};

/**! @brief The errors found in one block by the background text check.
 */
struct TextCheckResult
{
    int index = 0;
    TextCheckList spellErrors;
    TextCheckList formatErrors;
};

/**! @brief Dispatches background text check jobs to the thread pool.
 *
 * Each job is a one-shot runnable. Its result comes back on the GUI thread
 * through the resultsReady signal, tagged with the job id it was given.
 */
class TextCheckDispatcher : public QObject
{
    Q_OBJECT

public:
    explicit TextCheckDispatcher(QObject *parent = nullptr);
    ~TextCheckDispatcher();

    // Methods
    void dispatch(int jobId, const QList<TextCheckPayload> &payload, bool checkSpell, bool checkFormat);

    // Setters
    void setSpellChecker(SpellChecker *spell) { m_spell = spell; };

signals:
    void resultsReady(int jobId, const QList<Collett::TextCheckResult> &results);

private:
    SpellChecker *m_spell = nullptr;
};

/**! @brief The off-GUI thread text checker.
 *
 * Spell and format checks a batch of block snapshots in the thread pool, and
 * emits the results when done. The spell checker is thread safe, and the
 * checks only work on the snapshot strings, so no document access happens
 * off the GUI thread.
 */
class BackgroundTextCheck : public QObject, public QRunnable
{
    Q_OBJECT

public:
    BackgroundTextCheck(
        int jobId,
        const QList<TextCheckPayload> &payload,
        SpellChecker *spell,
        bool checkSpell,
        bool checkFormat
    );

    void run() override;

signals:
    void finished(int jobId, const QList<Collett::TextCheckResult> &results);

private:
    int m_jobId;
    QList<TextCheckPayload> m_payload;
    SpellChecker *m_spell;
    bool m_checkSpell;
    bool m_checkFormat;
};

} // namespace Collett

Q_DECLARE_METATYPE(Collett::TextCheckResult)
