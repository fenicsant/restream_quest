#include "filmlistitem.h"

#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPixmap>
#include <QJsonObject>
#include <QJsonArray>
#include "filmlistview.h"

//! Структура хранения закрытых данных класса FilmListItem.
class FilmListItem::Data
{
public:
  FilmListItem * owner;     //!< Внешний объект.
  FilmListView * listview;  //!< Указатель на список фильмов. Вызывает открытие окна просмотра при клике мыши.
  QPixmap poster;           //!< Обложка фильма в полном размере.
  class Frame;
  Frame       * widget;     //!< Обект выводимый на экран.
  QGridLayout * layout;     //!< Лайаут widget'а.
  QLabel      * lposter;    //!< Уменьшеная обложка.
  QLabel      * ltitle;     //!< Название фильма.
  QLabel      * lgenres;    //!< Жанры фильма.
  QLabel      * loverview;  //!< Краткое описание фильма.

                            //!  Конструктор.
  Data(FilmListItem *own, FilmListView *lv) :
    owner(own), listview(lv), widget(0), layout(0), lposter(0), ltitle(0),
    lgenres(0), loverview(0)
  {}
  ~Data();
  void clearWidget();
};

//! Класс обработывает событие нажатия левой кнопки мыши.
class FilmListItem::Data::Frame : public QFrame
{
public:
  FilmListItem * parent_;   //!< Внешний объект.
  FilmListView * listview;  //!< Указатель на список фильмов для открытия окна просмотра.

                            //! Конструктор.
  Frame(FilmListItem *parent, FilmListView *lv):
    QFrame(lv), parent_(parent), listview(lv){}
protected:
    //! Метод вызывается при нажатии кнопок мыши на объекте.
  void mousePressEvent(QMouseEvent *ev)
  {
    if (ev->button()==Qt::LeftButton) {
      if (listview) listview->previewFilm(parent_);
    }
    QFrame::mousePressEvent(ev);
  }
};

QWidget *FilmListItem::getAsWidget()
{
  if (d->widget) return d->widget;
  static const char * styleSheet =
      "#FilmListItem {border: 2px solid gray ; border-radius: 10px; background-color: #f2fcfc }\n"
      "#FilmListItem:hover {background-color: qradialgradient(spread:pad, cx:0.5, cy:0.5, radius:0.5, fx:0.3831, fy:0.33, stop:0.119441 rgba(255, 255, 255, 255), stop:1 #f2fcfc)}\n"
      "#FilmPoster {font: 14pt \"Tahoma\"; text-align: center center; }\n"
      "#FilmTitle {font: 18pt \"Tahoma\";  color: #070A0C; }\n"
      "#FilmGenres {font: 13pt \"Tahoma\";  color: #070A0C; }\n"
      "#FilmOverview {font: 10pt \"Tahoma\";  color: #070A0C; }\n";
  try {
    d->widget = new Data::Frame(this, d->listview);
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

    d->lposter->setWordWrap(true);
    d->loverview->setWordWrap(true);
    d->ltitle->setAlignment(Qt::AlignCenter);
    d->lgenres->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    d->loverview->setAlignment(Qt::AlignJustify | Qt::AlignTop);
    d->lposter->setFixedWidth(PosterFixetWidth);
    setPoster(d->poster);
    d->ltitle->setText(title);
    d->lgenres->setText(genres.join(", "));
    d->loverview->setText(overview);

    d->layout->addWidget(d->lposter, 1, 1, 3, 1);
    d->layout->addWidget(d->ltitle, 1, 2);
    d->layout->addWidget(d->lgenres, 2, 2);
    d->layout->addWidget(d->loverview, 3, 2);
    d->layout->setRowStretch(3, 999);
    d->widget->setLayout(d->layout);
    d->widget->setStyleSheet(styleSheet);
  }catch (...) {
    d->clearWidget();
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
  adult(false), id(0), d(new Data(this, parent))
{
}

FilmListItem::~FilmListItem()
{
  delete d;
}

FilmListItem::Data::~Data()
{
  clearWidget();
}

void FilmListItem::Data::clearWidget()
{
  delete loverview;  loverview = 0;
  delete lgenres;    lgenres = 0;
  delete ltitle;     ltitle = 0;
  delete lposter;    lposter = 0;
  delete layout;     layout = 0;
  delete widget;     widget = 0;
}
