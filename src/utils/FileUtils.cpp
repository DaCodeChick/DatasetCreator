#include "FileUtils.h"
#include <QDir>
#include <QFileInfo>

namespace DatasetCreator {
namespace FileUtils {

QString formatFileSize(qint64 bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unitIndex = 0;
    double size = bytes;
    
    while (size >= 1024 && unitIndex < 4) {
        size /= 1024;
        unitIndex++;
    }
    
    return QString::number(size, 'f', 2) + " " + units[unitIndex];
}

QStringList getFilesInDirectory(const QString& dirPath, const QStringList& filters) {
    QDir dir(dirPath);
    return dir.entryList(filters, QDir::Files);
}

bool isTextFile(const QString& filePath) {
    QFileInfo info(filePath);
    QStringList textExtensions = {"txt", "md", "cpp", "h", "py", "js", "java"};
    return textExtensions.contains(info.suffix().toLower());
}

}
}
