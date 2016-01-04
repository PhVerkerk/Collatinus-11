/*               lemme.h
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

#ifndef LEMME_H
#define LEMME_H

#include <QMultiMap>
#include <QObject>
#include <QString>
#include "irregs.h"
#include "lemmatiseur.h"
#include "modele.h"

class Irreg;
class Lemmat;
class Lemme;
class Modele;

class Radical: public QObject
{
	private:
		QString       _gr;
		QString       _grq;
		Lemme*        _lemme;
		int           _numero;
	public:
		Radical (QString g, int n, QObject *parent);
		QString gr ();
		QString grq ();
		Lemme*  lemme ();
		Modele* modele ();
		int     numRad ();
};

class Lemme: public QObject
{
	Q_OBJECT
	private:
		QString                 _cle;
		QString                 _gr;
		QString                 _grd;
		QString                 _grq;
		QString                 _grModele;
		QString                 _indMorph;
		QList<Irreg*>           _irregs;
		Modele*                 _modele;
		int                     _nh;
		Lemmat*                 _lemmatiseur;
		QList<int>              _morphosIrrExcl;
		QChar                   _pos;
		QMultiMap<int,Radical*> _radicaux;
		QString                 _renvoi;
		QMap<QString,QString>   _traduction;
	public:
		Lemme (QString  linea, QObject *parent);
		void            ajIrreg (Irreg *irr);
		void            ajRadical (int i, Radical* r);
		void            ajTrad (QString t, QString l);
		QString 		ambrogio();
		QString         cle ();
		QList<int>      clesR ();
		bool            estIrregExcl (int nm);
		QString         gr ();
		QString         grq ();
		QString         grModele ();
		QString         humain (bool html=false, QString l="fr");
		Modele*         modele ();
		int             nh();
		QString static  oteNh (QString g, int &nh);
		QChar           pos ();
		QList<Radical*> radical (int r);
		bool            renvoi();
		QString         traduction(QString l);
		inline bool operator   <(Lemme &l);
};

#endif

