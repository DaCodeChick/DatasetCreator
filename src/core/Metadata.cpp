#include "Metadata.h"

namespace DatasetCreator {

// SampleMetadata implementation
QVariantMap SampleMetadata::toVariantMap() const {
    QVariantMap map;
    if (!id.isEmpty()) map["id"] = id;
    if (!tags.isEmpty()) map["tags"] = tags;
    if (!labels.isEmpty()) map["labels"] = labels;
    if (!attributes.isEmpty()) map["attributes"] = attributes;
    if (timestamp.isValid()) map["timestamp"] = timestamp.toString(Qt::ISODate);
    if (!sourceFile.isEmpty()) map["source_file"] = sourceFile;
    if (!annotations.isEmpty()) map["annotations"] = annotations;
    return map;
}

SampleMetadata SampleMetadata::fromVariantMap(const QVariantMap& map) {
    SampleMetadata meta;
    meta.id = map.value("id").toString();
    meta.tags = map.value("tags").toStringList();
    meta.labels = map.value("labels").toMap();
    meta.attributes = map.value("attributes").toMap();
    
    QString timestampStr = map.value("timestamp").toString();
    if (!timestampStr.isEmpty()) {
        meta.timestamp = QDateTime::fromString(timestampStr, Qt::ISODate);
    }
    
    meta.sourceFile = map.value("source_file").toString();
    meta.annotations = map.value("annotations").toMap();
    return meta;
}

// SubsetMetadata implementation
QVariantMap SubsetMetadata::toVariantMap() const {
    QVariantMap map;
    map["name"] = name;
    if (!description.isEmpty()) map["description"] = description;
    if (!customMetadata.isEmpty()) map["metadata"] = customMetadata;
    return map;
}

SubsetMetadata SubsetMetadata::fromVariantMap(const QVariantMap& map) {
    SubsetMetadata meta;
    meta.name = map.value("name").toString();
    meta.description = map.value("description").toString();
    meta.customMetadata = map.value("metadata").toMap();
    return meta;
}

// DatasetMetadata implementation
QVariantMap DatasetMetadata::toVariantMap() const {
    QVariantMap map;
    map["name"] = name;
    if (!description.isEmpty()) map["description"] = description;
    if (!version.isEmpty()) map["version"] = version;
    if (created.isValid()) map["created"] = created.toString(Qt::ISODate);
    if (modified.isValid()) map["modified"] = modified.toString(Qt::ISODate);
    if (!author.isEmpty()) map["author"] = author;
    if (!license.isEmpty()) map["license"] = license;
    if (!customMetadata.isEmpty()) map["custom"] = customMetadata;
    return map;
}

DatasetMetadata DatasetMetadata::fromVariantMap(const QVariantMap& map) {
    DatasetMetadata meta;
    meta.name = map.value("name").toString();
    meta.description = map.value("description").toString();
    meta.version = map.value("version").toString();
    
    QString createdStr = map.value("created").toString();
    if (!createdStr.isEmpty()) {
        meta.created = QDateTime::fromString(createdStr, Qt::ISODate);
    }
    
    QString modifiedStr = map.value("modified").toString();
    if (!modifiedStr.isEmpty()) {
        meta.modified = QDateTime::fromString(modifiedStr, Qt::ISODate);
    }
    
    meta.author = map.value("author").toString();
    meta.license = map.value("license").toString();
    meta.customMetadata = map.value("custom").toMap();
    return meta;
}

} // namespace DatasetCreator
