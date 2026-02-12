#pragma once
#include <QWidget>
#include <QTreeView>
#include "core/Dataset.h"
#include "DatasetTreeModel.h"

namespace DatasetCreator {

class DatasetView : public QWidget {
    Q_OBJECT
public:
    explicit DatasetView(QWidget* parent = nullptr);
    
    void setDataset(Dataset* dataset);
    void refresh();
    void addSample(const DatasetSample& sample);
    
    DatasetSample* getSelectedSample();
    int getSelectedSampleIndex() const;
    bool isSubsetSelected() const;
    QString getSelectedSubsetName() const;
    QList<int> getSelectedSampleIndices() const;
    
signals:
    void sampleSelected(const DatasetSample& sample);
    void sampleSelectedWithIndex(const DatasetSample& sample, int index);
    void moveToSubsetRequested(int sampleIndex);
    void deleteSampleRequested(int sampleIndex);
    void batchMoveToSubsetRequested(const QList<int>& sampleIndices);
    void sampleDraggedToSubset(const QString& sampleId, const QString& subsetName);
    void sampleDraggedToRoot(const QString& sampleId);
    
private slots:
    void onItemClicked(const QModelIndex& index);
    void showContextMenu(const QPoint& pos);
    
private:
    void setupUI();
    void populateTree();
    
    QTreeView* treeView_;
    DatasetTreeModel* model_;
    Dataset* dataset_;
};

}
