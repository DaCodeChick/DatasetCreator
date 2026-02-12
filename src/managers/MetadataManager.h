#pragma once
#include "core/Dataset.h"
#include <QObject>

namespace DatasetCreator {

class MetadataManager : public QObject {
    Q_OBJECT
public:
    explicit MetadataManager(QObject* parent = nullptr);
    void addTag(DatasetSample& sample, const QString& tag);
    void setLabel(DatasetSample& sample, const QString& key, const QVariant& value);
    void createSubset(Dataset& dataset, const QString& name);
    void moveSampleToSubset(Dataset& dataset, int sampleIndex, const QString& subsetName);
    
signals:
    void metadataChanged();
};

}
