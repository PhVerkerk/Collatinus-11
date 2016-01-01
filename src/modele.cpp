/*          modele.cpp         */

#include <QDebug>

#include "ch.h"
#include "modele.h"
#include "lemmatiseur.h"

///////////////
// DESINENCE //
///////////////

/**
 * \fn Desinence::Desinence (QString d, int morph, int nr, Modele *parent)
 * \brief Constructeur de la classe Desinence. d est la graphie avec quantités,
 *        morph est le numéro de morphologie (dans la liste de la classe Lemmat),
 *        nr est le numéro de radical accepté par la désinence, et parent est
 *        un pointeur sur le modèle du lemme et du radical qui utilisent cette
 *        désinence.
 */
Desinence::Desinence (QString d, int morph, int nr, Modele *parent)
{
	// '-' est la désinence zéro
	if (d == "-") d = "";
	_grq = d;
	_gr = Ch::atone (_grq);
	_morpho = morph;
	_numR = nr;
	_modele = qobject_cast<Modele*>(parent);
}

/**
 * \fn QString Desinence::gr ()
 * \brief Graphie de la désinence, ramiste et sans quantités.
 */
QString Desinence::gr ()
{
	return _gr;
}

/**
 * \fn QString Desinence::grq ()
 * \brief Graphie ramiste avec quantités.
 */
QString Desinence::grq ()
{
	return _grq;
}

/**
 * \fn Modele* Desinence::modele ()
 * \brief Modèle de la désinence.
 */
Modele* Desinence::modele ()
{
	return _modele;
}

/**
 * \fn int Desinence::morphoNum ()
 * \brief Numéro de morpho de la désinence.
 */
int Desinence::morphoNum ()
{
	return _morpho;
}

/**
 * \fn int Desinence::numRad ()
 * \brief Numéro de radical de la désinence.
 */
int Desinence::numRad ()
{
	return _numR;
}

/**
 * \fn void Desinence::setModele (Modele *m)
 * \brief Attribue un modèle à la désinence.
 */
void Desinence::setModele (Modele *m)
{
	_modele = m;
}

////////////
// MODELE //
////////////

/**
 * \fn Modele::Modele (QStringList ll, Lemmat *parent)
 * \brief Constructeur de la classe modèle. Chaque item
 *        de la liste ll est constitué de champs séparé par
 *        le caractère ':'. Le premier champ est un mot clé.
 *        Le parent est le lemmatiseur. Pour le format du 
 *        fichier data/modeles.la, consulter la documentation
 *        utilisateur.
 */
Modele::Modele (QStringList ll, Lemmat *parent)
{
	_lemmatiseur = qobject_cast<Lemmat*>(parent);
	_pere = 0;
	QMultiMap<QString,int> msuff;
	foreach (QString l, ll)
	{
		// remplacement des variables par leur valeur
		while (l.contains ('$'))
		{
			int d=l.indexOf ('$');
			int f=l.indexOf (';', d);
			QString v;
			if (f<0) v = l.mid (d);
			else v = l.mid (d, f-d);
			l.replace (v, _lemmatiseur->variable (v));
		}
		QStringList eclats = l.simplified().split (":");
	    //modele pere des des+ R   abs
	    //  0    1    2   3    4   5
		int p = cles.indexOf (eclats.first());
		switch (p)
		{
			case 0: // modèle
			    _gr = eclats.at (1);	
				break;
			case 1: // père
				_pere = parent->modele (eclats.at (1));
				break;
			case 2: // des: désinences écrasant celles du père
			case 3: // des+: désinences s'ajoutant à celles du père
				{
					QList<int> li = listeI (eclats.at (1));
					int r = eclats.at (2).toInt ();
					QStringList ld = eclats.at (3).split (';');
					for (int i=0;i<li.count();++i)
					{
						QStringList ldd;
						if (i < ld.count())
							ldd = ld.at(i).split (',');
						else ldd = ld.last().split (',');
						foreach (QString g, ldd)
						{
							Desinence *nd = new Desinence (g, li.at (i), r, this);
							_desinences.insert (nd->morphoNum (), nd);
							_lemmatiseur->ajDesinence (nd);
						}
					}
					// si des+, aller chercher les autres désinences chez le père :
					if (p==3)
					{
						foreach (int i, li)
						{
							QList<Desinence*> ldp = _pere->desinences (i); 
							foreach (Desinence* dp, ldp)
							{
								// cloner la désinece
								Desinence *dh = clone (dp);
								_desinences.insert (i, dh);
								_lemmatiseur->ajDesinence (dh);
							}
						}
					}
					break;
				}
			case 4:   // R:n: radical n
				{
					int nr = eclats.at (1).toInt();
					_genRadicaux[nr] = eclats.at (2);
					break;
				}
			case 5:
				_absents = listeI (eclats.at (1));
				break;
			case 6:   // suffixes suf:<intervalle>:valeur
				{
					QList<int> lsuf = listeI (eclats.at (1));
					QString gr = eclats.at (2); // TODO verif : bien formée ?
					foreach (int m, lsuf)
						msuff.insert (gr, m);
					break;
				}
			case 7:  // sufd: les désinences du père, mais suffixées
				{
					if (_pere != 0)
					{
						QString suf = eclats.at (1);
						QList<Desinence*> ld = _pere->desinences ();
						foreach  (Desinence* d, ld)
						{
							if (_absents.contains (d->morphoNum()))
								// morpho absente chez le descendant
								continue;
							Desinence *dsuf = new Desinence 
								(d->grq()+suf, d->morphoNum(), d->numRad(), this);
							_desinences.insert (dsuf->morphoNum(), dsuf);
							_lemmatiseur->ajDesinence (dsuf);
						}
					}
					break;
				}
			default:
				qDebug()<<"Modèle, erreur"<<l;
		}
				
	} // fin de l'interprétation des lignes
	
	// père
	if (_pere != 0)
	{
		foreach (int m, _pere->morphos ())
		{
			// héritage des désinence
			if (deja (m))
				continue;
			QList<Desinence*> ld = _pere->desinences (m);
			foreach  (Desinence* d, ld)
			{
				if (_absents.contains (d->morphoNum())) // morpho absente chez le descendant
					continue;
				Desinence *dh = clone (d);
				_desinences.insert (dh->morphoNum(), dh);
				_lemmatiseur->ajDesinence (dh);
			}
		}
		// héritage des radicaux
		foreach (Desinence *d, _desinences)
		{
			if (!_genRadicaux.contains (d->numRad()))
			{
				QString nr = _pere->genRadical (d->numRad());
				_genRadicaux.insert (d->numRad(), nr);
			}
		}
		// héritage des absents
		_absents = _pere->absents ();
	}
	// génération des désinences suffixées
	QList<Desinence*> ldsuf;
	foreach (QString suff, msuff.keys())
	{
		foreach (Desinence *d, _desinences)
		{
			if (msuff.values (suff).contains (d->morphoNum()))
			{
				QString gq = d->grq();
				if (gq == "-") gq.clear();
				gq.append (suff);
				Desinence *dsuf = new Desinence 
					(gq, d->morphoNum(), d->numRad(), this);
				ldsuf.insert (dsuf->morphoNum(), dsuf);
			}
		}
	}
	foreach (Desinence *dsuf, ldsuf)
	{
		_desinences.insert (dsuf->morphoNum(), dsuf);
		_lemmatiseur->ajDesinence (dsuf);
	}
}

/**
 * \fn bool Modele::absent (int a)
 * \brief Renvoie true si la morpho de rang a
 *        n'existe pas dans le modèle. Certains
 *        modèles, par exemple, n'ont pas de singulier,
 *        certains verbes n'ont pas de passif.
 */
bool Modele::absent (int a)
{
	return _absents.contains (a);
}

/**
 * \fn QList<int> Modele::absents ()
 * \brief Retourne la liste des numéros des morphos absentes.
 */
QList<int> Modele::absents ()
{
	return _absents;
}

/**
 * \fn QList<int> Modele::clesR ()
 * \brief Liste des numéros de radicaux utilisés, et
 *        rangés dans la map _genRadicaux.
 */
QList<int> Modele::clesR ()
{
	return _genRadicaux.keys();
}

/**
 * \fn Desinence* Modele::clone (Desinence *d)
 * \brief Crée une Désinence copiée sur la désinence d.
 */
Desinence* Modele::clone (Desinence *d)
{
	return new Desinence (d->grq(), d->morphoNum(), d->numRad(), this);
}

/**
 * \fn bool Modele::deja (int m)
 * \brief Renvoie true si la désinence a une morpho de rang m.
 *        Permet de savoir s'il faut aller chercher la désinence
 *        de morpho m chez le modèle père.
 */
bool Modele::deja (int m)
{
	return _desinences.contains (m);
}

/**
 * \fn QList<Desinence*> Modele::desinences (int d)
 * \brief Renvoie la liste des désinence de morpho d du modèle.
 */
QList<Desinence*> Modele::desinences (int d)
{
	return _desinences.values (d);
}

/**
 * \fn QList<Desinence*> Modele::desinences ()
 * \brief Renvoie toutes les désinences du modèle.
 */
QList<Desinence*> Modele::desinences ()
{
	return _desinences.values ();
}

/**
 * \fn bool Modele::estUn (QString m)
 * \brief Renvoie true si le modèle se nomme m, ou si
 *        l'un de ses ancêtre se nomme m
 */
bool Modele::estUn (QString m)
{
	if (_gr == m) return true;
	if (_pere == 0) return false;
	return _pere->estUn (m);
}

/**
 * \fn QString Modele::gr ()
 * \brief Nom du modèle.
 */
QString Modele::gr ()
{
	return _gr;
}

QStringList const Modele::cles = QStringList ()
	<< "modele" << "pere" << "des" << "des+" << "R" << "abs" << "suf" << "sufd";
	//    0           1        2         3       4       5        6        7

/**
 * \fn QString Modele::genRadical (int r)
 * \brief Chaîne permettant de calculer un radical à partir
 *        de la forme canonique d'un lemme. r est le numéro
 *        du radical.
 */
QString Modele::genRadical (int r)
{
	return _genRadicaux[r];
}

/**
 * \fn QList<int> Modele::listeI (QString l)
 * \brief Fonction importante permettant de renvoyer 
 *        une liste d'entiers à partir d'une chaîne.
 *        La chaîne est une liste de sections séparées
 *        par des virgules. Une section peut être soit
 *        un entier, soit un intervalle d'entiers. On
 *        donne alors les limites inférieure et supérieure
 *        de l'intervale, séparées par le caractère '-'.
 *        Nombreux exemples d'intervalles dans le fichier
 *        data/modeles.la.
 */
QList<int> Modele::listeI (QString l)
{
	QList<int> result;
	QStringList lvirg = l.split (',');
	foreach (QString virg, lvirg)
	{
		if (virg.contains ('-'))
		{
			int deb = virg.section('-',0,0).toInt();
			int fin = virg.section('-',1,1).toInt(); 
			for (int i=deb;i<=fin;++i)
				result.append (i);
		}
		else
		{
			result.append (virg.toInt());
		}
	}
	return result;
}

/**
 * \fn QList<int> Modele::morphos ()
 * \brief Liste des numéros des désinences définies par le modèle.
 */
QList<int> Modele::morphos ()
{
	return _desinences.keys ();
}

/**
 * \fn QChar Modele::pos()
 * \brief Retourne la catégorie du modèle, en utilisant
 *        les ancêtres du modèle.
 */
QChar Modele::pos()
{
	if (estUn("uita") || estUn("lupus") || estUn("miles")
		|| estUn("manus") || estUn("res"))
		return 'n';
	if (estUn ("doctus") || estUn ("fortis"))
		return 'a';
	if (estUn ("amo") || estUn ("imitor"))
		return 'v';
	return 'd';
}