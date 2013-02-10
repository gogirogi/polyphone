/***************************************************************************
**                                                                        **
**  Polyphone, a soundfont editor                                         **
**  Copyright (C) 2013 Davy Triponney                                     **
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
**  Website/Contact: http://www.polyphone.fr/                             **
**             Date: 01.01.2013                                           **
***************************************************************************/

#ifndef PAGE_H
#define PAGE_H

#include "pile_sf2.h"
#include "synth.h"
#include <QStackedWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QHeaderView>
#include <QSpinBox>
#include <QCheckBox>
#include <QTextEdit>
#include <QApplication>
#include <QPushButton>

namespace Ui
{
    class Page;
    class PageTable;
    class TableComboBox;
    class ComboBox;
}

class Page : public QWidget
{
    Q_OBJECT
public:
    Page(QWidget *parent = 0);
    // Méthodes publiques
    virtual void afficher() = 0;
protected:
    // Attributs protégés
    bool preparation;
    static MainWindow *mainWindow;
    static Tree *tree;
    static QStackedWidget *qStackedWidget;
    static Pile_sf2 *sf2;
    static Synth * synth;
    // Méthodes protégées
    static char * getTextValue(char * T, WORD champ, genAmountType genVal);
    static char * getTextValue(char * T, WORD champ, int iVal);
    static char * getTextValue(char * T, WORD champ, SFModulator sfModVal);
    static QString getIndexName(WORD iVal, int CC);
    static QString getGenName(WORD iVal);
    static genAmountType getValue(QString texte, WORD champ, bool &ok);
private:
    // Méthodes privées
    static int limit(int iTmp, int min, int max);
};

// Classe QTableWidget avec inclusion d'une ID
class TableWidget : public QTableWidget
{
    Q_OBJECT
public:
    // Constructeur
    TableWidget(QWidget *parent = 0);
    ~TableWidget();
    // Méthodes publiques
    void clear();
    void efface();
    void addColumn(int column, QString title);
    void setID(EltID id, int colonne);
    EltID getID(int colonne);
    // Association champ - ligne (méthodes virtuelles pures)
    virtual Champ getChamp(int row) = 0;
    virtual int getRow(WORD champ) = 0;
private slots:
    void emitSet(int ligne, int colonne, bool newAction);
signals:
    void set(int ligne, int colonne, bool newAction);
};

// Classe QTableWidget pour mod
class TableWidgetMod : public QTableWidget
{
    Q_OBJECT
public:
    // Constructeur
    TableWidgetMod(QWidget *parent = 0);
    ~TableWidgetMod();
    // Méthodes publiques
    void clear();
    void addRow(int row);
    void setID(EltID id, int row);
    EltID getID(int row);
    EltID getID();
};

class KeyPressCatcher : public QObject
{
    Q_OBJECT
signals:
    void set(int ligne, int row, bool newAction);
public:
    KeyPressCatcher(TableWidget *table) : QObject()
    {
        this->table = table;
    }
    ~KeyPressCatcher() {}
    bool eventFilter(QObject* object,QEvent* event)
    {
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = dynamic_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_C && (keyEvent->modifiers() & Qt::ControlModifier))
            {
                if (this->table->selectedItems().count() == 1)
                    QApplication::clipboard()->setText(this->table->selectedItems().takeFirst()->text());
            }
            else if (keyEvent->key() == Qt::Key_V && (keyEvent->modifiers() & Qt::ControlModifier))
            {
                if (this->table->selectedItems().count() == 1)
                    this->table->selectedItems().takeFirst()->setText(QApplication::clipboard()->text());
            }
            else if (keyEvent->key() == Qt::Key_Backspace || keyEvent->key() == Qt::Key_Delete)
            {
                // Touche retour ou suppr (efface la cellule)
                this->table->blockSignals(true);
                // Retrait des éléments sélectionné sur les lignes 0 à 3 inclus
                QList<QTableWidgetItem *> listCell = this->table->selectedItems();
                int nbElt = listCell.count();
                for (int i = nbElt-1; i >= 0; i--)
                {
                    if (listCell.at(i)->row() < 4)
                        listCell.removeAt(i);
                }
                if (listCell.count() == 0) return QObject::eventFilter(object, event);
                listCell.at(0)->setText("");
                emit(set(listCell.at(0)->row(),
                         listCell.at(0)->column(),
                         true));
                for (int i = 1; i < listCell.count(); i++)
                {
                    listCell.at(i)->setText("");
                    emit(set(listCell.at(i)->row(),
                             listCell.at(i)->column(),
                             false));
                }
                this->table->blockSignals(false);
            }
            else if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
            {
                // Touche entrée (entre dans la cellule)
                if (this->table->selectedItems().count() == 1)
                {
                    //int row = this->table->selectedItems().takeAt(0)->row();
                    //int column = this->table->selectedItems().takeAt(0)->column();
                    //this->table->editItem(this->table->item(row, column));
                }
            }
        }
        // on laisse passer l'événement
        return QObject::eventFilter(object, event);
    }
private:
    TableWidget *table;
};

// Classe TableComboBox pour les formes de courbes
class TableComboBox : public QComboBox
{
    Q_OBJECT
public:
    TableComboBox(QWidget* parent = 0) : QComboBox(parent)
    {
        char T[60];
        // Nouvelle vue
        QTableView * view = new QTableView();
        view->viewport()->installEventFilter(this);
        view->horizontalHeader()->setVisible(false);
        view->verticalHeader()->setVisible(false);
        view->setShowGrid(false);
        view->setFixedSize(28*4, 30*4);
        this->setView(view);
        // Préparation du modèle
        QStandardItemModel * model = new QStandardItemModel();
        model->setColumnCount(4);
        model->setRowCount(4);
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                sprintf(T, ":/icones/courbe%.2d", 4*j+i+1);
                model->setItem(i, j, new QStandardItem(QIcon(T), ""));
            }
        }
        this->setModel(model);
        view->resizeColumnsToContents();
    }
    bool eventFilter(QObject* object, QEvent* event)
    {
        if (event->type() == QEvent::MouseButtonPress && object == view()->viewport())
        {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            view()->indexAt(mouseEvent->pos());
            this->setCurrentIndex(view()->currentIndex().row());
            this->setModelColumn(view()->currentIndex().column());
            emit(clicked(view()->currentIndex().row(), view()->currentIndex().column()));
        }
        return false;
    }
signals:
    void clicked(int row, int column);
};

// Classe ComboBox pour destination et source
class ComboBox : public QComboBox
{
    Q_OBJECT
public:
    ComboBox(QWidget* parent = 0) : QComboBox(parent)
    {
        this->limite = 0;
        this->view()->viewport()->installEventFilter(this);
    }
    bool eventFilter(QObject* object, QEvent* event)
    {
        if (event->type() == QEvent::MouseButtonPress && object == view()->viewport())
        {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            int index = view()->indexAt(mouseEvent->pos()).row();
            if (index >= this->limite)
            {
                // index du modulateur pointé
                char T[20];
                strcpy(T, this->itemText(index).toStdString().c_str());
                index = this->itemText(index).split("#").last().toInt();
                //sscanf(T, ": #%i", &index);
                index += 32767; // 32768 - 1
            }
            emit(this->clicked(index));
        }
        return false;
    }
    void setLimite(int lim) {this->limite = lim;}
    void removeItemsAfterLim()
    {
        for (int i = this->count(); i >= this->limite; i--)
            this->removeItem(i);
    }
    void selectIndex(WORD index, WORD numChamp)
    {
        if (index > 99)
        {
            // On cherche le modulateur numChamp
            numChamp -= 32768;
            char T[20];
            sprintf(T, "%s: #%d", tr("Modulateur").toStdString().c_str(), numChamp+1);
            int iVal = this->findText(T);
            if (iVal != -1)
                this->setCurrentIndex(iVal);
            else
                this->setCurrentIndex(0);
        }
        else
        {
            // Sélection de l'index pointé par index
            this->setCurrentIndex(index);
        }
    }
    void selectIndex(WORD index)
    {
        this->setCurrentIndex(index);
    }
private:
    int limite;
signals:
    void clicked(int index);
};

// Spécialisation de page pour inst et prst
class PageTable : public Page
{
    Q_OBJECT
public:
    PageTable(QWidget *parent = 0);
    // Méthodes publiques
    void afficher();
    void reselect();
    void updateId(EltID id);
protected:
    // Attributs protégés
    ElementType contenant;
    ElementType contenantGen;
    ElementType contenantMod;
    ElementType lien;
    ElementType lienGen;
    ElementType lienMod;
    ElementType contenu;
    TableWidget *table;
    TableWidgetMod *tableMod;
    QSpinBox *spinAmount;
    ComboBox *comboSource1;
    TableComboBox *comboSource1Courbe;
    ComboBox *comboSource2;
    TableComboBox *comboSource2Courbe;
    ComboBox *comboDestination;
    QCheckBox *checkAbs;
    QPushButton *pushSupprimerMod;
    QPushButton *pushNouveauMod;
    // Méthodes protégées
    void selectNone(bool refresh = false);
    void select(EltID id, bool refresh = false);
    static void remplirComboSource(ComboBox *comboBox);
    virtual int getDestIndex(int i) = 0;
    virtual int getDestNumber(int i) = 0;
    WORD getSrcIndex(WORD wVal, bool bVal);
    WORD getSrcNumber(WORD wVal, bool &CC);
private:
    void afficheMod(EltID id, int selectedRow = -1);
    static void addAvailableReceiverMod(ComboBox *combo, EltID id);
    static void addAvailableSenderMod(ComboBox *combo, EltID id);
    int getAssociatedMod(EltID id);
    int limit(int iVal, Champ champ, EltID id);
public slots:
    void set(int ligne, int colonne, bool newAction = true);
    void setAmount();
    void setAbs();
    void selected();
    void afficheEditMod();
    void setSourceType(int row, int column);
    void setSourceAmountType(int row, int column);
    void setDest(int index);
    void setSource(int index);
    void setSource2(int index);
    void supprimerMod();
    void nouveauMod();
};


#endif // PAGE_H