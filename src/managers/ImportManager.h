#pragma once
#include "core/Dataset.h"
#include <QObject>

namespace DatasetCreator {

class PluginManager;

class ImportManager : public QObject {
    Q_OBJECT
public:
    explicit ImportManager(PluginManager* pluginManager, QObject* parent = nullptr);
    void importFile(const QString& filePath);
    void importBatch(const QStringList& files);
    
signals:
    void importProgress(int current, int total);
    void sampleImported(const DatasetSample& sample);
    void importCompleted();
    void importError(const QString& error);
    
private:
    PluginManager* pluginManager_;
};

}
