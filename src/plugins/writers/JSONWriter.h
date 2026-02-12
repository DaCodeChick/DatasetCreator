#pragma once
#include "core/PluginInterface.h"

namespace DatasetCreator {
class JSONWriter : public IDataWriter {
public:
    QString name() const override { return "JSONWriter"; }
    QString version() const override { return "1.0.0"; }
    QString fileExtension() const override { return ".json"; }
    QString formatName() const override { return "JSON"; }
    QString description() const override { return "Standard JSON format"; }
    bool write(const QString& outputPath, const Dataset& dataset) override;
};
}
