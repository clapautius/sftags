#include <stdlib.h>
#include <vector>


#include <QString>
#include <QMessageBox>
#include <QFileSystemModel>
#include <QDir>
#include <QListWidget>
#include <QDesktopServices>
#include <QUrl>

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
using std::vector;

#define NO_TAGS_TEXT "<span style=\"font-weight: bold;\">No tags.</span>"
#define TAGS_TEXT "<span style=\"font-weight: bold;\">Tags: </span>"

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

    // expand on single click
    connect(treeView, SIGNAL(clicked(const QModelIndex &)),
            this, SLOT(tree_item_clicked(const QModelIndex&)));

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
    connect(mp_search, SIGNAL(clicked()), this, SLOT(search()));
    connect(mp_results_list, SIGNAL(itemClicked(QListWidgetItem*)),
            this, SLOT(result_clicked(QListWidgetItem*)));
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
            mp_tags_label->setText(NO_TAGS_TEXT);
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
        mp_tags_label->setText(NO_TAGS_TEXT);
    } else {
        mp_tags_label->setText(QString(TAGS_TEXT) + tags_str);
    }
}


void FilesAndTagsWnd::add_used_tag(const QString &tag)
{
    mp_tags_list->addItem(tag);
}



void FilesAndTagsWnd::tree_item_clicked(const QModelIndex &idx)
{
    if (treeView->isExpanded(idx)) {
        treeView->collapse(idx);
    } else {
        treeView->expand(idx);
    }
}


/**
 * Pseudo lamba expression.
 * Predicate that returns true if the file has any of the tags.
 * @param[in] p_data : a pointer to a QList<QListWidgetItem*>.
 **/
static bool contains_any_tag(const File &file, void *p_data)
{
    QList<QListWidgetItem*> *p_tags = static_cast<QList<QListWidgetItem*>*>(p_data);
    if (p_tags) {
        for (int i = 0; i < p_tags->size(); i++) {
            if (file.has_tag((*p_tags)[i]->text())) {
                return true;
            }
        }
    }
    return false;
}


/**
 * Pseudo lamba expression.
 * Predicate that returns true if the file has all the tags.
 * @param[in] p_data : a pointer to a QList<QListWidgetItem*>.
 **/
static bool contains_all_tags(const File &file, void *p_data)
{
    QList<QListWidgetItem*> *p_tags = static_cast<QList<QListWidgetItem*>*>(p_data);
    if (p_tags) {
        bool has_all = true;
        for (int i = 0; i < p_tags->size(); i++) {
            if (!file.has_tag((*p_tags)[i]->text())) {
                has_all = false;
                break;
            }
        }
        if (has_all) {
            return true;
        } else {
            return false;
        }
    }
    return false;
}


void FilesAndTagsWnd::search()
{
    QList<QListWidgetItem*> selected = mp_tags_list->selectedItems();
    if (selected.size() <= 0) {
        QMessageBox::information(this, "Error", "Nothing selected");
    } else {
        mp_results_list->clear();
        vector<QString> result;
        if (mp_any_button->isChecked()) {
            result = filter_files(contains_any_tag, &selected);
        } else {
            result = filter_files(contains_all_tags, &selected);
        }
        for (unsigned int i = 0; i < result.size(); i++) {
            mp_results_list->addItem(result[i]);
        }
    }
}


void FilesAndTagsWnd::result_clicked(QListWidgetItem *p_item)
{
    QString full_url = "file://";
    full_url += p_item->text();
    QDesktopServices::openUrl(QUrl(full_url));
}
