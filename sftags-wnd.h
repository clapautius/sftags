#ifndef FILES_AND_TAGS_WND_H
#define FILES_AND_TAGS_WND_H

#include <QFileSystemModel>

#include "ui_sftags.h"
#include "file.h"

class FilesAndTagsWnd : public QDialog, public Ui::Dialog
{
  Q_OBJECT

public:

  FilesAndTagsWnd( QWidget *parent = 0, char *name = 0 );

  void add_used_tag(const QString &tag);

protected slots:
  
  void selection_changed(const QModelIndex & current, const QModelIndex & previous);

  void change_tags();

private:

  void setup_slots();

  void display_tags(const File&);

  QFileSystemModel *mp_model;

  QString m_current_path;
};

#endif
