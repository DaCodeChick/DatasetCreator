#include "ExportManager.h"
#include "plugins/PluginManager.h"

namespace DatasetCreator {

ExportManager::ExportManager(PluginManager* pluginManager, QObject* parent)
    : QObject(parent), pluginManager_(pluginManager) {}

bool ExportManager::exportDataset(const Dataset& dataset, const QString& outputPath, const QString& format) {
    IDataWriter* writer = pluginManager_->getWriterForFormat(format);
    if (!writer) {
        emit exportError("No writer available for format: " + format);
        return false;
    }
    
    emit exportProgress(0);
    bool success = writer->write(outputPath, dataset);
    emit exportProgress(100);
    emit exportCompleted(success);
    
    return success;
}

}
