#include <QtWidgets/QApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDir>

#include "TileDataUI.hxx"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon("resources/images/app_icons/tiledataeditor_icon_iso.png"));

    QTranslator qtTranslator;
    if(qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        app.installTranslator(&qtTranslator);

    QTranslator TileDataUITranslator;
    if(TileDataUITranslator.load("TileDataEditor_" + QLocale::system().name(), ":/"))
        app.installTranslator(&TileDataUITranslator);

    TileDataUI ui;
    if (!ui.loadFile("resources/data/TileData.json"))
        return -1;

    ui.show();

    return app.exec();
}
