#pragma once

#include "DatasetSample.h"
#include "Metadata.h"
#include <QString>
#include <QList>
#include <QMap>
#include <memory>

namespace DatasetCreator {

/**
 * @brief Dataset subset - a logical grouping of samples
 */
class DatasetSubset {
public:
    DatasetSubset() = default;
    explicit DatasetSubset(const QString& name);
    
    // Name and metadata
    QString name() const { return metadata_.name; }
    void setName(const QString& name) { metadata_.name = name; }
    
    SubsetMetadata& metadata() { return metadata_; }
    const SubsetMetadata& metadata() const { return metadata_; }
    
    // Sample management
    void addSample(const DatasetSample& sample);
    void addSamples(const QList<DatasetSample>& samples);
    void removeSample(int index);
    void clearSamples();
    
    int sampleCount() const { return samples_.size(); }
    const QList<DatasetSample>& samples() const { return samples_; }
    QList<DatasetSample>& samples() { return samples_; }
    
    DatasetSample& operator[](int index) { return samples_[index]; }
    const DatasetSample& operator[](int index) const { return samples_[index]; }
    
    // Statistics
    qint64 totalSize() const;
    QMap<SampleType, int> typeDistribution() const;
    
    // Serialization
    QVariantMap toVariantMap() const;
    static DatasetSubset fromVariantMap(const QVariantMap& map);
    
private:
    SubsetMetadata metadata_;
    QList<DatasetSample> samples_;
};

/**
 * @brief Dataset - the root container for all dataset data
 */
class Dataset {
public:
    Dataset();
    explicit Dataset(const QString& name);
    
    // Global metadata
    DatasetMetadata& metadata() { return metadata_; }
    const DatasetMetadata& metadata() const { return metadata_; }
    
    // Sample management (flat structure - no subsets)
    void addSample(const DatasetSample& sample);
    void addSamples(const QList<DatasetSample>& samples);
    void removeSample(int index);
    void clearSamples();
    
    int sampleCount() const;
    const QList<DatasetSample>& samples() const { return samples_; }
    QList<DatasetSample>& samples() { return samples_; }
    
    // Subset management (hierarchical structure)
    void addSubset(const DatasetSubset& subset);
    void removeSubset(const QString& name);
    DatasetSubset* getSubset(const QString& name);
    const DatasetSubset* getSubset(const QString& name) const;
    
    bool hasSubsets() const { return !subsets_.isEmpty(); }
    int subsetCount() const { return subsets_.size(); }
    QList<QString> subsetNames() const;
    const QList<DatasetSubset>& subsets() const { return subsets_; }
    QList<DatasetSubset>& subsets() { return subsets_; }
    
    // Move samples between flat and hierarchical structures
    void moveSampleToSubset(int sampleIndex, const QString& subsetName);
    void moveSampleFromSubset(const QString& subsetName, int sampleIndex);
    
    // Statistics
    qint64 totalSize() const;
    QMap<SampleType, int> typeDistribution() const;
    int totalSampleCount() const;  // Includes all samples in subsets
    
    // Utility
    void clear();
    bool isEmpty() const;
    
    // Serialization
    QVariantMap toVariantMap() const;
    static Dataset fromVariantMap(const QVariantMap& map);
    
private:
    DatasetMetadata metadata_;
    QList<DatasetSample> samples_;        // Top-level samples (flat structure)
    QList<DatasetSubset> subsets_;        // Hierarchical subsets
};

} // namespace DatasetCreator
