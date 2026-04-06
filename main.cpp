#include <QApplication>
#include <QMessageBox>

#include "DBConnection.h"
#include "LoginWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    if (!DBConnection::instance().initialize()) {
        QMessageBox::critical(nullptr,
                              "Database Error",
                              "Unable to open the local database. Check file permissions and try again.");
        return 1;
    }

    LoginWindow window;
    window.show();

    return app.exec();
}
