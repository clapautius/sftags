#include "tagsbox.h"

#include <QGroupBox>
#include <qpainter.h>
#include <qlayout.h>
#include <QLineEdit>
#include <QMessageBox>

#include "main.h"
#include "file.h"

using std::set;
using std::map;

TagsBox::TagsBox()
{

}


TagsBox::TagsBox(const set<QString> &all_tags, const set<QString> &selected_tags,
                 bool add_enabled)
    : mAddEnabled(add_enabled)
{
    //setCaption(QString("Labels"));
    //setButtons(KDialog::Ok | KDialog::Cancel);
    setModal(true);

    m_selected_tags=selected_tags;
    layout(all_tags);
    connectButtons();
}


TagsBox::~TagsBox()
{
}


void TagsBox::connectButtons()
{
    if (mAddEnabled) {
        connect(mpAddNewButton, SIGNAL(clicked()), this, SLOT(addNewLabel()));
    }
    connect(mp_ok_button, SIGNAL(clicked()), this, SLOT(accept()));
    connect(mp_cancel_button, SIGNAL(clicked()), this, SLOT(reject()));
} 


void TagsBox::addLabelInList(QString labelText, bool checked)
{
    QTreeWidgetItem *pItem=new QTreeWidgetItem(QStringList(labelText));
    pItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    if (checked) {
        pItem->setCheckState(0, Qt::Checked);
    }
    else {
        pItem->setCheckState(0, Qt::Unchecked);
    }
    pItem->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
    mpLabels->addTopLevelItem(pItem);
}


void TagsBox::layout(const set<QString> &all_tags)
{
    resize(500,500);
    
    mpLabels=new QTreeWidget();
    mpLabels->setColumnCount(1);
    mpLabels->setAlternatingRowColors(true);
    mpLabels->setHeaderLabels(QStringList("Available labels"));
    set<QString>::const_iterator it;
    for (it = all_tags.begin(); it != all_tags.end(); it++) {
        addLabelInList(*it, (m_selected_tags.find(*it) != m_selected_tags.end()));
    }
    mpLabels->sortByColumn(0, Qt::AscendingOrder);
    mpLabels->setSortingEnabled(true);
    QVBoxLayout *p_v_layout = new QVBoxLayout;
    p_v_layout->addWidget(mpLabels);
    setLayout(p_v_layout);
    
    if (mAddEnabled) {
        QGroupBox *p_grp = new QGroupBox("New tag");
        mpLabelEdit=new QLineEdit();
        mpAddNewButton=new QPushButton("Add new label");
        QHBoxLayout *p_h_layout = new QHBoxLayout;
        p_h_layout->addWidget(mpLabelEdit);
        p_h_layout->addWidget(mpAddNewButton);
        p_grp->setLayout(p_h_layout);
        p_v_layout->addWidget(p_grp);
    }

    QGroupBox *p_grp2 = new QGroupBox();
    QHBoxLayout *p_h_layout2 = new QHBoxLayout;
    mp_ok_button = new QPushButton("OK");
    mp_cancel_button = new QPushButton("Cancel");
    p_h_layout2->addWidget(mp_ok_button);
    p_h_layout2->addWidget(mp_cancel_button);
    p_grp2->setLayout(p_h_layout2);
    p_v_layout->addWidget(p_grp2);
}


void TagsBox::accept()
{
    QList<QTreeWidgetItem*> list;

    m_selected_tags.clear();
    
    list=mpLabels->invisibleRootItem()->takeChildren();
    for (int i=0; i<list.size(); i++) {
        if (Qt::Checked == list[i]->checkState(0)) {
            m_selected_tags.insert(list[i]->text(0));
            // add tags to the global list
            add_used_tag(list[i]->text(0));
        }
    }
    QDialog::accept();
} 


void TagsBox::reject()
{
    QDialog::reject();
} 


void
TagsBox::addNewLabel()
{
    if (mpLabelEdit->text().isEmpty()) {
        QMessageBox::information(this, "Error", "Nothing to add");
    }
    else {
        addLabelInList(mpLabelEdit->text(), true);
        mpLabelEdit->clear();
    }
}


set<QString> TagsBox::get_selected_tags() const
{
    return m_selected_tags;
}
