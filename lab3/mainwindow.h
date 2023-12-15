#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QTextCharFormat>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_action_Open_triggered();

    void on_action_Save_triggered();

    void on_action_SaveAs_triggered();

    void on_action_New_triggered();

    void on_action_Bold_triggered();

    void on_action_Italic_triggered();

    void on_action_Underline_triggered();

    void on_action_Left_triggered();

    void on_action_Center_triggered();

    void on_action_Right_triggered();

    void on_action_Justify_triggered();

    void currentCharFormatChanged(const QTextCharFormat &format);

    void cursorPositionChanged();

    void on_action_Color_triggered();

    void on_actionFusion_triggered();

    void on_actionWindows_triggered();

    void readSettings();
    void writeSettings();
    void on_action_6_triggered();

    void on_action_3_triggered();

private:
    Ui::MainWindow *ui;
    QString fileName;
    QSettings settings;

    bool maybeSave();
    void loadFile(const QString &fileName);
    void setCurrentFileName(const QString &fileName);
    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
    void alignmentChanged(Qt::Alignment a);
    void fontChanged(const QFont &f);
    void colorChanged(const QColor &c);
    void filePrint();
    void textFamily(const QString &f);
    void textSize(const QString &p);
    void textStyle(int styleIndex);
    int app_Counter;
};
#endif // MAINWINDOW_H
