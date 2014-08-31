#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "config.h"

#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QTextStream>
#include <QSettings>

#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	bool ok;

	aqBinFolderPath = QCoreApplication::applicationDirPath() + "/aq/bin/";
#if defined(Q_OS_WIN)
	platformExeExt = ".exe";
#else
	platformExeExt = "";
#endif

	calVersion = 1.0f;
	if (QFile::exists(aqBinFolderPath + "cal_version.txt")) {
		QFile cverfile(aqBinFolderPath + "cal_version.txt");
		if (cverfile.open(QFile::ReadOnly | QFile::Text)) {
			float chkver = cverfile.readAll().trimmed().toFloat(&ok);
			if (ok)
				calVersion = chkver;
		}
	}

	QSettings settings;
	if (settings.contains("WINDOW_GEOMETRY"))
		 restoreGeometry(settings.value("WINDOW_GEOMETRY").toByteArray());

	//
	// GUI setup

	ui->setupUi(this);

	// hide variance button for older versions of cal program
	if (calVersion <= 1.0f)
		 ui->pushButton_var_cal3->hide();

	// GUI slots
	connect(ui->pushButton_Add_Static, SIGNAL(clicked()),this,SLOT(addStatic()));
	connect(ui->pushButton_Remov_Static, SIGNAL(clicked()),this,SLOT(delStatic()));
	connect(ui->pushButton_Add_Dynamic, SIGNAL(clicked()),this,SLOT(addDynamic()));
	connect(ui->pushButton_Remove_Dynamic, SIGNAL(clicked()),this,SLOT(delDynamic()));
	connect(ui->pushButton_Sel_params_file_user, SIGNAL(clicked()),this,SLOT(setUsersParams()));
	connect(ui->pushButton_Create_params_file_user, SIGNAL(clicked()),this,SLOT(CreateUsersParams()));
	connect(ui->pushButton_save, SIGNAL(clicked()),this,SLOT(WriteUsersParams()));
	connect(ui->pushButton_Calculate, SIGNAL(clicked()),this,SLOT(CalculatDeclination()));
	connect(ui->pushButton_start_cal1, SIGNAL(clicked()),this,SLOT(startcal1()));
	connect(ui->pushButton_start_cal2, SIGNAL(clicked()),this,SLOT(startcal2()));
	connect(ui->pushButton_start_cal3, SIGNAL(clicked()),this,SLOT(startcal3()));
	connect(ui->pushButton_var_cal3  , SIGNAL(clicked()),this,SLOT(checkVaraince()));
	connect(ui->pushButton_start_sim1, SIGNAL(clicked()),this,SLOT(startsim1()));
	connect(ui->pushButton_start_sim1_2, SIGNAL(clicked()),this,SLOT(startsim1b()));
	connect(ui->pushButton_start_sim2, SIGNAL(clicked()),this,SLOT(startsim2()));
	connect(ui->pushButton_start_sim3, SIGNAL(clicked()),this,SLOT(startsim3()));
	connect(ui->pushButton_abort_cal1, SIGNAL(clicked()),this,SLOT(abortcalc()));
	connect(ui->pushButton_abort_cal2, SIGNAL(clicked()),this,SLOT(abortcalc()));
	connect(ui->pushButton_abort_cal3, SIGNAL(clicked()),this,SLOT(abortcalc()));
	connect(ui->pushButton_abort_sim1, SIGNAL(clicked()),this,SLOT(abortcalc()));
	connect(ui->pushButton_abort_sim1_2, SIGNAL(clicked()),this,SLOT(abortcalc()));
	connect(ui->pushButton_abort_sim2, SIGNAL(clicked()),this,SLOT(abortcalc()));
	connect(ui->pushButton_abort_sim3, SIGNAL(clicked()),this,SLOT(abortcalc()));
	connect(ui->checkBox_sim3_4_var_1, SIGNAL(clicked()),this,SLOT(check_var()));
	connect(ui->checkBox_sim3_4_stop_1, SIGNAL(clicked()),this,SLOT(check_stop()));
	connect(ui->checkBox_sim3_4_var_2, SIGNAL(clicked()),this,SLOT(check_var()));
	connect(ui->checkBox_sim3_4_stop_2, SIGNAL(clicked()),this,SLOT(check_stop()));
	connect(ui->checkBox_sim3_5_var, SIGNAL(clicked()),this,SLOT(check_var()));
	connect(ui->checkBox_sim3_5_stop, SIGNAL(clicked()),this,SLOT(check_stop()));
	connect(ui->checkBox_sim3_6_var, SIGNAL(clicked()),this,SLOT(check_var()));
	connect(ui->checkBox_sim3_6_stop, SIGNAL(clicked()),this,SLOT(check_stop()));

	//Process Slots
	//ps_master.setProcessChannelMode(QProcess::MergedChannels);
	connect(&ps_master, SIGNAL(finished(int)), this, SLOT(prtstexit(int)));
	connect(&ps_master, SIGNAL(readyReadStandardOutput()), this, SLOT(prtstdout()));
	connect(&ps_master, SIGNAL(readyReadStandardError()), this, SLOT(prtstderr()));
	connect(&ps_master, SIGNAL(error(QProcess::ProcessError)), this, SLOT(extProcessError(QProcess::ProcessError)));

	loadSettings();

}

MainWindow::~MainWindow()
{
	writeSettings();
	delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
	QMainWindow::changeEvent(e);
	switch (e->type()) {
		case QEvent::LanguageChange:
			ui->retranslateUi(this);
			break;
		default:
			break;
	}
}

void MainWindow::loadSettings()
{
	QSettings settings;

	if (settings.contains("STATIC_FILE_COUNT"))
	{
		qint32 FileStaticCount = settings.value("STATIC_FILE_COUNT").toInt();
		StaticFiles.clear();
		for ( int i =0; i<FileStaticCount; i++) {
			StaticFiles.append(settings.value("STATIC_FILE" + QString::number(i)).toString());
			ui->listWidgetStatic->addItem(settings.value("STATIC_FILE" + QString::number(i)).toString());
		}
	}
	if (settings.contains("DYNAMIC_FILE_COUNT"))
	{
		qint32 FileDynamicCount = settings.value("DYNAMIC_FILE_COUNT").toInt();
		DynamicFiles.clear();
		for ( int i =0; i<FileDynamicCount; i++) {
			DynamicFiles.append(settings.value("DYNAMIC_FILE" + QString::number(i)).toString());
			ui->listWidgetDynamic->addItem(settings.value("DYNAMIC_FILE" + QString::number(i)).toString());
		}
	}

	ui->lineEdit_insert_declination->setText(settings.value("DECLINATION_SOURCE").toString());
	ui->lineEdit_cal_declination->setText(settings.value("DECLINATION_CALC").toString());

	ui->lineEdit_insert_inclination->setText(settings.value("INCLINATION_SOURCE").toString());
	ui->lineEdit_cal_inclination->setText(settings.value("INCLINATION_CALC").toString());

	if (settings.contains("USERS_PARAMS_FILE")) {
		UsersParamsFile = settings.value("USERS_PARAMS_FILE").toString();
		if (QFile::exists(UsersParamsFile))
			ShowUsersParams(QDir::toNativeSeparators(UsersParamsFile));
	}

	ui->sim3_4_var_1->setText(settings.value("AUTOQUAD_VARIANCE1").toString());
	ui->sim3_4_var_2->setText(settings.value("AUTOQUAD_VARIANCE2").toString());
	ui->sim3_5_var->setText(settings.value("AUTOQUAD_VARIANCE3").toString());
	ui->sim3_6_var->setText(settings.value("AUTOQUAD_VARIANCE4").toString());

	ui->sim3_4_stop_1->setText(settings.value("AUTOQUAD_STOP1").toString());
	ui->sim3_4_stop_2->setText(settings.value("AUTOQUAD_STOP2").toString());
	ui->sim3_5_stop->setText(settings.value("AUTOQUAD_STOP3").toString());
	ui->sim3_6_stop->setText(settings.value("AUTOQUAD_STOP4").toString());

	LastFilePath = settings.value("AUTOQUAD_LAST_PATH").toString();

}

void MainWindow::writeSettings()
{
	QSettings settings;

	settings.setValue("APP_VERSION", APP_VERSION);

	settings.setValue("STATIC_FILE_COUNT", QString::number(StaticFiles.count()));
	for ( int i = 0; i<StaticFiles.count(); i++) {
		settings.setValue("STATIC_FILE" + QString::number(i), StaticFiles.at(i));
	}
	settings.setValue("DYNAMIC_FILE_COUNT", QString::number(DynamicFiles.count()));
	for ( int i = 0; i<DynamicFiles.count(); i++) {
		settings.setValue("DYNAMIC_FILE" + QString::number(i), DynamicFiles.at(i));
	}

	settings.setValue("DECLINATION_SOURCE", ui->lineEdit_insert_declination->text());
	settings.setValue("DECLINATION_CALC", ui->lineEdit_cal_declination->text());
	settings.setValue("INCLINATION_SOURCE", ui->lineEdit_insert_inclination->text());
	settings.setValue("INCLINATION_CALC", ui->lineEdit_cal_inclination->text());

	settings.setValue("USERS_PARAMS_FILE", UsersParamsFile);

	settings.setValue("AUTOQUAD_VARIANCE1", ui->sim3_4_var_1->text());
	settings.setValue("AUTOQUAD_VARIANCE2", ui->sim3_4_var_2->text());
	settings.setValue("AUTOQUAD_VARIANCE3", ui->sim3_5_var->text());
	settings.setValue("AUTOQUAD_VARIANCE4", ui->sim3_6_var->text());

	settings.setValue("AUTOQUAD_STOP1", ui->sim3_4_stop_1->text());
	settings.setValue("AUTOQUAD_STOP2", ui->sim3_4_stop_2->text());
	settings.setValue("AUTOQUAD_STOP3", ui->sim3_5_stop->text());
	settings.setValue("AUTOQUAD_STOP4", ui->sim3_5_stop->text());

	settings.setValue("AUTOQUAD_LAST_PATH", LastFilePath);

	if (isVisible())
		 settings.setValue("WINDOW_GEOMETRY", saveGeometry());

	settings.sync();
}


void MainWindow::addStatic()
{
	QString dirPath;
	if ( LastFilePath == "")
		dirPath = QCoreApplication::applicationDirPath();
	else
		dirPath = LastFilePath;
	QFileInfo dir(dirPath);

	// use native file dialog
	QString fileName = QFileDialog::getOpenFileName(this, tr("Select AQ Static Log File"), dir.absoluteFilePath(),
																	tr("AQ Log File") + " (*.LOG);;" + tr("All File Types") + " (*.*)");

	if (fileName.length()) {
		QString fileNameLocale = QDir::toNativeSeparators(fileName);
		ui->listWidgetStatic->addItem(fileNameLocale);
		StaticFiles.append(fileNameLocale);
		LastFilePath = fileNameLocale;
	}
}

void MainWindow::delStatic()
{
	int currIndex = ui->listWidgetStatic->row(ui->listWidgetStatic->currentItem());
	if ( currIndex >= 0) {
		QString SelStaticFile =  ui->listWidgetStatic->item(currIndex)->text();
		StaticFiles.removeAt(StaticFiles.indexOf(SelStaticFile));
		ui->listWidgetStatic->takeItem(currIndex);
	}
}

void MainWindow::addDynamic()
{
	QString dirPath;
	if ( LastFilePath == "")
		dirPath = QCoreApplication::applicationDirPath();
	else
		dirPath = LastFilePath;
	QFileInfo dir(dirPath);

	// use native file dialog
	QString fileName = QFileDialog::getOpenFileName(this, tr("Select AQ Dynamic Log File"), dir.absoluteFilePath(),
																	tr("AQ Log File") + " (*.LOG);;" + tr("All File Types") + " (*.*)");

	if (fileName.length()) {
		QString fileNameLocale = QDir::toNativeSeparators(fileName);
		ui->listWidgetDynamic->addItem(fileNameLocale);
		DynamicFiles.append(fileNameLocale);
		LastFilePath = fileNameLocale;
	}

}

void MainWindow::delDynamic()
{
	int currIndex = ui->listWidgetDynamic->row(ui->listWidgetDynamic->currentItem());
	if ( currIndex >= 0) {
		QString SelDynamicFile =  ui->listWidgetDynamic->item(currIndex)->text();
		DynamicFiles.removeAt(DynamicFiles.indexOf(SelDynamicFile));
		ui->listWidgetDynamic->takeItem(currIndex);
	}
}


void MainWindow::setUsersParams() {
	QString dirPath = QDir::toNativeSeparators(UsersParamsFile);
	QFileInfo dir(dirPath);

	// use native file dialog
	QString fileName = QFileDialog::getOpenFileName(this, tr("Select Parameters File"), dir.absoluteFilePath(),
																	tr("AQ Parameter File") + " (*.params);;" + tr("All File Types") + " (*.*)");

	if (fileName.length())
		ShowUsersParams(QDir::toNativeSeparators(fileName));
	else
		return;
}

void MainWindow::ShowUsersParams(QString fileName) {
	QFile file(fileName);
	UsersParamsFile = file.fileName();
	ui->lineEdit_user_param_file->setText(QDir::toNativeSeparators(UsersParamsFile));
	file.open(QIODevice::ReadOnly | QIODevice::Text);
	ui->textOutput_Users_Params->setText(file.readAll());
	file.close();
}

void MainWindow::CreateUsersParams() {
	QString dirPath = UsersParamsFile ;
	QFileInfo dir(dirPath);

	// use native file dialog
	QString fileName = QFileDialog::getSaveFileName(this, tr("Select Parameters File"), dir.absoluteFilePath(),
																	tr("AQ Parameter File") + " (*.params);;" + tr("All File Types") + " (*.*)");

	if (fileName.length())
		UsersParamsFile = fileName;
	else
		return;

	if (!UsersParamsFile.endsWith(".params") )
		UsersParamsFile += ".params";

	UsersParamsFile = QDir::toNativeSeparators(UsersParamsFile);
	QFile file( UsersParamsFile );
	if ( file.exists())
	{
		QMessageBox msgBox;
		msgBox.setWindowTitle("Question");
		msgBox.setInformativeText("file already exists, Overwrite?");
		msgBox.setWindowModality(Qt::ApplicationModal);
		msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
		int ret = msgBox.exec();
		switch (ret) {
			case QMessageBox::Yes:
				file.close();
				QFile::remove(UsersParamsFile );
				break;
			default:
				return;
		}
	}

	if( !file.open( QIODevice::WriteOnly | QIODevice::Text ) ) {
		showInfoMessage("Warning!", tr("Could not open params file. %1").arg(file.errorString()));
		return;
	}

	QDataStream stream( &file );
	stream << "";
	file.close();

	ShowUsersParams(UsersParamsFile);
	//    ui->lineEdit_user_param_file->setText(QDir::toNativeSeparators(UsersParamsFile));
}

void MainWindow::WriteUsersParams() {
	QString message = ui->textOutput_Users_Params->toPlainText();
	QFile file(UsersParamsFile);
	if (!file.open(QIODevice::WriteOnly))
	{
		QMessageBox::information(this, tr("Unable to open file"), file.errorString());
		return;
	}
	QTextStream out(&file);
	out << message;
	file.close();
}

void MainWindow::CalculatDeclination() {

	QString dec_source = ui->lineEdit_insert_declination->text();
	if ( !dec_source.contains(".")) {
		QMessageBox::information(this, "Error", "Wrong format for magnetic declination!",QMessageBox::Ok, 0 );
		return;
	}
	/*
	 if ( !dec_source.startsWith("-")) {
		  QMessageBox::information(this, "Error", "Wrong format for magnetic declination!",QMessageBox::Ok, 0 );
		  return;
	 }
	 if ( dec_source.length() != 6 ) {
		  QMessageBox::information(this, "Error", "Wrong format for magnetic declination!",QMessageBox::Ok, 0 );
		  return;
	 }
	 */
	QStringList HoursMinutes = dec_source.split(".");

	if ( HoursMinutes.count() != 2 ) {
		QMessageBox::information(this, "Error", "Wrong format for magnetic declination!",QMessageBox::Ok, 0 );
		return;
	}
	qint32 secounds = HoursMinutes.at(1).toInt();
	float secounds_calc = (100.0f/60.0f) * secounds;
	// Set together
	QString recalculated;
	recalculated.append("#define");
	recalculated.append(' ');
	recalculated.append("IMU_MAG_INCL");
	recalculated.append(' ');
	recalculated.append(HoursMinutes.at(0));
	recalculated.append(".");
	recalculated.append( QString::number(secounds_calc,'f',0));

	ui->lineEdit_cal_declination->setText(recalculated);
	CalculatInclination();
}

double MainWindow::Round(double Zahl, unsigned int Stellen)
{
	Zahl *= pow((double)10, (double)Stellen);
	if (Zahl >= 0)
		floor(Zahl + 0.5);
	else
		ceil(Zahl - 0.5);
	Zahl /= pow((double)10, (double)Stellen);
	return Zahl;
}

void MainWindow::CalculatInclination() {

	QString inc_source = ui->lineEdit_insert_inclination->text();
	if ( !inc_source.contains(".")) {
		QMessageBox::information(this, "Error", "Wrong format for magnetic inclination!",QMessageBox::Ok, 0 );
		return;
	}
	if ( inc_source.length() < 3 ) {
		QMessageBox::information(this, "Error", "Wrong format for magnetic inclination!",QMessageBox::Ok, 0 );
		return;
	}
	QStringList HoursMinutes = inc_source.split('.');

	qint32 secounds = HoursMinutes.at(1).toInt();
	float secounds_calc =  (100.0f/60.0f) * secounds;
	//secounds_calc = Round(secounds_calc, 0);
	// Set together
	QString recalculated;
	recalculated.append(HoursMinutes.at(0));
	recalculated.append(".");
	recalculated.append( QString::number(secounds_calc,'f',0));
	ui->lineEdit_cal_inclination->setText(recalculated);

}


void MainWindow::check_var()
{
	if ( ui->checkBox_sim3_4_var_1->checkState()) {
		ui->sim3_4_var_1->setEnabled(true);
	}
	else if  ( !ui->checkBox_sim3_4_var_1->checkState()) {
		ui->sim3_4_var_1->setEnabled(false);
	}

	if ( ui->checkBox_sim3_4_var_2->checkState()) {
		ui->sim3_4_var_2->setEnabled(true);
	}
	else if  ( !ui->checkBox_sim3_4_var_2->checkState()) {
		ui->sim3_4_var_2->setEnabled(false);
	}

	if ( ui->checkBox_sim3_5_var->checkState()) {
		ui->sim3_5_var->setEnabled(true);
	}
	else if  ( !ui->checkBox_sim3_5_var->checkState()) {
		ui->sim3_5_var->setEnabled(false);
	}

	if ( ui->checkBox_sim3_6_var->checkState()) {
		ui->sim3_6_var->setEnabled(true);
	}
	else if  ( !ui->checkBox_sim3_6_var->checkState()) {
		ui->sim3_6_var->setEnabled(false);
	}

}

void MainWindow::check_stop()
{
	if ( ui->checkBox_sim3_4_stop_1->checkState()) {
		ui->sim3_4_stop_1->setEnabled(true);
	}
	else if  ( !ui->checkBox_sim3_4_stop_1->checkState()) {
		ui->sim3_4_stop_1->setEnabled(false);
	}

	if ( ui->checkBox_sim3_4_stop_2->checkState()) {
		ui->sim3_4_stop_2->setEnabled(true);
	}
	else if  ( !ui->checkBox_sim3_4_stop_2->checkState()) {
		ui->sim3_4_stop_2->setEnabled(false);
	}

	if ( ui->checkBox_sim3_5_stop->checkState()) {
		ui->sim3_5_stop->setEnabled(true);
	}
	else if  ( !ui->checkBox_sim3_5_stop->checkState()) {
		ui->sim3_5_stop->setEnabled(false);
	}

	if ( ui->checkBox_sim3_6_stop->checkState()) {
		ui->sim3_6_stop->setEnabled(true);
	}
	else if  ( !ui->checkBox_sim3_6_stop->checkState()) {
		ui->sim3_6_stop->setEnabled(false);
	}


}

void MainWindow::startCalculationProcess(QString appName, QStringList appArgs) {
	if (checkProcRunning(false))
		return;

	writeSettings();

	QString appPath = "\"" + QDir::toNativeSeparators(aqBinFolderPath + appName + platformExeExt) + "\" " + appArgs.join(" ");

	activeProcessStatusWdgt->clear();
	activeProcessStatusWdgt->append(appPath + "\n\n");

	currentCalcStartBtn->setEnabled(false);
	currentCalcAbortBtn->setEnabled(true);

	ps_master.setWorkingDirectory(aqBinFolderPath);
	ps_master.start(appPath, QIODevice::Unbuffered | QIODevice::ReadWrite);
}

void MainWindow::abortcalc(){
	if ( currentCalcAbortBtn == ui->pushButton_abort_cal3 && calVersion > 1.0f ) {
#ifdef Q_OS_WIN
		QFile f(aqBinFolderPath + "endCal");
		f.open(QIODevice::ReadWrite);
		f.close();
#else
		ps_master.write("e");
#endif
	}
	else {
		if ( ps_master.Running)
			ps_master.close();
	}
}

void MainWindow::checkVaraince() {
	if ( currentCalcAbortBtn == ui->pushButton_abort_cal3 && calVersion > 1.0f ) {
#ifdef Q_OS_WIN
		QFile f(aqBinFolderPath + "testVariance");
		f.open(QIODevice::ReadWrite);
		f.close();
#else
		ps_master.write("v");
#endif
	}
}

void MainWindow::calcAppendStaticFiles(QStringList *args) {
	for (int i=0; i < StaticFiles.size(); ++i)
		args->append("\"" + StaticFiles.at(i) + "\"");
	args->append(":");
}

void MainWindow::calcAppendDynamicFiles(QStringList *args) {
	for (int i=0; i < DynamicFiles.size(); ++i)
		args->append("\"" + DynamicFiles.at(i) + "\"");
}

void MainWindow::calcAppendParamsFile(QStringList *args) {
	args->append("-p");
	args->append("\"" + QDir::toNativeSeparators(ui->lineEdit_user_param_file->text()) + "\"");
}

QString MainWindow::calcGetSim3ParamPath() {
	QString Sim3ParaPath = "sim3.params";
	if ( ui->checkBox_DIMU->isChecked())
		Sim3ParaPath = "sim3_dimu.params";

	Sim3ParaPath = "\"" + QDir::toNativeSeparators(aqBinFolderPath + Sim3ParaPath) + "\"";

	return Sim3ParaPath;
}

void MainWindow::startcal1(){
	if (checkProcRunning())
		return;

	QStringList Arguments;

	Arguments.append("--rate");
	calcAppendStaticFiles(&Arguments);

	activeProcessStatusWdgt = ui->textOutput_cal1;
	currentCalcStartBtn = ui->pushButton_start_cal1;
	currentCalcAbortBtn = ui->pushButton_abort_cal1;

	startCalculationProcess("cal", Arguments);
}

void MainWindow::startcal2(){
	if (checkProcRunning())
		return;

	QStringList Arguments;

	Arguments.append("--acc");
	calcAppendStaticFiles(&Arguments);
	calcAppendDynamicFiles(&Arguments);

	activeProcessStatusWdgt = ui->textOutput_cal2;
	currentCalcStartBtn = ui->pushButton_start_cal2;
	currentCalcAbortBtn = ui->pushButton_abort_cal2;

	startCalculationProcess("cal", Arguments);
}

void MainWindow::startcal3(){
	if (checkProcRunning())
		return;

	QStringList Arguments;

#ifdef Q_OS_WIN
	if ( QFile::exists(aqBinFolderPath + "testVariance") ) {
		QFile::remove(aqBinFolderPath + "testVariance");
	}
	if ( QFile::exists(aqBinFolderPath + "endCal") ) {
		QFile::remove(aqBinFolderPath + "endCal");
	}
#endif

	if ( ui->checkBox_DIMU->isChecked())
		Arguments.append("-b");
	Arguments.append("--mag");

	calcAppendParamsFile(&Arguments);
	calcAppendStaticFiles(&Arguments);
	calcAppendDynamicFiles(&Arguments);

	activeProcessStatusWdgt = ui->textOutput_cal3;
	currentCalcStartBtn = ui->pushButton_start_cal3;
	currentCalcAbortBtn = ui->pushButton_abort_cal3;

	startCalculationProcess("cal", Arguments);
}

void MainWindow::startsim1(){
	if (checkProcRunning())
		return;

	QStringList Arguments;

	if ( ui->checkBox_DIMU->isChecked())
		Arguments.append("-b");
	Arguments.append("--gyo");
	Arguments.append("-p");
	Arguments.append(calcGetSim3ParamPath());
	calcAppendParamsFile(&Arguments);

	if ( ui->checkBox_sim3_4_var_1->checkState() ) {
		Arguments.append("--var=" + ui->sim3_4_var_1->text());
	}
	if ( ui->checkBox_sim3_4_stop_1->checkState() ) {
		Arguments.append("--stop=" + ui->sim3_4_stop_1->text());
	}

	calcAppendDynamicFiles(&Arguments);

	activeProcessStatusWdgt = ui->textOutput_sim1;
	currentCalcStartBtn = ui->pushButton_start_sim1;
	currentCalcAbortBtn = ui->pushButton_abort_sim1;

	startCalculationProcess("sim3", Arguments);
}

void MainWindow::startsim1b(){
	if (checkProcRunning())
		return;

	QStringList Arguments;

	if ( ui->checkBox_DIMU->isChecked())
		Arguments.append("-b");
	Arguments.append("--acc");
	Arguments.append("-p");
	Arguments.append(calcGetSim3ParamPath());
	calcAppendParamsFile(&Arguments);

	if ( ui->checkBox_sim3_4_var_2->checkState() ) {
		Arguments.append("--var=" + ui->sim3_4_var_2->text());
	}
	if ( ui->checkBox_sim3_4_stop_2->checkState() ) {
		Arguments.append("--stop=" + ui->sim3_4_var_2->text());
	}

	calcAppendDynamicFiles(&Arguments);

	activeProcessStatusWdgt = ui->textOutput_sim1_2;
	currentCalcStartBtn = ui->pushButton_start_sim1_2;
	currentCalcAbortBtn = ui->pushButton_abort_sim1_2;

	startCalculationProcess("sim3", Arguments);
}

void MainWindow::startsim2(){
	if (checkProcRunning())
		return;

	QStringList Arguments;

	if ( ui->checkBox_DIMU->isChecked())
		Arguments.append("-b");
	Arguments.append("--acc");
	Arguments.append("--gyo");
	Arguments.append("-p");
	Arguments.append(calcGetSim3ParamPath());
	calcAppendParamsFile(&Arguments);

	if ( ui->checkBox_sim3_5_var->checkState() ) {
		Arguments.append("--var=" + ui->sim3_5_var->text());
	}
	if ( ui->checkBox_sim3_5_stop->checkState() ) {
		Arguments.append("--stop=" + ui->sim3_5_stop->text());
	}

	calcAppendDynamicFiles(&Arguments);

	activeProcessStatusWdgt = ui->textOutput_sim2;
	currentCalcStartBtn = ui->pushButton_start_sim2;
	currentCalcAbortBtn = ui->pushButton_abort_sim2;

	startCalculationProcess("sim3", Arguments);
}

void MainWindow::startsim3(){
	if (checkProcRunning())
		return;

	QStringList Arguments;

	Arguments.append("--mag");
	if ( ui->checkBox_DIMU->isChecked())
		Arguments.append("-b");
	Arguments.append("--incl");
	Arguments.append("-p");
	Arguments.append(calcGetSim3ParamPath());
	calcAppendParamsFile(&Arguments);

	if ( ui->checkBox_sim3_6_var->checkState() ) {
		Arguments.append("--var=" + ui->sim3_6_var->text());
	}
	if ( ui->checkBox_sim3_6_stop->checkState() ) {
		Arguments.append("--stop=" + ui->sim3_6_var->text());
	}

	calcAppendDynamicFiles(&Arguments);

	activeProcessStatusWdgt = ui->textOutput_sim3;
	currentCalcStartBtn = ui->pushButton_start_sim3;
	currentCalcAbortBtn = ui->pushButton_abort_sim3;

	startCalculationProcess("sim3", Arguments);
}



bool MainWindow::checkProcRunning(bool warn) {
	if (ps_master.state() == QProcess::Running) {
		if (warn)
			showCriticalMessage(
						tr("Process already running."),
						tr("There appears to be a calculation step already running. Please abort it first."));
		return true;
	}
	return false;
}

void MainWindow::prtstexit(int stat) {
	Q_UNUSED(stat)

	prtstdout();

	if (currentCalcStartBtn)
		currentCalcStartBtn->setEnabled(true);

	if (currentCalcAbortBtn)
		currentCalcAbortBtn->setEnabled(false);
}

void MainWindow::prtstdout() {
	QString output = ps_master.readAllStandardOutput();

	showOutput(output);
}

void MainWindow::prtstderr() {
	QString output = ps_master.readAllStandardError();

	showOutput(output);
}

void MainWindow::showOutput(QString &output) {

	// hack to handle escape codes to clear screen
	if (output.indexOf(QRegExp("\\x1b\\[(H|2J)")) > -1)
		activeProcessStatusWdgt->clear();

	// hack fix for newline sometimes missing before Loops counter output
	output = output.replace("Loops", "\nLoops");

	// remove all escape codes
	output = output.replace(QRegExp("\\x1b\\[[\\dHJfKm]{1,2}"), "");

	activeProcessStatusWdgt->insertPlainText(output);
	activeProcessStatusWdgt->ensureCursorVisible();
}


/**
 * @brief Handle external process error code
 * @param err Error code
 */
void MainWindow::extProcessError(QProcess::ProcessError err) {
	QString msg;
	switch(err)
	{
		case QProcess::FailedToStart:
			msg = tr("Failed to start.");
			break;
		case QProcess::Crashed:
			msg = tr("Process terminated (aborted or crashed).");
			break;
		case QProcess::Timedout:
			msg = tr("Timeout waiting for process.");
			break;
		case QProcess::WriteError:
			msg = tr("Cannot write to process, exiting.");
			break;
		case QProcess::ReadError:
			msg = tr("Cannot read from process, exiting.");
			break;
		default:
			msg = tr("Unknown error");
			break;
	}
	activeProcessStatusWdgt->append(msg);
}

/**
 * The status message will be overwritten if a new message is posted to this function
 *
 * @param status message text
 * @param timeout how long the status should be displayed
 */
void MainWindow::showStatusMessage(const QString& status, int timeout)
{
	statusBar()->showMessage(status, timeout);
}

/**
 * The status message will be overwritten if a new message is posted to this function.
 * it will be automatically hidden after 5 seconds.
 *
 * @param status message text
 */
void MainWindow::showStatusMessage(const QString& status)
{
	statusBar()->showMessage(status, 20000);
}

void MainWindow::showCriticalMessage(const QString& title, const QString& message)
{
	QMessageBox msgBox(this);
	msgBox.setWindowFlags(msgBox.windowFlags() | Qt::WindowStaysOnTopHint);
	msgBox.setIcon(QMessageBox::Critical);
	msgBox.setText(title);
	msgBox.setInformativeText(message);
	msgBox.setStandardButtons(QMessageBox::Ok);
	msgBox.setDefaultButton(QMessageBox::Ok);
	msgBox.exec();
}

void MainWindow::showInfoMessage(const QString& title, const QString& message)
{
	QMessageBox msgBox(this);
	msgBox.setWindowFlags(msgBox.windowFlags() | Qt::WindowStaysOnTopHint);
	msgBox.setIcon(QMessageBox::Information);
	msgBox.setText(title);
	msgBox.setInformativeText(message);
	msgBox.setStandardButtons(QMessageBox::Ok);
	msgBox.setDefaultButton(QMessageBox::Ok);
	msgBox.exec();
}

