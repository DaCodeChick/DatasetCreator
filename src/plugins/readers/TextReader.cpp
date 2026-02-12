#include "TextReader.h"
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QMimeDatabase>

namespace DatasetCreator {

QStringList TextReader::supportedExtensions() const {
    return {".txt", ".text", ".md", ".markdown", ".cpp", ".h", ".hpp", ".c",
            ".py", ".js", ".java", ".cs", ".go", ".rs", ".html", ".css",
            ".xml", ".json", ".yaml", ".yml", ".toml", ".ini", ".conf"};
}

QStringList TextReader::supportedMimeTypes() const {
    return {"text/plain", "text/markdown", "application/json",
            "application/xml", "text/html", "text/css"};
}

bool TextReader::canRead(QIODevice* device) const {
    if (!device || !device->isReadable()) {
        return false;
    }
    // Try reading a small sample to see if it's text
    qint64 pos = device->pos();
    QByteArray sample = device->peek(1024);
    device->seek(pos);
    
    // Check if it contains mostly printable characters
    int printable = 0;
    for (char c : sample) {
        if (std::isprint(static_cast<unsigned char>(c)) || std::isspace(static_cast<unsigned char>(c))) {
            printable++;
        }
    }
    return sample.isEmpty() || (printable * 100 / sample.size() > 80);
}

bool TextReader::canRead(const QString& filePath) const {
    QFileInfo info(filePath);
    QString ext = "." + info.suffix().toLower();
    return supportedExtensions().contains(ext);
}

DatasetSample TextReader::read(const QString& filePath) {
    DatasetSample sample(SampleType::Text);
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return sample;
    }
    
    QTextStream in(&file);
    QString content = in.readAll();
    sample.setText(content);
    
    // Set metadata
    sample.metadata().id = QFileInfo(filePath).fileName();
    sample.metadata().sourceFile = filePath;
    sample.metadata().timestamp = QDateTime::currentDateTime();
    sample.metadata().attributes["file_size"] = file.size();
    sample.metadata().attributes["file_extension"] = QFileInfo(filePath).suffix();
    
    return sample;
}

QList<DatasetSample> TextReader::readBatch(const QStringList& files) {
    QList<DatasetSample> samples;
    for (const QString& file : files) {
        samples.append(read(file));
    }
    return samples;
}

QVariantMap TextReader::extractMetadata(const QString& filePath) {
    QVariantMap meta;
    QFileInfo info(filePath);
    meta["file_name"] = info.fileName();
    meta["file_size"] = info.size();
    meta["extension"] = info.suffix();
    meta["modified"] = info.lastModified().toString(Qt::ISODate);
    return meta;
}

} // namespace DatasetCreator
