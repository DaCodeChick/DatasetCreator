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
class SubsetStatsWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    
private slots:
    void onImportFiles();
    void onExportDataset();
    void onAutoSplit();
    void onSampleImported(const DatasetSample& sample);
    void onSampleSelectedWithIndex(const DatasetSample& sample, int index);
    void onTagsChanged(const QStringList& tags);
    void onLabelsChanged(const QStringList& labels);
    void onMoveToSubsetRequested(int sampleIndex);
    void onDeleteSampleRequested(int sampleIndex);
    void onBatchMoveToSubsetRequested(const QList<int>& sampleIndices);
    void onSampleDraggedToSubset(const QString& sampleId, const QString& subsetName);
    void onSampleDraggedToRoot(const QString& sampleId);
    
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
    SubsetStatsWidget* statsWidget_;
    
    int currentSampleIndex_ = -1;
};

}
