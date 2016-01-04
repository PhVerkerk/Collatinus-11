/*   mainwindow.cpp
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

#include <QDebug>
#include <QPrinter>
#include <QPrintDialog>

#include "flexion.h"
#include "mainwindow.h"

/**
 * \fn EditLatin::EditLatin (QWidget *parent): QTextEdit (parent)
 * \brief Créateur de la classe EditLatin, dérivée de
 * QTextEdit afin de pouvoir redéfinir l'action
 * connectée au clic de souris sur un mot ou après
 * sélection d'une portion de texte.
 */
EditLatin::EditLatin (QWidget *parent): QTextEdit (parent)
{	
	mainwindow = qobject_cast<MainWindow*>(parent);
}

/**
 * \fn bool EditLatin::event(QEvent *event)
 * \brief Captation du survol de la souris pour
 *        afficher dans une bulle lemmatisation et 
 *        analyses morphologiques.
 */
bool EditLatin::event(QEvent *event)
{
	switch (event->type())
	{
		case QEvent::ToolTip:
			{
        		QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
        		QPoint P = mapFromGlobal(helpEvent->globalPos());
        		QTextCursor tc = cursorForPosition (P);
				tc.select (QTextCursor::WordUnderCursor);
				QString mot = tc.selectedText();
				QString txtBulle = mainwindow->lemmatiseur->lemmatiseT (mot, true, true, true, false);
				txtBulle.prepend ("<p style='white-space:pre'>");
				txtBulle.append ("</p>");
        		QToolTip::setFont (font ());
        		QToolTip::showText (helpEvent->globalPos(), txtBulle.trimmed (), this);
				return true;
    		}
		default: return QTextEdit::event (event);
	}
}

/**
 * \fn void EditLatin::mouseReleaseEvent (QMouseEvent *e)
 * \brief Captation de la fin du clic de souris : ajout
 *        des lemmatisations et analyses morpho dans
 *        le dock correspondant.
 */
void EditLatin::mouseReleaseEvent (QMouseEvent *e)
{
    QTextCursor cursor = textCursor();
    if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);
	QString st = cursor.selectedText();
	bool unSeulMot = !st.contains (' ');
	MapLem ml = mainwindow->lemmatiseur->lemmatiseM (st);
	if (!mainwindow->dockLem->visibleRegion().isEmpty())
    {
		if (unSeulMot && mainwindow->calepAct->isChecked())
			foreach (Lemme *l, ml.keys())
				mainwindow->textEditLem->append(l->ambrogio());
		else
		{
        	if (mainwindow->html())
        	{
            	mainwindow->textEditLem->append(mainwindow->lemmatiseur->lemmatiseT(st));
        	}
        	else mainwindow->textEditLem->insertPlainText(mainwindow->lemmatiseur->lemmatiseT(st));
		}
    }
	if (unSeulMot)
	{
		// 1. dock de lemmatisation
		if (!mainwindow->dockFlex->visibleRegion().isEmpty())
		{
			if (!ml.empty())
			{
				mainwindow->textBrowserFlex->clear();
				mainwindow->textBrowserFlex->append (mainwindow->flechisseur->tableaux(&ml));
				mainwindow->textBrowserFlex->moveCursor (QTextCursor::Start);
			}
		}
		// 2. dock dictionnaires
		QStringList lemmes = mainwindow->lemmatiseur->lemmes(ml);
		if (!mainwindow->dockDic->visibleRegion().isEmpty())
			mainwindow->afficheLemsDic(lemmes);
		if (mainwindow->wDic->isVisible() && mainwindow->syncAct->isChecked())
			mainwindow->afficheLemsDicW(lemmes);
	}
	QTextEdit::mouseReleaseEvent (e);
}

/**
 * \fn MainWindow::MainWindow()
 * \brief Créateur de la fenêtre de l'appli.
 *        Les différentes tâches sont regrouppées
 *        et confiées à des fonctions spécialisées.
 */
MainWindow::MainWindow()
{
	QFile styleFile (":/res/collatinus.css");
	styleFile.open (QFile::ReadOnly);
	QString style (styleFile.readAll());
	qApp->setStyleSheet (style);

    editLatin = new EditLatin (this);
    setCentralWidget(editLatin);

	lemmatiseur = new Lemmat(this);
	flechisseur = new Flexion (lemmatiseur);

    createStatusBar();
    createActions();
    createDockWindows();
	createDicWindow();
    createMenus();
    createToolBars();
	createConnections();
	createDicos();
	createDicos(false);
	createCibles();

    setWindowTitle(tr("Collatinus 11"));
	setWindowIcon(QIcon (":/res/collatinus.svg"));

    setUnifiedTitleAndToolBarOnMac(true);

	//setTabPosition(Qt::BottomDockWidgetArea, QTabWidget::North );

	readSettings();
}

/**
 * \fn void MainWindow::afficheLemsDic (bool litt, bool prim)
 * \brief Surcharge. Récupère le contenu de la ligne de saisie du
 *        dock des dictionnaires si prim est à true,
 *        sinon la ligne de saisie de la fenêtre
 *        supplémentaire. Ce contenu est lemmatisé si litt
 *        est à false, puis la ou les pages/entrées
 *        correspondantes sont affichées, soit dans le
 *        dock, soit dans la fenêtre supplémentaire.
 */
void MainWindow::afficheLemsDic (bool litt, bool prim)
{
	QLineEdit *lineEdit;
	if (prim) lineEdit = lineEditDic;
	else lineEdit = lineEditDicW;
    if (lineEdit->text ().isEmpty ())
        return;
	lemsDic.clear();    
    QStringList requete;
    if (!litt)
    {
		MapLem lm = lemmatiseur->lemmatiseM(lineEdit->text(),true);
		requete = lemmatiseur->lemmes (lm);
    }
    if (requete.empty ()) requete << lineEdit->text ();
    requete.removeDuplicates ();
	if (prim) afficheLemsDic(requete, 0);
	else afficheLemsDicW(requete, 0);
    lineEdit->selectAll();
    lineEdit->setFocus();
}

/**
 * \fn void MainWindow::afficheLemsDicLitt()
 * \brief Fonction de relais permettant d'utiliser
 *        la connexion entre une action et la fonction
 *        afficheLemsDic().
 */
void MainWindow::afficheLemsDicLitt()
{
	afficheLemsDic(true);
}

/**
 * \fn void MainWindow::afficheLemsDicW () * \brief Fonction de relais permettant d'utiliser
 *        la connexion entre une action et la fonction
 *        afficheLemsDicW().
 *
 */
void MainWindow::afficheLemsDicW ()
{
	afficheLemsDic (false,false);
}

/**
 * \fn afficheLemsDic(true,false);
 * \brief
 * \brief Fonction de relais permettant d'utiliser
 *        la connexion entre une action et la fonction
 *        afficheLemsDicW(), sans lemmatisation.
 */
void MainWindow::afficheLemsDicLittW ()
{
	afficheLemsDic(true,false);
}

/**
 * \fn void MainWindow::afficheLemsDic(QStringList ll, int no)
 * \brief Affiche la page ou les entrées de
 * dictionnaire correspondant au lemme d'ordre no de la
 * liste ll, et règle le texte des boutons de
 * navigation.
 */
void MainWindow::afficheLemsDic(QStringList ll, int no)
{
	if (textBrowserDic == 0) return;
	lemsDic = ll;
    if (ll.empty () || no < 0 || listeD.courant () == NULL) 
        return;
    textBrowserDic->clear ();
    textBrowserDic->setHtml (listeD.courant()->page (ll, no));
    lineEditDic->setText (ll.at (no));  
    if (listeD.courant ()->estXml ())
    {
        anteButton->setText (listeD.courant()->pgPrec ());
        postButton->setText (listeD.courant()->pgSuiv ());
    }
    else 
    {
        anteButton->setText (tr ("Retro"));
        postButton->setText (tr ("Porro"));
        labelLewis->setText (QString::number (listeD.courant ()->noPageDjvu ()));
    }
	textBrowserDic->moveCursor(QTextCursor::Start);
}

/**
 * \fn void MainWindow::afficheLemsDicW(QStringList ll, int no)
 * \brief comme afficheLemsDic, mais pour le
 * dictionnaire supplémentaire.
 *
 */
void MainWindow::afficheLemsDicW(QStringList ll, int no)
{
	if (textBrowserW == 0) return;
	//lemsDic = ll;
    if (ll.empty () || no < 0 || listeD.courant2 () == NULL) 
        return;
    textBrowserW->clear ();
    textBrowserW->setHtml (listeD.courant2()->page (ll, no));
    lineEditDicW->setText (ll.at (no));  
    if (listeD.courant2()->estXml ())
    {
        anteButtonW->setText (listeD.courant2()->pgPrec ());
        postButtonW->setText (listeD.courant2()->pgSuiv ());
    }
    else 
    {
        anteButtonW->setText (tr ("Retro"));
        postButtonW->setText (tr ("Porro"));
        labelLewisW->setText (QString::number (listeD.courant2()->noPageDjvu ()));
    }
	textBrowserW->moveCursor(QTextCursor::Start);
}

/**
 * \fn void MainWindow::afficheLien (QUrl url)
 * \brief Prend en charge l'affichage des hyperliens de
 *        navigations insérés dans les pages/entrées
 *        des dictionnaires.
 *
 */
void MainWindow::afficheLien (QUrl url)
{
    if (listeD.courant()->estXml())
        return;
    // la ligne de liens en tête de page doit être gardée
    QStringList liens =  listeD.courant()->liens ();
    int no = liens.indexOf(url.toString());
    if (no < 0) no = 0;
    afficheLemsDic(liens, no);
}

/**
 * \fn void MainWindow::afficheLienW (QUrl url)
 * \brief Comme afficheLien, pour le dictionnaire
 * supplémentaire.
 */
void MainWindow::afficheLienW (QUrl url)
{
    if (listeD.courant2()->estXml())
        return;
    // la ligne de liens en tête de page doit être gardée
    QStringList liens =  listeD.courant2()->liens ();
    int no = liens.indexOf(url.toString());
    if (no < 0) no = 0;
    afficheLemsDicW(liens, no);
}

/**
 * \fn void MainWindow::alpha()
 * \brief Force la lemmatisation alphabétique de
 *        tout le texte, quelle que soit l'option alpha
 *        du lemmatiseur.
 */
void MainWindow::alpha()
{
	// pour que l'action provoque le basculement à true
	// de l'option alpha du lemmatiseur, supprimer la
	// première et la dernière ligne.
	bool tmpAlpha = lemmatiseur->optAlpha();
	lemmatiseur->setAlpha(true);
	lemmatiseTxt();
	lemmatiseur->setAlpha(tmpAlpha);
}

/**
 * \fn void MainWindow::apropos ()
 * \brief Affiche les informations essentielles au
 *        sujet de Collatinus 11.
 */
void MainWindow::apropos ()
{
   QMessageBox::about(this, tr("Collatinus 11"), tr (
         "COLLATINVS\nLinguae latinae lemmatizatio \n"
         "Licentia GPL, © Yves Ouvrard, 2009 - 2016 \n"
         "Nonnullas partes operis scripsit Philippe Verkerk\n"
         "Versio "VERSION"\n"
         "Gratias illis habeo :\n"
         "William Whitaker †\n"
         "Jose Luis Redrejo,\n"
         "Georges Khaznadar,\n"
         "Matthias Bussonier,\n"
         "Gérard Jeanneau,\n"
         "Jean-Paul Woitrain,\n"
         "Perseus Digital Library <http://www.perseus.tufts.edu>"));
}

/**
 * \fn void MainWindow::changeGlossarium (QString nomDic)
 * \brief Change le dictionnaire actif du dock
 * dictionnaires.
 *
 */
void MainWindow::changeGlossarium (QString nomDic)
{
		listeD.change_courant(nomDic);
    	if (listeD.courant () == NULL)
        	return;
    	if (listeD.courant ()->estXml ())
        	labelLewis->setText ("↔"); // "\u2194"
    	else
    	{
        	listeD.courant ()->vide_index ();
        	labelLewis->clear ();
    	}
    	if (!lemsDic.empty ())
        	afficheLemsDic (lemsDic, lemsDic.indexOf (lineEditDic->text ()));
    	else if (!lineEditDic->text ().isEmpty())
        	afficheLemsDic (QStringList () << lineEditDic->text ());
}

/**
 * \fn void MainWindow::changeGlossariumW (QString nomDic)
 * \brief Comme ChangeGlossarium, pour le dictionnaire
 *        supplémentaire.
 */
void MainWindow::changeGlossariumW (QString nomDic)
{
	listeD.change_courant2(nomDic);
    if (listeD.courant2() == NULL)
        return;
    if (listeD.courant2()->estXml ())
        labelLewisW->setText ("↔"); // "\u2194"
    else
    {
        listeD.courant2()->vide_index ();
        labelLewisW->clear ();
    }
    if (!lemsDic.empty ())
        afficheLemsDicW(lemsDic, lemsDic.indexOf (lineEditDicW->text ()));
    else if (!lineEditDicW->text ().isEmpty())
        afficheLemsDicW(QStringList () << lineEditDicW->text ());
}

/**
 * \fn void MainWindow::changePageDjvu (int p, bool prim)
 * \brief Change la page d'un dictionnaire au format
 *        djvu, pour le dock dictionnaire si prim est à
 *        true, sinon pour le dictionnaire
 *        supplémentaire.
 */
void MainWindow::changePageDjvu (int p, bool prim)
{
	QTextBrowser *browser;
	QLabel *label;
	if (prim) 
	{
		browser = textBrowserDic;
		label = labelLewis;
	}
	else
	{
		browser = textBrowserW;
		label = labelLewisW;
	}
	browser->clear ();
	if (prim) browser->setHtml (listeD.courant ()->pageDjvu (p));
	else browser->setHtml (listeD.courant2()->pageDjvu (p));
    label->setText (QString::number(p));
	browser->moveCursor(QTextCursor::Start);
}

/**
 * \fn void MainWindow::charger (QString f)
 * \brief Charge le fichier nommé f dans l'éditeur 
 *        de texte latin.
 */
void MainWindow::charger (QString f)
{
    QFile file(f);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox::warning(this, tr("Collatinus"),
                             tr("%1: Lecture impossible,\n%2.")
                             .arg(nfAb)
                             .arg(file.errorString()));
        return;
    }
    QTextStream in(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QString contenu = in.readAll ();
	file.close();
	editLatin->setPlainText (contenu);
    QApplication::restoreOverrideCursor();
}

/**
 * \fn void MainWindow::clicAnte ()
 * \brief Gère le passage à la page précédente.
 */
void MainWindow::clicAnte ()
{
    listeD.courant ()->vide_ligneLiens ();
    if (listeD.courant ()->estXml ())
    {
        afficheLemsDic(QStringList () << anteButton->text ());
    }
    else
    {
        int p = labelLewis->text ().toInt ();
        if (p > 0)
            changePageDjvu (labelLewis->text().toInt()-1);
    }
}

/**
 * \fn void MainWindow::clicAnteW()
 * \brief Comme clicAnte, pour le dictionnaire
 * supplémentaire.
 *
 */
void MainWindow::clicAnteW()
{
    listeD.courant2()->vide_ligneLiens ();
    if (listeD.courant2()->estXml ())
    {
        afficheLemsDicW(QStringList () << anteButton->text ());
    }
    else
    {
        int p = labelLewisW->text ().toInt ();
        if (p > 0)
            changePageDjvu(labelLewisW->text().toInt()-1, false);
    }
}

/**
 * \fn void MainWindow::clicPost ()
 * \brief Gère le passage du dictionnaire à la page
 *        suivante.
 */
void MainWindow::clicPost ()
{
    listeD.courant ()->vide_ligneLiens ();
    if (listeD.courant ()->estXml ())
    { 
        afficheLemsDic(QStringList () << postButton->text ());
    }
    else
    {
        int p = labelLewis->text ().toInt ();
        if (p < 8888)   // ATTENTION, déclarer la dernière page dans les cfg !
            changePageDjvu (labelLewis->text ().toInt ()+1);
    }
}

/**
 * \fn void MainWindow::clicPostW()
 * \brief Comme clicPost, pour le dictionnaire
 *        supplémentaire.
 *
 */
void MainWindow::clicPostW()
{
    listeD.courant2()->vide_ligneLiens ();
    if (listeD.courant2()->estXml ())
    { 
        afficheLemsDicW(QStringList () << postButtonW->text ());
    }
    else
    {
        int p = labelLewisW->text ().toInt ();
        if (p < 8888)   // ATTENTION, déclarer la dernière page dans les cfg !
            changePageDjvu (labelLewisW->text ().toInt ()+1,false);
    }
}

/**
 * \fn void MainWindow::closeEvent(QCloseEvent *event)
 * \brief Enregistre certains paramètres le la session
 *        avant fermeture de l'application.
 */
void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings("Collatinus", "collatinus11");
	settings.beginGroup("fenetre");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
	settings.endGroup();
	settings.beginGroup("fichiers");
	if (!nfAb.isEmpty()) settings.setValue("nfAb", nfAb);
	settings.endGroup();
	settings.beginGroup("options");
	settings.setValue("zoom", editLatin->font().pointSize());
	// options
	settings.setValue("alpha", alphaOptAct->isChecked());
	settings.setValue("html", htmlAct->isChecked());
	settings.setValue("formetxt", formeTAct->isChecked());
	settings.setValue("majpert", majPertAct->isChecked());
	settings.setValue("morpho", morphoAct->isChecked());
    settings.setValue("nonrec", nonRecAct->isChecked());
	settings.setValue("cible", lemmatiseur->cible());
	settings.endGroup();
	settings.beginGroup("dictionnaires");
	settings.setValue("courant", comboGlossaria->currentIndex());
	settings.setValue("wdic", wDic->isVisible());
	settings.setValue("courantW", comboGlossariaW->currentIndex()); 
	settings.setValue("posw", wDic->pos());
	settings.setValue("sizew", wDic->size());
	settings.setValue("sync", syncAct->isChecked());
	settings.endGroup();
	delete wDic;
    QMainWindow::closeEvent(event);
}

void MainWindow::copie()
{
	QClipboard *clipboard = QApplication::clipboard();
	clipboard->clear();
	
	QString texte;
	if (cbTexteLatin->isChecked()) texte.append(editLatin->toPlainText());
	if (cbLemmatisation->isChecked()) texte.append(textEditLem->toHtml());
	if (cbScansion->isChecked()) texte.append(textEditScand->toHtml());
	QMimeData *mime = new QMimeData;
	mime->setHtml(texte);
	clipboard->setMimeData(mime);
}

/**
 * \fn void MainWindow::createActions()
 * \brief Fonction appelée par le créateur. Initialise
 *        toutes les actions utilisées par
 *        l'application.
 */
void MainWindow::createActions()
{
	/*
    undoAct = new QAction(QIcon(":/images/undo.png"), tr("&Undo"), this);
    undoAct->setShortcuts(QKeySequence::Undo);
    undoAct->setStatusTip(tr("Undo the last editing action"));
    connect(undoAct, SIGNAL(triggered()), this, SLOT(undo()));
	// aussi SLOT(redo())
	*/
	alphaAct = new QAction(QIcon(":res/edit-alpha.svg"), tr("Lancer et classer &alphabétiquement"), this);
    aproposAct = new QAction(QIcon(":/res/collatinus.svg"), tr("à &Propos"), this);
	balaiAct = new QAction(QIcon(":res/edit-clear.svg"), tr("&Effacer les résultats"), this);
	copieAct = new QAction(QIcon(":res/copie.svg"), tr("&Copier dans un traitement de textes"), this);
	deZoomAct = new QAction(QIcon(":res/dezoom.svg"), tr("Plus petit"), this);
	findAct = new QAction(QIcon(":res/edit-find.svg"), tr("&Chercher"), this);
	flexAct = new QAction(QIcon(":res/flechir.svg"), tr("&Fléchir"), this);
	lancAct = new QAction(QIcon(":res/gear.svg"), tr("&Lancer"), this);
	nouvAct = new QAction(QIcon(":/res/document-new.svg"), tr("&Nouveau"), this);
	ouvrirAct = new QAction(QIcon(":/res/document-open.svg"), tr("&Ouvrir"), this);
	exportAct = new QAction(QIcon(":res/pdf.svg"), tr("Exporter en pdf"), this);
	printAct = new QAction(QIcon(":res/print.svg"), tr("Im&primer"), this);
    printAct->setShortcuts(QKeySequence::Print);
    quitAct = new QAction(QIcon(":/res/power.svg"), tr("&Quitter"), this);
    quitAct->setStatusTip(tr("Quitter l'application"));
	reFindAct = new QAction(tr("Chercher &encore"), this);
	statAct = new QAction(QIcon(":res/abacus.svg"), tr("S&tatistiques"), this);
	zoomAct = new QAction(QIcon(":res/zoom.svg"), tr("Plus gros"), this);

	// raccourcis
	findAct->setShortcut(QKeySequence::Find);
	reFindAct->setShortcut(QKeySequence(tr("Ctrl+J")));
    quitAct->setShortcut(QKeySequence (tr("Ctrl+Q"))); // QKeySequence::Quit inopérant
	nouvAct->setShortcuts(QKeySequence::New);
	ouvrirAct->setShortcuts(QKeySequence::Open);

	// lemmatisation et options
	// ordre alpha
	alphaOptAct = new QAction(tr("ordre alpha"), this);
	alphaOptAct->setCheckable(true);
	// calepino
	calepAct = new QAction(tr("Calep"), this);
	calepAct->setCheckable(true);
	// formes du texte dans la lemmatisation
	formeTAct = new QAction(tr("avec formes"), this);
	formeTAct->setCheckable(true);
	// lemmatisation en html
	htmlAct = new QAction(tr("format html"), this);
	htmlAct->setCheckable(true);
	// prise en compte des majuscules
	majPertAct = new QAction(tr("majuscules"), this);
	majPertAct->setCheckable(true);
	// analyses morpho dans la lemmatisation
	morphoAct = new QAction(tr("Morpho"), this);
	morphoAct->setCheckable(true);
    // non reconnus en fin de lemmatisation
    nonRecAct = new QAction(tr("grouper échecs"), this);
	nonRecAct->setCheckable(true); 
	// actions pour les dictionnaires
	dicAct = new QAction(QIcon(":/res/dicolem.svg"), tr("Lemmatiser et chercher"), this);
	dicLittAct = new QAction(QIcon(":/res/dicolitt.svg"), tr("Chercher"), this);
	dicActW = new QAction(QIcon(":/res/dicolem.svg"), tr("Lemmatiser et chercher"), this);
	dicLittActW = new QAction(QIcon(":/res/dicolitt.svg"), tr("Chercher"), this);
	// synchronisation des deux dictionnaires
	syncAct = new QAction(tr("sync+"), this);
	syncAct->setCheckable(true); // synchronisation des deux fenêtres
	syncDWAct = new QAction(tr("sync->"), this);
	syncWDAct = new QAction(tr("<-sync"), this);
	visibleWAct = new QAction(tr("Dictionnaire +"), this);
	visibleWAct->setCheckable(true);
}

/**
 * \fn void MainWindow::createCibles()
 * \brief Initialise toutes les actions liées aux
 *        fonctions de traduction.
 */
void MainWindow::createCibles()
{
    grCibles = new QActionGroup (lexMenu);
    foreach (QString cle, lemmatiseur->cibles().keys ()) 
    {
        QAction * action = new QAction (grCibles);
        action->setText (lemmatiseur->cibles()[cle]);
        action->setCheckable (true);
        lexMenu->addAction(action);
        connect(action, SIGNAL(triggered ()), this, SLOT(setCible ()));
    }
}

/**
 * \fn void MainWindow::createConnections()
 * \brief Initialisation des connections qui lancent
 *        toutes les actions des menus et des barres d'outils.
 */
void MainWindow::createConnections()
{
	// synchroniser zoom et dezoom
	connect (zoomAct, SIGNAL(triggered()), editLatin, SLOT(zoomIn()));
	connect (zoomAct, SIGNAL(triggered()), textBrowserDic, SLOT(zoomIn()));
	connect (zoomAct, SIGNAL(triggered()), textBrowserW, SLOT(zoomIn()));
	connect (zoomAct, SIGNAL(triggered()), textBrowserFlex, SLOT(zoomIn()));
	connect (zoomAct, SIGNAL(triggered()), textEditLem, SLOT(zoomIn()));
	connect (zoomAct, SIGNAL(triggered()), textEditScand, SLOT(zoomIn()));

	connect (deZoomAct, SIGNAL(triggered()), editLatin, SLOT(zoomOut()));
	connect (deZoomAct, SIGNAL(triggered()), textBrowserDic, SLOT(zoomOut()));
	connect (deZoomAct, SIGNAL(triggered()), textBrowserW, SLOT(zoomOut()));
	connect (deZoomAct, SIGNAL(triggered()), textBrowserFlex, SLOT(zoomOut()));
	connect (deZoomAct, SIGNAL(triggered()), textEditLem, SLOT(zoomOut()));
	connect (deZoomAct, SIGNAL(triggered()), textEditScand, SLOT(zoomOut()));

	// connexions des lignes de saisie
	connect (lineEditLem, SIGNAL(returnPressed()), this, SLOT(lemmatiseLigne()));
	connect (lineEditFlex, SIGNAL(returnPressed()), this, SLOT(flechisLigne()));

	// options et actions du lemmatiseur
	connect(alphaOptAct, SIGNAL(toggled(bool)), lemmatiseur, SLOT(setAlpha(bool)));
	connect(formeTAct, SIGNAL(toggled(bool)), lemmatiseur, SLOT(setFormeT(bool)));
	connect(htmlAct, SIGNAL(toggled(bool)), lemmatiseur, SLOT(setHtml(bool)));
	connect(majPertAct, SIGNAL(toggled(bool)), lemmatiseur, SLOT(setMajPert(bool)));
	connect(morphoAct, SIGNAL(toggled(bool)), lemmatiseur, SLOT(setMorpho(bool)));
    connect(nonRecAct, SIGNAL(toggled(bool)), lemmatiseur, SLOT(setNonRec(bool)));

	// actions des dictionnaires
    connect(anteButton, SIGNAL(clicked ()), this, SLOT(clicAnte ()));
    connect(comboGlossaria, SIGNAL(currentIndexChanged(QString)),this,SLOT(changeGlossarium(QString)));
	connect(dicAct, SIGNAL(triggered()), this, SLOT(afficheLemsDic()));
	connect(dicLittAct, SIGNAL(triggered()), this, SLOT(afficheLemsDicLitt()));
    connect(lineEditDic, SIGNAL(returnPressed ()), this, SLOT(afficheLemsDic()));
    connect(postButton, SIGNAL(clicked ()), this, SLOT(clicPost()));
	connect(syncDWAct, SIGNAL(triggered()), this, SLOT(syncDW()));
    connect(textBrowserDic, SIGNAL(anchorClicked(QUrl)), this, SLOT(afficheLien(QUrl)));

    connect(anteButtonW, SIGNAL(clicked ()), this, SLOT(clicAnteW()));
    connect(comboGlossariaW, SIGNAL(currentIndexChanged(QString)),this,SLOT(changeGlossariumW(QString)));
	connect(dicActW, SIGNAL(triggered()), this, SLOT(afficheLemsDicW()));
	connect(dicLittActW, SIGNAL(triggered()), this, SLOT(afficheLemsDicLittW()));
    connect(lineEditDicW, SIGNAL(returnPressed ()), this, SLOT(afficheLemsDicW()));
    connect(postButtonW, SIGNAL(clicked ()), this, SLOT(clicPostW()));
	connect(syncWDAct, SIGNAL(triggered()), this, SLOT(syncWD()));
    connect(textBrowserW, SIGNAL(anchorClicked(QUrl)), this, SLOT(afficheLienW(QUrl)));
	connect(visibleWAct, SIGNAL(toggled(bool)), this, SLOT(montreWDic(bool)));

	// autres actions
	connect(alphaAct, SIGNAL(triggered()), this, SLOT(alpha()));
    connect(aproposAct, SIGNAL(triggered()), this, SLOT(apropos()));
	connect(balaiAct, SIGNAL(triggered()), this, SLOT(effaceRes()));
	connect(copieAct, SIGNAL(triggered()), this, SLOT(dialogueCopie()));
	connect(exportAct, SIGNAL(triggered()), this, SLOT(exportPdf()));
	connect(findAct, SIGNAL(triggered()), this, SLOT(recherche()));
	connect(reFindAct, SIGNAL(triggered()), this, SLOT(rechercheBis()));
	connect(lancAct, SIGNAL(triggered()), this, SLOT(lancer()));
	connect(ouvrirAct, SIGNAL(triggered()), this, SLOT(ouvrir()));
	connect(printAct, SIGNAL(triggered()), this, SLOT(imprimer()));
	connect(quitAct, SIGNAL(triggered()), this, SLOT(close()));
	connect(statAct, SIGNAL(triggered()), this, SLOT(stat()));
}

/**
 * \fn void MainWindow::createDicos(bool prim)
 * \brief Chargement des index et des fichiers de
 *        configuration des dictionnaires.
 */
void MainWindow::createDicos(bool prim)
{
	QComboBox *combo = 0;
	if (prim) combo = comboGlossaria;
	else combo = comboGlossariaW;
    combo->clear ();
    QDir chDicos (qApp->applicationDirPath()+"/data/dicos");
    QStringList lcfg = chDicos.entryList (QStringList () << "*.cfg");
    ldic.clear();
    foreach (QString fcfg, lcfg)
    {
        Dictionnaire * d = new Dictionnaire (fcfg);
        listeD.ajoute (d);
        ldic << d->nom ();
    }
    combo->insertItems (0, ldic);
}

/**
 * \fn void MainWindow::createMenus()
 * \brief Initialisation des menus à partir des actions définies
 *        dans MainWindow::createActions().
 *
 */
void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&Fichier"));
	fileMenu->addAction (ouvrirAct);
    fileMenu->addSeparator();
	fileMenu->addAction(copieAct);
	fileMenu->addAction(exportAct);
	fileMenu->addAction(printAct);
    fileMenu->addSeparator();
    fileMenu->addAction(quitAct);

    editMenu = menuBar()->addMenu(tr("&Edition"));
	editMenu->addAction(findAct);
	editMenu->addAction(reFindAct);
    //editMenu->addAction(undoAct);

    viewMenu = menuBar()->addMenu(tr("&Vue"));
	viewMenu->addAction (balaiAct);
	viewMenu->addAction(zoomAct);
	viewMenu->addAction(deZoomAct);
	viewMenu->addAction(visibleWAct);

	lexMenu = menuBar()->addMenu(tr("&Lexique"));
	lexMenu->addAction (lancAct);
	lexMenu->addAction (alphaAct);
	lexMenu->addAction (statAct);

	optMenu = menuBar()->addMenu(tr("&Options"));
	optMenu->addAction(alphaOptAct);
	optMenu->addAction(formeTAct);
	optMenu->addAction(htmlAct);
	optMenu->addAction(majPertAct);
	optMenu->addAction(morphoAct);
    optMenu->addAction(nonRecAct);

    helpMenu = menuBar()->addMenu(tr("&Aide"));
    helpMenu->addAction(aproposAct);
}

/**
 * \fn void MainWindow::createToolBars()
 * \brief Initialisation de la barre d'outils à partir
 *        des actions.
 */
void MainWindow::createToolBars()
{
    toolBar = new QToolBar(this);
	toolBar->setObjectName("toolbar");
    addToolBar(Qt::TopToolBarArea, toolBar);

	toolBar->addAction(nouvAct);
	toolBar->addAction(ouvrirAct);
	toolBar->addAction(copieAct);
	toolBar->addAction(zoomAct);
	toolBar->addAction(deZoomAct);
	toolBar->addAction(findAct);
    toolBar->addSeparator();
	toolBar->addAction(lancAct);
	toolBar->addAction(alphaAct);
	toolBar->addAction(statAct);
	toolBar->addAction(calepAct);
	toolBar->addAction(visibleWAct);
	toolBar->addAction(balaiAct);
    toolBar->addSeparator();
    toolBar->addAction(quitAct);
}

/**
 * \fn void MainWindow::createStatusBar()
 * \brief Initialisation de la barre d'état. À compléter.
 *
 */
void MainWindow::createStatusBar()
{
}

/**
 * \fn void MainWindow::createDockWindows()
 * \brief Initialisation des différents docks.
 *
 */
void MainWindow::createDockWindows()
{
    dockLem = new QDockWidget(tr("Lexique et morphologie"), this);
	dockLem->setObjectName ("docklem");
    dockLem->setAllowedAreas(Qt::BottomDockWidgetArea|Qt::RightDockWidgetArea);
    dockLem->setFloating(false);
    dockLem->setFeatures(QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable);
    dockWidgetLem = new QWidget (dockLem);
	dockWidgetLem->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QVBoxLayout *vLayoutLem = new QVBoxLayout (dockWidgetLem);
    QHBoxLayout *hLayoutLem = new QHBoxLayout ();
    lineEditLem = new QLineEdit (dockWidgetLem);
	// boutons d'options
	QToolButton *tbCalep = new QToolButton(this);
	tbCalep->setDefaultAction(calepAct);
	QToolButton *tbMorpho = new QToolButton (this);
	tbMorpho->setDefaultAction(morphoAct);
	QToolButton *tbAlpha = new QToolButton (this);
	tbAlpha->setDefaultAction(alphaOptAct);
	QToolButton *tbFormeT = new QToolButton (this);
	tbFormeT->setDefaultAction(formeTAct);
	QToolButton *tbHtml = new QToolButton (this);
	tbHtml->setDefaultAction(htmlAct);
	QToolButton *tbMajPert = new QToolButton (this);
	tbMajPert->setDefaultAction(majPertAct);
    QToolButton *tbNonRec = new QToolButton (this);
	tbNonRec->setDefaultAction(nonRecAct); 
    QSpacerItem *hSpacerLem = new QSpacerItem (40,20);
	hLayoutLem->addWidget (lineEditLem);
	hLayoutLem->addWidget (tbCalep);
	hLayoutLem->addWidget (tbMorpho);
	hLayoutLem->addWidget (tbAlpha);
	hLayoutLem->addWidget (tbFormeT);
	hLayoutLem->addWidget (tbHtml);
	hLayoutLem->addWidget (tbMajPert);
	hLayoutLem->addWidget (tbNonRec);
	hLayoutLem->addItem (hSpacerLem);
    textEditLem = new QTextEdit(dockWidgetLem);
	vLayoutLem->addLayout (hLayoutLem);
	vLayoutLem->addWidget (textEditLem);
	dockLem->setWidget (dockWidgetLem);

	dockDic = new QDockWidget(tr("Dictionnaires"), this);
	dockDic->setObjectName("dockdic");
    dockDic->setFloating(false);
    dockDic->setFeatures(QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable);
    dockDic->setAllowedAreas(Qt::BottomDockWidgetArea);
    dockWidgetDic = new QWidget (dockDic);
    QVBoxLayout *vLayoutDic = new QVBoxLayout (dockWidgetDic);
    QHBoxLayout *hLayoutDic = new QHBoxLayout ();
    lineEditDic = new QLineEdit (dockWidgetDic);
	// Lemmatisation + recherche
	QToolButton *tbDic = new QToolButton (this);
	tbDic->setDefaultAction(dicAct);
	// recherche sans lemmatisation
	QToolButton *tbDicLitt = new QToolButton (this);
	tbDicLitt->setDefaultAction(dicLittAct);
	// dictionnaire
	QToolButton *tbSync = new QToolButton(this);
	tbSync->setDefaultAction(syncAct);
	QToolButton *tbDicW = new QToolButton(this);
	tbDicW->setDefaultAction(visibleWAct);
	QToolButton *tbSyncDW = new QToolButton(this);
	tbSyncDW->setDefaultAction(syncDWAct);
	// choix des dictionnaires
	comboGlossaria = new QComboBox (this);
	anteButton = new QPushButton (this);
	labelLewis = new QLabel (this);
	postButton = new QPushButton (this);
    QSpacerItem *hSpacerDic = new QSpacerItem (40, 20);
	//, QSizePolicy::Expanding, QSizePolicy::Minimum);
	hLayoutDic->addWidget(lineEditDic);
	hLayoutDic->addWidget(tbDic);
	hLayoutDic->addWidget(tbDicLitt);
	hLayoutDic->addWidget(comboGlossaria);
	hLayoutDic->addWidget(anteButton);
	hLayoutDic->addWidget(labelLewis);
	hLayoutDic->addWidget(postButton);
	hLayoutDic->addItem (hSpacerDic);
	hLayoutDic->addWidget(tbSync);
	hLayoutDic->addWidget (tbDicW);
	hLayoutDic->addWidget (tbSyncDW);
    textBrowserDic = new QTextBrowser(dockWidgetDic);
    textBrowserDic->setOpenExternalLinks(true);
	vLayoutDic->addLayout (hLayoutDic);
	vLayoutDic->addWidget (textBrowserDic);
	dockDic->setWidget (dockWidgetDic);

	dockScand = new QDockWidget(tr("Scansion"), this);
	dockScand->setObjectName("dockscand");
    dockScand->setFloating(false);
    dockScand->setFeatures(QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable);
    dockScand->setAllowedAreas(Qt::BottomDockWidgetArea|Qt::RightDockWidgetArea);
    dockWidgetScand = new QWidget (dockScand);
    QVBoxLayout *vLayoutScand = new QVBoxLayout (dockWidgetScand);
    QHBoxLayout *hLayoutScand = new QHBoxLayout ();
    QLineEdit *lineEditScand = new QLineEdit (dockWidgetScand);
    QSpacerItem *hSpacerScand = new QSpacerItem (40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
	hLayoutScand->addWidget (lineEditScand);
	hLayoutScand->addItem (hSpacerScand);
    textEditScand = new QTextEdit(dockWidgetScand);
	vLayoutScand->addLayout (hLayoutScand);
	vLayoutScand->addWidget (textEditScand);
	dockScand->setWidget (dockWidgetScand);

    dockFlex = new QDockWidget(tr("Flexion"), this);
	dockFlex->setObjectName("dockflex");
    dockFlex->setFloating(false);
    dockFlex->setFeatures(QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable);
    dockFlex->setAllowedAreas(Qt::BottomDockWidgetArea);
    dockWidgetFlex = new QWidget (dockFlex);
    QVBoxLayout *vLayoutFlex = new QVBoxLayout (dockWidgetFlex);
    QHBoxLayout *hLayoutFlex = new QHBoxLayout ();
    lineEditFlex = new QLineEdit (dockWidgetFlex);
    QSpacerItem *hSpacerFlex = new QSpacerItem (40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
	hLayoutFlex->addWidget (lineEditFlex);
	hLayoutFlex->addItem (hSpacerFlex);
    textBrowserFlex = new QTextBrowser(dockWidgetFlex);
	textBrowserFlex->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);
	vLayoutFlex->addLayout (hLayoutFlex);
	vLayoutFlex->addWidget (textBrowserFlex);
	dockFlex->setWidget (dockWidgetFlex);

	addDockWidget(Qt::BottomDockWidgetArea, dockLem);
	addDockWidget(Qt::BottomDockWidgetArea, dockDic);
	addDockWidget(Qt::BottomDockWidgetArea, dockScand);
	addDockWidget(Qt::BottomDockWidgetArea, dockFlex);

	tabifyDockWidget (dockLem, dockDic);
    tabifyDockWidget (dockDic, dockScand);
    tabifyDockWidget (dockScand, dockFlex);

	setTabPosition(Qt::BottomDockWidgetArea, QTabWidget::North );
	dockLem->raise();
}

/**
 * \fn void MainWindow::createDicWindow()
 * \brief Initialisation du widget de dictionnaire
 *        supplémentaire.
 */
void MainWindow::createDicWindow()
{
	wDic = new QWidget ();
	wDic->setObjectName("wDic");
	QVBoxLayout *vLayout = new QVBoxLayout(wDic);
	QHBoxLayout *hLayout = new QHBoxLayout();
	lineEditDicW = new QLineEdit(wDic);
	// Lemmatisation + recherche
	QToolButton *tbDic = new QToolButton (this);
	tbDic->setDefaultAction(dicActW);
	// recherche sans lemmatisation
	QToolButton *tbDicLittW = new QToolButton (this);
	tbDicLittW->setDefaultAction(dicLittActW);
	comboGlossariaW = new QComboBox (this);
	anteButtonW = new QPushButton (this);
	labelLewisW = new QLabel (this);
	postButtonW = new QPushButton (this);
    QSpacerItem *hSpacerDic = new QSpacerItem (40, 20);
	//, QSizePolicy::Expanding, QSizePolicy::Minimum);
	QToolButton *tbSyncWD = new QToolButton(this);
	tbSyncWD->setDefaultAction(syncWDAct);
	hLayout->addWidget(lineEditDicW);
	hLayout->addWidget(tbDic);
	hLayout->addWidget(tbDicLittW);
	hLayout->addWidget(comboGlossariaW);
	hLayout->addWidget(anteButtonW);
	hLayout->addWidget(labelLewisW);
	hLayout->addWidget(postButtonW);
	hLayout->addItem (hSpacerDic);
	hLayout->addWidget(tbSyncWD);
    textBrowserW = new QTextBrowser(wDic);
    textBrowserW->setOpenExternalLinks(true);
	vLayout->addLayout (hLayout);
	vLayout->addWidget (textBrowserW);
}

/**
 * \fn void MainWindow::dialogueCopie()
 * \brief Ouvre une boite de dialogue qui permet de 
 *        sélectionner les parties à copier, et 
 *        les place dans le presse-papier du système
 */
void MainWindow::dialogueCopie()
{
    QLabel *icon = new QLabel;
    icon->setPixmap(QPixmap(":/res/collatinus.ico"));
    QLabel *text = new QLabel;
    text->setWordWrap(true);
    text->setText("<p>Pour récupérer et modifier votre travail, la meilleure manière est "
				  "d'ouvrir le traitement de textes de votre choix, puis de sélectionner "
				  "ci-dessous ce que vous voulez utiliser. Cliquez ensuite sur le bouton "
				  "«Appliquer». Pour terminer, revenez dans votre traitement de texte, "
				  "et copiez votre sélection avec le raccourci <b>Ctrl-P</b>, ou l'option "
				  "de menu <b>Édition/Coller</b>.");

	cbTexteLatin    = new QCheckBox (tr("Texte latin"));
	cbLemmatisation = new QCheckBox (tr("Lemmatisation"));
	cbScansion      = new QCheckBox (tr("Scansion"));

    QPushButton *appliButton   = new QPushButton("Appliquer");
    QPushButton *cloreButton   = new QPushButton("Fermer");

    QVBoxLayout *topLayout     = new QVBoxLayout;
    topLayout->setMargin(10);
    topLayout->setSpacing(10);
    topLayout->addWidget(icon);
    topLayout->addWidget(text);

    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->addStretch();
    bottomLayout->addWidget(appliButton);
    bottomLayout->addWidget(cloreButton);
    bottomLayout->addStretch();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(topLayout);
	topLayout->addWidget(cbTexteLatin);
	topLayout->addWidget(cbLemmatisation);
	topLayout->addWidget(cbScansion);
    mainLayout->addLayout(bottomLayout);

	QDialog dCopie(this);
    dCopie.setModal(true);
    dCopie.setWindowTitle(tr("Récupérer son travail"));
    dCopie.setLayout(mainLayout);

	connect(appliButton, SIGNAL(clicked()), this, SLOT(copie()));
    connect(cloreButton, SIGNAL(clicked()), &dCopie, SLOT(close()));
    dCopie.exec();
}

/**
 * \fn bool MainWindow::dockVisible (QDockWidget *d)
 * \brief renvoie true si le dock d est visible.
 *
 */
bool MainWindow::dockVisible (QDockWidget *d)
{
	return !d->visibleRegion().isEmpty();
}

/**
 * \fn void MainWindow::effaceRes()
 * \brief Efface le contenu des docs visibles.
 */
void MainWindow::effaceRes()
{
	if (dockVisible(dockLem)) textEditLem->clear();
	if (dockVisible(dockFlex)) textBrowserFlex->clear();
	if (dockVisible(dockScand)) textEditScand->clear();
}

/**
 * \fn
 * \brief
 *
 */
void MainWindow::exportPdf()
{
#ifndef QT_NO_PRINTER
    QString nf = QFileDialog::getSaveFileName(this, "Export PDF",
                                                    QString(), "*.pdf");
    if (!nf.isEmpty()) {
        if (QFileInfo(nf).suffix().isEmpty())
            nf.append(".pdf");
        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(nf);
		QTextEdit *tmpTE = new QTextEdit();
		tmpTE->setHtml (editLatin->toHtml());
		tmpTE->append(textEditLem->toHtml());
		tmpTE->document()->print(&printer);
		delete tmpTE;
    }
#endif
}

/**
 * \fn void MainWindow::imprimer()
 * \brief Lance le dialogue d'impression pour la lemmatisation.
 */
void MainWindow::imprimer()
{
#if !defined(QT_NO_PRINTER) && !defined(QT_NO_PRINTDIALOG)
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog *dlg = new QPrintDialog(&printer, this);
    if (textEditLem->textCursor().hasSelection())
        dlg->addEnabledOption(QAbstractPrintDialog::PrintSelection);
    dlg->setWindowTitle(tr("Imprimer le texte et le lexique"));
    if (dlg->exec() == QDialog::Accepted)
	{
		QTextEdit *tmpTE = new QTextEdit();
		tmpTE->setHtml(editLatin->toHtml());
		tmpTE->append(textEditLem->toHtml());
        tmpTE->print(&printer);
		delete tmpTE;
	}
    delete dlg;
#endif
}

/**
 * \fn void MainWindow::flechisLigne()
 * \brief Provoque l'affichage des lemmes pouvant donner
 *        la forme affichée dans la ligne de saisie du dock
 *        Flexion.
 */
void MainWindow::flechisLigne()
{
	MapLem ml = lemmatiseur->lemmatiseM (lineEditFlex->text());
	if (!ml.empty())
	{
		textBrowserFlex->clear();
		foreach (Lemme *l, ml.keys())
			textBrowserFlex->append (flechisseur->tableau(l));
	}
}

/**
 * \fn bool MainWindow::html()
 * \brief Renvoie vrai si l'option html du lemmatiseur
 *        est armée.
 */
bool MainWindow::html()
{
    return htmlAct->isChecked();
}

/**
 * \fn void MainWindow::lancer()
 * \brief Lance la lemmatisation et la scansion si
 *        les docks correspondants sont visibles.
 */
void MainWindow::lancer()
{
	if (dockVisible(dockLem)) lemmatiseTxt();
	if (dockVisible(dockScand)) scandeTxt();
}

/**
 * \fn void MainWindow::lemmatiseLigne()
 * \brief Lance la lemmatisation des formes *
 *        présentes dans la ligne de saisie du dock
 *        lemmatisation.
 */
void MainWindow::lemmatiseLigne()
{
	textEditLem->append(lemmatiseur->lemmatiseT (lineEditLem->text()));
}

/**
 * \fn void MainWindow::lemmatiseTxt()
 * \brief Lance la lemmatisation de la totalité du
 *        texte contenu dans l'éditeur editLatin (partie supérieure
 *        de la fenêtre de l'application).
 */
void MainWindow::lemmatiseTxt()
{
	// si la tâche dure trop longtemps :
	// setUpdatesEnabled(false);
    if (html())
		textEditLem->setHtml(lemmatiseur->lemmatiseT(editLatin->toPlainText()));
    else
        textEditLem->setPlainText(lemmatiseur->lemmatiseT(editLatin->toPlainText()));
	// setUpdatesEnabled(true);
}

/**
 * \fn void MainWindow::montreWDic(bool visible)
 * \brief Rend visible le dictionnaire supplémentaire,
 *        et met à jour son contenu.
 */
void MainWindow::montreWDic(bool visible)
{
	wDic->move(x()+width() + 80, y());
	wDic->setVisible(visible);
	lineEditDicW->setText (lineEditDic->text());
    afficheLemsDicW();
	lineEditDicW->clearFocus();
}

/**
 * \fn void MainWindow::ouvrir()
 * \brief Affiche le dialogue d'ouverture de fichier.
 */
void MainWindow::ouvrir()
{
    if (precaution ()) return;
    nfAb = QFileDialog::getOpenFileName(this, "Collatinus - Ouvrir un fichier", repertoire);
    if (nfAb.isEmpty()) return;
	charger (nfAb);
	nfAd = nfAb;
	nfAd.prepend ("coll-");
}

/**
 * \fn bool MainWindow::precaution()
 * \brief Dialogue de précaution avant l'effacement du texte latin.
 */
bool MainWindow::precaution()
{
	return false;
}

/**
 * \fn void MainWindow::readSettings()
 * \brief Appelée à l'initialisation de l'application,
 *        pour retrouver les paramètres importants de
 *        la dernière session.
 */
void MainWindow::readSettings()
{
    QSettings settings("Collatinus", "collatinus11");
	// état de la fenêtre
	settings.beginGroup("fenetre");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
	settings.endGroup();
	// dernier fichier chargé
	settings.beginGroup("fichiers");
	nfAb = settings.value ("nfAb").toString();
	if (!nfAb.isEmpty())
	{
		charger (nfAb);
		nfAd = nfAb;
		nfAd.prepend ("coll-");
	}
	settings.endGroup();
	settings.beginGroup("options");
	// police
	font.setPointSize (settings.value("zoom").toInt());
	editLatin->setFont(font);
	textEditLem->setFont(font);
	textBrowserDic->setFont(font);
	textBrowserW->setFont(font);
	textEditScand->setFont(font);
	textBrowserFlex->setFont(font);
	// options de lemmatisation
	alphaOptAct->setChecked (settings.value("alpha").toBool());
	formeTAct->setChecked (settings.value("formetxt").toBool());
	htmlAct->setChecked (settings.value("html").toBool());
	majPertAct->setChecked (settings.value("majpert").toBool());
	morphoAct->setChecked (settings.value("morpho").toBool());
    nonRecAct->setChecked (settings.value("nonrec").toBool());
	QString l = settings.value("cible").toString();
	lemmatiseur->setCible (l);
    foreach (QAction * action, grCibles->actions ())
        if (action->text () == lemmatiseur->cibles()[l])
            action->setChecked (true);
	settings.endGroup();
	// options appliquées au lemmatiseur
    lemmatiseur->setAlpha (alphaOptAct->isChecked());
    lemmatiseur->setFormeT (formeTAct->isChecked());
    lemmatiseur->setHtml (htmlAct->isChecked());
    lemmatiseur->setMajPert (majPertAct->isChecked());
    lemmatiseur->setMorpho(morphoAct->isChecked());
	settings.beginGroup("dictionnaires");
	comboGlossaria->setCurrentIndex(settings.value("courant").toInt());
	wDic->move(settings.value("posw").toPoint());
	wDic->resize(settings.value("sizew").toSize());
	wDic->setVisible(settings.value("wdic").toBool());
	comboGlossariaW->setCurrentIndex(settings.value("courantW").toInt());
	syncAct->setChecked(settings.value("sync").toBool());
	settings.endGroup();
}

/**
 * \fn void MainWindow::recherche()
 * \brief Recherche dans l'éditeur de texte latin.
 */
void MainWindow::recherche()
{
    bool ok;
    rech = QInputDialog::getText(this, tr("Recherche"),
                                         tr("Chercher :"), QLineEdit::Normal,
                                         rech, &ok);
    if (ok && !rech.isEmpty())
    {
        if (!editLatin->find (rech))
        {
            rech  = QInputDialog::getText(this, tr("Chercher"),
                                                 tr("Retour au début ?"), QLineEdit::Normal,
                                                 rech, &ok);
            if (ok && !rech.isEmpty())
            {
                // Retourner au debut
                editLatin->moveCursor(QTextCursor::Start);
                // Chercher à nouveau
                editLatin->find(rech);
            }
         }
     }
}

/**
 * \fn void MainWindow::rechercheBis()
 * \brief Suite de la recherche.
 *
 */
void MainWindow::rechercheBis()
{
    if (rech.isEmpty())
        return;
    bool ok = editLatin->find(rech);
	if (!ok)
	{
		QTextCursor tc = editLatin->textCursor();
    	editLatin->moveCursor(QTextCursor::Start);
		ok = editLatin->find(rech);
		if (!ok) editLatin->setTextCursor(tc);
	}
}

/**
 * \fn void MainWindow::scandeTxt()
 * \brief Lance la scansion du texte latin, et affiche le
 *        résultat dans le dock scansion.
 */
void MainWindow::scandeTxt()
{
	textEditScand->setHtml(lemmatiseur->scandeTxt (editLatin->toPlainText(), false));
}

/**
 * \fn void MainWindow::setCible()
 * \brief Coordonne la langue cible cochée dans le menu
 *        et la langue cible du lemmatiseur.
 */
void MainWindow::setCible()
{
    QAction * action = grCibles->checkedAction ();
    foreach (QString cle, lemmatiseur->cibles().keys())
    {
        if (lemmatiseur->cibles()[cle] == action->text ())
        {
            lemmatiseur->setCible(cle);
            break;
        }
    }
}

/**
 * \fn void MainWindow::stat()
 * \brief Affiche les statistiques de lemmatisation et
 *        de scansion si le dock correspondant est visible.
 */
void MainWindow::stat()
{
	if (dockVisible(dockLem))
	{
		textEditLem->setHtml(lemmatiseur->frequences(editLatin->toPlainText()).join(""));
	}
	if (dockVisible(dockScand))
		textEditScand->setHtml(lemmatiseur->scandeTxt (editLatin->toPlainText(), true));
}

/**
 * \fn void MainWindow::syncDW()
 * \brief effectue dans le dictionnaire supplémentaire
 *        la même recherche que celle qui a été faite dans le
 *        principal.
 */
void MainWindow::syncDW()
{
	if (wDic->isVisible())
	{
		lineEditDicW->setText(lineEditDic->text());
		afficheLemsDicW ();
	}
	else montreWDic(true);
}
	
/**
 * \fn void MainWindow::syncWD()
 * \brief effectue dans le dictionnaire principal
 *        la même recherche que celle qui a été faite dans le
 *        supplémentaire.
 */
void MainWindow::syncWD()
{
	lineEditDic->setText(lineEditDicW->text());
	afficheLemsDic();
}
