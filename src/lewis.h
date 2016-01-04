/*                    lewis.h
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

#ifndef LEWIS_H
#define LEWIS_H
#include <QtCore>

typedef QPair <QString, qint64> pairL; 
typedef QList <pairL> llew;

class Dictionnaire: public QObject
{
    Q_OBJECT

    private:
        QString     auteur;
        QString     chData; // chemin complet du fichier djvu ou xml
        QString     cond_jv;
        int         debut;
        bool        djvu;
        QString     echelle; // échelle pour l'extraction d'une image d'un djvu
        QStringList idxDjvu;
        QString     idxJv; // chemin de l'index jv
        bool        ji;
        bool        JI;
        bool        jv;
        QStringList _liens;
        QString     ligneLiens;
        QString     n; // nom
        int         pdj;
        QString     prec; // pages précédente et suivante
        QString     repertoire;
        QString     suiv;
        QString     url;
        bool        xml;
        bool        xsl;
    public:
        Dictionnaire (QString cfg, QObject *parent=0);
        QString     convert (QString source);
        QString     entree_pos (qint64 pos);
        bool        estXml ();
        QString     indexIu ();
        QString     indexJv ();
        bool        lis_index_djvu ();
        QStringList liens (); // renvoie liens
        QString     nom ();
        int         noPageDjvu ();
        QString     page (QStringList req, int no=0); // lien si la requête vient d'un hyperlien djvu
        QString     pageDjvu (int p);
        QString     pageDjvu (QStringList req, int no=0); // surcharge avec calcul des no de page
        QString     pageXml (QStringList req);
        QString     pgPrec ();
        QString     pgSuiv ();
        QString     ramise (QString f);
        void        vide_index ();
        void        vide_ligneLiens ();
};

class ListeDic: public QObject
{
    Q_OBJECT

    private:
    QMultiMap<QString, Dictionnaire*> liste;
    Dictionnaire * currens;
    Dictionnaire * currens2;

    public:
    Dictionnaire * dictionnaire_par_nom (QString nom);
    void ajoute (Dictionnaire *d);
    void change_courant (QString nom);
    Dictionnaire * courant ();
    void change_courant2 (QString nom);
    Dictionnaire * courant2 ();
    // page renvoie le code html de la page
    // du dictionnaire courant 
    // correspondant au lemme l.
};
// Revoir cette classe ListeDic : elle ne devrait pas contenir les dicos courants

#endif

