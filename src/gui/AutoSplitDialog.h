#pragma once
#include <QDialog>
#include <QSpinBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>

namespace DatasetCreator {

/**
 * @brief Dialog for automatically splitting dataset into subsets
 * Supports predefined ratios (70/20/10) or custom splits
 */
class AutoSplitDialog : public QDialog {
    Q_OBJECT
public:
    explicit AutoSplitDialog(int totalSamples, QWidget* parent = nullptr);
    
    struct SplitConfig {
        QString trainingName;
        QString validationName;
        QString testName;
        int trainingPercent;
        int validationPercent;
        int testPercent;
        bool stratified;
        QString stratifyLabel;
        bool shuffle;
    };
    
    SplitConfig getConfig() const;
    
private slots:
    void onPresetChanged(int index);
    void onPercentageChanged();
    void onStratifiedToggled(bool checked);
    
private:
    void setupUI();
    void updateSampleCounts();
    
    int totalSamples_;
    
    // UI elements
    QComboBox* presetCombo_;
    QSpinBox* trainingSpin_;
    QSpinBox* validationSpin_;
    QSpinBox* testSpin_;
    QLineEdit* trainingNameEdit_;
    QLineEdit* validationNameEdit_;
    QLineEdit* testNameEdit_;
    QCheckBox* stratifiedCheck_;
    QComboBox* stratifyLabelCombo_;
    QCheckBox* shuffleCheck_;
    
    QLabel* trainingCountLabel_;
    QLabel* validationCountLabel_;
    QLabel* testCountLabel_;
};

}
