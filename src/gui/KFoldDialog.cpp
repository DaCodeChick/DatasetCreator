#include "KFoldDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>

namespace DatasetCreator {

KFoldDialog::KFoldDialog(int totalSamples, const QStringList& availableLabels, QWidget* parent)
    : QDialog(parent), totalSamples_(totalSamples)
{
    setWindowTitle(tr("K-Fold Cross-Validation Split"));
    setupUI();
    
    // Populate label combo box
    if (!availableLabels.isEmpty()) {
        stratifyLabelCombo_->addItems(availableLabels);
    } else {
        stratifiedCheck_->setEnabled(false);
        stratifiedCheck_->setToolTip(tr("No labels found in dataset"));
    }
    
    updateFoldCounts();
}

void KFoldDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Info label
    QLabel* infoLabel = new QLabel(tr("Generate K-Fold cross-validation splits for %1 samples.<br>"
        "<i>Creates K train/test subset pairs, where each sample appears once in test set.</i>")
        .arg(totalSamples_), this);
    infoLabel->setWordWrap(true);
    mainLayout->addWidget(infoLabel);
    
    // Configuration group
    QGroupBox* configGroup = new QGroupBox(tr("K-Fold Configuration"), this);
    QFormLayout* configLayout = new QFormLayout(configGroup);
    
    // Number of folds
    foldsSpin_ = new QSpinBox(this);
    foldsSpin_->setRange(2, 20);
    foldsSpin_->setValue(5);
    foldsSpin_->setToolTip(tr("Number of folds (typical values: 5 or 10)"));
    connect(foldsSpin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &KFoldDialog::onFoldsChanged);
    
    foldSizeLabel_ = new QLabel(this);
    
    QHBoxLayout* foldsRow = new QHBoxLayout();
    foldsRow->addWidget(foldsSpin_);
    foldsRow->addWidget(foldSizeLabel_);
    foldsRow->addStretch();
    
    configLayout->addRow(tr("Number of Folds (K):"), foldsRow);
    
    // Prefix for fold names
    prefixEdit_ = new QLineEdit("fold", this);
    prefixEdit_->setToolTip(tr("Prefix for subset names (e.g., 'fold' creates 'fold1_train', 'fold1_test', ...)"));
    configLayout->addRow(tr("Subset Name Prefix:"), prefixEdit_);
    
    mainLayout->addWidget(configGroup);
    
    // Options
    QGroupBox* optionsGroup = new QGroupBox(tr("Options"), this);
    QVBoxLayout* optionsLayout = new QVBoxLayout(optionsGroup);
    
    shuffleCheck_ = new QCheckBox(tr("Shuffle samples before splitting"), this);
    shuffleCheck_->setChecked(true);
    shuffleCheck_->setToolTip(tr("Randomly shuffle samples before creating folds"));
    optionsLayout->addWidget(shuffleCheck_);
    
    stratifiedCheck_ = new QCheckBox(tr("Stratified K-Fold (preserve label distribution)"), this);
    stratifiedCheck_->setChecked(false);
    stratifiedCheck_->setToolTip(tr("Maintain label proportions in each fold"));
    connect(stratifiedCheck_, &QCheckBox::toggled,
            this, &KFoldDialog::onStratifiedToggled);
    optionsLayout->addWidget(stratifiedCheck_);
    
    QHBoxLayout* stratifyRow = new QHBoxLayout();
    stratifyRow->addWidget(new QLabel(tr("Stratify by label:"), this));
    stratifyLabelCombo_ = new QComboBox(this);
    stratifyLabelCombo_->setEnabled(false);
    stratifyRow->addWidget(stratifyLabelCombo_);
    stratifyRow->addStretch();
    optionsLayout->addLayout(stratifyRow);
    
    mainLayout->addWidget(optionsGroup);
    
    // Dialog buttons
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);
    
    setMinimumWidth(500);
}

void KFoldDialog::onFoldsChanged(int value) {
    updateFoldCounts();
}

void KFoldDialog::onStratifiedToggled(bool checked) {
    stratifyLabelCombo_->setEnabled(checked);
}

void KFoldDialog::updateFoldCounts() {
    int folds = foldsSpin_->value();
    int foldSize = totalSamples_ / folds;
    int remainder = totalSamples_ % folds;
    
    QString info;
    if (remainder == 0) {
        info = tr("(~%1 samples per fold)").arg(foldSize);
    } else {
        info = tr("(~%1-%2 samples per fold)").arg(foldSize).arg(foldSize + 1);
    }
    
    foldSizeLabel_->setText(info);
}

KFoldDialog::KFoldConfig KFoldDialog::getConfig() const {
    KFoldConfig config;
    config.folds = foldsSpin_->value();
    config.prefixName = prefixEdit_->text();
    config.stratified = stratifiedCheck_->isChecked();
    config.stratifyLabel = stratifyLabelCombo_->currentText();
    config.shuffle = shuffleCheck_->isChecked();
    return config;
}

}
