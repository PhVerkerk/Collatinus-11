/*      lemme.cpp    */

#include <QString>
#include <QStringList>
#include "ch.h"
#include "lemmatiseur.h"
#include "lemme.h"
#include "modele.h"
#include <QDebug>

/////////////
// RADICAL //
/////////////
/**
 * \fn Radical::Radical (QString g, int n, QObject *parent)
 * \brief Créateur de la classe Radical. g est la forme
 *        canonique avec ses quantités, n est le numéro du radical
 */
Radical::Radical (QString g, int n, QObject *parent)
{
	_lemme = qobject_cast<Lemme*>(parent);
	_grq = g;
	_gr = Ch::atone (_grq);
	_numero = n;
}

/**
 * \fn QString Radical::gr ()
 * \brief Renvoie la graphie du radical
 *        dépourvue de diacritiques.
 */
QString Radical::gr ()
{
	return _gr;
}

/**
 * \fn QString Radical::grq ()
 * \brief Renvoie la graphie du radical
 *        pourvue de ѕes diacritiques.
 */
QString Radical::grq ()
{
	return _grq;
}

/**
 * \fn Lemme* Radical::lemme ()
 * \brief Le lemme auquel appartient le radical.
 */
Lemme* Radical::lemme ()
{
	return _lemme;
}

/**
 * \fn Modele* Radical::modele ()
 * \brief Le modèle de flexion du radical
 */
Modele* Radical::modele ()
{
	return _lemme->modele ();
}

/**
 * \fn int Radical::numRad ()
 * \brief Le numéro du radical.
 */
int Radical::numRad ()
{
	return _numero;
}

///////////
// LEMME //
///////////

/**
 * \fn Lemme::Lemme (QString linea, QObject *parent)
 * \brief Constructeur de la classe Lemme à partire de la
 *        ligne linea. *parent est le lemmatiseur (classe Lemmat).
 */
Lemme::Lemme (QString linea, QObject *parent)
{
    // cădo|lego|cĕcĭd|cās|is, ere, cecidi, casum
	//   0   1    2     3          4
	_lemmatiseur = qobject_cast<Lemmat*>(parent);
	QStringList eclats = linea.split ('|');
	QStringList lg = eclats.at(0).split ('=');
	_cle = Ch::atone(Ch::deramise (lg.at (0)));
	_grd = oteNh (lg.at(0), _nh);
	if (lg.count()==1)
		_grq = _grd;
	else
		_grq = lg.at (1);
	_gr = Ch::atone (_grq);
	_grModele = eclats.at (1);
	_modele = _lemmatiseur->modele (_grModele);	
	// lecture des radicaux, champs 2 et 3
	for (int i=2;i<4;++i)
		if (!eclats.at (i).isEmpty())
		{
			QStringList lrad = eclats.at (i).split (',');
			foreach (QString rad, lrad)
				_radicaux.insert (i-1, new Radical (rad, i-1, this));
		}
	_lemmatiseur->ajRadicaux (this);

	// écrire un contrôle d'erreur

	_indMorph = eclats.at (4);
	_pos = '-';
    if      (_indMorph.contains ("adj.")) _pos = 'a';
    else if (_indMorph.contains ("conj")) _pos = 'c';
    else if (_indMorph.contains ("excl")) _pos = 'e';
    else if (_indMorph.contains ("interj")) _pos = 'i';
    else if (_indMorph.contains ("num")) _pos = 'm';
    else if (_indMorph.contains ("pron.")) _pos = 'p';
	else if (_indMorph.contains ("prép")) _pos = 'r';
    else if (_indMorph.contains ("adv")) _pos = 'd';
    else if (_indMorph.contains ("n. ")) _pos = 'n';
	else _pos = _modele->pos();

	QRegExp c("cf\\.\\s(\\w+)$");
	int pos = c.indexIn(_indMorph);
	if (pos > -1)
    {
        //_renvoi = Lemmat::deramise (_indMorph.mid (pos + 4));
        _renvoi = c.cap(1);
    }
    else _renvoi = "";
}

/**
 * \fn void Lemme::ajIrreg (Irreg *irr)
 * \brief Ajoute au lemme l'obet irr, qui représente
 *        une forme irrégulière. Lorsque les formes irrégulières
 *        sont trop nombreuses, ou lorsque plusieurs lemmes 
 *        ont des formes analogues, mieux vaut ajouter un modèle
 *        dans data/modeles.la.
 */
void Lemme::ajIrreg (Irreg *irr)
{
	_irregs.append (irr);
	// ajouter les numéros de morpho à la liste 
	// des morphos irrégulières du lemme :
	if (irr->exclusif())
  	_morphosIrrExcl.append (irr->morphos ());
}

/**
 * \fn void Lemme::ajRadical (int i, Radical* r)
 * \brief Ajoute le radical r de numéro i à la map des
 *        radicaux du lemme.
 */
void Lemme::ajRadical (int i, Radical* r)
{
	_radicaux.insert (i, r);
}

/**
 * \fn void Lemme::ajTrad (QString t, QString l)
 * \brief ajoute la traduction t de langue l à 
 *        la map des traductions du lemme.
 */
void Lemme::ajTrad (QString t, QString l)
{
	_traduction[l] = t;
}

/**
 * \fn QString Lemme::ambrogio()
 * \brief Renvoie dans une chaîne un résumé
 *        de la traduction du lemme dans toutes les
 *        langues cibles disponibles.
 */
QString Lemme::ambrogio()
{
    QString retour;
    QTextStream ss (&retour);
	ss << "<hr/>"<<humain()<<"<br/>";
    ss << "<table>";
    foreach (QString lang, _traduction.keys ())
    {
		QString trad = _traduction[lang];
		QString langue = _lemmatiseur->cibles()[lang];
        if (!trad.isEmpty())
        ss <<"<tr><td>- "<<langue<<"</td><td>&nbsp;"<<trad<<"</td></tr>\n";
    }
    ss << "</table>";
    return retour;
}

/**
 * \fn QString Lemme::cle ()
 * \brief Renvoie la clé sous laquel le 
 *        lemme est enregistré dans le lemmatiseur parent.
 */
QString Lemme::cle ()
{
	return _cle;
}

/**
 * \fn QList<int> Lemme::clesR ()
 * \brief Retourne toutes les clés (formes non-ramistes
 *        sans diacritiques) de la map des radicaux du lemme.
 */
QList<int> Lemme::clesR ()
{
	return _radicaux.keys();
}

/**
 * \fn bool Lemme::estIrregExcl (int nm)
 * \brief Renvoie vrai si le radical remplace
 *        la forme irrégulière, faux si la 
 *        forme régulière existe aussi.
 */
bool Lemme::estIrregExcl (int nm)
{
	return _morphosIrrExcl.contains (nm);
}

/**
 * \fn return _gr;
 * \brief Retourne la graphie ramiste du lemme sans diacritiques.
 */
QString Lemme::gr ()
{
	return _gr;
}

/**
 * \fn QString Lemme::grq ()
 * \brief Retourne la graphie ramiste du lemme sans diacritiques.
 */
QString Lemme::grq ()
{
	return _grq;
}

/**
 * \fn QString Lemme::grModele ()
 * \brief Retourne la graphie du modèle du lemme.
 */
QString Lemme::grModele ()
{
	return _grModele;
}

/**
 * \fn QString Lemme::humain (bool html, QString l)
 * \brief Retourne une chaîne donnant le lemme ramiste avec diacritiques,
 *        ses indications morphologiques et sa traduction dans la langue l.
 *        Si html est true, le retour est au format html.
 */
QString Lemme::humain (bool html, QString l)
{
	QString res;
	QString tr;
	if (!_renvoi.isEmpty())
	{
		Lemme *lr = _lemmatiseur->lemme(_renvoi);
		if (lr != 0)
			tr = _lemmatiseur->lemme(_renvoi)->traduction(l);
		else tr="renvoi non trouvé";
	}
	else tr = traduction(l);
    if (html)
        QTextStream (&res) << "<strong>"<<_grq<<"</strong> "
            <<"<em>"<<_indMorph<<"</em> : " << tr;
	else QTextStream (&res) << _grq << ", " << _indMorph << " : " << tr;
	return res;
}

/**
 * \fn Modele* Lemme::modele ()
 * \brief Renvoie l'objet modèle du lemme.
 */
Modele* Lemme::modele ()
{
	return _modele;
}

/**
 * \fn int Lemme::nh()
 * \brief Renvoie le numéro d'homonymie du lemme.
 */
int Lemme::nh()
{
	return _nh;
}

/**
 * \fn QString Lemme::oteNh (QString g, int &nh)
 * \brief Supprime le dernier caractère de g si c'est
 *        un nombre et revoie le résultat après avoir
 *        donné la valeur de ce nombre à nh.
 */
QString Lemme::oteNh (QString g, int &nh)
{
	int c = g.right (1).toInt();
	if (c > 0)
	{
		nh = c;
		g.chop (1);
	}
	else
		c = 1;
	return g;
}

/**
 * \fn QChar Lemme::pos ()
 * \brief Renvoie un caractère représentant la 
 *        catégorie (part of speech, pars orationis)
 *        du lemme.
 */
QChar Lemme::pos ()
{
	return _pos;
}

/**
 * \fn QList<Radical*> Lemme::radical (int r)
 * \brief Renvoie le radical numéro r du lemme.
 */
QList<Radical*> Lemme::radical (int r)
{
	return _radicaux.values (r);
}

/**
 * \fn bool Lemme::renvoi()
 * \brief Renvoie true si le lemme est une forme
 *        alternative renvoyant à une autre entrée
 *        du lexique.
 */
bool Lemme::renvoi()
{
	return _indMorph.contains("cf. ");
}

/**
 * \fn QString Lemme::traduction(QString l)
 * \brief Renvoie la traduction du lemme dans la langue
 *        cible l (2 caractères).
 */
QString Lemme::traduction(QString l)
{
	if (_traduction.keys().contains(l))
		return _traduction[l];
	else return _traduction["fr"];
}

/**
 * \fn bool Lemme::operator<(Lemme &l)
 * \brief vrai si la graphie du lemme de gauche 
 *        précède celle de celui de droite dans 
 *        l'ordre alphabétique.
 */
bool Lemme::operator<(Lemme &l)
{
	return _gr < l.gr();
}
