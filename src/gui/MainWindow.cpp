#include "MainWindow.h"
#include "DatasetView.h"
#include "SamplePreview.h"
#include "MetadataEditor.h"
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

}
