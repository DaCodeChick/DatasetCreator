#include "core/Dataset.h"
#include "plugins/PluginManager.h"
#include "managers/ImportManager.h"
#include "managers/ExportManager.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>

using namespace DatasetCreator;

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    qDebug() << "=== DatasetCreator Test Program ===\n";
    
    // Create dataset
    Dataset dataset("Test Dataset");
    dataset.metadata().description = "Automated test dataset";
    dataset.metadata().author = "Test Script";
    dataset.metadata().version = "1.0";
    
    qDebug() << "1. Created dataset:" << dataset.metadata().name;
    
    // Initialize plugin manager
    PluginManager pluginManager;
    qDebug() << "\n2. Loaded plugins:";
    qDebug() << "   Readers:" << pluginManager.availableReaderNames();
    qDebug() << "   Writers:" << pluginManager.availableWriterNames();
    qDebug() << "   Supported extensions:" << pluginManager.supportedReadExtensions();
    
    // Test importing files
    qDebug() << "\n3. Importing test files...";
    QDir testDir("../test_data");
    if (testDir.exists()) {
        QStringList files = testDir.entryList(QStringList() << "*.txt" << "*.md" << "*.csv" << "*.png" << "*.jpg", QDir::Files);
        for (const QString& fileName : files) {
            QString filePath = testDir.absoluteFilePath(fileName);
            IDataReader* reader = pluginManager.getReaderForFile(filePath);
            
            if (reader) {
                qDebug() << "   Reading:" << fileName;
                DatasetSample sample = reader->read(filePath);
                dataset.addSample(sample);
                qDebug() << "     - Size:" << sample.dataSize() << "bytes";
                qDebug() << "     - Type:" << static_cast<int>(sample.type());
            } else {
                qDebug() << "   No reader for:" << fileName;
            }
        }
    }
    
    qDebug() << "\n4. Dataset statistics:";
    qDebug() << "   Total samples:" << dataset.totalSampleCount();
    qDebug() << "   Total size:" << dataset.totalSize() << "bytes";
    
    // Test exporting to different formats
    qDebug() << "\n5. Exporting dataset...";
    
    // Export to JSONL
    IDataWriter* jsonlWriter = pluginManager.getWriterForFormat("jsonl");
    if (jsonlWriter) {
        bool success = jsonlWriter->write("output.jsonl", dataset);
        qDebug() << "   JSONL export:" << (success ? "SUCCESS" : "FAILED");
    }
    
    // Export to JSON
    IDataWriter* jsonWriter = pluginManager.getWriterForFormat("json");
    if (jsonWriter) {
        bool success = jsonWriter->write("output.json", dataset);
        qDebug() << "   JSON export:" << (success ? "SUCCESS" : "FAILED");
    }
    
    // Export to CSV
    IDataWriter* csvWriter = pluginManager.getWriterForFormat("csv");
    if (csvWriter) {
        bool success = csvWriter->write("output.csv", dataset);
        qDebug() << "   CSV export:" << (success ? "SUCCESS" : "FAILED");
    }
    
    qDebug() << "\n6. Checking output files:";
    QDir currentDir(".");
    QStringList outputs = currentDir.entryList(QStringList() << "output.*", QDir::Files);
    for (const QString& file : outputs) {
        QFileInfo info(file);
        qDebug() << "   -" << file << ":" << info.size() << "bytes";
    }
    
    qDebug() << "\n=== Test Complete ===";
    
    return 0;
}
