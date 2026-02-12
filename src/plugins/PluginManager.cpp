#include "PluginManager.h"
#include "readers/TextReader.h"
#include "readers/ImageReader.h"
#include "readers/AudioReader.h"
#include "readers/CSVReader.h"
#include "writers/JSONWriter.h"
#include "writers/JSONLWriter.h"
#include "writers/CSVWriter.h"
#include <QDir>
#include <QFileInfo>
#include <QPluginLoader>
#include <QDebug>

namespace DatasetCreator {

PluginManager::PluginManager() {
    loadBuiltInPlugins();
}

PluginManager::~PluginManager() = default;

void PluginManager::loadBuiltInPlugins() {
    // Register built-in readers
    registerReader(std::make_unique<TextReader>());
    registerReader(std::make_unique<ImageReader>());
    registerReader(std::make_unique<AudioReader>());
    registerReader(std::make_unique<CSVReader>());
    
    // Register built-in writers
    registerWriter(std::make_unique<JSONWriter>());
    registerWriter(std::make_unique<JSONLWriter>());
    registerWriter(std::make_unique<CSVWriter>());
}

void PluginManager::loadDynamicPlugins(const QString& pluginDirectory) {
    QDir dir(pluginDirectory);
    if (!dir.exists()) {
        qWarning() << "Plugin directory does not exist:" << pluginDirectory;
        return;
    }
    
    const QStringList pluginFiles = dir.entryList(QDir::Files);
    for (const QString& fileName : pluginFiles) {
        QString filePath = dir.absoluteFilePath(fileName);
        QPluginLoader loader(filePath);
        QObject* plugin = loader.instance();
        
        if (plugin) {
            IDataPlugin* dataPlugin = qobject_cast<IDataPlugin*>(plugin);
            if (dataPlugin) {
                qDebug() << "Loaded plugin:" << dataPlugin->pluginName();
                
                if (dataPlugin->providesReader()) {
                    IDataReader* reader = dataPlugin->createReader();
                    if (reader) {
                        registerReader(std::unique_ptr<IDataReader>(reader));
                    }
                }
                
                if (dataPlugin->providesWriter()) {
                    IDataWriter* writer = dataPlugin->createWriter();
                    if (writer) {
                        registerWriter(std::unique_ptr<IDataWriter>(writer));
                    }
                }
            }
        } else {
            qDebug() << "Failed to load plugin:" << fileName << "-" << loader.errorString();
        }
    }
}

void PluginManager::registerReader(std::unique_ptr<IDataReader> reader) {
    IDataReader* readerPtr = reader.get();
    readers_.push_back(std::move(reader));
    indexReader(readerPtr);
}

void PluginManager::registerWriter(std::unique_ptr<IDataWriter> writer) {
    IDataWriter* writerPtr = writer.get();
    writers_.push_back(std::move(writer));
    indexWriter(writerPtr);
}

void PluginManager::indexReader(IDataReader* reader) {
    // Index by name
    readersByName_[reader->name()] = reader;
    
    // Index by extensions
    for (const QString& ext : reader->supportedExtensions()) {
        QString cleanExt = ext.toLower();
        if (cleanExt.startsWith(".")) {
            cleanExt = cleanExt.mid(1);
        }
        extensionToReader_[cleanExt] = reader;
    }
}

void PluginManager::indexWriter(IDataWriter* writer) {
    // Index by name
    writersByName_[writer->name()] = writer;
    
    // Index by format name
    formatToWriter_[writer->formatName().toLower()] = writer;
    formatToWriter_[writer->fileExtension().toLower()] = writer;
}

IDataReader* PluginManager::getReaderForFile(const QString& filePath) const {
    QFileInfo fileInfo(filePath);
    QString extension = fileInfo.suffix().toLower();
    return extensionToReader_.value(extension, nullptr);
}

IDataReader* PluginManager::getReaderForExtension(const QString& extension) const {
    QString cleanExt = extension.toLower();
    if (cleanExt.startsWith(".")) {
        cleanExt = cleanExt.mid(1);
    }
    return extensionToReader_.value(cleanExt, nullptr);
}

IDataReader* PluginManager::getReaderByName(const QString& name) const {
    return readersByName_.value(name, nullptr);
}

QStringList PluginManager::availableReaderNames() const {
    return readersByName_.keys();
}

QStringList PluginManager::supportedReadExtensions() const {
    return extensionToReader_.keys();
}

IDataWriter* PluginManager::getWriterForFormat(const QString& format) const {
    return formatToWriter_.value(format.toLower(), nullptr);
}

IDataWriter* PluginManager::getWriterByName(const QString& name) const {
    return writersByName_.value(name, nullptr);
}

QStringList PluginManager::availableWriterNames() const {
    return writersByName_.keys();
}

QStringList PluginManager::supportedWriteFormats() const {
    QStringList formats;
    for (const auto& writer : writers_) {
        formats.append(writer->formatName());
    }
    return formats;
}

bool PluginManager::canReadFile(const QString& filePath) const {
    return getReaderForFile(filePath) != nullptr;
}

bool PluginManager::canWriteFormat(const QString& format) const {
    return getWriterForFormat(format) != nullptr;
}

} // namespace DatasetCreator
