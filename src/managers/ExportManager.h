#pragma once
#include "core/Dataset.h"
#include <QObject>

namespace DatasetCreator {

class PluginManager;

class ExportManager : public QObject {
    Q_OBJECT
public:
    explicit ExportManager(PluginManager* pluginManager, QObject* parent = nullptr);
    bool exportDataset(const Dataset& dataset, const QString& outputPath, const QString& format);
    
signals:
    void exportProgress(int percent);
    void exportCompleted(bool success);
    void exportError(const QString& error);
    
private:
    PluginManager* pluginManager_;
};

}
