#include "DatasetView.h"
#include "DatasetTreeModel.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QStandardItem>
#include <QMenu>
#include <QAction>
#include <QToolBar>
#include <QStyle>

namespace DatasetCreator {

DatasetView::DatasetView(QWidget* parent) 
    : QWidget(parent), dataset_(nullptr) 
{
    setupUI();
}

void DatasetView::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    
    // Create toolbar with icon buttons
    toolbar_ = new QToolBar(this);
    toolbar_->setIconSize(QSize(16, 16));
    toolbar_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    
    // Import Files action
    importFilesAction_ = toolbar_->addAction(
        style()->standardIcon(QStyle::SP_FileDialogStart),
        tr("Import Files")
    );
    connect(importFilesAction_, &QAction::triggered, this, &DatasetView::onImportFiles);
    
    // Delete Sample(s) action
    deleteSamplesAction_ = toolbar_->addAction(
        style()->standardIcon(QStyle::SP_DialogDiscardButton),
        tr("Delete Sample")
    );
    connect(deleteSamplesAction_, &QAction::triggered, this, &DatasetView::onDeleteSamples);
    deleteSamplesAction_->setEnabled(false);
    
    toolbar_->addSeparator();
    
    // Add Subset action
    addSubsetAction_ = toolbar_->addAction(
        style()->standardIcon(QStyle::SP_FileDialogNewFolder),
        tr("Add Subset")
    );
    connect(addSubsetAction_, &QAction::triggered, this, &DatasetView::onAddSubset);
    
    // Delete Subset action
    deleteSubsetAction_ = toolbar_->addAction(
        style()->standardIcon(QStyle::SP_TrashIcon),
        tr("Delete Subset")
    );
    connect(deleteSubsetAction_, &QAction::triggered, this, &DatasetView::onDeleteSubset);
    deleteSubsetAction_->setEnabled(false);
    
    toolbar_->addSeparator();
    
    // Expand All action
    expandAllAction_ = toolbar_->addAction(
        style()->standardIcon(QStyle::SP_TitleBarUnshadeButton),
        tr("Expand All")
    );
    connect(expandAllAction_, &QAction::triggered, this, &DatasetView::onExpandAll);
    
    // Collapse All action
    collapseAllAction_ = toolbar_->addAction(
        style()->standardIcon(QStyle::SP_TitleBarShadeButton),
        tr("Collapse All")
    );
    connect(collapseAllAction_, &QAction::triggered, this, &DatasetView::onCollapseAll);
    
    layout->addWidget(toolbar_);
    
    treeView_ = new QTreeView(this);
    model_ = new DatasetTreeModel(this);
    
    // Set column headers
    model_->setHorizontalHeaderLabels({"Name", "Type", "Size", "Tags", "Labels"});
    
    treeView_->setModel(model_);
    treeView_->setAlternatingRowColors(true);
    treeView_->setSelectionMode(QAbstractItemView::ExtendedSelection);  // Allow multi-select
    treeView_->header()->setStretchLastSection(false);
    treeView_->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    
    // Enable drag and drop
    treeView_->setDragEnabled(true);
    treeView_->setAcceptDrops(true);
    treeView_->setDropIndicatorShown(true);
    treeView_->setDragDropMode(QAbstractItemView::InternalMove);
    
    // Connect drag-drop signals from custom model
    connect(model_, &DatasetTreeModel::sampleDropped,
            this, &DatasetView::sampleDraggedToSubset);
    connect(model_, &DatasetTreeModel::sampleDroppedToRoot,
            this, &DatasetView::sampleDraggedToRoot);
    
    // Enable context menu
    treeView_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(treeView_, &QTreeView::customContextMenuRequested,
            this, &DatasetView::showContextMenu);
    
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
            
            // Name column - store sample ID and index
            QStandardItem* nameItem = new QStandardItem(sample.metadata().id);
            nameItem->setData(i, Qt::UserRole); // Store sample index
            nameItem->setData(sample.metadata().id, Qt::UserRole + 2); // Store sample ID
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
            subsetItem->setData(subset.name(), Qt::UserRole + 2); // Store subset name
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
                nameItem->setData(sample.metadata().id, Qt::UserRole + 2); // Store sample ID
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
    if (!index.isValid() || !dataset_) {
        deleteSubsetAction_->setEnabled(false);
        deleteSamplesAction_->setEnabled(false);
        return;
    }
    
    QStandardItem* item = model_->itemFromIndex(index.siblingAtColumn(0));
    if (!item) {
        deleteSubsetAction_->setEnabled(false);
        deleteSamplesAction_->setEnabled(false);
        return;
    }
    
    bool isSubset = item->data(Qt::UserRole + 1).toBool();
    
    // Enable delete subset button if subset is selected
    deleteSubsetAction_->setEnabled(isSubset);
    
    // Enable delete sample button if sample is selected
    deleteSamplesAction_->setEnabled(!isSubset);
    
    if (isSubset) return;
    
    int sampleIndex = item->data(Qt::UserRole).toInt();
    if (sampleIndex >= 0 && sampleIndex < dataset_->samples().size()) {
        emit sampleSelected(dataset_->samples()[sampleIndex]);
        emit sampleSelectedWithIndex(dataset_->samples()[sampleIndex], sampleIndex);
    }
}

int DatasetView::getSelectedSampleIndex() const {
    QModelIndex index = treeView_->currentIndex();
    if (!index.isValid()) return -1;
    
    QStandardItem* item = model_->itemFromIndex(index.siblingAtColumn(0));
    if (!item) return -1;
    
    bool isSubset = item->data(Qt::UserRole + 1).toBool();
    if (isSubset) return -1;
    
    return item->data(Qt::UserRole).toInt();
}

bool DatasetView::isSubsetSelected() const {
    QModelIndex index = treeView_->currentIndex();
    if (!index.isValid()) return false;
    
    QStandardItem* item = model_->itemFromIndex(index.siblingAtColumn(0));
    if (!item) return false;
    
    return item->data(Qt::UserRole + 1).toBool();
}

void DatasetView::showContextMenu(const QPoint& pos) {
    QModelIndex index = treeView_->indexAt(pos);
    
    QMenu contextMenu(this);
    
    // Context menu for empty space (root)
    if (!index.isValid()) {
        QAction* importAction = contextMenu.addAction(tr("Import Files..."));
        contextMenu.addSeparator();
        QAction* addSubsetAction = contextMenu.addAction(tr("Add Subset..."));
        contextMenu.addSeparator();
        QAction* expandAction = contextMenu.addAction(tr("Expand All"));
        QAction* collapseAction = contextMenu.addAction(tr("Collapse All"));
        
        QAction* selectedAction = contextMenu.exec(treeView_->viewport()->mapToGlobal(pos));
        
        if (selectedAction == importAction) {
            emit importFilesRequested();
        } else if (selectedAction == addSubsetAction) {
            emit addSubsetRequested();
        } else if (selectedAction == expandAction) {
            treeView_->expandAll();
        } else if (selectedAction == collapseAction) {
            treeView_->collapseAll();
        }
        return;
    }
    
    QStandardItem* item = model_->itemFromIndex(index.siblingAtColumn(0));
    if (!item) return;
    
    bool isSubset = item->data(Qt::UserRole + 1).toBool();
    
    if (isSubset) {
        // Context menu for subsets
        QString subsetName = item->text();
        
        QAction* addSubsetAction = contextMenu.addAction(tr("Add Subset..."));
        contextMenu.addSeparator();
        QAction* deleteSubsetAction = contextMenu.addAction(tr("Delete Subset '%1'").arg(subsetName));
        contextMenu.addSeparator();
        QAction* expandAction = contextMenu.addAction(tr("Expand All"));
        QAction* collapseAction = contextMenu.addAction(tr("Collapse All"));
        
        QAction* selectedAction = contextMenu.exec(treeView_->viewport()->mapToGlobal(pos));
        
        if (selectedAction == addSubsetAction) {
            emit addSubsetRequested();
        } else if (selectedAction == deleteSubsetAction) {
            emit deleteSubsetRequested(subsetName);
        } else if (selectedAction == expandAction) {
            treeView_->expandAll();
        } else if (selectedAction == collapseAction) {
            treeView_->collapseAll();
        }
        return;
    }
    
    // Context menu for samples
    int sampleIndex = item->data(Qt::UserRole).toInt();
    if (sampleIndex < 0) return;
    
    // Check if multiple samples are selected
    QModelIndexList selectedIndexes = treeView_->selectionModel()->selectedRows();
    bool multipleSelected = selectedIndexes.size() > 1;
    
    if (multipleSelected) {
        QAction* batchMoveAction = contextMenu.addAction(tr("Move Selected to Subset..."));
        contextMenu.addSeparator();
        QAction* batchDeleteAction = contextMenu.addAction(tr("Delete Selected Samples"));
        
        QAction* selectedAction = contextMenu.exec(treeView_->viewport()->mapToGlobal(pos));
        
        if (selectedAction == batchMoveAction) {
            emit batchMoveToSubsetRequested(getSelectedSampleIndices());
        } else if (selectedAction == batchDeleteAction) {
            emit deleteSamplesRequested();
        }
    } else {
        QAction* moveToSubsetAction = contextMenu.addAction(tr("Move to Subset..."));
        contextMenu.addSeparator();
        QAction* deleteAction = contextMenu.addAction(tr("Delete Sample"));
        
        QAction* selectedAction = contextMenu.exec(treeView_->viewport()->mapToGlobal(pos));
        
        if (selectedAction == moveToSubsetAction) {
            emit moveToSubsetRequested(sampleIndex);
        } else if (selectedAction == deleteAction) {
            emit deleteSampleRequested(sampleIndex);
        }
    }
}

QString DatasetView::getSelectedSubsetName() const {
    QModelIndex index = treeView_->currentIndex();
    if (!index.isValid()) return QString();
    
    QStandardItem* item = model_->itemFromIndex(index.siblingAtColumn(0));
    if (!item) return QString();
    
    bool isSubset = item->data(Qt::UserRole + 1).toBool();
    if (!isSubset) return QString();
    
    return item->text();
}

QList<int> DatasetView::getSelectedSampleIndices() const {
    QList<int> indices;
    QModelIndexList selectedIndexes = treeView_->selectionModel()->selectedRows();
    
    for (const QModelIndex& index : selectedIndexes) {
        if (!index.isValid()) continue;
        
        QStandardItem* item = model_->itemFromIndex(index.siblingAtColumn(0));
        if (!item) continue;
        
        bool isSubset = item->data(Qt::UserRole + 1).toBool();
        if (isSubset) continue;
        
        int sampleIndex = item->data(Qt::UserRole).toInt();
        if (sampleIndex >= 0) {
            indices.append(sampleIndex);
        }
    }
    
    return indices;
}

void DatasetView::onAddSubset() {
    emit addSubsetRequested();
}

void DatasetView::onDeleteSubset() {
    QString subsetName = getSelectedSubsetName();
    if (!subsetName.isEmpty()) {
        emit deleteSubsetRequested(subsetName);
    }
}

void DatasetView::onExpandAll() {
    treeView_->expandAll();
}

void DatasetView::onCollapseAll() {
    treeView_->collapseAll();
}

void DatasetView::onImportFiles() {
    emit importFilesRequested();
}

void DatasetView::onDeleteSamples() {
    emit deleteSamplesRequested();
}

}

