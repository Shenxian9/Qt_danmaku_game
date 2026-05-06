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
    enum class SceneState {
        LevelSelect,
        Playing,
        GameOver
    };

    enum class LevelType {
        Level1,
        Level2
    };

    struct Bullet
    {
        QPointF pos;
        QPointF velocity;
        int age = 0;
        int bounces = 0;
    };

    void resetPlayer();
    void restartGame();
    void startLevel(LevelType level);
    void emitEnemyBullets();
    void emitLevel1Bullets();
    void emitLevel2Bullets();
    void updateEnemyBullets();
    void resolveCollisions();

    QRectF levelButtonRect(LevelType level) const;
    QRectF retryButtonRect() const;
    QRectF backButtonRect() const;
    QPointF toPortraitUiPoint(const QPointF &screenPoint) const;

    Ui::MainWindow *ui;

    QTimer m_gameTimer;
    int m_tickMs = 16;

    QRectF m_playerRect;
    float m_playerSpeedFactor = 1.0f;
    float m_playerRadius = 8.0f;

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

    SceneState m_sceneState = SceneState::LevelSelect;
    LevelType m_currentLevel = LevelType::Level1;

    int m_level2EmitEvery = 36;
    int m_level2BulletCount = 32;
    float m_level2InitialRingRadius = 10.0f;
    float m_level2MinSpawnDistance = 30.0f;
    float m_level2MaxSpawnDistance = 120.0f;
    float m_level2InitialSpeed = 5.4f;
    float m_level2MinSpeed = 2.4f;
    float m_level2SlowdownFactor = 0.985f;
};

#endif // MAINWINDOW_H
