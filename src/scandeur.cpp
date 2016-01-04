/*   scandeur.cpp
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

#include "ch.h"
#include "lemmatiseur.h"

#include <QDebug>

/**
 * \fn QStringList Lemmat::cherchePieds (int nbr, QString ligne, int i, bool pentam)
 * \brief cherchePieds est une routine récursive qui construit la liste
 * de toutes les combinaisons possibles de dactyles et de spondées
 * en commençant par le caractère d'indice i.
 * Elle est récursive car chaque fois qu'elle a trouvé un D ou un S,
 * elle va chercher toutes les combinaisons possibles à partir
 * du caractère d'indice i+3 ou i+2.
 * Les longues sont notées + et les brèves -.
 * Les mots sont séparés par un espace et on a gardé une trace 
 * des élisions avec un `.
 * Une grande partie de la difficulté vient des voyelles communes
 * ou indéterminées, notées *. S'il n'y avait que des + et des -,
 * on n'aurait que D=+-- et S=++. Avec l'* en plus, il faut considérer
 * toutes les possibilités :
 * s=*+, +* ou **
 * d=*--, +*-, +-*, +**, *-*, **- ou ***.
 **************************/
QStringList Lemmat::cherchePieds (int nbr, QString ligne, int i, bool pentam)
{
    QStringList res;
    QString longueurs="+-*";
    int ll=ligne.count();
    if (i>=ll) // trop loin !
    {
        res << "";
        return res;
    }
    if (nbr == 1) // Dernier pied !
    {
		// qDebug () << i << " " << ll << " " << ligne << ligne[i] << ligne[i+1];
        if (pentam)
        {
            // C'est un pentamètre, je ne dois avoir 
			// qu'une quantité indifférente
            if (ligne[i]==' ') i+=1;
            // J'étais sur un blanc (espace entre deux mots),
			// j'ai avancé d'une syllabe
            if ((i==ll-1) || (ligne[i+1]==' ')) res << "Z";
			// Fin de ligne ou du mot
            else res << "";
            return res;
        }
        else
        {
            // C'est un hexamètre, je cherche encore deux syllabes
            while (!longueurs.contains(ligne[i]) && i<ll) i+=1;
			//qDebug()<<i<<" "<<ll<<" "<<ligne<< ligne[i] << ligne[i+1];
            if (i>ll-2)
            {
                res << "";
                return res;
            }
            if (ligne[i]!='-')
            {
                if (i==ll-2 && longueurs.contains(ligne[i+1]) ) res << "X";
                else if (ligne[i+2]==' ') res << "X";
                else res << "";
            }
            else res << "";
            return res;
        }
    }
    // J'ai traité les cas qui terminent la récursion
    while (!longueurs.contains(ligne[i]) && i<ll) i+=1;
    if (i==ll)
    {   // Encore un cas qui termine
        res << "";
        return res;
    }
    QChar car1=ligne[i];
    int j=i+1;
    while (!longueurs.contains(ligne[j]) && j<ll) j+=1;
    if (j==ll) // Je n'ai qu'une syllabe : fin prématurée de pentamètre ?
    {
        res << "z";
        return res;
    }
    QChar car2=ligne[j];
    QChar car3;
    int k=j+1;
    while (!longueurs.contains(ligne[k]) && k<ll) k+=1;
    if (k==ll) car3=' ';
    else car3=ligne[k];
    if (car1=='-')
    {   // Encore un cas qui termine : aucun pied ne commence par une brève
        res << "";
        return res;
    }
    if (nbr==4 && car1=='+')
        res << Ch::ajoute ("Y",cherchePieds(3,ligne,i+1,true));
    if (nbr==4 && car1=='*')
        res << Ch::ajoute ("y",cherchePieds(3,ligne,i+1,true));
    if (car1=='+' && car2=='+')
        res << Ch::ajoute ("S",cherchePieds(nbr-1,ligne,j+1,pentam));
    if ((car1=='+' && car2=='*')||(car1=='*' && car2=='+')||(car1=='*' && car2=='*'))
        res << Ch::ajoute ("s",cherchePieds(nbr-1,ligne,j+1,pentam));
    if (car1=='+' && car2=='-' && car3=='-')
        res << Ch::ajoute ("D",cherchePieds(nbr-1,ligne,k+1,pentam));
    if (car1=='*' && (car2=='-'||car2=='*') && (car3=='-'||car3=='*'))
        res << Ch::ajoute ("d",cherchePieds(nbr-1,ligne,k+1,pentam));
    if (car1=='+' && ((car2=='*' && (car3=='-'||car3=='*'))||(car2=='-' && car3=='*')))
        res << Ch::ajoute ("d",cherchePieds(nbr-1,ligne,k+1,pentam));
    return res;
}

/**
 * \fn QStringList Lemmat::formeq (QString forme, bool *nonTrouve, bool debPhr)
 * \brief Renvoie forme scandée de toutes les manières possibles en appliquant
 *        les quantités données par les dictionnaires et les règles prosodiques. 
 */
QStringList Lemmat::formeq (QString forme, bool *nonTrouve, bool debPhr)
{
    QStringList lforme;
    *nonTrouve = true;
    if (forme.isEmpty ()) return lforme;
	MapLem mp = lemmatiseM (forme, debPhr);
    if (mp.empty ())
    {
        lforme.append (forme);
        return lforme;
    }
    *nonTrouve = false;
	foreach (Lemme *l, mp.keys())
	{
		foreach (SLem s, mp.value(l))
		{
			if (s.grq == "-")
				lforme.append (l->grq());
        	else lforme.append (parPos(s.grq));
		}
	}
    lforme.removeDuplicates ();
    return lforme;
}

/**
 * \fn QString Lemmat::scandeTxt (QString texte, bool stats)
 * \brief Scande texte, avec les statistiques si stats
 *        est à true, et renvoie le résultat
 */
QString Lemmat::scandeTxt (QString texte, bool stats)
{
    QString schemaMetric;
    QMap <QString, int> freqMetric;
    bool deb_phr;
    int decalage;
    QStringList vers;
    QStringList formes;
    QStringList aff;
    QStringList lignes = texte.split ("\n");
    foreach (QString ligne, lignes)
    {
		QStringList separ;
		if (ligne.isEmpty()) separ.append (ligne);
	    else separ = ligne.split (QRegExp ("\\b"));
		if (separ.at(0).at(0).isLetter()) separ.prepend("");
		if (separ.at(separ.count()-1).at(0).isLetter()) separ.append("");
        // J'ai maintenant une liste de formes et une liste de séparateurs
        // la ligne d'origine est la concaténation de separ[i]
        // Les termes pairs sont les séparateurs.
        // Les termes impairs sont les mots.
        // J'ai toujours un dernier séparateur, éventuellement vide.
        // La scansion peut commencer !
        decalage=aff.count();
        if (separ.size() < 3)
        {
            aff.append (ligne + "<br />\n");
            // C'est une ligne vide ou ne contenant pas de lettre :
            // je la laisse comme elle est !
            continue;
        }
        bool nonTr, nonTrSuiv;
        QStringList lforme;
        QStringList lfs = formeq (separ[1], &nonTrSuiv, true);
        schemaMetric = "";
        for (int i=1;i<separ.length();i+=2)
        {
            aff.append (separ[i-1]);
            lforme = lfs;
            nonTr = nonTrSuiv;
            if (i < separ.length ()-2)
            {
                deb_phr = separ[i+1].contains (Ch::rePonct);
                lfs = formeq (separ[i+2], &nonTrSuiv, deb_phr);
                if (Ch::consonnes.contains(lfs[0].at(0).toLower()))
                    for (int j=0;j<lforme.length();++j)
                        Ch::allonge (&lforme[j]);
                else for (int j=0;j<lforme.length();++j)
                    Ch::elide (&lforme[j]);
            }
            lforme.removeDuplicates ();
            // C'est le bon moment pour extraire le schéma métrique
            if (stats)
            {
                if (nonTr) schemaMetric.append("?"+Ch::versPC(lforme[0])+" ");
				else
            	{
                	QString schMet = Ch::versPedeCerto(lforme[0]);
                	if (lforme.length() > 1) for(int ii=1;ii<lforme.length(); ii++)
                	{
                    	QString schMet2 = Ch::versPedeCerto(lforme[ii]);
                    	if (schMet.size()!=schMet2.size())
                    	{
                        	schMet = "@" + lforme[0];
                        	continue;
                    	}
                    	else for (int j=0; j<schMet.size(); j++)
                        	if (schMet[j] != schMet2[j]) schMet[j]='*';
                    	// En cas de réponse multiple, 
						// je marque comme communes les voyelles qui diffèrent
                	}
                	schemaMetric.append(schMet+" ");
            	}
            }
            // ajouter des parenthèses pour les analyses multiples
            if (lforme.length () > 1)
            {
                lforme[1].prepend ('(');
                lforme[lforme.length () - 1].append (')');
            }
            if (nonTr) aff.append ("<em>"+lforme[0]+"</em>");
            else aff.append (lforme.join (" "));
            // pour les analyses multiples, je dois insérer des espaces.
        }
        aff.append (separ[separ.length ()-1] + "<br />\n");
        // Je termine la ligne par le dernier séparateur et un saut de ligne.
        if (stats)
        {
            // Je cherche des vers dans la prose
            int ii=0;
            int numMot=1;
            int lsch=schemaMetric.count()-10;
            // Un pentamètre compte au moins 10 syllabes, l'hexamètre 12.
            QString longueurs="+-*";
            QStringList result;
            while (ii < lsch)
            {
                while (!longueurs.contains(schemaMetric[ii]) && ii < lsch)
                    ii+=1;
                // Je suis au début du mot
                result.clear();
                if (ii < lsch && schemaMetric[ii] != '-')
                    result = cherchePieds(6, schemaMetric, ii, false);
                // analyse du résultat
                QString numero;
                numero.setNum (ii);
                QString ajout="";
                foreach (QString item, result)
                {
                    if (item.count()==6)
                    {
                        if (ajout=="") ajout="<span style='color:red' title='"+item;
                        else ajout+="\n"+item;
                        int syllabes=0;
                        for (int a=0; a<6;a++)
                        {
                            if (item[a]=='S'||item[a]=='s'||item[a]=='X')
                                syllabes+=2;
                            if (item[a]=='D'||item[a]=='d')
                                syllabes+=3;
                            if (item[a]=='Y'||item[a]=='y'||item[a]=='Z')
                                syllabes+=1;
                        }
                        int j=ii;
                        int nbMots=1;
                        while (syllabes>0)
                        {
                            if (schemaMetric[j]=='?'||schemaMetric[j]=='@') j+=1;
                            else if (longueurs.contains(schemaMetric[j]))
                            {
                                j+=1;
                                syllabes-=1;
                            }
                            else
                            {
                                nbMots+=2;
                                while (!longueurs.contains(schemaMetric[j])&&(j<schemaMetric.size()))
                                    j+=1;
                            }
                        }
                        QString it=item+" : ";
                        for (j=0;j<nbMots;j++)
                            it+=aff[decalage+numMot+j];
                        if (item.endsWith("Z"))
                            it="<span style='color:red'>"+it+"</span>";
                        vers << it+"<br>\n";
                    }
                }
                if (ajout!="")
                {
					// decalage+numMot est le numéro du
					//mot, dans la liste aff, où mon analyse a commencé.
                    aff[decalage+numMot]=ajout+"'>"+aff[decalage+numMot];
					// 3 premiers mots en rouge
                    //aff[decalage+numMot+5]=aff[decalage+numMot+5]+"</span>";
                    //aff[decalage+numMot+5]+="</span>";
                    if (aff.count() > decalage+numMot+5)
                    aff[decalage+numMot+5].append("</span>");
                }
                while ((schemaMetric[ii] != ' ') && ii < lsch)
                    ii+=1;
                numMot+=2;
                // Je suis sur le blanc qui précède un mot
            }
            // Je remplace les +-* par des signes plus conventionnels
            schemaMetric.replace('-', "∪");
            schemaMetric.replace('+', "‑");
            schemaMetric.replace('*', "∪̲");
			//schemaMetric.replace('-', "u");
			//schemaMetric.replace('+', "-");
			//schemaMetric.replace('*', "-\u0306");
			aff.append ("&nbsp;<small>"+schemaMetric + "</small>&nbsp;<br>\n");
			schemaMetric.remove(" ");
			schemaMetric.remove("`");
			// Pour ignorer la longueur de la dernière voyelle
			// if (!schemaMetric.endsWith("\u0306"))
			// { 
			// schemaMetric[schemaMetric.length()-1]='-';
			// schemaMetric.append("\u0306");
			// }
            freqMetric[schemaMetric] +=1;
        }
    }
    if (stats)
    {
        // Il me reste à trier les freqMetric
        formes.clear();
        foreach (QString schM, freqMetric.keys())
            if (freqMetric[schM] > 1) 
            {
				// Je ne garde que les schéma qui apparaissent plus d'une fois.
                int n = freqMetric[schM] + 10000;
                QString numero;
                numero.setNum (n);
                numero = numero.mid (1);
                formes << numero + " : " + schM ;
            }
        formes.sort();
        aff.prepend("<a href='#statistiques'>Statistiques</a> "
					"<a href='#analyses'>Analyses</a><br>\n");
        aff.prepend("<a name='texte'></a>");
		//aff.prepend("------------<br/>\n");
		// Pour séparer la liste du texte.
        vers.prepend("<hr><a href='#texte'>Texte</a> "
					 "<a href='#statistiques'>Statistiques</a><br>\n");
        vers.prepend("<a name='analyses'></a>");
        for (int i=0; i<formes.size(); i++)
        {
            QString lg = formes[i];
            while (lg[0]=='0') lg=lg.mid(1);
            vers.prepend (lg + "<br/>\n");
            // En faisant un prepend, j'inverse l'ordre :
            // le plus fréquent finira premier
        }
        vers.prepend("<hr><a href='#texte'>Texte</a> "
					 "<a href='#analyses'>Analyses</a><br>\n");
        vers.prepend("<a name='statistiques'></a>");
        vers.append("<a href='#texte'>Texte</a> "
					"<a href='#statistiques'>Statistiques</a> "
					"<a href='#analyses'>Analyses</a><br>\n");
        aff << vers;
		// aff.prepend("------------<br/>\n");
		// Pour séparer la liste du texte.
		// foreach (QString ligne, vers) aff.prepend(ligne);
    }
    return aff.join (" ");
}
