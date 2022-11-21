#include "updatewindow.h"
#include "ui_updatewindow.h"

updateWindow::updateWindow(QWidget* parent) :
	QDialog(parent),
	ui(new Ui::updateWindow)
{
	ui->setupUi(this);
	setWindowTitle("Update Dialog");
	setWindowFlag(Qt::SubWindow);
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

	bool isOK = false;

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
			isOK = false;
			goto update_end;
		}

		this->addLine("Reboot and reconnect... Please wait. ");
		//this->addLine("Interval: " + QString::number(REBOOT_INTERVAL) + "ms");

		for (int i = 0; i < REBOOT_RETRY_TIME; i++) {
			searchForDevices();
			if (link_mode == EDB_BIN) break;
			Sleep(REBOOT_INTERVAL);	//reboot interval

		}

		if (link_mode != EDB_BIN) {
			isOK = false;
			goto update_end;
		}

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
			isOK = false;
			goto update_end;
		}

		//set up callback function
		shared_ptr<updateWindow> self = make_shared<updateWindow>();
		callBack cb = bind(&updateWindow::refreshStatus, self);

		switch (work.at(i))
		{
		case UPDATE_SYSTEM:     //update system
			memset(&item, 0, sizeof(item));
			ret = fopen_s(&item.f, System_path.toLocal8Bit().data(), "rb");
			if (ret != 0) {
				this->addLine("ERROR: Can not open file.");
				isOK = false;
				goto update_end;
			}

			item.filename = System_path.toLocal8Bit().data();
			item.toPage = page_System;
			item.bootImg = false;

			this->addLine("Flashing system to page " + QString::number(page_System) + ".");
			break;
		case UPDATE_OSLOADER:     //update OSLoader
			memset(&item, 0, sizeof(item));

			ret = fopen_s(&item.f, OSLoader_path.toLocal8Bit().data(), "rb");
			if (ret != 0) {
				this->addLine("ERROR: Can not open file.");
				isOK = false;
				goto update_end;
			}

			item.filename =OSLoader_path.toLocal8Bit().data();
			item.toPage = page_OSLoader;
			item.bootImg = true;

			this->addLine("Flashing OS Loader to page " + QString::number(page_OSLoader) + ".");

			break;
		default:
			isOK = false;
			goto update_end;
			break;
		}

		if (link_mode == EDB_BIN) edb.reset(EDB_MODE_TEXT);
		if (!edb.ping()) {
			this->addLine("ERROR: Device no response.");
			isOK = false;
			goto update_end;
		}
		
		edb.vm_suspend();
		edb.flash(item, cb);
		std::fclose(item.f);

		this->addLine("Device rebooting...");
		edb.reboot();
		edb.close();

		for (int j = 0; j < REBOOT_RETRY_TIME; j++) {
			searchForDevices();
			if (link_mode == EDB_BIN) break;
			Sleep(REBOOT_EDB_INTERVAL);
		}
	}

	if (link_mode == EDB_BIN) {
		this->addLine("Done.");
		isOK = true;
		goto update_end;
	}
	else {
		isOK = false;
		goto update_end;
	}

update_end:
	emit sendUpdateStatus(UPDATE_NONE);
	return isOK;
}

int updateWindow::searchDevices() {
	if (libusb_init(nullptr) != 0) {
		QMessageBox::critical(this, " ", "Can not init libusb!");
		return UNCONNECT_MODE;
	}

	int device_count = 0;
	libusb_device** device_list;
	libusb_device* device_now;
	struct libusb_device_descriptor desc;

	device_count = libusb_get_device_list(nullptr, &device_list);

	if (device_count < 0) return false;

	for (int i = 0; i < device_count; i++) {
		device_now = device_list[i];
		libusb_get_device_descriptor(device_now, &desc);

		if (desc.idVendor == HOSTLINK_VID && desc.idProduct == HOSTLINK_PID)	return HOSTLINK_MODE;
		if (desc.idVendor == EOS_VID && desc.idProduct == EOS_PID)				return EDB_BIN;

	}

	return UNCONNECT_MODE;
}


int updateWindow::searchForDevices() {
	link_mode = searchDevices();
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

bool updateWindow::reboot() {
	if (edb.open(1)) {
		edb.reboot();
		edb.close(); 
		return true;
	}
	else {
		return false;
	}
}