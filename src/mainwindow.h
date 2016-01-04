/*            mainwindow.h
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H 
#include <QMainWindow>
#include <QtWidgets>

#include "flexion.h"
#include "lemmatiseur.h"
#include "lewis.h"

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QTextBrowser;
class QTextEdit;
QT_END_NAMESPACE

class MainWindow;

class EditLatin: public QTextEdit
{
	Q_OBJECT

	private:
		MainWindow *mainwindow;
	protected:
		void mouseReleaseEvent (QMouseEvent *e);
	public:
		EditLatin (QWidget *parent);
		bool event (QEvent *event);
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
	// docks
	QDockWidget      *dockLem;
	QDockWidget      *dockDic;
	QDockWidget      *dockScand;
	QDockWidget      *dockFlex;
	// et second dictionnaire
	QWidget          *wDic;
	// cœur
	Lemmat           *lemmatiseur;
	Flexion          *flechisseur;
	// widgets d'édition et d'affichage
	EditLatin        *editLatin;
    QTextEdit        *textEditLem;
    QTextEdit        *textEditScand;
	QTextBrowser     *textBrowserDic;
	QTextBrowser     *textBrowserW;
    QTextBrowser     *textBrowserFlex;
	QLineEdit        *lineEditLem;
	QLineEdit        *lineEditDic;
	QLineEdit        *lineEditDicW;
	QLineEdit        *lineEditFlex;
	// cases à cocher pour la copie
	QCheckBox *cbTexteLatin;
	QCheckBox *cbLemmatisation;
	QCheckBox *cbScansion;
	// contrôle des options
	QCheckBox        *cbAlpha;
	QCheckBox        *cbHtml;
	QCheckBox        *cbMajPert;
	QCheckBox        *cbMorpho;
    bool              html();
	QAction          *syncAct;
	QAction          *calepAct;
	// gr() de la dernière lemmatisation
	QStringList       lemsDic;

private slots: 
	void afficheLemsDic (bool litt=false,bool prim=true); // ligne de saisie
	void afficheLemsDicLitt ();            // relais pour le précédent
	void afficheLemsDicW (); // ligne de saisie
	void afficheLemsDicLittW ();            // relais pour le précédent
	void afficheLien(QUrl url);
	void afficheLienW(QUrl url);
	void alpha();
    void apropos();
	void changeGlossarium (QString nomDic);
	void changeGlossariumW (QString nomDic);
	void changePageDjvu(int p, bool prim=true);
	void clicAnte();
	void clicAnteW();
	void clicPost();
	void clicPostW();
	void closeEvent(QCloseEvent *event);
	void copie();
	void dialogueCopie();
	void effaceRes();
	void exportPdf();
	void flechisLigne();
	void imprimer();
	void lancer();
	void lemmatiseLigne();
    void lemmatiseTxt();
	void montreWDic(bool visible);
	void ouvrir();
	void readSettings();
	void recherche();
	void rechercheBis();
	void scandeTxt();
	void setCible();
	void stat();
	void syncDW();
	void syncWD();

public slots:
	void afficheLemsDic(QStringList ll, int no=0);
	void afficheLemsDicW(QStringList ll, int no=0);

private:
    void createActions();
	void createCibles(); // menu des langues cibles
	void createConnections();
	void createDicos(bool prim=true);
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void createDockWindows();
	void createDicWindow(); // second dictionnaire

    QMenu        *fileMenu;
    QMenu        *editMenu;
    QMenu        *viewMenu;
	QMenu        *lexMenu;
	QMenu        *optMenu;
	QMenu        *helpMenu;

    QToolBar     *toolBar;

	// bascules de lemmatisation
	QAction      *alphaAct;
	QAction      *alphaOptAct;
	QAction      *formeTAct;
	QAction      *htmlAct;
	QAction      *majPertAct;
	QAction      *morphoAct;
    QAction      *nonRecAct;
	// actions et groupes d'actions
	QAction      *aproposAct;
	QAction      *balaiAct;
	QAction      *copieAct;
	QAction      *deZoomAct;
	QAction      *dicAct;
	QAction      *dicActW;
	QAction      *dicLittAct;
	QAction      *dicLittActW;
	QAction      *visibleWAct;
	QAction      *exportAct;
	QAction      *findAct;
	QAction      *flexAct;
	QAction      *lancAct;
	QAction      *nouvAct;
	QAction      *ouvrirAct;
	QAction      *printAct;
	QAction      *quitAct;
	QAction      *reFindAct;
	QAction      *statAct;
	QAction      *syncDWAct;
	QAction      *syncWDAct;
	QAction      *zoomAct;
	QActionGroup *grCibles;
	// QAction *undoAct;
	// widgets, variables et fonctions du dock dictionnaires
	QComboBox   *comboGlossaria;
	QPushButton *anteButton;
	QLabel	    *labelLewis;
	QPushButton *postButton;
	ListeDic     listeD;
	QStringList  ldic;
    // les mêmes, pour le widget dictionnaires
	QComboBox   *comboGlossariaW;
	QPushButton *anteButtonW;
	QLabel      *labelLewisW;
	QPushButton *postButtonW;
	// widgets des docks
	QWidget         *dockWidgetLem;
	QWidget         *dockWidgetDic;
	QWidget         *dockWidgetScand;
	QWidget         *dockWidgetFlex;
	bool             dockVisible(QDockWidget *d);         
	// fonctions et variables diverses
	void             charger(QString f);
	QFont            font;
	QString          nfAb;          // nom du fichier à charger
	QString          nfAd;          // nom du fichier de sortie
	bool             precaution();  // autorise ou non la fermeture du fichier chargé
	QString          rech;          // dernière chaîne recherchée
	QString          repertoire;
};

#endif
