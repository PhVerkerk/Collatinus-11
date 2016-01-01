/*    flexion.h    */

#ifndef FLEXION_H
#define FLEXION_H

#include <QString>
#include <QStringList>
#include <QUrl>

#include "lemme.h"
#include "lemmatiseur.h"

class Flexion: public QObject
{
	Q_OBJECT

	private:
		Lemme        *_lemme;
		Lemmat       *_lemmatiseur;
		// constantes de table html :
		QString const static     entete;
		QString const static     lina;
		QString const static     linb;
		QString const static     linc;
		QString const static     queue;
		QStringList const static cas;
		QStringList const static genres;
		QStringList const static temps;
		// menu
		QString                  menuLem;
		// construction des tableaux par pos
		QString                  tabNom();
		QString                  tabPron();
		QString                  tabAdj();
		QString                  tabV();
	public:
		Flexion (QObject *parent=0);
		QString           forme (int m, bool label=false);
		QString static    gras (QString g);
		QStringList       menu ();
		void              setLemme (Lemme *l);
		void              setMenu (QStringList m);
		QString           tableau (Lemme *l);
        QString           tableaux (MapLem *ml);
		QString           tableaux (MapLem ml);
};

#endif
