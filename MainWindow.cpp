#include "MainWindow.h"
#include "./ui_MainWindow.h"

#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
	, settings("Manicorp", "VideoCutter", this)
	, videoDuration(0)
{
	ui->setupUi(this);

	ui->videoInLineEdit->setText(settings.value("VideoInLineEdit").toString());
	ui->startHourSpinBox->setValue(settings.value("StartHour").toInt());
	ui->startMinSpinBox->setValue(settings.value("StartMin").toInt());
	ui->startSecSpinBox->setValue(settings.value("StartSec").toInt());
	ui->endHourSpinBox->setValue(settings.value("EndHour").toInt());
	ui->endMinSpinBox->setValue(settings.value("EndMin").toInt());
	ui->endSecSpinBox->setValue(settings.value("EndSec").toInt());

	ui->progressBar->hide();
	ui->stopPushButton->hide();

	connect(&ffmpeg, &QProcess::readyReadStandardOutput, this, &MainWindow::onReadyRead);
	connect(&ffmpeg, &QProcess::readyReadStandardError, this, &MainWindow::onReadyRead);
	connect(&ffmpeg, &QProcess::finished, this, &MainWindow::onCutVideoProcessFinished);
}

MainWindow::~MainWindow()
{
	disconnect(&ffmpeg, &QProcess::readyReadStandardOutput, this, &MainWindow::onReadyRead);
	disconnect(&ffmpeg, &QProcess::readyReadStandardError, this, &MainWindow::onReadyRead);
	disconnect(&ffmpeg, &QProcess::finished, this, &MainWindow::onCutVideoProcessFinished);

	ffmpeg.kill();

	settings.setValue("VideoInLineEdit", ui->videoInLineEdit->text());
	settings.setValue("StartHour", ui->startHourSpinBox->value());
	settings.setValue("StartMin", ui->startMinSpinBox->value());
	settings.setValue("StartSec", ui->startSecSpinBox->value());
	settings.setValue("EndHour", ui->endHourSpinBox->value());
	settings.setValue("EndMin", ui->endMinSpinBox->value());
	settings.setValue("EndSec", ui->endSecSpinBox->value());
	
	delete ui;
}

int MainWindow::stringToSec(QString text)
{
	const QStringList values = text.split(":");
	const int sec = values[0].toInt() * 3600 + values[1].toInt() * 60 + values[2].toInt();
	return sec;
}

QString MainWindow::getEndTime()
{
	const int startSec = ui->startHourSpinBox->value() * 3600 + ui->startMinSpinBox->value() * 60 + ui->startSecSpinBox->value();
	const int endSec = ui->endHourSpinBox->value() * 3600 + ui->endMinSpinBox->value() * 60 + ui->endSecSpinBox->value();
	int duration = endSec - startSec;

	const int hour = duration / 3600;
	QString endTime = QString::number(hour) + ":";
	duration -= hour * 3600;
	const int min = duration / 60;
	endTime += QString::number(min) + ":";
	duration -= min * 60;
	endTime += QString::number(duration);

	return endTime;
}

void MainWindow::onVideoInPushButtonClicked()
{
	const QString folder = ui->videoInLineEdit->text().isEmpty() ? QDir::homePath() : ui->videoInLineEdit->text();
	const QString fileName = QFileDialog::getOpenFileName(this, tr("Ouvrir le fichier vidéo"), folder, "*.mkv *.mp4;;*.*");
	if (!fileName.isEmpty())
	{
		ui->videoInLineEdit->setText(fileName);
	}
}

void MainWindow::onCutVideoPushButtonClicked()
{
	const QString inputFileName = ui->videoInLineEdit->text();
	if (inputFileName.isEmpty())
	{
		QMessageBox::warning(this, tr("Fichier vide"), tr("Le fichier d'entrée est vide."));
		return;
	}
	else if (!QFile::exists(inputFileName))
	{
		QMessageBox::warning(this, tr("Problème avec le fichier d'entrée"), tr("Le fichier d'entrée n'existe pas."));
		return;
	}

	const QFileInfo inputFileInfo(inputFileName);
	const QString folder = inputFileInfo.absolutePath() + "/" + inputFileInfo.completeBaseName() + "-cutted." + inputFileInfo.completeSuffix();

	QString currentSuffix = "*." + inputFileInfo.completeSuffix();
	const QString outputFileName = QFileDialog::getSaveFileName(this, tr("Ouvrir le fichier vidéo"), folder, "*.mkv;;*.mp4;;*.*", &currentSuffix);
	if (outputFileName.isEmpty())
	{
		return;
	}

	if (ui->videoInLineEdit->text() == outputFileName)
	{
		QMessageBox::warning(this, tr("Problème"), tr("Le fichier d'entrée ne peux pas être le même que le fichier de sortie !"));
		return;
	}

	const QString ffmpegExe = "ffmpeg.exe";
	const QString startTime = QString::number(ui->startHourSpinBox->value()) + ":" + QString::number(ui->startMinSpinBox->value()) + ":" + QString::number(ui->startSecSpinBox->value());
	const QString endTime = getEndTime();
	videoDuration = stringToSec(endTime);
	QString vcodec = "h264";
	QStringList startTimeCommand;
	if (stringToSec(startTime) == 0)
	{
		vcodec = "copy";
	}
	else
	{
		QApplication::beep();
		QMessageBox message(tr("Attention"), tr("Couper la vidéo au début force le réencodage et ce processus peut-être très long. Continuer quand même ?"), QMessageBox::Question, QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton, this);
		message.setButtonText(QMessageBox::Yes, tr("Oui"));
		message.setButtonText(QMessageBox::No, tr("Non"));
		message.exec();
		if (message.result() == QMessageBox::No)
		{
			return;
		}
		startTimeCommand << "-ss" << startTime;
	}
	QStringList ffmpegArguments;
	ffmpegArguments
	<< "-y"
	<< "-i" << inputFileName
	<< "-vcodec" << vcodec
	<< "-acodec" << "copy"
	<< startTimeCommand
	<< "-t" << endTime
	<< outputFileName;

	ffmpeg.start(ffmpegExe, ffmpegArguments);
	ui->progressBar->show();
	ui->stopPushButton->show();
	ui->cutvideoPushButton->hide();
}

void MainWindow::onStopCutPushButtonClicked()
{
	ffmpeg.kill();
}

void MainWindow::onReadyRead()
{
	QString output = ffmpeg.readAllStandardOutput();
	QString error = ffmpeg.readAllStandardError();

	QString text = output.contains("time=") ? output : error;
	if (text.contains("time="))
	{
		const QString timeText = text.split("time=").last().split(" ").first().split(".").first();
		const float sec = stringToSec(timeText);
		if (sec > 0)
		{
			float progress = sec / videoDuration * 100.f;
			ui->progressBar->setValue(progress);
		}
	}
}

void MainWindow::onCutVideoProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
	ui->stopPushButton->hide();
	ui->progressBar->hide();
	ui->progressBar->setValue(0);
	ui->cutvideoPushButton->show();
	const QString output = ffmpeg.readAllStandardOutput();
	const QString error = ffmpeg.readAllStandardError();
	if (ui->actionLog->isChecked())
	{
		QFile outputFile(QDir::homePath() + "/output.txt");
		if (outputFile.open(QFile::WriteOnly))
		{
			QTextStream outputStream(&outputFile);
			outputStream << output;
		}

		QFile errorFile(QDir::homePath() + "/error.txt");
		if (errorFile.open(QFile::WriteOnly))
		{
			QTextStream errorStream(&errorFile);
			errorStream << error;
		}
	}
	QMessageBox::information(this, tr("Travail terminé"), tr("La vidéo a été coupée."));
}
