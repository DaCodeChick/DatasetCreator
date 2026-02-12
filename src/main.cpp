#include "gui/MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    app.setApplicationName("DatasetCreator");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("DatasetCreator");
    
    DatasetCreator::MainWindow window;
    window.show();
    
    return app.exec();
}
