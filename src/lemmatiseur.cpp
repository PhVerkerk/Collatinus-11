/*                 lemmatiseur.cpp
 * 
 *  This file is part of COLLATINUS.
 *                                                                            
 *  COLLATINUS is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *                                                                            
 *  COLLATINVS is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *                                                                            
 *  You should have received a copy of the GNU General Public License
 *  along with COLLATINUS; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * © Yves Ouvrard, 2009 - 2016    
 */

/**
 * \file lemmatiseur.cpp
 * \brief module de lemmatisation des formes latines 
 */

#include <QDir>
#include <QFile>
#include <QRegExp>
#include <QTextStream>
#include <iostream>

#include "ch.h"
#include "lemmatiseur.h"

#include <QDebug>
//#include <QElapsedTimer>
//#define DEBOG

/**
 * \fn Lemmat::Lemmat (QObject *parent)
 * \brief Constructeur de la classe Lemmat.
 *
 * Il définit quelques constantes, initialise
 * les options à false, et appelle les fonctions
 * de lecture des données : modèles, lexique,
 * traductions et irréguliers.
 */
Lemmat::Lemmat (QObject *parent): QObject (parent)
{
	// options
	_alpha   = false;
	_formeT  = false;
	_html    = false;
	_majPert = false;
	_morpho  = false;
	// suffixes
	suffixes << "ne"<<"que"<<"ue";
	// assimilations
	ajAssims();
	// contractions
	ajContractions ();
	// lecture des morphos
    QFile f (qApp->applicationDirPath () +"/data/morphos.la");
    f.open (QFile::ReadOnly);
    QTextStream fl (&f);
    while (!fl.atEnd ())
        _morphos.append (fl.readLine ());
    f.close ();

	lisModeles();
	lisLexique();
	lisTraductions();
	lisIrreguliers();
    lisParPos();
}

/**
* \fn void Lemmat::ajAssims ()
* \brief définit les débuts de mots
* non assimilés, et associe à chacun sa 
* forme assimilée.
* Pour rendre Collatinus plus générique,
* il faudrait que ces données se trouvent
* dans des fichiers
* assimilation
*/
void Lemmat::ajAssims ()
{
	assims.insert ("adc","acc");
	assims.insert ("adf","aff");
	assims.insert ("adg","agg");
	assims.insert ("adl","all");
	assims.insert ("adp","app");
	assims.insert ("adq","acq");
	assims.insert ("adr","arr");
	assims.insert ("adst","ast");
	assims.insert ("adsc","asc");
	assims.insert ("adsp","asp");
	assims.insert ("adst","ast");
	assims.insert ("adt","att");
	assims.insert ("aps","abs");
	assims.insert ("conl","coll");
	assims.insert ("conm","comm");
	assims.insert ("conp","comp");
	assims.insert ("conr","corr");
	assims.insert ("exs","ex");
	assims.insert ("inb","imb");
	assims.insert ("inl","ill");
	assims.insert ("inm","imm");
	assims.insert ("inp","imp");
	assims.insert ("inr","irr");
	assims.insert ("obc","occ");
	assims.insert ("obf","off");
	assims.insert ("obp","opp");
	assims.insert ("ops","obs");
	assims.insert ("subc","succ");
	assims.insert ("subf","suff");
	assims.insert ("subg","sugg");
	assims.insert ("subm","summ");
	assims.insert ("subp","supp");
	assims.insert ("supt","subt");
}

/**
 * \fn void Lemmat::ajContractions ()
 * \brief Établit une liste qui donne, pour chaque
 * contraction, la forme non contracte qui lui
 * correspond.
 */
void Lemmat::ajContractions ()
{
	_contractions.insert ("aram","aueram");
	_contractions.insert ("arant","auerant");
	_contractions.insert ("aras","aueras");
	_contractions.insert ("arat","auerat");
	_contractions.insert ("arim","auerim");
	_contractions.insert ("arimus","auerimus");
	_contractions.insert ("arint","auerint");
	_contractions.insert ("aris","aueris");
	_contractions.insert ("arit","auerit");
	_contractions.insert ("aritis","aueritis");
	_contractions.insert ("aro","auero");
	_contractions.insert ("arunt","auerunt");
	_contractions.insert ("assem","auissem");
	_contractions.insert ("assemus","auissemus");
	_contractions.insert ("assent","auissent");
	_contractions.insert ("asses","auisses");
	_contractions.insert ("asset","auisset");
	_contractions.insert ("assetis","auissetis");
	_contractions.insert ("ast","a");
	_contractions.insert ("asti","auisti");
	_contractions.insert ("astis","auistis");
	_contractions.insert ("eram","eueram");
	_contractions.insert ("eras","eueras");
	_contractions.insert ("erat","euerat");
	_contractions.insert ("eramus","eueramus");
	_contractions.insert ("eratis","eueratis");
	_contractions.insert ("erant","euerant");
	_contractions.insert ("erim","euerim");
	_contractions.insert ("eris","eueris");
	_contractions.insert ("erit","euerit");
	_contractions.insert ("erimus","euerimus");
	_contractions.insert ("eritis","eueritis");
	_contractions.insert ("erint","euerint");
	_contractions.insert ("erunt","euerunt");
	_contractions.insert ("esse","euisse");
	_contractions.insert ("esti","euisti");
	_contractions.insert ("estis","euistis");
	_contractions.insert ("umst","um");
	_contractions.insert ("uom","uum");
	_contractions.insert ("ust","us");
}

/**
 * \fn void Lemmat::ajDesinence (Desinence *d)
 * \brief ajoute la désinence d dans la map des 
 * désinences.
 */
void Lemmat::ajDesinence(Desinence *d)
{
	_desinences.insert (Ch::deramise (d->gr()), d);
}

/**
 * \fn void Lemmat::ajRadicaux (Lemme *l)
 * \brief Calcule tous les radicaux du lemme l,
 *  en se servant des modèles, les ajoute à ce lemme,
 *  et ensuite à la map *  des radicaux de la classe Lemmat.
 *
 */
void Lemmat::ajRadicaux (Lemme *l)
{
	// ablŭo=ā̆blŭo|lego|ā̆blŭ|ā̆blūt|is, ere, lui, lutum
	//      0        1    2    3         4
	Modele *m = modele (l->grModele ());
	// insérer d'abord les radicaux définis dans lemmes.la
	foreach (int i, l->clesR())
	{
		QList<Radical*> lr = l->radical (i);
		foreach (Radical *r, lr)
			_radicaux.insert (Ch::deramise (r->gr()), r);
	}
	// pour chaque radical du modèle
	foreach (int i, m->clesR())
	{
		if (l->clesR().contains (i))
			continue;
		QString g = l->grq ();
		Radical *r = NULL;
		{
			QString gen = m->genRadical (i);
			// si gen == 'K', le radical est la forme canonique
			if (gen == "K")
				r = new Radical (g, i, l);
			else
			{
				// sinon, appliquer la règle de formation du modèle
				int oter = gen.section(',',0,0).toInt();
				QString ajouter = gen.section (',',1,1);
    			if (g.endsWith(0x0306)) g.chop(1);
				g.chop (oter);
				if (ajouter != "0")
					g.append (ajouter);
				r = new Radical (g, i, l);
			}
		}
		l->ajRadical (i, r);
		_radicaux.insert (Ch::deramise (r->gr()), r);
	}
}

/**
 * \fn QString Lemmat::assim (QString a)
 * \brief Cherche si la chaîne a peut subir
 *        une assimilation, et renvoie
 *        cette chaîne éventuellement assimilée.
 */
QString Lemmat::assim (QString a)
{
	foreach (QString d, assims.keys())
		if (a.startsWith (d))
		{
			a.replace (d, assims.value (d));
			return a;
		}
	return a;
}

/**
 * \fn QString Lemmat::cible()
 * \brief Renvoie la langue cible dans sa forme
 *        abrégée (fr, uk, de, it, etc.).
 */
QString Lemmat::cible()
{
	return _cible;
}

/**
 * \fn QMap<QString,QString> Lemmat::cibles()
 * \brief Renvoie la map des langues cibles.
 *
 */
QMap<QString,QString> Lemmat::cibles()
{
	return _cibles;
}

/**
 * \fn QString Lemmat::decontracte (QString d)
 * \brief Essaie de remplacer la contractions de d
 *        par sa forme entière, et renvoie le résultat.
 */
QString Lemmat::decontracte (QString d)
{
	foreach (QString cle, _contractions.keys ())
	{
		if (d.endsWith (cle))
		{
			d.chop (cle.length());
			d.append (_contractions.value(cle));
			return d;
		}
	}
	return d;
}

/**
 * \fn QString Lemmat::desassim (QString a)
 * \brief Essaie de remplacer l'assimilation de a
 *        par sa forme non assimilée, et renvoie
 *        le résultat.
 */
QString Lemmat::desassim (QString a)
{
	foreach (QString d, assims.values ())
		if (a.startsWith (d))
		{
			a.replace (d, assims.key (d));
			return a;
		}
	return a;
}

/**
 * \fn MapLem Lemmat::lemmatise (QString f)
 * \brief Le cœur du lemmatiseur
 *
 *  renvoie une QMap<Lemme*,QStringlist> contenant
 *  - la liste de tous les lemmes pouvant donner
 *    la forme f;
 *  - pour chacun de ces lemmes la QStringList des morphologies
 *    correspondant à la forme.
 */
MapLem Lemmat::lemmatise (QString f)
{
	f = Ch::deramise (f);
	MapLem result;
	// formes irrégulières
	QList<Irreg*> lirr = _irregs.values (f);
	foreach (Irreg* irr, lirr)
	{
		foreach (int m, irr->morphos())
		{
		   	SLem sl = {irr->grq(),morpho(m)};
			//result[irr->lemme()].prepend (morpho (m));	
			result[irr->lemme()].prepend (sl);	
		}
	}
	// radical + désinence
	for (int i=0;i<=f.length();++i)
	{
		QString r = f.left(i);
		QString d = f.mid (i);
		QList<Radical*> lrad = _radicaux.values (r);
		if (lrad.empty())
			continue;
		// ii noté ī
		// 1. Patauium, gén. Pataui : Patau.i -> Patau+i.i
		// 2. conubium, ablP conubis : conubi.s -> conubi.i+s
		if ((d.isEmpty() && r.endsWith ('i')) 
			|| (d.startsWith ('i') && !d.startsWith ("ii") && !r.endsWith ('i')) 
			|| (r.endsWith ('i') && !r.endsWith ("ii") && !d.startsWith ('i')))
		{
			QString nf = r+'i'+d;
			MapLem nm = lemmatise (nf);
			foreach (Lemme *nl, nm.keys())
			{
				QList<SLem> lsl = nm.value(nl);
				for (int i=0;i<lsl.count();++i)
					lsl[i].grq.remove(r.length()-1,1);
				result.insert (nl, lsl);
			}
		}
		QList<Desinence*> ldes = _desinences.values (d);
		if (ldes.empty())
			continue;
		foreach (Radical *rad, lrad)
		{
			Lemme *l = rad->lemme();
			foreach (Desinence *des, ldes)
			{
				if (des->modele() == l->modele() 
					&& des->numRad() == rad->numRad ()
					&& !l->estIrregExcl (des->numRad ()))
				{
					if (des->morphoNum() < _morphos.count()-1)
					{
						SLem sl = {rad->grq()+des->grq(),morpho (des->morphoNum())};
						result[l].prepend(sl);
					}
					else 
					{
						SLem sl = {"-",""};
						result[l].prepend(sl);
					}
				}
			}
		}
	}
	return result;
}

/**
 * \fn bool Lemmat::inv (Lemme *l, const MapLem ml)
 * \brief Renvoie true si le lemme l faisant partie
 *        de la MaplLem ml est invariable.
 */
bool Lemmat::inv (Lemme *l, const MapLem ml)
{
	return ml.value(l).at(0).grq == "-";
}

/**
 * \fn MapLem Lemmat::lemmatiseM (QString f, bool debPhr)
 * \brief Renvoie dans une MapLem les lemmatisations de la
 *        forme f. le paramètre debPhr à true indique qu'il 
 *        s'agit d'un début de phrase, et la fonction
 *        peut tenir compte des majuscules pour savoir
 *        s'il s'agit d'un nom propre.
 */
MapLem Lemmat::lemmatiseM (QString f, bool debPhr)
{
	QString res;
	QTextStream fl (&res);
	MapLem mm = lemmatise (f);
	// suffixes
	foreach (QString suf, suffixes)
		if (mm.empty() && f.endsWith (suf))
		{
			QString sf = f;
			sf.chop (suf.length());
			// TODO : aequeque est la seule occurrence
            // de -queque dans le corpus classique
			mm = lemmatiseM (sf, debPhr);
		}
	if (debPhr && f.at (0).isUpper ())
	{
		QString nf = f.toLower ();
		MapLem nmm = lemmatiseM (nf);
		foreach (Lemme *nl, nmm.keys())
			mm.insert (nl, nmm.value (nl));
	}
	// assimilation
	if (mm.empty())
	{
		QString fa = assim (f);
		if (fa != f)
		{
			MapLem nmm = lemmatise (fa);
			foreach (Lemme *nl, nmm.keys())
				mm.insert (nl, nmm.value (nl));
		}
	}
	QString fd = decontracte (f);
	if (fd != f)
	{
		MapLem nmm = lemmatise (fd);
		foreach (Lemme *nl, nmm.keys())
			mm.insert (nl, nmm.value (nl));
	}
	if (mm.empty())
	{
		QString fa = desassim (f);
		if (fa != f)
		{
			MapLem nmm = lemmatise (fa);
			foreach (Lemme *nl, nmm.keys())
				mm.insert (nl, nmm.value (nl));
		}
	}
	if (mm.empty())
	{
		f[0] = f.at (0).toUpper();
		MapLem nmm = lemmatise (f);
		foreach (Lemme *nl, nmm.keys())
			mm.insert (nl, nmm.value (nl));
	}
	return mm;
}

/**
 * \fn QString Lemmat::lemmatiseT (QString t,
 *  						   bool alpha,
 *  						   bool cumVocibus,
 *  						   bool cumMorpho,
 *  						   bool nreconnu)
 * \brief Renvoie sous forme de chaîne la lemmatisation
 *        et la morphologie de chaque mot du texte t.
 *        Les paramètres permettent de classer la sortie
 *        par ordre alphabétique ; de reproduire la 
 *        forme du texte au début de chaque lemmatisation ;
 *        de donner les morphologies de chaque forme ; ou 
 *        de rejeter les échecs en fin de liste. D'autres
 *        paramètres, comme le format de sortie txt ou html,
 *        sont donnés par des variables de classe.
 */
QString Lemmat::lemmatiseT (QString t,
							   bool alpha,
							   bool cumVocibus,
							   bool cumMorpho,
							   bool nreconnu)
{
    // pour mesurer :
	// QElapsedTimer timer;
	// timer.start();
	// Les paramètres et options true outrepassent les false,
    // _majPert et _html sont dans les options de la classe.
    alpha = alpha || _alpha;
	cumVocibus = cumVocibus || _formeT;
	cumMorpho = cumMorpho || _morpho;
    nreconnu = nreconnu || _nonRec;
	// éliminer les chiffres et les espaces surnuméraires
	t.remove (QRegExp ("\\d"));
	t = t.simplified ();
	// découpage en mots
	QStringList lm = t.split (QRegExp ("\\b"));
	// conteneur pour les résultats 
	QStringList lsv;
	// conteneur pour les échecs
	QStringList nonReconnus;
	// lemmatisation pour chaque mot
    for (int i=1;i<lm.length();i+=2)
	{
		QString f = lm.at (i);
		if (f.toInt() != 0)
			continue;
		// nettoyage et identification des débuts de phrase
		QString sep = lm.at (i-1);
		bool debPhr = (i==1 || sep.contains (Ch::rePonct));
		// lemmatisation de la forme
		MapLem map = lemmatiseM (f, _majPert || debPhr);
		// échecs
		if (map.empty())
		{
			if (nreconnu) nonReconnus.append (f+"\n");	
            else 
            {
                if (_html) lsv.append("<li style=\"color:blue;\">"+f+"</li>");
                else lsv.append(f+" ÉCHEC");
            }
		}
        // avec affichage des formes du texte
        else if (cumVocibus)
        {
            QString lin;
            if (_html)
            {
                lin = "<h4>"+f+"</h4><ul>";
                foreach (Lemme *l, map.keys())
                {
                    lin.append ("<li>"+l->humain(true, _cible)+"</li>");
                    if (cumMorpho && !inv(l, map))
                    {
                        lin.append("<ul>");
                        foreach (SLem m, map.value(l))
                            lin.append ("<li>"+m.grq+" "+m.morpho+"</li>");
                        lin.append("</ul>");
                    }
                }
                lin.append ("</ul>");
            }
            else
            {
                lin = " "+f+"\n";
                foreach (Lemme *l, map.keys())
                {
                    lin.append ("  - "+l->humain(false, _cible)+"\n");
                    if (cumMorpho && !inv(l, map))
                    {
                        foreach (SLem m, map.value(l))
                            lin.append ("    . "+m.grq+" "+m.morpho+"\n");
                    }
                }
            }
            lsv.append (lin);
        }
		else // sans les formes du texte
		{
			foreach (Lemme *l, map.keys())
            {
                QString lin = l->humain(_html, _cible);
                if (cumMorpho && !inv (l, map))
                {
                    QTextStream fl(&lin);
                    if (_html) 
                    {
                        fl<<"<ul>";
                        foreach (SLem m, map.value(l))
                            fl<<"<li>"<<m.grq<<" "<<m.morpho<<"</li>";
                        fl<<"</ul>";
                    }
                    else foreach (SLem m, map.value(l))
                        fl<< "\n    . "<<m.grq<<" "<<m.morpho;
                }
                lsv.append (lin);
            }
        }
	} // fin de boucle de lemmatisation pour chaque mot

    if (alpha)
    {
        lsv.removeDuplicates();
        qSort (lsv.begin(), lsv.end(), Ch::sort_i);
    }
	// peupler lRet avec les résultats
	QStringList lRet;
    if (_html)
        lRet.append("<ul>");
    foreach (QString item, lsv)
    {
        if(_html)
            lRet.append("<li>"+item+"</li>");
        else lRet.append("* "+item+"\n");
    }
    if (_html)
        lRet.append("</ul>");
	// non-reconnus en fin de liste si l'option nreconnu
	// est armée
	if (nreconnu && !nonReconnus.empty())
	{
		nonReconnus.removeDuplicates();
        QString nl;
        if (_html) nl="<br/>";
		if (alpha)
			qSort(nonReconnus.begin(), nonReconnus.end(), Ch::sort_i);
		QString titreNR;
		QTextStream (&titreNR) <<"--- "
            << nonReconnus.count()<<"/"<<lm.count()
            << " ("<< ((nonReconnus.count()*100)/lm.count())
            <<" %) FORMES NON RECONNUES ---"<<nl<<"\n";
		lRet.append (titreNR+nl);
		foreach (QString nr, nonReconnus)
			lRet.append (nr+nl);
	}
    // fin de la mesure :
	// qDebug()<<"Eneide"<<timer.nsecsElapsed()<<"ns";
	return lRet.join("");
}

/**
 * \fn QString Lemmat::lemmatiseFichier (QString f, 
 *								  bool alpha, 
 *								  bool cumVocibus,
 *								  bool cumMorpho,
 *								  bool nreconnu)
 * \brief Applique lemmatiseT sur le contenu du fichier
 *        f et renvoie le résultat. Les paramètres sont
 *        les mêmes que ceux de lemmatiseT.
 */
QString Lemmat::lemmatiseFichier (QString f, 
								  bool alpha, 
								  bool cumVocibus,
								  bool cumMorpho,
								  bool nreconnu)
{
	// lecture du fichier
	QFile fichier (f);
    fichier.open (QFile::ReadOnly);
    QTextStream flf (&fichier);
	QString texte = flf.readAll();
	fichier.close();
	return lemmatiseT (texte, alpha, cumVocibus, cumMorpho, nreconnu);
}

/**
 * \fn Lemme* Lemmat::lemme (QString l)
 * \brief cherche dans la liste des lemmes le lemme
 *        dont la clé est l, et retourne le résultat.
 */
Lemme* Lemmat::lemme (QString l)
{
	return _lemmes.value (l);
}

/**
 * \fn QStringList Lemmat::lemmes (MapLem lm)
 * \brief renvoie la liste des graphies des lemmes
 *        de la MapLem lm sans signes diacritiques.
 */
QStringList Lemmat::lemmes (MapLem lm)
{
	QStringList res;
	foreach (Lemme *l, lm.keys())
		res.append(l->gr());
	return res;
}

/**
 * \fn void Lemmat::lisIrreguliers()
 * \brief Chargement des formes irrégulières
 *        du fichier data/irregs.la
 */
void Lemmat::lisIrreguliers()
{
	QFile firr (qApp->applicationDirPath () +"/data/irregs.la");
    firr.open (QFile::ReadOnly);
    QTextStream fli(&firr);
    while (!fli.atEnd ())
	{
		QString lin = fli.readLine ().simplified();
		if (lin.isEmpty() || lin.startsWith ("!"))
			continue;
		Irreg *irr = new Irreg (lin, this);
		if (irr!=0 && irr->lemme()!=0)
			_irregs.insert (Ch::deramise (irr->gr()), irr);
#ifdef DEBOG
		else std::cerr <<"Irréguliers, erreur dans la ligne" << qPrintable(lin);
#endif
	}
    firr.close ();
	// ajouter les irréguliers aux lemmes
	foreach (Irreg *ir, _irregs)
		ir->lemme()->ajIrreg (ir);
}

/**
 * \fn void Lemmat::lisLexique()
 * \brief Lecture des lemmes, synthèse et enregistrement
 *        de leurs radicaux
 */
void Lemmat::lisLexique()
{
	QFile flem (qApp->applicationDirPath () +"/data/lemmes.la");
    flem.open (QFile::ReadOnly);
    QTextStream fll (&flem);
    while (!fll.atEnd ())
	{
		QString lin = fll.readLine ().simplified();
		if (lin.isEmpty() || lin.startsWith ("!"))
			continue;
		Lemme *l = new Lemme (lin, this);
		_lemmes.insert (l->cle(), l);
	}
    flem.close ();
}

/**
 * \fn void Lemmat::lisModeles()
 * \brief Lecture des modèles, synthèse et enregistrement
 *        de leurs désinences
 */
void Lemmat::lisModeles()
{
	QFile fm (qApp->applicationDirPath () +"/data/modeles.la");
    fm.open (QFile::ReadOnly);
    QTextStream flm (&fm);
	QStringList sl;
    while (!flm.atEnd ())
	{
		QString l = flm.readLine ().simplified();
		if ((l.isEmpty() && !flm.atEnd()) || l.startsWith ("!"))
			continue;
		if (l.startsWith ('$'))
		{
			_variables[l.section('=',0,0)]=l.section('=',1,1);
			continue;
		}
		QStringList eclats = l.split (":");
		if ((eclats.at (0) == "modele" || flm.atEnd()) && !sl.empty())
		{
			Modele *m = new Modele (sl, this);
			_modeles.insert (m->gr(), m);
			sl.clear ();
		}
		sl.append (l);
	}
    fm.close ();
}

/**
 * \fn void Lemmat::lisParPos()
 * \brief Lecture des règles de quantité par position
 * enregistrées dans le fichier data/parpos.txt.
 */
void Lemmat::lisParPos()
{
    QFile fpp (qApp->applicationDirPath () +"/data/parpos.txt");
    fpp.open (QFile::ReadOnly);
    QTextStream flp (&fpp);
    //fle.setCodec ("UTF-8");
    QString ligne;
    QStringList rr;
    while (!fpp.atEnd ())
    {
        ligne = fpp.readLine ().simplified();
        if (!ligne.isEmpty () && !ligne.startsWith('!'))
        {
            rr = ligne.split (";");
            _reglesp.append (Reglep(QRegExp(rr.at (0)),rr.at (1)));
        }
    }
    fpp.close ();
}

/**
 * \fn void Lemmat::lisTraductions()
 * \brief Lecture des fichiers de traductions
 *        trouvés dans data/, nommés lemmes, avec
 *        un suffixe corresponant à la langue cible
 *        qu'ils fournissent.
 */
void Lemmat::lisTraductions()
{
	QString nrep = qApp->applicationDirPath()+"/data/";
	QDir rep = QDir(nrep, "lemmes.*");
	QStringList ltr = rep.entryList();
	ltr.removeOne("lemmes.la"); // n'est pas un fichier de traductions
	foreach (QString nfl, ltr)
	{
		// suffixe
		QString suff = QFileInfo(nfl).suffix();
		QFile fl (nrep+nfl);
		fl.open (QFile::ReadOnly);
		QTextStream flfl (&fl);
		// lire le nom de la langue
		flfl.readLine();
		QString lang=flfl.readLine();
		lang=lang.mid(1).simplified();
		_cibles[suff] = lang;

		while (!flfl.atEnd())
		{
			QString lin = flfl.readLine ().simplified();
			if (lin.isEmpty() || lin.startsWith ("!"))
				continue;
			Lemme *l = lemme (Ch::deramise(lin.section (':', 0, 0)));
			if (l!=0)
				l->ajTrad (lin.section (':',1), suff);
#ifdef DEBOG
			else qDebug()<<"traduction, erreur dans la ligne"
				<< lin<<"\n  clé"<<Ch::deramise(lin.section (':', 0, 0));
#endif
		}
		fl.close();
	}
}

/**
 * \fn Modele * Lemmat::modele (QString m)
 * \brief Renvoie l'objet de la classe Modele dont le nom est m.
 */
Modele * Lemmat::modele (QString m)
{
	return _modeles[m];
}

/**
 * \fn QString Lemmat::morpho (int m)
 * \brief Renvoie la chaîne de rang m dans la liste des morphologies
 *        donnée par le fichier data/morphos.la
 */
QString Lemmat::morpho (int m)
{
	if (m<0 ||m >= _morphos.count())
		return "morpho, erreur d'indice";
	return _morphos.at (m-1);
}

/**
 * \fn bool Lemmat::optAlpha()
 * \brief Accesseur de l'option alpha, qui
 *        permet de fournir par défaut des résultats dans 
 *        l'ordre alphabétique.
 */
bool Lemmat::optAlpha()
{
	return _alpha;
}

/**
 * \fn bool Lemmat::optHtml()
 * \brief Accesseur de l'option html, qui
 *        permet de renvoyer les résultats au format html.
 */
bool Lemmat::optHtml()
{
	return _html;
}

/**
 * \fn bool Lemmat::optFormeT()
 * \brief Accesseur de l'option formeT,
 *        qui donne en tête de lemmatisation
 *        la forme qui a été analysée.
 */
bool Lemmat::optFormeT()
{
	return _formeT;
}

/**
 * \fn bool Lemmat::optMajPert()
 * \brief Accesseur de l'option majPert,
 *        qui permet de tenir compte des majuscules
 *        dans la lemmatisation.
 */
bool Lemmat::optMajPert()
{
	return _majPert;
}

/**
 * \fn bool Lemmat::optMorpho()
 * \brief Accesseur de l'option morpho,
 *        qui donne l'analyse morphologique
 *        des formes lemmatisées.
 */
bool Lemmat::optMorpho()
{
	return _morpho;
}

QString Lemmat::parPos(QString f)
{
    bool maj = f.at(0).isUpper ();
    f = f.toLower ();
    foreach (Reglep r, _reglesp)
    {
        f.replace (r.first, r.second);
        //f = f.trimmed ();
    }
    if (maj) f[0] = f[0].toUpper ();
    return f;
}

/**
 * \fn void Lemmat::setAlpha (bool a)
 * \brief Modificateur de l'option alpha.
 */
// modificateurs d'options

void Lemmat::setAlpha (bool a)
{
	_alpha = a;
}

/**
 * \fn void Lemmat::setCible(QString c)
 * \brief Permet de changer la langue cible.
 */
void Lemmat::setCible(QString c)
{
	_cible = c;
}

/**
 * \fn void Lemmat::setHtml (bool h)
 * \brief Modificateur de l'option html.
 */
void Lemmat::setHtml (bool h)
{
	_html = h;
}

/**
 * \fn void Lemmat::setFormeT (bool f)
 * \brief Modificateur de l'option formeT.
 */
void Lemmat::setFormeT (bool f)
{
	_formeT = f;
}

/**
 * \fn void Lemmat::setMajPert (bool mp)
 * \brief Modificateur de l'option majpert.
 */
void Lemmat::setMajPert (bool mp)
{
	_majPert = mp;
}

/**
 * \fn void Lemmat::setMorpho (bool m)
 * \brief Modificateur de l'option morpho.
 */
void Lemmat::setMorpho (bool m)
{
	_morpho = m;
}

void Lemmat::setNonRec (bool n)
{
    _nonRec = n;
}

/**
 * \fn QString Lemmat::variable (QString v)
 * \brief permet de remplacer la métavariable v
 *        par son contenu. Ces métavariables sont
 *        utilisées par le fichier modeles.la, pour
 *        éviter de répéter des suites de désinences.
 *        Elles sont repérées comme en PHP, par leur
 *        premier caractère $.
 */
QString Lemmat::variable (QString v)
{
	return _variables[v];
}
