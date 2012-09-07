/*!	\file czdialog.cpp
	\brief Main window implementation source file.
	\author Andriy Golovnya <andrew_golovnia@ukr.net> http://ag.embedded.org.ru/
	\url http://cuda-z.sf.net/ http://sf.net/projects/cuda-z/
	\license GPLv2 http://www.gnu.org/licenses/gpl-2.0.html
*/

#include <QMenu>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QDesktopServices>

#include <time.h>

#include "log.h"
#include "czdialog.h"
#include "version.h"

#define CZ_TIMER_REFRESH	2000	/*!< Test results update timer period (ms). */

/*!	\def CZ_OS_PLATFORM_STR
	\brief Platform ID string.
*/
#if defined(Q_OS_WIN)
#define CZ_OS_PLATFORM_STR	"win32"
#elif defined(Q_OS_MAC)
#define CZ_OS_PLATFORM_STR	"macosx"
#elif defined(Q_OS_LINUX)
#define CZ_OS_PLATFORM_STR	"linux"
#else
#error Your platform is not supported by CUDA! Or it does but I know nothing about this...
#endif

/*!	\name Update progress icons definitions.
*/
/*@{*/
#define CZ_UPD_ICON_INFO	":/img/upd-info.png"
#define CZ_UPD_ICON_WARNING	":/img/upd-warning.png"
#define CZ_UPD_ICON_ERROR	":/img/upd-error.png"
#define CZ_UPD_ICON_DOWNLOAD	":/img/upd-download.png"
#define CZ_UPD_ICON_DOWNLOAD_CR	":/img/upd-download-critical.png"
/*@}*/

/*!	\class CZSplashScreen
	\brief Splash screen with multiline logging effect.
*/

/*!	\brief Creates a new #CZSplashScreen and initializes internal
	parameters of the class.
*/
CZSplashScreen::CZSplashScreen(
	const QPixmap &pixmap,	/*!<[in] Picture for window background. */
	int maxLines,		/*!<[in] Number of lines in boot log. */
	Qt::WindowFlags f	/*!<[in] Window flags. */
):	QSplashScreen(pixmap, f),
	m_maxLines(maxLines) {
	m_message = QString::null;
	m_lines = 0;
	m_alignment = Qt::AlignLeft;
	m_color = Qt::black;
}

/*!	\brief Creates a new #CZSplashScreen with the given \a parent and
	initializes internal parameters of the class.
*/
CZSplashScreen::CZSplashScreen(
	QWidget *parent,	/*!<[in,out] Parent of widget. */
	const QPixmap &pixmap,	/*!<[in] Picture for window background. */
	int maxLines,		/*!<[in] Number of lines in boot log. */
	Qt::WindowFlags f	/*!<[in] Window flags. */
):	QSplashScreen(parent, pixmap, f),
	m_maxLines(maxLines) {
	m_message = QString::null;
	m_lines = 0;
	m_alignment = Qt::AlignLeft;
	m_color = Qt::black;
}

/*!	\brief Class destructor.
*/
CZSplashScreen::~CZSplashScreen() {
}

/*!	\brief Sets the maximal number of lines in log.
*/
void CZSplashScreen::setMaxLines(
	int maxLines		/*!<[in] Number of lines in log. */
) {
	if(maxLines >= 1) {
		m_maxLines = maxLines;
		if(m_lines > m_maxLines) {
			deleteTop(m_lines - m_maxLines);
			QSplashScreen::showMessage(m_message, m_alignment, m_color);
		}
	}
}

/*!	\brief Returns the maximal number of lines in log.
	\return number of lines in log.
*/
int CZSplashScreen::maxLines() {
	return m_maxLines;
}

/*!	\brief Adds new message line in log.
*/
void CZSplashScreen::showMessage(
	const QString &message,	/*!<[in] Message text. */
	int alignment,		/*!<[in] Placement of log in window. */
	const QColor &color	/*!<[in] Color used for protocol display. */
) {

	m_alignment = alignment;
	m_color = color;

	if(m_message.size() != 0) {
		m_message += '\n' + message;
	} else {
		m_message = message;
	}
	QStringList linesList = m_message.split('\n');
	m_lines = linesList.size();

	if(m_lines > m_maxLines) {
		deleteTop(m_lines - m_maxLines);
	}

	QSplashScreen::showMessage(m_message, m_alignment, m_color);
}

/*!	\brief Removes all messages being displayed in log.
*/
void CZSplashScreen::clearMessage() {
	m_message = QString::null;
	m_lines = 0;
	QSplashScreen::showMessage(m_message, m_alignment, m_color);
}

/*!	\brief Removes first \a lines entries in log.
*/
void CZSplashScreen::deleteTop(
	int lines		/*!<[in] Number of lines to be removed. */
) {
	QStringList linesList = m_message.split('\n');
	for(int i = 0; i < lines; i++) {
		linesList.removeFirst();
	}

	m_message = linesList.join(QString('\n'));
	m_lines -= lines;
}

/*!	\brief Splash screen of application.
*/
CZSplashScreen *splash;

/*!	\class CZDialog
	\brief This class implements main window of the application.
*/

/*!	\brief Creates a new #CZDialog with the given \a parent.
	This function does following steps:
	- Sets up GUI.
	- Setup CUDA-device information containers and add them in list.
	- Sets up connections.
	- Fills up data in to tabs of GUI.
	- Starts Performance data update timer.
*/
CZDialog::CZDialog(
	QWidget *parent,	/*!<[in,out] Parent of widget. */
	Qt::WFlags f		/*!<[in] Window flags. */
)	: QDialog(parent, f /*| Qt::MSWindowsFixedSizeDialogHint*/ | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint) {

	setupUi(this);
	this->setWindowTitle(QString("%1 %2").arg(CZ_NAME_SHORT).arg(CZ_VERSION));
	connect(comboDevice, SIGNAL(activated(int)), SLOT(slotShowDevice(int)));

	QMenu *exportMenu = new QMenu(pushExport);
	exportMenu->addAction(tr("to &Text"), this, SLOT(slotExportToText()));
	exportMenu->addAction(tr("to &HTML"), this, SLOT(slotExportToHTML()));
	pushExport->setMenu(exportMenu);
	
	readCudaDevices();
	setupDeviceList();
	setupDeviceInfo(comboDevice->currentIndex());
	setupAboutTab();

	updateTimer = new QTimer(this);
	connect(updateTimer, SIGNAL(timeout()), SLOT(slotUpdateTimer()));
	updateTimer->start(CZ_TIMER_REFRESH);

	labelAppUpdateImg->setPixmap(QPixmap(CZ_UPD_ICON_INFO));
	labelAppUpdate->setText(tr("Looking for new version..."));
	startGetHistoryHttp();
}

/*!	\brief Class destructor.
	This function makes class data cleanup actions.
*/
CZDialog::~CZDialog() {
	updateTimer->stop();
	delete updateTimer;
	freeCudaDevices();
	cleanGetHistoryHttp();
}

/*!	\brief Reads CUDA devices information.
	For each of detected CUDA-devices does following:
	- Initialize CUDA-data structure.
	- Reads CUDA-information about device.
	- Shows progress message in splash screen.
	- Starts Performance calculation procedure.
	- Appends entry in to device-list.
*/
void CZDialog::readCudaDevices() {

	int num = getCudaDeviceNumber();

	for(int i = 0; i < num; i++) {

		CZCudaDeviceInfo *info = new CZCudaDeviceInfo(i);

		if(info->info().major != 0) {
			splash->showMessage(tr("Getting information about %1 ...").arg(info->info().deviceName),
				Qt::AlignLeft | Qt::AlignBottom);
			qApp->processEvents();

//			wait(10000000);

			info->waitPerformance();
			
			connect(info, SIGNAL(testedPerformance(int)), SLOT(slotUpdatePerformance(int)));
			deviceList.append(info);
		}
	}
}

/*!	\brief Cleans up after bandwidth tests.
*/
void CZDialog::freeCudaDevices() {

	while(deviceList.size() > 0) {
		CZCudaDeviceInfo *info = deviceList[0];
		deviceList.removeFirst();
		delete info;
	}
}

/*!	\brief Gets number of CUDA devices.
	\return number of CUDA-devices in case of success, \a 0 if no CUDA-devies were found.
*/
int CZDialog::getCudaDeviceNumber() {
	return CZCudaDeviceFound();
}

/*!	\brief Puts devices in combo box.
*/
void CZDialog::setupDeviceList() {
	comboDevice->clear();

	for(int i = 0; i < deviceList.size(); i++) {
		comboDevice->addItem(QString("%1: %2").arg(i).arg(deviceList[i]->info().deviceName));
	}
}

/*!	\brief This slot shows in dialog information about given device.
*/
void CZDialog::slotShowDevice(
	int index			/*!<[in] Index of device in list. */
) {
	setupDeviceInfo(index);
	if(checkUpdateResults->checkState() == Qt::Checked) {
		CZLog(CZLogLevelModerate, "Switch device -> update performance for device %d", index);
		deviceList[index]->testPerformance(index);
	}
}

/*!	\brief This slot updates performance information of device
	pointed by \a index.
*/
void CZDialog::slotUpdatePerformance(
	int index			/*!<[in] Index of device in list. */
) {
	if(index == comboDevice->currentIndex())
	setupPerformanceTab(deviceList[index]->info());
}

/*!	\brief This slot updates performance information of current device
	every timer tick.
*/
void CZDialog::slotUpdateTimer() {

	int index = comboDevice->currentIndex();
	if(checkUpdateResults->checkState() == Qt::Checked) {
		if(checkHeavyMode->checkState() == Qt::Checked) {
			deviceList[index]->info().heavyMode = 1;
		} else {
			deviceList[index]->info().heavyMode = 0;
		}
		CZLog(CZLogLevelModerate, "Timer shot -> update performance for device %d in mode %d", index, deviceList[index]->info().heavyMode);
		deviceList[index]->testPerformance(index);
	} else {
		CZLog(CZLogLevelModerate, "Timer shot -> update ignored");
	}
}

/*!	\brief Places in dialog's tabs information about given device.
*/
void CZDialog::setupDeviceInfo(
	int dev				/*!<[in] Number/index of CUDA-device. */
) {
	setupCoreTab(deviceList[dev]->info());
	setupMemoryTab(deviceList[dev]->info());
	setupPerformanceTab(deviceList[dev]->info());
}

/*!	\brief Fill tab "Core" with CUDA devices information.
*/
void CZDialog::setupCoreTab(
	struct CZDeviceInfo &info	/*!<[in] Information about CUDA-device. */
) {
	QString deviceName(info.deviceName);

	labelNameText->setText(deviceName);
	labelCapabilityText->setText(QString("%1.%2").arg(info.major).arg(info.minor));
	labelClockText->setText(getValue1000(info.core.clockRate, prefixKilo, tr("Hz")));
	if(info.core.muliProcCount == 0)
		labelMultiProcText->setText("<i>" + tr("Unknown") + "</i>");
	else
		labelMultiProcText->setNum(info.core.muliProcCount);
	labelWarpText->setNum(info.core.SIMDWidth);
	labelRegsText->setNum(info.core.regsPerBlock);
	labelThreadsText->setNum(info.core.maxThreadsPerBlock);
	if(info.core.watchdogEnabled == -1)
		labelWatchdogText->setText("<i>" + tr("Unknown") + "</i>");
	else
		labelWatchdogText->setText(info.core.watchdogEnabled? tr("Yes"): tr("No"));
	labelIntegratedText->setText(info.core.integratedGpu? tr("Yes"): tr("No"));
	labelConcurrentKernelsText->setText(info.core.concurrentKernels? tr("Yes"): tr("No"));
	if(info.core.computeMode == CZComputeModeDefault) {
		labelComputeModeText->setText(tr("Default"));
	} else if(info.core.computeMode == CZComputeModeExclusive) {
		labelComputeModeText->setText(tr("Compute-exclusive"));
	} else if(info.core.computeMode == CZComputeModeProhibited) {
		labelComputeModeText->setText(tr("Compute-prohibited"));
	} else {
		labelComputeModeText->setText("<i>" + tr("Unknown") + "</i>");
	}

	labelThreadsDimText->setText(QString("%1 x %2 x %3").arg(info.core.maxThreadsDim[0]).arg(info.core.maxThreadsDim[1]).arg(info.core.maxThreadsDim[2]));
	labelGridDimText->setText(QString("%1 x %2 x %3").arg(info.core.maxGridSize[0]).arg(info.core.maxGridSize[1]).arg(info.core.maxGridSize[2]));

	labelDeviceLogo->setPixmap(QPixmap(":/img/logo-unknown.png"));
	if(deviceName.contains("tesla", Qt::CaseInsensitive)) {
		labelDeviceLogo->setPixmap(QPixmap(":/img/logo-tesla.png"));
	} else
	if(deviceName.contains("quadro", Qt::CaseInsensitive)) {
		labelDeviceLogo->setPixmap(QPixmap(":/img/logo-quadro.png"));
	} else
	if(deviceName.contains("ion", Qt::CaseInsensitive)) {
		labelDeviceLogo->setPixmap(QPixmap(":/img/logo-ion.png"));
	} else
	if(deviceName.contains("geforce", Qt::CaseInsensitive)) {
		labelDeviceLogo->setPixmap(QPixmap(":/img/logo-geforce.png"));
	}

	QString version;
	if(strlen(info.drvVersion) != 0) {
		version = " (" + QString(info.drvVersion) + ")";
	} else {
		version = "<i>" + tr("Unknown") + "</i>";
	}
	labelDrvVersionText->setText(info.drvVersion);

	if(info.drvDllVer == 0) {
		version = "<i>" + tr("Unknown") + "</i>";
	} else {
		version = QString("%1.%2")
			.arg(info.drvDllVer / 1000)
			.arg(info.drvDllVer % 1000);
	}
	if(strlen(info.drvDllVerStr) != 0) {
		version += " (" + QString(info.drvDllVerStr) + ")";
	}
	labelDrvDllVersionText->setText(version);

	if(info.rtDllVer == 0) {
		version = "<i>" + tr("Unknown") + "</i>";
	} else {
		version = QString("%1.%2")
			.arg(info.rtDllVer / 1000)
			.arg(info.rtDllVer % 1000);
	}
	if(strlen(info.rtDllVerStr) != 0) {
		version += " (" + QString(info.rtDllVerStr) + ")";
	}
	labelRtDllVersionText->setText(version);
}

/*!	\brief Fill tab "Memory" with CUDA devices information.
*/
void CZDialog::setupMemoryTab(
	struct CZDeviceInfo &info	/*!<[in] Information about CUDA-device. */
) {
	labelTotalGlobalText->setText(getValue1024(info.mem.totalGlobal, prefixNothing, tr("B")));
	labelSharedText->setText(getValue1024(info.mem.sharedPerBlock, prefixNothing, tr("B")));
	labelPitchText->setText(getValue1024(info.mem.maxPitch, prefixNothing, tr("B")));
	labelTotalConstText->setText(getValue1024(info.mem.totalConst, prefixNothing, tr("B")));
	labelTextureAlignmentText->setText(getValue1024(info.mem.textureAlignment, prefixNothing, tr("B")));
	labelTexture1DText->setText(QString("%1")
		.arg((double)info.mem.texture1D[0]));
	labelTexture2DText->setText(QString("%1 x %2")
		.arg((double)info.mem.texture2D[0])
		.arg((double)info.mem.texture2D[1]));
	labelTexture3DText->setText(QString("%1 x %2 x %3")
		.arg((double)info.mem.texture3D[0])
		.arg((double)info.mem.texture3D[1])
		.arg((double)info.mem.texture3D[2]));
	labelGpuOverlapText->setText(info.mem.gpuOverlap? tr("Yes"): tr("No"));
	labelMapHostMemoryText->setText(info.mem.mapHostMemory? tr("Yes"): tr("No"));
	labelErrorCorrectionText->setText(info.mem.errorCorrection? tr("Yes"): tr("No"));
}

/*!	\brief Fill tab "Performance" with CUDA devices information.
*/
void CZDialog::setupPerformanceTab(
	struct CZDeviceInfo &info	/*!<[in] Information about CUDA-device. */
) {

	if(info.band.copyHDPin == 0)
		labelHDRatePinText->setText("--");
	else
		labelHDRatePinText->setText(getValue1024(info.band.copyHDPin, prefixKibi, tr("B/s")));

	if(info.band.copyHDPage == 0)
		labelHDRatePageText->setText("--");
	else
		labelHDRatePageText->setText(getValue1024(info.band.copyHDPage, prefixKibi, tr("B/s")));

	if(info.band.copyDHPin == 0)
		labelDHRatePinText->setText("--");
	else
		labelDHRatePinText->setText(getValue1024(info.band.copyDHPin, prefixKibi, tr("B/s")));

	if(info.band.copyDHPage == 0)
		labelDHRatePageText->setText("--");
	else
		labelDHRatePageText->setText(getValue1024(info.band.copyDHPage, prefixKibi, tr("B/s")));

	if(info.band.copyDD == 0)
		labelDDRateText->setText("--");
	else
		labelDDRateText->setText(getValue1024(info.band.copyDD, prefixKibi, tr("B/s")));

	if(info.perf.calcFloat == 0)
		labelFloatRateText->setText("--");
	else
		labelFloatRateText->setText(getValue1000(info.perf.calcFloat, prefixKilo, tr("flop/s")));

	if(((info.major > 1)) ||
		((info.major == 1) && (info.minor >= 3))) {
		if(info.perf.calcDouble == 0)
			labelDoubleRateText->setText("--");
		else
			labelDoubleRateText->setText(getValue1000(info.perf.calcDouble, prefixKilo, tr("flop/s")));
	} else {
		labelDoubleRateText->setText("<i>" + tr("Not Supported") + "</i>");
	}

	if(info.perf.calcInteger32 == 0)
		labelInt32RateText->setText("--");
	else
		labelInt32RateText->setText(getValue1000(info.perf.calcInteger32, prefixKilo, tr("iop/s")));

	if(info.perf.calcInteger24 == 0)
		labelInt24RateText->setText("--");
	else
		labelInt24RateText->setText(getValue1000(info.perf.calcInteger24, prefixKilo, tr("iop/s")));
}

/*!	\brief Fill tab "About" with information about this program.
*/
void CZDialog::setupAboutTab() {
	labelAppName->setText(QString("<b><font size=\"+2\">%1</font></b>")
		.arg(CZ_NAME_LONG));

	QString version = QString("<b>%1</b> %2").arg(tr("Version")).arg(CZ_VERSION);
#ifdef CZ_VER_STATE
	version += QString("<br /><b>%1</b> %2 %3").arg(tr("Built")).arg(CZ_DATE).arg(CZ_TIME);
#endif//CZ_VER_STATE
	labelAppVersion->setText(version);
	labelAppURL->setText(QString("<b>%1:</b> <a href=\"%2\">%2</a><br /><b>%3:</b> <a href=\"%4\">%4</a>")
		.arg(tr("Main page")).arg(CZ_ORG_URL_MAINPAGE)
		.arg(tr("Project page")).arg(CZ_ORG_URL_PROJECT));
	labelAppAuthor->setText(QString("<b>%1</b> %2").arg(tr("Author")).arg(CZ_ORG_NAME));
	labelAppCopy->setText(CZ_COPY_INFO);
}

/*!	\fn CZDialog::getOSVersion
	\brief Get OS version string.
	\return string that describes version of OS we running at.
*/
#if defined(Q_OS_WIN)
#include <windows.h>
typedef BOOL (WINAPI *IsWow64Process_t)(HANDLE, PBOOL);

QString CZDialog::getOSVersion() {
	QString OSVersion = "Windows";

	BOOL is_os64bit = FALSE;
	IsWow64Process_t p_IsWow64Process = (IsWow64Process_t)GetProcAddress(GetModuleHandle(TEXT("kernel32")), "IsWow64Process");
	if(p_IsWow64Process != NULL) {
		if(!p_IsWow64Process(GetCurrentProcess(), &is_os64bit)) {
			is_os64bit = FALSE;
	        }
	}

	OSVersion += QString(" %1").arg(
		(is_os64bit == TRUE)? "AMD64": "x86");

/*	GetSystemInfo(&systemInfo);
	OSVersion += QString(" %1").arg(
		(systemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)? "AMD64":
		(systemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)? "IA64":
		(systemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)? "x86":
		"Unknown architecture");*/

	OSVERSIONINFO versionInfo;
	ZeroMemory(&versionInfo, sizeof(OSVERSIONINFO));
	versionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&versionInfo);
	OSVersion += QString(" %1.%2.%3 %4")
		.arg(versionInfo.dwMajorVersion)
		.arg(versionInfo.dwMinorVersion)
		.arg(versionInfo.dwBuildNumber)
		.arg(QString::fromWCharArray(versionInfo.szCSDVersion));

	return OSVersion;
}
#elif defined (Q_OS_LINUX)
#include <QProcess>
QString CZDialog::getOSVersion() {
	QProcess uname; 

	uname.start("uname", QStringList() << "-srvm");
	if(!uname.waitForFinished())
		return QString("Linux (unknown)");
	QString OSVersion = uname.readLine();

	return OSVersion.remove('\n');
}
#elif defined (Q_OS_MAC)
#include "plist.h"
QString CZDialog::getOSVersion() {
	char osName[256];
	char osVersion[256];
	char osBuild[256];

	if((CZPlistGet("/System/Library/CoreServices/SystemVersion.plist", "ProductName", osName, sizeof(osName)) != 0) ||
	   (CZPlistGet("/System/Library/CoreServices/SystemVersion.plist", "ProductUserVisibleVersion", osVersion, sizeof(osVersion)) != 0) ||
	   (CZPlistGet("/System/Library/CoreServices/SystemVersion.plist", "ProductBuildVersion", osBuild, sizeof(osBuild)) != 0))
	   return QString("Mac OS X (unknown)");
	QString OSVersion = QString(osName) + " " + QString(osVersion) + " " + QString(osBuild);
	
	return OSVersion.remove('\n');
}
#else//!Q_WS_WIN
#error Function getOSVersion() is not implemented for your platform!
#endif//Q_WS_WIN

/*!	\brief Export information to plane text file.
*/
void CZDialog::slotExportToText() {

	struct CZDeviceInfo info = deviceList[comboDevice->currentIndex()]->info();

	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Text Report as..."),
		QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + QDir::separator() + tr("%1.txt").arg(tr(CZ_NAME_SHORT)),
		tr("Text files (*.txt);;All files (*.*)"));

	if(fileName.isEmpty())
		return;

	CZLog(CZLogLevelModerate, "Export to text as %s", fileName.toLocal8Bit().data());

	QFile file(fileName);
	if(!file.open(QFile::WriteOnly | QFile::Text)) {
		QMessageBox::warning(this, tr(CZ_NAME_SHORT),
			tr("Cannot write file %1:\n%2.").arg(fileName).arg(file.errorString()));
		return;
	}

	QTextStream out(&file);
	QString title = tr(CZ_NAME_SHORT " Report");
	QString subtitle;

	out << title << endl;
	for(int i = 0; i < title.size(); i++)
		out << "=";
	out << endl;
	out << QString("%1: %2").arg(tr("Version")).arg(CZ_VERSION);
#ifdef CZ_VER_STATE
	out << QString(" %1 %2 %3 ").arg("Built").arg(CZ_DATE).arg(CZ_TIME);
#endif//CZ_VER_STATE
	out << endl;
	out << CZ_ORG_URL_MAINPAGE << endl;
	out << QString("%1: %2").arg(tr("OS Version")).arg(getOSVersion()) << endl;

	QString version;
	if(info.drvDllVer == 0) {
		version = tr("Unknown");
	} else {
		version = QString("%1").arg(info.drvVersion);
	}
	out << QString("%1: %2").arg(tr("Driver Version")).arg(version) << endl;

	if(info.drvDllVer == 0) {
		version = tr("Unknown");
	} else {
		version = QString("%1.%2")
			.arg(info.drvDllVer / 1000)
			.arg(info.drvDllVer % 1000);
	}
	if(strlen(info.drvDllVerStr) != 0) {
		version += " (" + QString(info.drvDllVerStr) + ")";
	}
	out << QString("%1: %2").arg(tr("Driver Dll Version")).arg(version) << endl;

	if(info.rtDllVer == 0) {
		version = tr("Unknown");
	} else {
		version = QString("%1.%2")
			.arg(info.rtDllVer / 1000)
			.arg(info.rtDllVer % 1000);
	}
	if(strlen(info.rtDllVerStr) != 0) {
		version += " (" + QString(info.rtDllVerStr) + ")";
	}
	out << QString("%1: %2").arg(tr("Runtime Dll Version")).arg(version) << endl;
	out << endl;

	subtitle = tr("Core Information");
	out << subtitle << endl;
	for(int i = 0; i < subtitle.size(); i++)
		out << "-";
	out << endl;
	out << "\t" << QString("%1: %2").arg(tr("Name")).arg(info.deviceName) << endl;
	out << "\t" << QString("%1: %2.%3").arg(tr("Compute Capability")).arg(info.major).arg(info.minor) << endl;
	out << "\t" << QString("%1: %2").arg(tr("Clock Rate")).arg(getValue1000(info.core.clockRate, prefixKilo, tr("Hz"))) << endl;
	out << "\t" << tr("Multiprocessors") << ": ";
	if(info.core.muliProcCount == 0)
		out << tr("Unknown") << endl;
	else
		out << info.core.muliProcCount << endl;
	out << "\t" << QString("%1: %2").arg(tr("Warp Size")).arg(info.core.SIMDWidth) << endl;
	out << "\t" << QString("%1: %2").arg(tr("Regs Per Block")).arg(info.core.regsPerBlock) << endl;
	out << "\t" << QString("%1: %2").arg(tr("Threads Per Block")).arg(info.core.maxThreadsPerBlock) << endl;
	out << "\t" << QString("%1: %2 x %3 x %4").arg(tr("Threads Dimensions")).arg(info.core.maxThreadsDim[0]).arg(info.core.maxThreadsDim[1]).arg(info.core.maxThreadsDim[2]) << endl;
	out << "\t" << QString("%1: %2 x %3 x %4").arg(tr("Grid Dimensions")).arg(info.core.maxGridSize[0]).arg(info.core.maxGridSize[1]).arg(info.core.maxGridSize[2]) << endl;
	out << "\t" << QString("%1: %2").arg(tr("Watchdog Enabled")).arg(info.core.watchdogEnabled? tr("Yes"): tr("No")) << endl;
	out << "\t" << QString("%1: %2").arg(tr("Integrated GPU")).arg(info.core.integratedGpu? tr("Yes"): tr("No")) << endl;
	out << "\t" << QString("%1: %2").arg(tr("Concurrent Kernels")).arg(info.core.concurrentKernels? tr("Yes"): tr("No")) << endl;
	out << "\t" << QString("%1: %2").arg(tr("Compute Mode")).arg(
		(info.core.computeMode == CZComputeModeDefault)? tr("Default"):
		(info.core.computeMode == CZComputeModeExclusive)? tr("Compute-exclusive"):
		(info.core.computeMode == CZComputeModeProhibited)? tr("Compute-prohibited"):
		tr("Unknown")) << endl;
	out << endl;

	subtitle = tr("Memory Information");
	out << subtitle << endl;
	for(int i = 0; i < subtitle.size(); i++)
		out << "-";
	out << endl;
	out << "\t" << QString("%1: %2").arg(tr("Total Global")).arg(getValue1024(info.mem.totalGlobal, prefixNothing, tr("B"))) << endl;
	out << "\t" << QString("%1: %2").arg(tr("Shared Per Block")).arg(getValue1024(info.mem.sharedPerBlock, prefixNothing, tr("B"))) << endl;
	out << "\t" << QString("%1: %2").arg(tr("Pitch")).arg(getValue1024(info.mem.maxPitch, prefixNothing, tr("B"))) << endl;
	out << "\t" << QString("%1: %2").arg(tr("Total Constant")).arg(getValue1024(info.mem.totalConst, prefixNothing, tr("B"))) << endl;
	out << "\t" << QString("%1: %2").arg(tr("Texture Alignment")).arg(getValue1024(info.mem.textureAlignment, prefixNothing, tr("B"))) << endl;
	out << "\t" << QString("%1: %2").arg(tr("Texture 1D Size")).arg((double)info.mem.texture1D[0]) << endl;
	out << "\t" << QString("%1: %2 x %3").arg(tr("Texture 2D Size")).arg((double)info.mem.texture2D[0]).arg((double)info.mem.texture2D[1]) << endl;
	out << "\t" << QString("%1: %2 x %3 x %4").arg(tr("Texture 3D Size")).arg((double)info.mem.texture3D[0]).arg((double)info.mem.texture3D[1]).arg((double)info.mem.texture3D[2]) << endl;
	out << "\t" << QString("%1: %2").arg(tr("GPU Overlap")).arg(info.mem.gpuOverlap? tr("Yes"): tr("No")) << endl;
	out << "\t" << QString("%1: %2").arg(tr("Map Host Memory")).arg(info.mem.mapHostMemory? tr("Yes"): tr("No")) << endl;
	out << "\t" << QString("%1: %2").arg(tr("Error Correction")).arg(info.mem.errorCorrection? tr("Yes"): tr("No")) << endl;
	out << endl;

	subtitle = tr("Performance Information");
	out << subtitle << endl;
	for(int i = 0; i < subtitle.size(); i++)
		out << "-";
	out << endl;
	out << tr("Memory Copy") << endl;
	out << "\t" << tr("Host Pinned to Device") << ": ";
	if(info.band.copyHDPin == 0)
		out << "--" << endl;
	else
		out << getValue1024(info.band.copyHDPin, prefixKibi, tr("B/s")) << endl;
	out << "\t" << tr("Host Pageable to Device") << ": ";
	if(info.band.copyHDPage == 0)
		out << "--" << endl;
	else
		out << getValue1024(info.band.copyHDPage, prefixKibi, tr("B/s")) << endl;

	out << "\t" << tr("Device to Host Pinned") << ": ";
	if(info.band.copyDHPin == 0)
		out << "--" << endl;
	else
		out << getValue1024(info.band.copyDHPin, prefixKibi, tr("B/s")) << endl;
	out << "\t" << tr("Device to Host Pageable") << ": ";
	if(info.band.copyDHPage == 0)
		out << "--" << endl;
	else
		out << getValue1024(info.band.copyDHPage, prefixKibi, tr("B/s")) << endl;
	out << "\t" << tr("Device to Device") << ": ";
	if(info.band.copyDD == 0)
		out << "--" << endl;
	else
		out << getValue1024(info.band.copyDD, prefixKibi, tr("B/s")) << endl;
	out << tr("GPU Core Performance") << endl;
	out << "\t" << tr("Single-precision Float") << ": ";
	if(info.perf.calcFloat == 0)
		out << "--" << endl;
	else
		out << getValue1000(info.perf.calcFloat, prefixKilo, tr("flop/s")) << endl;
	out << "\t" << tr("Double-precision Float") << ": ";
	if(((info.major > 1)) ||
		((info.major == 1) && (info.minor >= 3))) {
		if(info.perf.calcDouble == 0)
			out << "--" << endl;
		else
			out << getValue1000(info.perf.calcDouble, prefixKilo, tr("flop/s")) << endl;
	} else {
		out << tr("Not Supported") << endl;
	}
	out << "\t" << tr("32-bit Integer") << ": ";
	if(info.perf.calcInteger32 == 0)
		out << "--" << endl;
	else
		out << getValue1000(info.perf.calcInteger32, prefixKilo, tr("iop/s")) << endl;
	out << "\t" << tr("24-bit Integer") << ": ";
	if(info.perf.calcInteger24 == 0)
		out << "--" << endl;
	else
		out << getValue1000(info.perf.calcInteger24, prefixKilo, tr("iop/s")) << endl;
	out << endl;

	time_t t;
	time(&t);
	out << QString("%1: %2").arg(tr("Generated")).arg(ctime(&t)) << endl;
}

/*!	\brief Export information to HTML file.
*/
void CZDialog::slotExportToHTML() {

	struct CZDeviceInfo info = deviceList[comboDevice->currentIndex()]->info();

	QString fileName = QFileDialog::getSaveFileName(this, tr("Save HTML Report as..."),
		QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + QDir::separator() + tr("%1.html").arg(tr(CZ_NAME_SHORT)),
		tr("HTML files (*.html *.htm);;All files (*.*)"));

	if(fileName.isEmpty())
		return;

	CZLog(CZLogLevelModerate, "Export to HTML as %s", fileName.toLocal8Bit().data());

	QFile file(fileName);
	if(!file.open(QFile::WriteOnly | QFile::Text)) {
		QMessageBox::warning(this, tr(CZ_NAME_SHORT),
			tr("Cannot write file %1:\n%2.").arg(fileName).arg(file.errorString()));
		return;
	}

	QTextStream out(&file);
	QString title = tr(CZ_NAME_SHORT " Report");

	out << 	"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
		"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n"
		"<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"mul\" lang=\"mul\" dir=\"ltr\">\n"
		"<head>\n"
		"<title>" << title << "</title>\n"
		"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\n"
		"<style type=\"text/css\">\n"

		"@charset \"utf-8\";\n"
		"body { font-size: 12px; font-family: Verdana, Arial, Helvetica, sans-serif; font-weight: normal; font-style: normal; }\n"
		"h1 { font-size: 15px; color: #690; }\n"
		"h2 { font-size: 13px; color: #690; }\n"
		"table { border-collapse: collapse; border: 1px solid #000; width: 500px; }\n"
		"th { background-color: #deb; text-align: left; }\n"
		"td { width: 50%; }\n"
		"a:link { color: #9c3; text-decoration: none; }\n"
		"a:visited { color: #690; text-decoration: none; }\n"
		"a:hover { color: #9c3; text-decoration: underline; }\n"
		"a:active { color: #9c3; text-decoration: underline; }\n"

		"</style>\n"
		"</head>\n"
		"<body style=\"background: #fff;\">\n";

	out <<	"<h1>" << title << "</h1>\n";
	out <<	"<p><small>";
	out <<	tr("<b>%1:</b> %2").arg(tr("Version")).arg(CZ_VERSION);
#ifdef CZ_VER_STATE
	out <<	tr(" <b>%1</b> %2 %3 ").arg(tr("Built")).arg(CZ_DATE).arg(CZ_TIME);
#endif//CZ_VER_STATE
	out <<	QString("<a href=\"%1\">%1</a><br/>\n").arg(CZ_ORG_URL_MAINPAGE);
	out <<	tr("<b>%1:</b> %2<br/>").arg(tr("OS Version")).arg(getOSVersion());

	QString version;
	if(info.drvDllVer == 0) {
		version = "<i>" + tr("Unknown") + "</i>";
	} else {
		version = QString("%1").arg(info.drvVersion);
	}
	out <<	QString("<b>%1</b>: %2<br/>").arg(tr("Driver Version")).arg(version) << endl;

	if(info.drvDllVer == 0) {
		version = "<i>" + tr("Unknown") + "</i>";
	} else {
		version = QString("%1.%2")
			.arg(info.drvDllVer / 1000)
			.arg(info.drvDllVer % 1000);
	}
	if(strlen(info.drvDllVerStr) != 0) {
		version += " (" + QString(info.drvDllVerStr) + ")";
	}
	out <<	QString("<b>%1</b>: %2<br/>").arg(tr("Driver Dll Version")).arg(version) << endl;

	if(info.rtDllVer == 0) {
		version = "<i>" + tr("Unknown") + "</i>";
	} else {
		version = QString("%1.%2")
			.arg(info.rtDllVer / 1000)
			.arg(info.rtDllVer % 1000);
	}
	if(strlen(info.rtDllVerStr) != 0) {
		version += " (" + QString(info.rtDllVerStr) + ")";
	}
	out <<	QString("<b>%1</b>: %2<br/>").arg(tr("Runtime Dll Version")).arg(version) << endl;
	out <<	"</small></p>\n";

	out << 	"<h2>" << tr("Core Information") << "</h2>\n"
		"<table border=\"1\">\n"
		"<tr><th>" << tr("Name") << "</th><td>" << info.deviceName << "</td></tr>\n"
		"<tr><th>" << tr("Compute Capability") << "</th><td>" << info.major << "." << info.minor << "</td></tr>\n"
		"<tr><th>" << tr("Clock Rate") << "</th><td>" << getValue1000(info.core.clockRate, prefixKilo, tr("Hz")) << "</td></tr>\n";
	out <<	"<tr><th>" << tr("Multiprocessors") << "</th><td>";
	if(info.core.muliProcCount == 0)
		out << "<i>" << tr("Unknown") << "</i>";
	else
		out << info.core.muliProcCount;
	out <<	"</td></tr>\n"
		"<tr><th>" << tr("Warp Size") << "</th><td>" << info.core.SIMDWidth << "</td></tr>\n"
		"<tr><th>" << tr("Regs Per Block") << "</th><td>" << info.core.regsPerBlock << "</td></tr>\n"
		"<tr><th>" << tr("Threads Per Block") << "</th><td>" << info.core.maxThreadsPerBlock << "</td></tr>\n"
		"<tr><th>" << tr("Threads Dimensions") << "</th><td>" << info.core.maxThreadsDim[0] << " x " << info.core.maxThreadsDim[1] << " x " << info.core.maxThreadsDim[2] << "</td></tr>\n"
		"<tr><th>" << tr("Grid Dimensions") << "</th><td>" << info.core.maxGridSize[0] << " x " << info.core.maxGridSize[1] << " x " << info.core.maxGridSize[2] << "</td></tr>\n"
		"<tr><th>" << tr("Watchdog Enabled") << "</th><td>" << (info.core.watchdogEnabled? tr("Yes"): tr("No")) << "</td></tr>\n"
		"<tr><th>" << tr("Integrated GPU") << "</th><td>" << (info.core.integratedGpu? tr("Yes"): tr("No")) << "</td></tr>\n"
		"<tr><th>" << tr("Concurrent Kernels") << "</th><td>" << (info.core.concurrentKernels? tr("Yes"): tr("No")) << "</td></tr>\n"
		"<tr><th>" << tr("Compute Mode") << "</th><td>" << (
			(info.core.computeMode == CZComputeModeDefault)? tr("Default"):
			(info.core.computeMode == CZComputeModeExclusive)? tr("Compute-exclusive"):
			(info.core.computeMode == CZComputeModeProhibited)? tr("Compute-prohibited"):
			tr("Unknown")) << "</td></tr>\n"
		"</table>\n";

	out << 	"<h2>" << tr("Memory Information") << "</h2>\n"
		"<table border=\"1\">\n"
		"<tr><th>" << tr("Total Global") << "</th><td>" << getValue1024(info.mem.totalGlobal, prefixNothing, tr("B")) << "</td></tr>\n"
		"<tr><th>" << tr("Shared Per Block") << "</th><td>" << getValue1024(info.mem.sharedPerBlock, prefixNothing, tr("B")) << "</td></tr>\n"
		"<tr><th>" << tr("Pitch") << "</th><td>" << getValue1024(info.mem.maxPitch, prefixNothing, tr("B")) << "</td></tr>\n"
		"<tr><th>" << tr("Total Constant") << "</th><td>" << getValue1024(info.mem.totalConst, prefixNothing, tr("B")) << "</td></tr>\n"
		"<tr><th>" << tr("Texture Alignment") << "</th><td>" << getValue1024(info.mem.textureAlignment, prefixNothing, tr("B")) << "</td></tr>\n"
		"<tr><th>" << tr("Texture 1D Size") << "</th><td>" << info.mem.texture1D[0] << "</td></tr>\n"
		"<tr><th>" << tr("Texture 2D Size") << "</th><td>" << info.mem.texture2D[0] << " x " << info.mem.texture2D[1] << "</td></tr>\n"
		"<tr><th>" << tr("Texture 3D Size") << "</th><td>" << info.mem.texture3D[0] << " x " << info.mem.texture3D[1] << " x " << info.mem.texture3D[2] << "</td></tr>\n"
		"<tr><th>" << tr("GPU Overlap") << "</th><td>" << (info.mem.gpuOverlap? tr("Yes"): tr("No")) << "</td></tr>\n"
		"<tr><th>" << tr("Map Host Memory") << "</th><td>" << (info.mem.mapHostMemory? tr("Yes"): tr("No")) << "</td></tr>\n"
		"<tr><th>" << tr("Error Correction") << "</th><td>" << (info.mem.errorCorrection? tr("Yes"): tr("No")) << "</td></tr>\n"
		"</table>\n";

	out << 	"<h2>" << tr("Performance Information") << "</h2>\n"
		"<table border=\"1\">\n"
		"<tr><th colspan=\"2\">" << tr("Memory Copy") << "</th></tr>\n"
		"<tr><th>" << tr("Host Pinned to Device") << "</th><td>";
		if(info.band.copyHDPin == 0)
			out << "--";
		else
			out << getValue1024(info.band.copyHDPin, prefixKibi, tr("B/s"));
		out << "</td></tr>\n"
		"<tr><th>" << tr("Host Pageable to Device") << "</th><td>";
		if(info.band.copyHDPage == 0)
			out << "--";
		else
			out << getValue1024(info.band.copyHDPage, prefixKibi, tr("B/s"));
		out << "</td></tr>\n"
		"<tr><th>" << tr("Device to Host Pinned") << "</th><td>";
		if(info.band.copyDHPin == 0)
			out << "--";
		else
			out << getValue1024(info.band.copyDHPin, prefixKibi, tr("B/s"));
		out << "</td></tr>\n"
		"<tr><th>" << tr("Device to Host Pageable") << "</th><td>";
		if(info.band.copyDHPage == 0)
			out << "--";
		else
			out << getValue1024(info.band.copyDHPage, prefixKibi, tr("B/s"));
		out << "</td></tr>\n"
		"<tr><th>" << tr("Device to Device") << "</th><td>";
		if(info.band.copyDD == 0)
			out << "--";
		else
			out << getValue1024(info.band.copyDD, prefixKibi, tr("B/s"));
		out << "</td></tr>\n"
		"<tr><th colspan=\"2\">" << tr("GPU Core Performance") << "</th></tr>\n"
		"<tr><th>" << tr("Single-precision Float") << "</th><td>";
		if(info.perf.calcFloat == 0)
			out << "--";
		else
			out << getValue1000(info.perf.calcFloat, prefixKilo, tr("flop/s"));
		out << "</td></tr>\n"
		"<tr><th>" << tr("Double-precision Float") << "</th><td>";
		if(((info.major > 1)) ||
			((info.major == 1) && (info.minor >= 3))) {
			if(info.perf.calcDouble == 0)
				out << "--";
			else
				out << getValue1000(info.perf.calcDouble, prefixKilo, tr("flop/s"));
		} else {
			out << "<i>" << tr("Not Supported") << "</i>";
		}
		out << "</td></tr>\n"
		"<tr><th>" << tr("32-bit Integer") << "</th><td>";
		if(info.perf.calcInteger32 == 0)
			out << "--";
		else
			out << getValue1000(info.perf.calcInteger32, prefixKilo, tr("iop/s"));
		out << "</td></tr>\n"
		"<tr><th>" << tr("24-bit Integer") << "</th><td>";
		if(info.perf.calcInteger24 == 0)
			out << "--";
		else
			out << getValue1000(info.perf.calcInteger24, prefixKilo, tr("iop/s"));
		out << "</td></tr>\n"
		"</table>\n";

	time_t t;
	time(&t);
	out <<	"<p><small><b>" << tr("Generated") << ":</b> " << ctime(&t) << "</small></p>\n";

	out <<	"<p><a href=\"http://cuda-z.sourceforge.net/\"><img src=\"http://cuda-z.sourceforge.net/img/web-button.png\" border=\"0\" alt=\"CUDA-Z\" title=\"CUDA-Z\" /></a></p>\n";

	out <<	"</body>\n"
		"</html>\n";
}

/*!	\brief Start version reading procedure.
*/
void CZDialog::startGetHistoryHttp() {

	url = QString(CZ_ORG_URL_MAINPAGE) + "/history.txt";
	startHttpRequest(url);
}

/*!	\brief Clean up after version reading procedure.
*/
void CZDialog::cleanGetHistoryHttp() {

}

/*!	\brief Start a HTTP request with a given \a url.
*/
void CZDialog::startHttpRequest(
	QUrl url			/*!<[in] URL to be read out. */
) {
	history = "";
	reply = qnam.get(QNetworkRequest(url));
	connect(reply, SIGNAL(finished()), this, SLOT(slotHttpFinished()));
	connect(reply, SIGNAL(readyRead()), this, SLOT(slotHttpReadyRead()));
}

/*!	\brief HTTP requst status processing slot.
*/
void CZDialog::slotHttpFinished() {

	QVariant redirectionTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);

	if(reply->error()) {
		QString errorString;
		errorString = QString("%1 %2.").arg(tr("Error")).arg(reply->errorString());
		CZLog(CZLogLevelWarning, "Get version request done with error: %s", errorString.toLocal8Bit().data());

		labelAppUpdateImg->setPixmap(QPixmap(CZ_UPD_ICON_ERROR));
		labelAppUpdate->setText(tr("Can't load version information. ") + errorString);
	} else if (!redirectionTarget.isNull()) {
		QUrl newUrl = url.resolved(redirectionTarget.toUrl());
		CZLog(CZLogLevelModerate, "Get version redirected to %s", newUrl.toString().toLocal8Bit().data());

		url = newUrl;
		startHttpRequest(url);
	} else {
		CZLog(CZLogLevelModerate, "Get version request done successfully");

		parseHistoryTxt(history);
	}

}

/*!	\brief HTTP data processing slot.
*/
void CZDialog::slotHttpReadyRead() {
	CZLog(CZLogLevelLow, "Get potrion of data %d", reply->size());
	history += reply->readAll().data();
}

/*!	\brief Parse \a history.txt received over HTTP.
*/
void CZDialog::parseHistoryTxt(
	QString history			/*!<[in] history.txt as a string. */
) {

	history.remove('\r');
	QStringList historyStrings(history.split("\n"));

	for(int i = 0; i < historyStrings.size(); i++) {
		CZLog(CZLogLevelLow, "%3d %s", i, historyStrings[i].toLocal8Bit().data());
	}

	QString lastVersion;
	QString downloadUrl;
	QString releaseNotes;

	bool validVersion = false;
	QString version;
	QString notes;
	bool criticalVersion = false;
	QString url;

	QString nameVersion("version ");
	QString nameNotes("release-notes ");
	QString nameCritical("release-critical");
	QString nameDownload = QString("download-") + CZ_OS_PLATFORM_STR + " ";

	for(int i = 0; i < historyStrings.size(); i++) {

		if(historyStrings[i].left(nameVersion.size()) == nameVersion) {

			if(validVersion) {
				downloadUrl = url;
				releaseNotes = notes;
				lastVersion = version;
			}

			version = historyStrings[i];
			version.remove(0, nameVersion.size());
			CZLog(CZLogLevelLow, "Version found: %s", version.toLocal8Bit().data());
			notes = "";
			url = "";
			criticalVersion = false;
			validVersion = false;
		}

		if(historyStrings[i].left(nameNotes.size()) == nameNotes) {
			notes = historyStrings[i];
			notes.remove(0, nameNotes.size());
			CZLog(CZLogLevelLow, "Notes found: %s", notes.toLocal8Bit().data());
		}

		if(historyStrings[i].left(nameDownload.size()) == nameDownload) {
			url = historyStrings[i];
			url.remove(0, nameDownload.size());
			CZLog(CZLogLevelLow, "Valid URL found: %s", url.toLocal8Bit().data());
			validVersion = true;
		}

		if(historyStrings[i].left(nameCritical.size()) == nameCritical) {
			criticalVersion = true;
			CZLog(CZLogLevelLow, "Version is critical!");
		}
	}

	if(validVersion) {
		downloadUrl = url;
		releaseNotes = notes;
		lastVersion = version;
	}

	CZLog(CZLogLevelModerate, "Last valid version: %s\n%s\n%s",
		lastVersion.toLocal8Bit().data(),
		releaseNotes.toLocal8Bit().data(),
		downloadUrl.toLocal8Bit().data());

	bool isNewest = true;
	bool isNonReleased = false;

	if(!lastVersion.isEmpty()) {

		QStringList versionNumbers = lastVersion.split('.');

		#define GEN_VERSION(major, minor) ((major * 10000) + minor)
		unsigned int myVersion = GEN_VERSION(CZ_VER_MAJOR, CZ_VER_MINOR);
		unsigned int lastVersion = GEN_VERSION(versionNumbers[0].toInt(), versionNumbers[1].toInt());;

		if(myVersion < lastVersion) {
			isNewest = false;
		} else if(myVersion == lastVersion) {
			isNewest = true;
#ifdef CZ_VER_BUILD
			if(CZ_VER_BUILD < versionNumbers[2].toInt()) {
				isNewest = false;
			}
#endif//CZ_VER_BUILD
		} else { // myVersion > lastVersion
			isNonReleased = true;
		}
	}

	if(isNewest) {
		if(isNonReleased) {
			labelAppUpdateImg->setPixmap(QPixmap(CZ_UPD_ICON_WARNING));
			labelAppUpdate->setText(tr("WARNING: You are running non-released version!"));
		} else {
			labelAppUpdateImg->setPixmap(QPixmap(CZ_UPD_ICON_INFO));
			labelAppUpdate->setText(tr("No new version was found."));
		}
	} else {
		QString updateString = QString("%1 <b>%2</b>!")
			.arg(tr("New version is available")).arg(lastVersion);
		if(!downloadUrl.isEmpty()) {
			updateString += QString(" <a href=\"%1\">%2</a>")
				.arg(downloadUrl)
				.arg(tr("Download"));
		} else {
			updateString += QString(" <a href=\"%1\">%2</a>")
				.arg(CZ_ORG_URL_MAINPAGE)
				.arg(tr("Main page"));
		}
		if(!releaseNotes.isEmpty()) {
			updateString += QString(" <a href=\"%1\">%2</a>")
				.arg(releaseNotes)
				.arg(tr("Release notes"));
		}
		labelAppUpdateImg->setPixmap(QPixmap((criticalVersion == true)? CZ_UPD_ICON_DOWNLOAD_CR: CZ_UPD_ICON_DOWNLOAD));
		labelAppUpdate->setText(updateString);
	}
}

/*!	\brief This function returns value and unit in SI format.
*/
QString CZDialog::getValue1000(
	double value,			/*!<[in] Value to print. */
	int valuePrefix,		/*!<[in] Value current prefix. */
	QString unitBase		/*!<[in] Unit base string. */
) {
	const int prefixBase = 1000;
	int resPrefix = valuePrefix;

	static const char *prefixTab[prefixSiMax + 1] = {
		"",	/* prefixNothing */
		"k",	/* prefixKilo */
		"M",	/* prefixMega */
		"G",	/* prefixGiga */
		"T",	/* prefixTera */
		"P",	/* prefixPeta */
		"E",	/* prefixExa */
		"Z",	/* prefixZetta */
		"Y",	/* prefixYotta */
	};

	while((value > (10 * prefixBase)) && (resPrefix < prefixSiMax)) {
		value /= prefixBase;
		resPrefix++;
	}

	return QString("%1 %2%3").arg(value).arg(prefixTab[resPrefix]).arg(unitBase);
}

/*!	\brief This function returns value and unit in IEC 60027 format.
*/
QString CZDialog::getValue1024(
	double value,			/*!<[in] Value to print. */
	int valuePrefix,		/*!<[in] Value current prefix. */
	QString unitBase		/*!<[in] Unit base string. */
) {
	const int prefixBase = 1024;
	int resPrefix = valuePrefix;

	static const char *prefixTab[prefixIecMax + 1] = {
		"",	/* prefixNothing */
		"Ki",	/* prefixKibi */
		"Mi",	/* prefixMebi */
		"Gi",	/* prefixGibi */
		"Ti",	/* prefixTebi */
		"Pi",	/* prefixPebi */
		"Ei",	/* prefixExbi */
		"Zi",	/* prefixZebi */
		"Yi",	/* prefixYobi */
	};

	while((value > (10 * prefixBase)) && (resPrefix < prefixIecMax)) {
		value /= prefixBase;
		resPrefix++;
	}

	return QString("%1 %2%3").arg(value).arg(prefixTab[resPrefix]).arg(unitBase);
}
