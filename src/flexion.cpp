/*      flexion.cpp   
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

#include <QList>
#include <QRegExp>

#include "flexion.h"

#include <QDebug>

/**
 * \fn Flexion::Flexion (QObject *parent);
 * \brief Constructeur de la classe Flexion. Le paramètre
 *        *parent est un pointeur sur le lemmatiseur.
 */
Flexion::Flexion (QObject *parent): QObject (parent)
{
	_lemmatiseur = qobject_cast<Lemmat*>(parent);

}

const QString Flexion::entete = "<table border=1 cellspacing=\"0\" cellpadding=\"5\">";
const QString Flexion::lina = "<tr><td>";
const QString Flexion::linb = "</td><td>";
const QString Flexion::linc = "</td></tr>";
const QString Flexion::queue = "</table>";

const QStringList Flexion::cas = QStringList()
	<< "nominatif"<<"vocatif"<<"accusatif"<<"génitif"<<"datif"<<"ablatif";
const QStringList Flexion::genres = QStringList()
	<<"masculin"<<"féminin"<<"neutre";
const QStringList Flexion::temps = QStringList()
	<<"présent"<<"imparfait"<<"futur"
	<<"parfait"<<"plus-que-parfait"<<"futur antérieur";

/**
 * \fn QString Flexion::forme (int n, bool label)
 * \brief Renvoie entre virgules les formes dont
 *        morphologie occupe de rang n dans la liste
 *        des morphologies du lemmatiseur. Si label est
 *        true, le retour est précédé de la morphologie.
 */
QString Flexion::forme (int n, bool label)
{
	if (_lemme == 0) return "lemme absent";
	Modele *m = _lemme->modele();
	QList<Desinence*> ld = m->desinences (n);
	if (ld.empty()) return "-";
	QStringList lres;
	if (label) lres.append (_lemmatiseur->morpho (n));
	foreach (Desinence *d, ld)
	{
		QString grqd = d->grq();
		int nr = d->numRad ();
		QList<Radical*> lr = _lemme->radical (nr);
		foreach (Radical *r, lr)
		{
			QString grqr = r->grq ();
			lres.append (grqr + grqd);
		}
	}
	lres.removeDuplicates();
	return lres.join (", ");
}

/**
 * \fn QString Flexion::gras (QString g)
 * \brief Utilitaire renvoyant g encadré
 *        des balises html <strong> et </strong>.
 */
QString Flexion::gras (QString g)
{
	return QString("<strong>%1</strong>").arg(g);
}

/**
 * \fn void Flexion::setLemme (Lemme *l)
 * \brief Attribue le lemme l à l'objet Flexion.
 *        Aucun tableau ne peut être calculé
 *        avant que cette fonction ait été appelée.
 */
void Flexion::setLemme (Lemme *l)
{
	_lemme = l;
}

/**
 * \fn QString Flexion::tableau (Lemme *l)
 * \brief Renvoie le tableau de flexion de l.
 *        Cette fonction se contente d'appeler
 *        la fonction spécialisée correspondant
 *        à la catégorie du lemme.
 */
QString Flexion::tableau (Lemme *l)
{
	if (l == 0) return "lemme absent\n";
	setLemme (l);
	switch (l->pos().unicode())
	{
		case 'n': return tabNom ();
		case 'p': return tabPron();
		case 'a': return tabAdj();
		case 'v': return tabV();
		default: return l->humain();
	}
	return "erreur tableau";
}

/**
 * \fn QString Flexion::tableaux (MapLem *ml)
 * \brief Calcule les tableau de chaque lemme 
 *        de la MapLem ml (cf. la classe Lemmat),
 *        et renvoie leur concaténation.
 */
QString Flexion::tableaux (MapLem *ml)
{
    QString ret;
    QTextStream fl(&ret);
	menuLem.clear();
    QTextStream flm(&menuLem);
    flm<<"<h4>";
    foreach (Lemme *l, ml->keys())
	{
		// numéro d'homonymie
        flm<<"<a href=\"#"<<l->cle()<<"\">"<<l->grq()<<"</a> "<<l->humain()<<"<br/>";
	}
    flm<<"</h4>";
    foreach (Lemme *l, ml->keys())
        fl<<"<hr/>"<<tableau(l);
    return ret;
}

/**
 * \fn QString Flexion::tabNom()
 * \brief Fonction spécialisée dans les noms.
 */
QString Flexion::tabNom()
{
	QString ret;
	QTextStream fl(&ret);
    fl<<"<hr/><a name=\""<<_lemme->cle()<<"\"></a>"
		<<menuLem<<entete;
	fl<<lina<<"cas"<<linb<<"singulier"<<linb<<"pluriel"<<linc;
	for (int i=1;i<7;++i)
		fl <<lina<<cas[i-1]<<linb<<forme(i)<<linb<<forme(i+6)<<linc;
	fl<<queue;
	return ret;
}

/**
 * \fn QString Flexion::tabPron()
 * \brief Fonction spécialisée dans les pronoms.
 */
QString Flexion::tabPron()
{
	QString ret;
	QTextStream fl(&ret);
    fl<<"<hr/><a name=\""<<_lemme->cle()<<"\"></a>"
		<<menuLem;
	fl<<"singulier<p>";
	fl<<entete;
	fl <<lina<<"cas"<<linb<<genres[0]<<linb<<genres[1]<<linb<<genres[2]<<linc;
	for (int i=13;i<19;++i)
		fl<< lina<<cas[(i-13)%6]<<linb<<forme(i)<<linb<<forme(i+12)<<linb<<forme(i+18)<<linc;
	fl<<queue;
	fl<<"</p>pluriel<p>";
	fl<<entete;
	fl <<lina<<"cas"<<linb<<genres[0]<<linb<<genres[1]<<linb<<genres[2]<<linc;
	for (int i=19;i<25;++i)
		fl<< lina<<cas[(i-19)%6]<<linb<<forme(i)<<linb<<forme(i+12)<<linb<<forme(i+18)<<linc;
	fl<<queue<<"</p>";
	return ret;
}

/**
 * \fn QString Flexion::tabAdj()
 * \brief Fonction spécialisée dans les adjectifs.
 */
QString Flexion::tabAdj()
{
	QString ret;
	QTextStream fl(&ret);
    fl<<"<a name=\""<<_lemme->cle()<<"\"></a>";
	fl<<menuLem;
	fl<<"<p>"<<_lemme->grq()<<"</p>";
	fl<<"<p>"<<genres[0]<<"<p>";
	fl<<entete;
	fl<<"<tr><td colspan=4>singulier</td></tr>";
	fl<<lina<<"cas"<<linb<<"positif"<<linb<<"comparatif"<<linb<<"superlatif"<<linc;
	for (int i=13;i<19;++i)
		fl<<lina<<cas[i-13]<<linb<<forme(i)<<linb<<forme(i+36)<<linb<<forme(i+72)<<linc;
	fl<<"<tr><td colspan=4>pluriel</td></tr>";
	for (int i=19;i<25;++i)
		fl<<lina<<cas[i-19]<<linb<<forme(i)<<linb<<forme(i+36)<<linb<<forme(i+72)<<linc;
	fl<<queue<<"</p>";

	fl<<"<p>"<<genres[1]<<"<p>";
	fl<<entete;
	fl<<"<tr><td colspan=4>singulier</td></tr>";
	fl<<lina<<"cas"<<linb<<"positif"<<linb<<"comparatif"<<linb<<"superlatif"<<linc;
	for (int i=25;i<31;++i)
		fl<<lina<<cas[i-25]<<linb<<forme(i)<<linb<<forme(i+36)<<linb<<forme(i+72)<<linc;
	fl<<"<tr><td colspan=4>pluriel</td></tr>";
	for (int i=32;i<38;++i)
		fl<<lina<<cas[i-32]<<linb<<forme(i)<<linb<<forme(i+36)<<linb<<forme(i+72)<<linc;
	fl<<queue<<"</p>";

	fl<<"<p>"<<genres[2]<<"<p>";
	fl<<entete;
	fl<<"<tr><td colspan=4>singulier</td></tr>";
	fl<<lina<<"cas"<<linb<<"positif"<<linb<<"comparatif"<<linb<<"superlatif"<<linc;
	for (int i=37;i<43;++i)
		fl<<lina<<cas[i-37]<<linb<<forme(i)<<linb<<forme(i+36)<<linb<<forme(i+72)<<linc;
	fl<<"<tr><td colspan=4>pluriel</td></tr>";
	for (int i=43;i<49;++i)
		fl<<lina<<cas[i-43]<<linb<<forme(i)<<linb<<forme(i+36)<<linb<<forme(i+72)<<linc;
	fl<<queue<<"</p>";

	return ret;
}

/**
 * \fn QString Flexion::tabV()
 * \brief Fonction spécialisée dans les verbes.
 */
QString Flexion::tabV()
{
	// menu
	QString menu;
	QTextStream(&menu)
		<<"<hr/>"<<menuLem
        <<"<a name=\""<<_lemme->cle()<<"\"></a>"
        <<"<a href\"#actif\">ACTIF</a><br/>"
		<<"<a href=\"#actif\">indicatif</a>&nbsp;"
		<<"<a href=\"#subactif\">subjonctif</a>&nbsp;"
	    <<"<a href=\"#impactif\">imp&eacute;ratif et infinitif</a>&nbsp;"
	    <<"<a href=\"#partpres\">participe pr&eacute;sent</a>&nbsp;"
	    <<"<a href=\"#partfut\">participe futur</a><br/>"
	    <<"<a href=\"#indpass\">PASSIF</a><br/>&nbsp;"
		<<"<a href=\"#indpass\">indicatif</a>&nbsp;"
	    <<"<a href=\"#subpass\">subjonctif</a>&nbsp;"
	    <<"<a href=\"#ppp\">participe</a>&nbsp;"
	    <<"<a href=\"#adjv\">adjectif verbal</a><br/>";

	QString ret;
	QTextStream fl(&ret);
	fl<<"<a name=\"actif\"></a>";
	fl<<"<div>"<<_lemme->humain()<<"</div>";
	fl <<"<a name=\"indactif\"></a>"
		<<menu
	  	<<"<h4>ACTIF</h4><br/>"
	  	<<"Indicatif infectum<br/>";
	fl<<entete;
	fl<<lina<<temps[0]<<linb<<temps[1]<<linb<<temps[2]<<linc;
	for (int i=121;i<127;++i)
		fl<<lina<<forme(i)<<linb<<forme(i+6)<<linb<<forme(i+12)<<linc;
	fl<<queue<<"</p>";

	fl<<"Indicatif perfectum<br/>";
	fl<<entete;
	fl<<lina<<temps[3]<<linb<<temps[4]<<linb<<temps[5]<<linc;
	for (int i=139;i<145;++i)
		fl<<lina<<forme(i)<<linb<<forme(i+6)<<linb<<forme(i+12)<<linc;
	fl<<queue<<"</p>";

	fl<<"<a name=\"subactif\"></a>";
	fl<<menu;
	fl<<"<p>Subjonctif</p>";
	fl<<entete;
	fl<<lina<<temps[0]<<linb<<temps[1]<<linb<<temps[3]<<linb<<temps[4]<<linc;
	for (int i=157;i<163;++i)
		fl<<lina<<forme(i)<<linb<<forme(i+6)<<linb<<forme(i+12)<<linb<<forme(i+18)<<linc;
	fl<<queue<<"</p>";
		
	fl<<QString("<a name=\"impactif\"></a>"
				"<p>Impératif</p>");
	fl<<entete;
	fl<<lina<<"personne"<<linb<<"singulier"<<linb<<"pluriel"<<linc;
	fl<<lina<<"2&egrave;me pr."<<linb<<forme(181)<<linb<<forme(182)<<linc;
	fl<<lina<<"2&egrave;me fut."<<linb<<forme(183)<<linb<<forme(185)<<linc;
	fl<<lina<<"3&egrave;me fut."<<linb<<forme(184)<<linb<<forme(186)<<linc;
	fl<<queue;

	fl<< "<p>infinitif pr&eacute;sent : "<<forme(187)<<"<br/>"
		 //"infinifif futur : "<<forme(415)<<"</br>"
		 "infinifif parfait : "<<forme(188)<<"</p>";

	fl<<"<a name=\"partpres\"/>";
	fl<<menu;
	fl<<"<p>Participe pr&eacute;sent</p>";
	fl<<entete;
	fl<<lina<<"cas"<<linb<<genres[0]<<linb<<genres[1]<<linb<<genres[2]<<linc;
	for (int i=189;i<201;++i)
		fl<<lina<<cas[(i-189)%6]<<linb<<forme(i)<<linb<<forme(i+12)<<linb<<forme(i+24)<<linc;
	fl<<queue;

	fl<<"<a name=\"partfut\"></a>";
	fl<<menu;
	fl<<"<p>Participe futur</p>";
	fl<<entete;
	fl<<lina<<"cas"<<linb<<genres[0]<<linb<<genres[1]<<linb<<genres[2]<<linc;
	for (int i=225;i<237;++i)
		fl<<lina<<cas[(i-225)%6]<<linb<<forme(i)<<linb<<forme(i+12)<<linb<<forme(i+24)<<linc;
	fl<<queue;

	fl  <<entete
		<<lina<<"g&eacute;rondif accusatif"<<linb<<forme(261)<<linc
		<<lina<<"g&eacute;rondif gé&eacute;nitif "<<linb<<forme(262)<<linc
		<<lina<<"g&eacute;rondif datif"<<linb<<forme(263)<<linc
		<<lina<<"g&eacute;rondif ablatif"<<linb<<forme(264)<<linc
		<<lina<<"supin en -um"<<linb<<forme(265)<<linc
		<<lina<<"supin en -u"<<linb<<forme(266)<<linc
		<<queue;

	fl<<"<a name=\"indpass\"></a><h4>PASSIF</h4><br/>";
	fl<<menu;
	fl<<"Indicatif<br/>";
	fl<<entete;
	fl<<lina<<temps[0]<<linb<<temps[1]<<linb<<temps[2]<<linc;
	for (int i=267;i<273;++i)
		fl<<lina<<forme(i)<<linb<<forme(i+6)<<linb<<forme(i+12)<<linc;
	fl<<queue<<"</p>";

	fl<<"<a name=\"subpass\"></a>";
	fl<<menu;
	fl<<"<p>Subjonctif</p>";
	fl<<entete;
	fl<<lina<<temps[0]<<linb<<temps[1]<<linb<<temps[3]<<linc;
	for (int i=285;i<291;++i)
		fl<<lina<<forme(i)<<linb<<forme(i+6)<<linb<<forme(i+12)<<linc;
	fl<<queue;

	fl<<QString("<p>infinitif présent passif ")<<forme(302)<<"</p>";

	fl<<"<a name=\"ppp\"></a>";
	fl<<menu;
	fl<<"<p>Participe parfait passif</p>";
	fl<<entete;
	fl<<lina<<"cas"<<linb<<genres[0]<<linb<<genres[1]<<linb<<genres[2]<<linc;
	for (int i=303;i<315;++i)
		// TODO ajouter des colspan pour le nombre
		fl<<lina<<cas[(i-303)%6]<<linb<<forme(i)<<linb<<forme(i+12)<<linb<<forme(i+24)<<linc;
	fl<<queue;

	fl<<"<a name=\"adjv\"></a>";
	fl<<menu;
	fl<<"<p>Adjectif verbal</p>";
	fl<<entete;
	fl<<lina<<"cas"<<linb<<genres[0]<<linb<<genres[1]<<linb<<genres[2]<<linc;
	for (int i=339;i<351;++i)
		// TODO ajouter des colspan pour le nombre
		fl<<lina<<cas[(i-339)%6]<<linb<<forme(i)<<linb<<forme(i+12)<<linb<<forme(i+24)<<linc;
	fl<<queue;

	return ret;
}
