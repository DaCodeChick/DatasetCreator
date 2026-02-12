#pragma once
#include <QMainWindow>
#include "core/Dataset.h"

namespace DatasetCreator {

class PluginManager;
class ImportManager;
class ExportManager;
class MetadataManager;
class DatasetView;
class SamplePreview;
class MetadataEditor;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    
private slots:
    void onImportFiles();
    void onExportDataset();
    void onSampleImported(const DatasetSample& sample);
    
private:
    void setupUI();
    void createMenuBar();
    
    Dataset currentDataset_;
    PluginManager* pluginManager_;
    ImportManager* importManager_;
    ExportManager* exportManager_;
    MetadataManager* metadataManager_;
    
    DatasetView* datasetView_;
    SamplePreview* samplePreview_;
    MetadataEditor* metadataEditor_;
};

}
