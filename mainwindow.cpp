#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMouseEvent>
#include <QPainter>

#include <algorithm>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setMouseTracking(true);
    setWindowTitle(QStringLiteral("弹幕生存"));

    restartGame();

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

    p.setBrush(QColor(255, 120, 120));
    p.drawEllipse(m_enemyCenter, m_enemyRadius, m_enemyRadius);

    p.setBrush(QColor(255, 180, 180, 120));
    p.drawEllipse(m_enemyCenter, m_spawnRadius, m_spawnRadius);

    p.setBrush(QColor(255, 220, 120));
    for (const Bullet &bullet : std::as_const(m_enemyBullets)) {
        p.drawEllipse(bullet.pos, 4.0, 4.0);
    }

    p.setBrush(QColor(80, 180, 255));
    p.drawEllipse(m_playerRect.center(), m_playerRadius, m_playerRadius);

    p.setPen(QColor(220, 220, 220));
    p.drawText(20, 36, QStringLiteral("Score: %1").arg(m_score));

    if (m_gameOver) {
        p.setPen(QColor(255, 140, 140));
        QFont f = p.font();
        f.setPointSize(26);
        f.setBold(true);
        p.setFont(f);
        p.drawText(rect(), Qt::AlignCenter, QStringLiteral("GAME OVER\n按住并滑动重新开始"));
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    restartGame();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (m_gameOver) {
        restartGame();
    }
    m_lastDragPos = event->localPos();
    m_dragging = true;
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_dragging || m_gameOver) {
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
    if (m_gameOver) {
        update();
        return;
    }

    m_phase += static_cast<float>(m_frame) * m_alpha;

    if (m_frame % m_emitEvery == 0) {
        emitEnemyBullets();
    }

    updateEnemyBullets();
    resolveCollisions();

    if (!m_gameOver) {
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

void MainWindow::restartGame()
{
    resetPlayer();
    m_enemyCenter = QPointF(width() * 0.80, height() * 0.5);
    m_enemyBullets.clear();
    m_frame = 0;
    m_phase = 0.0f;
    m_score = 0;
    m_gameOver = false;
}

void MainWindow::emitEnemyBullets()
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

void MainWindow::updateEnemyBullets()
{
    const float limitMargin = 30.0f;
    const float minX = -limitMargin;
    const float minY = -limitMargin;
    const float maxX = width() + limitMargin;
    const float maxY = height() + limitMargin;

    QVector<Bullet> kept;
    kept.reserve(m_enemyBullets.size());

    for (Bullet bullet : std::as_const(m_enemyBullets)) {
        bullet.pos += bullet.velocity;
        ++bullet.age;

        if (bullet.age > m_maxAge) {
            continue;
        }

        if (bullet.pos.x() < minX || bullet.pos.x() > maxX || bullet.pos.y() < minY || bullet.pos.y() > maxY) {
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
            return;
        }
    }
}
