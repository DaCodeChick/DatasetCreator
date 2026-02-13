#include "AutoSplitDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>

namespace DatasetCreator {

AutoSplitDialog::AutoSplitDialog(int totalSamples, const QStringList& availableLabels, QWidget* parent)
    : QDialog(parent), totalSamples_(totalSamples)
{
    setWindowTitle(tr("Automatic Dataset Split"));
    setupUI();
    
    // Populate label combo box
    if (!availableLabels.isEmpty()) {
        stratifyLabelCombo_->addItems(availableLabels);
    } else {
        stratifiedCheck_->setEnabled(false);
        stratifiedCheck_->setToolTip(tr("No labels found in dataset"));
    }
    
    updateSampleCounts();
}

void AutoSplitDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Info label
    QLabel* infoLabel = new QLabel(tr("Split %1 samples into training, validation, and test subsets.")
        .arg(totalSamples_), this);
    infoLabel->setWordWrap(true);
    mainLayout->addWidget(infoLabel);
    
    // Preset selection
    QGroupBox* presetGroup = new QGroupBox(tr("Preset Ratios"), this);
    QVBoxLayout* presetLayout = new QVBoxLayout(presetGroup);
    
    presetCombo_ = new QComboBox(this);
    presetCombo_->addItem(tr("70% Train / 20% Val / 10% Test (Recommended)"), QVariantList{70, 20, 10});
    presetCombo_->addItem(tr("80% Train / 10% Val / 10% Test"), QVariantList{80, 10, 10});
    presetCombo_->addItem(tr("60% Train / 20% Val / 20% Test"), QVariantList{60, 20, 20});
    presetCombo_->addItem(tr("Custom"), QVariantList{0, 0, 0});
    connect(presetCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AutoSplitDialog::onPresetChanged);
    presetLayout->addWidget(presetCombo_);
    mainLayout->addWidget(presetGroup);
    
    // Custom percentages
    QGroupBox* percentGroup = new QGroupBox(tr("Split Percentages"), this);
    QFormLayout* percentLayout = new QFormLayout(percentGroup);
    
    trainingSpin_ = new QSpinBox(this);
    trainingSpin_->setRange(0, 100);
    trainingSpin_->setValue(70);
    trainingSpin_->setSuffix("%");
    connect(trainingSpin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &AutoSplitDialog::onPercentageChanged);
    
    validationSpin_ = new QSpinBox(this);
    validationSpin_->setRange(0, 100);
    validationSpin_->setValue(20);
    validationSpin_->setSuffix("%");
    connect(validationSpin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &AutoSplitDialog::onPercentageChanged);
    
    testSpin_ = new QSpinBox(this);
    testSpin_->setRange(0, 100);
    testSpin_->setValue(10);
    testSpin_->setSuffix("%");
    connect(testSpin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &AutoSplitDialog::onPercentageChanged);
    
    trainingCountLabel_ = new QLabel(this);
    validationCountLabel_ = new QLabel(this);
    testCountLabel_ = new QLabel(this);
    
    QHBoxLayout* trainingRow = new QHBoxLayout();
    trainingRow->addWidget(trainingSpin_);
    trainingRow->addWidget(trainingCountLabel_);
    trainingRow->addStretch();
    
    QHBoxLayout* validationRow = new QHBoxLayout();
    validationRow->addWidget(validationSpin_);
    validationRow->addWidget(validationCountLabel_);
    validationRow->addStretch();
    
    QHBoxLayout* testRow = new QHBoxLayout();
    testRow->addWidget(testSpin_);
    testRow->addWidget(testCountLabel_);
    testRow->addStretch();
    
    percentLayout->addRow(tr("Training:"), trainingRow);
    percentLayout->addRow(tr("Validation:"), validationRow);
    percentLayout->addRow(tr("Test:"), testRow);
    mainLayout->addWidget(percentGroup);
    
    // Subset names
    QGroupBox* namesGroup = new QGroupBox(tr("Subset Names"), this);
    QFormLayout* namesLayout = new QFormLayout(namesGroup);
    
    trainingNameEdit_ = new QLineEdit("training", this);
    validationNameEdit_ = new QLineEdit("validation", this);
    testNameEdit_ = new QLineEdit("test", this);
    
    namesLayout->addRow(tr("Training:"), trainingNameEdit_);
    namesLayout->addRow(tr("Validation:"), validationNameEdit_);
    namesLayout->addRow(tr("Test:"), testNameEdit_);
    mainLayout->addWidget(namesGroup);
    
    // Options
    QGroupBox* optionsGroup = new QGroupBox(tr("Options"), this);
    QVBoxLayout* optionsLayout = new QVBoxLayout(optionsGroup);
    
    shuffleCheck_ = new QCheckBox(tr("Shuffle samples before splitting"), this);
    shuffleCheck_->setChecked(true);
    optionsLayout->addWidget(shuffleCheck_);
    
    stratifiedCheck_ = new QCheckBox(tr("Stratified split (preserve label distribution)"), this);
    stratifiedCheck_->setChecked(false);
    connect(stratifiedCheck_, &QCheckBox::toggled,
            this, &AutoSplitDialog::onStratifiedToggled);
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

void AutoSplitDialog::onPresetChanged(int index) {
    QVariantList ratios = presetCombo_->currentData().toList();
    if (ratios.size() == 3) {
        trainingSpin_->setValue(ratios[0].toInt());
        validationSpin_->setValue(ratios[1].toInt());
        testSpin_->setValue(ratios[2].toInt());
    }
}

void AutoSplitDialog::onPercentageChanged() {
    updateSampleCounts();
}

void AutoSplitDialog::onStratifiedToggled(bool checked) {
    stratifyLabelCombo_->setEnabled(checked);
}

void AutoSplitDialog::updateSampleCounts() {
    int trainingCount = (totalSamples_ * trainingSpin_->value()) / 100;
    int validationCount = (totalSamples_ * validationSpin_->value()) / 100;
    int testCount = totalSamples_ - trainingCount - validationCount; // Remainder goes to test
    
    trainingCountLabel_->setText(tr("(%1 samples)").arg(trainingCount));
    validationCountLabel_->setText(tr("(%1 samples)").arg(validationCount));
    testCountLabel_->setText(tr("(%1 samples)").arg(testCount));
    
    // Validate total
    int total = trainingSpin_->value() + validationSpin_->value() + testSpin_->value();
    if (total > 100) {
        trainingCountLabel_->setStyleSheet("color: red;");
        validationCountLabel_->setStyleSheet("color: red;");
        testCountLabel_->setStyleSheet("color: red;");
    } else {
        trainingCountLabel_->setStyleSheet("");
        validationCountLabel_->setStyleSheet("");
        testCountLabel_->setStyleSheet("");
    }
}

AutoSplitDialog::SplitConfig AutoSplitDialog::getConfig() const {
    SplitConfig config;
    config.trainingName = trainingNameEdit_->text();
    config.validationName = validationNameEdit_->text();
    config.testName = testNameEdit_->text();
    config.trainingPercent = trainingSpin_->value();
    config.validationPercent = validationSpin_->value();
    config.testPercent = testSpin_->value();
    config.stratified = stratifiedCheck_->isChecked();
    config.stratifyLabel = stratifyLabelCombo_->currentText();
    config.shuffle = shuffleCheck_->isChecked();
    return config;
}

}
