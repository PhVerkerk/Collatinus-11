/*     frequences.cpp
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

#include <QHash>

#include "ch.h"
#include "lemmatiseur.h"

#include <QDebug>

/**
 * \fn QStringList Lemmat::lemmatiseF (QString f, bool deb)
 * \brief Lemmatise la chaîne f, sans tenir compte des majuscules
 *        si deb (= début de phrase) est à true, et renvoie le
 *        résultat dans une QStringList.
 */
QStringList Lemmat::lemmatiseF (QString f, bool deb)
{
    QStringList res;
    MapLem ml = lemmatiseM(f, deb);
    foreach (Lemme *l, ml.keys())
        res.append (l->humain());
    //if (res.empty()) res.append(f);
    return res;
}

/**
 * \fn QStringList Lemmat::frequences (QString txt)
 * \brief Lemmatise txt et renvoie le résultat accompagné
 *        d'informations sur la fréquence d'emploi de
 *        chaque lemme.
 */
QStringList Lemmat::frequences (QString txt)
{
    // J'essaie d'échanger comptage et lemmatisation Ph.
    QStringList formes = txt.split (Ch::reEspace);
    QHash <QString, int> freq; // Occurrences par formes
    QStringList res;
    QString forme;
    int c = formes.count ();
    for (int i= 0;i<c;++i)
    {
        forme = formes.at (i);
        if (forme.isEmpty () || forme.toInt ()) continue;
        // supprimer les ponctuations
        int pos = Ch::reAlphas.indexIn (forme);
        if (pos < 0) continue;
        forme = Ch::reAlphas.cap (1);
        if ((i == 0) || formes[i-1].contains (Ch::rePonct)) forme.prepend('*');// = "*" + forme;
        freq[forme]++; // je suppose que le créateur d'entiers l'initialise à 0
        /* sinon prendre des précautons :
           if (freq.keys ().contains (r))
           freq[r] = freq[r]+1;
           else freq[r] = 1;
        */
    }
    QHash<QString,int> lemOcc; // Nombre d'occurrences par lemme
    QHash<QString,QStringList> lemFormUnic; // liste de formes uniques = par lemme
    QHash<QString,QStringList> lemFormAmb; // Liste de formes ambiguës par lemme
    QHash<QString,QStringList> formLemAmb; // Liste de lemmes par forme ambiguë
    foreach (forme, freq.keys())
    {
        if (forme.startsWith("*"))
        {
            QString forme2 = forme.mid(1);
            res = lemmatiseF (forme2, true);
        }
        else res = lemmatiseF (forme, false);
        int occ = freq[forme];
        if (res.count()== 1)
        {
            //lemFormUnic[res.at(0)].append(forme);
            //lemOcc[res.at(0)] += occ;
            lemFormUnic[res.first()].append(forme);
            lemOcc[res.first()] += occ;
        }
        else
        {
            for (int i= 0;i<res.count();i++)
            {
                lemFormAmb[res.at(i)].append(forme);
                lemOcc[res.at(i)] += occ;
            }
            formLemAmb[forme] = res;
        }
    }
    formes.clear ();
    formes << lemFormUnic.keys();
    formes << lemFormAmb.keys();
    formes.removeDuplicates();
    QStringList sortie;
    // formater les nombres pour tri inverse
    int nUnic;
    float nTotLem;
    int nAmb;
    float xAmb;
    foreach (QString lemme, formes)
    {
        nUnic = 0;
        foreach (forme, lemFormUnic[lemme])
        {
            nUnic += freq[forme];
        }
        nAmb = 0;
        xAmb = 0;
        foreach (forme, lemFormAmb[lemme])
        { // Le lemme considéré a des formes qu'il partage avec d'autres lemmes
            nTotLem = 0;
            foreach (QString lem, formLemAmb[forme])
                nTotLem += lemOcc[lem];
            // Je somme les occurrences de tous les lemmes qui se rattachent à ma forme
            nAmb += freq[forme];
            xAmb += freq[forme] * lemOcc[lemme] / nTotLem;
            // J'attribue une contribution de ma forme au lemme au prorata des occ des lemmes
        }
        int n = xAmb + nUnic + 10000.5;
        QString numero;
        numero.setNum (n);
        numero = numero.mid (1);
        n = xAmb+0.5; // pour faire un arrondi et pas une troncature
        sortie << QString ("%1 (%2, %3, %5)\t%4<br/>\n").arg (numero).arg(nUnic).arg(nAmb).arg (lemme).arg(n);
    }
    qSort(sortie.begin(), sortie.end(), Ch::sort_i);
    // déformatage des nombres
    int cs = sortie.count ();
    for (int i= 0;i<cs;++i)
    {
        QString ls = sortie.at (i);
        int z = 0;
        while (ls.at (z) == '0') ++z;
        ls = ls.mid(z);
        if (ls.at(0) == ' ') ls.prepend("&lt;1");
        sortie[i] = ls;
    }
    sortie.insert (0, "légende : n (a, b, c)<br/>\n");
    sortie.insert (1, "n = a+c<br/>\n");
    sortie.insert (2, "a = nombre de formes rattachées seulement à ce lemme<br/>\n");
    sortie.insert (3, "b = nombre de formes ambigu\u00ebs (partagées par plusieurs lemmes)<br/>\n");
    sortie.insert (4, "c = nombre probable de formes ambigu\u00ebs rattachées à ce lemme<br/>\n");
    sortie.insert (5, "------------<br/>\n");
    return sortie;
}
