#include "DatasetView.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QStandardItem>

namespace DatasetCreator {

DatasetView::DatasetView(QWidget* parent) 
    : QWidget(parent), dataset_(nullptr) 
{
    setupUI();
}

void DatasetView::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    
    treeView_ = new QTreeView(this);
    model_ = new QStandardItemModel(this);
    
    // Set column headers
    model_->setHorizontalHeaderLabels({"Name", "Type", "Size", "Tags", "Labels"});
    
    treeView_->setModel(model_);
    treeView_->setAlternatingRowColors(true);
    treeView_->setSelectionMode(QAbstractItemView::SingleSelection);
    treeView_->header()->setStretchLastSection(false);
    treeView_->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    
    connect(treeView_, &QTreeView::clicked, this, &DatasetView::onItemClicked);
    
    layout->addWidget(treeView_);
}

void DatasetView::setDataset(Dataset* dataset) {
    dataset_ = dataset;
    refresh();
}

void DatasetView::refresh() {
    model_->removeRows(0, model_->rowCount());
    
    if (!dataset_) return;
    
    populateTree();
}

void DatasetView::populateTree() {
    // Add root dataset item
    QStandardItem* rootItem = model_->invisibleRootItem();
    
    // Check if dataset has subsets
    if (dataset_->subsets().isEmpty()) {
        // Flat structure: add samples directly
        const auto& samples = dataset_->samples();
        for (int i = 0; i < samples.size(); ++i) {
            const auto& sample = samples[i];
            
            QList<QStandardItem*> row;
            
            // Name column
            QStandardItem* nameItem = new QStandardItem(sample.metadata().id);
            nameItem->setData(i, Qt::UserRole); // Store sample index
            nameItem->setData(false, Qt::UserRole + 1); // Not a subset
            row.append(nameItem);
            
            // Type column
            QString typeStr;
            switch (sample.type()) {
                case SampleType::Text: typeStr = "Text"; break;
                case SampleType::Image: typeStr = "Image"; break;
                case SampleType::Audio: typeStr = "Audio"; break;
                case SampleType::Binary: typeStr = "Binary"; break;
                case SampleType::Multimodal: typeStr = "Multimodal"; break;
            }
            row.append(new QStandardItem(typeStr));
            
            // Size column
            QString sizeStr = QString::number(sample.dataSize()) + " bytes";
            row.append(new QStandardItem(sizeStr));
            
            // Tags column
            row.append(new QStandardItem(sample.metadata().tags.join(", ")));
            
            // Labels column (convert QVariantMap to string)
            QString labelsStr;
            QVariantMap labels = sample.metadata().labels;
            for (auto it = labels.begin(); it != labels.end(); ++it) {
                if (!labelsStr.isEmpty()) labelsStr += ", ";
                labelsStr += it.key() + ":" + it.value().toString();
            }
            row.append(new QStandardItem(labelsStr));
            
            rootItem->appendRow(row);
        }
    } else {
        // Hierarchical structure: add subsets with samples
        const auto& subsets = dataset_->subsets();
        for (const auto& subset : subsets) {
            QList<QStandardItem*> subsetRow;
            
            QStandardItem* subsetItem = new QStandardItem(subset.name());
            subsetItem->setData(-1, Qt::UserRole); // Not a sample
            subsetItem->setData(true, Qt::UserRole + 1); // Is a subset
            subsetRow.append(subsetItem);
            
            subsetRow.append(new QStandardItem("Subset"));
            subsetRow.append(new QStandardItem(QString::number(subset.samples().size()) + " samples"));
            subsetRow.append(new QStandardItem(""));
            subsetRow.append(new QStandardItem(""));
            
            // Add samples to subset
            const auto& samples = subset.samples();
            for (int i = 0; i < samples.size(); ++i) {
                const auto& sample = samples[i];
                
                QList<QStandardItem*> row;
                
                QStandardItem* nameItem = new QStandardItem(sample.metadata().id);
                nameItem->setData(i, Qt::UserRole);
                nameItem->setData(false, Qt::UserRole + 1);
                row.append(nameItem);
                
                QString typeStr;
                switch (sample.type()) {
                    case SampleType::Text: typeStr = "Text"; break;
                    case SampleType::Image: typeStr = "Image"; break;
                    case SampleType::Audio: typeStr = "Audio"; break;
                    case SampleType::Binary: typeStr = "Binary"; break;
                    case SampleType::Multimodal: typeStr = "Multimodal"; break;
                }
                row.append(new QStandardItem(typeStr));
                
                QString sizeStr = QString::number(sample.dataSize()) + " bytes";
                row.append(new QStandardItem(sizeStr));
                
                row.append(new QStandardItem(sample.metadata().tags.join(", ")));
                
                // Labels column (convert QVariantMap to string)
                QString labelsStr2;
                QVariantMap labels2 = sample.metadata().labels;
                for (auto it = labels2.begin(); it != labels2.end(); ++it) {
                    if (!labelsStr2.isEmpty()) labelsStr2 += ", ";
                    labelsStr2 += it.key() + ":" + it.value().toString();
                }
                row.append(new QStandardItem(labelsStr2));
                
                subsetItem->appendRow(row);
            }
            
            rootItem->appendRow(subsetRow);
        }
    }
    
    treeView_->expandAll();
}

void DatasetView::addSample(const DatasetSample& sample) {
    if (!dataset_) return;
    
    QStandardItem* rootItem = model_->invisibleRootItem();
    
    QList<QStandardItem*> row;
    
    // Name column
    QStandardItem* nameItem = new QStandardItem(sample.metadata().id);
    nameItem->setData(dataset_->samples().size() - 1, Qt::UserRole);
    nameItem->setData(false, Qt::UserRole + 1);
    row.append(nameItem);
    
    // Type column
    QString typeStr;
    switch (sample.type()) {
        case SampleType::Text: typeStr = "Text"; break;
        case SampleType::Image: typeStr = "Image"; break;
        case SampleType::Audio: typeStr = "Audio"; break;
        case SampleType::Binary: typeStr = "Binary"; break;
        case SampleType::Multimodal: typeStr = "Multimodal"; break;
    }
    row.append(new QStandardItem(typeStr));
    
    // Size column
    QString sizeStr = QString::number(sample.dataSize()) + " bytes";
    row.append(new QStandardItem(sizeStr));
    
    // Tags column
    row.append(new QStandardItem(sample.metadata().tags.join(", ")));
    
    // Labels column (convert QVariantMap to string)
    QString labelsStr3;
    QVariantMap labels3 = sample.metadata().labels;
    for (auto it = labels3.begin(); it != labels3.end(); ++it) {
        if (!labelsStr3.isEmpty()) labelsStr3 += ", ";
        labelsStr3 += it.key() + ":" + it.value().toString();
    }
    row.append(new QStandardItem(labelsStr3));
    
    rootItem->appendRow(row);
}

DatasetSample* DatasetView::getSelectedSample() {
    QModelIndex index = treeView_->currentIndex();
    if (!index.isValid() || !dataset_) return nullptr;
    
    QStandardItem* item = model_->itemFromIndex(index.siblingAtColumn(0));
    if (!item) return nullptr;
    
    bool isSubset = item->data(Qt::UserRole + 1).toBool();
    if (isSubset) return nullptr;
    
    int sampleIndex = item->data(Qt::UserRole).toInt();
    if (sampleIndex < 0 || sampleIndex >= dataset_->samples().size()) return nullptr;
    
    // Return pointer to sample (const_cast needed since samples() returns const ref)
    return const_cast<DatasetSample*>(&dataset_->samples()[sampleIndex]);
}

void DatasetView::onItemClicked(const QModelIndex& index) {
    if (!index.isValid() || !dataset_) return;
    
    QStandardItem* item = model_->itemFromIndex(index.siblingAtColumn(0));
    if (!item) return;
    
    bool isSubset = item->data(Qt::UserRole + 1).toBool();
    if (isSubset) return;
    
    int sampleIndex = item->data(Qt::UserRole).toInt();
    if (sampleIndex >= 0 && sampleIndex < dataset_->samples().size()) {
        emit sampleSelected(dataset_->samples()[sampleIndex]);
        emit sampleSelectedWithIndex(dataset_->samples()[sampleIndex], sampleIndex);
    }
}

}
