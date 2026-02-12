#include "AudioReader.h"
#include <QFileInfo>

namespace DatasetCreator {

bool AudioReader::canRead(const QString& filePath) const {
    QFileInfo info(filePath);
    QString ext = "." + info.suffix().toLower();
    return supportedExtensions().contains(ext);
}

DatasetSample AudioReader::read(const QString& filePath) {
    DatasetSample sample(SampleType::Audio);
    sample.metadata().id = QFileInfo(filePath).fileName();
    sample.metadata().sourceFile = filePath;
    sample.metadata().timestamp = QDateTime::currentDateTime();
    // TODO: Implement audio decoding with QAudioDecoder
    return sample;
}

QList<DatasetSample> AudioReader::readBatch(const QStringList& files) {
    QList<DatasetSample> samples;
    for (const QString& file : files) {
        samples.append(read(file));
    }
    return samples;
}

QVariantMap AudioReader::extractMetadata(const QString& filePath) {
    QVariantMap meta;
    meta["file_name"] = QFileInfo(filePath).fileName();
    return meta;
}

}
