/*           modele.h           */
#ifndef MODELE_H
#define MODELE_H

#include <QList>
#include <QMultiMap>
#include <QString>
#include <QStringList>

class Lemmat;
class Modele;

class Desinence: public QObject
{
	Q_OBJECT
	private:
		QString _gr;
		QString _grq;
		int     _morpho;
		Modele *_modele;
		int     _numR;
	public:
		Desinence (QString d, int morph, int nr, Modele *parent=0);
		QString gr ();
		QString grq ();
		Modele* modele ();
		int     morphoNum ();
		int     numRad ();
		void    setModele (Modele *m);
};

class Modele: public QObject
{
	Q_OBJECT
	private:
		QList<int>                 _absents;
		QStringList static const    cles;
		QMultiMap<int,Desinence*>  _desinences;
		QMap<int,QString>          _genRadicaux;
		QString                    _gr;
		QString                    _grq;
		Lemmat                    *_lemmatiseur;
		Modele*                    _pere;
	public:
		Modele (QStringList ll, Lemmat *parent=0);
		bool               absent (int a);
		QList<int>         absents ();
		QList<int>         clesR();
		Desinence*         clone (Desinence *d);
		bool               deja (int m);
		QList<Desinence*>  desinences (int d);         
		QList<Desinence*>  desinences ();         
		bool               estUn (QString m);
		QString            genRadical (int r);
		QString            gr ();
		QString            grq ();
		static QList<int>  listeI(QString l);
		QList<int>         morphos ();
		QChar              pos ();
};

#endif

