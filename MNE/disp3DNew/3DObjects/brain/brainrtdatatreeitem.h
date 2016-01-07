//=============================================================================================================
/**
* @file     brainrtdatatreeitem.h
* @author   Lorenz Esch <Lorenz.Esch@tu-ilmenau.de>;
*           Matti Hamalainen <msh@nmr.mgh.harvard.edu>
* @version  1.0
* @date     November, 2015
*
* @section  LICENSE
*
* Copyright (C) 2015, Lorenz Esch and Matti Hamalainen. All rights reserved.
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
* @brief     BrainRTDataTreeItem class declaration.
*
*/

#ifndef BRAINRTDATATREEITEM_H
#define BRAINRTDATATREEITEM_H

//*************************************************************************************************************
//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include "../../disp3DNew_global.h"

#include "../../helpers/abstracttreeitem.h"
#include "../../helpers/types.h"
#include "../../rt/sourcelevel/rtdataworker.h"

#include "braintreemetaitem.h"

#include "mne/mne_sourceestimate.h"
#include "mne/mne_forwardsolution.h"


//*************************************************************************************************************
//=============================================================================================================
// Qt INCLUDES
//=============================================================================================================

#include <QList>
#include <QVariant>
#include <QStringList>
#include <QColor>
#include <QStandardItem>
#include <QStandardItemModel>


//*************************************************************************************************************
//=============================================================================================================
// Eigen INCLUDES
//=============================================================================================================

#include <Eigen/Core>


//*************************************************************************************************************
//=============================================================================================================
// DEFINE NAMESPACE DISP3DNEWLIB
//=============================================================================================================

namespace DISP3DNEWLIB
{


//*************************************************************************************************************
//=============================================================================================================
// USED NAMESPACES
//=============================================================================================================

using namespace Eigen;
using namespace MNELIB;


//*************************************************************************************************************
//=============================================================================================================
// FORWARD DECLARATIONS
//=============================================================================================================


//=============================================================================================================
/**
* BrainRTDataTreeItem provides a generic item to hold information about real time data to plot onto the brain surface.
*
* @brief Provides a generic brain tree item to hold real time data.
*/
class DISP3DNEWSHARED_EXPORT BrainRTDataTreeItem : public AbstractTreeItem
{
    Q_OBJECT;

public:
    typedef QSharedPointer<BrainRTDataTreeItem> SPtr;             /**< Shared pointer type for BrainRTDataTreeItem class. */
    typedef QSharedPointer<const BrainRTDataTreeItem> ConstSPtr;  /**< Const shared pointer type for BrainRTDataTreeItem class. */

    //=========================================================================================================
    /**
    * Default constructor.
    *
    * @param[in] iType      The type of the item. See types.h for declaration and definition.
    * @param[in] text       The text of this item. This is also by default the displayed name of the item in a view.
    */
    explicit BrainRTDataTreeItem(const int& iType = BrainTreeModelItemTypes::UnknownItem, const QString& text = "RT Data");

    //=========================================================================================================
    /**
    * Default destructor
    */
    ~BrainRTDataTreeItem();

    //=========================================================================================================
    /**
    * AbstractTreeItem functions
    */
    QVariant data(int role = Qt::UserRole + 1) const;
    void setData(const QVariant& value, int role = Qt::UserRole + 1);

    //=========================================================================================================
    /**
    * Adds FreeSurfer data based on annotation information to this item.
    *
    * @param[in] tSourceEstimate    The MNESourceEstimate.
    * @param[in] tForwardSolution   The MNEForwardSolution.
    * @param[in] hemi               The hemispehre of this brain rt data item. This information is important in order to cut out the wanted source estimations from the MNESourceEstimate
    *
    * @return                       Returns true if successful.
    */
    bool addData(const MNESourceEstimate& tSourceEstimate, const MNEForwardSolution& tForwardSolution, const QString& hemi = "Unknown");

    //=========================================================================================================
    /**
    * Updates the rt data which is streamed by this item's worker thread item.
    *
    * @param[in] tSourceEstimate    The MNESourceEstimate.
    *
    * @return                       Returns true if successful.
    */
    bool updateData(const MNESourceEstimate& tSourceEstimate);

    //=========================================================================================================
    /**
    * Updates the rt data which is streamed by this item's worker thread item.
    *
    * @return                       Returns true if this item is initialized.
    */
    inline bool isInit() const;

signals:
    //=========================================================================================================
    /**
    * Emit this signal whenever you want to provide newly generated colors from the stream rt data.
    *
    * @param[in] sourceColorSamples     The color values for each estimated source.
    * @param[in] vertexIndex            The vertex idnex of each estiamted source.
    */
    void rtVertColorUpdated(QByteArray sourceColorSamples, VectorXi vertexIndex);

private slots:
    //=========================================================================================================
    /**
    * This slot gets called whenever the check/actiation state of the rt data worker changed.
    *
    * @param[in] checkState     The check state of the worker.
    */
    void onCheckStateWorkerChanged(const Qt::CheckState& checkState);

    //=========================================================================================================
    /**
    * This slot gets called whenever this item receives new color values for each estimated source.
    *
    * @param[in] sourceColorSamples     The color values for each estimated source.
    */
    void onStcSample(QByteArray sourceColorSamples);

    //=========================================================================================================
    /**
    * This slot gets called whenever the used colormap type changed.
    *
    * @param[in] sColormapType     The name of the new colormap type.
    */
    void onColormapTypeChanged(const QString& sColormapType);

    //=========================================================================================================
    /**
    * This slot gets called whenever the time interval in between the streamed samples changed.
    *
    * @param[in] iMSec     The new time in milliseconds waited in between each streamed sample.
    */
    void onTimeIntervalChanged(const int& iMSec);

    //=========================================================================================================
    /**
    * This slot gets called whenever the normaization value changed. The normalization value is used to normalize the estimated source activation.
    *
    * @param[in] iMSec     The new time normalization value.
    */
    void onDataNormalizationValueChanged(const double& dValue);

private:
    bool                m_bInit;            /**< The init flag. */
    RtDataWorker*       m_pRtDataWorker;    /**< The source data worker. This worker streams the rt data to this item.*/
};

//*************************************************************************************************************
//=============================================================================================================
// INLINE DEFINITIONS
//=============================================================================================================

inline bool BrainRTDataTreeItem::isInit() const
{
    return m_bInit;
}

} //NAMESPACE DISP3DNEWLIB

#endif // BRAINRTDATATREEITEM_H
