#include "updatewindow.h"
#include "ui_updatewindow.h"

updateWindow::updateWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::updateWindow)
{
    ui->setupUi(this);
    setWindowTitle("Update Dialog");
    setWindowFlag(Qt::SubWindow);

	waitWindow->moveToThread(&waitThread);
}

updateWindow::~updateWindow()
{
    delete ui;
}

void updateWindow::addLine(const QString &text){
    time_t currentTime;
    struct tm* pstTime;
    char timeString[32];
    time(&currentTime);
    pstTime = localtime(&currentTime);
    strftime(timeString, 32, "%H:%M:%S", pstTime);
    ui->textEdit->setText(ui->textEdit->toPlainText() + "["+ timeString + "] " + text + "\n");
    ui->textEdit->moveCursor(QTextCursor::End);
    qApp->processEvents();
}

void updateWindow::clear()
{
    ui->textEdit->clear();
}

void updateWindow::refresh() {
    qApp->processEvents();
    this->show();
}

void updateWindow::setProgressBarVisible(bool isVisible) {
    ui->progressBar->setVisible(isVisible);
}

void updateWindow::setProgressBarValue(int n) {
    ui->progressBar->setValue(n);
}

bool updateWindow::startUpdate(const QList<int>& work, const QString &OSLoader_path, const QString& System_path, const int& page_OSLoader, const int& page_System)
{
	this->clear();
	this->setProgressBarVisible(false);
	this->show();

	this->addLine("Start update...");

	if (link_mode == HOSTLINK_MODE) {
		//use sbloader to send OSLoader to calculator RAM.
		TCHAR* argv[2];
		argv[0] = (TCHAR*)TEXT("-f");
		argv[1] = (TCHAR*)reinterpret_cast<const wchar_t*>(OSLoader_path.utf16());
		this->addLine("Sending OS Loader to calculator RAM...");
		int exitCode = _tmain(2, argv);
		this->addLine("Finished with code " + QString::number(exitCode) + ".");
		if (exitCode != 0) {
			this->addLine("ERROR: Failed to send OS Loader to calculator RAM.");
			return false;
		}

		this->addLine("Reboot and reconnect... Please wait. ");
		//this->addLine("Interval: " + QString::number(REBOOT_INTERVAL) + "ms");

		for (int i = 0; i < REBOOT_RETRY_TIME; i++) {
			Sleep(REBOOT_INTERVAL);	//reboot interval
			if (searchForDevices() == EDB_BIN) break;
		}

		if (link_mode != EDB_BIN) return false;

		this->addLine("Device found.");
	}

	flashImg item;
	errno_t ret;
	for (int i = 0; i < work.size(); i++) {

		//open edb connection
		if (edb.open(EDB_MODE_BIN)) {
			this->addLine("Use USB mode to flash.");
		}
		else {
			this->addLine("ERROR: Can not open EDB connection.");
			return false;
		}

		//set up callback function
		shared_ptr<updateWindow> self = make_shared<updateWindow>();
		callBack cb = bind(&updateWindow::refreshStatus, self);

		switch (work.at(i))
		{
		case 1:     //update system
			memset(&item, 0, sizeof(item));
			ret = fopen_s(&item.f, System_path.toLocal8Bit().data(), "rb");
			if (ret != 0) {
				this->addLine("ERROR: Can not open file.");
				return false;
			}

			item.filename = System_path.toLocal8Bit().data();
			item.toPage = page_System;
			item.bootImg = false;

			this->addLine("Flashing system to page " + QString::number(page_System) + ".");
			break;
		case 2:     //update OSLoader
			memset(&item, 0, sizeof(item));

			ret = fopen_s(&item.f, OSLoader_path.toLocal8Bit().data(), "rb");
			if (ret != 0) {
				this->addLine("ERROR: Can not open file.");
				return false;
			}

			item.filename =OSLoader_path.toLocal8Bit().data();
			item.toPage = page_OSLoader;
			item.bootImg = true;

			this->addLine("Flashing OS Loader to page " + QString::number(page_OSLoader) + ".");

			break;
		default:
			return false;
			break;
		}

		if (link_mode == EDB_BIN) edb.reset(EDB_MODE_TEXT);
		if (!edb.ping()) {
			this->addLine("ERROR: Device no response.");
			return false;
		}

		edb.vm_suspend();
		edb.flash(item, cb);
		fclose(item.f);

		this->addLine("Device rebooting...");
		edb.reboot();
		edb.close();

		for (int j = 0; j < REBOOT_RETRY_TIME; j++) {
			Sleep(REBOOT_EDB_INTERVAL);
			if (searchForDevices() == EDB_BIN) break;
		}
	}

	if (link_mode == EDB_BIN) {
		this->addLine("Done.");
		return true;
	}
	else {
		return false;
	}
}

bool updateWindow::searchRecoveryModeDevice() {
	//init libusb
	if (libusb_init(nullptr) != 0) {
		QMessageBox::critical(this, " ", "Can not init libusb!");
	}

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
	libusb_exit(nullptr);	//exit libusb
	return isFound;
}

int updateWindow::searchForDevices() {
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
	}

	waitWindow->hide();
	//ui->pushButton_refresh->setEnabled(true);

	return link_mode;
}

void updateWindow::refreshStatus() {
	this->clear();
	int per = int((double(uploadedSize) / double(fsize)) * 100);
	this->setProgressBarValue(per);
	this->setWindowTitle("Flashing...");
	this->addLine(
		"\n================================\nSpeed: " +
		QString::number(speed) + "KB/s\n" +
		"Uploaded Size: " + QString::number(uploadedSize) + "\n" +
		"Page: " + QString::number(pageNow) + "\n" +
		"Block: " + QString::number(blockNow) +
		"\n================================");
	this->refresh();

}