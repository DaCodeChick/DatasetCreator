#include "JSONLWriter.h"
#include <QFile>
#include <QJsonDocument>
#include <QTextStream>

namespace DatasetCreator {

bool JSONLWriter::write(const QString& outputPath, const Dataset& dataset) {
    QFile file(outputPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&file);
    
    // Write metadata as first line
    QVariantMap metaLine;
    metaLine["_meta"] = dataset.metadata().toVariantMap();
    QJsonDocument metaDoc = QJsonDocument::fromVariant(metaLine);
    out << metaDoc.toJson(QJsonDocument::Compact) << "\n";
    
    // Write samples
    for (const auto& sample : dataset.samples()) {
        QJsonDocument sampleDoc = QJsonDocument::fromVariant(sample.toVariantMap());
        out << sampleDoc.toJson(QJsonDocument::Compact) << "\n";
    }
    
    // Write subsets
    for (const auto& subset : dataset.subsets()) {
        for (const auto& sample : subset.samples()) {
            QVariantMap sampleMap = sample.toVariantMap();
            sampleMap["_subset"] = subset.name();
            QJsonDocument sampleDoc = QJsonDocument::fromVariant(sampleMap);
            out << sampleDoc.toJson(QJsonDocument::Compact) << "\n";
        }
    }
    
    return true;
}

}
