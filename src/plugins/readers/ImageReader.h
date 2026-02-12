#pragma once
#include "core/PluginInterface.h"

namespace DatasetCreator {
class ImageReader : public IDataReader {
public:
    QString name() const override { return "ImageReader"; }
    QString version() const override { return "1.0.0"; }
    QStringList supportedExtensions() const override;
    QStringList supportedMimeTypes() const override;
    bool canRead(QIODevice* device) const override;
    bool canRead(const QString& filePath) const override;
    DatasetSample read(const QString& filePath) override;
    QList<DatasetSample> readBatch(const QStringList& files) override;
    QVariantMap extractMetadata(const QString& filePath) override;
};
}
