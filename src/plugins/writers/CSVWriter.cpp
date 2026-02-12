#include "CSVWriter.h"
#include <QFile>
#include <QTextStream>

namespace DatasetCreator {

bool CSVWriter::write(const QString& outputPath, const Dataset& dataset) {
    QFile file(outputPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&file);
    
    // Write header
    out << "id,type,data,tags,labels,source_file\n";
    
    // Write samples
    for (const auto& sample : dataset.samples()) {
        out << sample.metadata().id << ",";
        out << static_cast<int>(sample.type()) << ",";
        out << "\"" << sample.asText().replace("\"", "\"\"") << "\",";
        out << sample.metadata().tags.join(";") << ",";
        out << ",";  // labels (would need more complex serialization)
        out << sample.metadata().sourceFile << "\n";
    }
    
    return true;
}

}
