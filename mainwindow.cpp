#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDateTime>
#include <QMouseEvent>
#include <QPainter>
#include <QRandomGenerator>

#include <algorithm>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setMouseTracking(true);
    setWindowTitle(QStringLiteral("简易弹幕躲避"));

    qsrand(QDateTime::currentMSecsSinceEpoch() & 0xffffffff);

    connect(&m_gameTimer, &QTimer::timeout, this, &MainWindow::gameTick);
    m_gameTimer.start(m_tickMs);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.fillRect(rect(), QColor(10, 10, 20));

    p.setPen(Qt::NoPen);
    p.setBrush(QColor(80, 180, 255));
    p.drawRoundedRect(m_playerRect, 8, 8);

    p.setBrush(QColor(255, 235, 120));
    for (const Bullet &bullet : std::as_const(m_bullets)) {
        p.drawEllipse(bullet.pos, 5.0, 5.0);
    }

    p.setBrush(QColor(255, 90, 90));
    for (const Enemy &enemy : std::as_const(m_enemies)) {
        p.drawEllipse(enemy.pos, enemy.radius, enemy.radius);
    }

    p.setPen(QColor(220, 220, 220));
    p.setBrush(Qt::NoBrush);
    p.drawText(20, 36, QStringLiteral("Score: %1").arg(m_score));

    if (m_gameOver) {
        p.setPen(QColor(255, 140, 140));
        QFont f = p.font();
        f.setPointSize(26);
        f.setBold(true);
        p.setFont(f);
        p.drawText(rect(), Qt::AlignCenter, QStringLiteral("GAME OVER\n继续滑动可移动"));
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    resetPlayer();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    m_lastDragPos = event->position();
    m_dragging = true;
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_dragging) {
        return;
    }

    const QPointF delta = event->position() - m_lastDragPos;
    m_lastDragPos = event->position();

    m_playerRect.translate(delta.x() * m_playerSpeedFactor, delta.y() * m_playerSpeedFactor);

    const qreal maxX = width() - m_playerRect.width();
    const qreal maxY = height() - m_playerRect.height();
    m_playerRect.moveLeft(qBound(0.0, m_playerRect.left(), maxX));
    m_playerRect.moveTop(qBound(0.0, m_playerRect.top(), maxY));
}

void MainWindow::gameTick()
{
    const float dt = m_tickMs / 1000.0f;

    m_bulletSpawnAccumulator += dt;
    m_enemySpawnAccumulator += dt;

    while (m_bulletSpawnAccumulator >= 0.10f) {
        shootBullet();
        m_bulletSpawnAccumulator -= 0.10f;
    }

    while (m_enemySpawnAccumulator >= 0.55f) {
        spawnEnemy();
        m_enemySpawnAccumulator -= 0.55f;
    }

    updateBullets(dt);
    updateEnemies(dt);
    resolveCollisions();

    update();
}

void MainWindow::resetPlayer()
{
    const float w = 44;
    const float h = 44;
    m_playerRect = QRectF(width() * 0.12, height() * 0.5 - h * 0.5, w, h);
}

void MainWindow::updateBullets(float dt)
{
    for (Bullet &bullet : m_bullets) {
        bullet.pos.rx() += bullet.speed * dt;
    }

    m_bullets.erase(std::remove_if(m_bullets.begin(), m_bullets.end(), [this](const Bullet &b) {
        return b.pos.x() > width() + 10;
    }), m_bullets.end());
}

void MainWindow::updateEnemies(float dt)
{
    for (Enemy &enemy : m_enemies) {
        enemy.pos.rx() -= enemy.speed * dt;
    }

    m_enemies.erase(std::remove_if(m_enemies.begin(), m_enemies.end(), [this](const Enemy &e) {
        return e.pos.x() < -e.radius - 10;
    }), m_enemies.end());
}

void MainWindow::spawnEnemy()
{
    Enemy enemy;
    enemy.radius = QRandomGenerator::global()->bounded(16.0, 30.0);
    enemy.speed = QRandomGenerator::global()->bounded(130.0, 240.0);
    enemy.pos = QPointF(width() + enemy.radius + 2,
                        QRandomGenerator::global()->bounded(enemy.radius, height() - enemy.radius));
    m_enemies.push_back(enemy);
}

void MainWindow::shootBullet()
{
    Bullet bullet;
    bullet.pos = QPointF(m_playerRect.right() + 8, m_playerRect.center().y());
    bullet.speed = 420.0f;
    m_bullets.push_back(bullet);
}

void MainWindow::resolveCollisions()
{
    const QPointF playerCenter = m_playerRect.center();
    const double playerRadius = m_playerRect.width() * 0.46;

    for (int i = m_enemies.size() - 1; i >= 0; --i) {
        Enemy &enemy = m_enemies[i];

        const double dxp = enemy.pos.x() - playerCenter.x();
        const double dyp = enemy.pos.y() - playerCenter.y();
        if (dxp * dxp + dyp * dyp < (enemy.radius + playerRadius) * (enemy.radius + playerRadius)) {
            m_gameOver = true;
        }

        bool removed = false;
        for (int j = m_bullets.size() - 1; j >= 0; --j) {
            const Bullet &bullet = m_bullets[j];
            const double dx = enemy.pos.x() - bullet.pos.x();
            const double dy = enemy.pos.y() - bullet.pos.y();
            if (dx * dx + dy * dy < (enemy.radius + 5.0) * (enemy.radius + 5.0)) {
                m_enemies.removeAt(i);
                m_bullets.removeAt(j);
                ++m_score;
                removed = true;
                break;
            }
        }

        if (removed) {
            continue;
        }
    }
}
