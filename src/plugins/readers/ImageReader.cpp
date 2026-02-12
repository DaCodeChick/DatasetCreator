#include "ImageReader.h"
#include <QImageReader>
#include <QFileInfo>

namespace DatasetCreator {

QStringList ImageReader::supportedExtensions() const {
    QStringList exts;
    for (const QByteArray& format : QImageReader::supportedImageFormats()) {
        exts.append("." + QString(format).toLower());
    }
    return exts;
}

QStringList ImageReader::supportedMimeTypes() const {
    QStringList mimes;
    for (const QByteArray& mimeType : QImageReader::supportedMimeTypes()) {
        mimes.append(QString(mimeType));
    }
    return mimes;
}

bool ImageReader::canRead(QIODevice* device) const {
    return QImageReader(device).canRead();
}

bool ImageReader::canRead(const QString& filePath) const {
    return QImageReader::imageFormat(filePath).size() > 0;
}

DatasetSample ImageReader::read(const QString& filePath) {
    DatasetSample sample(SampleType::Image);
    QImageReader reader(filePath);
    QImage image = reader.read();
    sample.setImage(image);
    sample.metadata().id = QFileInfo(filePath).fileName();
    sample.metadata().sourceFile = filePath;
    sample.metadata().timestamp = QDateTime::currentDateTime();
    sample.metadata().attributes["width"] = image.width();
    sample.metadata().attributes["height"] = image.height();
    sample.metadata().attributes["format"] = reader.format();
    return sample;
}

QList<DatasetSample> ImageReader::readBatch(const QStringList& files) {
    QList<DatasetSample> samples;
    for (const QString& file : files) {
        samples.append(read(file));
    }
    return samples;
}

QVariantMap ImageReader::extractMetadata(const QString& filePath) {
    QVariantMap meta;
    QImageReader reader(filePath);
    meta["size"] = reader.size();
    meta["format"] = QString(reader.format());
    return meta;
}

}
