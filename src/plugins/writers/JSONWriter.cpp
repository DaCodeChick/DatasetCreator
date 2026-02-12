#include "JSONWriter.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace DatasetCreator {

bool JSONWriter::write(const QString& outputPath, const Dataset& dataset) {
    QVariantMap rootMap = dataset.toVariantMap();
    QJsonDocument doc = QJsonDocument::fromVariant(rootMap);
    
    QFile file(outputPath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    file.write(doc.toJson(QJsonDocument::Indented));
    return true;
}

}
