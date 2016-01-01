/*    irregs.cpp */

#include <QStringList>
#include "ch.h"
#include "irregs.h"
#include "lemme.h"
#include "modele.h"

#include <QDebug>

/**
 * \fn Irreg::Irreg (QString l, QObject *parent)
 * \brief Constructeur de la classe Irreg. l est la
 *        clé du lemme dans la map des lemmes du
 *        lemmatiseur (classe Lemmat) représenté par 
 *        le paramètre *parent.
 */
Irreg::Irreg (QString l, QObject *parent)
{
	if (parent != 0)
		_lemmat = qobject_cast<Lemmat*>(parent);
	QStringList ecl = l.split (':');
	_grq = ecl.at (0);
	if (_grq.endsWith ("*"))
	{
		_grq.chop (1);
		_exclusif = true;
	}
	else _exclusif = false;
	_gr = Ch::atone (_grq);
	_lemme = _lemmat->lemme (ecl.at (1));
 	_morphos = Modele::listeI (ecl.at (2));	 
}

/**
 * \fn bool Irreg::exclusif ()
 * \brief True si le lemmes est exclusif, c'est à dire
 *        si la forme régulière calculée par le modèle
 *        est inusitée, et remplace par la forme irrégulière.
 */
bool Irreg::exclusif ()
{
	return _exclusif;
}

/**
 * \fn QString Irreg::gr ()
 * \brief Graphie ramiste sans diacritique.
 */
QString Irreg::gr ()
{
	return _gr;
}

/**
 * \fn QString Irreg::grq ()
 * \brief Graphie ramiset avec diacritiques.
 */
QString Irreg::grq ()
{
	return _grq;
}

/**
 * \fn Lemme* Irreg::lemme ()
 * \brief Le lemme de l'irrégulier.
 */
Lemme* Irreg::lemme ()
{
	return _lemme;
}

/**
 * \fn QList<int> Irreg::morphos ()
 * \brief liste des numéros de morphos
 *        que peut prendre l'irrégulier, en 
 *        tenant compte des quantités.
 */
QList<int> Irreg::morphos ()
{
	return _morphos;
}
