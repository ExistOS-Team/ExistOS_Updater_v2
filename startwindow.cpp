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

	//init libusb
	if (libusb_init(nullptr) != 0) {
		QMessageBox::critical(this, " ", "Can not init libusb!");
	}

	image_OSLoader->addPixmap(QPixmap(QDir::currentPath() + "/image/update_OSLoader.bmp").scaled(64, 64));
	image_System->addPixmap(QPixmap(QDir::currentPath() + "/image/update_System.bmp").scaled(64, 64));
	image_OSLoader_System->addPixmap(QPixmap(QDir::currentPath() + "/image/update_OSLoader_System.bmp").scaled(64, 64));

	ui->graphicsView_O->setScene(image_OSLoader);
	ui->graphicsView_S->setScene(image_System);
	ui->graphicsView_OandS->setScene(image_OSLoader_System);
	ui->label->setText(VERSION);
	ui->lineEdit_status->setText(TEXT_DEVICE_DISCONNECTED);

	ui->pushButton_update_O->setDisabled(true);
	ui->pushButton_update_OandS->setDisabled(true);
	ui->pushButton_update_S->setDisabled(true);

	QFileInfo* ini_file = new QFileInfo;
	ini_file->setFile("config.ini");

	if (!ini_file->exists()) {

		ini->beginGroup(tr("Path"));
		ini->setValue(tr("osloader"), "");
		ini->setValue(tr("system"), "");
		ini->endGroup();

		ini->beginGroup(tr("Statu"));
		ini->setValue(tr("savePath"), false);
		ini->endGroup();

		ui->checkBox_remember_path->setChecked(false);
	}
	else {
		if (ini->value(tr("/Statu/savePath")) == true) {
			QStringList path;
			path.append(ini->value(tr("/Path/osloader")).toString());
			path.append(ini->value(tr("/Path/system")).toString());

			if (path.at(0) != "") {
				if (QFileInfo(path.at(0)).exists()) ui->OSLoader_path->setText(path.at(0));
			}
			if (path.at(1) != "") {
				if (QFileInfo(path.at(1)).exists()) ui->System_path->setText(path.at(1));
			}

			ui->checkBox_remember_path->setChecked(true);
		}
	}
	delete ini_file;

	connect(optionsWindow, SIGNAL(returnData(int, int, int)), this, SLOT(getReturnData(int, int, int)));
}

startWindow::~startWindow()
{
	if (ui->checkBox_remember_path->isChecked()) {
		ini->setValue(tr("/Path/osloader"), ui->OSLoader_path->text());
		ini->setValue(tr("/Path/system"), ui->System_path->text());
		ini->setValue(tr("/Statu/savePath"), true);
	}
	else {
		ini->setValue(tr("/Path/osloader"), "");
		ini->setValue(tr("/Path/system"), "");
		ini->setValue(tr("/Statu/savePath"), false);
	}

	libusb_exit(nullptr);	//exit libusb

	delete ui;
	delete ini;
	delete image_OSLoader;
	delete image_OSLoader_System;
	delete image_System;
	delete aboutWindow;
	delete updWindow;
	delete optionsWindow;
	delete waitWindow;
}

bool startWindow::searchRecoveryModeDevice() {
	struct libusb_device* dev = nullptr;
	struct libusb_device_handle* dev_hdl = nullptr;
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
			QMessageBox::warning(this, " ", "This OS Loader file is too large to flash.");
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
			QMessageBox::warning(this, " ", "This System file is too large to flash.");
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

int startWindow::searchForDevices() {
	ui->pushButton_refresh->setDisabled(true);
	waitWindow->show();
	waitWindow->refresh();

	if (searchRecoveryModeDevice()) {
		link_mode = HOSTLINK_MODE;
	}
	else {
		//hostlink mode device not found:
		if (edb.open(EDB_MODE_BIN)) {
			link_mode = EDB_BIN;
		}
		else {
			link_mode = UNCONNECT_MODE;
		}
		edb.close();

		/*
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
		*/
	}

	switch (link_mode)
	{
	case HOSTLINK_MODE:
		ui->lineEdit_status->setText(TEXT_DEVICE_CONNECTED_HOSTLINK);
		break;
	case EDB_BIN:
		ui->lineEdit_status->setText(TEXT_DEVICE_CONNECTED_EDB_BIN);
		break;
	case UNCONNECT_MODE:
		ui->lineEdit_status->setText(TEXT_DEVICE_DISCONNECTED);
		break;
	default:
		break;
	}

	switch (link_mode)
	{
	case UNCONNECT_MODE:
		setButtonStatus(false, false, false);
		break;
	case HOSTLINK_MODE:
		setButtonStatus(true, true, true);
		break;
	case EDB_BIN:
		setButtonStatus(true, true, true);
		break;
	default:
		break;
	}

	waitWindow->hide();
	ui->pushButton_refresh->setEnabled(true);

	return link_mode;
}

void startWindow::on_pushButton_refresh_clicked()
{
	searchForDevices();
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

bool startWindow::startUpdate(const QList<int>& work)
{
	updWindow->clear();
	updWindow->setProgressBarVisible(false);
	updWindow->show();

	updWindow->addLine("Start update...");

	if (link_mode == HOSTLINK_MODE) {
		//use sbloader to send OSLoader to calculator RAM.
		TCHAR* argv[2];
		argv[0] = (TCHAR*)TEXT("-f");
		argv[1] = (TCHAR*)reinterpret_cast<const wchar_t*>(ui->OSLoader_path->text().utf16());
		updWindow->addLine("Sending OS Loader to calculator RAM...");
		int exitCode = _tmain(2, argv);
		updWindow->addLine("Finished with code " + QString::number(exitCode) + ".");
		if (exitCode != 0) {
			updWindow->addLine("ERROR: Failed to send OS Loader to calculator RAM.");
			return false;
		}

		updWindow->addLine("Reboot and reconnect... Please wait. ");
		//updWindow->addLine("Interval: " + QString::number(REBOOT_INTERVAL) + "ms");

		for (int i = 0; i < REBOOT_RETRY_TIME; i++) {
			Sleep(REBOOT_INTERVAL);	//reboot interval
			if (searchForDevices() == EDB_BIN) break;
		}

		if (link_mode != EDB_BIN) return false;

		updWindow->addLine("Device found.");
	}

	flashImg item;
	errno_t ret;
	for (int i = 0; i < work.size(); i++) {

		//open edb connection
		if (edbMode == EDB_BIN) {
			if (edb.open(EDB_MODE_BIN)) {
				updWindow->addLine("Use USB mode to flash.");
			}
			else {
				updWindow->addLine("ERROR: Can not open EDB connection.");
				return false;
			}

		}
		else if (edbMode == EDB_TEXT) {
			if (edb.open(EDB_MODE_TEXT)) {
				updWindow->addLine("Use Serial mode to flash.");
			}
			else {
				updWindow->addLine("ERROR: Can not open EDB connection.");
				return false;
			}
		}

		//set up callback function
		shared_ptr<startWindow> self = make_shared<startWindow>();
		callBack cb = bind(&startWindow::refreshStatus, self);

		switch (work.at(i))
		{
		case 1:     //update system
			memset(&item, 0, sizeof(item));
			ret = fopen_s(&item.f, ui->System_path->text().toLocal8Bit().data(), "rb");
			if (ret != 0) {
				updWindow->addLine("ERROR: Can not open file.");
				return false;
			}

			item.filename = ui->System_path->text().toLocal8Bit().data();
			item.toPage = page_System;
			item.bootImg = false;

			updWindow->addLine("Flashing system to page " + QString::number(page_System) + ".");

			/*
			edb.reset(EDB_MODE_TEXT);
			if (!edb.ping()) {
				updWindow->addLine("ERROR: Device no response.");
				return false;
			}

			edb.vm_suspend();
			edb.flash(item, cb);
			fclose(item.f);

			updWindow->addLine("Device rebooting...");
			edb.reboot();
			edb.close();
			*/
			break;
		case 2:     //update OSLoader
			memset(&item, 0, sizeof(item));

			ret = fopen_s(&item.f, ui->OSLoader_path->text().toLocal8Bit().data(), "rb");
			if (ret != 0) {
				updWindow->addLine("ERROR: Can not open file.");
				return false;
			}

			item.filename = ui->OSLoader_path->text().toLocal8Bit().data();
			item.toPage = page_OSLoader;
			item.bootImg = true;

			updWindow->addLine("Flashing OS Loader to page " + QString::number(page_OSLoader) + ".");

			/*
			edb.reset(EDB_MODE_TEXT);
			if (!edb.ping()) {
				updWindow->addLine("ERROR: Device no response.");
				return false;
			}

			edb.vm_suspend();
			edb.flash(item, cb);
			fclose(item.f);

			updWindow->addLine("Device rebooting...");
			edb.reboot();
			edb.close();
			*/
			break;
		default:
			return false;
			break;
		}

		edb.reset(EDB_MODE_TEXT);
		if (!edb.ping()) {
			updWindow->addLine("ERROR: Device no response.");
			return false;
		}

		edb.vm_suspend();
		edb.flash(item, cb);
		fclose(item.f);

		updWindow->addLine("Device rebooting...");
		edb.reboot();
		edb.close();

		for (int j = 0; j < REBOOT_RETRY_TIME; j++) {
			Sleep(REBOOT_EDB_INTERVAL);
			if (searchForDevices() == EDB_BIN) break;
		}
	}

	if (link_mode == EDB_BIN) {
		updWindow->addLine("Done.");
		return true;
	}
	else {
		return false;
	}
}

void startWindow::on_pushButton_update_O_clicked()
{
	if (ui->OSLoader_path->text() == "") {
		QMessageBox::critical(this, " ", "You have to select a file for OS Loader first.");
	}
	else {
		setButtonStatus(false, false, false);
		if (startUpdate({ UPDATE_OSLOADER })) {
			QMessageBox::information(this, " ", "Update device successfully.");
		}
		else {
			QMessageBox::critical(this, " ", "An error occurred while updating device.");
		}
	}
	updWindow->hide();
	searchForDevices();
}

void startWindow::on_pushButton_update_S_clicked()
{
	if (link_mode == HOSTLINK_MODE) {
		if (ui->OSLoader_path->text() == "") {
			QMessageBox::information(this, " ", "In HostLink mode,\nyou need to select an OS Loader file for updating.");
			return;
		}
	}

	if (ui->System_path->text() == "") {
		QMessageBox::critical(this, " ", "You have to select a file for System first.");
	}
	else {
		setButtonStatus(false, false, false);
		if (startUpdate({ UPDATE_SYSTEM })) {
			QMessageBox::information(this, " ", "Update device successfully.");
		}
		else {
			QMessageBox::critical(this, " ", "An error occurred while updating device.");
		}
	}
	updWindow->hide();
	searchForDevices();
}

void startWindow::on_pushButton_update_OandS_clicked()
{
	if (ui->OSLoader_path->text() == "") {
		QMessageBox::critical(this, " ", "You have to select a file for OS Loader first.");
	}
	else {
		if (ui->System_path->text() == "") {
			QMessageBox::critical(this, " ", "You have to select a file for System first.");
		}
		else {
			setButtonStatus(false, false, false);
			if (startUpdate({ UPDATE_OSLOADER, UPDATE_SYSTEM })) {
				QMessageBox::information(this, " ", "Update device successfully.");
			}
			else {
				QMessageBox::critical(this, " ", "An error occurred while updating device.");
				
			}
		}
	}
	updWindow->hide();
	searchForDevices();
}

void startWindow::refreshStatus() {
	updWindow->clear();
	//updWindow->setProgressBarVisible(true);

	//updWindow->setProgressBarValue(12);
	//if (uploadedSize / fsize > 0.5)updWindow->setProgressBarValue(24);
	//updWindow->repaint();
	//updWindow->setProgressBarValue(100);
	int per = int((double(uploadedSize) / double(fsize)) * 100);
	updWindow->setProgressBarValue(per);
	updWindow->setWindowTitle("Flashing...");
	updWindow->addLine(
		"\n================================\nSpeed: " +
		QString::number(speed) + "KB/s\n" +
		"Uploaded Size: " + QString::number(uploadedSize) + "\n" +
		"Page: " + QString::number(pageNow) + "\n" +
		"Block: " + QString::number(blockNow) +
		"\n================================");

	//updWindow->setProgressBarValue((uploadedSize / fsize) * 100);

	updWindow->refresh();
	/*
	QMessageBox::information(this, " ", "Speed: " + QString::number(speed) + "KB/s\n" +
		"Uploaded Size: " + QString::number(uploadedSize) + "\n" +
		"Page: " + QString::number(pageNow) + "\n" +
		"Block: " + QString::number(blockNow));
	*/
	
}