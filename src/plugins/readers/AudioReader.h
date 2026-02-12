#pragma once
#include "core/PluginInterface.h"

namespace DatasetCreator {
class AudioReader : public IDataReader {
public:
    QString name() const override { return "AudioReader"; }
    QString version() const override { return "1.0.0"; }
    QStringList supportedExtensions() const override { return {".mp3", ".wav", ".ogg", ".flac"}; }
    QStringList supportedMimeTypes() const override { return {"audio/mpeg", "audio/wav", "audio/ogg"}; }
    bool canRead(QIODevice* device) const override { Q_UNUSED(device); return false; }
    bool canRead(const QString& filePath) const override;
    DatasetSample read(const QString& filePath) override;
    QList<DatasetSample> readBatch(const QStringList& files) override;
    QVariantMap extractMetadata(const QString& filePath) override;
};
}
