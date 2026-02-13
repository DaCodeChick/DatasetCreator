#include "MainWindow.h"
#include "DatasetView.h"
#include "SamplePreview.h"
#include "MetadataEditor.h"
#include "SubsetDialog.h"
#include "SubsetStatsWidget.h"
#include "AutoSplitDialog.h"
#include "KFoldDialog.h"
#include "plugins/PluginManager.h"
#include "managers/ImportManager.h"
#include "managers/ExportManager.h"
#include "managers/MetadataManager.h"
#include "managers/ProjectManager.h"
#include <QMenu>
#include <QMenuBar>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QInputDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QPushButton>
#include <QStatusBar>
#include <QSet>
#include <QMap>
#include <algorithm>
#include <random>
#include <numeric>

namespace DatasetCreator {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , hasUnsavedChanges_(false)
{
    currentDataset_ = Dataset("My Dataset");
    
    pluginManager_ = new PluginManager();
    importManager_ = new ImportManager(pluginManager_, this);
    exportManager_ = new ExportManager(pluginManager_, this);
    metadataManager_ = new MetadataManager(this);
    projectManager_ = new ProjectManager(this);
    
    setupUI();
    createMenuBar();
    
    connect(importManager_, &ImportManager::sampleImported, 
            this, &MainWindow::onSampleImported);
    
    updateWindowTitle();
    resize(1200, 800);
}

MainWindow::~MainWindow() {
    delete pluginManager_;
}

void MainWindow::setupUI() {
    QWidget* central = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    
    // Top section: Dataset view and preview in horizontal splitter
    QSplitter* topSplitter = new QSplitter(Qt::Horizontal, this);
    
    // Dataset tree view on the left
    datasetView_ = new DatasetView(this);
    datasetView_->setDataset(&currentDataset_);
    topSplitter->addWidget(datasetView_);
    
    // Sample preview on the right
    samplePreview_ = new SamplePreview(this);
    topSplitter->addWidget(samplePreview_);
    
    // Set initial sizes (60% tree, 40% preview)
    topSplitter->setStretchFactor(0, 60);
    topSplitter->setStretchFactor(1, 40);
    
    // Bottom section: Metadata editor
    metadataEditor_ = new MetadataEditor(this);
    
    // Stats widget for subset distribution
    statsWidget_ = new SubsetStatsWidget(this);
    statsWidget_->setDataset(&currentDataset_);
    statsWidget_->setMinimumHeight(60);
    statsWidget_->setMaximumHeight(80);
    
    // Vertical splitter for top/bottom sections
    QSplitter* verticalSplitter = new QSplitter(Qt::Vertical, this);
    verticalSplitter->addWidget(topSplitter);
    verticalSplitter->addWidget(metadataEditor_);
    
    // Set initial sizes (70% top, 30% bottom)
    verticalSplitter->setStretchFactor(0, 70);
    verticalSplitter->setStretchFactor(1, 30);
    
    mainLayout->addWidget(verticalSplitter);
    mainLayout->addWidget(statsWidget_);
    
    // Connect signals
    connect(datasetView_, &DatasetView::sampleSelected,
            samplePreview_, &SamplePreview::showSample);
    connect(datasetView_, &DatasetView::sampleSelected,
            metadataEditor_, &MetadataEditor::editSample);
    connect(datasetView_, &DatasetView::sampleSelectedWithIndex,
            this, &MainWindow::onSampleSelectedWithIndex);
    
    connect(metadataEditor_, &MetadataEditor::tagsChanged,
            this, &MainWindow::onTagsChanged);
    connect(metadataEditor_, &MetadataEditor::labelsChanged,
            this, &MainWindow::onLabelsChanged);
    
    // Context menu actions
    connect(datasetView_, &DatasetView::moveToSubsetRequested,
            this, &MainWindow::onMoveToSubsetRequested);
    connect(datasetView_, &DatasetView::deleteSampleRequested,
            this, &MainWindow::onDeleteSampleRequested);
    connect(datasetView_, &DatasetView::batchMoveToSubsetRequested,
            this, &MainWindow::onBatchMoveToSubsetRequested);
    
    // Drag-drop actions
    connect(datasetView_, &DatasetView::sampleDraggedToSubset,
            this, &MainWindow::onSampleDraggedToSubset);
    connect(datasetView_, &DatasetView::sampleDraggedToRoot,
            this, &MainWindow::onSampleDraggedToRoot);
    
    // Toolbar actions
    connect(datasetView_, &DatasetView::addSubsetRequested,
            this, &MainWindow::onAddSubsetFromToolbar);
    connect(datasetView_, &DatasetView::deleteSubsetRequested,
            this, &MainWindow::onDeleteSubsetFromToolbar);
    connect(datasetView_, &DatasetView::importFilesRequested,
            this, &MainWindow::onImportFilesFromToolbar);
    connect(datasetView_, &DatasetView::deleteSamplesRequested,
            this, &MainWindow::onDeleteSamplesFromToolbar);
    
    setCentralWidget(central);
    
    // Status bar
    statusBar()->showMessage("Ready - " + 
        QString::number(currentDataset_.totalSampleCount()) + " samples loaded");
}

void MainWindow::createMenuBar() {
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    
    // Project operations
    QAction* newAction = fileMenu->addAction(tr("&New Project"), this, &MainWindow::onNewProject);
    newAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_N));
    
    QAction* openAction = fileMenu->addAction(tr("&Open Project..."), this, &MainWindow::onOpenProject);
    openAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_O));
    
    QAction* saveAction = fileMenu->addAction(tr("&Save Project"), this, &MainWindow::onSaveProject);
    saveAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_S));
    
    QAction* saveAsAction = fileMenu->addAction(tr("Save Project &As..."), this, &MainWindow::onSaveProjectAs);
    saveAsAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S));
    
    fileMenu->addSeparator();
    
    // Import/Export operations
    QAction* importAction = fileMenu->addAction(tr("&Import Files..."), this, &MainWindow::onImportFiles);
    importAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_I));
    
    QAction* exportAction = fileMenu->addAction(tr("&Export Dataset..."), this, &MainWindow::onExportDataset);
    exportAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_E));
    
    fileMenu->addSeparator();
    fileMenu->addAction(tr("E&xit"), this, &QWidget::close);
    
    // Dataset menu
    QMenu* datasetMenu = menuBar()->addMenu(tr("&Dataset"));
    
    QAction* autoSplitAction = datasetMenu->addAction(tr("&Auto Split..."), this, &MainWindow::onAutoSplit);
    autoSplitAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S));
    
    QAction* kfoldAction = datasetMenu->addAction(tr("&K-Fold Cross-Validation..."), this, &MainWindow::onKFoldSplit);
    kfoldAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_K));
    
    datasetMenu->addSeparator();
    
    undoAction_ = datasetMenu->addAction(tr("&Undo Split"), this, &MainWindow::onUndoSplit);
    undoAction_->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Z));
    undoAction_->setEnabled(false);
}

void MainWindow::onImportFiles() {
    QStringList files = QFileDialog::getOpenFileNames(
        this, tr("Import Files"), QString(),
        tr("All Supported Files (*.*)")
    );
    
    if (!files.isEmpty()) {
        importManager_->importBatch(files);
    }
}

void MainWindow::onExportDataset() {
    QString fileName = QFileDialog::getSaveFileName(
        this, tr("Export Dataset"), QString(),
        tr("JSONL (*.jsonl);;JSON (*.json);;CSV (*.csv)")
    );
    
    if (!fileName.isEmpty()) {
        QString format = "jsonl";
        if (fileName.endsWith(".json")) format = "json";
        else if (fileName.endsWith(".csv")) format = "csv";
        
        bool success = exportManager_->exportDataset(currentDataset_, fileName, format);
        if (success) {
            QMessageBox::information(this, tr("Success"), 
                                   tr("Dataset exported successfully!"));
        }
    }
}

void MainWindow::onSampleImported(const DatasetSample& sample) {
    currentDataset_.addSample(sample);
    datasetView_->addSample(sample);
    statsWidget_->refresh();
    setUnsavedChanges(true);
    
    statusBar()->showMessage(
        tr("Imported: %1 (Total: %2 samples)")
            .arg(sample.metadata().sourceFile)
            .arg(currentDataset_.totalSampleCount())
    );
}

void MainWindow::onSampleSelectedWithIndex(const DatasetSample& sample, int index) {
    currentSampleIndex_ = index;
}

void MainWindow::onTagsChanged(const QStringList& tags) {
    if (currentSampleIndex_ < 0) return;
    
    if (currentDataset_.updateSampleTags(currentSampleIndex_, tags)) {
        datasetView_->refresh();
        statusBar()->showMessage(tr("Tags updated for sample %1").arg(currentSampleIndex_));
    }
}

void MainWindow::onLabelsChanged(const QStringList& labels) {
    if (currentSampleIndex_ < 0) return;
    
    // Parse labels from string format "key:value" to QVariantMap
    QVariantMap labelsMap;
    for (const QString& label : labels) {
        QStringList parts = label.split(":", Qt::SkipEmptyParts);
        if (parts.size() >= 2) {
            QString key = parts[0].trimmed();
            QString value = parts.mid(1).join(":").trimmed(); // Handle colons in values
            labelsMap[key] = value;
        }
    }
    
    if (currentDataset_.updateSampleLabels(currentSampleIndex_, labelsMap)) {
        datasetView_->refresh();
        statusBar()->showMessage(tr("Labels updated for sample %1").arg(currentSampleIndex_));
    }
}

void MainWindow::onMoveToSubsetRequested(int sampleIndex) {
    if (sampleIndex < 0 || sampleIndex >= currentDataset_.sampleCount()) {
        QMessageBox::warning(this, tr("Error"), tr("Invalid sample index"));
        return;
    }
    
    // Show subset selection dialog
    SubsetDialog dialog(currentDataset_.subsetNames(), this);
    if (dialog.exec() == QDialog::Accepted) {
        QString subsetName = dialog.getSubsetName();
        
        if (subsetName.isEmpty()) {
            QMessageBox::warning(this, tr("Error"), tr("Subset name cannot be empty"));
            return;
        }
        
        // Move sample to subset
        currentDataset_.moveSampleToSubset(sampleIndex, subsetName);
        
        // Refresh views
        datasetView_->refresh();
        statsWidget_->refresh();
        
        statusBar()->showMessage(tr("Moved sample to subset '%1'").arg(subsetName));
    }
}

void MainWindow::onDeleteSampleRequested(int sampleIndex) {
    if (sampleIndex < 0 || sampleIndex >= currentDataset_.sampleCount()) {
        QMessageBox::warning(this, tr("Error"), tr("Invalid sample index"));
        return;
    }
    
    // Confirm deletion
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, tr("Delete Sample"),
        tr("Are you sure you want to delete this sample?"),
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        currentDataset_.removeSample(sampleIndex);
        datasetView_->refresh();
        statsWidget_->refresh();
        
        // Clear selection
        currentSampleIndex_ = -1;
        
        statusBar()->showMessage(tr("Sample deleted. Total: %1 samples")
            .arg(currentDataset_.totalSampleCount()));
    }
}

void MainWindow::onBatchMoveToSubsetRequested(const QList<int>& sampleIndices) {
    if (sampleIndices.isEmpty()) {
        return;
    }
    
    // Show subset selection dialog
    SubsetDialog dialog(currentDataset_.subsetNames(), this);
    if (dialog.exec() == QDialog::Accepted) {
        QString subsetName = dialog.getSubsetName();
        
        if (subsetName.isEmpty()) {
            QMessageBox::warning(this, tr("Error"), tr("Subset name cannot be empty"));
            return;
        }
        
        // Move samples to subset (in reverse order to preserve indices)
        QList<int> sortedIndices = sampleIndices;
        std::sort(sortedIndices.begin(), sortedIndices.end(), std::greater<int>());
        
        for (int sampleIndex : sortedIndices) {
            if (sampleIndex >= 0 && sampleIndex < currentDataset_.sampleCount()) {
                currentDataset_.moveSampleToSubset(sampleIndex, subsetName);
            }
        }
        
        // Refresh views
        datasetView_->refresh();
        statsWidget_->refresh();
        
        statusBar()->showMessage(tr("Moved %1 samples to subset '%2'")
            .arg(sampleIndices.size()).arg(subsetName));
    }
}

void MainWindow::onSampleDraggedToSubset(const QString& sampleId, const QString& subsetName) {
    if (sampleId.isEmpty() || subsetName.isEmpty()) {
        return;
    }
    
    // Find the sample by ID in the flat samples list
    int sampleIndex = -1;
    for (int i = 0; i < currentDataset_.samples().size(); ++i) {
        if (currentDataset_.samples()[i].metadata().id == sampleId) {
            sampleIndex = i;
            break;
        }
    }
    
    if (sampleIndex >= 0) {
        // Move sample from root to subset
        currentDataset_.moveSampleToSubset(sampleIndex, subsetName);
        datasetView_->refresh();
        statsWidget_->refresh();
        statusBar()->showMessage(tr("Moved sample to subset '%1'").arg(subsetName));
        return;
    }
    
    // If not found in root, check if it's in a subset and needs to be moved
    for (const auto& subset : currentDataset_.subsets()) {
        for (int i = 0; i < subset.samples().size(); ++i) {
            if (subset.samples()[i].metadata().id == sampleId) {
                // Sample found in subset, move it to different subset
                // First move to root, then to target subset
                currentDataset_.moveSampleFromSubset(subset.name(), i);
                
                // Now find it in root and move to target
                for (int j = 0; j < currentDataset_.samples().size(); ++j) {
                    if (currentDataset_.samples()[j].metadata().id == sampleId) {
                        currentDataset_.moveSampleToSubset(j, subsetName);
                        break;
                    }
                }
                
                datasetView_->refresh();
                statsWidget_->refresh();
                statusBar()->showMessage(tr("Moved sample from '%1' to '%2'")
                    .arg(subset.name()).arg(subsetName));
                return;
            }
        }
    }
}

void MainWindow::onSampleDraggedToRoot(const QString& sampleId) {
    if (sampleId.isEmpty()) {
        return;
    }
    
    // Find which subset contains this sample
    for (const auto& subset : currentDataset_.subsets()) {
        for (int i = 0; i < subset.samples().size(); ++i) {
            if (subset.samples()[i].metadata().id == sampleId) {
                // Move sample from subset back to root
                currentDataset_.moveSampleFromSubset(subset.name(), i);
                datasetView_->refresh();
                statsWidget_->refresh();
                statusBar()->showMessage(tr("Moved sample from '%1' back to root")
                    .arg(subset.name()));
                return;
            }
        }
    }
}

void MainWindow::onAutoSplit() {
    // Check if we have samples to split
    if (currentDataset_.sampleCount() == 0) {
        QMessageBox::warning(this, tr("No Samples"), 
            tr("Cannot perform auto-split: no samples in the dataset."));
        return;
    }
    
    // Collect all available label keys from dataset
    QSet<QString> labelKeysSet;
    for (const auto& sample : currentDataset_.samples()) {
        for (const QString& key : sample.metadata().labels.keys()) {
            labelKeysSet.insert(key);
        }
    }
    QStringList availableLabels = labelKeysSet.values();
    availableLabels.sort();
    
    // Show auto-split dialog
    AutoSplitDialog dialog(currentDataset_.sampleCount(), availableLabels, this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
    
    // Save state before split for undo
    saveStateBeforeSplit();
    
    // Get split configuration
    AutoSplitDialog::SplitConfig config = dialog.getConfig();
    
    // Collect all samples from root
    QList<DatasetSample> samples = currentDataset_.samples();
    if (samples.isEmpty()) {
        QMessageBox::warning(this, tr("No Samples"), 
            tr("Cannot perform auto-split: no samples in the root dataset."));
        return;
    }
    
    // Create indices for splitting
    std::vector<int> indices(samples.size());
    std::iota(indices.begin(), indices.end(), 0);
    
    // Apply stratified sampling if requested
    if (config.stratified && !config.stratifyLabel.isEmpty()) {
        // Group samples by label value
        QMap<QString, std::vector<int>> labelGroups;
        for (int i = 0; i < samples.size(); ++i) {
            QVariant labelValue = samples[i].metadata().labels.value(config.stratifyLabel);
            QString labelStr = labelValue.toString();
            labelGroups[labelStr].push_back(i);
        }
        
        // Shuffle within each group if requested
        if (config.shuffle) {
            std::random_device rd;
            std::mt19937 g(rd());
            for (auto& group : labelGroups) {
                std::shuffle(group.begin(), group.end(), g);
            }
        }
        
        // Distribute samples from each group proportionally
        indices.clear();
        
        for (const auto& group : labelGroups) {
            int groupSize = group.size();
            int groupTrainingSize = static_cast<int>(groupSize * config.trainingPercent / 100.0);
            int groupValidationSize = static_cast<int>(groupSize * config.validationPercent / 100.0);
            
            // Add indices for this group in order: training, validation, test
            for (int i = 0; i < groupSize; ++i) {
                indices.push_back(group[i]);
            }
        }
    } else {
        // Non-stratified: shuffle if requested
        if (config.shuffle) {
            std::random_device rd;
            std::mt19937 g(rd());
            std::shuffle(indices.begin(), indices.end(), g);
        }
    }
    
    // Calculate split sizes
    int totalSamples = samples.size();
    int trainingSize = static_cast<int>(totalSamples * config.trainingPercent / 100.0);
    int validationSize = static_cast<int>(totalSamples * config.validationPercent / 100.0);
    int testSize = totalSamples - trainingSize - validationSize; // Remaining samples
    
    // Ensure we have valid split sizes
    if (trainingSize < 0) trainingSize = 0;
    if (validationSize < 0) validationSize = 0;
    if (testSize < 0) testSize = 0;
    
    // Create subsets if they don't exist and names are provided
    if (!config.trainingName.isEmpty() && !currentDataset_.subsetNames().contains(config.trainingName)) {
        currentDataset_.addSubset(DatasetSubset(config.trainingName));
    }
    if (!config.validationName.isEmpty() && !currentDataset_.subsetNames().contains(config.validationName)) {
        currentDataset_.addSubset(DatasetSubset(config.validationName));
    }
    if (!config.testName.isEmpty() && !currentDataset_.subsetNames().contains(config.testName)) {
        currentDataset_.addSubset(DatasetSubset(config.testName));
    }
    
    // Move samples according to split (process in reverse order to preserve indices)
    std::vector<std::pair<int, QString>> moves; // index, target subset
    
    int currentIndex = 0;
    for (int i = 0; i < trainingSize && currentIndex < static_cast<int>(indices.size()); ++i, ++currentIndex) {
        moves.push_back(std::make_pair(indices[currentIndex], config.trainingName));
    }
    for (int i = 0; i < validationSize && currentIndex < static_cast<int>(indices.size()); ++i, ++currentIndex) {
        moves.push_back(std::make_pair(indices[currentIndex], config.validationName));
    }
    for (int i = 0; i < testSize && currentIndex < static_cast<int>(indices.size()); ++i, ++currentIndex) {
        moves.push_back(std::make_pair(indices[currentIndex], config.testName));
    }
    
    // Sort by index in descending order to preserve indices during moves
    std::sort(moves.begin(), moves.end(), 
        [](const auto& a, const auto& b) { return a.first > b.first; });
    
    // Perform the moves
    for (const auto& move : moves) {
        if (!move.second.isEmpty()) {
            currentDataset_.moveSampleToSubset(move.first, move.second);
        }
    }
    
    // Refresh views
    datasetView_->refresh();
    statsWidget_->refresh();
    
    // Show success message
    QString message = tr("Auto-split completed:\n");
    if (trainingSize > 0) message += tr("- %1: %2 samples\n").arg(config.trainingName).arg(trainingSize);
    if (validationSize > 0) message += tr("- %1: %2 samples\n").arg(config.validationName).arg(validationSize);
    if (testSize > 0) message += tr("- %1: %2 samples").arg(config.testName).arg(testSize);
    
    QMessageBox::information(this, tr("Auto Split Complete"), message);
    
    statusBar()->showMessage(tr("Auto-split completed: %1 samples distributed")
        .arg(totalSamples));
}

void MainWindow::onKFoldSplit() {
    // Check if we have samples to split
    if (currentDataset_.sampleCount() == 0) {
        QMessageBox::warning(this, tr("No Samples"), 
            tr("Cannot perform K-Fold split: no samples in the dataset."));
        return;
    }
    
    // Collect all available label keys from dataset
    QSet<QString> labelKeysSet;
    for (const auto& sample : currentDataset_.samples()) {
        for (const QString& key : sample.metadata().labels.keys()) {
            labelKeysSet.insert(key);
        }
    }
    QStringList availableLabels = labelKeysSet.values();
    availableLabels.sort();
    
    // Show K-Fold dialog
    KFoldDialog dialog(currentDataset_.sampleCount(), availableLabels, this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
    
    // Save state before split for undo
    saveStateBeforeSplit();
    
    // Get K-Fold configuration
    KFoldDialog::KFoldConfig config = dialog.getConfig();
    
    // Collect all samples from root
    QList<DatasetSample> samples = currentDataset_.samples();
    if (samples.isEmpty()) {
        QMessageBox::warning(this, tr("No Samples"), 
            tr("Cannot perform K-Fold split: no samples in the root dataset."));
        return;
    }
    
    int totalSamples = samples.size();
    int numFolds = config.folds;
    
    // Check if we have enough samples
    if (totalSamples < numFolds) {
        QMessageBox::warning(this, tr("Insufficient Samples"), 
            tr("Cannot perform %1-Fold split: need at least %1 samples, but only %2 available.")
                .arg(numFolds).arg(totalSamples));
        return;
    }
    
    // Create indices for splitting
    std::vector<int> indices(totalSamples);
    std::iota(indices.begin(), indices.end(), 0);
    
    // Apply stratified K-Fold if requested
    if (config.stratified && !config.stratifyLabel.isEmpty()) {
        // Group samples by label value
        QMap<QString, std::vector<int>> labelGroups;
        for (int i = 0; i < samples.size(); ++i) {
            QVariant labelValue = samples[i].metadata().labels.value(config.stratifyLabel);
            QString labelStr = labelValue.toString();
            labelGroups[labelStr].push_back(i);
        }
        
        // Shuffle within each group if requested
        if (config.shuffle) {
            std::random_device rd;
            std::mt19937 g(rd());
            for (auto& group : labelGroups) {
                std::shuffle(group.begin(), group.end(), g);
            }
        }
        
        // Distribute samples from each group to create stratified folds
        std::vector<std::vector<int>> foldIndices(numFolds);
        
        for (const auto& group : labelGroups) {
            // Distribute this group's samples evenly across folds
            for (int i = 0; i < static_cast<int>(group.size()); ++i) {
                int foldIndex = i % numFolds;
                foldIndices[foldIndex].push_back(group[i]);
            }
        }
        
        // Flatten back to indices array
        indices.clear();
        for (int fold = 0; fold < numFolds; ++fold) {
            for (int idx : foldIndices[fold]) {
                indices.push_back(idx);
            }
        }
    } else {
        // Non-stratified: shuffle if requested
        if (config.shuffle) {
            std::random_device rd;
            std::mt19937 g(rd());
            std::shuffle(indices.begin(), indices.end(), g);
        }
    }
    
    // Calculate fold size
    int baseFoldSize = totalSamples / numFolds;
    int remainder = totalSamples % numFolds;
    
    // Create K fold subsets
    for (int fold = 0; fold < numFolds; ++fold) {
        QString foldName = QString("%1%2").arg(config.prefixName).arg(fold + 1);
        
        // Create subset if it doesn't exist
        if (!currentDataset_.subsetNames().contains(foldName)) {
            currentDataset_.addSubset(DatasetSubset(foldName));
        }
    }
    
    // Assign samples to folds (process in reverse order to preserve indices)
    std::vector<std::pair<int, QString>> moves; // index, target subset
    
    int currentIndex = 0;
    for (int fold = 0; fold < numFolds; ++fold) {
        int thisFoldSize = baseFoldSize + (fold < remainder ? 1 : 0);
        QString foldName = QString("%1%2").arg(config.prefixName).arg(fold + 1);
        
        // Mark samples for this fold
        for (int i = 0; i < thisFoldSize; ++i) {
            moves.push_back(std::make_pair(indices[currentIndex + i], foldName));
        }
        
        currentIndex += thisFoldSize;
    }
    
    // Sort by index in descending order to preserve indices during moves
    std::sort(moves.begin(), moves.end(), 
        [](const auto& a, const auto& b) { return a.first > b.first; });
    
    // Perform the moves
    for (const auto& move : moves) {
        currentDataset_.moveSampleToSubset(move.first, move.second);
    }
    
    // Refresh views
    datasetView_->refresh();
    statsWidget_->refresh();
    
    // Show success message with fold sizes
    QString message = tr("K-Fold split completed:\n");
    message += tr("- %1 folds created\n").arg(numFolds);
    if (remainder > 0) {
        message += tr("- Fold sizes: %1 folds with %2 samples, %3 folds with %4 samples\n")
            .arg(remainder).arg(baseFoldSize + 1).arg(numFolds - remainder).arg(baseFoldSize);
    } else {
        message += tr("- Each fold contains %1 samples\n").arg(baseFoldSize);
    }
    message += tr("\nUse each fold as test set and combine others for training.");
    
    QMessageBox::information(this, tr("K-Fold Split Complete"), message);
    
    statusBar()->showMessage(tr("K-Fold split completed: %1 folds, %2 samples distributed")
        .arg(numFolds).arg(totalSamples));
}

void MainWindow::saveStateBeforeSplit() {
    SplitCommand command;
    
    // Save current state: all samples in root
    for (const auto& sample : currentDataset_.samples()) {
        command.addOriginalLocation(sample.metadata().id, ""); // Empty string = root
    }
    
    // Save samples in subsets
    for (const auto& subset : currentDataset_.subsets()) {
        for (const auto& sample : subset.samples()) {
            command.addOriginalLocation(sample.metadata().id, subset.name());
        }
    }
    
    command.setDescription("Split operation");
    undoStack_.push(command);
    
    // Enable undo action
    if (undoAction_) {
        undoAction_->setEnabled(true);
    }
}

void MainWindow::onUndoSplit() {
    if (undoStack_.isEmpty()) {
        QMessageBox::information(this, tr("No Action to Undo"),
            tr("There are no split operations to undo."));
        return;
    }
    
    SplitCommand command = undoStack_.pop();
    
    // Clear all subsets
    QStringList subsetNames = currentDataset_.subsetNames();
    for (const QString& name : subsetNames) {
        // Move all samples from subset back to root first
        const DatasetSubset* subset = currentDataset_.getSubset(name);
        if (subset) {
            int sampleCount = subset->samples().size();
            for (int i = sampleCount - 1; i >= 0; --i) {
                currentDataset_.moveSampleFromSubset(name, i);
            }
        }
    }
    
    // Now restore original locations
    for (const auto& location : command.originalLocations()) {
        const QString& sampleId = location.first;
        const QString& subsetName = location.second;
        
        if (subsetName.isEmpty()) {
            // Sample should be in root - it already is, do nothing
            continue;
        }
        
        // Find sample in root by ID
        const auto& rootSamples = currentDataset_.samples();
        for (int i = 0; i < rootSamples.size(); ++i) {
            if (rootSamples[i].metadata().id == sampleId) {
                // Create subset if it doesn't exist
                if (!currentDataset_.subsetNames().contains(subsetName)) {
                    currentDataset_.addSubset(DatasetSubset(subsetName));
                }
                // Move sample to original subset
                currentDataset_.moveSampleToSubset(i, subsetName);
                break;
            }
        }
    }
    
    // Refresh views
    datasetView_->refresh();
    statsWidget_->refresh();
    
    // Disable undo action if stack is empty
    if (undoStack_.isEmpty() && undoAction_) {
        undoAction_->setEnabled(false);
    }
    
    statusBar()->showMessage(tr("Undo split operation completed"));
}

void MainWindow::onAddSubsetFromToolbar() {
    bool ok;
    QString subsetName = QInputDialog::getText(this, tr("Add Subset"),
        tr("Enter subset name:"), QLineEdit::Normal, "", &ok);
    
    if (ok && !subsetName.isEmpty()) {
        if (currentDataset_.subsetNames().contains(subsetName)) {
            QMessageBox::warning(this, tr("Duplicate Name"),
                tr("A subset with name '%1' already exists.").arg(subsetName));
            return;
        }
        
        currentDataset_.addSubset(DatasetSubset(subsetName));
        datasetView_->refresh();
        statsWidget_->refresh();
        statusBar()->showMessage(tr("Added subset '%1'").arg(subsetName));
    }
}

void MainWindow::onDeleteSubsetFromToolbar(const QString& subsetName) {
    if (subsetName.isEmpty()) return;
    
    QMessageBox::StandardButton reply = QMessageBox::question(this,
        tr("Delete Subset"),
        tr("Are you sure you want to delete subset '%1'?\nSamples will be moved back to root.")
            .arg(subsetName),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        // Move all samples from subset back to root first
        const DatasetSubset* subset = currentDataset_.getSubset(subsetName);
        if (subset) {
            int sampleCount = subset->samples().size();
            for (int i = sampleCount - 1; i >= 0; --i) {
                currentDataset_.moveSampleFromSubset(subsetName, i);
            }
        }
        
        // Remove the subset
        currentDataset_.removeSubset(subsetName);
        
        datasetView_->refresh();
        statsWidget_->refresh();
        statusBar()->showMessage(tr("Deleted subset '%1'").arg(subsetName));
    }
}

void MainWindow::onImportFilesFromToolbar() {
    // Reuse existing import functionality
    onImportFiles();
}

void MainWindow::onDeleteSamplesFromToolbar() {
    // Get selected sample indices from DatasetView
    QList<int> selectedIndices = datasetView_->getSelectedSampleIndices();
    
    if (selectedIndices.isEmpty()) {
        return;
    }
    
    // Confirm deletion
    QString message;
    if (selectedIndices.size() == 1) {
        message = tr("Are you sure you want to delete 1 sample?");
    } else {
        message = tr("Are you sure you want to delete %1 samples?").arg(selectedIndices.size());
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(this,
        tr("Delete Sample(s)"),
        message,
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        // Sort in descending order to delete from end to start (avoids index shifting issues)
        std::sort(selectedIndices.begin(), selectedIndices.end(), std::greater<int>());
        
        // Delete each sample
        for (int index : selectedIndices) {
            onDeleteSampleRequested(index);
        }
        
        // Refresh views
        datasetView_->refresh();
        statsWidget_->refresh();
        statusBar()->showMessage(tr("Deleted %1 sample(s)").arg(selectedIndices.size()));
    }
}

// Project Management

void MainWindow::onNewProject() {
    if (!promptSaveChanges()) {
        return;  // User cancelled
    }
    
    currentDataset_ = Dataset("New Dataset");
    currentProjectPath_.clear();
    hasUnsavedChanges_ = false;
    
    datasetView_->refresh();
    statsWidget_->refresh();
    samplePreview_->clear();
    metadataEditor_->clear();
    
    updateWindowTitle();
    statusBar()->showMessage(tr("New project created"));
}

void MainWindow::onOpenProject() {
    if (!promptSaveChanges()) {
        return;  // User cancelled
    }
    
    QString fileName = QFileDialog::getOpenFileName(
        this, tr("Open Project"), QString(),
        tr("Dataset Creator Project (*.dscp);;All Files (*.*)")
    );
    
    if (fileName.isEmpty()) {
        return;
    }
    
    Dataset loadedDataset;
    if (projectManager_->loadProject(fileName, loadedDataset)) {
        currentDataset_ = loadedDataset;
        currentProjectPath_ = fileName;
        hasUnsavedChanges_ = false;
        
        datasetView_->refresh();
        statsWidget_->refresh();
        samplePreview_->clear();
        metadataEditor_->clear();
        
        updateWindowTitle();
        statusBar()->showMessage(tr("Project loaded: %1").arg(fileName));
    } else {
        QMessageBox::warning(this, tr("Load Error"),
            tr("Failed to load project: %1").arg(projectManager_->lastError()));
    }
}

void MainWindow::onSaveProject() {
    if (currentProjectPath_.isEmpty()) {
        onSaveProjectAs();
        return;
    }
    
    if (projectManager_->saveProject(currentDataset_, currentProjectPath_)) {
        hasUnsavedChanges_ = false;
        updateWindowTitle();
        statusBar()->showMessage(tr("Project saved: %1").arg(currentProjectPath_));
    } else {
        QMessageBox::warning(this, tr("Save Error"),
            tr("Failed to save project: %1").arg(projectManager_->lastError()));
    }
}

void MainWindow::onSaveProjectAs() {
    QString fileName = QFileDialog::getSaveFileName(
        this, tr("Save Project As"), QString(),
        tr("Dataset Creator Project (*.dscp)")
    );
    
    if (fileName.isEmpty()) {
        return;
    }
    
    // Add extension if not present
    if (!fileName.endsWith(".dscp", Qt::CaseInsensitive)) {
        fileName += ".dscp";
    }
    
    if (projectManager_->saveProject(currentDataset_, fileName)) {
        currentProjectPath_ = fileName;
        hasUnsavedChanges_ = false;
        updateWindowTitle();
        statusBar()->showMessage(tr("Project saved: %1").arg(fileName));
    } else {
        QMessageBox::warning(this, tr("Save Error"),
            tr("Failed to save project: %1").arg(projectManager_->lastError()));
    }
}

void MainWindow::setUnsavedChanges(bool hasChanges) {
    hasUnsavedChanges_ = hasChanges;
    updateWindowTitle();
}

void MainWindow::updateWindowTitle() {
    QString title = "Dataset Creator";
    
    if (!currentProjectPath_.isEmpty()) {
        QFileInfo fileInfo(currentProjectPath_);
        title += " - " + fileInfo.fileName();
    } else if (!currentDataset_.metadata().name.isEmpty()) {
        title += " - " + currentDataset_.metadata().name;
    }
    
    if (hasUnsavedChanges_) {
        title += " *";
    }
    
    setWindowTitle(title);
}

bool MainWindow::promptSaveChanges() {
    if (!hasUnsavedChanges_) {
        return true;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(this,
        tr("Unsaved Changes"),
        tr("You have unsaved changes. Do you want to save them?"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    
    if (reply == QMessageBox::Save) {
        onSaveProject();
        return !hasUnsavedChanges_;  // Return false if save failed
    } else if (reply == QMessageBox::Discard) {
        return true;
    } else {
        return false;  // Cancel
    }
}

}

