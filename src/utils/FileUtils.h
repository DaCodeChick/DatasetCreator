#pragma once
#include <QString>
#include <QStringList>

namespace DatasetCreator {
namespace FileUtils {

QString formatFileSize(qint64 bytes);
QStringList getFilesInDirectory(const QString& dirPath, const QStringList& filters);
bool isTextFile(const QString& filePath);

}
}
