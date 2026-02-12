#pragma once

#include "core/Dataset.h"
#include <QString>
#include <QStringList>
#include <QIODevice>
#include <QVariantMap>
#include <memory>

namespace DatasetCreator {

/**
 * @brief Base interface for data readers (input plugins)
 * 
 * Implement this interface to create custom file format readers.
 * Readers are responsible for loading data from files and converting
 * them into DatasetSample objects.
 */
class IDataReader {
public:
    virtual ~IDataReader() = default;
    
    // Plugin metadata
    virtual QString name() const = 0;
    virtual QString version() const = 0;
    virtual QStringList supportedExtensions() const = 0;
    virtual QStringList supportedMimeTypes() const = 0;
    
    // Format detection
    virtual bool canRead(QIODevice* device) const = 0;
    virtual bool canRead(const QString& filePath) const = 0;
    
    // Reading operations
    virtual DatasetSample read(const QString& filePath) = 0;
    virtual QList<DatasetSample> readBatch(const QStringList& files) = 0;
    
    // Optional: Progressive reading for large files
    virtual bool supportsStreaming() const { return false; }
    virtual bool beginRead(const QString& filePath) { Q_UNUSED(filePath); return false; }
    virtual DatasetSample readNext() { return DatasetSample(); }
    virtual bool endRead() { return false; }
    
    // Metadata extraction (without loading full data)
    virtual QVariantMap extractMetadata(const QString& filePath) = 0;
    
    // Configuration
    virtual void setOption(const QString& key, const QVariant& value) { Q_UNUSED(key); Q_UNUSED(value); }
    virtual QVariant option(const QString& key) const { Q_UNUSED(key); return QVariant(); }
};

/**
 * @brief Base interface for data writers (output plugins)
 * 
 * Implement this interface to create custom dataset serialization formats.
 * Writers are responsible for converting Dataset objects into specific
 * file formats.
 */
class IDataWriter {
public:
    virtual ~IDataWriter() = default;
    
    // Plugin metadata
    virtual QString name() const = 0;
    virtual QString version() const = 0;
    virtual QString fileExtension() const = 0;
    virtual QString formatName() const = 0;
    virtual QString description() const { return QString(); }
    
    // Writing operations
    virtual bool write(const QString& outputPath, 
                      const Dataset& dataset) = 0;
    
    // Optional: Streaming for large datasets
    virtual bool supportsStreaming() const { return false; }
    virtual bool beginWrite(const QString& outputPath, const DatasetMetadata& metadata) { 
        Q_UNUSED(outputPath); Q_UNUSED(metadata); return false; 
    }
    virtual bool writeSample(const DatasetSample& sample) { 
        Q_UNUSED(sample); return false; 
    }
    virtual bool writeSubset(const DatasetSubset& subset) { 
        Q_UNUSED(subset); return false; 
    }
    virtual bool endWrite() { return false; }
    
    // Configuration
    virtual QVariantMap defaultOptions() const { return QVariantMap(); }
    virtual void setOptions(const QVariantMap& options) { Q_UNUSED(options); }
    virtual QVariantMap currentOptions() const { return QVariantMap(); }
    
    // Validation
    virtual bool canWrite(const Dataset& dataset) const { Q_UNUSED(dataset); return true; }
    virtual QString validationError() const { return QString(); }
};

/**
 * @brief Plugin factory interface for dynamic loading
 * 
 * Implement this interface when creating dynamically loadable plugins.
 * This is the entry point for external plugins loaded at runtime.
 */
class IDataPlugin {
public:
    virtual ~IDataPlugin() = default;
    
    virtual QString pluginName() const = 0;
    virtual QString pluginVersion() const = 0;
    virtual QString pluginDescription() const { return QString(); }
    
    // Create plugin instances
    virtual IDataReader* createReader() = 0;
    virtual IDataWriter* createWriter() = 0;
    
    // Plugin capabilities
    virtual bool providesReader() const { return true; }
    virtual bool providesWriter() const { return true; }
};

} // namespace DatasetCreator

// Qt plugin interface declaration
Q_DECLARE_INTERFACE(DatasetCreator::IDataPlugin, "com.datasetcreator.IDataPlugin/1.0")
