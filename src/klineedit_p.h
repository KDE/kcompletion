/* This file is part of the KDE libraries

   Copyright (C) 2007 Aaron Seigo <aseigo@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License (LGPL) as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later
   version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KLINEEDIT_P_H
#define KLINEEDIT_P_H

#include "klineedit.h"

#include <QPainter>
#include <QPaintEvent>
#include <QPropertyAnimation>
#include <QIcon>
#include <QProxyStyle>

class KCompletionBox;
class LineEditUrlDropEventFilter;

class KLineEditButton : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int opacity READ opacity WRITE setOpacity)

public:
    KLineEditButton(QWidget *parent)
        : QWidget(parent),
          m_opacity(0)
    {
        m_animation = new QPropertyAnimation(this, "opacity", this);
        m_animation->setStartValue(0);
        m_animation->setEndValue(255);
        m_animation->setEasingCurve(QEasingCurve::InOutQuad);
    }

    QSize sizeHint() const
    {
        return m_pixmap.size() / m_pixmap.devicePixelRatio();
    }

    void animateVisible(bool visible)
    {
        if (visible) {
            if (m_animation->direction() == QPropertyAnimation::Forward && m_opacity == 255) {
                return;
            }

            m_animation->setDirection(QPropertyAnimation::Forward);
            m_animation->setDuration(150);
            show();
        } else {
            if (m_animation->direction() == QPropertyAnimation::Backward && m_opacity == 0) {
                return;
            }

            m_animation->setDirection(QPropertyAnimation::Backward);
            m_animation->setDuration(250);
        }

        if (style()->styleHint(QStyle::SH_Widget_Animate, 0, this)) {
            if (m_animation->state() != QPropertyAnimation::Running) {
                m_animation->start();
            }
        } else {
            setVisible(m_animation->direction() == QPropertyAnimation::Forward);
        }
    }

    void setPixmap(const QPixmap &p)
    {
        m_pixmap = p;
        m_icon = QIcon(p);
    }

    QPixmap pixmap() const
    {
        return m_pixmap;
    }

    void setAnimationsEnabled(bool animationsEnabled)
    {
        // We need to set the current time in the case that we had the clear
        // button shown, for it being painted on the paintEvent(). Otherwise
        // it wont be painted, resulting (m_opacity == 0) true,
        // and therefore a bad painting. This is needed for the case that we
        // come from a non animated widget and want it animated. (ereslibre)
        if (animationsEnabled && m_animation->direction() == QPropertyAnimation::Forward) {
            m_animation->setCurrentTime(150);
        }
    }

    int opacity() const
    {
        return m_opacity;
    }

    void setOpacity(int value)
    {
        m_opacity = value;
        update();
    }

protected:
    void paintEvent(QPaintEvent *event)
    {
        Q_UNUSED(event)

        // check pixmap
        if (m_pixmap.isNull()) {
            return;
        }

        const QSize pmSize(m_pixmap.size() / m_pixmap.devicePixelRatio());
        if (style()->styleHint(QStyle::SH_Widget_Animate, 0, this)) {

            if (m_opacity == 0) {
                if (m_animation->direction() == QPropertyAnimation::Backward) {
                    hide();
                }
                return;
            }

            if (m_opacity < 255) {
                // fade pixmap
                QPixmap pm(m_pixmap);
                QColor color(Qt::black);
                color.setAlpha(m_opacity);
                QPainter p(&pm);
                p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
                p.fillRect(pm.rect(), color);
                p.end();

                // draw pixmap on widget
                QPainter pp(this);
                pp.drawPixmap((width() - pmSize.width()) / 2,
                              (height() - pmSize.height()) / 2,
                              pm);
            } else {
                QPainter p(this);
                p.drawPixmap((width() - pmSize.width()) / 2,
                             (height() - pmSize.height()) / 2,
                             m_pixmap);
            }
        } else {
            QPainter p(this);
            p.drawPixmap((width() - pmSize.width()) / 2,
                         (height() - pmSize.height()) / 2,
                         m_pixmap);
        }
    }

protected:
    virtual bool event(QEvent *event)
    {
        if (event->type() == QEvent::EnabledChange) {
            // QIcon::pixmap will return HiDPI pixmaps that are larger than the requested size => scale pixmap size back
            m_pixmap = m_icon.pixmap(m_pixmap.size() / m_pixmap.devicePixelRatio(), isEnabled() ? QIcon::Normal : QIcon::Disabled);
        }
        return QWidget::event(event);
    }

private:
    QPropertyAnimation *m_animation;
    int m_opacity;
    QPixmap m_pixmap;
    QIcon m_icon;
};

class KLineEditStyle : public QProxyStyle
{
    Q_OBJECT
public:
    KLineEditStyle(QStyle *style)
        : QProxyStyle(),
          m_overlap(0),
          m_subStyle(style),
          m_sentinel(false)
    {
    }

    QRect subElementRect(SubElement element, const QStyleOption *option, const QWidget *widget) const;

    int m_overlap;
    QPointer<QStyle> m_subStyle;
    QString m_lastStyleClass;
    bool m_sentinel;
};

class KLineEditPrivate
{
public:
    KLineEditPrivate(KLineEdit *parent)
        : q_ptr(parent) {}

    ~KLineEditPrivate();

    void _k_textChanged(const QString &text);
    void _k_completionMenuActivated(QAction *act);
    void _k_tripleClickTimeout();  // resets possibleTripleClick
    void _k_restoreSelectionColors();
    void _k_completionBoxTextChanged(const QString &text);
    /**
     * updates the icon of the clear button on text change
     **/
    void _k_updateClearButtonIcon(const QString &);

    void adjustForReadOnly();
    void updateUserText(const QString &text);

    /**
     * Checks whether we should/should not consume a key used as a shortcut.
     * This makes it possible to handle shortcuts in the focused widget before any
     * window-global QAction is triggered.
     */
    bool overrideShortcut(const QKeyEvent *e);

    void init();

    bool copySqueezedText(bool copy) const;

    /**
     * Properly sets the squeezed text whenever the widget is
     * created or resized.
     */
    void setSqueezedText();

    /**
     * updates the geometry of the clear button on resize events
     **/
    void updateClearButton();

    QMap<KCompletion::CompletionMode, bool> disableCompletionMap;

    QColor previousHighlightColor;
    QColor previousHighlightedTextColor;

    QPalette::ColorRole bgRole;

    QString squeezedText;
    QString userText;
    QString lastStyleClass;

    KLineEditButton *clearButton;
    QPointer<KLineEditStyle> style;

    KCompletionBox *completionBox;

    LineEditUrlDropEventFilter *urlDropEventFilter;

    QAction *noCompletionAction;
    QAction *shellCompletionAction;
    QAction *autoCompletionAction;
    QAction *popupCompletionAction;
    QAction *shortAutoCompletionAction;
    QAction *popupAutoCompletionAction;
    QAction *defaultAction;

    KLineEdit *q_ptr;

    int squeezedEnd;
    int squeezedStart;

    static bool s_initialized;
    static bool s_backspacePerformsCompletion; // Configuration option

    bool userSelection: 1;
    bool autoSuggest : 1;
    bool disableRestoreSelection: 1;
    bool handleURLDrops: 1;
    bool trapReturnKeyEvents: 1;
    bool enableSqueezedText: 1;
    bool completionRunning: 1;
    bool italicizePlaceholder: 1;
    bool threeStars: 1;
    bool possibleTripleClick : 1; // set in mousePressEvent, deleted in tripleClickTimeout
    bool clickInClear: 1;
    bool wideEnoughForClear: 1;
    Q_DECLARE_PUBLIC(KLineEdit)
};


#endif

