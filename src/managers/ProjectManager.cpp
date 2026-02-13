#include "ProjectManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include <QDir>

namespace DatasetCreator {

ProjectManager::ProjectManager(QObject* parent)
    : QObject(parent) {
}

bool ProjectManager::saveProject(const Dataset& dataset, const QString& filePath) {
    lastError_.clear();
    
    // Convert dataset to QVariantMap
    QVariantMap datasetMap = dataset.toVariantMap();
    
    // Convert to JSON
    QJsonDocument doc = QJsonDocument::fromVariant(datasetMap);
    if (doc.isNull()) {
        lastError_ = tr("Failed to serialize dataset to JSON");
        emit error(lastError_);
        return false;
    }
    
    // Save to file
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        lastError_ = tr("Failed to open file for writing: %1").arg(file.errorString());
        emit error(lastError_);
        return false;
    }
    
    qint64 bytesWritten = file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    
    if (bytesWritten == -1) {
        lastError_ = tr("Failed to write to file: %1").arg(file.errorString());
        emit error(lastError_);
        return false;
    }
    
    emit projectSaved(filePath);
    return true;
}

bool ProjectManager::loadProject(const QString& filePath, Dataset& dataset) {
    lastError_.clear();
    
    // Check if file exists
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        lastError_ = tr("Project file does not exist: %1").arg(filePath);
        emit error(lastError_);
        return false;
    }
    
    // Load file
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        lastError_ = tr("Failed to open file for reading: %1").arg(file.errorString());
        emit error(lastError_);
        return false;
    }
    
    QByteArray fileData = file.readAll();
    file.close();
    
    // Parse JSON
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(fileData, &parseError);
    
    if (doc.isNull() || parseError.error != QJsonParseError::NoError) {
        lastError_ = tr("Failed to parse JSON: %1").arg(parseError.errorString());
        emit error(lastError_);
        return false;
    }
    
    if (!doc.isObject()) {
        lastError_ = tr("Invalid project file format: root must be an object");
        emit error(lastError_);
        return false;
    }
    
    // Convert to QVariantMap
    QVariantMap datasetMap = doc.object().toVariantMap();
    
    // Deserialize dataset
    try {
        dataset = Dataset::fromVariantMap(datasetMap);
    } catch (const std::exception& e) {
        lastError_ = tr("Failed to deserialize dataset: %1").arg(e.what());
        emit error(lastError_);
        return false;
    } catch (...) {
        lastError_ = tr("Failed to deserialize dataset: unknown error");
        emit error(lastError_);
        return false;
    }
    
    emit projectLoaded(filePath);
    return true;
}

} // namespace DatasetCreator
