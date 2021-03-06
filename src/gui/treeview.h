#ifndef TREEVIEW
#define TREEVIEW

#include <QFileSystemModel>
#include <QTreeView>
#include <QUrl>
#include "neovimconnector.h"

namespace NeovimQt {

class TreeView : public QTreeView {
    Q_OBJECT
 public:
	TreeView(NeovimConnector *, QWidget *parent = 0);

 public slots:
	void open(const QModelIndex &);
	void setDirectory(const QString &, bool notify = true);
	void handleNeovimNotification(const QByteArray &, const QVariantList &);
	void connector_ready_cb();

 protected:
	std::map<std::string, QKeySequence> keys = {
		{"SetRoot", QKeySequence("Shift+R")},
		{"Close", QKeySequence("Ctrl+B")},
		{"Switch", QKeySequence("Tab")},
		{"GoUp", QKeySequence("Shift+U")},
		{"CmdLine", QKeySequence("Shift+:")},
		{"OpenE", QKeySequence("Enter")},
		{"OpenR", QKeySequence("Return")}
	};

	void keyPressEvent(QKeyEvent *event);
	QFileSystemModel *model;
	NeovimConnector *m_nvim;
};

}  // namespace NeovimQt

#endif  // TREEVIEW
