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
	ui->pushButton_reboot->setDisabled(true);

	QFileInfo ini_file;
	ini_file.setFile("config.ini");

	if (!ini_file.exists()) {

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
			QFileInfo tmp;
			path.append(ini->value(tr("/Path/osloader")).toString());
			path.append(ini->value(tr("/Path/system")).toString());

			if (path.at(0) != "") {
				tmp.setFile(path.at(0));
				if (tmp.exists()) {
					ui->OSLoader_path->setText(path.at(0));
					size_OSLoader = tmp.size();
				}
			}
			if (path.at(1) != "") {
				tmp.setFile(path.at(1));
				if (tmp.exists()) {
					ui->System_path->setText(path.at(1));
					size_System = tmp.size();
				}
			}

			ui->checkBox_remember_path->setChecked(true);
		}
	}

	connect(optionsWindow, SIGNAL(returnData(int, int)), this, SLOT(getReturnData(int, int)));

	updWindow->moveToThread(&deviceUpdateThread);

	connect(updWindow, SIGNAL(sendUpdateStatus(int)), this, SLOT(getUpdateStatus(int)));

	refresh_timer->setInterval(REFRESH_INTERVAL);

	connect(refresh_timer, SIGNAL(timeout()), this, SLOT(refreshLinkStatus()));

	refresh_timer->start();

	flashInfoWindow->moveToThread(&infoWindowThread);

	//flashInfoWindow->show();
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

	delete ui;
	delete ini;
	delete refresh_timer;
	delete image_OSLoader;
	delete image_OSLoader_System;
	delete image_System;
	delete aboutWindow;
	delete updWindow;
	delete optionsWindow;
	delete flashInfoWindow;
}



void startWindow::on_button_OSLoader_path_clicked()
{
	ui->OSLoader_path->setText(QFileDialog::getOpenFileName(this, tr("Select a file"), tr(""), tr("OSLoader File(*.sb)")));
	if (ui->OSLoader_path->text() != "") {
		QFileInfo* info = new QFileInfo(ui->OSLoader_path->text());
		if (info->size() > (page_System - page_OSLoader) * 2112) {
			QMessageBox::warning(this, " ", "This OSLoader file is too large to flash.");
			ui->OSLoader_path->clear();
		}
		else {
			size_OSLoader = info->size();
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
		else {
			size_System = info->size();
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
	optionsWindow->set(page_OSLoader, page_System);
	optionsWindow->exec();
}

void startWindow::refreshLinkStatus() {
	setStatus(updWindow->searchForDevices());
}

void startWindow::setButtonStatus(const bool& O, const bool& S, const bool& OandS)
{
	ui->pushButton_update_O->setEnabled(O);
	ui->pushButton_update_OandS->setEnabled(S);
	ui->pushButton_update_S->setEnabled(OandS);
}

void startWindow::getReturnData(int OSLoader, int System) {
	page_OSLoader = OSLoader;
	page_System = System;
}

void startWindow::on_pushButton_update_O_clicked()
{
	if (ui->OSLoader_path->text() == "") {
		QMessageBox::critical(this, " ", "You have to select a file for OSLoader first.");
	}
	else {
		setButtonStatus(false, false, false);
		updateDevice({ UPDATE_OSLOADER });
	}
	updWindow->hide();
}

void startWindow::on_pushButton_update_S_clicked()
{
	if (link_mode == HOSTLINK_MODE) {
		if (ui->OSLoader_path->text() == "") {
			QMessageBox::information(this, " ", "In HostLink mode, you need to select an OSLoader file for updating.");
			return;
		}
	}

	if (ui->System_path->text() == "") {
		QMessageBox::critical(this, " ", "You have to select a file for System first.");
	}
	else {
		setButtonStatus(false, false, false);
		updateDevice({ UPDATE_SYSTEM });
	}
	updWindow->hide();
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
			setButtonStatus(false, false, false);
			updateDevice({ UPDATE_OSLOADER, UPDATE_SYSTEM });
		}
	}
	updWindow->hide();
}

void startWindow::setStatus(int mode){
	link_mode = mode;
	switch (mode)
	{
	case HOSTLINK_MODE:
		ui->lineEdit_status->setText(TEXT_DEVICE_CONNECTED_HOSTLINK);
		ui->pushButton_reboot->setDisabled(true);
		setButtonStatus(true, true, true);
		break;
	case EDB_BIN:
		ui->lineEdit_status->setText(TEXT_DEVICE_CONNECTED_EDB_BIN);
		if (updateStatus == UPDATE_NONE) {
			ui->pushButton_reboot->setEnabled(true);
		}
		setButtonStatus(true, true, true);
		break;
	case UNCONNECT_MODE:
		ui->lineEdit_status->setText(TEXT_DEVICE_DISCONNECTED);
		ui->pushButton_reboot->setDisabled(true);
		setButtonStatus(false, false, false);
		break;
	default:
		break;
	}
}

void startWindow::updateDevice(const QList<int>& work) {
	ui->pushButton_reboot->setDisabled(true);
	setButtonStatus(false, false, false);

	flashInfoWindow->timer_start(10);
	flashInfoWindow->move(this->x() + 330, this->y());
	updWindow->move(this->x() + 330, this->y() + 260);

	updateStatus = UPDATE_PROCESSING;
	if (updWindow->startUpdate(work, ui->OSLoader_path->text(), ui->System_path->text(), page_OSLoader, page_System)) {
		QMessageBox::information(this, " ", "Update device successfully.\n\nIt is now safe and recommended to disconnect your calculator.");
	}
	else {
		QMessageBox::critical(this, " ", "Update device failed.\n\nAn error occurred while updating device.");
	}
	flashInfoWindow->timer_stop();
	flashInfoWindow->hide();
}

void startWindow::on_pushButton_reboot_clicked() {
	if (updWindow->reboot()) {
		ui->pushButton_reboot->setDisabled(true);
	}
	else {
		QMessageBox::critical(this, " ", "Command send failed.\n\nPlease check your connection.");
	}
}

void startWindow::getUpdateStatus(int status) {
	updateStatus = status;
	int tmp_size;
	switch (updateStatus)
	{
	case UPDATE_FLASHING_OSLOADER:
		tmp_size = size_OSLoader;
		break;

	case UPDATE_FLASHING_SYSTEM:
		tmp_size = size_System;
		break;

	default:
		tmp_size = 1;
		break;
	}
	flashInfoWindow->refreshBasicInfo(flashInfoText.at(updateStatus), tmp_size);
	if (updateStatus == UPDATE_NONE || updateStatus == UPDATE_PROCESSING) {
		if(flashInfoWindow->isVisible()) flashInfoWindow->hide();
	}
	else {
		if (flashInfoWindow->isHidden()) flashInfoWindow->show();
	}

}