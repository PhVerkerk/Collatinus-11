/*      lemmatiseur.h        */

#ifndef LEMMATISEUR_H
#define LEMMATISEUR_H

#include <QtCore/QCoreApplication>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QTextBrowser>

#include "irregs.h"
#include "lemme.h"
#include "modele.h"

class Irreg;
class Lemme;
class Radical;

typedef QMap<Lemme*,QStringList> MapLem;
typedef QPair<QRegExp,QString> Reglep;

class Lemmat: public QObject
{
	Q_OBJECT

	private:
		// fonction d'initialisation
		void                             ajAssims();
		void                             ajContractions();
		void                             lisIrreguliers();
		void                             lisLexique();
		void                             lisModeles();
		void                             lisParPos();
		void                             lisTraductions();
		// variables et utils
		QMap<QString,QString>            assims;
		QStringList                      cherchePieds (int nbr, QString ligne, int i, bool pentam);
		QMap<QString,QString>           _contractions;
		QMultiMap<QString,Desinence*>   _desinences;
		QString                          decontracte (QString d);
		QStringList                      formeq (QString forme, bool *nonTrouve, bool debPhr);
		bool                             inv(Lemme *l, const MapLem ml);
		QMultiMap<QString,Irreg*>       _irregs;
		QString                         _cible; // langue courante, 2 caractères
		QMap<QString,QString>           _cibles;
		QTextBrowser                    _lemmatiseur;
		QMap<QString, Lemme*>           _lemmes;
		QMap<QString, Modele*>          _modeles;
		QStringList                     _morphos;
		QMultiMap<QString,Radical*>     _radicaux;
        QList<Reglep>                   _reglesp;
		QMap<QString,QString>           _variables;
		// options
		bool                            _alpha;
		bool                            _formeT;
		bool                            _html;
		bool                            _majPert;
		bool                            _morpho;
        bool                            _nonRec;
	public:
		Lemmat (QObject *parent=0);
		void                  ajDesinence (Desinence *d);
		void                  ajModele (Modele *m);
		void                  ajRadicaux (Lemme *l);
		QString               assim (QString a);
		QString               cible();
		QMap<QString,QString> cibles();
		QString               desassim (QString a);
		static QString        deramise (QString r);
		QStringList           frequences (QString txt);
		MapLem                lemmatise (QString f);                    // lemmatise une forme
		QString               lemmatiseFichier (QString f,
					           		   		 bool alpha=false,
					           		   		 bool cumVocibus=false,
					           		   		 bool cumMorpho=false,
					           		   		 bool nreconnu=true);
        QStringList           lemmatiseF (QString f, bool deb);
		// lemmatiseM lemmatise une forme en contexte
		MapLem                lemmatiseM (QString f, bool debPhr=true);
		// lemmatiseT lemmatise un texte
		QString               lemmatiseT (QString t,
					           		   bool alpha=false,
					           		   bool cumVocibus=false,
					           		   bool cumMorpho=false,
					           		   bool nreconnu=false);
		Lemme*                lemme (QString l);
        // lemmes(ml) renvoie la liste des graphies des lemmes
		QStringList           lemmes (MapLem ml);
	    Modele*               modele (QString m);
		QString               morpho (int i);
        QString               parPos (QString f);
		QString               scandeTxt (QString texte, bool stats);
		QStringList           suffixes;
		QString               variable (QString v);

		// accesseurs d'options
		bool                  optAlpha();
		bool                  optHtml();
		bool                  optFormeT();
		bool                  optMajPert();
		bool                  optMorpho();

		public slots:
		// modificateur       s d'options
		void                  setAlpha (bool a);
		void                  setCible (QString c);
		void                  setHtml (bool h);
		void                  setFormeT (bool f);
		void                  setMajPert (bool mp);
		void                  setMorpho (bool m);
        void                  setNonRec(bool n);

};

#endif
