#include "startwindow.h"
#include "ui_startwindow.h"

startWindow::startWindow(QWidget* parent)
	: QMainWindow(parent)
	, ui(new Ui::startWindow)
{
	ui->setupUi(this);

	QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

	setWindowTitle("ExistOS Updater");
	setWindowIcon(QIcon("image/icon.png"));

	image_OSLoader->addPixmap(QPixmap("D:/Projects/ExistOS_Updater_v2/image/update_OSLoader.bmp").scaled(64, 64));
	image_System->addPixmap(QPixmap("D:/Projects/ExistOS_Updater_v2/image/update_System.bmp").scaled(64, 64));
	image_OSLoader_System->addPixmap(QPixmap("D:/Projects/ExistOS_Updater_v2/image/update_OSLoader_System.bmp").scaled(64, 64));
	ui->graphicsView_O->setScene(image_OSLoader);
	ui->graphicsView_S->setScene(image_System);
	ui->graphicsView_OandS->setScene(image_OSLoader_System);
	ui->label->setText(VERSION);
	ui->lineEdit_status->setText(link_texts[link_mode]);

	ui->pushButton_update_O->setDisabled(true);
	ui->pushButton_update_OandS->setDisabled(true);
	ui->pushButton_update_S->setDisabled(true);

	connect(optionsWindow, SIGNAL(returnData(int, int, int)), this, SLOT(getReturnData(int, int, int)));

}

startWindow::~startWindow()
{
	delete ui;
	delete image_OSLoader;
	delete image_OSLoader_System;
	delete image_System;
	delete aboutWindow;
	delete updWindow;
	delete optionsWindow;
	delete waitWindow;
}

void startWindow::on_button_OSLoader_path_clicked()
{
	ui->OSLoader_path->setText(QFileDialog::getOpenFileName(this, tr("Select a file"), tr(""), tr("OS Loader File(*.sb)")));
}


void startWindow::on_button_System_path_clicked()
{
	ui->System_path->setText(QFileDialog::getOpenFileName(this, tr("Select a file"), tr(""), tr("System File(*.sys)")));
}

void startWindow::on_pushButton_about_clicked()
{
	aboutWindow->exec();
}


void startWindow::on_pushButton_options_clicked()
{
	optionsWindow->set(page_OSLoader, page_System, edbMode);
	optionsWindow->exec();
}

void startWindow::on_pushButton_refresh_clicked()
{
	ui->pushButton_refresh->setDisabled(true);
	waitWindow->show();



	switch (edbMode)
	{
	case EDB_BIN:

		if (edb.open(EDB_MODE_BIN)) {
			link_mode = EDB_BIN;
			edb.close();
		}
		else {
			link_mode = UNCONNECT_MODE;
		}
		break;
	case EDB_TEXT:

		if (edb.open(EDB_MODE_TEXT)) {
			link_mode = EDB_TEXT;
			edb.close();
		}
		else {
			link_mode = UNCONNECT_MODE;
		}
		break;
	default:
		break;
	}

	ui->lineEdit_status->setText(link_texts[link_mode]);

	switch (link_mode)
	{
	case UNCONNECT_MODE:
		ui->pushButton_update_O->setDisabled(true);
		ui->pushButton_update_OandS->setDisabled(true);
		ui->pushButton_update_S->setDisabled(true);
		break;
	case RECOVERY_MODE:
		ui->pushButton_update_O->setEnabled(true);
		ui->pushButton_update_OandS->setEnabled(true);
		ui->pushButton_update_S->setEnabled(true);
		break;
	case EDB_TEXT:
		ui->pushButton_update_O->setDisabled(true);
		ui->pushButton_update_OandS->setDisabled(true);
		ui->pushButton_update_S->setDisabled(true);
		break;
	case EDB_BIN:
		ui->pushButton_update_O->setEnabled(true);
		ui->pushButton_update_OandS->setEnabled(true);
		ui->pushButton_update_S->setEnabled(true);
		break;
	default:
		break;
	}

	waitWindow->hide();
	ui->pushButton_refresh->setEnabled(true);
}

void startWindow::getReturnData(int OSLoader, int System, int edb) {
	page_OSLoader = OSLoader;
	page_System = System;
	edbMode = edb;
}

bool startWindow::startUpdate(const QList<int>& work)
{
	updWindow->clear();
	updWindow->show();
	updWindow->addLine("Start update...");

	for (int i = 0; i < work.size(); i++) {
		switch (link_mode)
		{
		case 1:     //recovery mode

			break;
		case 3:     //edb bin mode
			flashImg item;
			errno_t ret;
			edb.open(EDB_MODE_BIN);
			switch (work.at(i))
			{
			case 1:     //update system
				memset(&item, 0, sizeof(item));

				ret = fopen_s(&item.f, ui->System_path->text().toLocal8Bit().data(), "rb");
				if (ret != 0) {
					updWindow->addLine("ERROR: Can not open file. Make sure you select the correct file.");
					return false;
				}

				item.filename = ui->System_path->text().toLocal8Bit().data();
				item.toPage = page_System;
				item.bootImg = false;
				updWindow->addLine("Flash system to page " + QString::number(page_System) + ".");
				imglist.push_back(item);
				edb.reset(EDB_MODE_TEXT);
				if (!edb.ping()) {
					updWindow->addLine("ERROR: Device no response.");
					return false;
				}
				//edb.vm_suspend();
				//edb.mscmode();

				edb.vm_suspend();
				for (flashImg& item : imglist)
				{
					edb.flash(item);
					//updWindow->addLine(QString::number(fclose(item.f)));
				}

				updWindow->addLine("Reboot...");
				edb.reboot();
				edb.close();

				break;
			case 2:     //update OSLoader
				memset(&item, 0, sizeof(item));

				ret = fopen_s(&item.f, ui->OSLoader_path->text().toLocal8Bit().data(), "rb");
				if (ret != 0) {
					updWindow->addLine("ERROR: Can not open file. Make sure you select the correct file.");
					return false;
				}

				item.filename = ui->OSLoader_path->text().toLocal8Bit().data();
				item.toPage = page_OSLoader;
				item.bootImg = true;
				updWindow->addLine("Flash OSLoader to page " + QString::number(page_OSLoader) + ".");
				imglist.push_back(item);
				edb.reset(EDB_MODE_TEXT);
				if (!edb.ping()) {
					updWindow->addLine("ERROR: Device no response.");
					return false;
				}

				edb.vm_suspend();
				for (flashImg& item : imglist)
				{
					edb.flash(item);
				}

				updWindow->addLine("Reboot...");
				edb.reboot();
				edb.close();

				break;
			default:
				return false;
				break;
			}
			break;
		default:
			return false;
			break;
		}

		Sleep(2000);
		on_pushButton_refresh_clicked();
	}

	updWindow->addLine("Done!");
	return true;
}


void startWindow::on_pushButton_update_O_clicked()
{
	if (ui->OSLoader_path->text() == "") {
		QMessageBox::critical(this, " ", "You have to select a file for OSLoader first.");
	}
	else {
		if (startUpdate({ UPDATE_OSLOADER })) {
			QMessageBox::information(this, " ", "Update OSLoader successfully.");
		}
		else {
			QMessageBox::critical(this, " ", "An error occurred while updating device.");
		}
	}
	updWindow->hide();
	on_pushButton_refresh_clicked();
}

void startWindow::on_pushButton_update_S_clicked()
{
	if (ui->System_path->text() == "") {
		QMessageBox::critical(this, " ", "You have to select a file for System first.");
	}
	else {
		if (startUpdate({ UPDATE_SYSTEM })) {
			QMessageBox::information(this, " ", "Update System successfully.");
		}
		else {
			QMessageBox::critical(this, " ", "An error occurred while updating device.");
		}
	}
	updWindow->hide();
	on_pushButton_refresh_clicked();
}

void startWindow::on_pushButton_update_OandS_clicked()
{


	if (ui->OSLoader_path->text() == "") {
		QMessageBox::critical(this, " ", "You have to select a file for OSLoader first.");
	}
	else {
		if (ui->System_path->text() == "") {
			QMessageBox::critical(this, " ", "You have to select a file for System first.");
		}
		else {
			if (startUpdate({ UPDATE_OSLOADER, UPDATE_SYSTEM })) {
				QMessageBox::information(this, " ", "Update OSLoader & System successfully.");
			}
			else {
				QMessageBox::critical(this, " ", "An error occurred while updating device.");
				
			}
		}
	}
	updWindow->hide();
	on_pushButton_refresh_clicked();
}
