#include "ImportManager.h"
#include "plugins/PluginManager.h"

namespace DatasetCreator {

ImportManager::ImportManager(PluginManager* pluginManager, QObject* parent)
    : QObject(parent), pluginManager_(pluginManager) {}

void ImportManager::importFile(const QString& filePath) {
    IDataReader* reader = pluginManager_->getReaderForFile(filePath);
    if (!reader) {
        emit importError("No reader available for file: " + filePath);
        return;
    }
    
    DatasetSample sample = reader->read(filePath);
    emit sampleImported(sample);
    emit importCompleted();
}

void ImportManager::importBatch(const QStringList& files) {
    int total = files.size();
    int current = 0;
    
    for (const QString& file : files) {
        importFile(file);
        emit importProgress(++current, total);
    }
}

}
