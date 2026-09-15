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

#include "runnables.h"

#include <QThreadPool>

namespace Collett {

// WordCounterDispatcher
// =====================

WordCounterDispatcher::WordCounterDispatcher(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<TextCounts>();
}

WordCounterDispatcher::~WordCounterDispatcher()
{
    qDebug() << "Destructor: WordCounterDispatcher";
}

/**! @brief Queue a count job in the global thread pool.
 *
 * The runnable's finished signal is delivered to onFinished with a queued
 * connection, so the result arrives on the GUI thread. If the dispatcher is
 * gone by then, Qt drops the delivery.
 *
 * @return Returns false if a job is already in flight and nothing was queued.
 */
bool WordCounterDispatcher::count(const CountBlockList &blocks)
{
    if (m_busy) {
        return false;
    }
    m_busy = true;
    BackgroundWordCounter *job = new BackgroundWordCounter(blocks);
    connect(job, &BackgroundWordCounter::finished, this, &WordCounterDispatcher::onFinished, Qt::QueuedConnection);
    QThreadPool::globalInstance()->start(job);
    return true;
}

/**! @brief Clear the busy state before forwarding the result.
 *
 * This way the receiver is free to dispatch another count right away.
 */
void WordCounterDispatcher::onFinished(const TextCounts &counts)
{
    m_busy = false;
    emit countsReady(counts);
}

// BackgroundWordCounter
// =====================

BackgroundWordCounter::BackgroundWordCounter(const CountBlockList &blocks) : m_blocks(blocks)
{
    this->setAutoDelete(true);
}

/**! @brief Run the standard counter on the snapshot and emit the result.
 */
void BackgroundWordCounter::run()
{
    emit finished(TextCounter::standardCount(m_blocks));
}

// TextCheckDispatcher
// ===================

TextCheckDispatcher::TextCheckDispatcher(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<QList<TextCheckResult>>();
}

TextCheckDispatcher::~TextCheckDispatcher()
{
    qDebug() << "Destructor: TextCheckDispatcher";
}

/**! @brief Queue a text check job in the global thread pool.
 *
 * The runnable's finished signal is forwarded to resultsReady with a queued
 * connection, so the results arrive on the GUI thread. If the dispatcher is
 * gone by then, Qt drops the delivery.
 */
void TextCheckDispatcher::dispatch(int jobId, const QList<TextCheckPayload> &payload, bool checkSpell, bool checkFormat)
{
    BackgroundTextCheck *job = new BackgroundTextCheck(jobId, payload, m_spell, checkSpell, checkFormat);
    connect(job, &BackgroundTextCheck::finished, this, &TextCheckDispatcher::resultsReady, Qt::QueuedConnection);
    QThreadPool::globalInstance()->start(job);
}

// BackgroundTextCheck
// ===================

BackgroundTextCheck::BackgroundTextCheck(
    int jobId,
    const QList<TextCheckPayload> &payload,
    SpellChecker *spell,
    bool checkSpell,
    bool checkFormat
)
    : m_jobId(jobId), m_payload(payload), m_spell(spell), m_checkSpell(checkSpell), m_checkFormat(checkFormat)
{
    this->setAutoDelete(true);
}

/**! @brief Run the checks on each snapshot and emit the results.
 */
void BackgroundTextCheck::run()
{
    QList<TextCheckResult> results;
    results.reserve(m_payload.size());
    for (const TextCheckPayload &item : std::as_const(m_payload)) {
        TextCheckResult result;
        result.index = item.index;
        if (m_checkSpell) {
            result.spellErrors = spellCheckText(item.text, m_spell);
        }
        if (m_checkFormat) {
            result.formatErrors = formatCheckText(item.text);
        }
        results.append(result);
    }
    emit finished(m_jobId, results);
}

} // namespace Collett
