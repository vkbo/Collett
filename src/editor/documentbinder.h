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
#include <QHash>
#include <QObject>
#include <QPointer>
#include <QRectF>
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
    Q_PROPERTY(QFont textFont READ textFont NOTIFY textFontChanged)
    Q_PROPERTY(QFont headingFont READ headingFont NOTIFY textFontChanged)
    Q_PROPERTY(QColor spellErrorColor READ spellErrorColor WRITE setSpellErrorColor NOTIFY spellErrorColorChanged)
    Q_PROPERTY(QColor formatErrorColor READ formatErrorColor WRITE setFormatErrorColor NOTIFY formatErrorColorChanged)
    Q_PROPERTY(bool bold READ bold WRITE setBold NOTIFY formatChanged)
    Q_PROPERTY(bool italic READ italic WRITE setItalic NOTIFY formatChanged)
    Q_PROPERTY(bool underline READ underline WRITE setUnderline NOTIFY formatChanged)
    Q_PROPERTY(bool strikeOut READ strikeOut WRITE setStrikeOut NOTIFY formatChanged)
    Q_PROPERTY(bool superscript READ superscript WRITE setSuperscript NOTIFY formatChanged)
    Q_PROPERTY(bool subscript READ subscript WRITE setSubscript NOTIFY formatChanged)
    Q_PROPERTY(int alignment READ alignment WRITE setAlignment NOTIFY formatChanged)
    Q_PROPERTY(int headingLevel READ headingLevel WRITE setHeadingLevel NOTIFY formatChanged)
    Q_PROPERTY(QString currentHandle READ currentHandle NOTIFY cursorItemChanged)
    Q_PROPERTY(bool inTitle READ inTitle NOTIFY cursorItemChanged)
    Q_PROPERTY(int layoutRevision READ layoutRevision NOTIFY layoutChanged)
    Q_PROPERTY(qreal topSpace READ topSpace NOTIFY layoutChanged)

public:
    explicit DocumentBinder(QObject *parent = nullptr);
    ~DocumentBinder();

    // Getters
    QQuickItem *target() const { return m_target; };
    Project *project() const { return m_project; };
    QFont textFont() const;
    QFont headingFont() const;
    QColor spellErrorColor() const { return m_spellErrorColor; };
    QColor formatErrorColor() const { return m_formatErrorColor; };
    bool bold() const { return m_format.fontWeight() >= QFont::Bold; };
    bool italic() const { return m_format.fontItalic(); };
    bool underline() const { return m_format.fontUnderline(); };
    bool strikeOut() const { return m_format.fontStrikeOut(); };
    bool superscript() const { return m_format.verticalAlignment() == QTextCharFormat::AlignSuperScript; };
    bool subscript() const { return m_format.verticalAlignment() == QTextCharFormat::AlignSubScript; };
    int alignment() const { return m_alignment; };
    int headingLevel() const { return m_headingLevel; };
    QString currentHandle() const { return m_currentHandle; };
    bool inTitle() const { return m_inTitle; };
    int layoutRevision() const { return m_layoutRevision; };
    qreal topSpace() const;

    // Setters
    void setTarget(QQuickItem *target);
    void setProject(Project *project);
    void setSpellErrorColor(const QColor &color);
    void setFormatErrorColor(const QColor &color);
    void setBold(bool bold);
    void setItalic(bool italic);
    void setUnderline(bool underline);
    void setStrikeOut(bool strikeOut);
    void setSuperscript(bool superscript);
    void setSubscript(bool subscript);
    void setAlignment(int alignment);
    void setHeadingLevel(int level);

    // Formatting
    Q_INVOKABLE void indent();
    Q_INVOKABLE void outdent();
    Q_INVOKABLE bool newParagraph();
    Q_INVOKABLE bool addFirstLineIndent();
    Q_INVOKABLE bool removeFirstLineIndent();

    // Keys
    Q_INVOKABLE bool keyEnter(int modifiers);
    Q_INVOKABLE bool keyBackspace();
    Q_INVOKABLE bool keyDelete();
    Q_INVOKABLE bool keyTab();

    // Documents
    Q_INVOKABLE QRectF titleRect(const QString &handle) const;
    Q_INVOKABLE QRectF emptyBodyRect(const QString &handle) const;
    Q_INVOKABLE qreal dividerHeight(int level, bool hardBreak) const;
    Q_INVOKABLE qreal labelHeight() const;
    Q_INVOKABLE void enterItem(const QString &handle);
    Q_INVOKABLE void enterTitle(const QString &handle);
    Q_INVOKABLE void enterText(const QString &handle, int position);
    Q_INVOKABLE int itemPosition(const QString &handle) const;
    Q_INVOKABLE bool deleteItem(const QString &handle);

    // Spell Checking
    Q_INVOKABLE QVariantMap misspelledWordAt(int position) const;
    Q_INVOKABLE void replaceText(int start, int end, const QString &text);
    Q_INVOKABLE bool addWord(const QString &word);

signals:
    void targetChanged();
    void projectChanged();
    void cursorItemChanged();
    void layoutChanged();
    void spellErrorColorChanged();
    void formatErrorColorChanged();
    void textFontChanged();
    void formatChanged();

private:
    QPointer<QQuickItem> m_target;
    QPointer<Project> m_project;
    QPointer<Document> m_document;
    Highlighter *m_highlighter = nullptr;
    QTextDocument *m_placeholder = nullptr;
    QColor m_spellErrorColor = Qt::red;
    QColor m_formatErrorColor = QColor(255, 165, 0);

    // The format at the cursor, and a format waiting for text to be typed
    // at a cursor that is not on a word
    QTextCharFormat m_format;
    int m_alignment = Qt::AlignLeft;
    int m_headingLevel = 0;
    QTextCharFormat m_pending;
    int m_pendingPosition = -1;
    bool m_applying = false;

    // The document at the cursor, and the one made by the last split, which
    // Ctrl+Enter in its title changes the type of
    QString m_currentHandle;
    bool m_inTitle = false;
    QString m_splitHandle;

    // The block number of each title, and a count of layout changes, for
    // the editor to place what it draws around the titles
    QHash<QString, int> m_titles;
    int m_layoutRevision = 0;
    bool m_layoutPending = false;

    void openDocument();
    void bindDocument(Document *document);
    void releaseDocument(Document *document);
    QTextCursor targetCursor() const;
    void mergeFormat(const QTextCharFormat &format);
    void changeIndent(int step);
    void setCursor(int position);
    bool mergeUp(const QTextBlock &title);
    void upgradeSplit();
    QTextBlock titleBlock(const QString &handle) const;

private slots:
    void updateFormat();
    void updateCursor();
    void applyPending(int position, int removed, int added);
    void updateTitles();
    void scheduleLayoutChanged();
};
} // namespace Collett
