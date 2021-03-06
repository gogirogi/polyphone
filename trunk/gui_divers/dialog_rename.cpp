/***************************************************************************
**                                                                        **
**  Polyphone, a soundfont editor                                         **
**  Copyright (C) 2013-2017 Davy Triponney                                **
**                                                                        **
**  This program is free software: you can redistribute it and/or modify  **
**  it under the terms of the GNU General Public License as published by  **
**  the Free Software Foundation, either version 3 of the License, or     **
**  (at your option) any later version.                                   **
**                                                                        **
**  This program is distributed in the hope that it will be useful,       **
**  but WITHOUT ANY WARRANTY; without even the implied warranty of        **
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
**  GNU General Public License for more details.                          **
**                                                                        **
**  You should have received a copy of the GNU General Public License     **
**  along with this program.  If not, see http://www.gnu.org/licenses/.   **
**                                                                        **
****************************************************************************
**           Author: Davy Triponney                                       **
**  Website/Contact: http://polyphone-soundfonts.com                      **
**             Date: 01.01.2013                                           **
***************************************************************************/


#include "dialog_rename.h"
#include "ui_dialog_rename.h"
#include "confmanager.h"

DialogRename::DialogRename(bool isSample, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogRename),
    _isSample(isSample)
{
    ui->setupUi(this);
    this->setWindowFlags((windowFlags() & ~Qt::WindowContextHelpButtonHint));
    ui->comboBox->blockSignals(true);
    ui->comboBox->setCurrentIndex(ConfManager::getInstance()->getValue(ConfManager::SECTION_BULK_RENAME, "option", 0).toInt());
    ui->comboBox->blockSignals(false);
    if (!_isSample)
        ui->comboBox->removeItem(0);

    ui->lineText1->setText(ConfManager::getInstance()->getValue(ConfManager::SECTION_BULK_RENAME, "text_1", "").toString());
    ui->lineText2->setText(ConfManager::getInstance()->getValue(ConfManager::SECTION_BULK_RENAME, "text_2", "").toString());
    ui->spinPos1->setValue(ConfManager::getInstance()->getValue(ConfManager::SECTION_BULK_RENAME, "int_1", 0).toInt());
    ui->spinPos2->setValue(ConfManager::getInstance()->getValue(ConfManager::SECTION_BULK_RENAME, "int_2", 0).toInt());

    on_comboBox_currentIndexChanged(ui->comboBox->currentIndex());

    ui->lineText1->selectAll();
    ui->lineText1->setFocus();
}

DialogRename::~DialogRename()
{
    delete ui;
}

void DialogRename::accept()
{
    int type = ui->comboBox->currentIndex() + !_isSample;
    this->updateNames(type, ui->lineText1->text(), ui->lineText2->text(),
                      ui->spinPos1->value(), ui->spinPos2->value());

    // Mémorisation des paramètres
    ConfManager::getInstance()->setValue(ConfManager::SECTION_BULK_RENAME, "option", ui->comboBox->currentIndex() + !_isSample);
    ConfManager::getInstance()->setValue(ConfManager::SECTION_BULK_RENAME, "int_1", ui->spinPos1->value());
    ConfManager::getInstance()->setValue(ConfManager::SECTION_BULK_RENAME, "int_2", ui->spinPos2->value());
    ConfManager::getInstance()->setValue(ConfManager::SECTION_BULK_RENAME, "text_1", ui->lineText1->text());
    ConfManager::getInstance()->setValue(ConfManager::SECTION_BULK_RENAME, "text_2", ui->lineText2->text());

    QDialog::accept();
}

void DialogRename::on_comboBox_currentIndexChanged(int index)
{
    switch (index + !_isSample)
    {
    case 0:
        // Remplacer avec note en suffixe
        ui->labelPos->hide();
        ui->spinPos1->hide();
        ui->spinPos2->hide();
        ui->labelString1->setText(trUtf8("Nouveau nom :"));
        ui->labelString1->show();
        ui->lineText1->show();
        ui->labelString2->hide();
        ui->lineText2->hide();
        break;
    case 1:
        // Remplacer avec index en suffixe
        ui->labelPos->hide();
        ui->spinPos1->hide();
        ui->spinPos2->hide();
        ui->labelString1->setText(trUtf8("Nouveau nom :"));
        ui->labelString1->show();
        ui->lineText1->show();
        ui->labelString2->hide();
        ui->lineText2->hide();
        break;
    case 2:
        // Remplacer
        ui->labelPos->hide();
        ui->spinPos1->hide();
        ui->spinPos2->hide();
        ui->labelString1->setText(trUtf8("Trouver :"));
        ui->labelString1->show();
        ui->lineText1->show();
        ui->labelString2->setText(trUtf8("Et remplacer par :"));
        ui->labelString2->show();
        ui->lineText2->show();
        break;
    case 3:
        // Insérer
        ui->labelPos->show();
        ui->spinPos1->show();
        ui->spinPos2->hide();
        ui->labelPos->setText(trUtf8("Position"));
        ui->labelString1->setText(trUtf8("Texte à insérer :"));
        ui->labelString1->show();
        ui->lineText1->show();
        ui->labelString2->hide();
        ui->lineText2->hide();
        break;
    case 4:
        // Supprimer
        ui->labelPos->show();
        ui->spinPos1->show();
        ui->spinPos2->show();
        ui->labelPos->setText(trUtf8("Étendue"));
        ui->labelString1->hide();
        ui->lineText1->hide();
        ui->labelString2->hide();
        ui->lineText2->hide();
        break;
    default:
        break;
    }

    this->adjustSize();
}
