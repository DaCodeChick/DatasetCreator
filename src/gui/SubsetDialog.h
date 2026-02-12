#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QRadioButton>
#include <QStringList>

namespace DatasetCreator {

class SubsetDialog : public QDialog {
    Q_OBJECT
public:
    explicit SubsetDialog(const QStringList& existingSubsets, QWidget* parent = nullptr);
    
    QString getSubsetName() const;
    bool isNewSubset() const;
    
private slots:
    void onNewSubsetToggled(bool checked);
    void validate();
    
private:
    void setupUI();
    
    QRadioButton* newSubsetRadio_;
    QRadioButton* existingSubsetRadio_;
    QLineEdit* newSubsetNameEdit_;
    QComboBox* existingSubsetCombo_;
    QPushButton* okButton_;
    
    QStringList existingSubsets_;
};

}
