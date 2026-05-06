#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMouseEvent>
#include <QPainter>
#include <QRandomGenerator>

#include <algorithm>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setMouseTracking(true);
    setWindowTitle(QStringLiteral("弹幕生存"));

    resetPlayer();

    connect(&m_gameTimer, &QTimer::timeout, this, &MainWindow::gameTick);
    m_gameTimer.start(m_tickMs);
}

MainWindow::~MainWindow()
{
    delete ui;
}

QRectF MainWindow::levelButtonRect(LevelType level) const
{
    const qreal w = std::min(width() * 0.5, 360.0);
    const qreal h = 72.0;
    const qreal x = (width() - w) * 0.5;
    const qreal yBase = height() * 0.36;
    const qreal y = (level == LevelType::Level1) ? yBase : (yBase + 100.0);
    return QRectF(x, y, w, h);
}

QRectF MainWindow::retryButtonRect() const
{
    const qreal w = 180.0;
    const qreal h = 60.0;
    return QRectF(width() * 0.5 - w - 16.0, height() * 0.65, w, h);
}

QRectF MainWindow::backButtonRect() const
{
    const qreal w = 220.0;
    const qreal h = 60.0;
    return QRectF(width() * 0.5 + 16.0, height() * 0.65, w, h);
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.fillRect(rect(), QColor(10, 10, 20));

    if (m_sceneState == SceneState::LevelSelect) {
        p.setPen(QColor(230, 230, 230));
        QFont titleFont = p.font();
        titleFont.setPointSize(28);
        titleFont.setBold(true);
        p.setFont(titleFont);
        p.drawText(rect().adjusted(0, 40, 0, 0), Qt::AlignTop | Qt::AlignHCenter, QStringLiteral("选择关卡"));

        const QRectF level1Rect = levelButtonRect(LevelType::Level1);
        const QRectF level2Rect = levelButtonRect(LevelType::Level2);

        p.setPen(Qt::NoPen);
        p.setBrush(QColor(70, 120, 220));
        p.drawRoundedRect(level1Rect, 12, 12);
        p.setBrush(QColor(130, 80, 220));
        p.drawRoundedRect(level2Rect, 12, 12);

        p.setPen(QColor(245, 245, 245));
        QFont btnFont = p.font();
        btnFont.setPointSize(16);
        btnFont.setBold(true);
        p.setFont(btnFont);
        p.drawText(level1Rect, Qt::AlignCenter, QStringLiteral("第一关：旋转散射"));
        p.drawText(level2Rect, Qt::AlignCenter, QStringLiteral("第二关：弹性扩散环"));

        p.setPen(QColor(200, 200, 200));
        p.drawText(QRectF(0, height() - 80, width(), 40), Qt::AlignCenter, QStringLiteral("点击任意关卡开始，按 ESC 或关闭窗口退出"));
        return;
    }

    p.setPen(Qt::NoPen);

    p.setBrush(QColor(255, 120, 120));
    p.drawEllipse(m_enemyCenter, m_enemyRadius, m_enemyRadius);

    p.setBrush(QColor(255, 180, 180, 120));
    p.drawEllipse(m_enemyCenter, m_spawnRadius, m_spawnRadius);

    p.setBrush(QColor(255, 220, 120));
    for (const Bullet &bullet : std::as_const(m_enemyBullets)) {
        const double angle = std::atan2(bullet.velocity.y(), bullet.velocity.x()) * 180.0 / 3.1415926;
        p.save();
        p.translate(bullet.pos);
        p.rotate(angle);
        p.drawEllipse(QRectF(-6.0, -3.5, 12.0, 7.0));
        p.restore();
    }

    p.setBrush(QColor(80, 180, 255));
    p.drawEllipse(m_playerRect.center(), m_playerRadius, m_playerRadius);

    p.setPen(QColor(220, 220, 220));
    p.drawText(20, 36, QStringLiteral("Score: %1").arg(m_score));
    p.drawText(20, 64, QStringLiteral("关卡: %1").arg(m_currentLevel == LevelType::Level1 ? QStringLiteral("1") : QStringLiteral("2")));

    if (m_sceneState == SceneState::GameOver) {
        p.setPen(QColor(255, 140, 140));
        QFont f = p.font();
        f.setPointSize(26);
        f.setBold(true);
        p.setFont(f);
        p.drawText(QRectF(0, height() * 0.2, width(), 120), Qt::AlignHCenter, QStringLiteral("GAME OVER"));

        const QRectF retryRect = retryButtonRect();
        const QRectF backRect = backButtonRect();

        p.setPen(Qt::NoPen);
        p.setBrush(QColor(70, 170, 90));
        p.drawRoundedRect(retryRect, 10, 10);
        p.setBrush(QColor(120, 120, 140));
        p.drawRoundedRect(backRect, 10, 10);

        p.setPen(QColor(245, 245, 245));
        QFont btnFont = p.font();
        btnFont.setPointSize(14);
        btnFont.setBold(true);
        p.setFont(btnFont);
        p.drawText(retryRect, Qt::AlignCenter, QStringLiteral("重来"));
        p.drawText(backRect, Qt::AlignCenter, QStringLiteral("回到选关"));
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    resetPlayer();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    const QPointF clickPos = event->localPos();

    if (m_sceneState == SceneState::LevelSelect) {
        if (levelButtonRect(LevelType::Level1).contains(clickPos)) {
            startLevel(LevelType::Level1);
        } else if (levelButtonRect(LevelType::Level2).contains(clickPos)) {
            startLevel(LevelType::Level2);
        }
        update();
        return;
    }

    if (m_sceneState == SceneState::GameOver) {
        if (retryButtonRect().contains(clickPos)) {
            restartGame();
            m_sceneState = SceneState::Playing;
        } else if (backButtonRect().contains(clickPos)) {
            m_sceneState = SceneState::LevelSelect;
            m_enemyBullets.clear();
            m_dragging = false;
        }
        update();
        return;
    }

    m_lastDragPos = clickPos;
    m_dragging = true;
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_dragging || m_sceneState != SceneState::Playing) {
        return;
    }

    const QPointF delta = event->localPos() - m_lastDragPos;
    m_lastDragPos = event->localPos();

    m_playerRect.translate(delta.x() * m_playerSpeedFactor, delta.y() * m_playerSpeedFactor);

    const qreal maxX = width() - m_playerRect.width();
    const qreal maxY = height() - m_playerRect.height();
    m_playerRect.moveLeft(qBound(0.0, m_playerRect.left(), maxX));
    m_playerRect.moveTop(qBound(0.0, m_playerRect.top(), maxY));
}

void MainWindow::gameTick()
{
    if (m_sceneState != SceneState::Playing) {
        update();
        return;
    }

    m_phase += static_cast<float>(m_frame) * m_alpha;

    emitEnemyBullets();
    updateEnemyBullets();
    resolveCollisions();

    if (m_sceneState == SceneState::Playing) {
        ++m_score;
    }

    ++m_frame;
    update();
}

void MainWindow::resetPlayer()
{
    const float d = m_playerRadius * 2.0f;
    m_playerRect = QRectF(width() * 0.20 - m_playerRadius, height() * 0.5 - m_playerRadius, d, d);
}

void MainWindow::startLevel(LevelType level)
{
    m_currentLevel = level;
    restartGame();
    m_sceneState = SceneState::Playing;
}

void MainWindow::restartGame()
{
    resetPlayer();
    m_enemyCenter = QPointF(width() * 0.80, height() * 0.5);
    m_enemyBullets.clear();
    m_frame = 0;
    m_phase = 0.0f;
    m_score = 0;
    m_gameOver = false;
    m_dragging = false;
}

void MainWindow::emitEnemyBullets()
{
    if (m_currentLevel == LevelType::Level1) {
        if (m_frame % m_emitEvery == 0) {
            emitLevel1Bullets();
        }
    } else {
        if (m_frame % m_level2EmitEvery == 0) {
            emitLevel2Bullets();
        }
    }
}

void MainWindow::emitLevel1Bullets()
{
    for (int i = 0; i < m_ways; ++i) {
        const float theta = m_phase + static_cast<float>(i) * 2.0f * 3.1415926f / static_cast<float>(m_ways);
        Bullet bullet;
        bullet.pos = QPointF(m_enemyCenter.x() + m_spawnRadius * std::cos(theta),
                             m_enemyCenter.y() + m_spawnRadius * std::sin(theta));
        bullet.velocity = QPointF(m_bulletSpeed * std::cos(theta),
                                  m_bulletSpeed * std::sin(theta));
        m_enemyBullets.push_back(bullet);
    }
}

void MainWindow::emitLevel2Bullets()
{
    const double angle = QRandomGenerator::global()->bounded(0.0, 2.0 * 3.1415926);
    const double distance = QRandomGenerator::global()->bounded(m_level2MinSpawnDistance, m_level2MaxSpawnDistance);
    const QPointF ringCenter = QPointF(m_enemyCenter.x() + distance * std::cos(angle),
                                       m_enemyCenter.y() + distance * std::sin(angle));

    for (int i = 0; i < m_level2BulletCount; ++i) {
        const double t = static_cast<double>(i) * 2.0 * 3.1415926 / static_cast<double>(m_level2BulletCount);
        const QPointF dir(std::cos(t), std::sin(t));

        Bullet bullet;
        bullet.pos = ringCenter + dir * m_level2InitialRingRadius;
        bullet.velocity = dir * m_bulletSpeed;
        bullet.bounces = 0;
        m_enemyBullets.push_back(bullet);
    }
}

void MainWindow::updateEnemyBullets()
{
    QVector<Bullet> kept;
    kept.reserve(m_enemyBullets.size());

    for (Bullet bullet : std::as_const(m_enemyBullets)) {
        bullet.pos += bullet.velocity;
        ++bullet.age;

        bool bounced = false;
        if (bullet.pos.x() <= 0.0 || bullet.pos.x() >= width()) {
            bullet.velocity.setX(-bullet.velocity.x());
            bullet.pos.setX(qBound(0.0, bullet.pos.x(), static_cast<double>(width())));
            bounced = true;
        }
        if (bullet.pos.y() <= 0.0 || bullet.pos.y() >= height()) {
            bullet.velocity.setY(-bullet.velocity.y());
            bullet.pos.setY(qBound(0.0, bullet.pos.y(), static_cast<double>(height())));
            bounced = true;
        }

        if (bounced) {
            ++bullet.bounces;
        }

        if (bullet.age > m_maxAge) {
            continue;
        }

        if (m_currentLevel == LevelType::Level2 && bullet.bounces > 3) {
            continue;
        }

        const float margin = 40.0f;
        if (m_currentLevel == LevelType::Level1 &&
            (bullet.pos.x() < -margin || bullet.pos.x() > width() + margin || bullet.pos.y() < -margin || bullet.pos.y() > height() + margin)) {
            continue;
        }

        kept.push_back(bullet);
    }

    m_enemyBullets = kept;
}

void MainWindow::resolveCollisions()
{
    const QPointF playerCenter = m_playerRect.center();

    for (const Bullet &bullet : std::as_const(m_enemyBullets)) {
        const double dx = bullet.pos.x() - playerCenter.x();
        const double dy = bullet.pos.y() - playerCenter.y();
        if (dx * dx + dy * dy <= (m_playerRadius + 4.0) * (m_playerRadius + 4.0)) {
            m_gameOver = true;
            m_sceneState = SceneState::GameOver;
            m_dragging = false;
            return;
        }
    }
}
