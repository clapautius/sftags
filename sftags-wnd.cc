#include <stdlib.h>
#include <vector>
#include <iostream>

#include <QString>
#include <QMessageBox>
#include <QFileSystemModel>
#include <QDir>

#include "sftags-wnd.h"
#include "main.h"
#include "tagsbox.h"
#include "file.h"

using std::set;
using std::cout;
using std::endl;


FilesAndTagsWnd::FilesAndTagsWnd( QWidget *, char *)
{
    setupUi(this);

    mp_model = new QFileSystemModel;
    mp_model->setRootPath(QDir::home().path());
    treeView->setModel(mp_model);
    treeView->setRootIndex(mp_model->index(QDir::home().path()));
    treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    mp_tags_button->setEnabled(false);
    treeView->hideColumn(3);
    treeView->hideColumn(2);
    treeView->hideColumn(1);
    treeView->setIconSize(QSize(24, 24));
    setup_slots();
}


void FilesAndTagsWnd::setup_slots()
{
    QItemSelectionModel *p_model = treeView->selectionModel();
    connect(p_model, SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
            this, SLOT(selection_changed(const QModelIndex&, const QModelIndex&)));
    connect(mp_tags_button, SIGNAL(clicked()), this, SLOT(change_tags()));
    connect(mp_close_button, SIGNAL(clicked()), this, SLOT(close()));
}


void FilesAndTagsWnd::selection_changed(const QModelIndex & current, const QModelIndex &)
{
    if (current.isValid()) {
        mp_tags_button->setEnabled(true);
        // get full real path
        QString full_path = mp_model->filePath(current);
        char *p_real_path = realpath(qstr2cchar(full_path), NULL);
        m_current_path = QString::fromUtf8(p_real_path);
        free(p_real_path);
        int slash_pos = full_path.lastIndexOf('/');
        if (slash_pos > 0) {
            mp_fname_label->setText(
              full_path.right(full_path.length() - slash_pos - 1));
        } else {
            mp_fname_label->setText(full_path);
        }
        File file_details;
        if (get_file_for_path(m_current_path, file_details)) {
            display_tags(file_details);
        } else {
            mp_tags_label->setText("");
        }
    } else {
        mp_fname_label->setText("");
        mp_tags_label->setText("");
        mp_tags_button->setEnabled(false);
    }
}


void FilesAndTagsWnd::change_tags()
{
    set<QString> current_tags;
    File file_details;
    file_details.set_path(m_current_path);
    if (get_file_for_path(m_current_path, file_details)) {
        current_tags = file_details.get_tags();
    }
    TagsBox *p_tags_box = new TagsBox(get_all_used_tags(), current_tags, true);
    if (p_tags_box->exec() == QDialog::Accepted) {
        set<QString> new_tags = p_tags_box->get_selected_tags();
        // print tags
        set<QString>::const_iterator it;
        cout<<"File tags: ";
        for (it = new_tags.begin(); it != new_tags.end(); it++) {
            cout<<qstr2cchar(*it);
        }
        cout<<endl;
        
        if (new_tags.empty()) {
            erase_file(file_details);
        } else {
            file_details.set_tags(new_tags);
            add_file(file_details);
        }
        display_tags(file_details);
    }
}


void FilesAndTagsWnd::display_tags(const File &file_details)
{
    QString tags_str;
    set<QString> tags = file_details.get_tags();
    set<QString>::const_iterator it;
    for (it = tags.begin(); it != tags.end(); it++) {
        tags_str += *it + " ";
    }
    mp_tags_label->setText(tags_str);
}
