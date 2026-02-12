#include "DatasetTreeModel.h"
#include <QMimeData>
#include <QIODevice>

namespace DatasetCreator {

DatasetTreeModel::DatasetTreeModel(QObject* parent)
    : QStandardItemModel(parent)
{
}

Qt::DropActions DatasetTreeModel::supportedDropActions() const {
    return Qt::MoveAction;
}

Qt::ItemFlags DatasetTreeModel::flags(const QModelIndex& index) const {
    Qt::ItemFlags defaultFlags = QStandardItemModel::flags(index);
    
    if (!index.isValid()) {
        return defaultFlags | Qt::ItemIsDropEnabled;
    }
    
    QStandardItem* item = itemFromIndex(index);
    if (!item) return defaultFlags;
    
    bool isSubset = item->data(Qt::UserRole + 1).toBool();
    
    if (isSubset) {
        // Subsets can accept drops but cannot be dragged
        return (defaultFlags | Qt::ItemIsDropEnabled) & ~Qt::ItemIsDragEnabled;
    }
    
    // Samples can be dragged
    return defaultFlags | Qt::ItemIsDragEnabled;
}

bool DatasetTreeModel::dropMimeData(const QMimeData* data, Qt::DropAction action,
                                    int row, int column, const QModelIndex& parent) {
    if (!data || action != Qt::MoveAction) {
        return false;
    }
    
    // Get the dropped items from MIME data
    if (!data->hasFormat("application/x-qabstractitemmodeldatalist")) {
        return false;
    }
    
    // Decode the MIME data to get source indices
    QByteArray encoded = data->data("application/x-qabstractitemmodeldatalist");
    QDataStream stream(&encoded, QIODevice::ReadOnly);
    
    QList<QString> droppedSampleIds;
    
    while (!stream.atEnd()) {
        int sourceRow, sourceCol;
        QMap<int, QVariant> roleDataMap;
        stream >> sourceRow >> sourceCol >> roleDataMap;
        
        // Get sample ID from UserRole + 2
        if (roleDataMap.contains(Qt::UserRole + 2)) {
            QString sampleId = roleDataMap[Qt::UserRole + 2].toString();
            if (!sampleId.isEmpty()) {
                droppedSampleIds.append(sampleId);
            }
        }
    }
    
    if (droppedSampleIds.isEmpty()) {
        return false;
    }
    
    // Determine the drop target
    QString targetSubset;
    
    if (parent.isValid()) {
        QStandardItem* parentItem = itemFromIndex(parent);
        if (parentItem) {
            bool isSubset = parentItem->data(Qt::UserRole + 1).toBool();
            if (isSubset) {
                targetSubset = parentItem->text();
            }
        }
    }
    
    // Emit signals for each dropped sample
    for (const QString& sampleId : droppedSampleIds) {
        if (targetSubset.isEmpty()) {
            emit sampleDroppedToRoot(sampleId);
        } else {
            emit sampleDropped(sampleId, targetSubset);
        }
    }
    
    // Don't actually move the items in the model - let the Dataset class
    // handle the data move and then refresh the view
    return false;  // Return false to prevent default Qt behavior
}

}
