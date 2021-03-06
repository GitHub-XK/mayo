/****************************************************************************
** Copyright (c) 2016, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
**     1. Redistributions of source code must retain the above copyright
**        notice, this list of conditions and the following disclaimer.
**
**     2. Redistributions in binary form must reproduce the above
**        copyright notice, this list of conditions and the following
**        disclaimer in the documentation and/or other materials provided
**        with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
****************************************************************************/

#include "dialog_options.h"

#include "options.h"
#include "property_enumeration.h"
#include "ui_dialog_options.h"
#include "fougtools/qttools/gui/qwidget_utils.h"
#include "fougtools/occtools/qt_utils.h"

#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QColorDialog>

namespace Mayo {

namespace Internal {

static QPixmap colorPixmap(const QColor& color)
{
    QPixmap pix(16, 16);
    pix.fill(color);
    return pix;
}

} // namespace Internal

DialogOptions::DialogOptions(QWidget *parent)
    : QDialog(parent),
      m_ui(new Ui_DialogOptions)
{
    m_ui->setupUi(this);

    const Options* opts = Options::instance();
    const auto& vecGpxMaterialMapping =
            Mayo::enum_Graphic3dNameOfMaterial().mappings();

    // STL import/export
    auto btnGrp_stlIoLib = new QButtonGroup(this);
    btnGrp_stlIoLib->addButton(m_ui->radioBtn_UseGmio);
    btnGrp_stlIoLib->addButton(m_ui->radioBtn_UseOcc);
    QObject::connect(
                m_ui->radioBtn_UseGmio, &QAbstractButton::toggled,
                m_ui->widget_gmioStlImport, &QWidget::setEnabled);

    const Options::StlIoLibrary lib = opts->stlIoLibrary();
    m_ui->radioBtn_UseGmio->setChecked(lib == Options::StlIoLibrary::Gmio);
    m_ui->radioBtn_UseOcc->setChecked(lib == Options::StlIoLibrary::OpenCascade);

    const Options::GmioStlImportType impType = opts->gmioStlImportType();
    m_ui->radioBtn_GmioImportStlMesh->setChecked(
                impType == Options::GmioStlImportType::OccStlMesh);
    m_ui->radioBtn_GmioImportPolyTriShape->setChecked(
                impType == Options::GmioStlImportType::OccPolyTriShape);

    // BRep shape defaults
    m_ui->toolBtn_BRepShapeDefaultColor->setIcon(
                Internal::colorPixmap(opts->brepShapeDefaultColor()));
    m_brepShapeDefaultColor = opts->brepShapeDefaultColor();
    QObject::connect(
                m_ui->toolBtn_BRepShapeDefaultColor, &QAbstractButton::clicked,
                [=] {
        this->chooseColor(opts->brepShapeDefaultColor(),
                          m_ui->toolBtn_BRepShapeDefaultColor,
                          &m_brepShapeDefaultColor);
    } );
    for (const Enumeration::Mapping& m : vecGpxMaterialMapping)
        m_ui->comboBox_BRepShapeDefaultMaterial->addItem(m.string, m.value);
    m_ui->comboBox_BRepShapeDefaultMaterial->setCurrentIndex(
                m_ui->comboBox_BRepShapeDefaultMaterial->findData(
                    static_cast<int>(opts->brepShapeDefaultMaterial())));

    // Mesh defaults
    m_ui->toolBtn_MeshDefaultColor->setIcon(
                Internal::colorPixmap(opts->meshDefaultColor()));
    m_meshDefaultColor = opts->meshDefaultColor();
    QObject::connect(
                m_ui->toolBtn_MeshDefaultColor, &QAbstractButton::clicked,
                [=] {
        this->chooseColor(opts->meshDefaultColor(),
                          m_ui->toolBtn_MeshDefaultColor,
                          &m_meshDefaultColor);
    } );
    for (const Enumeration::Mapping& m : vecGpxMaterialMapping)
        m_ui->comboBox_MeshDefaultMaterial->addItem(m.string, m.value);
    m_ui->comboBox_MeshDefaultMaterial->setCurrentIndex(
                m_ui->comboBox_MeshDefaultMaterial->findData(
                    static_cast<int>(opts->meshDefaultMaterial())));
    m_ui->checkBox_MeshShowEdges->setChecked(opts->meshDefaultShowEdges());
    m_ui->checkBox_MeshShowNodes->setChecked(opts->meshDefaultShowNodes());
}

DialogOptions::~DialogOptions()
{
    delete m_ui;
}

void DialogOptions::accept()
{
    Options* opts = Options::instance();

    // STL import/export
    if (m_ui->radioBtn_UseGmio->isChecked())
        opts->setStlIoLibrary(Options::StlIoLibrary::Gmio);
    else if (m_ui->radioBtn_UseOcc->isChecked())
        opts->setStlIoLibrary(Options::StlIoLibrary::OpenCascade);

    if (opts->stlIoLibrary() == Options::StlIoLibrary::Gmio) {
        if (m_ui->radioBtn_GmioImportStlMesh->isChecked())
            opts->setGmioStlImportType(Options::GmioStlImportType::OccStlMesh);
        else if (m_ui->radioBtn_GmioImportPolyTriShape->isChecked())
            opts->setGmioStlImportType(Options::GmioStlImportType::OccPolyTriShape);
    }

    // BRep shape defaults
    opts->setBrepShapeDefaultColor(m_brepShapeDefaultColor);
    opts->setBrepShapeDefaultMaterial(
                static_cast<Graphic3d_NameOfMaterial>(
                    m_ui->comboBox_BRepShapeDefaultMaterial->currentData().toInt()));

    // Mesh defaults
    opts->setMeshDefaultColor(m_meshDefaultColor);
    opts->setMeshDefaultMaterial(
                static_cast<Graphic3d_NameOfMaterial>(
                    m_ui->comboBox_MeshDefaultMaterial->currentData().toInt()));
    opts->setMeshDefaultShowEdges(m_ui->checkBox_MeshShowEdges->isChecked());
    opts->setMeshDefaultShowNodes(m_ui->checkBox_MeshShowNodes->isChecked());

    QDialog::accept();
}

void DialogOptions::chooseColor(
        const QColor& currentColor, QToolButton* targetBtn, QColor* targetColor)
{
    auto dlg = new QColorDialog(this);
    dlg->setCurrentColor(currentColor);
    QObject::connect(
                dlg, &QDialog::finished,
                [=](int result) {
        if (result == QDialog::Accepted) {
            targetBtn->setIcon(Internal::colorPixmap(dlg->selectedColor()));
            *targetColor = dlg->selectedColor();
        }
    } );
    qtgui::QWidgetUtils::asyncDialogExec(dlg);
}

} // namespace Mayo
