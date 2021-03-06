#include "treeview.h"

#include <QDir>
#include <QHeaderView>
#include <QKeyEvent>
#include <QProcess>
#include <QStandardPaths>

static const int MAX_COLUMNS_ON_INIT = 10;

namespace NeovimQt {
TreeView::TreeView(NeovimConnector *nvim, QWidget *parent)
: QTreeView(parent), m_nvim(nvim) {
	model = new QFileSystemModel(this);

	setModel(model);

	header()->hide();

	for (int i = 1; i < MAX_COLUMNS_ON_INIT; i++) {
		hideColumn(i);
	}

	if (m_nvim->isReady()) {
		connector_ready_cb();
	}
	connect(m_nvim, &NeovimConnector::ready, this, &TreeView::connector_ready_cb);
}

void TreeView::connector_ready_cb() {
	connect(this, &TreeView::doubleClicked, this, &TreeView::open);

	connect(m_nvim->neovimObject(), &NeovimApi1::neovimNotification, this,
			&TreeView::handleNeovimNotification);

	m_nvim->neovimObject()->vim_subscribe("Dir");
	m_nvim->neovimObject()->vim_subscribe("Gui");
}

void TreeView::keyPressEvent(QKeyEvent *event) {
	QModelIndex currentIndex = TreeView::currentIndex();
	
	if (currentIndex.isValid())
	{
		QKeySequence currentK = QKeySequence(event->modifiers()+event->key());

		if (keys["OpenE"].matches(currentK) || keys["OpenR"].matches(currentK)) {
			QFileInfo info = model->fileInfo(currentIndex);

			if (info.isFile() && info.isReadable())
				open(currentIndex);
			else if (info.isDir()) {
				if (isExpanded(currentIndex))
					collapse(currentIndex);
				else 
					expand(currentIndex);
			}
		} else if (keys["Switch"].matches(currentK)) {
			focusNextChild();
		} else if (keys["Close"].matches(currentK)) {
			focusNextChild();
			hide();
		} else if (keys["CmdLine"].matches(currentK)) {
			focusNextChild();
			m_nvim->neovimObject()->vim_feedkeys(":", "m", true);		
		} else if (keys["SetRoot"].matches(currentK)) {
			QFileInfo file = model->fileInfo(currentIndex);
			QString dir = file.absoluteFilePath();

			setDirectory(dir, 1);	
		} else if (keys["GoUp"].matches(currentK)) {
			QDir tmp = QDir(QDir::current());
			tmp.cdUp();
			QString dir = tmp.path();

			setDirectory(dir, 1);
		} else {
			QTreeView::keyPressEvent(event);
		}
	}
}

void TreeView::open(const QModelIndex &index) {
	QFileInfo info = model->fileInfo(index);
	if (info.isFile() && info.isReadable()) {
		QVariantList args;
		args << info.filePath();
		m_nvim->neovimObject()->vim_call_function("GuiDrop", args);
	}
	// focusNextChild();
}

void TreeView::setDirectory(const QString &dir, bool notify) {
	if (QDir(dir).exists()) {
		QDir::setCurrent(dir);
		model->setRootPath(dir);
		setRootIndex(model->index(dir));
		if (notify) {
			m_nvim->neovimObject()->vim_change_directory(
				QByteArray::fromStdString(dir.toStdString()));
		}
	}
}

void TreeView::handleNeovimNotification(const QByteArray &name,
					const QVariantList &args) {
	if (name == "Dir" && args.size() >= 0) {
		setDirectory(m_nvim->decode(args.at(0).toByteArray()), false);
	} else if (name == "Gui"
	           && args.size() > 1
			   && m_nvim->decode(args.at(0).toByteArray()) == "TreeView") {
		QByteArray action = args.at(1).toByteArray();
		if (action == "Toggle") {
			if (isVisible())
				hide();
			else {
				show();
				focusNextChild();
			}
		} else if (action == "Switch") {
			focusNextChild();
		} else if (action == "SetKey") {
			QByteArray key = args.at(2)
				.toString()
				.remove(QChar(' '))
				.remove(QChar('\''))
				.remove(QChar('\"'))
				.toUtf8();
			QString value = args.at(3)
				.toString()
				.remove(QChar(' '))
				.remove(QChar('\''))
				.remove(QChar('\"'));

			if (key == "Switch")
				keys["Switch"] = QKeySequence(value);
			else if (key == "Close")
				keys["Close"] = QKeySequence(value);
			else if (key == "SetRoot")
				keys["SetRoot"] = QKeySequence(value);
			else if (key == "GoUp")
				keys["GoUp"] = QKeySequence(value);
		} else if (action == "ShowHide" && args.size() == 3) {
			if(args.at(2).toBool()) {
				show();
				focusNextChild();
			} else
				hide();
		}
	}
}

}  // namespace NeovimQt
