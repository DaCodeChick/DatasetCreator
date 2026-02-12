#pragma once
#include "core/PluginInterface.h"

namespace DatasetCreator {
class CSVReader : public IDataReader {
public:
    QString name() const override { return "CSVReader"; }
    QString version() const override { return "1.0.0"; }
    QStringList supportedExtensions() const override { return {".csv", ".tsv"}; }
    QStringList supportedMimeTypes() const override { return {"text/csv"}; }
    bool canRead(QIODevice* device) const override { Q_UNUSED(device); return true; }
    bool canRead(const QString& filePath) const override;
    DatasetSample read(const QString& filePath) override;
    QList<DatasetSample> readBatch(const QStringList& files) override;
    QVariantMap extractMetadata(const QString& filePath) override;
};
}
