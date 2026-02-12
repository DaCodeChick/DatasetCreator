#include "Dataset.h"

namespace DatasetCreator {

// DatasetSubset implementation
DatasetSubset::DatasetSubset(const QString& name) {
    metadata_.name = name;
}

void DatasetSubset::addSample(const DatasetSample& sample) {
    samples_.append(sample);
}

void DatasetSubset::addSamples(const QList<DatasetSample>& samples) {
    samples_.append(samples);
}

void DatasetSubset::removeSample(int index) {
    if (index >= 0 && index < samples_.size()) {
        samples_.removeAt(index);
    }
}

void DatasetSubset::clearSamples() {
    samples_.clear();
}

qint64 DatasetSubset::totalSize() const {
    qint64 total = 0;
    for (const auto& sample : samples_) {
        total += sample.dataSize();
    }
    return total;
}

QMap<SampleType, int> DatasetSubset::typeDistribution() const {
    QMap<SampleType, int> distribution;
    for (const auto& sample : samples_) {
        distribution[sample.type()]++;
    }
    return distribution;
}

QVariantMap DatasetSubset::toVariantMap() const {
    QVariantMap map;
    map["metadata"] = metadata_.toVariantMap();
    
    QVariantList samplesList;
    for (const auto& sample : samples_) {
        samplesList.append(sample.toVariantMap());
    }
    map["samples"] = samplesList;
    
    return map;
}

DatasetSubset DatasetSubset::fromVariantMap(const QVariantMap& map) {
    DatasetSubset subset;
    subset.metadata_ = SubsetMetadata::fromVariantMap(map.value("metadata").toMap());
    
    QVariantList samplesList = map.value("samples").toList();
    for (const auto& sampleVar : samplesList) {
        subset.samples_.append(DatasetSample::fromVariantMap(sampleVar.toMap()));
    }
    
    return subset;
}

// Dataset implementation
Dataset::Dataset() {
    metadata_.created = QDateTime::currentDateTime();
    metadata_.modified = QDateTime::currentDateTime();
}

Dataset::Dataset(const QString& name) {
    metadata_.name = name;
    metadata_.created = QDateTime::currentDateTime();
    metadata_.modified = QDateTime::currentDateTime();
}

void Dataset::addSample(const DatasetSample& sample) {
    samples_.append(sample);
    metadata_.modified = QDateTime::currentDateTime();
}

void Dataset::addSamples(const QList<DatasetSample>& samples) {
    samples_.append(samples);
    metadata_.modified = QDateTime::currentDateTime();
}

void Dataset::removeSample(int index) {
    if (index >= 0 && index < samples_.size()) {
        samples_.removeAt(index);
        metadata_.modified = QDateTime::currentDateTime();
    }
}

void Dataset::clearSamples() {
    samples_.clear();
    metadata_.modified = QDateTime::currentDateTime();
}

int Dataset::sampleCount() const {
    return samples_.size();
}

void Dataset::addSubset(const DatasetSubset& subset) {
    subsets_.append(subset);
    metadata_.modified = QDateTime::currentDateTime();
}

void Dataset::removeSubset(const QString& name) {
    for (int i = 0; i < subsets_.size(); ++i) {
        if (subsets_[i].name() == name) {
            subsets_.removeAt(i);
            metadata_.modified = QDateTime::currentDateTime();
            return;
        }
    }
}

DatasetSubset* Dataset::getSubset(const QString& name) {
    for (auto& subset : subsets_) {
        if (subset.name() == name) {
            return &subset;
        }
    }
    return nullptr;
}

const DatasetSubset* Dataset::getSubset(const QString& name) const {
    for (const auto& subset : subsets_) {
        if (subset.name() == name) {
            return &subset;
        }
    }
    return nullptr;
}

QList<QString> Dataset::subsetNames() const {
    QList<QString> names;
    for (const auto& subset : subsets_) {
        names.append(subset.name());
    }
    return names;
}

void Dataset::moveSampleToSubset(int sampleIndex, const QString& subsetName) {
    if (sampleIndex < 0 || sampleIndex >= samples_.size()) {
        return;
    }
    
    DatasetSubset* subset = getSubset(subsetName);
    if (!subset) {
        // Create new subset if it doesn't exist
        DatasetSubset newSubset(subsetName);
        subsets_.append(newSubset);
        subset = &subsets_.last();
    }
    
    subset->addSample(samples_[sampleIndex]);
    samples_.removeAt(sampleIndex);
    metadata_.modified = QDateTime::currentDateTime();
}

void Dataset::moveSampleFromSubset(const QString& subsetName, int sampleIndex) {
    DatasetSubset* subset = getSubset(subsetName);
    if (!subset || sampleIndex < 0 || sampleIndex >= subset->sampleCount()) {
        return;
    }
    
    samples_.append((*subset)[sampleIndex]);
    subset->removeSample(sampleIndex);
    metadata_.modified = QDateTime::currentDateTime();
}

qint64 Dataset::totalSize() const {
    qint64 total = 0;
    
    for (const auto& sample : samples_) {
        total += sample.dataSize();
    }
    
    for (const auto& subset : subsets_) {
        total += subset.totalSize();
    }
    
    return total;
}

QMap<SampleType, int> Dataset::typeDistribution() const {
    QMap<SampleType, int> distribution;
    
    for (const auto& sample : samples_) {
        distribution[sample.type()]++;
    }
    
    for (const auto& subset : subsets_) {
        auto subsetDist = subset.typeDistribution();
        for (auto it = subsetDist.begin(); it != subsetDist.end(); ++it) {
            distribution[it.key()] += it.value();
        }
    }
    
    return distribution;
}

int Dataset::totalSampleCount() const {
    int total = samples_.size();
    for (const auto& subset : subsets_) {
        total += subset.sampleCount();
    }
    return total;
}

void Dataset::clear() {
    samples_.clear();
    subsets_.clear();
    metadata_ = DatasetMetadata();
    metadata_.created = QDateTime::currentDateTime();
    metadata_.modified = QDateTime::currentDateTime();
}

bool Dataset::isEmpty() const {
    return samples_.isEmpty() && subsets_.isEmpty();
}

QVariantMap Dataset::toVariantMap() const {
    QVariantMap map;
    map["metadata"] = metadata_.toVariantMap();
    
    if (!samples_.isEmpty()) {
        QVariantList samplesList;
        for (const auto& sample : samples_) {
            samplesList.append(sample.toVariantMap());
        }
        map["samples"] = samplesList;
    }
    
    if (!subsets_.isEmpty()) {
        QVariantList subsetsList;
        for (const auto& subset : subsets_) {
            subsetsList.append(subset.toVariantMap());
        }
        map["subsets"] = subsetsList;
    }
    
    return map;
}

Dataset Dataset::fromVariantMap(const QVariantMap& map) {
    Dataset dataset;
    dataset.metadata_ = DatasetMetadata::fromVariantMap(map.value("metadata").toMap());
    
    if (map.contains("samples")) {
        QVariantList samplesList = map.value("samples").toList();
        for (const auto& sampleVar : samplesList) {
            dataset.samples_.append(DatasetSample::fromVariantMap(sampleVar.toMap()));
        }
    }
    
    if (map.contains("subsets")) {
        QVariantList subsetsList = map.value("subsets").toList();
        for (const auto& subsetVar : subsetsList) {
            dataset.subsets_.append(DatasetSubset::fromVariantMap(subsetVar.toMap()));
        }
    }
    
    return dataset;
}

} // namespace DatasetCreator
