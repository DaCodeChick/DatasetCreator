#pragma once
#include <QDialog>
#include <QSpinBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QStringList>

namespace DatasetCreator {

/**
 * @brief Dialog for generating K-Fold cross-validation splits
 * Creates K train/test subset pairs for cross-validation
 */
class KFoldDialog : public QDialog {
    Q_OBJECT
public:
    explicit KFoldDialog(int totalSamples, const QStringList& availableLabels, QWidget* parent = nullptr);
    
    struct KFoldConfig {
        int folds;                    // Number of folds (default: 5)
        QString prefixName;           // Prefix for fold names (e.g., "fold")
        bool stratified;              // Stratify by label
        QString stratifyLabel;        // Label to stratify by
        bool shuffle;                 // Shuffle before splitting
    };
    
    KFoldConfig getConfig() const;
    
private slots:
    void onFoldsChanged(int value);
    void onStratifiedToggled(bool checked);
    
private:
    void setupUI();
    void updateFoldCounts();
    
    int totalSamples_;
    
    // UI elements
    QSpinBox* foldsSpin_;
    QLineEdit* prefixEdit_;
    QCheckBox* stratifiedCheck_;
    QComboBox* stratifyLabelCombo_;
    QCheckBox* shuffleCheck_;
    
    QLabel* foldSizeLabel_;
};

}
