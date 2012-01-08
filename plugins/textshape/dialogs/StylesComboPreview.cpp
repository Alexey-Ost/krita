/* This file is part of the KDE project
 * Copyright (C) 2011 Pierre Stirnweiss <pstirnweiss@googlemail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

/*
 * This class is heavily inspired by KLineEdit, so here goes credit because credit is due (from klineedit.h):
 *  This class was originally inspired by Torben Weis'
 *  fileentry.cpp for KFM II.

 *  Sven Radej <sven.radej@iname.com>
 *  Patrick Ward <PAT_WARD@HP-USA-om5.om.hp.com>
 *  Preston Brown <pbrown@kde.org>

 *  Completely re-designed:
 *  Dawit Alemayehu <adawit@kde.org>

*/

#include "StylesComboPreview.h"

#include <QModelIndex>
#include <QPainter>
#include <QPaintEvent>
#include <QFocusEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPixmap>
#include <QPushButton>
#include <QString>

#include <KIcon>
#include <KLocale>

#include <KDebug>

StylesComboPreview::StylesComboPreview(QWidget *parent) :
    QLineEdit(parent),
    m_renamingNewStyle(false),
    m_shouldAddNewStyle(false),
    m_addButton(0)
{
    init();
}

StylesComboPreview::~StylesComboPreview()
{
    delete m_addButton;
    m_addButton = 0;
}

void StylesComboPreview::init()
{
    setReadOnly(true);
    if (m_addButton) {
        return;
    }

    m_addButton = new QPushButton(this);
    m_addButton->setCursor( Qt::ArrowCursor );
    m_addButton->setIcon(KIcon("list-add"));
    m_addButton->setFlat(true);
    m_addButton->setMinimumSize(16,16);
    m_addButton->setMaximumSize(16, 16);
//TODO uncomment the following line after string freeze
//    m_addButton->setToolTip( i18nc( "@action:button Create a new style with the current properties", "Create style" ) );
    connect(m_addButton, SIGNAL(clicked()), this, SLOT(addNewStyle()));

    updateAddButton();
}

void StylesComboPreview::updateAddButton()
{
    if (!m_addButton) {
        return;
    }

    const QSize geom = size();
    const int buttonWidth = m_addButton->size().width();
    m_addButton->move(geom.width() - buttonWidth , (geom.height()-m_addButton->size().height())/2);
}

void StylesComboPreview::setAddButtonShown(bool show)
{
    m_addButton->setVisible(show);
}

QSize StylesComboPreview::availableSize() const
{
    return QSize(contentsRect().width()- m_addButton->width(), contentsRect().height()); ///TODO dynamic resizing when button shown/hidden.
}

void StylesComboPreview::setPreview(QPixmap pixmap)
{
    m_stylePreview = pixmap;
}

bool StylesComboPreview::isAddButtonShown() const
{
    return m_addButton != 0;
}

void StylesComboPreview::resizeEvent( QResizeEvent * ev )
{
    QLineEdit::resizeEvent(ev);
    emit resized();
    updateAddButton();
}

void StylesComboPreview::keyPressEvent( QKeyEvent *e )
{
    if (m_shouldAddNewStyle && e->key() == Qt::Key_Escape) {
        m_renamingNewStyle = false;
        m_shouldAddNewStyle = false;
        setReadOnly(true);
        setText(QString());
        e->accept();
    }
    else if (m_shouldAddNewStyle && (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)) {
        m_renamingNewStyle = false;
        m_shouldAddNewStyle = false;
        emit newStyleRequested(text());
        setReadOnly(true);
        setText(QString());
        e->accept();
    }
    else {
        QLineEdit::keyPressEvent(e);
    }
}

void StylesComboPreview::focusOutEvent(QFocusEvent *e)
{
    if (e->reason() != Qt::ActiveWindowFocusReason && e->reason() != Qt::PopupFocusReason) {
        if (m_shouldAddNewStyle) {
            m_renamingNewStyle = false;
            m_shouldAddNewStyle = false;
            emit newStyleRequested(text());
            setReadOnly(true);
            setText(QString());
            e->accept();
        }
        setReadOnly(true);
        m_renamingNewStyle = false;
        setText(QString());
    }
    else {
        QLineEdit::focusOutEvent(e);
    }
}

void StylesComboPreview::paintEvent( QPaintEvent *ev )
{
    if (!m_renamingNewStyle) {
        QLineEdit::paintEvent(ev);
        QPainter p(this);
        p.setClipRect(ev->rect());
        p.drawPixmap(contentsRect().topLeft(), m_stylePreview);
    }
    else {
        QLineEdit::paintEvent(ev);
    }
}

void StylesComboPreview::addNewStyle()
{
    m_renamingNewStyle = true;
    m_shouldAddNewStyle = true;
    setText(i18n("New style"));
    selectAll();
    setReadOnly(false);
    this->setFocus();
}

#include "StylesComboPreview.moc"
