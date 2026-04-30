#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPointF>
#include <QRectF>
#include <QTimer>
#include <QVector>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private slots:
    void gameTick();

private:
    struct Bullet
    {
        QPointF pos;
        QPointF velocity;
        int age = 0;
    };

    void resetPlayer();
    void restartGame();
    void emitEnemyBullets();
    void updateEnemyBullets();
    void resolveCollisions();

    Ui::MainWindow *ui;

    QTimer m_gameTimer;
    int m_tickMs = 16;

    QRectF m_playerRect;
    float m_playerSpeedFactor = 1.0f;
    float m_playerRadius = 18.0f;

    QPointF m_enemyCenter;
    float m_enemyRadius = 30.0f;

    QVector<Bullet> m_enemyBullets;

    QPointF m_lastDragPos;
    bool m_dragging = false;

    int m_frame = 0;
    float m_phase = 0.0f;

    int m_ways = 5;
    float m_alpha = 3.1415926f / 1600.0f;
    int m_emitEvery = 2;
    float m_spawnRadius = 12.0f;
    float m_bulletSpeed = 3.0f;
    int m_maxAge = 420;

    int m_score = 0;
    bool m_gameOver = false;
};

#endif // MAINWINDOW_H
