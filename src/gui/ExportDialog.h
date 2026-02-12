#pragma once
#include <QDialog>

namespace DatasetCreator {
class ExportDialog : public QDialog {
    Q_OBJECT
public:
    explicit ExportDialog(QWidget* parent = nullptr);
};
}
