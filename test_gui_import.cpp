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
    
    qDebug() << "=== DatasetCreator Mixed Dataset Test ===\n";
    
    // Create dataset
    Dataset dataset("Mixed Dataset Test");
    dataset.metadata().description = "Test with text, images, and CSV";
    dataset.metadata().author = "Test Script";
    dataset.metadata().version = "1.0";
    
    qDebug() << "1. Created dataset:" << dataset.metadata().name;
    
    // Initialize plugin manager
    PluginManager pluginManager;
    qDebug() << "\n2. Available plugins:";
    qDebug() << "   Readers:" << pluginManager.availableReaderNames();
    qDebug() << "   Writers:" << pluginManager.availableWriterNames();
    
    // Test importing mixed files (text, images, CSV)
    qDebug() << "\n3. Importing mixed dataset...";
    QDir testDir("../test_data");
    if (testDir.exists()) {
        QStringList filters;
        filters << "*.txt" << "*.md" << "*.csv" << "*.png" << "*.jpg";
        QStringList files = testDir.entryList(filters, QDir::Files);
        
        qDebug() << "   Found" << files.size() << "files in test_data/";
        
        for (const QString& fileName : files) {
            QString filePath = testDir.absoluteFilePath(fileName);
            IDataReader* reader = pluginManager.getReaderForFile(filePath);
            
            if (reader) {
                qDebug() << "   Reading:" << fileName;
                DatasetSample sample = reader->read(filePath);
                dataset.addSample(sample);
                
                QString typeStr;
                switch (sample.type()) {
                    case SampleType::Text: typeStr = "Text"; break;
                    case SampleType::Image: typeStr = "Image"; break;
                    case SampleType::Audio: typeStr = "Audio"; break;
                    case SampleType::Binary: typeStr = "Binary"; break;
                    case SampleType::Multimodal: typeStr = "Multimodal"; break;
                }
                
                qDebug() << "     - Type:" << typeStr;
                qDebug() << "     - Size:" << sample.dataSize() << "bytes";
                qDebug() << "     - ID:" << sample.metadata().id;
            } else {
                qDebug() << "   ERROR: No reader for:" << fileName;
            }
        }
    } else {
        qDebug() << "   ERROR: test_data/ directory not found!";
        return 1;
    }
    
    qDebug() << "\n4. Dataset statistics:";
    qDebug() << "   Total samples:" << dataset.totalSampleCount();
    qDebug() << "   Total size:" << dataset.totalSize() << "bytes";
    
    // Count by type
    int textCount = 0, imageCount = 0, otherCount = 0;
    for (const auto& sample : dataset.samples()) {
        switch (sample.type()) {
            case SampleType::Text: textCount++; break;
            case SampleType::Image: imageCount++; break;
            default: otherCount++; break;
        }
    }
    qDebug() << "   Text samples:" << textCount;
    qDebug() << "   Image samples:" << imageCount;
    qDebug() << "   Other samples:" << otherCount;
    
    // Export to all formats
    qDebug() << "\n5. Exporting to all formats...";
    
    IDataWriter* jsonlWriter = pluginManager.getWriterForFormat("jsonl");
    if (jsonlWriter) {
        bool success = jsonlWriter->write("mixed_output.jsonl", dataset);
        qDebug() << "   JSONL export:" << (success ? "SUCCESS" : "FAILED");
    }
    
    IDataWriter* jsonWriter = pluginManager.getWriterForFormat("json");
    if (jsonWriter) {
        bool success = jsonWriter->write("mixed_output.json", dataset);
        qDebug() << "   JSON export:" << (success ? "SUCCESS" : "FAILED");
    }
    
    IDataWriter* csvWriter = pluginManager.getWriterForFormat("csv");
    if (csvWriter) {
        bool success = csvWriter->write("mixed_output.csv", dataset);
        qDebug() << "   CSV export:" << (success ? "SUCCESS" : "FAILED");
    }
    
    qDebug() << "\n6. Output file details:";
    QDir currentDir(".");
    QStringList outputs = currentDir.entryList(QStringList() << "mixed_output.*", QDir::Files);
    for (const QString& file : outputs) {
        QFileInfo info(file);
        qDebug() << "   -" << file << ":" << info.size() << "bytes";
    }
    
    qDebug() << "\n=== Test Complete ===";
    
    return 0;
}
