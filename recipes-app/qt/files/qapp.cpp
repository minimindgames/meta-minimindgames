#include <QApplication>
#include <QPushButton>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qDebug() << "Hello from Qapp";

    QPushButton button ("Hello from Qapp");
    button.show();

    return a.exec();
}
