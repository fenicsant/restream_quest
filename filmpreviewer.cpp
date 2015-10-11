#include "filmpreviewer.h"
#include "mediaplayer.h"
#include "ui_filmpreviewer.h"
#include "filmlistitem.h"


FilmPreviewer::FilmPreviewer(QWidget *parent) :
  QWidget(parent),ui(new Ui::FilmPreviewer)
{
  ui->setupUi(this);
  setAutoFillBackground(true);
}

FilmPreviewer::~FilmPreviewer()
{
  delete ui;
}

void FilmPreviewer::setFilm(FilmListItem *value)
{
  if (!value) {
    static FilmListItem hideitem(0);
    value = & hideitem;
  }
  ffilm = value;
  QList<QLabel *> list = findChildren<QLabel *>();
  for(QList<QLabel*>::Iterator labIter = list.begin(); labIter!=list.end(); ++labIter) {
    QLabel *lab = *labIter;
    static QRegExp rxStaticText = QRegExp("^statictext[\\d]+$");
    QString name = lab->objectName();
    if (rxStaticText.indexIn( name ) >=0 ) continue;
    int baseProp = FilmListItem::jsonNames().indexOf(name);
    switch (baseProp) {
    case FilmListItem::JsonNPoster:
      if (ffilm->poster().isNull()) lab->setText(FilmListItem::noposter());
      else lab->setPixmap(ffilm->poster());
      break;
    case FilmListItem::JsonNTitle:
      lab->setText(ffilm->title);
      break;
    case FilmListItem::JsonNGenres:
      lab->setText("\""+ffilm->genres.join("\", \"")+"\"");
      break;
    case FilmListItem::JsonNOverview:
      lab->setText(ffilm->overview);
      break;
    default:
      lab->setText(ffilm->property.value(name).toString());
    }
  }
  ui->player_content->playMedia(ffilm->video);
}
