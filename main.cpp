
#include <QApplication>
#include <QDesktopWidget>
#include "filmlistview.h"

int main(int ac, char **av)
{
  QApplication app(ac, av);
  FilmListView w;
  w.setWindowTitle("A Move Review for Restream");
  QRect rect = app.desktop()->screenGeometry(&w);
  w.resize(rect.width()*2/3, rect.height()*2/3);
  w.move((rect.width()-w.width())/2, (rect.height()-w.height())/2);
  w.beginReload();
  w.show();
  app.exec();
  return 0;
}
