#include <QtGui/QIcon>
#include <QtGui/QKeyEvent>
#include <QtCore/Qt>
#include <QtCore/QObject>
#include <QtCore/QSize>
#include <QtCore/QProcess>
#include <QtCore/QFileInfo>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTableWidgetItem>

#include <stdio.h>

enum wcrename_modify_mode
{
	WCRENAME_MODIFY_MODE_REPLACE,
	WCRENAME_MODIFY_MODE_REPLACE_BEGIN,
	WCRENAME_MODIFY_MODE_REPLACE_END,
	WCRENAME_MODIFY_MODE_APPEND,
	WCRENAME_MODIFY_MODE_PREPEND,
	WCRENAME_MODIFY_MODE_CHANGE_EXT,
	WCRENAME_MODIFY_MODE_CHANGE_CASE,

	WCRENAME_MODIFY_MODE_COUNT
};

enum wcrename_casing
{
	WCRENAME_CASING_LOWER_WITH_UNDERSCORE,
	WCRENAME_CASING_LOWER_WITH_SPACE,
	WCRENAME_CASING_UPPER_WITH_UNDERSCORE,
	WCRENAME_CASING_UPPER_WITH_SPACE,
	WCRENAME_CASING_CAMEL,

	WCRENAME_CASING_COUNT
};

enum wcrename_copy_type
{
	WCRENAME_COPY_TYPE_SIMULATE,
	WCRENAME_COPY_TYPE_FS_MOVE,
	WCRENAME_COPY_TYPE_FS_COPY,
	WCRENAME_COPY_TYPE_P4,

	WCRENAME_COPY_TYPE_COUNT
};

const char* WCRENAME_MODIFY_MODE_NAMES[WCRENAME_MODIFY_MODE_COUNT] = {
		"replace",
		"replace-begin",
		"replace-end",
		"append",
		"prepend",
		"change-ext",
		"change-case"
};

const char* WCRENAME_CASING_NAMES[WCRENAME_CASING_COUNT] = {
		"lower_case",
		"lower case",
		"UPPER_CASE",
		"UPPER CASE",
		"CamelCase"
};

const char* WCRENAME_COPY_TYPES_NAMES[WCRENAME_COPY_TYPE_COUNT] = {
		"simulate",
		"move",
		"copy",
		"p4"
};


static void wcrename_copy_files( wcrename_copy_type type, const QStringList& src, const QStringList& dst, int count )
{
	for( int i = 0; i < count; ++i )
	{
		if( src[i] == dst[i] )
			continue;

		switch( type )
		{
			case WCRENAME_COPY_TYPE_SIMULATE:
				printf("copy %s -> %s\n", src[i].toUtf8().data(), dst[i].toUtf8().data() );
				break;
			case WCRENAME_COPY_TYPE_FS_MOVE:
				QFile( src[i] ).rename( dst[i] ); // TODO: handle error!
				break;
			case WCRENAME_COPY_TYPE_FS_COPY:
				QFile( src[i] ).copy( dst[i] ); // TODO: handle error!
				break;
			case WCRENAME_COPY_TYPE_P4:
				QProcess().start("p4", QStringList() << "move" << src[i] << dst[i]);
				break;
		}
	}
}

struct wcrename_filter_op
{
	wcrename_modify_mode _op;
	wcrename_casing _casing;
	QString _arg1;
	QString _arg2;
};

void wcrename_filter_apply( const QList<wcrename_filter_op>& ops, const QStringList& src, QStringList& dst )
{
	// ... reset ...
	for( int i = 0; i < src.size(); ++i )
		dst[i] = src[i];

	for( int op_index = 0; op_index < ops.size(); ++op_index )
	{
		for( int file_index = 0; file_index < src.size(); ++file_index )
		{
			QFileInfo fi = src[file_index];
			QString path = fi.path();
			QString file = fi.baseName();
			QString ext  = fi.suffix();

			switch( ops[op_index]._op )
			{
				case WCRENAME_MODIFY_MODE_REPLACE:
				{
					QString replace = ops[op_index]._arg1;
					QString with    = ops[op_index]._arg2;
					file = file.replace( replace, with );
				}
				break;
				case WCRENAME_MODIFY_MODE_REPLACE_BEGIN:
				{
					QString replace = ops[op_index]._arg1;
					QString with    = ops[op_index]._arg2;
					if( file.startsWith(replace) )
						file = file.replace( 0, replace.size(), with );
				}
				break;
				case WCRENAME_MODIFY_MODE_REPLACE_END:
				{
					QString replace = ops[op_index]._arg1;
					QString with    = ops[op_index]._arg2;
					if( file.endsWith(replace) )
						file = file.replace( file.size() - replace.size(), replace.size(), with );
				}
				break;
				case WCRENAME_MODIFY_MODE_APPEND:
				{
					file = file + ops[op_index]._arg1;
				}
				break;
				case WCRENAME_MODIFY_MODE_PREPEND:
				{
					file = ops[op_index]._arg1 + file;
				}
				break;
				case WCRENAME_MODIFY_MODE_CHANGE_EXT:
				{
					QString newext = ops[op_index]._arg1;
					if( !newext.isEmpty() )
						ext = newext;
				}
				break;
				case WCRENAME_MODIFY_MODE_CHANGE_CASE:
				{
					// change case ...
					file = file.replace('_', ' ');
					file = file.simplified();
					QStringList split = file.split(' ');

					switch( ops[op_index]._casing )
					{
						case WCRENAME_CASING_LOWER_WITH_UNDERSCORE:
							for( int i = 0; i < split.count(); ++i )
								split[i] = split[i].toLower();
							file = split.join('_');
							break;
						case WCRENAME_CASING_LOWER_WITH_SPACE:
							for( int i = 0; i < split.count(); ++i )
								split[i] = split[i].toLower();
							file = split.join(' ');
							break;
						case WCRENAME_CASING_UPPER_WITH_UNDERSCORE:
							for( int i = 0; i < split.count(); ++i )
								split[i] = split[i].toUpper();
							file = split.join('_');
							break;
						case WCRENAME_CASING_UPPER_WITH_SPACE:
							for( int i = 0; i < split.count(); ++i )
								split[i] = split[i].toUpper();
							file = split.join(' ');
							break;
						case WCRENAME_CASING_CAMEL:
							for( int i = 0; i < split.count(); ++i )
								split[i] = split[i].left(1).toUpper() + split[i].mid(1);
							file = split.join("");
							break;
						default:
							break;
					}
				}
				break;
			}

			dst[file_index] = path + '/' + file + '.' + ext;
		}
	}
}


class WcRenameReplaceWidget : public QWidget
{
	Q_OBJECT

	QLineEdit* _replace;
	QLineEdit* _with;

Q_SIGNALS:
	void updated();

public:

	QString replace() { return _replace->text(); }
	QString with()    { return _with->text(); }

	WcRenameReplaceWidget( QWidget* parent )
		: QWidget(parent)
	{
		setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );

		QHBoxLayout* layout = new QHBoxLayout(this);
		layout->setContentsMargins(0,0,0,0);
		_replace = new QLineEdit(this);
		_with = new QLineEdit(this);
		layout->addWidget( _replace );
		layout->addWidget( new QLabel("to:", this) );
		layout->addWidget( _with );

		_replace->setFocus();

		QObject::connect( _replace, &QLineEdit::textChanged, this, &WcRenameReplaceWidget::updated );
		QObject::connect( _with, &QLineEdit::textChanged, this, &WcRenameReplaceWidget::updated );
	}
};

class WcRenameAddWidget : public QWidget
{
	Q_OBJECT

	QLineEdit* _add;

Q_SIGNALS:
	void updated();

public:

	QString add() { return _add->text(); }

	WcRenameAddWidget( QWidget* parent )
		: QWidget(parent)
	{
		setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );

		QHBoxLayout* layout = new QHBoxLayout(this);
		layout->setContentsMargins(0,0,0,0);
		_add = new QLineEdit(this);
		layout->addWidget( _add );

		_add->setFocus();

		QObject::connect( _add, &QLineEdit::textChanged, this, &WcRenameAddWidget::updated );
	}
};

class WcRenameChangeCaseWidget : public QWidget
{
	Q_OBJECT

Q_SIGNALS:
	void updated();

public:
	QComboBox* _casing;

	wcrename_casing casing()
	{
		return (wcrename_casing)_casing->currentIndex();
	}

	WcRenameChangeCaseWidget( QWidget* parent )
		: QWidget(parent)
	{
		setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );

		QHBoxLayout* layout = new QHBoxLayout(this);
		layout->setContentsMargins(0,0,0,0);
		_casing = new QComboBox(this);
		for( int i = 0; i < WCRENAME_CASING_COUNT; ++ i )
			_casing->addItem( WCRENAME_CASING_NAMES[i] );
		layout->addWidget( _casing );

		_casing->setFocus();

		QObject::connect( _casing, (void (QComboBox::*)(int))&QComboBox::currentIndexChanged, this, &WcRenameChangeCaseWidget::updated );
	}
};

class WcRenameEditLineWidget : public QWidget
{
	Q_OBJECT
Q_SIGNALS:
	void updated();
	void addClicked();
	void subClicked();

private:
	bool eventFilter(QObject* watched, QEvent* event)
	{
		if( event->type() != QEvent::KeyPress ||
			!(QApplication::keyboardModifiers() & Qt::AltModifier) )
			return QWidget::eventFilter(watched, event);

		QKeyEvent* keyEvent = (QKeyEvent*)(event);

		if( keyEvent->key() == Qt::Key_Up )
		{
			int next_index = _replace_mode->currentIndex() - 1;
			if( next_index < 0 )
				next_index = _replace_mode->count() - 1;
			 _replace_mode->setCurrentIndex( next_index );
			 return true;
		}
		else if( keyEvent->key() == Qt::Key_Down )
		{
			int next_index = _replace_mode->currentIndex() + 1;
			if( next_index >= _replace_mode->count() )
				next_index = 0;
			 _replace_mode->setCurrentIndex( next_index );
			 return true;
		}

		return QWidget::eventFilter(watched, event);
	}

	QWidget* createModeWidget( wcrename_modify_mode mode )
	{
		switch( mode )
		{
			case WCRENAME_MODIFY_MODE_REPLACE:
			case WCRENAME_MODIFY_MODE_REPLACE_BEGIN:
			case WCRENAME_MODIFY_MODE_REPLACE_END:
			{
				WcRenameReplaceWidget* replace_widget = new WcRenameReplaceWidget(this);
				QObject::connect( replace_widget, &WcRenameReplaceWidget::updated, this, &WcRenameEditLineWidget::updated );
				replace_widget->installEventFilter(this);
				return replace_widget;
			}
			case WCRENAME_MODIFY_MODE_APPEND:
			case WCRENAME_MODIFY_MODE_PREPEND:
			case WCRENAME_MODIFY_MODE_CHANGE_EXT:
			{
				WcRenameAddWidget* replace_widget = new WcRenameAddWidget(this);
				QObject::connect( replace_widget, &WcRenameAddWidget::updated, this, &WcRenameEditLineWidget::updated );
				replace_widget->installEventFilter(this);
				return replace_widget;
			}
			case WCRENAME_MODIFY_MODE_CHANGE_CASE:
			{
				WcRenameChangeCaseWidget* replace_widget = new WcRenameChangeCaseWidget(this);
				QObject::connect( replace_widget, &WcRenameChangeCaseWidget::updated, this, &WcRenameEditLineWidget::updated );
				replace_widget->installEventFilter(this);
				replace_widget->_casing->installEventFilter(this);
				return replace_widget;
			}
		}
		return nullptr;
	}

	QWidget* createAddSubWidget()
	{
		QWidget* w = new QWidget(this);
		QHBoxLayout* addsub = new QHBoxLayout(w);
		QPushButton* add = new QPushButton(QIcon("icons/plus.png"), "", w);
		QPushButton* sub = new QPushButton(QIcon("icons/minus.png"), "", w);
		addsub->addWidget(add);
		addsub->addWidget(sub);
		add->setFocusPolicy(Qt::NoFocus);
		sub->setFocusPolicy(Qt::NoFocus);
		QObject::connect( add, &QPushButton::clicked, this, &WcRenameEditLineWidget::addClicked );
		QObject::connect( sub, &QPushButton::clicked, this, &WcRenameEditLineWidget::subClicked );
		return w;
	}

	void changeMode( int mode )
	{
		QWidget* new_mode_widget = createModeWidget((wcrename_modify_mode)mode);
		layout()->replaceWidget(_current_mode_widget, new_mode_widget);
		delete _current_mode_widget;
		_current_mode_widget = new_mode_widget;
		updated();
	}

	QComboBox* _replace_mode;
public:
	QWidget*   _current_mode_widget;

	wcrename_modify_mode replaceMode()
	{
		return (wcrename_modify_mode)_replace_mode->currentIndex();
	}

	WcRenameEditLineWidget(QWidget* parent)
		: QWidget(parent)
	{
		QHBoxLayout* layout = new QHBoxLayout(this);
		layout->setContentsMargins(0,0,0,0);

		_replace_mode = new QComboBox(this);
		for( int i = 0; i < WCRENAME_MODIFY_MODE_COUNT; ++i )
			_replace_mode->addItem( WCRENAME_MODIFY_MODE_NAMES[i] );
		_replace_mode->setCurrentIndex(WCRENAME_MODIFY_MODE_REPLACE);
		_current_mode_widget = createModeWidget(WCRENAME_MODIFY_MODE_REPLACE);

		QObject::connect( _replace_mode, (void (QComboBox::*)(int))&QComboBox::currentIndexChanged, this, &WcRenameEditLineWidget::changeMode );

		layout->addWidget( _replace_mode );
		layout->addWidget( _current_mode_widget );
		layout->addWidget( createAddSubWidget() );
	}
};

class WcRenameMainWindow : public QDialog
{
	QTableWidget* _preview;
	QVBoxLayout*  _edits;
	QComboBox*    _copy_type;

	// TODO: no more qt here to be able to compile cli without qt.
	QList<wcrename_filter_op> _ops;
	QStringList _src;
	QStringList _dst;

	QTableWidget* createPreviewWidget()
	{
		_preview = new QTableWidget(this);
		_preview->setColumnCount(2);
		_preview->setRowCount(_src.size());
		_preview->setEditTriggers(QAbstractItemView::NoEditTriggers);
		_preview->setSelectionMode(QAbstractItemView::NoSelection);
		_preview->setFocusPolicy(Qt::NoFocus);
		_preview->setHorizontalHeaderLabels(QStringList("original") << "new");
		_preview->verticalHeader()->setVisible(false);
		_preview->horizontalHeader()->setStretchLastSection(true);
		_preview->horizontalHeader()->setHighlightSections(false);
		_preview->horizontalHeader()->setSectionsClickable(false);
		_preview->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

		for( int i = 0; i < _src.size(); ++i )
		{
            _preview->setItem( i, 0, new QTableWidgetItem(_src[i]) );
            _preview->setItem( i, 1, new QTableWidgetItem(_dst[i]) );
		}

		_preview->resizeColumnsToContents();
		return _preview;
	}

	void populatePreview()
	{
        for( int i = 0; i < _dst.size(); ++i )
            _preview->item( i, 1 )->setText(_dst[i]);
        _preview->resizeColumnsToContents();
	}

	void accepted()
	{
		wcrename_copy_files( (wcrename_copy_type)_copy_type->currentIndex(), _src, _dst, _src.size() );
	}

    void keyPressEvent(QKeyEvent* ev)
    {
        if( ev->key() == Qt::Key_Return && ev->modifiers() == Qt::ControlModifier )
            accept();
        if( ev->key() == Qt::Key_Return )
            addClicked();
        else
        	QDialog::keyPressEvent(ev);
    }

    void addClicked()
	{
		WcRenameEditLineWidget* line = new WcRenameEditLineWidget(this);
		QObject::connect(line, &WcRenameEditLineWidget::updated, this, [this, line]() {
			updateEditItem(line);
		} );
		QObject::connect(line, &WcRenameEditLineWidget::addClicked, this, [this]() {
			addClicked();
		} );
		QObject::connect(line, &WcRenameEditLineWidget::subClicked, this, [this, line]() {
			subClicked(line);
		} );

		_edits->addWidget( line );
		_ops.append( wcrename_filter_op() );
		updateEditItem( line );
	}

	void subClicked(QWidget* w)
	{
		if( _edits->count() == 1 )
			return;
		int index = _edits->indexOf(w);
		_ops.removeAt(index);
		delete _edits->takeAt(index);
	}

    void updateEditItem( WcRenameEditLineWidget* line )
    {
    	int index = _edits->indexOf(line);
    	QWidget* mode_widget = line->_current_mode_widget;

		wcrename_filter_op op;
		op._op = line->replaceMode();

		switch( op._op )
		{
			case WCRENAME_MODIFY_MODE_REPLACE:
			case WCRENAME_MODIFY_MODE_REPLACE_BEGIN:
			case WCRENAME_MODIFY_MODE_REPLACE_END:
				op._arg1 = ((WcRenameReplaceWidget*)mode_widget)->replace();
				op._arg2 = ((WcRenameReplaceWidget*)mode_widget)->with();
				break;
			case WCRENAME_MODIFY_MODE_APPEND:
			case WCRENAME_MODIFY_MODE_PREPEND:
			case WCRENAME_MODIFY_MODE_CHANGE_EXT:
				op._arg1 = ((WcRenameAddWidget*)mode_widget)->add();
				break;
			case WCRENAME_MODIFY_MODE_CHANGE_CASE:
				op._casing = ((WcRenameChangeCaseWidget*)mode_widget)->casing();
				break;
		}

		_ops[index] = op;
		wcrename_filter_apply( _ops, _src, _dst );
		populatePreview();
    }

    QWidget* createEditsWidget()
    {
    	QWidget* w = new QWidget(this);
    	_edits = new QVBoxLayout(w);
    	_edits->setContentsMargins(0,0,0,0);
    	return w;
    }

    QWidget* createBottomWidget()
    {
    	QWidget* w = new QWidget(this);

    	QHBoxLayout* bottom = new QHBoxLayout(w);
    	bottom->setContentsMargins(0,0,0,0);
		_copy_type = new QComboBox(w);
		for( int i = 0; i < WCRENAME_COPY_TYPE_COUNT; ++i )
			_copy_type->addItem( WCRENAME_COPY_TYPES_NAMES[i] );
		_copy_type->setCurrentIndex( WCRENAME_COPY_TYPE_FS_MOVE );

		QWidget* ok_cancel_w = new QWidget(w);
		QHBoxLayout* ok_cancel = new QHBoxLayout(ok_cancel_w);
		ok_cancel->setContentsMargins(0,0,0,0);
		ok_cancel->setAlignment(Qt::AlignRight);
		ok_cancel->addWidget( new QPushButton("Perform", w) );
		ok_cancel->addWidget( new QPushButton("Cancel", w) );

		bottom->addWidget(_copy_type);
		bottom->addWidget(ok_cancel_w);

    	return w;
    }

public:
	WcRenameMainWindow( QStringList& source_files )
	{
		_src = source_files;
		_dst = source_files;

		setModal(true);
        setWindowTitle("wcrename - rename ALL THE FILES! (ctrl+Enter to apply, esc to cancel)");
        setWindowIcon(QIcon("icons/replace.png"));
        setMinimumSize(QSize(1024, 500));

        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->addWidget( createEditsWidget() );
        layout->addWidget( createPreviewWidget() );
        layout->addWidget( createBottomWidget() );

    	// ... add initial item ...
    	addClicked();

        QObject::connect(this, &QDialog::accepted, this, &WcRenameMainWindow::accepted );
	}
};

int main( int argc, char** argv )
{
	QApplication app(argc, argv);

	QStringList src;
	for( int i = 1; i < argc; ++i )
		src << argv[i];
	WcRenameMainWindow( src ).exec();
	return 0;
}

#include "wcrename.moc.cpp"

