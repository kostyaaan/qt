#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFileDialog>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QtCore/QTextCodec>
#else
#include <QtCore5Compat/QTextCodec>
#endif

#include <QMessageBox>
#include <QTextStream>
#include <QTextDocumentWriter>

#include <QClipboard>
#include <QMimeData>
#include <QColorDialog>

#ifndef QT_NO_PRINTER
#include <QPrintDialog>
#include <QPrinter>
#include <QPrintPreviewDialog>
#include <QRegExp>
#endif

#include <QComboBox>
#include <QFontComboBox>

#include<QDebug>

#include <QStyleFactory>
#include <QSettings>

#include <QTranslator>



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , settings(QSettings::NativeFormat, QSettings::SystemScope, "KhAI", "QReader")
{
    ui->setupUi(this);
    setCurrentFileName(QString());

    setCentralWidget(ui->textEdit);
    ui->textEdit->setFocus();

    #ifndef QT_NO_CLIPBOARD
        if (const QMimeData *md = QApplication::clipboard()->mimeData())
            ui->action_Paste->setEnabled(md->hasText());
    #endif

    alignmentChanged(ui->textEdit->alignment());

    connect(ui->textEdit, SIGNAL(currentCharFormatChanged(QTextCharFormat)), this, SLOT(currentCharFormatChanged(QTextCharFormat)));
    connect(ui->textEdit, SIGNAL(cursorPositionChanged()), this, SLOT(cursorPositionChanged()));

    fontChanged(ui->textEdit->font());

    auto comboStyle = new QComboBox(ui->toolBar_2);
    ui->toolBar_2->addWidget(comboStyle);
    comboStyle->addItem("Standard");
    comboStyle->addItem("Bullet List (Disc)");
    comboStyle->addItem("Bullet List (Circle)");
    comboStyle->addItem("Bullet List (Square)");
    comboStyle->addItem("Ordered List (Decimal)");
    comboStyle->addItem("Ordered List (Alpha lower)");
    comboStyle->addItem("Ordered List (Alpha upper)");
    comboStyle->addItem("Ordered List (Roman lower)");
    comboStyle->addItem("Ordered List (Roman upper)");
    connect(comboStyle, SIGNAL(activated(int)),this, SLOT(textStyle(int)));

    auto comboFont = new QFontComboBox(ui->toolBar_2);
    ui->toolBar_2->addWidget(comboFont);
    connect(comboFont, SIGNAL(activated(QString)), this, SLOT(textFamily(QString)));

    auto comboSize = new QComboBox(ui->toolBar_2);
    comboSize->setObjectName("comboSize");
    ui->toolBar_2->addWidget(comboSize);
    comboSize->setEditable(true);
    QFontDatabase db;
    foreach(int size, db.standardSizes()) comboSize->addItem(QString::number(size));
    connect(comboSize, SIGNAL(activated(QString)), this, SLOT(textSize(QString)));

    comboSize->setCurrentIndex(comboSize->findText(QString::number(QApplication::font().pointSize())));

    auto combo_App_Style = new QComboBox;
    combo_App_Style->addItems(QStyleFactory::keys());
    ui->toolBar->addWidget(combo_App_Style);
    connect(combo_App_Style,SIGNAL(activated(const QString&)),SLOT(slotChangeStyle(const QString&)));

    QString defaultStyle = QApplication::style()->metaObject()->className();
    QRegExp regExp(".(.*)\\+?Style");
    if (regExp.exactMatch(defaultStyle)) defaultStyle = regExp.cap(1);
    combo_App_Style->setCurrentIndex(combo_App_Style->findText(defaultStyle, Qt::MatchContains));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_action_Open_triggered()
{
    if (maybeSave())
    {
        QString fileName = QFileDialog::getOpenFileName(this);
        // Здесь стоит написать ваши собственные поля метода!!!
        if (!fileName.isEmpty()) loadFile(fileName);
    }
}

bool MainWindow::maybeSave()
{
    if (!ui->textEdit->document()->isModified())
        return true;
    if (fileName.startsWith(QLatin1String(":/")))
        return true;
    QMessageBox::StandardButton ret;
    ret = QMessageBox::warning(this, tr("Application"),
                               tr("The document has been modified.\n" "Do you want to save your changes?"),
                               QMessageBox::Save | QMessageBox::Discard
                                   | QMessageBox::Cancel);
    if (ret == QMessageBox::Save)
    {
        on_action_Save_triggered();
        return true;
    }
    else if (ret == QMessageBox::Cancel)
        return false;
    return true;
}

void MainWindow::setCurrentFileName(const QString &fileName)
{
    this->fileName = fileName;
    ui->textEdit->document()->setModified(false);
    QString shownName;
    if (fileName.isEmpty())
        shownName = "untitled.txt";
    else
        shownName = QFileInfo(fileName).fileName();
    setWindowTitle(tr("%1[*] - %2").arg(shownName).arg(tr("Rich Text")));
    setWindowModified(false);
}


void MainWindow::loadFile(const QString &fileName)
{
    if (!QFile::exists(fileName))
    {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot find file %1:\n%2.")
                                 .arg(fileName));
        return;
    }
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
    {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot open file %1:\n%2.")
                                 .arg(fileName));
        return;
    }
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::WaitCursor); //делаем курсор "часики"
#endif
    QByteArray data = file.readAll();
    QTextCodec *codec = Qt::codecForHtml(data);
    QString str = codec->toUnicode(data);
    if (Qt::mightBeRichText(str)) ui->textEdit->setHtml(str);
    else {
        str = QString::fromLocal8Bit(data);
        ui->textEdit->setPlainText(str);
    }
    setCurrentFileName(fileName);
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor(); //восстанавливаем обычный курсор
#endif
    statusBar()->showMessage(tr("File loaded"), 2000);
}

void MainWindow::on_action_Save_triggered()
{
    if (fileName.isEmpty())
    {
        on_action_SaveAs_triggered();
        return;
    }
    QTextDocumentWriter writer(fileName);
    bool success = writer.write(ui->textEdit->document());
    if (success) ui->textEdit->document()->setModified(false);
}


void MainWindow::on_action_SaveAs_triggered()
{
    QString fn = QFileDialog::getSaveFileName(this, tr("Save as..."), QString(), tr("ODF files (*.odt);;HTML-Files (*.htm *.html);;All Files (*)"));
    if (fn.isEmpty())
        return;
    if (! (fn.endsWith(".odt", Qt::CaseInsensitive) || fn.endsWith(".htm", Qt::CaseInsensitive) || fn.endsWith(".html", Qt::CaseInsensitive)) ) fn += ".odt";
    // default
    setCurrentFileName(fn);
    on_action_Save_triggered();
}


void MainWindow::on_action_New_triggered()
{
    if (maybeSave())
    {
        ui->textEdit->clear();
        setCurrentFileName(QString());
    }
}


void MainWindow::on_action_Bold_triggered()
{
    QTextCharFormat fmt;
    fmt.setFontWeight(ui->action_Bold->isChecked() ? QFont::Bold : QFont::Normal);
    mergeFormatOnWordOrSelection(fmt);
}


void MainWindow::on_action_Italic_triggered()
{
    QTextCharFormat fmt;
    fmt.setFontItalic(ui->action_Italic->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}


void MainWindow::on_action_Underline_triggered()
{
    QTextCharFormat fmt;
    fmt.setFontUnderline(ui->action_Underline->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void MainWindow::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
{
    QTextCursor cursor = ui->textEdit->textCursor();
    if (!cursor.hasSelection()) cursor.select(QTextCursor::WordUnderCursor);
    cursor.mergeCharFormat(format);
    ui->textEdit->mergeCurrentCharFormat(format);
}


void MainWindow::on_action_Left_triggered()
{
    ui->textEdit->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
    alignmentChanged(ui->textEdit->alignment());
}


void MainWindow::on_action_Center_triggered()
{
    ui->textEdit->setAlignment(Qt::AlignHCenter);
    alignmentChanged(ui->textEdit->alignment());
}


void MainWindow::on_action_Right_triggered()
{
    ui->textEdit->setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
    alignmentChanged(ui->textEdit->alignment());
}


void MainWindow::on_action_Justify_triggered()
{
    ui->textEdit->setAlignment(Qt::AlignJustify);
    alignmentChanged(ui->textEdit->alignment());
}

void MainWindow::alignmentChanged(Qt::Alignment a)
{
    if (a & Qt::AlignLeft) {
        ui->action_Left->setChecked(true);
        ui->action_Center->setChecked(false);
        ui->action_Right->setChecked(false);
        ui->action_Justify->setChecked(false);
    } else if (a & Qt::AlignHCenter) {
        ui->action_Left->setChecked(false);
        ui->action_Center->setChecked(true);
        ui->action_Right->setChecked(false);
        ui->action_Justify->setChecked(false);
    } else if (a & Qt::AlignRight) {
        ui->action_Left->setChecked(false);
        ui->action_Center->setChecked(false);
        ui->action_Right->setChecked(true);
        ui->action_Justify->setChecked(false);
    } else if (a & Qt::AlignJustify) {
        ui->action_Left->setChecked(false);
        ui->action_Center->setChecked(false);
        ui->action_Right->setChecked(false);
        ui->action_Justify->setChecked(true);
    }
}

void MainWindow::currentCharFormatChanged(const QTextCharFormat &format)

{
    fontChanged(format.font());
    // colorChanged(format.foreground().color());
}
void MainWindow::cursorPositionChanged()

{
    alignmentChanged(ui->textEdit->alignment());
}
void MainWindow::fontChanged(const QFont &f)
{
    ui->action_Bold->setChecked(f.bold());
    ui->action_Italic->setChecked(f.italic());
    ui->action_Underline->setChecked(f.underline());
}

void MainWindow::on_action_Color_triggered()
{
    QColor col = QColorDialog::getColor(ui->textEdit->textColor(), this);
    if (!col.isValid())
        return;
    QTextCharFormat fmt; fmt.setForeground(col);
    mergeFormatOnWordOrSelection(fmt);
    colorChanged(col);
}

void MainWindow::colorChanged(const QColor &c)
{
    QPixmap pix(16, 16);
    pix.fill(c);
    ui->action_Color->setIcon(pix);
}

void MainWindow::textFamily(const QString &f)
{
    QTextCharFormat fmt;
    fmt.setFontFamily(f);
    mergeFormatOnWordOrSelection(fmt);
}

void MainWindow::textSize(const QString &p)
{
    qreal pointSize = p.toFloat();
    if (p.toFloat() > 0) {
        QTextCharFormat fmt;
        fmt.setFontPointSize(pointSize);
        mergeFormatOnWordOrSelection(fmt);
    }
}

void MainWindow::textStyle(int styleIndex)
{
    QTextCursor cursor = ui->textEdit->textCursor();
    if (styleIndex != 0) {
        QTextListFormat::Style style = QTextListFormat::ListDisc;
        switch (styleIndex) {
            default:
            case 1:
                style = QTextListFormat::ListDisc; break;
            case 2:
                style = QTextListFormat::ListCircle; break;
            case 3:
                style = QTextListFormat::ListSquare; break;
            case 4:
                style = QTextListFormat::ListDecimal; break;
            case 5:
                style = QTextListFormat::ListLowerAlpha; break;
            case 6:
                style = QTextListFormat::ListUpperAlpha; break;
            case 7:
                style = QTextListFormat::ListLowerRoman; break;
            case 8:
                style = QTextListFormat::ListUpperRoman; break;
            }
            cursor.beginEditBlock();
            QTextBlockFormat blockFmt = cursor.blockFormat();
            QTextListFormat listFmt;
            if (cursor.currentList()) {
                //listFmt = cursor.currentList()->format();
            } else {
                listFmt.setIndent(blockFmt.indent() + 1);
                blockFmt.setIndent(0);
                cursor.setBlockFormat(blockFmt);
            }
            listFmt.setStyle(style);
            cursor.createList(listFmt);
            cursor.endEditBlock();
    } else {
            QTextBlockFormat bfmt;
            bfmt.setObjectIndex(-1);
            cursor.mergeBlockFormat(bfmt);
    }
}

void MainWindow::on_actionFusion_triggered()
{
    QApplication::setStyle(QStyleFactory::create("Fusion"));
}


void MainWindow::on_actionWindows_triggered()
{
    QApplication::setStyle(QStyleFactory::create("Windows"));
}

void MainWindow::readSettings()
{
    settings.beginGroup("/Settings");
    int app_Width = settings.value("/width", width()).toInt(); // Ширина
    int app_Height = settings.value("/height", height()).toInt(); // Высота
    app_Counter = settings.value("/counter", 1).toInt(); // Кол-во запусков
    QString str = tr("This program has been started ") +
    QString().setNum(app_Counter++) + tr(" times");
    statusBar()->showMessage(str, 3000);
    this->resize(app_Width, app_Height);
    settings.endGroup();
}
void MainWindow::writeSettings()
{
    settings.beginGroup("/Settings");
    settings.setValue("/counter", app_Counter);
    settings.setValue("/width", width());
    settings.setValue("/height", height());
    settings.endGroup();
}

void MainWindow::on_action_6_triggered()
{
    QTranslator translator;
    translator.load(":/QReader_en.qm");
    qApp->installTranslator(&translator);
    ui->retranslateUi(this);
}


void MainWindow::on_action_3_triggered()
{
    QTranslator translator;
    QCoreApplication::removeTranslator(&translator);
    ui->retranslateUi(this);
}

