#include "CSVReader.h"
#include <QFile>
#include <QTextStream>
#include <QFileInfo>

namespace DatasetCreator {

bool CSVReader::canRead(const QString& filePath) const {
    QFileInfo info(filePath);
    QString ext = "." + info.suffix().toLower();
    return supportedExtensions().contains(ext);
}

DatasetSample CSVReader::read(const QString& filePath) {
    DatasetSample sample(SampleType::Text);
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        sample.setText(in.readAll());
    }
    sample.metadata().id = QFileInfo(filePath).fileName();
    sample.metadata().sourceFile = filePath;
    sample.metadata().timestamp = QDateTime::currentDateTime();
    return sample;
}

QList<DatasetSample> CSVReader::readBatch(const QStringList& files) {
    QList<DatasetSample> samples;
    for (const QString& file : files) {
        samples.append(read(file));
    }
    return samples;
}

QVariantMap CSVReader::extractMetadata(const QString& filePath) {
    QVariantMap meta;
    meta["file_name"] = QFileInfo(filePath).fileName();
    return meta;
}

}
