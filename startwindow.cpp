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

	libusb_init(nullptr);	//init libusb
}

startWindow::~startWindow()
{
	QFile::remove("./sb_loader.exe");		//for virtual box ONLY! Disable it when debugging!

	delete ui;
	delete image_OSLoader;
	delete image_OSLoader_System;
	delete image_System;
	delete aboutWindow;
	delete updWindow;
	delete optionsWindow;
	delete waitWindow;
	delete process;

	libusb_exit(nullptr);	//exit libusb
}

bool startWindow::searchRecoveryModeDevice() {
	struct libusb_device* dev = nullptr;
	struct libusb_device_handle *dev_hdl = nullptr;
	struct libusb_device_descriptor dev_dsp;
	bool isFound = true;

	dev_hdl = libusb_open_device_with_vid_pid(nullptr, HOSTLINK_VID, HOSTLINK_PID);

	if (dev_hdl != nullptr) {
		dev = libusb_get_device(dev_hdl);
		if (libusb_get_device_descriptor(dev, &dev_dsp) != 0) {
			isFound = false;
		}
	}
	else {
		isFound = false;
	}

	libusb_close(dev_hdl);
	return isFound;
}

void startWindow::on_button_OSLoader_path_clicked()
{
	ui->OSLoader_path->setText(QFileDialog::getOpenFileName(this, tr("Select a file"), tr(""), tr("OS Loader File(*.sb)")));
	if (ui->OSLoader_path->text() != "") {
		QFileInfo* info = new QFileInfo(ui->OSLoader_path->text());
		if (info->size() > (page_System - page_OSLoader) * 2112) {
			QMessageBox::critical(this, " ", "This OSLoader file is too large to flash.");
			ui->OSLoader_path->clear();
		}
		delete info;
	}
}


void startWindow::on_button_System_path_clicked()
{
	ui->System_path->setText(QFileDialog::getOpenFileName(this, tr("Select a file"), tr(""), tr("System File(*.sys)")));
	if (ui->System_path->text() != "") {
		QFileInfo* info = new QFileInfo(ui->System_path->text());
		if (info->size() > (MAX_PAGE - page_System) * 2112) {
			QMessageBox::critical(this, " ", "This System file is too large to flash.");
			ui->System_path->clear();
		}
		delete info;
	}
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

	if (searchRecoveryModeDevice()) {
		link_mode = HOSTLINK_MODE;
	}
	else {

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
	}

	ui->lineEdit_status->setText(link_texts[link_mode]);

	switch (link_mode)
	{
	case UNCONNECT_MODE:
		setButtonStatus(false, false, false);
		break;
	case HOSTLINK_MODE:
		setButtonStatus(true, true, true);
		break;
	case EDB_TEXT:
		setButtonStatus(false, false, false);
		break;
	case EDB_BIN:
		setButtonStatus(true, true, true);
		break;
	default:
		break;
	}

	waitWindow->hide();
	ui->pushButton_refresh->setEnabled(true);
}

void startWindow::setButtonStatus(const bool& O, const bool& S, const bool& OandS)
{
	ui->pushButton_update_O->setEnabled(O);
	ui->pushButton_update_OandS->setEnabled(S);
	ui->pushButton_update_S->setEnabled(OandS);
}

void startWindow::getReturnData(int OSLoader, int System, int edb) {
	page_OSLoader = OSLoader;
	page_System = System;
	edbMode = edb;
}

void startWindow::openProcess(const QString& path, const QStringList& argu) {
		process->start(path, argu);
		connect(process, SIGNAL(finished(int)), this, SLOT(readResult(int)));
}

void startWindow::readResult(int exitCode) {
	//QByteArray result;
	//result = process->readAll();
	//updWindow->addLine(QString::fromLocal8Bit(result));
	process->close();
}


bool startWindow::startUpdate(const QList<int>& work)
{
	updWindow->clear();
	updWindow->show();
	updWindow->addLine("Start update...");

	QStringList argu;

	for (int i = 0; i < work.size(); i++) {
		switch (link_mode)
		{
		case 1:     //recovery mode
			argu.append("-f");
			argu.append(ui->OSLoader_path->text());
			updWindow->addLine("Sending OSLoader to calculator RAM...");
			//openProcess(QDir::currentPath() + "/tool/sb_loader.exe", argu);
			openProcess( "./sb_loader.exe", argu);
			updWindow->addLine("Reboot and reconnect...");
			Sleep(5000);	//reboot interval
			on_pushButton_refresh_clicked();
			if (link_mode == EDB_BIN) {
				if (work[0] != UPDATE_OSLOADER) {
					startUpdate({ UPDATE_OSLOADER, UPDATE_SYSTEM });
				}
				else
				{
					startUpdate(work);
				}
			}
			else {
				return false;
			}
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
			QMessageBox::information(this, " ", "Update device successfully.");
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
	if (link_mode == HOSTLINK_MODE) {
		if (ui->OSLoader_path->text() == "") {
			QMessageBox::information(this, " ", "In HostLink mode,\nyou need to select an OSLoder file for updating.");
			return;
		}
	}

	if (ui->System_path->text() == "") {
		QMessageBox::critical(this, " ", "You have to select a file for System first.");
	}
	else {
		if (startUpdate({ UPDATE_SYSTEM })) {
			QMessageBox::information(this, " ", "Update device successfully.");
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
	if (link_mode == HOSTLINK_MODE) {
		on_pushButton_update_S_clicked();
		return;
	}

	if (ui->OSLoader_path->text() == "") {
		QMessageBox::critical(this, " ", "You have to select a file for OSLoader first.");
	}
	else {
		if (ui->System_path->text() == "") {
			QMessageBox::critical(this, " ", "You have to select a file for System first.");
		}
		else {
			if (startUpdate({ UPDATE_OSLOADER, UPDATE_SYSTEM })) {
				QMessageBox::information(this, " ", "Update device successfully.");
			}
			else {
				QMessageBox::critical(this, " ", "An error occurred while updating device.");
				
			}
		}
	}
	updWindow->hide();
	on_pushButton_refresh_clicked();
}
