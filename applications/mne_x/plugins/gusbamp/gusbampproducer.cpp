//=============================================================================================================
/**
* @file     gusbampproducer.cpp
* @author   Viktor Klüber <viktor.klueber@tu-ilmenau.de>;
*           Lorenz Esch <lorenz.esch@tu-ilmenau.de>;
*           Matti Hamalainen <msh@nmr.mgh.harvard.edu>;
* @version  1.0
* @date     November, 2015
*
* @section  LICENSE
*
* Copyright (C) 2015, Viktor Klüber, Lorenz Esch and Matti Hamalainen. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification, are permitted provided that
* the following conditions are met:
*     * Redistributions of source code must retain the above copyright notice, this list of conditions and the
*       following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
*       the following disclaimer in the documentation and/or other materials provided with the distribution.
*     * Neither the name of MNE-CPP authors nor the names of its contributors may be used
*       to endorse or promote products derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
* PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*
* @brief    Contains the implementation of the GUSBAmpProducer class.
*
*/

//*************************************************************************************************************
//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include "gusbampproducer.h"
#include "gusbamp.h"
#include "gusbampdriver.h"

#include <QDebug>


//*************************************************************************************************************
//=============================================================================================================
// USED NAMESPACES
//=============================================================================================================

using namespace GUSBAmpPlugin;


//*************************************************************************************************************
//=============================================================================================================
// DEFINE MEMBER METHODS
//=============================================================================================================

GUSBAmpProducer::GUSBAmpProducer(GUSBAmp* pGUSBAmp)
: m_pGUSBAmp(pGUSBAmp)
, m_pGUSBAmpDriver(new GUSBAmpDriver(this))
, m_bIsRunning(true)
{
}


//*************************************************************************************************************

GUSBAmpProducer::~GUSBAmpProducer()
{
    //qDebug() << "GUSBAmpProducer::~GUSBAmpProducer()" << endl;
}


//*************************************************************************************************************

void GUSBAmpProducer::start()
{
    //Initialise device
    if(m_pGUSBAmpDriver->initDevice())
    {
        m_bIsRunning = true;
        QThread::start();
    }
    else
        m_bIsRunning = false;
}


//*************************************************************************************************************

void GUSBAmpProducer::stop()
{
    //Wait until this thread (GUSBAmpProducer) is stopped
    m_bIsRunning = false;

    //In case the semaphore blocks the thread -> Release the QSemaphore and let it exit from the push function (acquire statement)
    m_pGUSBAmp->m_pRawMatrixBuffer_In->releaseFromPush();

    while(this->isRunning())
        m_bIsRunning = false;

    //Unitialise device only after the thread stopped
    m_pGUSBAmpDriver->uninitDevice();
}


//*************************************************************************************************************

void GUSBAmpProducer::run()
{
    MatrixXf matRawBuffer(128,100);

    while(m_bIsRunning) {
        //std::qDebug()<<"GUSBAmpProducer::run()"<<std::endl;
        //Get the GUSBAmp EEG data out of the device buffer and write received data to circular buffer
        if(m_pGUSBAmpDriver->getSampleMatrixValue(matRawBuffer))
            m_pGUSBAmp->m_pRawMatrixBuffer_In->push(&matRawBuffer);
    }

    //std::qDebug()<<"EXITING - GUSBAmpProducer::run()"<<std::endl;
}


//*************************************************************************************************************


