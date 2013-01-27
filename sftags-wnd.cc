#include <stdlib.h>
#include <vector>


#include <QString>
#include <QMessageBox>
#include <QFileSystemModel>
#include <QDir>
#include <QListWidget>

#include "sftags-wnd.h"
#include "main.h"
#include "tagsbox.h"
#include "file.h"

#ifdef SFTAGS_DEBUG
  #include <iostream>
  using std::cout;
  using std::endl;
#endif

using std::set;


FilesAndTagsWnd::FilesAndTagsWnd( QWidget *, char *)
{
    setupUi(this);

    QString start_path = QDir::home().path();
    //QString start_path = "/"; // testing
    mp_model = new QFileSystemModel;
    mp_model->setRootPath(start_path);
    treeView->setModel(mp_model);
    treeView->setRootIndex(mp_model->index(start_path));
    treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    mp_tags_button->setEnabled(false);
    treeView->hideColumn(3);
    treeView->hideColumn(2);
    treeView->hideColumn(1);
    treeView->setIconSize(QSize(24, 24));
    treeView->setAlternatingRowColors(true);
    setup_slots();

    // add used tags to tags list in search tab
    const set<QString> &current_used_tags = get_all_used_tags();
    set<QString>::const_iterator it;
    for (it = current_used_tags.begin(); it != current_used_tags.end(); it++) {
        this->add_used_tag(*it);
    }    
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
        char *p_real_path = realpath(Q_STR(full_path), NULL);
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
            mp_tags_label->setText("No tags.");
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
#ifdef SFTAGS_DEBUG
        cout<<"File tags: ";
        for (it = new_tags.begin(); it != new_tags.end(); it++) {
            cout<<Q_STR(*it)<<" ";
        }
        cout<<endl;
#endif
        
        if (new_tags.empty()) {
            erase_file(file_details);
        } else {
            file_details.set_tags(new_tags);
            add_file(file_details);
        }

        // save changes
        if (!save_xml()) {
            QMessageBox::critical(NULL, "Error", "Error saving XML document");
            // :fixme: do something (revert?)
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
    if (tags_str.isEmpty()) {
        mp_tags_label->setText("No tags.");
    } else {
        mp_tags_label->setText(QString("Tags: ") + tags_str);
    }
}


void FilesAndTagsWnd::add_used_tag(const QString &tag)
{
    mp_tags_list->addItem(tag);
}

