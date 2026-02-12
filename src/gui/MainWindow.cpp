#include "MainWindow.h"
#include "DatasetView.h"
#include "SamplePreview.h"
#include "MetadataEditor.h"
#include "SubsetDialog.h"
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
    
    // Vertical splitter for top/bottom sections
    QSplitter* verticalSplitter = new QSplitter(Qt::Vertical, this);
    verticalSplitter->addWidget(topSplitter);
    verticalSplitter->addWidget(metadataEditor_);
    
    // Set initial sizes (70% top, 30% bottom)
    verticalSplitter->setStretchFactor(0, 70);
    verticalSplitter->setStretchFactor(1, 30);
    
    mainLayout->addWidget(verticalSplitter);
    
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
    
    setCentralWidget(central);
    
    // Status bar
    statusBar()->showMessage("Ready - " + 
        QString::number(currentDataset_.totalSampleCount()) + " samples loaded");
}

void MainWindow::createMenuBar() {
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(tr("&Import Files..."), this, &MainWindow::onImportFiles);
    fileMenu->addAction(tr("&Export Dataset..."), this, &MainWindow::onExportDataset);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("E&xit"), this, &QWidget::close);
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
        
        // Refresh view
        datasetView_->refresh();
        
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
        
        // Clear selection
        currentSampleIndex_ = -1;
        
        statusBar()->showMessage(tr("Sample deleted. Total: %1 samples")
            .arg(currentDataset_.totalSampleCount()));
    }
}

}

