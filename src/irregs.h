/*      irregs.h    */
# ifndef IRREGS_H
# define IRREGS_H

#include <QList>
#include <QString>
#include "lemmatiseur.h"

class Lemmat;
class Lemme;

class Irreg: public QObject
{
	Q_OBJECT

	private:
		bool         _exclusif;
		QString      _gr;
		QString      _grq;
		Lemmat*      _lemmat;
		Lemme*       _lemme;
		QList<int>   _morphos;
	public:
		Irreg (QString l, QObject *parent=0);
		bool       exclusif ();
		QString    gr ();
		QString    grq ();
		Lemme     *lemme ();
		QList<int> morphos ();
};

# endif
