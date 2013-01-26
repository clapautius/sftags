#ifndef TAGSBOX_H
#define TAGSBOX_H

#include <set>

#include <qlayout.h>
#include <QTreeWidget>
#include <QDialog>
#include <QLabel>
#include <QPushButton>

class TagsBox : public QDialog
{
    Q_OBJECT
            
public:
    TagsBox();
    
    TagsBox(const std::set<QString> &all_labels,
            const std::set<QString> &selected_labels,
            bool addEnabled = true);

    virtual ~TagsBox();

    std::set<QString> get_selected_tags() const;
    
public slots:

protected slots:  
    
    virtual void accept();
    virtual void reject();
    virtual void addNewLabel();
        
private:
    
    void connectButtons();
    
    void layout(const std::set<QString>&);

    void addLabelInList(QString labelText, bool checked=false);

    std::set<QString> m_selected_tags;

    bool mAddEnabled;

    QTreeWidget *mpLabels;
    QLineEdit *mpLabelEdit;
    QPushButton *mpAddNewButton;
    QPushButton *mp_ok_button;
    QPushButton *mp_cancel_button;
};

#endif
