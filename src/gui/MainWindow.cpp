#include "MainWindow.h"
#include "DatasetView.h"
#include "SamplePreview.h"
#include "MetadataEditor.h"
#include "SubsetDialog.h"
#include "SubsetStatsWidget.h"
#include "AutoSplitDialog.h"
#include "plugins/PluginManager.h"
#include "managers/ImportManager.h"
#include "managers/ExportManager.h"
#include "managers/MetadataManager.h"
#include <QMenu>
#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QPushButton>
#include <QStatusBar>
#include <algorithm>
#include <random>
#include <numeric>

namespace DatasetCreator {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    currentDataset_ = Dataset("My Dataset");
    
    pluginManager_ = new PluginManager();
    importManager_ = new ImportManager(pluginManager_, this);
    exportManager_ = new ExportManager(pluginManager_, this);
    metadataManager_ = new MetadataManager(this);
    
    setupUI();
    createMenuBar();
    
    connect(importManager_, &ImportManager::sampleImported, 
            this, &MainWindow::onSampleImported);
    
    setWindowTitle("Dataset Creator");
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
    
    setCentralWidget(central);
    
    // Status bar
    statusBar()->showMessage("Ready - " + 
        QString::number(currentDataset_.totalSampleCount()) + " samples loaded");
}

void MainWindow::createMenuBar() {
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    
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
    
    // Show auto-split dialog
    AutoSplitDialog dialog(currentDataset_.sampleCount(), this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
    
    // Get split configuration
    AutoSplitDialog::SplitConfig config = dialog.getConfig();
    
    // Collect all samples from root
    QList<DatasetSample> samples = currentDataset_.samples();
    if (samples.isEmpty()) {
        QMessageBox::warning(this, tr("No Samples"), 
            tr("Cannot perform auto-split: no samples in the root dataset."));
        return;
    }
    
    // Create indices for shuffling
    std::vector<int> indices(samples.size());
    std::iota(indices.begin(), indices.end(), 0);
    
    // Shuffle if requested
    if (config.shuffle) {
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(indices.begin(), indices.end(), g);
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

}

