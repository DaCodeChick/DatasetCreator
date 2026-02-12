#include "MetadataManager.h"

namespace DatasetCreator {

MetadataManager::MetadataManager(QObject* parent) : QObject(parent) {}

void MetadataManager::addTag(DatasetSample& sample, const QString& tag) {
    if (!sample.metadata().tags.contains(tag)) {
        sample.metadata().tags.append(tag);
        emit metadataChanged();
    }
}

void MetadataManager::setLabel(DatasetSample& sample, const QString& key, const QVariant& value) {
    sample.metadata().labels[key] = value;
    emit metadataChanged();
}

void MetadataManager::createSubset(Dataset& dataset, const QString& name) {
    DatasetSubset subset(name);
    dataset.addSubset(subset);
    emit metadataChanged();
}

void MetadataManager::moveSampleToSubset(Dataset& dataset, int sampleIndex, const QString& subsetName) {
    dataset.moveSampleToSubset(sampleIndex, subsetName);
    emit metadataChanged();
}

}
