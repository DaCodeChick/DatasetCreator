#pragma once
#include <QString>
#include <QList>
#include <QPair>
#include "core/DatasetSample.h"

namespace DatasetCreator {

/**
 * @brief Command for undoing/redoing split operations
 * Stores the state of sample assignments before a split operation
 */
class SplitCommand {
public:
    SplitCommand() = default;
    
    // Store original sample locations (sample ID -> subset name, empty for root)
    void addOriginalLocation(const QString& sampleId, const QString& subsetName) {
        originalLocations_.append(qMakePair(sampleId, subsetName));
    }
    
    const QList<QPair<QString, QString>>& originalLocations() const {
        return originalLocations_;
    }
    
    void setDescription(const QString& desc) {
        description_ = desc;
    }
    
    QString description() const {
        return description_;
    }
    
private:
    QList<QPair<QString, QString>> originalLocations_; // (sampleId, subsetName)
    QString description_;
};

}
