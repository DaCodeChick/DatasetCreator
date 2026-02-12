#pragma once

#include "core/PluginInterface.h"
#include <QString>
#include <QMap>
#include <QList>
#include <memory>

namespace DatasetCreator {

/**
 * @brief Manages plugin registration and discovery
 * 
 * The PluginManager handles both built-in and dynamically loaded plugins.
 * It provides methods for registering plugins, discovering available formats,
 * and retrieving appropriate readers/writers for specific files.
 */
class PluginManager {
public:
    PluginManager();
    ~PluginManager();
    
    // Initialize and load plugins
    void loadBuiltInPlugins();
    void loadDynamicPlugins(const QString& pluginDirectory);
    
    // Reader management
    void registerReader(std::unique_ptr<IDataReader> reader);
    IDataReader* getReaderForFile(const QString& filePath) const;
    IDataReader* getReaderForExtension(const QString& extension) const;
    IDataReader* getReaderByName(const QString& name) const;
    QStringList availableReaderNames() const;
    QStringList supportedReadExtensions() const;
    
    // Writer management
    void registerWriter(std::unique_ptr<IDataWriter> writer);
    IDataWriter* getWriterForFormat(const QString& format) const;
    IDataWriter* getWriterByName(const QString& name) const;
    QStringList availableWriterNames() const;
    QStringList supportedWriteFormats() const;
    
    // Query capabilities
    bool canReadFile(const QString& filePath) const;
    bool canWriteFormat(const QString& format) const;
    
private:
    // Reader storage
    std::vector<std::unique_ptr<IDataReader>> readers_;
    QMap<QString, IDataReader*> extensionToReader_;  // ext -> reader
    QMap<QString, IDataReader*> readersByName_;       // name -> reader
    
    // Writer storage
    std::vector<std::unique_ptr<IDataWriter>> writers_;
    QMap<QString, IDataWriter*> formatToWriter_;      // format -> writer
    QMap<QString, IDataWriter*> writersByName_;       // name -> writer
    
    void indexReader(IDataReader* reader);
    void indexWriter(IDataWriter* writer);
};

} // namespace DatasetCreator
