/*
** Collett - Font Tools
** ====================
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

#include "fonts.h"
#include "tools.h"

#include <QFontDatabase>
#include <QFontInfo>

namespace Collett {

/**! @brief The installed font families, or only those with a fixed pitch.
 */
QStringList Fonts::families(bool fixedPitch)
{
    QStringList families = QFontDatabase::families();
    if (fixedPitch) {
        families.removeIf([](const QString &family) { return !QFontDatabase::isFixedPitch(family); });
    }
    return families;
}

/**! @brief The styles of a font family, like "Regular" and "Bold Italic".
 */
QStringList Fonts::styles(const QString &family)
{
    return QFontDatabase::styles(family);
}

/**! @brief The style a font resolves to on this system.
 */
QString Fonts::styleOf(const QFont &font)
{
    return QFontInfo(font).styleName();
}

/**! @brief The standard point sizes to pick from.
 */
QList<int> Fonts::sizes()
{
    return QFontDatabase::standardSizes();
}

/**! @brief A font from a family, a style name and a point size.
 */
QFont Fonts::font(const QString &family, const QString &style, qreal pointSize)
{
    QFont font(family);
    font.setStyleName(style);
    font.setPointSizeF(pointSize);
    return font;
}

/**! @brief The parts of a font that the controls of a window inherit.
 *
 * Only the family, size, weight and slant are set, so each control keeps
 * the rest of its own font, and text that sets its own weight keeps it. The
 * weight and slant come from the font's style.
 */
QFont Fonts::interfaceFont(const QFont &font)
{
    QFont base;
    base.setFamilies({font.family()});
    base.setPointSizeF(font.pointSizeF());
    const QString style = styleOf(font);
    const int weight = QFontDatabase::weight(font.family(), style);
    base.setWeight(weight > 0 ? QFont::Weight(weight) : font.weight());
    base.setItalic(QFontDatabase::italic(font.family(), style) || font.italic());
    return base;
}

/**! @brief A copy of a font with its size scaled.
 */
QFont Fonts::scaled(const QFont &font, qreal factor)
{
    QFont copy = font;
    copy.setPointSizeF(font.pointSizeF() * factor);
    return copy;
}

/**! @brief A short description of a font, like "12 pt Noto Serif".
 */
QString Fonts::describe(const QFont &font)
{
    return FontUtils::describeFont(font);
}

} // namespace Collett
