#pragma once
#include "core/Dataset.h"
#include <QObject>
#include <QString>

namespace DatasetCreator {

/**
 * @brief Manages project save/load operations
 * 
 * Handles serialization of Dataset to JSON files and deserialization back.
 * Project files have .dscp extension (DataSet Creator Project).
 */
class ProjectManager : public QObject {
    Q_OBJECT
public:
    explicit ProjectManager(QObject* parent = nullptr);
    
    /**
     * @brief Save dataset to a project file
     * @param dataset The dataset to save
     * @param filePath Path to save the project file
     * @return true if save succeeded, false otherwise
     */
    bool saveProject(const Dataset& dataset, const QString& filePath);
    
    /**
     * @brief Load dataset from a project file
     * @param filePath Path to the project file
     * @param dataset Output parameter for the loaded dataset
     * @return true if load succeeded, false otherwise
     */
    bool loadProject(const QString& filePath, Dataset& dataset);
    
    /**
     * @brief Get the last error message
     */
    QString lastError() const { return lastError_; }
    
signals:
    void saveProgress(int current, int total);
    void loadProgress(int current, int total);
    void projectSaved(const QString& filePath);
    void projectLoaded(const QString& filePath);
    void error(const QString& error);
    
private:
    QString lastError_;
};

} // namespace DatasetCreator
