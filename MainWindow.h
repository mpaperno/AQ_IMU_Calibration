#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QPushButton>
#include <QProcess>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

public slots:
	void loadSettings();
	void writeSettings();

	void startCalculationProcess(QString appName, QStringList appArgs);
	void startcal1();
	void startcal2();
	void startcal3();
	void checkVaraince();
	void calcAppendStaticFiles(QStringList *args);
	void calcAppendDynamicFiles(QStringList *args);
	void calcAppendParamsFile(QStringList *args);
	QString calcGetSim3ParamPath();
	void startsim1();
	void startsim1b();
	void startsim2();
	void startsim3();
	void abortcalc();
	void check_var();
	void check_stop();
	void addStatic();
	void delStatic();
	void addDynamic();
	void delDynamic();
	void setUsersParams();
	void CreateUsersParams();
	void WriteUsersParams();
	void ShowUsersParams(QString fileName);
	double Round(double Zahl, unsigned int Stellen);
	void CalculatDeclination();
	void CalculatInclination();

	bool checkProcRunning(bool warn = true);
	void prtstexit(int stat);
	void prtstdout();
	void prtstderr();
	void showOutput(QString &output);
	void extProcessError(QProcess::ProcessError err);

	/** @brief Shows a status message on the bottom status bar */
	void showStatusMessage(const QString& status, int timeout);
	/** @brief Shows a status message on the bottom status bar */
	void showStatusMessage(const QString& status);
	/** @brief Shows a critical message as popup or as widget */
	void showCriticalMessage(const QString& title, const QString& message);
	/** @brief Shows an info message as popup or as widget */
	void showInfoMessage(const QString& title, const QString& message);

protected:
	void changeEvent(QEvent *e);

private:
	Ui::MainWindow *ui;

	QString aqBinFolderPath;    // absolute path to AQ supporting utils
	const char *platformExeExt; // OS-specific executables suffix (.exe for Win)

	QStringList StaticFiles;
	QStringList DynamicFiles;
	QString UsersParamsFile;
	float calVersion;
	QTextEdit* activeProcessStatusWdgt;
	QPushButton* currentCalcStartBtn;
	QPushButton* currentCalcAbortBtn;

	QString LastFilePath;
	QProcess ps_master;
};

#endif // MAINWINDOW_H
