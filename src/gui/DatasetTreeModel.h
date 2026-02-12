#pragma once
#include <QStandardItemModel>

namespace DatasetCreator {

class DatasetTreeModel : public QStandardItemModel {
    Q_OBJECT
public:
    explicit DatasetTreeModel(QObject* parent = nullptr);
    
    Qt::DropActions supportedDropActions() const override;
    bool dropMimeData(const QMimeData* data, Qt::DropAction action,
                     int row, int column, const QModelIndex& parent) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

signals:
    void sampleDropped(const QString& sampleId, const QString& targetSubset);
    void sampleDroppedToRoot(const QString& sampleId);
};

}
