#include "filmlistitem.h"

#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPixmap>
#include <QJsonObject>
#include <QJsonArray>
#include "filmlistview.h"

class FilmListItem::Data
{
public:
  FilmListItem * owner;
  FilmListView * listview;
  QPixmap poster;
  class Frame;
  Frame       * widget;
  QGridLayout * layout;
  QLabel      * lposter;
  QLabel      * ltitle;
  QLabel      * lgenres;
  QLabel      * loverview;
  Data(FilmListItem *own,FilmListView *lv) :
    owner(own),listview(lv),widget(0),layout(0),lposter(0),ltitle(0),lgenres(0),loverview(0){}
};

class FilmListItem::Data::Frame : public QFrame
{
public:
  FilmListItem * parent_;
  FilmListView * listview;
  Frame(FilmListItem *parent,FilmListView *lv):QFrame(lv),parent_(parent),listview(lv){}
protected:
  void mousePressEvent(QMouseEvent *ev)
  {
    if (ev->button()==Qt::LeftButton) {
      //! \todo  сделать вызов превью
    }
    QFrame::mousePressEvent(ev);
  }
};

QWidget *FilmListItem::getAsWidget()
{
  try {
    d->widget = new Data::Frame(this,d->listview);
    d->layout = new QGridLayout();
    d->lposter = new QLabel(d->widget);
    d->ltitle = new QLabel(d->widget);
    d->lgenres = new QLabel(d->widget);
    d->loverview = new QLabel(d->widget);

    d->widget->setObjectName(QStringLiteral("FilmListItem"));
    d->lposter->setObjectName(QStringLiteral("FilmPoster"));
    d->ltitle->setObjectName(QStringLiteral("FilmTitle"));
    d->lgenres->setObjectName(QStringLiteral("FilmGenres"));
    d->loverview->setObjectName(QStringLiteral("FilmOverview"));

    setPoster(d->poster);
    d->ltitle->setText(title);
    d->lgenres->setText(genres.join(" "));
    d->loverview->setText(overview);
  }catch (...) {
    delete d->loverview;  d->loverview = 0;
    delete d->lgenres;    d->lgenres = 0;
    delete d->ltitle;     d->ltitle = 0;
    delete d->lposter;    d->lposter = 0;
    delete d->layout;     d->layout = 0;
    delete d->widget;     d->widget = 0;
    throw;
  }
  return d->widget;
}

const QPixmap &FilmListItem::poster() const
{
  return d->poster;
}

void FilmListItem::setPoster(const QPixmap &value)
{
  d->poster = value;
  if (d->lposter) {
    if (value.isNull()) d->lposter->setText(noposter());
    else d->lposter->setPixmap(d->poster.scaledToWidth(PosterFixetWidth));
  }
}

void FilmListItem::setPoster(const QByteArray &value)
{
  QPixmap px;
  if (px.loadFromData(value)) setPoster(px);
}

bool FilmListItem::load(const QJsonObject &json)
{
  QStringList keys = json.keys();
  for(QStringList::ConstIterator key = keys.begin(); key!=keys.end(); ++key) {
    switch (jsonNames().indexOf(*key)) {
    case JsonNId:
      id=json[*key].toInt();
      break;
    case JsonNAdult:
      adult=json[*key].toBool();
      break;
    case JsonNGenres:
    case JsonNGenreIds:
      genres = loadGenre(json[*key].toArray());
      break;
    case JsonNTitle:
      title=json[*key].toString();
      break;
    case JsonNOverview:
      overview=json[*key].toString();
      break;
    case JsonNPoster:
      poster_path=json[*key].toString();
      break;
    case JsonNVideo:
      video=json[*key].toString();
      break;
    default:
      property[*key] = json[*key].toVariant();
      break;
    }
  }
  return id && !title.trimmed().isEmpty();
}

QStringList FilmListItem::loadGenre(const QJsonArray &json)
{
  QStringList res;
  for(QJsonArray::ConstIterator g=json.begin(); g!=json.end(); ++g) {
    QString genre = g->toObject()["name"].toString().trimmed();
    if (!genre.isEmpty()) res.append(genre);
  }
  return res;
}

const QVector<QString> &FilmListItem::jsonNames()
{
  static QVector<QString> res =
      (QList<QString>()<<QStringLiteral("id")<<QStringLiteral("adult")
       <<QStringLiteral("genres") <<QStringLiteral("genre_ids")
       <<QStringLiteral("title")<<QStringLiteral("overview")
       <<QStringLiteral("poster_path")<<QStringLiteral("video")).toVector();
  return res;
}

const QString &FilmListItem::noposter()
{
  static const QString res = QString::fromUtf8("I'm so sorry, we don't found poster");
  return res;
}

FilmListItem::FilmListItem(FilmListView *parent) :
  id(0),d(new Data(this,parent))
{
}

