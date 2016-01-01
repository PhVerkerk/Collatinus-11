/*       ch.h     */

#ifndef CH_H
#define CH_H

#include <QRegExp>
#include <QString>
#include <QStringList>

namespace Ch
{
	QStringList    ajoute (QString mot, QStringList liste);
	void           allonge (QString *f);
	QString        atone(const QString a, bool bdc=false);
	void           deQuant (QString *c);
	QString        deramise (QString r);
	void           elide (QString *mp);
    QString const  consonnes="bcdfgjklmnpqrstvxz";
	const QRegExp  reAlphas("(\\w+)");
	const QRegExp  reEspace("\\s+");
	const QRegExp  reLettres("\\w");
	const QRegExp  rePonct("([\\.?!;:]|$$)");
    bool           sort_i(const QString &a, const QString &b);
	QString        versPC (QString k);
	QString        versPedeCerto (QString k);
	QString const  voyelles="āăēĕīĭōŏūŭȳўĀĂĒĔĪĬŌŎŪŬȲЎ"; 
}
#endif
