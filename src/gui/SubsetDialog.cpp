#include "SubsetDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QMessageBox>

namespace DatasetCreator {

SubsetDialog::SubsetDialog(const QStringList& existingSubsets, QWidget* parent)
    : QDialog(parent), existingSubsets_(existingSubsets)
{
    setWindowTitle("Move to Subset");
    setupUI();
    validate();
}

void SubsetDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Instructions
    QLabel* instructions = new QLabel(
        "Select an existing subset or create a new one:", this);
    mainLayout->addWidget(instructions);
    
    // Radio button group for new subset
    newSubsetRadio_ = new QRadioButton("Create new subset:", this);
    newSubsetRadio_->setChecked(true);
    mainLayout->addWidget(newSubsetRadio_);
    
    newSubsetNameEdit_ = new QLineEdit(this);
    newSubsetNameEdit_->setPlaceholderText("Enter subset name (e.g., training, validation, test)");
    mainLayout->addWidget(newSubsetNameEdit_);
    
    // Radio button for existing subset
    existingSubsetRadio_ = new QRadioButton("Use existing subset:", this);
    existingSubsetRadio_->setEnabled(!existingSubsets_.isEmpty());
    mainLayout->addWidget(existingSubsetRadio_);
    
    existingSubsetCombo_ = new QComboBox(this);
    existingSubsetCombo_->addItems(existingSubsets_);
    existingSubsetCombo_->setEnabled(!existingSubsets_.isEmpty());
    mainLayout->addWidget(existingSubsetCombo_);
    
    if (existingSubsets_.isEmpty()) {
        existingSubsetRadio_->setToolTip("No existing subsets available");
        existingSubsetCombo_->setToolTip("No existing subsets available");
    }
    
    mainLayout->addStretch();
    
    // Dialog buttons
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    okButton_ = buttonBox->button(QDialogButtonBox::Ok);
    
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(newSubsetRadio_, &QRadioButton::toggled, 
            this, &SubsetDialog::onNewSubsetToggled);
    connect(newSubsetNameEdit_, &QLineEdit::textChanged,
            this, &SubsetDialog::validate);
    
    mainLayout->addWidget(buttonBox);
    
    resize(400, 250);
}

void SubsetDialog::onNewSubsetToggled(bool checked) {
    newSubsetNameEdit_->setEnabled(checked);
    existingSubsetCombo_->setEnabled(!checked && !existingSubsets_.isEmpty());
    validate();
}

QString SubsetDialog::getSubsetName() const {
    if (newSubsetRadio_->isChecked()) {
        return newSubsetNameEdit_->text().trimmed();
    } else {
        return existingSubsetCombo_->currentText();
    }
}

bool SubsetDialog::isNewSubset() const {
    return newSubsetRadio_->isChecked();
}

void SubsetDialog::validate() {
    bool valid = false;
    
    if (newSubsetRadio_->isChecked()) {
        QString name = newSubsetNameEdit_->text().trimmed();
        valid = !name.isEmpty() && !existingSubsets_.contains(name);
    } else {
        valid = !existingSubsets_.isEmpty();
    }
    
    okButton_->setEnabled(valid);
}

}
