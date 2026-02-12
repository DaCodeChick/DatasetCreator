#pragma once

#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QDateTime>

namespace DatasetCreator {

/**
 * @brief Sample-level metadata
 * Stores metadata for individual dataset samples
 */
struct SampleMetadata {
    QString id;                           // Unique identifier
    QStringList tags;                     // Free-form tags
    QVariantMap labels;                   // Classification labels (key: label_name, value: label_value)
    QVariantMap attributes;               // Custom key-value pairs
    QDateTime timestamp;                  // When sample was created/imported
    QString sourceFile;                   // Original file path
    QVariantMap annotations;              // Format-specific annotations (e.g., bboxes, timestamps)
    
    // Serialization
    QVariantMap toVariantMap() const;
    static SampleMetadata fromVariantMap(const QVariantMap& map);
};

/**
 * @brief Subset-level metadata
 * Metadata for a group of related samples
 */
struct SubsetMetadata {
    QString name;                         // Subset name (e.g., "characters", "locations")
    QString description;                  // Subset description
    QVariantMap customMetadata;           // Custom metadata for this subset
    
    QVariantMap toVariantMap() const;
    static SubsetMetadata fromVariantMap(const QVariantMap& map);
};

/**
 * @brief Dataset-level metadata
 * Global metadata for the entire dataset
 */
struct DatasetMetadata {
    QString name;                         // Dataset name
    QString description;                  // Dataset description
    QString version;                      // Dataset version
    QDateTime created;                    // Creation timestamp
    QDateTime modified;                   // Last modification timestamp
    QString author;                       // Dataset author
    QString license;                      // License information
    QVariantMap customMetadata;           // Additional custom metadata
    
    QVariantMap toVariantMap() const;
    static DatasetMetadata fromVariantMap(const QVariantMap& map);
};

} // namespace DatasetCreator
