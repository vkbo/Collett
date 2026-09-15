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
#include "spellchecker.h"
#include "textblock.h"

#include <QList>
#include <QMetaType>
#include <QRunnable>
#include <QString>

namespace Collett {

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
