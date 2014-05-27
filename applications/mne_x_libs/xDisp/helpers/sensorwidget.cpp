#include "sensorwidget.h"
#include "sensoritem.h"

#include <QDebug>

#include <QFile>
#include <QToolButton>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QHBoxLayout>

SensorWidget::SensorWidget(QWidget *parent)
: QWidget(parent)
, m_pSensorModel(NULL)
{
    m_pGraphicsView = new QGraphicsView(this);

    m_pGraphicsScene = new QGraphicsScene(this);

    m_pGraphicsView->setScene(m_pGraphicsScene);

    createUI();

}



void SensorWidget::contextUpdate(const QModelIndex & topLeft, const QModelIndex & bottomRight, const QVector<int> & roles)
{
    Q_UNUSED(topLeft)
    Q_UNUSED(bottomRight)
    Q_UNUSED(roles)
    contextUpdate();
}


void SensorWidget::contextUpdate()
{

}

void SensorWidget::createUI()
{
    if(m_pSensorModel)
    {

        QVBoxLayout *layoutSelection = new QVBoxLayout;
        for(qint32 i = 0; i < m_pSensorModel->getSensorGroups().size(); ++i)
        {
            QToolButton *sensorSelectionButton = new QToolButton;
            sensorSelectionButton->setText(m_pSensorModel->getSensorGroups()[i].getGroupName());
            layoutSelection->addWidget(sensorSelectionButton);
        }


        QButtonGroup *buttonGroup = new QButtonGroup;
        buttonGroup->setExclusive(true);

        QHBoxLayout *layoutButtonGroup = new QHBoxLayout;

        for(qint32 i = 0; i < m_pSensorModel->getNumLayouts(); ++i)
        {
            QToolButton *buttonLayout = new QToolButton;
            buttonLayout->setText(m_pSensorModel->getSensorLayouts()[i].getName());
            buttonLayout->setCheckable(true);

            if(i == 0)
                buttonLayout->setChecked(true);
            else
                buttonLayout->setChecked(false);

            buttonGroup->addButton(buttonLayout, i);

            layoutButtonGroup->addWidget(buttonLayout);
        }

        connect(buttonGroup, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), m_pSensorModel, &SensorModel::setCurrentLayout);


        QGridLayout *topLayout = new QGridLayout;
        topLayout->addWidget(m_pGraphicsView, 0, 0);
        topLayout->addLayout(layoutSelection, 0, 1);
        topLayout->addLayout(layoutButtonGroup, 1, 0);

        setLayout(topLayout);
    }
}


void SensorWidget::setModel(SensorModel *model)
{
    m_pSensorModel = model;

    drawChannels();

    connect(m_pSensorModel, &QAbstractTableModel::dataChanged, this, static_cast<void (SensorWidget::*)(const QModelIndex &, const QModelIndex &, const QVector<int> &)>(&SensorWidget::contextUpdate));
    connect(m_pSensorModel, &QAbstractTableModel::modelReset, this, static_cast<void (SensorWidget::*)(void)>(&SensorWidget::contextUpdate));

    connect(m_pSensorModel, &SensorModel::newLayout, this, &SensorWidget::drawChannels);

    createUI();
}


void SensorWidget::drawChannels()
{
    if(m_pGraphicsScene)
    {
        m_pGraphicsScene->clear();

        for(qint32 i = 0; i < m_pSensorModel->rowCount(); ++i)
        {
            QString name = m_pSensorModel->data(i, 0).toString();
            QPointF loc = m_pSensorModel->data(i, 1).toPointF();
            SensorItem *item = new SensorItem(name, loc);
            item->setPos(loc);

            connect(item, &SensorItem::itemChanged, m_pSensorModel, &SensorModel::updateChannelState);
            m_pGraphicsScene->addItem(item);
        }
    }
}
