/*
 *  Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_clipboard_brush_widget.h"

#include <QLabel>
#include <QImage>
#include <QPixmap>
#include <QShowEvent>
#include <QPushButton>
#include <QDialogButtonBox>

#include <KoResourcePaths.h>

#include <kis_debug.h>
#include "kis_image.h"
#include "kis_clipboard.h"
#include "kis_paint_device.h"
#include "kis_gbr_brush.h"
#include "KisBrushServerProvider.h"
#include "kis_icon.h"

KisClipboardBrushWidget::KisClipboardBrushWidget(QWidget *parent, const QString &caption, KisImageWSP /*image*/)
    : KisWdgClipboardBrush(parent)
{
    setWindowTitle(caption);
    preview->setScaledContents(false);
    preview->setFixedSize(preview->size());
    preview->setStyleSheet("border: 2px solid #222; border-radius: 4px; padding: 5px; font: normal 10px;");


    m_rServer = KisBrushServerProvider::instance()->brushServer();

    m_brush = 0;

    m_clipboard = KisClipboard::instance();

    connect(m_clipboard, SIGNAL(clipChanged()), this, SLOT(slotCreateBrush()));
    connect(colorAsmask, SIGNAL(toggled(bool)), this, SLOT(slotUpdateUseColorAsMask(bool)));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(slotAddPredefined()));
    connect(nameEdit, SIGNAL(textEdited(const QString&)), this, SLOT(slotUpdateSaveButton()));

    spacingWidget->setSpacing(true, 1.0);
    connect(spacingWidget, SIGNAL(sigSpacingChanged()), SLOT(slotSpacingChanged()));
}

KisClipboardBrushWidget::~KisClipboardBrushWidget()
{
}

void KisClipboardBrushWidget::slotCreateBrush()
{
    // do nothing if it's hidden otherwise it can break the active brush is something is copied
    if (m_clipboard->hasClip() && !isHidden()) {

        pd = m_clipboard->clip(QRect(0, 0, 0, 0), false);     //Weird! Don't know how this works!
        if (pd) {
            QRect rc = pd->exactBounds();

            m_brush = KisBrushSP(new KisGbrBrush(pd, rc.x(), rc.y(), rc.width(), rc.height()));

            m_brush->setSpacing(spacingWidget->spacing());
            m_brush->setAutoSpacing(spacingWidget->autoSpacingActive(), spacingWidget->autoSpacingCoeff());
            m_brush->setFilename(TEMPORARY_CLIPBOARD_BRUSH_FILENAME);
            m_brush->setName(TEMPORARY_CLIPBOARD_BRUSH_NAME);
            m_brush->setValid(true);

            int w = preview->size().width()-10;
            preview->setPixmap(QPixmap::fromImage(m_brush->image().scaled(w, w, Qt::KeepAspectRatio)));
        }
    } else {
        preview->setText(i18n("Nothing copied\n to Clipboard"));
    }

    if (!m_brush) {
        buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);
    } else {
        buttonBox->button(QDialogButtonBox::Save)->setEnabled(true);
        colorAsmask->setChecked(true); // initializing this has to happen here since we need a valid brush for it to work
        preserveAlpha->setEnabled(true);
        preserveAlpha->setChecked(false);
    }
}

void KisClipboardBrushWidget::slotSpacingChanged()
{
    if (m_brush) {
        m_brush->setSpacing(spacingWidget->spacing());
        m_brush->setAutoSpacing(spacingWidget->autoSpacingActive(), spacingWidget->autoSpacingCoeff());
    }
}

void KisClipboardBrushWidget::showEvent(QShowEvent *)
{
    slotCreateBrush();
}

void KisClipboardBrushWidget::slotUpdateUseColorAsMask(bool useColorAsMask)
{
    preserveAlpha->setEnabled(useColorAsMask);
    if (m_brush) {
        static_cast<KisGbrBrush*>(m_brush.data())->setUseColorAsMask(useColorAsMask);
        int w = preview->size().width()-10;
        preview->setPixmap(QPixmap::fromImage(m_brush->image().scaled(w, w, Qt::KeepAspectRatio)));
    }
}

void KisClipboardBrushWidget::slotAddPredefined()
{
    if(!m_brush)
        return;

    QString dir = KoResourcePaths::saveLocation("data", ResourceType::Brushes);
    QString extension = ".gbr";
    QString name = nameEdit->text();

    if (m_rServer) {
        KisGbrBrushSP resource = m_brush->clone().dynamicCast<KisGbrBrush>();

        if (nameEdit->text().isEmpty()) {
            resource->setName(QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm"));
        }
        else {
            resource->setName(name);
        }

        resource->setFilename(resource->name().split(" ").join("_") + extension);


        if (colorAsmask->isChecked()) {
            resource->makeMaskImage(preserveAlpha->isChecked());
        }
        m_rServer->addResource(resource.dynamicCast<KisBrush>());
        emit sigNewPredefinedBrush(resource);
    }

    close();
}

void KisClipboardBrushWidget::slotUpdateSaveButton()
{
    if (QFileInfo(m_rServer->saveLocation() + "/" + nameEdit->text().split(" ").join("_")
                  + ".gbr").exists()) {
        buttonBox->button(QDialogButtonBox::Save)->setText(i18n("Overwrite"));
    } else {
        buttonBox->button(QDialogButtonBox::Save)->setText(i18n("Save"));
    }
}

#include "moc_kis_clipboard_brush_widget.cpp"
