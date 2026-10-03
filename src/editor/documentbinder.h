/*
** Collett - Document Binder Class
** ===============================
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
#include "document.h"
#include "highlighter.h"
#include "project.h"

#include <QColor>
#include <QFont>
#include <QObject>
#include <QPointer>
#include <QQuickItem>
#include <QString>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QVariantMap>
#include <QtQml/qqmlregistration.h>

namespace Collett {

class DocumentBinder : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QQuickItem *target READ target WRITE setTarget NOTIFY targetChanged)
    Q_PROPERTY(Collett::Project *project READ project WRITE setProject NOTIFY projectChanged)
    Q_PROPERTY(QString handle READ handle WRITE setHandle NOTIFY handleChanged)
    Q_PROPERTY(QFont textFont READ textFont NOTIFY textFontChanged)
    Q_PROPERTY(QFont headingFont READ headingFont NOTIFY textFontChanged)
    Q_PROPERTY(QColor spellErrorColor READ spellErrorColor WRITE setSpellErrorColor NOTIFY spellErrorColorChanged)
    Q_PROPERTY(bool bold READ bold WRITE setBold NOTIFY formatChanged)
    Q_PROPERTY(bool italic READ italic WRITE setItalic NOTIFY formatChanged)
    Q_PROPERTY(bool underline READ underline WRITE setUnderline NOTIFY formatChanged)
    Q_PROPERTY(bool strikeOut READ strikeOut WRITE setStrikeOut NOTIFY formatChanged)
    Q_PROPERTY(bool superscript READ superscript WRITE setSuperscript NOTIFY formatChanged)
    Q_PROPERTY(bool subscript READ subscript WRITE setSubscript NOTIFY formatChanged)
    Q_PROPERTY(int alignment READ alignment WRITE setAlignment NOTIFY formatChanged)

public:
    explicit DocumentBinder(QObject *parent = nullptr);
    ~DocumentBinder();

    // Getters
    QQuickItem *target() const { return m_target; };
    Project *project() const { return m_project; };
    QString handle() const { return m_handle; };
    QFont textFont() const;
    QFont headingFont() const;
    QColor spellErrorColor() const { return m_spellErrorColor; };
    bool bold() const { return m_format.fontWeight() >= QFont::Bold; };
    bool italic() const { return m_format.fontItalic(); };
    bool underline() const { return m_format.fontUnderline(); };
    bool strikeOut() const { return m_format.fontStrikeOut(); };
    bool superscript() const { return m_format.verticalAlignment() == QTextCharFormat::AlignSuperScript; };
    bool subscript() const { return m_format.verticalAlignment() == QTextCharFormat::AlignSubScript; };
    int alignment() const { return m_alignment; };

    // Setters
    void setTarget(QQuickItem *target);
    void setProject(Project *project);
    void setHandle(const QString &handle);
    void setSpellErrorColor(const QColor &color);
    void setBold(bool bold);
    void setItalic(bool italic);
    void setUnderline(bool underline);
    void setStrikeOut(bool strikeOut);
    void setSuperscript(bool superscript);
    void setSubscript(bool subscript);
    void setAlignment(int alignment);

    // Formatting
    Q_INVOKABLE void indent();
    Q_INVOKABLE void outdent();

    // Spell Checking
    Q_INVOKABLE QVariantMap misspelledWordAt(int position) const;
    Q_INVOKABLE void replaceText(int start, int end, const QString &text);
    Q_INVOKABLE bool addWord(const QString &word);

signals:
    void targetChanged();
    void projectChanged();
    void handleChanged();
    void spellErrorColorChanged();
    void textFontChanged();
    void formatChanged();

private:
    QPointer<QQuickItem> m_target;
    QPointer<Project> m_project;
    QString m_handle = "";
    QPointer<Document> m_document;
    Highlighter *m_highlighter = nullptr;
    QTextDocument *m_placeholder = nullptr;
    QColor m_spellErrorColor = Qt::red;

    // The format at the cursor, and a format waiting for text to be typed
    // at a cursor that is not on a word
    QTextCharFormat m_format;
    int m_alignment = Qt::AlignLeft;
    QTextCharFormat m_pending;
    int m_pendingPosition = -1;
    bool m_applying = false;

    void openDocument();
    void bindDocument(Document *document);
    void releaseDocument(Document *document);
    QTextCursor targetCursor() const;
    void mergeFormat(const QTextCharFormat &format);
    void changeIndent(int step);

private slots:
    void updateFormat();
    void applyPending(int position, int removed, int added);
};
} // namespace Collett
